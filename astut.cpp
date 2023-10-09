#include "ASTUTGen.hpp"
#include "ASTUTMatchers.hpp"
#include "auxiliary_functions.hpp"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"

#include "llvm/Support/CommandLine.h"

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <ctime>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

using namespace std;

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nIf you are working with C++ headers use the option -xc++ at the end.\nAuthor: Kevin J. Valle-Gomez (kevin.valle@uca.es)\n");
static llvm::cl::OptionCategory OptC("ASkeleTon - Unit Test Generator for C/C++");

cl::opt<bool> BoostFramework ("boost", cl::desc("Enable boost"), cl::init(true));
cl::opt<bool> CatchFramework ("catch2", cl::desc("Enable catch"), cl::init(true));
cl::opt<bool> GtestFramework ("gtest", cl::desc("Enable google test"), cl::init(true));

static std::string GENERATION_FOLDER="Generated";

int main(int argc, const char **argv) {
  //CommonOptionsParser OptionsParser(argc, argv, OptC);
  // Esto se ha quedado 'deprecated', usando esta solucion temporal
  Expected<CommonOptionsParser> options = CommonOptionsParser::create(argc, argv, OptC);
  ClangTool Tool(options->getCompilations(),
                 options->getSourcePathList());

  MatchFinder Finder;
  ASTUTGen Functionality;

  for(auto i : createMapMatchers())
  	Finder.addMatcher(i.second, &Functionality);

  //We'll get the time for the LOG folder
  if (folderExists(GENERATION_FOLDER)) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d%m%Y%H%M%S");
    std::string today = oss.str();
  
    //DEBUG: Generation of log folders is temporarily disabled
    //std::string system_op = "mkdir Generated_LOG" + today + "/ && cp -r " +
    //GENERATION_FOLDER + "/UT/* Generated_LOG" + today + "/ && " +
    //"rm -R " + GENERATION_FOLDER + "/UT/*";
    std::string system_op = "rm -R " + GENERATION_FOLDER + "/UT/*";

    //Reset the results Folder
    system(system_op.c_str());
  }

  return Tool.run(newFrontendActionFactory(&Finder).get());

}
