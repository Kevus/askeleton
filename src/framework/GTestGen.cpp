#include "framework/GTestGen.hpp"

#include "utils/system.hpp"
#include "utils/templating.hpp"

using namespace std;
namespace fs = std::filesystem;

GTestGenerator::GTestGenerator(const string &targetName, const string &filePath,
                               bool isFromClass)
    : Generator(targetName, filePath, isFromClass) {
    setFrameworkTemplatePath(getAskeletonHome() /
                             config.get("route.gtest_templates"));

    map<string, string> tokensToReplace;
    setValuesToChange(tokensToReplace);
    setOutputFiles(tokensToReplace);

    copyMainFile();
}

void GTestGenerator::generateFunctionAssert(
    const string &function, const vector<InfoVariable> &parameters,
    const InfoType &returnType) {
    const string init = generateParameterInitialization(parameters, function);
    const string paramInvocation = generateParameterInvocation(parameters);
    const string pointers = generatePointersAsserts(parameters, function);
    const string functionInvocation = to_string(getFunctionCounter(function));
    const string returnContent =
        (returnType.isPointer() ? "*" : "") +
        generateReturnTypeInvocation(returnType, function);

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["gtest"]["target"], targetName},
        {templateItems["tplitem"]["gtest"]["function"], function},
        {templateItems["tplitem"]["gtest"]["number"], functionInvocation},
        {templateItems["tplitem"]["gtest"]["initializations"], init},
        {templateItems["tplitem"]["gtest"]["invocation"], function},
        {templateItems["tplitem"]["gtest"]["parameters"], paramInvocation},
        {templateItems["tplitem"]["gtest"]["return"], returnContent},
        {templateItems["tplitem"]["gtest"]["pointers"], pointers}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.function"),
        tokensToReplace);
    appendTestCaseToTestFile(testContent);
}

void GTestGenerator::generateMethodAssert(
    const std::string &method, const std::vector<InfoVariable> &parameters,
    const InfoType &returnType) {
    const string functionInvocation = to_string(getFunctionCounter(method));

    string returnContent = returnType.isPointer() ? "*" : "";
    returnContent += generateReturnTypeInvocation(returnType, method);

    string objectTest = targetName + "_test";
    objectTest[0] = tolower(objectTest[0]);

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["gtest"]["target"], targetName},
        {templateItems["tplitem"]["gtest"]["function"], method},
        {templateItems["tplitem"]["gtest"]["number"], functionInvocation},

        {templateItems["tplitem"]["gtest"]["class"], targetName},
        {templateItems["tplitem"]["gtest"]["object"], objectTest},

        {templateItems["tplitem"]["gtest"]["initializations"],
         generateParameterInitialization(parameters, method)},

        {templateItems["tplitem"]["gtest"]["assert_ending"],
         returnType.isContainer() ? "" : "_EQ"},
        {templateItems["tplitem"]["gtest"]["invocation"], method},
        {templateItems["tplitem"]["gtest"]["parameters"],
         generateParameterInvocation(parameters)},
        {templateItems["tplitem"]["gtest"]["return"], returnContent},

        {templateItems["tplitem"]["gtest"]["pointers"],
         generatePointersAsserts(parameters, method)}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.method"),
        tokensToReplace);

    appendTestCaseToTestFile(testContent);
    incrementFunctionCounter(method);
}

void GTestGenerator::generateConstructorAssert(
    const std::vector<InfoVariable> &parameters) {
    const string functionInvocation = to_string(getFunctionCounter(targetName));
    string invocationContent = targetName;

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["gtest"]["target"], targetName},
        {templateItems["tplitem"]["gtest"]["function"], targetName},
        {templateItems["tplitem"]["gtest"]["number"], functionInvocation},

        {templateItems["tplitem"]["gtest"]["initializations"],
         generateParameterInitialization(parameters, targetName)},

        {templateItems["tplitem"]["gtest"]["class"], targetName},
        {templateItems["tplitem"]["gtest"]["parameters"],
         generateParameterInvocation(parameters)},

        {templateItems["tplitem"]["gtest"]["pointers"],
         generatePointersAsserts(parameters, targetName)}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.constructor"),
        tokensToReplace);

    appendTestCaseToTestFile(testContent);
    incrementFunctionCounter(targetName);
}

std::string GTestGenerator::generatePointersAsserts(
    const std::vector<InfoVariable> &parameters,
    const std::string &function) const {
    const string TPLITEM_GTEST_PARAMETER =
                     templateItems["tplitem"]["gtest"]["parameter"],
                 TPLITEM_GTEST_EXPECTED =
                     templateItems["tplitem"]["gtest"]["expected"],
                 TPLITEM_GTEST_POINTER =
                     templateItems["templating"]["gtest"]["assert_pointer"];

    std::stringstream ss;
    for (const auto &param : parameters) {
        if (param.isPointer() || param.isReference()) {
            InfoType underlyingType = param.getUnderlyingType();
            InfoVariable underlyingVar{param.name + "_output",
                                       underlyingType.original,
                                       underlyingType.formatted};

            map<string, string> tokensToReplace = {
                {TPLITEM_GTEST_PARAMETER, param.name},
                {TPLITEM_GTEST_EXPECTED,
                 generateReadInvocation(underlyingVar, function)}};

            string pointerAssert = TPLITEM_GTEST_POINTER;
            replaceTokensInText(pointerAssert, tokensToReplace);
            ss << "\n\t" << pointerAssert;
            if (&param == &parameters.back()) {
                ss << "\n";
            }
        }
    }

    return ss.str();
}

void GTestGenerator::copyMainFile() const {
    fs::path tplMain =
                 templateFrameworkPath / config.get("file.template.gtest_main"),
             outputMain = utPath / config.get("file.output.gtest_main");
    fs::copy_file(tplMain, outputMain, fs::copy_options::overwrite_existing);
}