#include "ASTUTGen.hpp"
#include "ASTUTMatchers.hpp"

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
static cl::extrahelp MoreHelp("\nAuthor: Kevin J. Valle-Gomez\n");
static llvm::cl::OptionCategory OptC("example");

static std::string GENERATION_FOLDER="Generated";

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, OptC);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

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

    std::string system_op = "mkdir Generated_LOG" + today + "/ && cp -r " +
    GENERATION_FOLDER + "/UT/* Generated_LOG" + today + "/ && " +
    "rm -R " + GENERATION_FOLDER + "/UT/*";

    //Reset the results Folder
    system(system_op.c_str());
  }

  return Tool.run(newFrontendActionFactory(&Finder).get());

}
