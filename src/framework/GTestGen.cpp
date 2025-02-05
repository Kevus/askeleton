#include "framework/GTestGen.hpp"

#include "utils/system.hpp"
#include "utils/templating.hpp"

using namespace std;
namespace fs = std::filesystem;

GTestGenerator::GTestGenerator(const string &targetName, const string &filePath,
                               bool isFromClass)
    : Generator(targetName, filePath, isFromClass) {
    setFrameworkTemplatePath(getAskeletonHome() /
                             config["route"]["gtest_templates"]);

    map<string, string> tokensToReplace;
    setValuesToChange(tokensToReplace);
    setOutputFiles(tokensToReplace);

    copyMainFile();
}

void GTestGenerator::generateFunctionAssert(
    const string &function, const vector<InfoVariable> &parameters,
    const InfoType &returnType) {
    const static fs::path tplFunctionPath =
        templateFrameworkPath / config["file"]["template"]["case"]["function"];
    InfoType underlying = returnType.getUnderlyingType();
    const string number = to_string(getFunctionCounter(function) + 1);

    string init = generateParameterInitialization(parameters, function);
    if (!init.empty())
        init = "\n" + init;

    const string returnTypeOriginal = underlying.original;
    const string returnReadMethod =
        generateReadInvocation(underlying.getTypeAsReturn(), function);

    const string parametersInvocation = generateParameterInvocation(parameters);
    string pointers = generatePointersAsserts(parameters, function);
    if (!pointers.empty())
        pointers = pointers + "\n";

    string invocation =
        isFromClass ? generateTestObjectForTarget(targetName) + "." + function
                    : function;
    if (returnType.isPointer())
        invocation = "*" + invocation;

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["gtest"]["target"], targetName},
        {templateItems["tplitem"]["gtest"]["function"], function},
        {templateItems["tplitem"]["gtest"]["number"], number},
        {templateItems["tplitem"]["gtest"]["initializations"], init},
        {templateItems["tplitem"]["gtest"]["return_type"], returnTypeOriginal},
        {templateItems["tplitem"]["gtest"]["return_read_method"],
         returnReadMethod},
        {templateItems["tplitem"]["gtest"]["invocation"], invocation},
        {templateItems["tplitem"]["gtest"]["parameters"], parametersInvocation},
        {templateItems["tplitem"]["gtest"]["pointers"], pointers}};

    string testContent = replaceTokensInFile(tplFunctionPath, tokensToReplace);
    appendTestCaseToTestFile(testContent);
    incrementFunctionCounter(function);
}

void GTestGenerator::generateMethodAssert(
    const std::string &method, const std::vector<InfoVariable> &parameters,
    const InfoType &returnType) {
    generateFunctionAssert(method, parameters, returnType);
}

void GTestGenerator::generateConstructorAssert(
    const std::vector<InfoVariable> &parameters) {
    // Constructor test case generation is not supported in Google Test
}

std::string GTestGenerator::generatePointersAsserts(
    const std::vector<InfoVariable> &parameters,
    const std::string &function) const {
    const static string
        TPLITEM_GTEST_PARAMETER =
            templateItems["tplitem"]["gtest"]["parameter"],
        TPLITEM_GTEST_EXPECTED = templateItems["tplitem"]["gtest"]["expected"],
        TPLITEM_GTEST_POINTER =
            templateItems["templating"]["gtest"]["assert_pointer"];

    std::stringstream ss;
    for (const auto &param : parameters) {
        if (param.isPointer() || param.isReference()) {
            pair<string, string> pointers = param.getPointersVarName();
            map<string, string> tokensToReplace = {
                {TPLITEM_GTEST_PARAMETER, pointers.first},
                {TPLITEM_GTEST_EXPECTED, pointers.second}};

            string pointerAssert = TPLITEM_GTEST_POINTER;
            replaceTokensInText(pointerAssert, tokensToReplace);
            ss << "\n\t" << pointerAssert;
        }
    }

    return ss.str();
}

void GTestGenerator::copyMainFile() const {
    fs::path tplMain = templateFrameworkPath /
                       config["file"]["template"]["main"],
             outputMain = utPath / config["file"]["output"]["gtest_main"];
    fs::copy_file(tplMain, outputMain, fs::copy_options::overwrite_existing);
}