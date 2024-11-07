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
    const string functionInvocation = to_string(getFunctionCounter(function));
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
    string paramInvocation = generateParameterInvocation(parameters);
    string returnContent = (returnType.isPointer() ? "*" : "") +
                           generateReturnTypeInvocation(returnType, method);

    map<string, string> tokensToReplace = {{"{{target}}", targetName},
                                           {"{{method}}", method},
                                           {"{{parameters}}", paramInvocation},
                                           {"{{return}}", returnContent}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.method"),
        tokensToReplace);
    appendTestCaseToTestFile(testContent);
}

void CatchGenerator::generateConstructorAssert(
    const vector<InfoVariable> &parameters) {
    string paramInvocation = generateParameterInvocation(parameters);

    map<string, string> tokensToReplace = {{"{{target}}", targetName},
                                           {"{{parameters}}", paramInvocation}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.constructor"),
        tokensToReplace);
    appendTestCaseToTestFile(testContent);
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