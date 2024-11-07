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
    : targetName(targetName), targetFilePath(filePath),
      targetFileName(extractFileName(filePath)), isFromClass(isFromClass),
      utPath(getAskeletonHome() / config.get("route.ut") / targetName) {

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
        {templateItems["tplitem"]["file_path"], targetFilePath},
        {templateItems["tplitem"]["target"], targetName},
        {templateItems["tplitem"]["file_name"], targetFileName},

        {templateItems["tplitem"]["cpp_path"],
         sourceFile.value_or(targetFilePath)},
        {templateItems["tplitem"]["header_path"],
         headerFile.value_or(targetFilePath)},

        {templateItems["tplitem"]["date_of_generation"], getTodayString()},
        {templateItems["tplitem"]["includes"], ""},
        {templateItems["tplitem"]["namespaces"], ""},
        {templateItems["tplitem"]["new_methods"], ""},
    };

    if (isFromClass) {
        valuesToChange.insert(
            {templateItems["tplitem"]["class_name"], targetName});
        valuesToChange.insert({templateItems["tplitem"]["class_name_test"],
                               targetName + "_test;"});
    } else {
        valuesToChange.insert({templateItems["tplitem"]["class_name"], ""});
        valuesToChange.insert(
            {templateItems["tplitem"]["class_name_test"], ""});
    }
}

void Generator::setSupportedTypes() {
    fs::path supportedTypesPath =
        getAskeletonHome() / config.get("file.data.supported_types_json");
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

    createFileFromTemplate(templatePath / config.get("file.template.test_tpl"),
                           testPath, tokensToReplace);
    createFileFromTemplate(templatePath /
                               config.get("file.template.fixture_tpl"),
                           fixturePath, tokensToReplace);
    createFileFromTemplate(templatePath /
                               config.get("file.template.makefile_tpl"),
                           makefilePath, tokensToReplace);
}

void Generator::appendReadMethodToFixture(const std::string &method) const {
    replaceTokensInFile(
        fixturePath, fixturePath,
        {{templateItems["tplitem"]["read_object"],
          "\n\n" + method +
              templateItems["tplitem"]["read_object"].get<string>()}});
}

void Generator::appendOverloadMethodsToFixture(const std::string &op) const {
    replaceTokensInFile(
        fixturePath, fixturePath,
        {{templateItems["tplitem"]["overload_operator"],
          "\n\n" + op +
              templateItems["tplitem"]["overload_operator"].get<string>()}});
}

void Generator::createPointerReadToFixture(const InfoType &type) const {
    InfoType underlying = type.getUnderlyingType();
    string method = readFromFile(
        getMethodTemplatePath(config.get("file.method.pointer_read")));
    replaceTokensInText(
        method, {{templateItems["tplitem"]["type"], type.original},
                 {templateItems["tplitem"]["formatted"], type.formatted},
                 {templateItems["tplitem"]["underlying"], underlying.original},
                 {templateItems["tplitem"]["underlying_formatted"],
                  underlying.formatted}});

    appendReadMethodToFixture(method);
}

void Generator::createEnumReadToFixture(const InfoType &type) const {
    string method = readFromFile(
        getMethodTemplatePath(config.get("file.template.method.enum_read")));
    replaceTokensInText(
        method, {{templateItems["tplitem"]["type"], type.original},
                 {templateItems["tplitem"]["formatted"], type.formatted}});
    appendReadMethodToFixture(method);
}

void Generator::createRecordReadToFixture(const InfoType &type) const {
    stringstream assigns;
    string method = readFromFile(
        getMethodTemplatePath(config.get("file.template.method.record_read")));

    for (const auto &field : type.getRecordFields()) {
        string assignField = templateItems["templating"]["field_assign"];
        replaceTokensInText(
            assignField,
            {{templateItems["tplitem"]["field"], field.name},
             {templateItems["tplitem"]["formatted"], field.formatted}});
        assigns << "\n\t\t" << assignField;
    }

    replaceTokensInText(
        method, {{templateItems["tplitem"]["type"], type.original},
                 {templateItems["tplitem"]["formatted"], type.formatted},
                 {templateItems["tplitem"]["fields"], assigns.str()}});

    appendReadMethodToFixture(method);
}

void Generator::createRecordOverloadToFixture(const InfoType &type) const {
    const static string comparison =
                            templateItems["templating"]["field_comparison"],
                        insertion =
                            templateItems["templating"]["field_insertion"],
                        tplitemField = templateItems["tplitem"]["field"],
                        tplitemFieldFormatted =
                            templateItems["tplitem"]["field_formatted"];

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

    string method = readFromFile(getMethodTemplatePath(
        config.get("file.template.method.record_overload")));
    replaceTokensInText(
        method, {{templateItems["tplitem"]["type"], type.original},
                 {templateItems["tplitem"]["formatted"], type.formatted},
                 {templateItems["tplitem"]["comparisons"], comparisons.str()},
                 {templateItems["tplitem"]["insertions"], insertions.str()}});
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
        templateItems["templating"]["assign_instruction"];
    InfoType underlying = variable.getUnderlyingType();
    const string variableName =
        variable.name + (variable.isPointer() ? "_input" : "");
    std::string typeForReadMethod = underlying.formatted;
    replaceTypeCharacters(typeForReadMethod);

    std::map<std::string, std::string> replacements = {
        {templateItems["tplitem"]["underlying"], underlying.original},
        {templateItems["tplitem"]["name"], variable.name},
        {templateItems["tplitem"]["target"], function},
        {templateItems["tplitem"]["formatted"], variableName},
        {templateItems["tplitem"]["underlying_formatted"], typeForReadMethod}};

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
    string readInvocation = templateItems["templating"]["read_instruction"];
    replaceTokensInText(
        readInvocation,
        {{templateItems["tplitem"]["underlying_formatted"], type.formatted},
         {templateItems["tplitem"]["target"], function},
         {templateItems["tplitem"]["name"], type.name}});

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