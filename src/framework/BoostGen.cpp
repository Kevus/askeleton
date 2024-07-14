#include "framework/BoostGen.hpp"

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
    returnContent += generateReadType(returnType);

    appendTestCaseToTestFile(replaceTokensInFile(
        getFrameworkTemplatePath("functionCase.tpl"),
        {{"{target}", targetName},
         {"{function}", function},
         {"{number}", "1"},

         {"{inicializations}", generateParamsInitilizations(parameters)},

         {"{assertEnding}", returnType.isContainer() ? "" : "_EQUAL"},
         {"{invocation}", invocationContent},
         {"{parameters}", generateParamsInvocation(parameters)},
         {"{return}", returnContent},

         {"{pointers}", generatePointersAsserts(parameters)}}));
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
    returnContent += generateReadType(returnType);

    string objectTest = targetName + "_test";
    objectTest[0] = toLower(objectTest[0]);

    appendTestCaseToTestFile(replaceTokensInFile(
        getFrameworkTemplatePath("methodCase.tpl"),
        {{"{target}", targetName},
         {"{function}", method},
         {"{number}", "1"},

         {"{class}", targetName},
         {"{object}", objectTest},

         {"{inicializations}", generateParamsInitilizations(parameters)},

         {"{assertEnding}", returnType.isContainer() ? "" : "_EQUAL"},
         {"{invocation}", invocationContent},
         {"{parameters}", generateParamsInvocation(parameters)},
         {"{return}", returnContent},

         {"{pointers}", generatePointersAsserts(parameters)}}));
}

void BoostGen::generateConstructorAssert(
    const std::vector<InfoVariable> &parameters) {

    appendTestCaseToTestFile(replaceTokensInFile(
        getFrameworkTemplatePath("constructorCase.tpl"),
        {{"{target}", targetName},
         {"{function}", targetName},
         {"{number}", "1"},

         {"{inicializations}", generateParamsInitilizations(parameters)},

         {"{class}", targetName},
         {"{parameters}", generateParamsInvocation(parameters)},

         {"{pointers}", generatePointersAsserts(parameters)}}));
}

std::string BoostGen::generatePointersAsserts(
    const std::vector<InfoVariable> &parameters) const {

    std::stringstream ss;

    for (const auto &param : parameters) {
        if (param.isPointer() || param.isReference()) {
            string pointerAssert = BOOST_ASSERT_POINTER;
            replaceTokensInText(pointerAssert,
                                {{"{parameter}", param.name},
                                 {"{expected}", generateReadType(param.type)}});
            ss << pointerAssert << "\n";
        }
    }

    return ss.str();
}

const std::string BoostGen::BOOST_ASSERT_POINTER =
    "BOOST_CHECK_EQUAL({parameter}, {expected});";