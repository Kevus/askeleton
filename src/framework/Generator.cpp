#include "framework/Generator.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

// #include "auxiliary_functions.hpp"
#include "constants.hpp"
#include "utils/strings.hpp"

using namespace askeleton;

using std::cerr;
using std::cout;
using std::string;
using std::stringstream;
using std::vector;

std::optional<std::string>
getFileWithExtensions(const std::string &filePath,
                      const std::vector<std::string> &extensions) {
    std::string basePath = filePath.substr(0, filePath.find_last_of("."));

    for (const auto &ext : extensions) {
        std::string fullPath = basePath + ext;
        if (fileExists(fullPath)) {
            return fullPath;
        }
    }
    return std::nullopt;
}

std::optional<string> getSourceFile(const string &filePath) {
    return getFileWithExtensions(filePath, {".cpp", ".c"});
}

std::optional<string> getHeaderFile(const string &filePath) {
    return getFileWithExtensions(filePath, {".hpp", ".h"});
}

Generator::Generator(const std::string &targetName, const std::string &filePath,
                     const std::string &framework, bool isFromClass)
    : targetName(targetName), filePath(filePath), templatePath(ASKELETON_HOME),
      isFromClass(isFromClass), fileName(extractFileName(filePath)),
      folderPath(createPath({routes::TEST_ROUTE, targetName})),
      fixturePath(createPath(
          {folderPath, config.get("file.output.test_fixture")}, true)),
      makefilePath(
          createPath({folderPath, config.get("file.output.makefile")}, true)),
      supportedPath(createPath(
          {folderPath, config.get("file.output.supported_types")}, true)),
      configPath(createPath({folderPath, config.get("file.output.cfg")}, true)),
      testPath(
          createPath({folderPath, config.get("file.output.test_file")}, true)) {
    generateTemplatePath();
    replaceTargetOnPaths();
    std::map<std::string, std::string> valuesToChange;
    initializeValuesToChange(valuesToChange);
    initializeSupportedTypes();

    createTestDirectory();

    generateTest();
    generateFixture(valuesToChange);
    generateMakefile(valuesToChange);
}

string Generator::readFromFile(const std::string &filePath) const {
    std::ifstream file(filePath);
    if (!file.is_open())
        exitWithError(errors::openFileError(filePath));

    return {std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
}

void Generator::writeToFile(const std::string &filePath,
                            const std::string &content) const {
    std::ofstream file(filePath);
    if (!file.is_open())
        exitWithError(errors::openFileError(filePath));

    file << content;
}

void Generator::replaceTokensInText(
    string &text,
    const std::map<std::string, std::string> &replacements) const {

    for (const auto &[key, value] : replacements)
        replaceAll(text, key, value);
}

void Generator::replaceTokensInFile(
    const std::string &templateFilePath, const std::string &outputFilePath,
    const std::map<std::string, std::string> &replacements) const {

    string fileContent = readFromFile(templateFilePath);
    replaceTokensInText(fileContent, replacements);
    writeToFile(outputFilePath, fileContent);
}

std::string Generator::replaceTokensInFile(
    const std::string &inputFilePath,
    const std::map<std::string, std::string> &replacements) const {

    string fileContent = readFromFile(inputFilePath);
    replaceTokensInText(fileContent, replacements);
    return fileContent;
}

void Generator::appendToFile(const std::string &filePath,
                             const std::string &content) const {
    std::ofstream file(filePath, std::ios_base::app);
    if (!file.is_open())
        exitWithError(errors::openFileError(filePath));

    file << content;
}

void Generator::appendTestCaseToTestFile(const std::string &testCase) const {
    appendToFile(testPath, "\n\n" + testCase);
}

void Generator::replaceTargetOnPaths() {
    std::map<std::string, std::string> replacements = {
        {tplitems::TARGET, targetName}};
    replaceTokensInText(supportedPath, replacements);
    replaceTokensInText(makefilePath, replacements);
    replaceTokensInText(testPath, replacements);
    replaceTokensInText(fixturePath, replacements);
}

void Generator::generateTemplatePath() {
    switch (FRAMEWORK) {
    case Framework::BOOST:
        templatePath += config.get("route.boost_templates");
        break;
    case Framework::CATCH:
        templatePath += config.get("route.catch_templates");
        break;
    case Framework::GTEST:
        templatePath += config.get("route.gtest_templates");
        break;
    }
}

void Generator::initializeValuesToChange(
    std::map<std::string, std::string> &valuesToChange) const {
    auto sourceFile = getSourceFile(filePath);
    auto headerFile = getHeaderFile(filePath);

    if (!sourceFile)
        std::cerr << "WARNING: Source file was not found for " << filePath
                  << "\n\tRemind to add it manually in the Makefile\n";
    if (!headerFile)
        std::cerr << "WARNING: Header file was not found for " << filePath
                  << "\n\tRemind to add it manually in the fixture\n";

    valuesToChange = {
        {tplitems::FILE_PATH, filePath},
        {tplitems::TARGET, targetName},
        {tplitems::FILE_NAME, fileName},

        {tplitems::CPP_PATH, sourceFile.value_or(filePath)},
        {tplitems::HEADER_PATH, headerFile.value_or(filePath)},

        {tplitems::DATE_OF_GENERATION, getTodayString()},
        {tplitems::INCLUDES, ""},
        {tplitems::NAMESPACES, ""},
        {tplitems::NEW_METHODS, ""},
    };

    if (isFromClass) {
        valuesToChange.insert({tplitems::CLASS_NAME, targetName});
        valuesToChange.insert(
            {tplitems::CLASS_NAME_TEST, targetName + "_test;"});
    } else {
        valuesToChange.insert({tplitems::CLASS_NAME, ""});
        valuesToChange.insert({tplitems::CLASS_NAME_TEST, ""});
    }
}

void Generator::createTestDirectory() const {
    try {
        std::filesystem::create_directories(folderPath);
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "ERROR: directory " << folderPath
                  << " couldn't be created: " << e.what() << "\n";
    }
}

void Generator::initializeSupportedTypes() {
    std::string supportedTypesPath =
        ASKELETON_HOME + askeleton::files::SUPPORTED_TYPES_JSON;
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

void Generator::generateTest() const {
    replaceTokensInFile(templatePath + files::TEST_TPL, testPath,
                        {{tplitems::TARGET, targetName}});
}

void Generator::generateFixture(
    const std::map<std::string, std::string> &valuesToChange) const {
    replaceTokensInFile(templatePath + files::FIXTURE_TPL, fixturePath,
                        valuesToChange);
}

void Generator::generateMakefile(
    const std::map<std::string, std::string> &valuesToChange) const {
    replaceTokensInFile(templatePath + files::MAKEFILE_TPL, makefilePath,
                        valuesToChange);
}

void Generator::appendReadMethodToFixture(const std::string &method) const {
    replaceTokensInFile(
        fixturePath, fixturePath,
        {{tplitems::READ_OBJECT, "\n\n" + method + tplitems::READ_OBJECT}});
}

void Generator::appendOverloadMethodsToFixture(const std::string &op) const {
    replaceTokensInFile(fixturePath, fixturePath,
                        {{tplitems::OVERLOAD_OPERATOR,
                          "\n\n" + op + tplitems::OVERLOAD_OPERATOR}});
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
        string assignField = FIELD_ASSIGN_TPL;
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
    stringstream comparisons, insertions;
    string method = readFromFile(
               getMethodTemplatePath(files::OVERLOAD_RECORD_METHOD)),
           comparison = FIELD_COMPARISON_TPL, insertion = FIELD_INSERTION_TPL;

    unsigned n_fields = type.getRecordFields().size();
    if (n_fields == 0) {
        comparisons << "\t\ttrue";
    }

    for (const auto &field : type.getRecordFields()) {
        --n_fields;
        string comparisonField = comparison, insertionField = insertion;

        replaceTokensInText(comparisonField,
                            {{tplitems::FIELD, field.name},
                             {tplitems::FIELD_FORMATTED, field.formatted}});
        replaceTokensInText(insertionField,
                            {{tplitems::FIELD, field.name},
                             {tplitems::FIELD_FORMATTED, field.formatted}});

        if (n_fields > 0)
            comparisonField += " &&";

        comparisons << "\t\t" << comparisonField;
        insertions << "\t" << insertionField;

        if (n_fields > 0) {
            comparisons << "\n";
            insertions << "\n";
        }
    }

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
        createPointerReadToFixture(type);
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
    // writeToFile(supportedPath,
    //             readFromFile(supportedPath) + type.formatted + "\n");
}

bool Generator::isTypeSupported(const InfoType &type) const {
    return supportedTypes.find(type.original) != supportedTypes.end();
}

std::string Generator::getFrameworkTemplatePath(const std::string &tpl) const {
    return templatePath + tpl;
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

    std::string readInstructionContent = ASSIGN_INSTRUCTION_TEMPLATE;
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
    string readInvocation = READ_INSTRUCTION_TEMPLATE;
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
    return ASKELETON_HOME + routes::TEMPLATES_ROUTE + methodTemplate;
}

std::string Generator::ASKELETON_HOME;
Framework Generator::FRAMEWORK = BOOST;
unsigned Generator::MAX_DEPTH;
const Config &Generator::config = Config::getInstance();

const std::string Generator::ASSIGN_INSTRUCTION_TEMPLATE =
    "{underlying} {name} = Read_{underlyingFormatted}(\"{target}.{name}\")";
const std::string Generator::READ_INSTRUCTION_TEMPLATE =
    "Read_{underlyingFormatted}(\"{target}.{name}\")";
const std::string Generator::FIELD_ASSIGN_TPL =
    "result.{field} = Read_{formatted}(objectKey + \".{field}\");";
const std::string Generator::FIELD_COMPARISON_TPL = "a.{field} == b.{field}";
const std::string Generator::FIELD_INSERTION_TPL =
    "os << \"{field}:\" << object.{field} << \"\\n\";";