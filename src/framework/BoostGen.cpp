#include "framework/BoostGen.hpp"

#include "constants.hpp"
#include "utils/templating.hpp"
#include <cctype>
#include <iostream>

using namespace askeleton;
using namespace std;

BoostGen::BoostGen(const std::string &targetName, const std::string &filePath,
                   bool isFromClass)
    : Generator(targetName, filePath, "boost", isFromClass) {}

void BoostGen::generateFunctionAssert(
    const std::string &function, const std::vector<InfoVariable> &parameters,
    const InfoType &returnType) {

    string invocationContent, returnContent;
    if (returnType.isPointer()) {
        invocationContent = "*";
        returnContent = "*";
    }

    invocationContent += function;
    returnContent += generateReturnTypeInvocation(returnType, function);

    map<string, string> tokensToReplace = {
        {"{target}", targetName},
        {"{function}", function},
        {"{number}", "1"},

        {"{initializations}",
         generateParameterInitialization(parameters, function)},

        {"{assertEnding}", returnType.isContainer() ? "" : "_EQUAL"},
        {"{invocation}", invocationContent},
        {"{parameters}", generateParameterInvocation(parameters)},
        {"{return}", returnContent},

        {"{pointers}", generatePointersAsserts(parameters, function)}};

    string testTemplate = getFrameworkTemplatePath("functionCase.tpl");
    string testContent = replaceTokensInFile(testTemplate, tokensToReplace);

    appendTestCaseToTestFile(testContent);
}

void BoostGen::generateMethodAssert(const std::string &method,
                                    const std::vector<InfoVariable> &parameters,
                                    const InfoType &returnType) {

    string invocationContent, returnContent;
    if (returnType.isPointer()) {
        invocationContent = "*";
        returnContent = "*";
    }

    invocationContent += method;
    returnContent += generateReturnTypeInvocation(returnType, method);

    string objectTest = targetName + "_test";
    objectTest[0] = tolower(objectTest[0]);

    appendTestCaseToTestFile(replaceTokensInFile(
        getFrameworkTemplatePath("methodCase.tpl"),
        {{"{target}", targetName},
         {"{function}", method},
         {"{number}", "1"},

         {"{class}", targetName},
         {"{object}", objectTest},

         {"{inicializations}",
          generateParameterInitialization(parameters, method)},

         {"{assertEnding}", returnType.isContainer() ? "" : "_EQUAL"},
         {"{invocation}", invocationContent},
         {"{parameters}", generateParameterInvocation(parameters)},
         {"{return}", returnContent},

         {"{pointers}", generatePointersAsserts(parameters, method)}}));
}

void BoostGen::generateConstructorAssert(
    const std::vector<InfoVariable> &parameters) {

    string invocationContent = targetName;
    cout << "constructor\n";

    map<string, string> tokensToReplace = {
        {"{target}", targetName},
        {"{function}", targetName},
        {"{number}", "1"},

        {"{initializations}",
         generateParameterInitialization(parameters, targetName)},

        {"{class}", targetName},
        {"{parameters}", generateParameterInvocation(parameters)},

        {"{pointers}", generatePointersAsserts(parameters, targetName)}};

    appendTestCaseToTestFile(replaceTokensInFile(
        getFrameworkTemplatePath("constructorCase.tpl"), tokensToReplace));
}

std::string
BoostGen::generatePointersAsserts(const std::vector<InfoVariable> &parameters,
                                  const std::string &function) const {

    std::stringstream ss;

    for (const auto &param : parameters) {
        if (param.isPointer() || param.isReference()) {
            InfoType underlyingType = param.getUnderlyingType();
            InfoVariable underlyingVar{param.name + "_output",
                                       underlyingType.original,
                                       underlyingType.formatted};
            string pointerAssert = BOOST_ASSERT_POINTER;
            replaceTokensInText(pointerAssert,
                                {{"{parameter}", param.name},
                                 {"{expected}", generateReadInvocation(
                                                    underlyingVar, function)}});
            ss << "\n\t" << pointerAssert;
            if (&param == &parameters.back()) {
                ss << "\n";
            }
        }
    }

    return ss.str();
}

const std::string BoostGen::BOOST_ASSERT_POINTER =
    "BOOST_CHECK_EQUAL({parameter}, {expected});";