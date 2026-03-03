#include "framework/Generator.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "color.h"
#include "Logging.hpp"
#include "constants.hpp"
#include "TypeFactoryRegistry.hpp"
#include "utils/default_values.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "utils/templating.hpp"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"

using namespace askeleton;
using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {
// Internal-only prefix for the mirrored execution used to derive `expected`.
// These variables are not extra user inputs; they are reloaded from the same
// `.cfg` case to keep the "oracle" run isolated from the assertion run.
constexpr const char *kOraclePrefix = "oracle_";
// Constructor parameters are also read from the case data so instance setup
// uses the same inputs as the test body instead of compile-only placeholders.
constexpr const char *kCtorPrefix = "ctor_";

bool isCStringLike(const InfoType &type) {
    return type.isPointer() && type.getUnderlyingType().original == "char";
}

std::string buildStructuredReadSuffix(const InfoType &type) {
    std::string suffix = removeNamespaceQualifier(type.original);
    replaceAll(suffix, " ", "_");
    replaceAll(suffix, "<", "_");
    replaceAll(suffix, ">", "");
    replaceAll(suffix, ",", "_");
    replaceTypeCharacters(suffix);
    return suffix;
}

std::string buildFactoryReadMethod(const InfoType &type, const std::string &expr) {
    std::ostringstream ss;
    ss << "    " << type.original << " Read_" << type.formatted
       << "(string objectKey) {\n";
    ss << "        (void)objectKey;\n";
    ss << "        return " << expr << ";\n";
    ss << "    }\n";
    return ss.str();
}

std::string buildFunctionScopedFactoryReadMethod(
    const InfoType &type, const std::vector<std::pair<std::string, TypeFactory>> &scoped,
    const std::optional<TypeFactory> &fallbackFactory) {
    std::ostringstream ss;
    ss << "    " << type.original << " Read_" << type.formatted
       << "(string objectKey) {\n";
    ss << "        string functionKey = objectKey;\n";
    ss << "        size_t dot = functionKey.find(\".\");\n";
    ss << "        if (dot != string::npos) functionKey = functionKey.substr(0, dot);\n";
    ss << "        size_t suffix = functionKey.find_last_of('_');\n";
    ss << "        if (suffix != string::npos && suffix + 1 < functionKey.size() &&\n";
    ss << "            std::all_of(functionKey.begin() + suffix + 1, functionKey.end(),\n";
    ss << "                        [](unsigned char c) { return std::isdigit(c); })) {\n";
    ss << "            functionKey = functionKey.substr(0, suffix);\n";
    ss << "        }\n";

    for (const auto &[functionName, factory] : scoped) {
        ss << "        if (functionKey == \"" << functionName << "\") {\n";
        ss << "            return " << factory.expr << ";\n";
        ss << "        }\n";
    }

    if (fallbackFactory.has_value() &&
        fallbackFactory->strategy == TypeInitStrategy::Factory &&
        !fallbackFactory->expr.empty()) {
        ss << "        return " << fallbackFactory->expr << ";\n";
    } else {
        ss << "        return {};\n";
    }
    ss << "    }\n";
    return ss.str();
}

std::string buildDummyRecordReadMethod(const InfoType &type) {
    std::ostringstream ss;
    ss << "    " << type.original << " Read_" << type.formatted
       << "(string objectKey) {\n";
    ss << "        (void)objectKey;\n";
    ss << "        " << type.original << " result{};\n";

    for (const auto &field : type.getRecordFields()) {
        InfoType fieldType = field.getUnderlyingType();
        std::string value;
        if (field.defaultValue.has_value()) {
            value = formatLiteralForType(field.defaultValue.value(), fieldType);
        } else if (auto def = getDefaultValueForType(fieldType)) {
            value = formatLiteralForType(def.value(), fieldType);
        } else {
            value = formatLiteralForType(getZeroValueForType(fieldType), fieldType);
        }
        if (fieldType.isRecord()) {
            value = "{}";
        }
        ss << "        result." << field.name << " = " << value << ";\n";
    }

    ss << "        return result;\n";
    ss << "    }\n";
    return ss.str();
}

std::string buildOptionalReadMethod(const InfoType &type,
                                    const std::string &valueReader) {
    std::ostringstream ss;
    ss << "    " << type.original << " Read_" << buildStructuredReadSuffix(type)
       << "(string objectKey) {\n";
    ss << "        bool hasValue = Read_bool(objectKey + \".has_value\");\n";
    ss << "        if (!hasValue) {\n";
    ss << "            return std::nullopt;\n";
    ss << "        }\n";
    ss << "        return " << type.original << "{";
    ss << valueReader << "(objectKey + \".value\")};\n";
    ss << "    }\n";
    return ss.str();
}

std::string buildPairReadMethod(const InfoType &type,
                                const std::vector<std::string> &readers) {
    std::ostringstream ss;
    ss << "    " << type.original << " Read_" << buildStructuredReadSuffix(type)
       << "(string objectKey) {\n";
    ss << "        return " << type.original << "{";
    ss << readers[0] << "(objectKey + \".first\"), ";
    ss << readers[1] << "(objectKey + \".second\")";
    ss << "};\n";
    ss << "    }\n";
    return ss.str();
}

std::string buildTupleReadMethod(const InfoType &type,
                                 const std::vector<std::string> &readers) {
    std::ostringstream ss;
    ss << "    " << type.original << " Read_" << buildStructuredReadSuffix(type)
       << "(string objectKey) {\n";
    ss << "        return std::make_tuple(";
    for (size_t i = 0; i < readers.size(); ++i) {
        if (i > 0) {
            ss << ", ";
        }
        ss << readers[i] << "(objectKey + \"."
           << i << "\")";
    }
    ss << ");\n";
    ss << "    }\n";
    return ss.str();
}

std::string getEnclosingNamespace(const InfoType &type) {
    if (type.type.isNull()) {
        return "";
    }

    const auto *record = type.type->getAsCXXRecordDecl();
    if (!record) {
        return "";
    }

    std::vector<std::string> parts;
    const auto *ctx = record->getDeclContext();
    while (ctx && !ctx->isTranslationUnit()) {
        if (const auto *ns = llvm::dyn_cast<clang::NamespaceDecl>(ctx)) {
            if (!ns->isAnonymousNamespace()) {
                parts.push_back(ns->getNameAsString());
            }
        } else if (llvm::isa<clang::RecordDecl>(ctx)) {
            // Nested classes are not namespace scopes for ADL overload injection.
            return "";
        }
        ctx = ctx->getParent();
    }

    if (parts.empty()) {
        return "";
    }

    std::reverse(parts.begin(), parts.end());
    std::ostringstream ss;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            ss << "::";
        }
        ss << parts[i];
    }
    return ss.str();
}

const clang::CXXRecordDecl *getRecordDecl(const InfoType &type) {
    if (type.type.isNull()) {
        return nullptr;
    }
    return type.type->getAsCXXRecordDecl();
}

clang::QualType normalizeComparableType(clang::QualType type) {
    if (type.isNull()) {
        return type;
    }
    return type.getNonReferenceType().getCanonicalType().getUnqualifiedType();
}

std::string normalizedComparableName(clang::QualType type) {
    if (type.isNull()) {
        return "";
    }
    std::string name = normalizeComparableType(type).getAsString();
    removeTypeQualifiers(name);
    return name;
}

bool isSameRecordType(clang::QualType candidate, const InfoType &type) {
    if (type.type.isNull() || candidate.isNull()) {
        return false;
    }
    return normalizedComparableName(candidate) == normalizedComparableName(type.type);
}

bool isOstreamLike(clang::QualType type) {
    if (type.isNull()) {
        return false;
    }
    const std::string name =
        normalizeComparableType(type).getAsString();
    return containsSubstring(name, "basic_ostream") || containsSubstring(name, "ostream");
}

bool hasMatchingFreeOperatorInContext(const clang::DeclContext *context,
                                      const InfoType &type,
                                      clang::OverloadedOperatorKind op) {
    if (!context) {
        return false;
    }

    for (const auto *decl : context->decls()) {
        if (const auto *function = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
            if (!function->isOverloadedOperator() ||
                function->getOverloadedOperator() != op) {
                continue;
            }

            if (op == clang::OO_EqualEqual && function->getNumParams() == 2 &&
                isSameRecordType(function->getParamDecl(0)->getType(), type) &&
                isSameRecordType(function->getParamDecl(1)->getType(), type)) {
                return true;
            }

            if (op == clang::OO_LessLess && function->getNumParams() == 2 &&
                isOstreamLike(function->getParamDecl(0)->getType()) &&
                isSameRecordType(function->getParamDecl(1)->getType(), type)) {
                return true;
            }
        }

        if (const auto *namespaceDecl = llvm::dyn_cast<clang::NamespaceDecl>(decl)) {
            if (hasMatchingFreeOperatorInContext(namespaceDecl, type, op)) {
                return true;
            }
        }
    }

    return false;
}

bool hasExistingEqualityOperator(const InfoType &type) {
    const auto *record = getRecordDecl(type);
    if (!record) {
        return false;
    }

    for (const auto *method : record->methods()) {
        if (method && method->isOverloadedOperator() &&
            method->getOverloadedOperator() == clang::OO_EqualEqual) {
            return true;
        }
    }

    if (hasMatchingFreeOperatorInContext(record, type, clang::OO_EqualEqual)) {
        return true;
    }

    return hasMatchingFreeOperatorInContext(
        record->getASTContext().getTranslationUnitDecl(), type, clang::OO_EqualEqual);
}

bool hasExistingStreamOperator(const InfoType &type) {
    const auto *record = getRecordDecl(type);
    if (!record) {
        return false;
    }

    if (hasMatchingFreeOperatorInContext(record, type, clang::OO_LessLess)) {
        return true;
    }

    return hasMatchingFreeOperatorInContext(
        record->getASTContext().getTranslationUnitDecl(), type, clang::OO_LessLess);
}
} // namespace

unsigned Generator::MAX_DEPTH;
const json &Generator::config = getConfig();
const nlohmann::json &Generator::templateItems = getTemplateItems();
OracleMode Generator::oracleMode = OracleMode::Mirror;

Generator::Generator(const string &targetName, const string &targetQualifiedName,
                     const string &filePath, bool isFromClass)
    : targetName(targetName), targetFilePath(filePath),
      targetFileName(extractFileName(filePath)),
      targetQualifiedName(targetQualifiedName.empty() ? targetName : targetQualifiedName),
      isFromClass(isFromClass),
      utPath(getAskeletonHome() / config["route"]["ut"] / targetName),
      configGenerator{targetName} {

    setOutputFilesPath();
    setSupportedTypes();

    fs::create_directory(utPath);
}

void Generator::appendTestCaseToTestFile(const std::string &testCase) const {
    appendToFile(testPath, "\n\n" + testCase);
}

void Generator::setOutputFilesPath() {
    const std::map<std::string, std::string> replacements = {
        {templateItems["tplitem"]["target"], targetName}};

    string fixtureFileName = config["file"]["output"]["test_fixture"];
    string makefileFileName = config["file"]["output"]["makefile"];
    string testFileName = config["file"]["output"]["test_file"];
    string supportedFileName = config["file"]["output"]["supported_types"];

    replaceTokensInText(fixtureFileName, replacements);
    replaceTokensInText(makefileFileName, replacements);
    replaceTokensInText(testFileName, replacements);
    replaceTokensInText(supportedFileName, replacements);

    fixturePath = utPath / fixtureFileName;
    makefilePath = utPath / makefileFileName;
    supportedPath = utPath / supportedFileName;
    testPath = utPath / testFileName;
}

void Generator::setValuesToChange(std::map<std::string, std::string> &valuesToChange) {
    auto sourceFile = getSourceFile(targetFilePath);
    auto headerFile = getHeaderFile(targetFilePath);
    const std::string sourcePath = sourceFile.value_or(targetFilePath);
    const bool includeSourceInFixture = !isFromClass || !headerFile.has_value();
    const std::string includePath =
        includeSourceInFixture ? sourcePath : headerFile.value();
    const bool compileSourceSeparately = includePath != sourcePath;

    if (!missingFilesWarn) {
        std::string ext = fs::path(targetFilePath).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        const bool isHeaderInput =
            (ext == ".h" || ext == ".hpp" || ext == ".hh" || ext == ".hxx");
        const bool isSourceInput = (ext == ".c" || ext == ".cc" || ext == ".cpp");

        if (!sourceFile && (isHeaderInput || (!isHeaderInput && !isSourceInput)))
            Logger::instance().recordMissingSource(targetFilePath);
        if (!headerFile && (!isHeaderInput && !isSourceInput))
            Logger::instance().recordMissingHeader(targetFilePath);
        missingFilesWarn = true;
    }

    valuesToChange = {
        {templateItems["tplitem"]["file_path"], targetFilePath},
        {templateItems["tplitem"]["target"], targetName},
        {templateItems["tplitem"]["file_name"], targetFileName},

        {templateItems["tplitem"]["cpp_path"], sourcePath},
        {templateItems["tplitem"]["header_path"], includePath},

        {templateItems["tplitem"]["date_of_generation"], getTodayString()},
        {templateItems["tplitem"]["includes"], ""},
        {templateItems["tplitem"]["namespaces"], ""},
        {templateItems["tplitem"]["new_methods"], ""},
        {"{classMemberDecl}", ""},
        {"{objectFiles}",
         compileSourceSeparately ? (targetName + ".o tests.o main.o")
                                 : "tests.o main.o"},
        {"{sourceBuildRule}",
         compileSourceSeparately
             ? (targetName + ".o: " + sourcePath + "\n\t$(CXX) -c $< -o $@\n")
             : ""},
    };
}

void Generator::setSupportedTypes() {
    fs::path supportedTypesPath =
        getAskeletonHome() / config["file"]["data"]["supported_types_json"];
    std::ifstream file(supportedTypesPath);
    if (!file.is_open())
        exitWithError(errors::openFileError(supportedTypesPath));

    nlohmann::json supportedTypesJson;
    file >> supportedTypesJson;

    supportedTypes.clear();
    for (const auto &type : supportedTypesJson) {
        supportedTypes.insert(type.get<std::string>());
    }
}

void Generator::setOutputFiles(const map<string, string> &tokensToReplace) const {
    const fs::path templatePath = this->templateFrameworkPath;

    createFileFromTemplate(templatePath / config["file"]["template"]["test_tpl"],
                           testPath, tokensToReplace);
    createFileFromTemplate(templatePath / config["file"]["template"]["fixture_tpl"],
                           fixturePath, tokensToReplace);
    createFileFromTemplate(templatePath / config["file"]["template"]["makefile_tpl"],
                           makefilePath, tokensToReplace);
}

void Generator::appendReadMethodToFixture(const std::string &method) const {
    replaceTokensInFile(
        fixturePath, fixturePath,
        {{templateItems["tplitem"]["read_object"],
          "\n\n" + method + templateItems["tplitem"]["read_object"].get<string>()}});
}

void Generator::appendOverloadMethodsToFixture(const std::string &op) const {
    replaceTokensInFile(
        fixturePath, fixturePath,
        {{templateItems["tplitem"]["overload_operator"],
          "\n\n" + op + templateItems["tplitem"]["overload_operator"].get<string>()}});
}

void Generator::createPointerReadToFixture(const InfoType &type) const {
    InfoType underlying = type.getUnderlyingType();
    string method =
        readFromFile(getMethodTemplatePath(config["file"]["method"]["pointer_read"]));
    replaceTokensInText(
        method,
        {{templateItems["tplitem"]["type"], type.original},
         {templateItems["tplitem"]["formatted"], type.formatted},
         {templateItems["tplitem"]["underlying"], underlying.original},
         {templateItems["tplitem"]["underlying_formatted"], underlying.formatted}});

    appendReadMethodToFixture(method);
}

void Generator::createEnumReadToFixture(const InfoType &type) const {
    string method = readFromFile(
        getMethodTemplatePath(config["file"]["template"]["method"]["enum_read"]));
    replaceTokensInText(method,
                        {{templateItems["tplitem"]["type"], type.original},
                         {templateItems["tplitem"]["formatted"], type.formatted}});
    appendReadMethodToFixture(method);
}

void Generator::createRecordReadToFixture(const InfoType &type) const {
    stringstream assigns;
    string method = readFromFile(
        getMethodTemplatePath(config["file"]["template"]["method"]["record_read"]));

    for (const auto &field : type.getRecordFields()) {
        string assignField = templateItems["templating"]["field_assign"];
        replaceTokensInText(assignField,
                            {{templateItems["tplitem"]["field"], field.name},
                             {templateItems["tplitem"]["formatted"], field.formatted}});
        assigns << "\n\t\t" << assignField;
    }

    replaceTokensInText(method, {{templateItems["tplitem"]["type"], type.original},
                                 {templateItems["tplitem"]["formatted"], type.formatted},
                                 {templateItems["tplitem"]["fields"], assigns.str()}});

    appendReadMethodToFixture(method);
}

void Generator::createRecordOverloadToFixture(const InfoType &type) const {
    const static string comparison = templateItems["templating"]["field_comparison"],
                        insertion = templateItems["templating"]["field_insertion"],
                        tplitemField = templateItems["tplitem"]["field"],
                        tplitemFieldFormatted =
                            templateItems["tplitem"]["field_formatted"];
    const std::string enclosingNamespace = getEnclosingNamespace(type);
    const std::string overloadType =
        enclosingNamespace.empty() ? type.original : removeNamespaceQualifier(type.original);

    stringstream comparisons, insertions;
    unsigned n_fields = type.getRecordFields().size();
    if (n_fields == 0)
        comparisons << "\t\ttrue";

    for (const auto &field : type.getRecordFields()) {
        string comparisonField = comparison, insertionField = insertion;
        map<string, string> replacements = {{tplitemField, field.name},
                                            {tplitemFieldFormatted, field.formatted}};

        replaceTokensInText(comparisonField, replacements);
        replaceTokensInText(insertionField, replacements);

        --n_fields;
        if (n_fields > 0)
            comparisonField += " &&";

        comparisons << "\t\t" << comparisonField;
        insertions << "\t" << insertionField;

        if (n_fields > 0) {
            comparisons << "\n";
            insertions << "\n";
        }
    }

    const bool needsStreamOperator = !hasExistingStreamOperator(type);
    const bool needsEqualityOperator = !hasExistingEqualityOperator(type);
    if (!needsStreamOperator && !needsEqualityOperator) {
        return;
    }

    std::ostringstream method;
    if (needsStreamOperator) {
        method << "ostream &operator<<(ostream &os, const " << overloadType
               << " &object) {\n";
        method << "\tos << \"" << overloadType << " {\\n\";\n";
        method << insertions.str() << "\n";
        method << "\tos << \"}\";\n";
        method << "\treturn os;\n";
        method << "}\n";
    }

    if (needsEqualityOperator) {
        if (needsStreamOperator) {
            method << "\n";
        }
        method << "bool operator==(const " << overloadType << "& a, const "
               << overloadType << "& b) {\n";
        method << "\treturn (\n" << comparisons.str() << "\n\t);\n";
        method << "}\n";
    }

    if (!enclosingNamespace.empty()) {
        std::ostringstream namespaced;
        namespaced << "namespace " << enclosingNamespace << " {\n\n"
                   << method.str() << "\n}\n";
        appendOverloadMethodsToFixture(namespaced.str());
        return;
    }
    appendOverloadMethodsToFixture(method.str());
}

void Generator::createTypeReadToFixture(const InfoType &type, unsigned level) {
    if (level > 1 || isTypeSupported(type))
        return;

    if (type.isOptional()) {
        const auto args = type.getTemplateArguments();
        if (args.size() == 1) {
            createTypeReadToFixture(args[0], level + 1);
            appendReadMethodToFixture(
                buildOptionalReadMethod(type, "Read_" + normalizeReadMethodType(args[0])));
            markTypeAsSupported(type);
            return;
        }
    }

    if (type.isPair()) {
        const auto args = type.getTemplateArguments();
        if (args.size() == 2) {
            std::vector<std::string> readers;
            readers.reserve(2);
            for (const auto &arg : args) {
                createTypeReadToFixture(arg, level + 1);
                readers.push_back("Read_" + normalizeReadMethodType(arg));
            }
            appendReadMethodToFixture(buildPairReadMethod(type, readers));
            markTypeAsSupported(type);
            return;
        }
    }

    if (type.isTuple()) {
        const auto args = type.getTemplateArguments();
        if (!args.empty()) {
            std::vector<std::string> readers;
            readers.reserve(args.size());
            for (const auto &arg : args) {
                createTypeReadToFixture(arg, level + 1);
                readers.push_back("Read_" + normalizeReadMethodType(arg));
            }
            appendReadMethodToFixture(buildTupleReadMethod(type, readers));
            markTypeAsSupported(type);
            return;
        }
    }

    if (type.isRecord()) {
        const auto scopedFactories = TypeFactoryRegistry::get().findFunctionFactories(type);
        const auto factory = TypeFactoryRegistry::get().find(type);
        if (!scopedFactories.empty()) {
            appendReadMethodToFixture(
                buildFunctionScopedFactoryReadMethod(type, scopedFactories, factory));
            markTypeAsSupported(type);
            return;
        }
        if (factory && factory->strategy != TypeInitStrategy::Random) {
            if (factory->strategy == TypeInitStrategy::Factory) {
                appendReadMethodToFixture(buildFactoryReadMethod(
                    type, factory->expr.empty() ? "{}" : factory->expr));
            } else if (factory->strategy == TypeInitStrategy::Zeroed) {
                appendReadMethodToFixture(buildFactoryReadMethod(type, "{}"));
            } else if (factory->strategy == TypeInitStrategy::Dummy) {
                appendReadMethodToFixture(buildDummyRecordReadMethod(type));
            }
            markTypeAsSupported(type);
            return;
        }
    }

    if (type.isPointer()) {
        createTypeReadToFixture(type.getUnderlyingType(), level + 1);

    } else if (type.isReference()) {
        createTypeReadToFixture(type.getUnderlyingType(), level + 1);

    } else if (type.isEnum()) {
        createEnumReadToFixture(type);

    } else if (type.isContainer()) {
        return;

    } else if (type.isRecord()) {
        createRecordReadToFixture(type);
        createRecordOverloadToFixture(type);
    }

    markTypeAsSupported(type);
}

void Generator::markTypeAsSupported(const InfoType &type) {
    supportedTypes.insert(type.original);
}

bool Generator::isTypeSupported(const InfoType &type) const {
    return supportedTypes.find(type.original) != supportedTypes.end();
}

void Generator::setFrameworkTemplatePath(const fs::path &frameworkPath) {
    templateFrameworkPath = frameworkPath;
}

std::string
Generator::generateParameterInitialization(const std::vector<InfoVariable> &parameters,
                                           const std::string &function,
                                           unsigned invocation,
                                           const std::string &variablePrefix,
                                           const std::string &keyPrefix) const {
    std::stringstream ss;
    for (const auto &param : parameters) {
        if (param.isOutputOnly()) {
            ss << buildWritableParameterInitialization(param, variablePrefix);
        } else if (isCStringLike(param)) {
            const std::string storageName = variablePrefix + param.name + "_storage";
            const std::string variableName = variablePrefix + param.name;
            const std::string readKey = function + "_" + std::to_string(invocation) + "." +
                                        keyPrefix + param.name;
            const bool isConstCString = containsSubstring(param.original, "const");
            ss << "\tstd::string " << storageName << " = Read_string(\"" << readKey
               << "\");\n\t" << param.original << " " << variableName << " = "
               << storageName
               << (isConstCString ? ".c_str()" : ".data()");
        } else {
            ss << "\t" << generateParameterInitialization(param.getPointers().first, function,
                                                          invocation, variablePrefix,
                                                          keyPrefix)
               ;
        }
        ss << ";";
        if (&param != &parameters.back())
            ss << "\n";
    }

    return ss.str();
}

std::string Generator::buildWritableParameterInitialization(
    const InfoVariable &variable, const std::string &variablePrefix) const {
    const InfoType underlying = variable.getUnderlyingType();
    const std::string variableName = variablePrefix + variable.name;

    if (isCStringLike(variable)) {
        std::ostringstream ss;
        ss << "\tstd::string " << variablePrefix << variable.name
           << "_storage(64, '\\0');\n";
        ss << "\t" << variable.original << " " << variableName << " = "
           << variablePrefix << variable.name << "_storage.data()";
        return ss.str();
    }

    if (underlying.isContainer()) {
        return "\t" + underlying.original + " " + variableName + "{}";
    }

    const std::string readKey = "__out." + variable.name;
    return "\t" + underlying.original + " " + variableName + " = Read_" +
           normalizeReadMethodType(underlying) + "(\"" + readKey + "\")";
}

std::string Generator::generateParameterInitialization(const InfoVariable &variable,
                                                       const std::string &function,
                                                       unsigned invocation,
                                                       const std::string &variablePrefix,
                                                       const std::string &keyPrefix) const {

    std::string readInstructionContent =
        templateItems["templating"]["assign_instruction"];
    InfoType underlying = variable.getUnderlyingType();
    std::string typeForReadMethod = normalizeReadMethodType(underlying);
    const std::string variableName = variablePrefix + variable.name;
    const std::string readKeyName = keyPrefix + variable.name;

    std::map<std::string, std::string> replacements = {
        {templateItems["tplitem"]["underlying"], underlying.original},
        {templateItems["tplitem"]["name"], variableName},
        {templateItems["tplitem"]["target"], function},
        {templateItems["tplitem"]["number"], to_string(invocation)},
        {templateItems["tplitem"]["formatted"], readKeyName},
        {templateItems["tplitem"]["underlying_formatted"], typeForReadMethod}};

    replaceTokensInText(readInstructionContent, replacements);
    return readInstructionContent;
}

std::string Generator::generateParameterInitialization(const InfoType &type,
                                                       const std::string &function,
                                                       unsigned invocation,
                                                       const std::string &variablePrefix,
                                                       const std::string &keyPrefix) const {
    InfoVariable variable{type.original, type.formatted, function + "_return"};
    return generateParameterInitialization(variable, function, invocation, variablePrefix,
                                           keyPrefix);
}

std::string Generator::generateReadInvocation(const InfoVariable &type,
                                              const string &function,
                                              unsigned invocation) const {
    string readInvocation = templateItems["templating"]["read_instruction"];
    replaceTokensInText(
        readInvocation,
        {
            {templateItems["tplitem"]["underlying_formatted"],
             normalizeReadMethodType(type)},
            {templateItems["tplitem"]["target"], function},
            {templateItems["tplitem"]["name"], type.name},
            {templateItems["tplitem"]["number"], to_string(invocation)},
        });

    return readInvocation;
}

std::string Generator::generateReturnTypeInvocation(const InfoType &type,
                                                    const string &function) const {
    InfoVariable variable{"return_" + type.formatted, type.original, type.formatted};

    return generateReadInvocation(variable, function);
}

std::pair<std::string, std::string> Generator::buildInvocationTokens(
    const std::vector<InfoVariable> &parameters, const std::string &function,
    bool isStatic, const InfoType &returnType, const std::string &symbolPrefix) const {
    const bool cStringReturn = isCStringLike(returnType);
    const std::string invocationHead = buildInvocation(
        function, isStatic, cStringReturn ? false : returnType.isPointer(), symbolPrefix);
    const std::string parametersExpr = generateParameterInvocation(parameters, symbolPrefix);

    if (!cStringReturn) {
        return {invocationHead, parametersExpr};
    }

    return {"([&]() { auto ptr = " + invocationHead,
            parametersExpr + "); return std::string(ptr ? ptr : \"\"); }()"};
}

std::string Generator::generateParameterInvocation(
    const std::vector<InfoVariable> &parameters,
    const std::string &variablePrefix) const {

    std::stringstream ss;
    for (const auto &param : parameters) {
        if (param.isPointer() && !isCStringLike(param))
            ss << "&";
        ss << variablePrefix << param.name;
        if (&param != &parameters.back())
            ss << ", ";
    }

    return ss.str();
}

std::string Generator::buildInitializations(
    const std::vector<InfoVariable> &parameters, const std::string &function,
    unsigned invocation, bool isStatic) const {
    std::vector<std::string> blocks;

    const std::string instanceInit = buildInstanceInitialization(
        function, invocation, isStatic, kCtorPrefix, kCtorPrefix, "");
    if (!instanceInit.empty()) {
        blocks.push_back(instanceInit);
    }

    const std::string paramInit =
        generateParameterInitialization(parameters, function, invocation);
    if (!paramInit.empty()) {
        blocks.push_back(paramInit);
    }

    const std::string oracleInstanceInit = buildInstanceInitialization(
        function, invocation, isStatic, std::string(kOraclePrefix) + kCtorPrefix,
        kCtorPrefix, kOraclePrefix);
    if (!oracleInstanceInit.empty()) {
        blocks.push_back(oracleInstanceInit);
    }

    const std::string oracleParamInit = generateParameterInitialization(
        parameters, function, invocation, kOraclePrefix);
    if (!oracleParamInit.empty()) {
        blocks.push_back(oracleParamInit);
    }

    if (blocks.empty()) {
        return "";
    }

    std::ostringstream ss;
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (i > 0) {
            ss << "\n\n";
        }
        ss << blocks[i];
    }

    return "\n" + ss.str();
}

std::string Generator::buildExpectedType(const InfoType &returnType) const {
    return isCStringLike(returnType) ? "std::string"
                                     : returnType.getUnderlyingType().original;
}

std::string Generator::buildExpectedInvocation(
    const std::vector<InfoVariable> &parameters, const std::string &function,
    unsigned invocation, bool isStatic, const InfoType &returnType) const {
    const auto mirrorTokens =
        buildInvocationTokens(parameters, function, isStatic, returnType, kOraclePrefix);
    const std::string mirrorInvocation =
        mirrorTokens.first + "(" + mirrorTokens.second + ")";

    if (oracleMode == OracleMode::Explicit && supportsExplicitOracle(returnType)) {
        const std::string expectedKey =
            function + "_" + std::to_string(invocation) + ".expected";
        std::string readExpected;
        if (isCStringLike(returnType)) {
            readExpected = "Read_string(\"" + expectedKey + "\")";
        } else {
            readExpected = "Read_" +
                           normalizeReadMethodType(returnType.getUnderlyingType()) +
                           "(\"" + expectedKey + "\")";
        }
        return "([&]() { const std::string key = \"" + expectedKey +
               "\"; if (HasObject(key)) return " + readExpected + "; return " +
               mirrorInvocation + "; }())";
    }
    // This is a mirror execution of the same SUT, not an independent oracle.
    return mirrorInvocation;
}

std::string Generator::normalizeReadMethodType(const InfoType &type) const {
    if (type.isMap()) {
        const std::string &formatted = type.formatted;
        size_t left = formatted.find('<');
        size_t right = formatted.find_last_of('>');
        if (left == std::string::npos || right == std::string::npos || right <= left) {
            return type.formatted;
        }

        std::vector<std::string> args;
        std::string current;
        int depth = 0;
        for (size_t i = left + 1; i < right; ++i) {
            char c = formatted[i];
            if (c == '<') {
                depth++;
                current.push_back(c);
            } else if (c == '>') {
                depth--;
                current.push_back(c);
            } else if (c == ',' && depth == 0) {
                ltrim(current);
                rtrim(current);
                args.push_back(current);
                current.clear();
            } else {
                current.push_back(c);
            }
        }
        ltrim(current);
        rtrim(current);
        if (!current.empty()) {
            args.push_back(current);
        }

        if (args.size() < 2) {
            return type.formatted;
        }

        return "map<" + args[0] + ", " + args[1] + ">";
    }

    if (type.isOptional() || type.isPair() || type.isTuple()) {
        return buildStructuredReadSuffix(type);
    }
    if (containsSubstring(type.original, "string")) {
        return "string";
    }
    return type.formatted;
}

std::string Generator::buildInvocation(const std::string &function, bool isStatic,
                                       bool returnsPointer,
                                       const std::string &instancePrefix) const {
    std::string invocation = returnsPointer ? "*" : "";
    if (isStatic)
        invocation += targetQualifiedName + "::";
    else if (isFromClass)
        invocation += instancePrefix + generateTestObjectForTarget(targetName) + ".";
    invocation += function;
    return invocation;
}

bool Generator::setInstanceConstruction(
    const std::vector<InfoVariable> &constructorParams, bool useDefaultConstructor) {
    if (!isFromClass) {
        constructorParams_.clear();
        useDefaultConstructor_ = true;
        return true;
    }

    constructorParams_.clear();
    constructorParams_.reserve(constructorParams.size());
    for (const auto &param : constructorParams) {
        constructorParams_.emplace_back(param.name, param);
    }
    useDefaultConstructor_ = useDefaultConstructor;
    return true;
}

void Generator::setOracleMode(OracleMode mode) {
    oracleMode = effectiveOracleMode(mode);
}

std::string Generator::buildInstanceInitialization(const std::string &function,
                                                   unsigned invocation, bool isStatic,
                                                   const std::string &variablePrefix,
                                                   const std::string &keyPrefix,
                                                   const std::string &instancePrefix) const {
    if (isStatic || !isFromClass) {
        return "";
    }

    std::ostringstream ss;
    if (!useDefaultConstructor_ && !constructorParams_.empty()) {
        ss << generateParameterInitialization(constructorParams_, function, invocation,
                                             variablePrefix, keyPrefix)
           << "\n";
    }

    ss << "\t" << targetQualifiedName << " "
       << instancePrefix + generateTestObjectForTarget(targetName) << "{";
    if (!useDefaultConstructor_) {
        for (size_t i = 0; i < constructorParams_.size(); ++i) {
            ss << variablePrefix << constructorParams_[i].name;
            if (i + 1 < constructorParams_.size()) {
                ss << ", ";
            }
        }
    }
    ss << "};";

    return ss.str();
}

std::string Generator::generatePointersAssertsWithTemplate(
    const std::vector<InfoVariable> &parameters, const std::string &paramToken,
    const std::string &expectedToken, const std::string &assertTemplate) const {
    std::stringstream ss;
    for (const auto &param : parameters) {
        if ((param.isPointer() || param.isReference()) && param.isMutableOutput()) {
            auto pointers = param.getPointersVarName();
            if (isCStringLike(param)) {
                continue;
            }
            std::map<std::string, std::string> tokens = {
                {paramToken, pointers.first},
                {expectedToken, std::string(kOraclePrefix) + pointers.first},
            };

            std::string pointerAssert = assertTemplate;
            replaceTokensInText(pointerAssert, tokens);
            ss << "\n\t" << pointerAssert;
        }
    }

    return ss.str();
}

int Generator::getFunctionCounter(const std::string &function) const {
    auto it = functionCounter.find(function);
    if (it != functionCounter.end()) {
        return it->second;
    } else {
        return 0;
    }
}

void Generator::incrementFunctionCounter(const std::string &function) {
    functionCounter[function]++;
}

void Generator::generateConfigFileTestCase(const std::string &functionName,
                                           const std::vector<InfoVariable> &params,
                                           const InfoType &returnType,
                                           unsigned invocation) const {
    if (constructorParams_.empty()) {
        configGenerator.generateTestCase(functionName, params, returnType, invocation);
        return;
    }

    configGenerator.generateTestCaseWithSetup(functionName, params, constructorParams_,
                                              kCtorPrefix, returnType, invocation);
}

void Generator::generateConfigFileTestCase(const std::string &functionName,
                                           const std::vector<InfoVariable> &params,
                                           unsigned invocation) const {
    configGenerator.generateConstructorTest(functionName, params, invocation);
}

Generator::~Generator() {
    std::map<string, string> valuesToDelete = {
        {templateItems["tplitem"]["read_object"], ""},
        {templateItems["tplitem"]["overload_operator"], ""}};

    std::ostringstream supportedTypesContent;
    for (const auto &type : supportedTypes)
        supportedTypesContent << type << "\n";
    writeToFile(supportedPath, supportedTypesContent.str());

    replaceTokensInFile(fixturePath, fixturePath, valuesToDelete);
}

void Generator::setRuleValues(
    const std::map<std::string, std::map<std::string, std::vector<long long>>> &rules) {
    configGenerator.setRuleValues(rules);
}

void Generator::setStringRuleValues(
    const std::map<std::string, std::map<std::string, std::vector<std::string>>>
        &rules) {
    configGenerator.setStringRuleValues(rules);
}

std::string Generator::getMethodTemplatePath(const std::string &methodTemplate) {
    const static fs::path methodsTplPath =
        getAskeletonHome() / config["route"]["templates"];
    return methodsTplPath / methodTemplate;
}
