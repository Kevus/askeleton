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
    InfoType underlying = returnType.getUnderlyingType();
    const unsigned number = getFunctionCounter(function) + 1;

    string init = buildInitializations(parameters, function, number, isStatic);

    const string returnTypeOriginal = underlying.original;
    const string returnReadMethod = buildReturnReadMethod(underlying, function, number);

    const string parametersInvocation = generateParameterInvocation(parameters);
    string pointers = generatePointersAsserts(parameters, function);
    if (!pointers.empty())
        pointers = pointers + "\n";

    string invocation = buildInvocation(function, isStatic, returnType.isPointer());

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
    // Constructor test case generation is not supported in Google Test
}

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
