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
    : Generator(targetName, filePath, isFromClass), functionCounter() {

    setFrameworkTemplatePath(getAskeletonHome() /
                             config.get("route.boost_templates"));

    map<string, string> tokensToReplace;
    setValuesToChange(tokensToReplace);
    setOutputFiles(tokensToReplace);
}

void BoostGen::generateFunctionAssert(
    const std::string &function, const std::vector<InfoVariable> &parameters,
    const InfoType &returnType) {
    const unsigned functionInvocation = getFunctionCounter(function);

    string returnContent = returnType.isPointer() ? "*" : "";
    returnContent += generateReturnTypeInvocation(returnType, function);

    map<string, string> tokensToReplace = {
        {config.get("tplitem.boost.target"), targetName},
        {config.get("tplitem.boost.function"), function},
        {config.get("tplitem.boost.number"), to_string(functionInvocation)},

        {config.get("tplitem.boost.initializations"),
         generateParameterInitialization(parameters, function)},

        {config.get("tplitem.boost.assert_ending"),
         returnType.isContainer() ? "" : "_EQUAL"},
        {config.get("tplitem.boost.invocation"), function},
        {config.get("tplitem.boost.parameters"),
         generateParameterInvocation(parameters)},
        {config.get("tplitem.boost.return"), returnContent},

        {config.get("tplitem.boost.pointers"),
         generatePointersAsserts(parameters, function)}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.function"),
        tokensToReplace);

    appendTestCaseToTestFile(testContent);
    incrementFunctionCounter(function);
}

void BoostGen::generateMethodAssert(const std::string &method,
                                    const std::vector<InfoVariable> &parameters,
                                    const InfoType &returnType) {
    const unsigned functionInvocation = getFunctionCounter(method);

    string returnContent = returnType.isPointer() ? "*" : "";
    returnContent += generateReturnTypeInvocation(returnType, method);

    string objectTest = targetName + "_test";
    objectTest[0] = tolower(objectTest[0]);

    map<string, string> tokensToReplace = {
        {config.get("tplitem.boost.target"), targetName},
        {config.get("tplitem.boost.function"), method},
        {config.get("tplitem.boost.number"), "1"},

        {config.get("tplitem.boost.class"), targetName},
        {config.get("tplitem.boost.object"), objectTest},

        {config.get("tplitem.boost.initializations"),
         generateParameterInitialization(parameters, method)},

        {config.get("tplitem.boost.assert_ending"),
         returnType.isContainer() ? "" : "_EQUAL"},
        {config.get("tplitem.boost.invocation"), method},
        {config.get("tplitem.boost.parameters"),
         generateParameterInvocation(parameters)},
        {config.get("tplitem.boost.return"), returnContent},

        {config.get("tplitem.boost.pointers"),
         generatePointersAsserts(parameters, method)}};

    string testContent = replaceTokensInFile(
        templateFrameworkPath / config.get("file.template.case.method"),
        tokensToReplace);

    appendTestCaseToTestFile(testContent);
    incrementFunctionCounter(method);
}

void BoostGen::generateConstructorAssert(
    const std::vector<InfoVariable> &parameters) {
    const unsigned functionInvocation = getFunctionCounter(targetName);
    string invocationContent = targetName;

    map<string, string> tokensToReplace = {
        {config.get("tplitem.boost.target"), targetName},
        {config.get("tplitem.boost.function"), targetName},
        {config.get("tplitem.boost.number"), "1"},

        {config.get("tplitem.boost.initializations"),
         generateParameterInitialization(parameters, targetName)},

        {config.get("tplitem.boost.class"), targetName},
        {config.get("tplitem.boost.parameters"),
         generateParameterInvocation(parameters)},

        {config.get("tplitem.boost.pointers"),
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
                     config.get("tplitem.boost.parameter"),
                 TPLITEM_BOOST_EXPECTED = config.get("tplitem.boost.expected");

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

int BoostGen::getFunctionCounter(const std::string &function) const {
    return functionCounter.at(function);
}

void BoostGen::incrementFunctionCounter(const std::string &function) {
    functionCounter[function]++;
}