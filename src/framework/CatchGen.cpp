#include "framework/CatchGen.hpp"

#include "utils/system.hpp"
#include "utils/templating.hpp"

using namespace std;
namespace fs = std::filesystem;

CatchGenerator::CatchGenerator(const string &targetName, const string &filePath,
                               bool isFromClass)
    : Generator(targetName, filePath, isFromClass) {
    setFrameworkTemplatePath(getAskeletonHome() / config["route"]["catch_templates"]);

    map<string, string> tokensToReplace;
    setValuesToChange(tokensToReplace);
    setOutputFiles(tokensToReplace);

    copyMainFile();
}

void CatchGenerator::generateFunctionAssert(const string &function,
                                            const vector<InfoVariable> &parameters,
                                            const InfoType &returnType) {
    const static fs::path tplFunctionPath =
        templateFrameworkPath / config["file"]["template"]["case"]["function"];
    InfoType underlying = returnType.getUnderlyingType();

    const string number = to_string(getFunctionCounter(function) + 1);
    const string parametersInvocation = generateParameterInvocation(parameters);
    const string returnTypeOriginal = underlying.original;
    const string returnReadMethod =
        generateReadInvocation(underlying.getTypeAsReturn(), function);
    const string invocation =
        (returnType.isPointer() ? "*" : "") +
        (isFromClass ? generateTestObjectForTarget(targetName) + "." : "") + function;

    string pointers = generatePointersAsserts(parameters, function);
    if (!pointers.empty())
        pointers = pointers + "\n";

    string initializations = generateParameterInitialization(parameters, function);
    if (!initializations.empty())
        initializations = "\n" + initializations;

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["catch"]["target"], targetName},
        {templateItems["tplitem"]["catch"]["function"], function},
        {templateItems["tplitem"]["catch"]["number"], number},
        {templateItems["tplitem"]["catch"]["initializations"], initializations},
        {templateItems["tplitem"]["catch"]["return_type"], returnTypeOriginal},
        {templateItems["tplitem"]["catch"]["return_read_method"], returnReadMethod},
        {templateItems["tplitem"]["catch"]["invocation"], invocation},
        {templateItems["tplitem"]["catch"]["parameters"], parametersInvocation},
        {templateItems["tplitem"]["catch"]["pointers"], pointers}};

    string testContent = replaceTokensInFile(tplFunctionPath, tokensToReplace);
    appendTestCaseToTestFile(testContent);

    incrementFunctionCounter(function);
}

void CatchGenerator::generateMethodAssert(const string &method,
                                          const vector<InfoVariable> &parameters,
                                          const InfoType &returnType) {
    generateFunctionAssert(method, parameters, returnType);
}

void CatchGenerator::generateConstructorAssert(const vector<InfoVariable> &parameters) {
    // Constructor test case generation is not supported in Catch2
}

string CatchGenerator::generatePointersAsserts(const vector<InfoVariable> &parameters,
                                               const string &function) const {
    const static string TPLITEM_CATCH_PARAMETER =
                            templateItems["tplitem"]["catch"]["parameter"].get<string>(),
                        TPLITEM_CATCH_EXPECTED =
                            templateItems["tplitem"]["catch"]["expected"].get<string>(),
                        TPLITEM_CATCH_POINTER =
                            templateItems["templating"]["catch"]["assert_pointer"]
                                .get<string>();

    stringstream ss;
    for (const auto &param : parameters) {
        if (param.isPointer() || param.isReference()) {
            pair<string, string> pointers = param.getPointersVarName();

            map<string, string> tokensToReplace = {
                {TPLITEM_CATCH_PARAMETER, pointers.first},
                {TPLITEM_CATCH_EXPECTED, pointers.second}};

            string pointerAssert = TPLITEM_CATCH_POINTER;
            replaceTokensInText(pointerAssert, tokensToReplace);
            ss << "\n\t" << pointerAssert;
        }
    }
    return ss.str();
}

void CatchGenerator::copyMainFile() const {
    fs::path tplMain = templateFrameworkPath / config["file"]["template"]["catch_main"],
             outputMain = utPath / config["file"]["output"]["catch_main"];
    fs::copy_file(tplMain, outputMain, fs::copy_options::overwrite_existing);
}