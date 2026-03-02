#include "framework/Generator.hpp"

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

using namespace askeleton;
using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {
std::string buildFactoryReadMethod(const InfoType &type, const std::string &expr) {
    std::ostringstream ss;
    ss << "    " << type.original << " Read_" << type.formatted
       << "(string objectKey) {\n";
    ss << "        (void)objectKey;\n";
    ss << "        return " << expr << ";\n";
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
} // namespace

unsigned Generator::MAX_DEPTH;
const json &Generator::config = getConfig();
const nlohmann::json &Generator::templateItems = getTemplateItems();

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

    string method = readFromFile(
        getMethodTemplatePath(config["file"]["template"]["method"]["record_overload"]));
    replaceTokensInText(method,
                        {{templateItems["tplitem"]["type"], type.original},
                         {templateItems["tplitem"]["formatted"], type.formatted},
                         {templateItems["tplitem"]["comparisons"], comparisons.str()},
                         {templateItems["tplitem"]["insertions"], insertions.str()}});
    appendOverloadMethodsToFixture(method);
}

void Generator::createTypeReadToFixture(const InfoType &type, unsigned level) {
    if (level > 1 || isTypeSupported(type))
        return;

    if (type.isRecord()) {
        const auto factory = TypeFactoryRegistry::get().find(type);
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
                                           unsigned invocation) const {
    std::stringstream ss;
    for (const auto &param : parameters) {
        pair<InfoVariable, InfoVariable> pointers = param.getPointers();
        ss << "\t"
           << generateParameterInitialization(pointers.first, function, invocation)
           << ";";
        if (param.isPointer() || param.isReference())
            ss << "\n\t"
               << generateParameterInitialization(pointers.second, function, invocation)
               << ";";
        if (&param != &parameters.back())
            ss << "\n";
    }

    return ss.str();
}

std::string Generator::generateParameterInitialization(const InfoVariable &variable,
                                                       const std::string &function,
                                                       unsigned invocation) const {

    std::string readInstructionContent =
        templateItems["templating"]["assign_instruction"];
    InfoType underlying = variable.getUnderlyingType();
    std::string typeForReadMethod = normalizeReadMethodType(underlying);

    std::map<std::string, std::string> replacements = {
        {templateItems["tplitem"]["underlying"], underlying.original},
        {templateItems["tplitem"]["name"], variable.name},
        {templateItems["tplitem"]["target"], function},
        {templateItems["tplitem"]["number"], to_string(invocation)},
        {templateItems["tplitem"]["formatted"], variable.name},
        {templateItems["tplitem"]["underlying_formatted"], typeForReadMethod}};

    replaceTokensInText(readInstructionContent, replacements);
    return readInstructionContent;
}

std::string Generator::generateParameterInitialization(const InfoType &type,
                                                       const std::string &function,
                                                       unsigned invocation) const {
    InfoVariable variable{type.original, type.formatted, function + "_return"};
    return generateParameterInitialization(variable, function, invocation);
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

std::string Generator::generateParameterInvocation(
    const std::vector<InfoVariable> &parameters) const {

    std::stringstream ss;
    for (const auto &param : parameters) {
        if (param.isPointer())
            ss << "&";
        ss << param.name;
        if (&param != &parameters.back())
            ss << ", ";
    }

    return ss.str();
}

std::string Generator::buildInitializations(
    const std::vector<InfoVariable> &parameters, const std::string &function,
    unsigned invocation, bool isStatic) const {
    std::string init = generateParameterInitialization(parameters, function, invocation);
    const std::string instanceInit = buildInstanceInitialization(isStatic);
    if (!instanceInit.empty()) {
        if (!init.empty()) {
            init = instanceInit + "\n" + init;
        } else {
            init = instanceInit;
        }
    }
    if (!init.empty())
        init = "\n" + init;
    return init;
}

std::string Generator::buildReturnReadMethod(const InfoType &underlying,
                                             const std::string &function,
                                             unsigned invocation) const {
    return generateReadInvocation(underlying.getTypeAsReturn(), function, invocation);
}

std::string Generator::normalizeReadMethodType(const InfoType &type) const {
    if (containsSubstring(type.original, "string")) {
        return "string";
    }
    if (!type.isMap()) {
        return type.formatted;
    }

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

std::string Generator::buildInvocation(const std::string &function, bool isStatic,
                                       bool returnsPointer) const {
    std::string invocation = returnsPointer ? "*" : "";
    if (isStatic)
        invocation += targetQualifiedName + "::";
    else if (isFromClass)
        invocation += generateTestObjectForTarget(targetName) + ".";
    invocation += function;
    return invocation;
}

bool Generator::setInstanceConstruction(
    const std::vector<InfoVariable> &constructorParams, bool useDefaultConstructor) {
    if (!isFromClass) {
        instanceConstruction_.clear();
        return true;
    }

    std::ostringstream ss;
    ss << targetQualifiedName << " " << generateTestObjectForTarget(targetName) << "{";

    if (!useDefaultConstructor) {
        for (size_t i = 0; i < constructorParams.size(); ++i) {
            const auto &param = constructorParams[i];
            if (param.isReference()) {
                return false;
            }
            ss << buildConstructorArgumentExpression(param);
            if (i + 1 < constructorParams.size()) {
                ss << ", ";
            }
        }
    }

    ss << "};";
    instanceConstruction_ = ss.str();
    return true;
}

std::string Generator::buildConstructorArgumentExpression(const InfoType &type) const {
    // TODO: This is only a compile-oriented fallback. Revisit it together with the
    // test-oracle work so instance construction uses semantically meaningful data,
    // not just "something that compiles".
    if (type.isPointer()) {
        return "nullptr";
    }
    if (containsSubstring(type.original, "string")) {
        return type.original + "{}";
    }
    if (type.isContainer() || type.isRecord()) {
        return type.original + "{}";
    }
    if (type.isEnum()) {
        return type.original + "{}";
    }

    const auto def = getDefaultValueForType(type);
    const std::string literal =
        def.has_value() ? def.value() : getZeroValueForType(type);
    return formatLiteralForType(literal, type);
}

std::string Generator::buildInstanceInitialization(bool isStatic) const {
    if (isStatic || !isFromClass) {
        return "";
    }
    return instanceConstruction_;
}

std::string Generator::generatePointersAssertsWithTemplate(
    const std::vector<InfoVariable> &parameters, const std::string &paramToken,
    const std::string &expectedToken, const std::string &assertTemplate) const {
    std::stringstream ss;
    for (const auto &param : parameters) {
        if (param.isPointer() || param.isReference()) {
            auto pointers = param.getPointersVarName();
            std::map<std::string, std::string> tokens = {
                {paramToken, pointers.first},
                {expectedToken, pointers.second},
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
    configGenerator.generateTestCase(functionName, params, returnType, invocation);
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
