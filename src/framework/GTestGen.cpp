#include "framework/GTestGen.hpp"

#include "utils/system.hpp"
#include "utils/templating.hpp"

using namespace std;
namespace fs = std::filesystem;

GTestGenerator::GTestGenerator(const string &targetName,
                               const string &targetQualifiedName,
                               const string &filePath, bool isFromClass)
    : Generator(targetName, targetQualifiedName, filePath, isFromClass) {
    setFrameworkTemplatePath(getAskeletonHome() / config["route"]["gtest_templates"]);

    map<string, string> tokensToReplace;
    setValuesToChange(tokensToReplace);
    setOutputFiles(tokensToReplace);

    copyMainFile();
}

void GTestGenerator::generateFullAssert(const string &function,
                                        const vector<InfoVariable> &parameters,
                                        const InfoType &returnType, bool isStatic) {
    const static fs::path tplFunctionPath =
        templateFrameworkPath / config["file"]["template"]["case"]["function"];
    const unsigned number = getFunctionCounter(function) + 1;
    const auto invocationTokens =
        buildInvocationTokens(parameters, function, isStatic, returnType);

    string init = buildInitializations(parameters, function, number, isStatic);

    const string returnTypeOriginal = buildExpectedType(returnType);
    const string returnReadMethod =
        buildExpectedInvocation(parameters, function, number, isStatic, returnType);

    const string parametersInvocation = invocationTokens.second;
    string pointers = generatePointersAsserts(parameters, function);
    if (!pointers.empty())
        pointers = pointers + "\n";
    const string invocation = invocationTokens.first;

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["gtest"]["target"], targetName},
        {templateItems["tplitem"]["gtest"]["function"], function},
        {templateItems["tplitem"]["gtest"]["number"], to_string(number)},
        {templateItems["tplitem"]["gtest"]["initializations"], init},
        {templateItems["tplitem"]["gtest"]["return_type"], returnTypeOriginal},
        {templateItems["tplitem"]["gtest"]["return_read_method"], returnReadMethod},
        {templateItems["tplitem"]["gtest"]["invocation"], invocation},
        {templateItems["tplitem"]["gtest"]["parameters"], parametersInvocation},
        {templateItems["tplitem"]["gtest"]["pointers"], pointers}};

    string testContent = replaceTokensInFile(tplFunctionPath, tokensToReplace);
    appendTestCaseToTestFile(testContent);
    generateConfigFileTestCase(function, parameters, returnType, number);
    incrementFunctionCounter(function);
}

void GTestGenerator::generateFunctionAssert(const string &function,
                                            const vector<InfoVariable> &parameters,
                                            const InfoType &returnType) {
    generateFullAssert(function, parameters, returnType, false);
}

void GTestGenerator::generateMethodAssert(const std::string &method,
                                          const std::vector<InfoVariable> &parameters,
                                          const InfoType &returnType, bool isStatic) {
    generateFullAssert(method, parameters, returnType, isStatic);
}

void GTestGenerator::generateConstructorAssert(
    const std::vector<InfoVariable> &parameters) {
    const static fs::path tplCtorPath =
        templateFrameworkPath / config["file"]["template"]["case"]["constructor"];
    const unsigned number = getFunctionCounter(targetName) + 1;
    const std::string ctorKey = targetName;
    const std::string initializations =
        buildInitializations(parameters, ctorKey, number, true);
    const std::string parametersInvocation = generateParameterInvocation(parameters);

    std::string pointers = generatePointersAsserts(parameters, ctorKey);
    if (!pointers.empty())
        pointers = pointers + "\n";

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["gtest"]["target"], targetName},
        {templateItems["tplitem"]["gtest"]["function"], ctorKey},
        {templateItems["tplitem"]["gtest"]["number"], to_string(number)},
        {templateItems["tplitem"]["gtest"]["initializations"], initializations},
        {templateItems["tplitem"]["gtest"]["class"], targetQualifiedName},
        {templateItems["tplitem"]["gtest"]["parameters"], parametersInvocation},
        {templateItems["tplitem"]["gtest"]["pointers"], pointers},
    };

    string testContent = replaceTokensInFile(tplCtorPath, tokensToReplace);
    appendTestCaseToTestFile(testContent);
    generateConfigFileTestCase(ctorKey, parameters, number);
    incrementFunctionCounter(ctorKey);
}

bool GTestGenerator::supportsConstructorTests() const { return true; }

std::string
GTestGenerator::generatePointersAsserts(const std::vector<InfoVariable> &parameters,
                                        const std::string &function) const {
    const static string TPLITEM_GTEST_PARAMETER =
                            templateItems["tplitem"]["gtest"]["parameter"],
                        TPLITEM_GTEST_EXPECTED =
                            templateItems["tplitem"]["gtest"]["expected"],
                        TPLITEM_GTEST_POINTER =
                            templateItems["templating"]["gtest"]["assert_pointer"];
    return generatePointersAssertsWithTemplate(
        parameters, TPLITEM_GTEST_PARAMETER, TPLITEM_GTEST_EXPECTED,
        TPLITEM_GTEST_POINTER);
}

void GTestGenerator::copyMainFile() const {
    fs::path tplMain = templateFrameworkPath / config["file"]["template"]["main"],
             outputMain = utPath / config["file"]["output"]["gtest_main"];
    fs::copy_file(tplMain, outputMain, fs::copy_options::overwrite_existing);
}
