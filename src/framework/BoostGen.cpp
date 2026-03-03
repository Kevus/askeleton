#include "framework/BoostGen.hpp"

#include "constants.hpp"
#include "utils/system.hpp"
#include "utils/templating.hpp"
#include <cctype>
#include <iostream>
#include <sstream>

using namespace askeleton;
using namespace std;
namespace fs = std::filesystem;

BoostGen::BoostGen(const std::string &targetName,
                   const std::string &targetQualifiedName,
                   const std::string &filePath, bool isFromClass)
    : Generator(targetName, targetQualifiedName, filePath, isFromClass) {

    setFrameworkTemplatePath(getAskeletonHome() /
                             config["route"]["boost_templates"].get<string>());

    map<string, string> tokensToReplace;
    setValuesToChange(tokensToReplace);
    setOutputFiles(tokensToReplace);
}

void BoostGen::generateFullAssert(const std::string &function,
                                  const std::vector<InfoVariable> &parameters,
                                  const InfoType &returnType, bool isStatic) {
    const static fs::path tplFunctionPath =
        templateFrameworkPath / config["file"]["template"]["case"]["function"];
    const unsigned number = getFunctionCounter(function) + 1;
    const string returnTypeOriginal = buildExpectedType(returnType);
    const string returnReadMethod =
        buildExpectedInvocation(parameters, function, isStatic, returnType);

    string pointers = generatePointersAsserts(parameters, function);
    if (!pointers.empty())
        pointers = pointers + "\n";

    string initializations =
        buildInitializations(parameters, function, number, isStatic);

    const string assert =
        generateAssertForFunction(function, parameters, returnType, isStatic);

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["boost"]["target"], targetName},
        {templateItems["tplitem"]["boost"]["function"], function},
        {templateItems["tplitem"]["boost"]["number"], to_string(number)},
        {templateItems["tplitem"]["boost"]["initializations"], initializations},
        {templateItems["tplitem"]["boost"]["return_type"], returnTypeOriginal},
        {templateItems["tplitem"]["boost"]["return_read_method"], returnReadMethod},
        {templateItems["tplitem"]["assert"], assert},
        {templateItems["tplitem"]["boost"]["pointers"], pointers}};

    string testContent = replaceTokensInFile(tplFunctionPath, tokensToReplace);

    appendTestCaseToTestFile(testContent);
    generateConfigFileTestCase(function, parameters, returnType, number);
    incrementFunctionCounter(function);
}

void BoostGen::generateMethodAssert(const std::string &method,
                                    const std::vector<InfoVariable> &parameters,
                                    const InfoType &returnType, bool isStatic) {
    generateFullAssert(method, parameters, returnType, isStatic);
}

void BoostGen::generateFunctionAssert(const std::string &function,
                                      const std::vector<InfoVariable> &parameters,
                                      const InfoType &returnType) {
    generateFullAssert(function, parameters, returnType, false);
}

void BoostGen::generateConstructorAssert(const std::vector<InfoVariable> &parameters) {}

std::string BoostGen::generatePointersAsserts(const std::vector<InfoVariable> &parameters,
                                              const std::string &function) const {
    const static string TPLITEM_BOOST_PARAMETER =
                            templateItems["tplitem"]["boost"]["parameter"],
                        TPLITEM_BOOST_EXPECTED =
                            templateItems["tplitem"]["boost"]["expected"],
                        TPLITEM_BOOST_ASSERT =
                            templateItems["templating"]["boost"]["assert_pointer"],
                        TPLITEM_BOOST_ASSERT_LIST =
                            templateItems["templating"]["boost"]["assert_pointer_list"],
                        TPLITEM_BOOST_ASSERT_MAP =
                            templateItems["templating"]["boost"]["assert_pointer_map"];

    std::stringstream ss;
    for (const auto &param : parameters) {
        if (!(param.isPointer() || param.isReference())) {
            continue;
        }

        if (param.isPointer() && param.getUnderlyingType().original == "char") {
            continue;
        }

        std::string assertTemplate;
        if (param.getUnderlyingType().isList()) {
            assertTemplate = TPLITEM_BOOST_ASSERT_LIST;
        } else if (param.getUnderlyingType().isMap()) {
            assertTemplate = TPLITEM_BOOST_ASSERT_MAP;
        } else {
            assertTemplate = TPLITEM_BOOST_ASSERT;
        }

        auto pointers = param.getPointersVarName();
        std::map<std::string, std::string> tokens = {
            {TPLITEM_BOOST_PARAMETER, pointers.first},
            {TPLITEM_BOOST_EXPECTED, std::string("oracle_") + pointers.first},
        };

        replaceTokensInText(assertTemplate, tokens);
        ss << "\n\t" << assertTemplate;
    }

    return ss.str();
}

string BoostGen::generateAssertForFunction(const string &function,
                                           const vector<InfoVariable> &params,
                                           const InfoType &returnType,
                                           bool isStatic) const {
    const static string TPLITEM_FUNC =
                            templateItems["templating"]["boost"]["assert_function"],
                        TPLITEM_FUNC_LIST =
                            templateItems["templating"]["boost"]["assert_function_list"],
                        TPLITEM_FUNC_MAP =
                            templateItems["templating"]["boost"]["assert_function_map"];
    const auto invocationTokens = buildInvocationTokens(params, function, isStatic, returnType);
    const string paramsList = invocationTokens.second;
    string invocation = invocationTokens.first;

    const map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["boost"]["invocation"], invocation},
        {templateItems["tplitem"]["boost"]["parameters"], paramsList}};

    string assertFunction;
    if (returnType.isList()) {
        assertFunction = TPLITEM_FUNC_LIST;
    } else if (returnType.isMap()) {
        assertFunction = TPLITEM_FUNC_MAP;
    } else {
        assertFunction = TPLITEM_FUNC;
    }

    replaceTokensInText(assertFunction, tokensToReplace);
    return assertFunction;
}
