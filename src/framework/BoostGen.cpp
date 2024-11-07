#include "framework/BoostGen.hpp"

#include "constants.hpp"
#include "utils/system.hpp"
#include "utils/templating.hpp"
#include <cctype>
#include <iostream>

using namespace askeleton;
using namespace std;

BoostGen::BoostGen(const std::string &targetName, const std::string &filePath,
                   bool isFromClass)
    : Generator(targetName, filePath, isFromClass) {

    setFrameworkTemplatePath(getAskeletonHome() /
                             config.get("route.boost_templates"));

    map<string, string> tokensToReplace;
    setValuesToChange(tokensToReplace);
    setOutputFiles(tokensToReplace);
}

void BoostGen::generateFunctionAssert(
    const std::string &function, const std::vector<InfoVariable> &parameters,
    const InfoType &returnType) {

    const string init = generateParameterInitialization(parameters, function);
    const string assertEnding = returnType.isContainer() ? "" : "_EQUAL";
    const string paramInvocation = generateParameterInvocation(parameters);
    const string pointers = generatePointersAsserts(parameters, function);
    const string functionInvocation = to_string(getFunctionCounter(function));
    const string returnContent =
        (returnType.isPointer() ? "*" : "") +
        generateReturnTypeInvocation(returnType, function);

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["boost"]["target"], targetName},
        {templateItems["tplitem"]["boost"]["function"], function},
        {templateItems["tplitem"]["boost"]["number"], functionInvocation},
        {templateItems["tplitem"]["boost"]["initializations"], init},
        {templateItems["tplitem"]["boost"]["assert_ending"], assertEnding},
        {templateItems["tplitem"]["boost"]["invocation"], function},
        {templateItems["tplitem"]["boost"]["parameters"], paramInvocation},
        {templateItems["tplitem"]["boost"]["return"], returnContent},
        {templateItems["tplitem"]["boost"]["pointers"], pointers}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.function"),
        tokensToReplace);

    appendTestCaseToTestFile(testContent);
    incrementFunctionCounter(function);
}

void BoostGen::generateMethodAssert(const std::string &method,
                                    const std::vector<InfoVariable> &parameters,
                                    const InfoType &returnType) {
    const string functionInvocation = to_string(getFunctionCounter(method));

    string returnContent = returnType.isPointer() ? "*" : "";
    returnContent += generateReturnTypeInvocation(returnType, method);

    string objectTest = targetName + "_test";
    objectTest[0] = tolower(objectTest[0]);

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["boost"]["target"], targetName},
        {templateItems["tplitem"]["boost"]["function"], method},
        {templateItems["tplitem"]["boost"]["number"], functionInvocation},

        {templateItems["tplitem"]["boost"]["class"], targetName},
        {templateItems["tplitem"]["boost"]["object"], objectTest},

        {templateItems["tplitem"]["boost"]["initializations"],
         generateParameterInitialization(parameters, method)},

        {templateItems["tplitem"]["boost"]["assert_ending"],
         returnType.isContainer() ? "" : "_EQUAL"},
        {templateItems["tplitem"]["boost"]["invocation"], method},
        {templateItems["tplitem"]["boost"]["parameters"],
         generateParameterInvocation(parameters)},
        {templateItems["tplitem"]["boost"]["return"], returnContent},

        {templateItems["tplitem"]["boost"]["pointers"],
         generatePointersAsserts(parameters, method)}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.method"),
        tokensToReplace);

    appendTestCaseToTestFile(testContent);
    incrementFunctionCounter(method);
}

void BoostGen::generateConstructorAssert(
    const std::vector<InfoVariable> &parameters) {
    const string functionInvocation = to_string(getFunctionCounter(targetName));
    string invocationContent = targetName;

    map<string, string> tokensToReplace = {
        {templateItems["tplitem"]["boost"]["target"], targetName},
        {templateItems["tplitem"]["boost"]["function"], targetName},
        {templateItems["tplitem"]["boost"]["number"], functionInvocation},

        {templateItems["tplitem"]["boost"]["initializations"],
         generateParameterInitialization(parameters, targetName)},

        {templateItems["tplitem"]["boost"]["class"], targetName},
        {templateItems["tplitem"]["boost"]["parameters"],
         generateParameterInvocation(parameters)},

        {templateItems["tplitem"]["boost"]["pointers"],
         generatePointersAsserts(parameters, targetName)}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.constructor"),
        tokensToReplace);

    appendTestCaseToTestFile(testContent);
    incrementFunctionCounter(targetName);
}

std::string
BoostGen::generatePointersAsserts(const std::vector<InfoVariable> &parameters,
                                  const std::string &function) const {
    std::stringstream ss;
    const string TPLITEM_BOOST_PARAMETER =
                     templateItems["tplitem"]["boost"]["parameter"],
                 TPLITEM_BOOST_EXPECTED =
                     templateItems["tplitem"]["boost"]["expected"];

    for (const auto &param : parameters) {
        if (param.isPointer() || param.isReference()) {
            InfoType underlyingType = param.getUnderlyingType();
            InfoVariable underlyingVar{param.name + "_output",
                                       underlyingType.original,
                                       underlyingType.formatted};

            map<string, string> tokensToReplace = {
                {TPLITEM_BOOST_PARAMETER, param.name},
                {TPLITEM_BOOST_EXPECTED,
                 generateReadInvocation(underlyingVar, function)}};

            string pointerAssert =
                config.get("templating.boost.assert_pointer");
            replaceTokensInText(pointerAssert, tokensToReplace);
            ss << "\n\t" << pointerAssert;
            if (&param == &parameters.back()) {
                ss << "\n";
            }
        }
    }

    return ss.str();
}