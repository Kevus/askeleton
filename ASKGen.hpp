#ifndef ASKGEN_HPP
#define ASKGEN_HPP

#include "Generator/ConfigGenerator.hpp"
#include "Generator/CustomGenerator.hpp"
#include "Generator/TestFrameworks.hpp"
#include "framework/Generator.hpp"

#include "framework/BoostGen.hpp"

#include "auxiliary_functions.hpp"

#include <map>
#include <stdio.h>
#include <string>
#include <vector>

using namespace clang::tooling;
using namespace llvm;

using namespace clang;
using namespace clang::ast_matchers;

using namespace std;

class ASKGen : public MatchFinder::MatchCallback {
public:
    ASKGen() {}

    virtual void run(const MatchFinder::MatchResult &Result);

private:
    void apply_FD1(const MatchFinder::MatchResult &Result);
    void apply_MD1(const MatchFinder::MatchResult &Result);
    void apply_CT1(const MatchFinder::MatchResult &Result);
    void apply_CC1(const MatchFinder::MatchResult &Result);

    // experimental
    void apply_PD1(const MatchFinder::MatchResult &Result);

    // Generaing test data
    void apply_DG1(const MatchFinder::MatchResult &Result);
    void apply_DG2(const MatchFinder::MatchResult &Result);

    void generateReadMethod(Generator &testGen,
                            const std::vector<InfoVariable> &variables);
    void generateReadMethod(Generator &testGen,
                            const std::vector<InfoType> &types);
    void generateReadMethod(Generator &testGen, const InfoVariable &variable);
    void generateReadMethod(Generator &testGen, const InfoType &type);

    std::vector<InfoVariable>
    getParameters(const std::vector<ParmVarDecl *> &params);

    void generateTest(Generator &, ConfigGenerator &, const FunctionDecl *UT);
    void generateTest(Generator &, ConfigGenerator &, const CXXMethodDecl *UT);
    void generateTest(Generator &, ConfigGenerator &,
                      const CXXConstructorDecl *UT);

    std::shared_ptr<Generator> getGenerator(const std::string &target,
                                            const std::string &filePath,
                                            bool isFromClass = false);

    void generateTestData(string source, string function_name, string param,
                          string type, string value);
    vector<string> obtainTestData(string type, string value);

    map<string, int> function_occurrences;

    std::map<std::string, std::shared_ptr<Generator>> generators;

    /**
     * TODO: eliminar todo lo que continua
     */
    void generateFunctionTest(string source_file, string function_name,
                              ArrayRef<ParmVarDecl *> parameters,
                              QualType return_type, BoostGenerator bGen);
    void generateConstructorTest(string source, string constructor_name,
                                 ArrayRef<ParmVarDecl *> parameters,
                                 BoostGenerator bgen);
    // This generates a struct read
    void generateCustomTypeFixture(string source, string type_name,
                                   vector<FieldDecl *> parameters,
                                   bool overloadedEq, bool overloadedFlux,
                                   BoostGenerator bGen);

    // Receives a list of every type and generates a read for each one
    void generateCustomTypeFixture(string filename,
                                   const vector<const CXXRecordDecl *> &records,
                                   const vector<const EnumDecl *> &enums,
                                   const vector<pair<string, string>> &pointers,
                                   BoostGenerator &);

    //   This generates a enum read
    void generateEnumTypeFixture(string source,
                                 const pair<string, string> &type,
                                 BoostGenerator &bGen);

    // string convertExpressionToString(Expr *E, SourceManager &SM);
    // bool isInParameters(string name, ArrayRef<ParmVarDecl *> params, string&
    // type);
};

#endif