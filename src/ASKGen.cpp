#include "ASKGen.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "color.h"
#include "framework/BoostGen.hpp"
#include "framework/CatchGen.hpp"
#include "framework/GTestGen.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "clang/AST/ASTContext.h"

using namespace clang::ast_matchers;
using namespace clang;
using namespace std;
namespace fs = std::filesystem;

void printDebugInfo(const vector<InfoVariable> &parameters, const InfoType &returnType) {
    cout << "Params list: ";
    if (parameters.empty()) {
        cout << "no params";
    } else {
        unsigned i = 1;
        for (const auto &param : parameters) {
            cout << i++ << " - " << param.original << " " << param.name;
            if (&param != &parameters.back())
                cout << ", ";
        }
    }

    cout << "\nReturn type: " << returnType.original << "\n";
}

void ASKGen::run(const MatchFinder::MatchResult &Result) {
    apply_FD1(Result);
    apply_MD1(Result);
    // apply_CT1(Result); // Necessary for structs and classes
    apply_CC1(Result);

    // Kevin: dejamos estos fuera, nos interesa ahora solo las funciones, los
    // datos vendrán por KLEE
    // apply_DG1(Result);
    // apply_DG2(Result);
}

void ASKGen::apply_FD1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const FunctionDecl *UT = Result.Nodes.getNodeAs<clang::FunctionDecl>("FD1")) {

        FullSourceLoc FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            // In this case, we do not want class functions
            if (!isa<CXXMethodDecl>(UT)) {

                // Get the file name
                fs::path filePath = fs::absolute(
                    Context->getSourceManager().getFilename(UT->getBeginLoc()).str());
                string fileName = extractFileName(filePath);
                string target = fileName;

                // Print auxiliary
                // ======================================================================
                llvm::outs() << ANSI_BLUE << "Found FunctionDecl at "
                             << FullLocation.getSpellingLineNumber() << ":"
                             << FullLocation.getSpellingColumnNumber() << " - ";

                llvm::outs() << UT->getNameInfo().getAsString() << " in file " << fileName
                             << "\n"
                             << ANSI_RESET;
                // Print auxiliary
                // ======================================================================

                auto generator = getGenerator(target, filePath, false);
                auto configGenerator = getConfigGenerator(target);

                try {
                    generateTest(*generator, *configGenerator, UT);
                    llvm::outs()
                        << ANSI_GREEN << "Test for function "
                        << UT->getNameInfo().getAsString() << " generated successfully\n"
                        << ANSI_RESET;
                } catch (const ComplexTypeException &e) {
                    llvm::outs() << ANSI_MAGENTA << "Skipping test for function "
                                 << UT->getNameInfo().getAsString()
                                 << " due to complex type: " << e.type << "\n"
                                 << ANSI_RESET;
                }

                llvm::outs() << "--------------\n";
            }
        }
    }
}

void ASKGen::apply_MD1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const CXXMethodDecl *UT = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("MD1")) {

        FullSourceLoc FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            // No constructor, destructor or operator
            if (!isa<CXXConstructorDecl>(UT) && !isa<CXXDestructorDecl>(UT) &&
                !UT->isOverloadedOperator()) {
                string source_file =
                    Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
                string parentname = UT->getParent()->getName().str();

                // Print auxiliary
                // ======================================================================
                llvm::outs() << ANSI_BLUE << "Found CxxMethodDecl at "
                             << FullLocation.getSpellingLineNumber() << ":"
                             << FullLocation.getSpellingColumnNumber() << " - ";

                llvm::outs() << UT->getNameInfo().getAsString() << " from class "
                             << parentname << "\n"
                             << ANSI_RESET;
                // Print auxiliary
                // ======================================================================

                auto generator = getGenerator(parentname, source_file, true);
                auto configGenerator = getConfigGenerator(parentname);

                try {
                    generateTest(*generator, *configGenerator, UT);
                    llvm::outs()
                        << ANSI_GREEN << "Test for method "
                        << UT->getNameInfo().getAsString() << " generated successfully\n"
                        << ANSI_RESET;
                } catch (const ComplexTypeException &e) {
                    llvm::outs() << ANSI_MAGENTA << "Skipping test for method "
                                 << UT->getNameInfo().getAsString()
                                 << " due to complex type: " << e.type << "\n"
                                 << ANSI_RESET;
                }

                llvm::outs() << "--------------\n";
            }
        }
    }
}

void ASKGen::apply_CT1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const CXXRecordDecl *UT = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("CT1")) {

        FullSourceLoc FullLocation;

        FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            // Get the file name
            string source_file =
                Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
            unsigned first = source_file.find_last_of('/') + 1;
            unsigned last = source_file.find_last_of('.');

            string filename = source_file.substr(first, last - first);

            InfoType record(QualType(UT->getTypeForDecl(), 0));
            auto generator = getGenerator(filename, source_file, true);

            try {
                generateReadMethod(*generator, record);
                llvm::outs() << "Read method for class " << UT->getName().str()
                             << " generated successfully\n";
            } catch (const ComplexTypeException &e) {
                llvm::outs() << "Skipping read method for class " << UT->getName().str()
                             << " due to complex type: " << e.type << "\n";
            }

            // Print auxiliary
            // ======================================================================
            llvm::outs() << "Found CXXRecordDecl (struct-customtype) at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            llvm::outs() << record.original << " in file " << filename << "\n";
            // Print auxiliary
            // ======================================================================
        }
    }
}

void ASKGen::apply_CC1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const CXXConstructorDecl *UT =
            Result.Nodes.getNodeAs<clang::CXXConstructorDecl>("CC1")) {

        FullSourceLoc FullLocation;

        FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            string filePath =
                Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
            string fileName = extractFileName(filePath);
            string target = UT->getParent()->getName().str();

            // Print auxiliary
            // ======================================================================
            llvm::outs() << ANSI_BLUE << "Found CXXConstructorDecl at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            llvm::outs() << UT->getNameInfo().getAsString() << " from class " << target
                         << "\n"
                         << ANSI_RESET;
            // Print auxiliary
            // ======================================================================

            auto generator = getGenerator(target, filePath, true);
            auto configGenerator = getConfigGenerator(target);

            try {
                generateTest(*generator, *configGenerator, UT);
                llvm::outs() << ANSI_GREEN << "Test for method "
                             << UT->getNameInfo().getAsString()
                             << " generated successfully\n";
            } catch (const ComplexTypeException &e) {
                llvm::outs() << ANSI_MAGENTA << "Skipping test for method "
                             << UT->getNameInfo().getAsString()
                             << " due to complex type: " << e.type << "\n";
            }

            llvm::outs() << ANSI_RESET << "--------------\n";
        }
    }
}

void ASKGen::apply_PD1(const MatchFinder::MatchResult &Result) {
    // TO-DO: make this SHOW when a private member is called
}

// void ASKGen::apply_DG1(const MatchFinder::MatchResult &Result) {
//     ASTContext *Context = Result.Context;

//     if (const BinaryOperator *UT =
//             Result.Nodes.getNodeAs<clang::BinaryOperator>("DG1")) {

//         const FunctionDecl *FD =
//             Result.Nodes.getNodeAs<clang::FunctionDecl>("DG1b");
//         FullSourceLoc FullLocation;

//         FullLocation = Context->getFullLoc(UT->getBeginLoc());

//         if (FullLocation.isValid() &&
//             !Context->getSourceManager().isInSystemHeader(FullLocation)) {

//             string LHS_string = convertExpressionToString(
//                 UT->getLHS(), Context->getSourceManager());
//             string RHS_string = convertExpressionToString(
//                 UT->getRHS(), Context->getSourceManager());
//             string LHS_type = UT->getLHS()->getType().getAsString();
//             string RHS_type = UT->getRHS()->getType().getAsString();

//             string source_file = Context->getSourceManager()
//                                      .getFilename(UT->getBeginLoc())
//                                      .str();
//             unsigned first = source_file.find_last_of('/') + 1;
//             unsigned last = source_file.find_last_of('.');
//             string filename = source_file.substr(first, last - first);

//             string type;

//             if (!isNumeric(LHS_string) && isNumeric(RHS_string) &&
//                 isInParameters(LHS_string, FD->parameters(), type)) {
//                 generateTestData(filename, FD->getName().str(), LHS_string,
//                                  type, RHS_string);
//             } else if (isNumeric(LHS_string) && !isNumeric(RHS_string) &&
//                        isInParameters(RHS_string, FD->parameters(), type)) {
//                 generateTestData(filename, FD->getName().str(), RHS_string,
//                                  type, LHS_string);
//             } else {
//                 llvm::outs() << "non-numeric condition\n";
//             }

//             // Print auxiliary
//             //
//             ======================================================================
//             /*llvm::outs() << "Found BinaryOperator at "
//                          << FullLocation.getSpellingLineNumber() << ":"
//                          << FullLocation.getSpellingColumnNumber() << " - ";

//             llvm::outs() << " from function " << FD->getName().str() <<
//             "\n";*/
//             // Print auxiliary
//             //
//             ======================================================================
//         }
//     }
// }

// void ASKGen::apply_DG2(const MatchFinder::MatchResult &Result) {
//     ASTContext *Context = Result.Context;

//     if (const SwitchStmt *UT =
//             Result.Nodes.getNodeAs<clang::SwitchStmt>("DG2")) {

//         const FunctionDecl *FD =
//             Result.Nodes.getNodeAs<clang::FunctionDecl>("DG2b");
//         FullSourceLoc FullLocation;

//         FullLocation = Context->getFullLoc(UT->getBeginLoc());

//         if (FullLocation.isValid() &&
//             !Context->getSourceManager().isInSystemHeader(FullLocation)) {

//             string source_file = Context->getSourceManager()
//                                      .getFilename(UT->getBeginLoc())
//                                      .str();
//             unsigned first = source_file.find_last_of('/') + 1;
//             unsigned last = source_file.find_last_of('.');
//             string filename = source_file.substr(first, last - first);

//             // string test = UT->getCond()->getAsString();
//             // llvm::outs() << "test: " << test << "\n";
//             // string cname = UT->getCond()->getName().str();
//             // string ctype = UT->getCond()->getType().getAsString();

//             // llvm::outs() << /*"Cname: " << cname << */" Ctype: " << ctype
//             <<
//             // "\n";

//             /*string LHS_string = convertExpressionToString(UT->getLHS(),
//             Context->getSourceManager()); string RHS_string =
//             convertExpressionToString(UT->getRHS(),
//             Context->getSourceManager()); string LHS_type =
//             UT->getLHS()->getType().getAsString(); string RHS_type =
//             UT->getRHS()->getType().getAsString();

//             //llvm::outs() << "LHS: " << LHS_string << " " << LHS_type << " -
//             RHS: " << RHS_string << " " << RHS_type << "\n";

//             string source_file =
//             Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
//             unsigned first = source_file.find_last_of('/') + 1;
//             unsigned last = source_file.find_last_of('.');
//             string filename = source_file.substr(first, last-first);

//             string type;
//             if(!isNumeric(LHS_string) && isNumeric(RHS_string) &&
//             isInParameters(LHS_string, FD->parameters(), type))
//             {
//                 generateTestData(filename, FD->getName().str(), LHS_string,
//             type, RHS_string); } else if (isNumeric(LHS_string) &&
//             !isNumeric(RHS_string) && isInParameters(RHS_string,
//             FD->parameters(), type))
//             {
//                 generateTestData(filename, FD->getName().str(), RHS_string,
//             type, LHS_string); } else
//             {
//                 llvm::outs() << "non-numeric condition\n";
//             }*/

//             // Print auxiliary
//             //
//             ======================================================================
//             llvm::outs() << "Found SwitchStmt at "
//                          << FullLocation.getSpellingLineNumber() << ":"
//                          << FullLocation.getSpellingColumnNumber() << " - ";

//             llvm::outs() << " from function " << FD->getName().str() << "\n";
//             // Print auxiliary
//             //
//             ======================================================================
//         }
//     }
// }

void ASKGen::generateReadMethod(Generator &testGen,
                                const std::vector<InfoVariable> &variables) {
    for (const auto &variable : variables) {
        generateReadMethod(testGen, variable);
    }
}

void ASKGen::generateReadMethod(Generator &testGen, const std::vector<InfoType> &types) {
    for (const auto &type : types) {
        generateReadMethod(testGen, type);
    }
}

void ASKGen::generateReadMethod(Generator &testGen, const InfoVariable &variable) {
    testGen.createTypeReadToFixture(variable);
}

void ASKGen::generateReadMethod(Generator &testGen, const InfoType &type) {
    testGen.createTypeReadToFixture(type);
}

std::shared_ptr<Generator> ASKGen::getGenerator(const std::string &target,
                                                const std::string &filePath,
                                                bool isFromClass) {
    auto pos = generators.find(target);

    if (pos == generators.end()) {
        std::shared_ptr<Generator> generator;

        switch (getFramework()) {
        case CATCH:
            generator = std::make_shared<CatchGenerator>(target, filePath, isFromClass);
            break;
        case GTEST:
            generator = std::make_shared<GTestGenerator>(target, filePath, isFromClass);
            break;
        default:
            generator = std::make_shared<BoostGen>(target, filePath, isFromClass);
            break;
        }
        generators.insert({target, generator});
        return generator;
    }

    return pos->second;
}

std::shared_ptr<ConfigGenerator> ASKGen::getConfigGenerator(const std::string &target) {
    auto pos = configGenerators.find(target);

    if (pos == configGenerators.end()) {
        auto configGenerator = std::make_shared<ConfigGenerator>(target);
        configGenerators.insert({target, configGenerator});
        return configGenerator;
    }

    return pos->second;
}

std::vector<InfoVariable>
ASKGen::getParameters(const std::vector<ParmVarDecl *> &params) {
    std::vector<InfoVariable> parameters;
    std::transform(params.begin(), params.end(), std::back_inserter(parameters),
                   [](const ParmVarDecl *param) { return param; });
    return parameters;
}

void ASKGen::generateTest(Generator &testGen, ConfigGenerator &configGenerator,
                          const FunctionDecl *UT) {
    InfoType returnType(UT->getReturnType());
    std::vector<InfoVariable> parameters(getParameters(UT->parameters()));

    std::string functionName = UT->getName().str();
    if (function_occurrences[functionName]++ > 1) {
        functionName += "_" + std::to_string(function_occurrences[functionName]);
    }

#ifdef FULL_DEBUG
    printDebugInfo(parameters, returnType);
#endif /* FULL_DEBUG */

    generateReadMethod(testGen, parameters);
    generateReadMethod(testGen, returnType);

    testGen.generateFunctionAssert(functionName, parameters, returnType);
}

void ASKGen::generateTest(Generator &testGen, ConfigGenerator &configGenerator,
                          const CXXMethodDecl *UT) {
    InfoType returnType(UT->getReturnType());
    std::vector<InfoVariable> parameters(getParameters(UT->parameters()));
    bool isStatic = UT->isStatic();

    std::string functionName = UT->getNameInfo().getAsString();
    if (function_occurrences[functionName]++ > 1) {
        functionName += "_" + std::to_string(function_occurrences[functionName]);
    }

#ifdef FULL_DEBUG
    printDebugInfo(parameters, returnType);
#endif /* FULL_DEBUG */

    generateReadMethod(testGen, parameters);
    generateReadMethod(testGen, returnType);

    testGen.generateMethodAssert(functionName, parameters, returnType, isStatic);
}

void ASKGen::generateTest(Generator &testGen, ConfigGenerator &configGenerator,
                          const CXXConstructorDecl *UT) {
    std::vector<InfoVariable> parameters(getParameters(UT->parameters()));
    std::string constructorName = UT->getParent()->getName().str();

    if (function_occurrences[constructorName]++ > 1) {
        constructorName += "_" + std::to_string(function_occurrences[constructorName]);
    }

#ifdef FULL_DEBUG
    printDebugInfo(parameters, {"no return"});
#endif /* FULL_DEBUG */

    generateReadMethod(testGen, parameters);

    testGen.generateConstructorAssert(parameters);
}

void ASKGen::generateTestData(string source, string function_name, string param,
                              string type, string value) {
    // function.value=a,b,c,d,e
    map<string, vector<string>> function_values;
    string outputPath = "Generated/UT/" + source + "/" + source + "_data.txt";
    vector<string> values;
    string fileContent;

    ifstream tmp_output(outputPath);
    for (string line; getline(tmp_output, line);) {
        // for each line...
        auto delimiter = line.find(":");

        string fvalue = line.substr(0, delimiter);
        line = line.substr(delimiter + 1);

        delimiter = line.find(",");
        while (delimiter != string::npos) {
            auto key = line.substr(0, delimiter);
            line = line.substr(delimiter + 1);

            values.push_back(key);

            delimiter = line.find(",");
        }

        values.push_back(line);
        function_values.insert(std::pair<string, vector<string>>(fvalue, values));
        values.clear();
    }

    if (function_values.find(function_name + "." + param) != function_values.end()) {
        values = function_values.at(function_name + "." + param);
    }

    vector<string> realvalues = obtainTestData(type, value);
    for (auto it : realvalues)
        values.push_back(it);

    function_values[(function_name + "." + param)] = values;

    stringstream ss;

    for (auto it : function_values) {
        ss << it.first << ":";

        for (unsigned long i = 0; i < it.second.size(); i++) {
            ss << it.second[i];
            if (i < it.second.size() - 1)
                ss << ",";
        }

        ss << "\n";
    }

    ofstream outputFile(outputPath);
    outputFile << ss.str();
}

vector<string> ASKGen::obtainTestData(string type, string value) {
    vector<string> result;
    replaceAll(value, "\'", "");
    replaceAll(value, "\"", "");
    result.push_back(value);

    if (type.find("bool") != string::npos || type.find("_Bool") != string::npos) {
        result.push_back((value == "true") ? "false" : "true");
    } else if (type.find("string") != string::npos) {
        result.push_back(value + "_another");
    } else if (type.find("char") != string::npos) {
        char res = value.c_str()[0];
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    } else if (type.find("int") != string::npos) {
        int res = stoi(value);
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    } else if (type.find("double") != string::npos) {
        double res = stod(value);
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    } else if (type.find("float") != string::npos) {
        float res = stof(value);
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    }

    return result;
}

ASKGen::~ASKGen() {
    if (function_occurrences.empty()) {
        llvm::outs() << "ASkeleTon has not found any function to generate tests for\n";
        remove_all(getAskeletonHome() / getConfig()["route"]["generated"]);
    }
}