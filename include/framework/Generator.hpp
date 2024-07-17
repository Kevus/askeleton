#pragma once

#include "VariableInfo.hpp"
#include <map>
#include <set>
#include <string>
#include <vector>

class Generator {
public:
    Generator() = delete;

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

    virtual ~Generator();

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

    std::string
    generateParameterInitialization(const std::vector<InfoVariable> &parameters,
                                    const std::string &function) const;
    std::string
    generateParameterInitialization(const InfoType &type,
                                    const std::string &function) const;
    std::string
    generateParameterInitialization(const InfoVariable &variable,
                                    const std::string &function) const;

    std::string generateReadInvocation(const InfoVariable &type,
                                       const std::string &function) const;
    std::string generateReturnTypeInvocation(const InfoType &type,
                                             const std::string &function) const;

    std::string
    generateParameterInvocation(const std::vector<InfoVariable> &) const;

    virtual std::string
    generatePointersAsserts(const std::vector<InfoVariable> &parameters,
                            const std::string &function) const = 0;

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

    const static std::string ASSIGN_INSTRUCTION_TEMPLATE,
        READ_INSTRUCTION_TEMPLATE;
    const static std::string FIELD_ASSIGN_TPL, FIELD_COMPARISON_TPL,
        FIELD_INSERTION_TPL;
};
