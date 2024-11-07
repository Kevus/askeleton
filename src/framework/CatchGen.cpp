#include "framework/CatchGen.hpp"

#include "utils/system.hpp"
#include "utils/templating.hpp"

using namespace std;
namespace fs = std::filesystem;

CatchGenerator::CatchGenerator(const string &targetName, const string &filePath,
                               bool isFromClass)
    : Generator(targetName, filePath, isFromClass) {
    setFrameworkTemplatePath(getAskeletonHome() /
                             config.get("route.catch_templates"));

    map<string, string> tokensToReplace;
    setValuesToChange(tokensToReplace);
    setOutputFiles(tokensToReplace);

    copyMainFile();
}

void CatchGenerator::generateFunctionAssert(
    const string &function, const vector<InfoVariable> &parameters,
    const InfoType &returnType) {
    const string init = generateParameterInitialization(parameters, function);
    const string paramInvocation = generateParameterInvocation(parameters);
    const string pointers = generatePointersAsserts(parameters, function);
    const string functionInvocation =
        to_string(getFunctionCounter(function) + 1);
    const string returnContent =
        (returnType.isPointer() ? "*" : "") +
        generateReturnTypeInvocation(returnType, function);

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["catch"]["target"], targetName},
        {templateItems["tplitem"]["catch"]["function"], function},
        {templateItems["tplitem"]["catch"]["number"], functionInvocation},
        {templateItems["tplitem"]["catch"]["initializations"], init},
        {templateItems["tplitem"]["catch"]["invocation"], function},
        {templateItems["tplitem"]["catch"]["parameters"], paramInvocation},
        {templateItems["tplitem"]["catch"]["return"], returnContent},
        {templateItems["tplitem"]["catch"]["pointers"], pointers}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.function"),
        tokensToReplace);
    appendTestCaseToTestFile(testContent);
}

void CatchGenerator::generateMethodAssert(
    const string &method, const vector<InfoVariable> &parameters,
    const InfoType &returnType) {
    const string functionInvocation = to_string(getFunctionCounter(method) + 1);
    const string returnContent =
        (returnType.isPointer() ? "*" : "") +
        generateReturnTypeInvocation(returnType, method);
    const string paramInvocation = generateParameterInvocation(parameters);
    const string paramInit =
        generateParameterInitialization(parameters, method);
    const string pointers = generatePointersAsserts(parameters, method);
    string objectTest = targetName + "_test";
    objectTest[0] = tolower(objectTest[0]);

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["catch"]["target"], targetName},
        {templateItems["tplitem"]["catch"]["function"], method},
        {templateItems["tplitem"]["catch"]["number"], functionInvocation},

        {templateItems["tplitem"]["catch"]["class"], targetName},
        {templateItems["tplitem"]["catch"]["object"], objectTest},

        {templateItems["tplitem"]["catch"]["initializations"], paramInit},

        {templateItems["tplitem"]["catch"]["invocation"], method},
        {templateItems["tplitem"]["catch"]["parameters"], paramInvocation},
        {templateItems["tplitem"]["catch"]["return"], returnContent},

        {templateItems["tplitem"]["catch"]["pointers"], pointers}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.method"),
        tokensToReplace);
    appendTestCaseToTestFile(testContent);
    incrementFunctionCounter(method);
}

void CatchGenerator::generateConstructorAssert(
    const vector<InfoVariable> &parameters) {
    const string paramInvocation = generateParameterInvocation(parameters);
    const string functionInvocation =
        to_string(getFunctionCounter(targetName) + 1);
    const string paramInit =
        generateParameterInitialization(parameters, targetName);
    const string pointers = generatePointersAsserts(parameters, targetName);

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["catch"]["target"], targetName},
        {templateItems["tplitem"]["catch"]["function"], targetName},
        {templateItems["tplitem"]["catch"]["number"], functionInvocation},

        {templateItems["tplitem"]["catch"]["initializations"], paramInit},

        {templateItems["tplitem"]["catch"]["class"], targetName},
        {templateItems["tplitem"]["catch"]["parameters"], paramInvocation},

        {templateItems["tplitem"]["catch"]["pointers"], pointers}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.constructor"),
        tokensToReplace);
    appendTestCaseToTestFile(testContent);
    incrementFunctionCounter(targetName);
}

string
CatchGenerator::generatePointersAsserts(const vector<InfoVariable> &parameters,
                                        const string &function) const {
    const string TPLITEM_CATCH_PARAMETER =
                     templateItems["tplitem"]["catch"]["parameter"],
                 TPLITEM_CATCH_EXPECTED =
                     templateItems["tplitem"]["catch"]["expected"],
                 TPLITEM_CATCH_POINTER =
                     templateItems["templating"]["catch"]["assert_pointer"];

    stringstream ss;
    for (const auto &param : parameters) {
        if (param.isPointer() || param.isReference()) {
            InfoType underlyingType = param.getUnderlyingType();
            InfoVariable underlyingVar{param.name + "_output",
                                       underlyingType.original,
                                       underlyingType.formatted};

            map<string, string> tokensToReplace = {
                {TPLITEM_CATCH_PARAMETER, param.name},
                {TPLITEM_CATCH_EXPECTED,
                 generateReadInvocation(underlyingVar, function)}};

            string pointerAssert = TPLITEM_CATCH_POINTER;
            replaceTokensInText(pointerAssert, tokensToReplace);
            ss << "\n\t" << pointerAssert;
            if (&param == &parameters.back()) {
                ss << "\n";
            }
        }
    }
    return ss.str();
}

void CatchGenerator::copyMainFile() const {
    fs::path tplMain =
                 templateFrameworkPath / config.get("file.template.catch_main"),
             outputMain = utPath / config.get("file.output.catch_main");
    fs::copy_file(tplMain, outputMain, fs::copy_options::overwrite_existing);
}