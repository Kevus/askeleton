#include "framework/CatchGen.hpp"

#include "utils/system.hpp"
#include "utils/templating.hpp"

using namespace std;
namespace fs = std::filesystem;

CatchGenerator::CatchGenerator(const string &targetName,
                               const string &targetQualifiedName,
                               const string &filePath, bool isFromClass)
    : Generator(targetName, targetQualifiedName, filePath, isFromClass) {
    setFrameworkTemplatePath(getAskeletonHome() / config["route"]["catch_templates"]);

    map<string, string> tokensToReplace;
    setValuesToChange(tokensToReplace);
    setOutputFiles(tokensToReplace);
    copyMainFile();
}

void CatchGenerator::generateFullAssert(const string &function,
                                        const vector<InfoVariable> &parameters,
                                        const InfoType &returnType, bool isStatic) {
    const static fs::path tplFunctionPath =
        templateFrameworkPath / config["file"]["template"]["case"]["function"];
    const unsigned number = getFunctionCounter(function) + 1;
    const auto invocationTokens =
        buildInvocationTokens(parameters, function, isStatic, returnType);
    const string parametersInvocation = invocationTokens.second;
    const string returnTypeOriginal = buildExpectedType(returnType);
    const string returnReadMethod =
        buildExpectedInvocation(parameters, function, number, isStatic, returnType);

    string invocation = invocationTokens.first;

    string pointers = generatePointersAsserts(parameters, function);
    if (!pointers.empty())
        pointers = pointers + "\n";

    string initializations =
        buildInitializations(parameters, function, number, isStatic);

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["catch"]["target"], targetName},
        {templateItems["tplitem"]["catch"]["function"], function},
        {templateItems["tplitem"]["catch"]["number"], to_string(number)},
        {templateItems["tplitem"]["catch"]["initializations"], initializations},
        {templateItems["tplitem"]["catch"]["return_type"], returnTypeOriginal},
        {templateItems["tplitem"]["catch"]["return_read_method"], returnReadMethod},
        {templateItems["tplitem"]["catch"]["invocation"], invocation},
        {templateItems["tplitem"]["catch"]["parameters"], parametersInvocation},
        {templateItems["tplitem"]["catch"]["pointers"], pointers}};

    string testContent = replaceTokensInFile(tplFunctionPath, tokensToReplace);

    appendTestCaseToTestFile(testContent);
    generateConfigFileTestCase(function, parameters, returnType, number);
    incrementFunctionCounter(function);
}

void CatchGenerator::generateMethodAssert(const string &method,
                                          const vector<InfoVariable> &parameters,
                                          const InfoType &returnType, bool isStatic) {
    generateFullAssert(method, parameters, returnType, isStatic);
}

void CatchGenerator::generateFunctionAssert(const string &function,
                                            const vector<InfoVariable> &parameters,
                                            const InfoType &returnType) {
    generateFullAssert(function, parameters, returnType, false);
}

void CatchGenerator::generateConstructorAssert(const vector<InfoVariable> &parameters) {
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
        {templateItems["tplitem"]["catch"]["target"], targetName},
        {templateItems["tplitem"]["catch"]["function"], ctorKey},
        {templateItems["tplitem"]["catch"]["number"], to_string(number)},
        {templateItems["tplitem"]["catch"]["initializations"], initializations},
        {templateItems["tplitem"]["catch"]["class"], targetQualifiedName},
        {templateItems["tplitem"]["catch"]["parameters"], parametersInvocation},
        {templateItems["tplitem"]["catch"]["pointers"], pointers},
    };

    string testContent = replaceTokensInFile(tplCtorPath, tokensToReplace);
    appendTestCaseToTestFile(testContent);
    generateConfigFileTestCase(ctorKey, parameters, number);
    incrementFunctionCounter(ctorKey);
}

bool CatchGenerator::supportsConstructorTests() const { return true; }

string CatchGenerator::generatePointersAsserts(const vector<InfoVariable> &parameters,
                                               const string &function) const {
    const static string TPLITEM_CATCH_PARAMETER =
                            templateItems["tplitem"]["catch"]["parameter"].get<string>(),
                        TPLITEM_CATCH_EXPECTED =
                            templateItems["tplitem"]["catch"]["expected"].get<string>(),
                        TPLITEM_CATCH_POINTER =
                            templateItems["templating"]["catch"]["assert_pointer"]
                                .get<string>();

    return generatePointersAssertsWithTemplate(
        parameters, TPLITEM_CATCH_PARAMETER, TPLITEM_CATCH_EXPECTED,
        TPLITEM_CATCH_POINTER);
}

void CatchGenerator::copyMainFile() const {
    fs::path tplMain = templateFrameworkPath / config["file"]["template"]["main"],
             outputMain = utPath / "main.cpp";
    fs::copy_file(tplMain, outputMain, fs::copy_options::overwrite_existing);
}
