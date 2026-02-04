#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ConfigGenerator.hpp"
#include "framework/Generator.hpp"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

class ASKGen : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    ASKGen() = default;

    virtual void
    run(const clang::ast_matchers::MatchFinder::MatchResult &Result);

	~ASKGen();

private:
    void apply_FD1(const clang::ast_matchers::MatchFinder::MatchResult &Result);
    void apply_MD1(const clang::ast_matchers::MatchFinder::MatchResult &Result);
    void apply_CT1(const clang::ast_matchers::MatchFinder::MatchResult &Result);
    void apply_CC1(const clang::ast_matchers::MatchFinder::MatchResult &Result);

    // experimental
    void apply_PD1(const clang::ast_matchers::MatchFinder::MatchResult &Result);

    // Generaing test data
    // void apply_DG1(const MatchFinder::MatchResult &Result);
    // void apply_DG2(const MatchFinder::MatchResult &Result);

    void generateReadMethod(Generator &testGen,
                            const std::vector<InfoVariable> &variables);
    void generateReadMethod(Generator &testGen,
                            const std::vector<InfoType> &types);
    void generateReadMethod(Generator &testGen, const InfoVariable &variable);
    void generateReadMethod(Generator &testGen, const InfoType &type);

    std::vector<InfoVariable>
    getParameters(const std::vector<clang::ParmVarDecl *> &params);

    void generateTest(Generator &, ConfigGenerator &,
                      const clang::FunctionDecl *UT);
    void generateTest(Generator &, ConfigGenerator &,
                      const clang::CXXMethodDecl *UT);
    void generateTest(Generator &, ConfigGenerator &,
                      const clang::CXXConstructorDecl *UT);

    std::shared_ptr<Generator> getGenerator(const std::string &target,
                                            const std::string &filePath,
                                            bool isFromClass = false);
    std::shared_ptr<ConfigGenerator>
    getConfigGenerator(const std::string &target);

    void generateTestData(std::string source, std::string function_name,
                          std::string param, std::string type,
                          std::string value);
    std::vector<std::string> obtainTestData(std::string type,
                                            std::string value);

    std::map<std::string, int> function_occurrences;

    std::map<std::string, std::shared_ptr<Generator>> generators;
    std::map<std::string, std::shared_ptr<ConfigGenerator>> configGenerators;
};
