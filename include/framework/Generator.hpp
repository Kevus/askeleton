#pragma once

#include "VariableInfo.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>

class Generator {
public:
    Generator() = delete;
    virtual ~Generator() = default;

    /**
     * @brief Construct a new Generator object
     *
     * @param targetName The unit software under test
     * @param filePath The SUT file path
     * @param templatePath Path of the directory where the templates are located
     * @param isFromClass Indicates if the target is a class
     *
     * @note This class is an abstract class and cannot be instantiated.
     */
    Generator(const std::string &targetName, const std::string &filePath,
              const std::string &framework, bool isFromClass = false);

    void createTypeReadToFixture(const InfoType &type, unsigned level = 0);
    void markTypeAsSupported(const InfoType &type);

    bool isTypeSupported(const InfoType &type) const;

    virtual void
    generateFunctionAssert(const std::string &function,
                           const std::vector<InfoVariable> &parameters,
                           const InfoType &returnType) = 0;
    virtual void
    generateMethodAssert(const std::string &method,
                         const std::vector<InfoVariable> &parameters,
                         const InfoType &returnType) = 0;
    virtual void
    generateConstructorAssert(const std::vector<InfoVariable> &parameters) = 0;

    static std::string ASKELETON_HOME;
    static unsigned MAX_DEPTH;

protected:
    std::string readFromFile(const std::string &filePath) const;
    void writeToFile(const std::string &filePath,
                     const std::string &content) const;
    void appendToFile(const std::string &filePath,
                      const std::string &content) const;

    void replaceTokensInText(
        std::string &text,
        const std::map<std::string, std::string> &replacements) const;
    void replaceTokensInFile(
        const std::string &inputFilePath, const std::string &outputFilePath,
        const std::map<std::string, std::string> &replacements) const;
    string replaceTokensInFile(
        const std::string &inputFilePath,
        const std::map<std::string, std::string> &replacements) const;

    std::string generateParamsInitilizations(
        const std::vector<InfoVariable> &parameters) const;
    std::string
    generateParamsInvocation(const std::vector<InfoVariable> &parameters) const;
    std::string generateReadType(const InfoType &type) const;
    std::string generateReadVariable(const InfoVariable &variable) const;
    virtual std::string generatePointersAsserts(
        const std::vector<InfoVariable> &parameters) const = 0;

    std::string getFrameworkTemplatePath(const std::string &tpl = "") const;

    void appendTestCaseToTestFile(const std::string &testCase) const;

    std::string targetName, filePath, templatePath;
    bool isFromClass;
    std::string fileName;
    std::string folderPath, fixturePath, makefilePath, supportedPath,
        configPath, testPath;
    std::set<std::string> supportedTypes;

private:
    void initializeValuesToChange(
        std::map<std::string, std::string> &valuesToChange) const;
    void createTestDirectory() const;

    void generateTest() const;
    void generateFixture(const std::map<std::string, std::string> &) const;
    void generateMakefile(const std::map<std::string, std::string> &) const;
    void generateSupported() const;

    void createPointerReadToFixture(const InfoType &type) const;
    void createEnumReadToFixture(const InfoType &type) const;
    void createRecordReadToFixture(const InfoType &type) const;
    void createRecordOverloadToFixture(const InfoType &type) const;

    void appendReadMethodToFixture(const std::string &method) const;
    void appendOverloadMethodsToFixture(const std::string &op) const;

    static std::string
    getMethodTemplatePath(const std::string &methodTemplate = "");
};
