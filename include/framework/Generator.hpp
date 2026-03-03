#pragma once

#include "ConfigGenerator.hpp"
#include "VariableInfo.hpp"

#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

struct targetInfo {
    std::string name;
    std::string path;
    bool isClass;
};

struct outputFiles {
    std::string fixture;
    std::string makefile;
    std::string supported;
    std::string test;
};

class Generator {
public:
    Generator() = delete;
    Generator(const std::string &targetName, const std::string &targetQualifiedName,
              const std::string &filePath,
              bool isFromClass = false);

    bool isTypeSupported(const InfoType &type) const;
    void createTypeReadToFixture(const InfoType &type, unsigned level = 0);
    void markTypeAsSupported(const InfoType &type);

    virtual void generateFunctionAssert(const std::string &function,
                                        const std::vector<InfoVariable> &parameters,
                                        const InfoType &returnType) = 0;
    virtual void generateMethodAssert(const std::string &method,
                                      const std::vector<InfoVariable> &parameters,
                                      const InfoType &returnType,
                                      bool isStatic = false) = 0;
    virtual void
    generateConstructorAssert(const std::vector<InfoVariable> &parameters) = 0;

    virtual ~Generator();

    static unsigned MAX_DEPTH;

    void setRuleValues(const std::map<std::string,
                                      std::map<std::string, std::vector<long long>>> &rules);
    void setStringRuleValues(
        const std::map<std::string, std::map<std::string, std::vector<std::string>>>
            &rules);
    bool setInstanceConstruction(const std::vector<InfoVariable> &constructorParams,
                                 bool useDefaultConstructor);

protected:
    virtual void generateFullAssert(const std::string &function,
                                    const std::vector<InfoVariable> &parameters,
                                    const InfoType &returnType, bool isStatic) = 0;

    void setValuesToChange(std::map<std::string, std::string> &valuesToChange);
    void setOutputFiles(const std::map<std::string, std::string> &) const;
    void setFrameworkTemplatePath(const std::filesystem::path &frwPath);

    int getFunctionCounter(const std::string &function) const;
    void incrementFunctionCounter(const std::string &function);

    std::string
    generateParameterInitialization(const std::vector<InfoVariable> &parameters,
                                    const std::string &function,
                                    unsigned invocation = 1,
                                    const std::string &variablePrefix = "",
                                    const std::string &keyPrefix = "") const;
    std::string generateParameterInitialization(const InfoType &type,
                                                const std::string &function,
                                                unsigned invocation = 1,
                                                const std::string &variablePrefix = "",
                                                const std::string &keyPrefix = "") const;
    std::string generateParameterInitialization(const InfoVariable &variable,
                                                const std::string &function,
                                                unsigned invocation = 1,
                                                const std::string &variablePrefix = "",
                                                const std::string &keyPrefix = "") const;

    std::string generateReadInvocation(const InfoVariable &type,
                                       const std::string &function,
                                       unsigned invocation = 1) const;
    std::string generateReturnTypeInvocation(const InfoType &type,
                                             const std::string &function) const;

    std::string generateParameterInvocation(
        const std::vector<InfoVariable> &parameters,
        const std::string &variablePrefix = "") const;

    std::string buildInitializations(const std::vector<InfoVariable> &parameters,
                                     const std::string &function,
                                     unsigned invocation, bool isStatic) const;
    std::string buildExpectedInvocation(const std::vector<InfoVariable> &parameters,
                                        const std::string &function,
                                        bool isStatic,
                                        bool returnsPointer) const;

    std::string normalizeReadMethodType(const InfoType &type) const;

    std::string buildInvocation(const std::string &function, bool isStatic,
                                bool returnsPointer,
                                const std::string &instancePrefix = "") const;

    std::string generatePointersAssertsWithTemplate(
        const std::vector<InfoVariable> &parameters, const std::string &paramToken,
        const std::string &expectedToken, const std::string &assertTemplate) const;

    virtual std::string
    generatePointersAsserts(const std::vector<InfoVariable> &parameters,
                            const std::string &function) const = 0;

    void appendTestCaseToTestFile(const std::string &testCase) const;

    void generateConfigFileTestCase(const std::string &functionName,
                                    const std::vector<InfoVariable> &params,
                                    const InfoType &returnType,
                                    unsigned invocation) const;
    void generateConfigFileTestCase(const std::string &ctorName,
                                    const std::vector<InfoVariable> &params,
                                    unsigned invocation) const;

    const std::string targetName, targetFilePath, targetFileName;
    std::string targetQualifiedName;
    const bool isFromClass;

    std::filesystem::path templateFrameworkPath, templateMethodPath;
    std::filesystem::path utPath, fixturePath, makefilePath, supportedPath, testPath;
    static const nlohmann::json &config;
    static const nlohmann::json &templateItems;

private:
    void setOutputFilesPath();
    void setSupportedTypes();
    std::string buildInstanceInitialization(const std::string &function,
                                            unsigned invocation, bool isStatic,
                                            const std::string &variablePrefix,
                                            const std::string &keyPrefix,
                                            const std::string &instancePrefix) const;

    void createPointerReadToFixture(const InfoType &type) const;
    void createEnumReadToFixture(const InfoType &type) const;
    void createRecordReadToFixture(const InfoType &type) const;
    void createRecordOverloadToFixture(const InfoType &type) const;

    void appendReadMethodToFixture(const std::string &method) const;
    void appendOverloadMethodsToFixture(const std::string &op) const;

    static std::string getMethodTemplatePath(const std::string &methodTemplate = "");

    std::set<std::string> supportedTypes;
    std::map<std::string, unsigned> functionCounter;
    bool missingFilesWarn = false;
    std::vector<InfoVariable> constructorParams_;
    bool useDefaultConstructor_ = true;
    ConfigGenerator configGenerator;
};
