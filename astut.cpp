#include "ASTUTGen.hpp"
#include "ASTUTMatchers.hpp"

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"

#include "llvm/Support/CommandLine.h"

#include <stdio.h>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

using namespace std;

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");
static llvm::cl::OptionCategory OptC("example");

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, OptC);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  MatchFinder Finder;
  ASTUTGen Functionality;

  for(auto i : createMapMatchers())
  {
  	Finder.addMatcher(i.second, &Functionality);
  }

  return Tool.run(newFrontendActionFactory(&Finder).get());

}
