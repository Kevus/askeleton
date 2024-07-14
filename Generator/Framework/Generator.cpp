#include "Generator.hpp"
#include "constants.hpp"
#include <filesystem>

using namespace askeleton;

string getTodayString() {
    auto t = time(nullptr);
    auto tm = *localtime(&t);
    ostringstream oss;
    oss << put_time(&tm, "%d-%m-%Y %H:%M:%S");
    return oss.str();
}

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
    : targetName(targetName), filePath(filePath), templatePath(templatePath),
      isFromClass(isFromClass), fileName(extractFileName(filePath)),
      folderPath(routes::TEST_ROUTE + targetName + "/"),
      fixturePath(folderPath + files::SUPPORTED_TYPES),
      makefilePath(folderPath + files::MAKEFILE),
      supportedPath(folderPath + files::SUPPORTED_TYPES),
      configPath(folderPath + targetName + files::CFG),
      testPath(folderPath + targetName + files::TEST_FILE) {

    std::map<std::string, std::string> valuesToChange;
    initializeValuesToChange(valuesToChange);

    createTestDirectory();

    generateTest();
    generateFixture(valuesToChange);
    generateMakefile(valuesToChange);
    generateSupported();
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
    writeToFile(fileContent, outputFilePath);
}

std::string Generator::replaceTokensInFile(
    const std::string &inputFilePath,
    const std::map<std::string, std::string> &replacements) const {

    string fileContent = readFromFile(inputFilePath);
    replaceTokensInText(fileContent, replacements);
    return fileContent;
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
        {tplitems::CFG_NAME, targetName},
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
        std::filesystem::create_directory(folderPath);
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "ERROR: directory " << folderPath
                  << " couldn't be created: " << e.what() << "\n";
    }
}

void Generator::generateTest() const {
    replaceTokensInFile(templatePath + files::TEMPLATE_BOOST, testPath,
                        {
                            {tplitems::CLASS_NAME, targetName},
                            {tplitems::POINTER_INIT_TOKEN, ""},
                            {tplitems::POINTER_DESTROY_TOKEN, ""},
                        });
}

void Generator::generateFixture(
    const std::map<std::string, std::string> &valuesToChange) const {
    replaceTokensInFile(templatePath + files::TEST_FIXTURE, fixturePath,
                        valuesToChange);
}

void Generator::generateMakefile(
    const std::map<std::string, std::string> &valuesToChange) const {
    replaceTokensInFile(templatePath + files::MAKEFILE, makefilePath,
                        valuesToChange);
}

void Generator::generateSupported() const {
    try {
        std::filesystem::copy_file(
            templatePath + files::SUPPORTED_TYPES, supportedPath,
            std::filesystem::copy_options::overwrite_existing);
    } catch (const std::filesystem::filesystem_error &e) {
        exitWithError("ERROR: file " + supportedPath +
                      " couldn't be copied: " + e.what());
    }
}

void Generator::appendReadMethodToFixture(const std::string &method) const {
    replaceTokensInFile(
        fixturePath, fixturePath,
        {{tplitems::READ_OBJECT, method + "\n\t" + tplitems::READ_OBJECT}});
}

void Generator::appendOverloadMethodsToFixture(const std::string &op) const {
    replaceTokensInFile(fixturePath, fixturePath,
                        {{tplitems::OVERLOAD_OPERATOR,
                          op + "\n" + tplitems::OVERLOAD_OPERATOR}});
}

void Generator::createPointerReadToFixture(const InfoType &type) const {
    InfoType underlying = type.getUnderlyingType();
    string method = readFromFile(getTemplatePath(files::READ_POINTER_METHOD));
    replaceTokensInText(
        method, {{tplitems::TYPE, type.original},
                 {tplitems::FORMATTED, type.formatted},
                 {tplitems::UNDERLYING, underlying.original},
                 {tplitems::UNDERLYING_FORMATTED, underlying.formatted}});

    appendReadMethodToFixture(method);
}

void Generator::createEnumReadToFixture(const InfoType &type) const {
    string method = readFromFile(getTemplatePath(files::READ_ENUM_METHOD));
    replaceTokensInText(method, {{tplitems::TYPE, type.original},
                                 {tplitems::FORMATTED, type.formatted}});
    appendReadMethodToFixture(method);
}

void Generator::createRecordReadToFixture(const InfoType &type) const {
    stringstream assigns;
    string method = readFromFile(getTemplatePath(files::READ_RECORD_METHOD)),
           assign =
               readFromFile(getTemplatePath(files::RECORD_FIELD_ASSIGNMENT));

    for (const auto &field : type.getRecordFields()) {
        InfoType fieldType = field.type;
        string assignField = assign;
        replaceTokensInText(assignField,
                            {{tplitems::FIELD, fieldType.original},
                             {tplitems::FIELD_FORMATTED, fieldType.formatted}});
        assigns << assignField;
    }

    replaceTokensInText(method, {{tplitems::TYPE, type.original},
                                 {tplitems::FORMATTED, type.formatted},
                                 {tplitems::FIELDS, assigns.str()}});

    appendReadMethodToFixture(method);
}

void Generator::createRecordOverloadToFixture(const InfoType &type) const {
    stringstream comparisons, insertions;
    string method =
               readFromFile(getTemplatePath(files::OVERLOAD_RECORD_METHOD)),
           comparison =
               readFromFile(getTemplatePath(files::RECORD_FIELD_COMPARISON)),
           insertion =
               readFromFile(getTemplatePath(files::RECORD_FIELD_INSERTION));

    for (const auto &field : type.getRecordFields()) {
        InfoType fieldType = field.type;
        string comparisonField = comparison, insertionField = insertion;
        replaceTokensInText(comparisonField,
                            {{tplitems::FIELD, fieldType.original},
                             {tplitems::FIELD_FORMATTED, fieldType.formatted}});
        replaceTokensInText(insertionField,
                            {{tplitems::FIELD, fieldType.original},
                             {tplitems::FIELD_FORMATTED, fieldType.formatted}});
        comparisons << comparisonField;
        insertions << insertionField;
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
    }

    markTypeAsSupported(type);
}

void Generator::markTypeAsSupported(const InfoType &type) {
    supportedTypes.insert(type.original);
    writeToFile(supportedPath,
                readFromFile(supportedPath) + type.formatted + "\n");
}

bool Generator::isTypeSupported(const InfoType &type) const {
    return supportedTypes.find(type.original) != supportedTypes.end();
}

std::string Generator::getTemplatePath(const std::string &methodTemplate) {
    return ASKELETON_HOME + routes::TEMPLATES_ROUTE + methodTemplate;
}