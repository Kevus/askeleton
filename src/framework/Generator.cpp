#include "framework/Generator.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "constants.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "utils/templating.hpp"

using namespace askeleton;
using namespace std;
namespace fs = std::filesystem;

Generator::Generator(const string &targetName, const string &filePath,
                     bool isFromClass)
    : targetName(targetName), targetFilePath(filePath), templateFrameworkPath(),
      isFromClass(isFromClass), targetFileName(extractFileName(filePath)),
      utPath(getAskeletonHome() / config.get("route.ut") / targetName) {

    setOutputFilesPath();
    setSupportedTypes();
}

void Generator::appendTestCaseToTestFile(const std::string &testCase) const {
    appendToFile(testPath, "\n\n" + testCase);
}

void Generator::setOutputFilesPath() {
    const std::map<std::string, std::string> replacements = {
        {config.get("tplitem.target"), targetName}};

    string fixtureFileName = config.get("file.output.test_fixture");
    string makefileFileName = config.get("file.output.makefile");
    string testFileName = config.get("file.output.test_file");
    string supportedFileName = config.get("file.output.supported_types");

    replaceTokensInText(fixtureFileName, replacements);
    replaceTokensInText(makefileFileName, replacements);
    replaceTokensInText(testFileName, replacements);
    replaceTokensInText(supportedFileName, replacements);

    fixturePath = utPath / fixtureFileName;
    makefilePath = utPath / makefileFileName;
    supportedPath = utPath / supportedFileName;
    testPath = utPath / testFileName;
}

void Generator::setValuesToChange(
    std::map<std::string, std::string> &valuesToChange) const {
    auto sourceFile = getSourceFile(targetFilePath);
    auto headerFile = getHeaderFile(targetFilePath);

    if (!sourceFile)
        std::cerr << "WARNING: Source file was not found for " << targetFilePath
                  << "\n\tRemind to add it manually in the Makefile\n";
    if (!headerFile)
        std::cerr << "WARNING: Header file was not found for " << targetFilePath
                  << "\n\tRemind to add it manually in the fixture\n";

    valuesToChange = {
        {config.get("tplitem.file_path"), targetFilePath},
        {config.get("tplitem.target"), targetName},
        {config.get("tplitem.file_name"), targetFileName},

        {config.get("tplitem.cpp_path"), sourceFile.value_or(targetFilePath)},
        {config.get("tplitem.header_path"),
         headerFile.value_or(targetFilePath)},

        {config.get("tplitem.date_of_generation"), getTodayString()},
        {config.get("tplitem.includes"), ""},
        {config.get("tplitem.namespaces"), ""},
        {config.get("tplitem.new_methods"), ""},
    };

    if (isFromClass) {
        valuesToChange.insert({config.get("tplitem.class_name"), targetName});
        valuesToChange.insert(
            {config.get("tplitem.class_name_test"), targetName + "_test;"});
    } else {
        valuesToChange.insert({config.get("tplitem.class_name"), ""});
        valuesToChange.insert({config.get("tplitem.class_name_test"), ""});
    }
}

void Generator::setSupportedTypes() {
    fs::path supportedTypesPath =
        getAskeletonHome() / config.get("file.supported_types_json");
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

void Generator::setOutputFiles(
    const map<string, string> &tokensToReplace) const {
    const fs::path templatePath = this->templateFrameworkPath;

    replaceTokensInFile(templatePath / config.get("file.template.test_tpl"),
                        tokensToReplace);
    replaceTokensInFile(templatePath / config.get("file.template.fixture_tpl"),
                        tokensToReplace);
    replaceTokensInFile(templatePath / config.get("file.template.makefile_tpl"),
                        tokensToReplace);
}

void Generator::appendReadMethodToFixture(const std::string &method) const {
    replaceTokensInFile(
        fixturePath, fixturePath,
        {{config.get("tplitem.read_object"),
          "\n\n" + method + config.get("tplitem.read_object")}});
}

void Generator::appendOverloadMethodsToFixture(const std::string &op) const {
    replaceTokensInFile(
        fixturePath, fixturePath,
        {{config.get("tplitem.overload_operator"),
          "\n\n" + op + config.get("tplitem.overload_operator")}});
}

void Generator::createPointerReadToFixture(const InfoType &type) const {
    InfoType underlying = type.getUnderlyingType();
    string method =
        readFromFile(getMethodTemplatePath(files::READ_POINTER_METHOD));
    replaceTokensInText(
        method, {{tplitems::TYPE, type.original},
                 {tplitems::FORMATTED, type.formatted},
                 {tplitems::UNDERLYING, underlying.original},
                 {tplitems::UNDERLYING_FORMATTED, underlying.formatted}});

    appendReadMethodToFixture(method);
}

void Generator::createEnumReadToFixture(const InfoType &type) const {
    string method =
        readFromFile(getMethodTemplatePath(files::READ_ENUM_METHOD));
    replaceTokensInText(method, {{tplitems::TYPE, type.original},
                                 {tplitems::FORMATTED, type.formatted}});
    appendReadMethodToFixture(method);
}

void Generator::createRecordReadToFixture(const InfoType &type) const {
    stringstream assigns;
    string method =
        readFromFile(getMethodTemplatePath(files::READ_RECORD_METHOD));

    for (const auto &field : type.getRecordFields()) {
        string assignField = config.get("templating.field_assign");
        replaceTokensInText(assignField,
                            {{tplitems::FIELD, field.name},
                             {tplitems::FORMATTED, field.formatted}});
        assigns << "\n\t\t" << assignField;
    }

    replaceTokensInText(method, {{tplitems::TYPE, type.original},
                                 {tplitems::FORMATTED, type.formatted},
                                 {tplitems::FIELDS, assigns.str()}});

    appendReadMethodToFixture(method);
}

void Generator::createRecordOverloadToFixture(const InfoType &type) const {
    const static string comparison = config.get("templating.field_comparison"),
                        insertion = config.get("templating.field_insertion"),
                        tplitemField = config.get("tplitem.field"),
                        tplitemFieldFormatted =
                            config.get("tplitem.field_formatted");

    stringstream comparisons, insertions;
    unsigned n_fields = type.getRecordFields().size();
    if (n_fields == 0)
        comparisons << "\t\ttrue";

    for (const auto &field : type.getRecordFields()) {
        string comparisonField = comparison, insertionField = insertion;
        map<string, string> replacements = {
            {tplitemField, field.name},
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

    string method =
        readFromFile(getMethodTemplatePath(files::OVERLOAD_RECORD_METHOD));
    replaceTokensInText(method, {{tplitems::TYPE, type.original},
                                 {tplitems::FORMATTED, type.formatted},
                                 {tplitems::COMPARISONS, comparisons.str()},
                                 {tplitems::INSERTIONS, insertions.str()}});
    appendOverloadMethodsToFixture(method);
}

void Generator::createTypeReadToFixture(const InfoType &type, unsigned level) {
    if (level > 1 || isTypeSupported(type))
        return;

    if (type.isPointer()) {
        createTypeReadToFixture(type.getUnderlyingType());

    } else if (type.isReference()) {
        createTypeReadToFixture(type.getUnderlyingType());

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

std::string Generator::generateParameterInitialization(
    const std::vector<InfoVariable> &parameters,
    const std::string &function) const {

    std::stringstream ss;
    for (const auto &param : parameters) {
        ss << "\t" << generateParameterInitialization(param, function) << ";\n";
    }

    return ss.str();
}

std::string
Generator::generateParameterInitialization(const InfoVariable &variable,
                                           const std::string &function) const {

    std::string readInstructionContent =
        config.get("templating.assign_instruction");
    InfoType underlying = variable.getUnderlyingType();
    string variableName =
        variable.name + (underlying.isPointer() ? "_input" : "");
    InfoVariable variableForRead{variableName, underlying.original,
                                 underlying.formatted};
    std::string typeForReadMethod = underlying.formatted;
    replaceTypeCharacters(typeForReadMethod);

    generateReadInvocation(variable, function);

    std::map<std::string, std::string> replacements = {
        {"{underlying}", underlying.original},
        {"{name}", variable.name},
        {"{target}", function},
        {tplitems::UNDERLYING_FORMATTED, typeForReadMethod}};

    replaceTokensInText(readInstructionContent, replacements);

    return readInstructionContent;
}

std::string
Generator::generateParameterInitialization(const InfoType &type,
                                           const std::string &function) const {
    InfoVariable variable{type.original, type.formatted, function + "_return"};
    return generateParameterInitialization(variable, function);
}

std::string Generator::generateReadInvocation(const InfoVariable &type,
                                              const string &function) const {
    string readInvocation = config.get("templating.read_instruction");
    replaceTokensInText(readInvocation,
                        {{tplitems::UNDERLYING_FORMATTED, type.formatted},
                         {"{target}", function},
                         {"{name}", type.name}});

    return readInvocation;
}

std::string
Generator::generateReturnTypeInvocation(const InfoType &type,
                                        const string &function) const {
    InfoVariable variable{"return_" + type.formatted, type.original,
                          type.formatted};

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

Generator::~Generator() {
    std::map<string, string> valuesToDelete = {
        {tplitems::READ_OBJECT, ""}, {tplitems::OVERLOAD_OPERATOR, ""}};

    std::ostringstream supportedTypesContent;
    for (const auto &type : supportedTypes)
        supportedTypesContent << type << "\n";
    writeToFile(supportedPath, supportedTypesContent.str());

    replaceTokensInFile(fixturePath, fixturePath, valuesToDelete);
}

std::string
Generator::getMethodTemplatePath(const std::string &methodTemplate) {
    const static fs::path methodsTplPath =
        getAskeletonHome() / config.get("route.templates");
    return methodsTplPath / methodTemplate;
}

void Generator::setTemplateItems() { templateItems = loadTemplateItems(); }

unsigned Generator::MAX_DEPTH;
const Config &Generator::config = Config::getInstance();
nlohmann::json Generator::templateItems;