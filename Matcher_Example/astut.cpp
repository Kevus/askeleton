#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersInternal.h"
#include "clang/ASTMatchers/ASTMatchersMacros.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"

#include "llvm/Support/CommandLine.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "clang/Lex/Lexer.h"

#include <stdio.h>

using namespace clang::tooling;
using namespace llvm;

using namespace clang;
using namespace clang::ast_matchers;

using namespace std;

namespace clang{
	namespace ast_matchers{
	  using namespace clang::ast_matchers::internal;

	}
}

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...");

/*******************************************************************************/

//FOR
DeclarationMatcher FunctionExample =
recordDecl(
	forEachDescendant(
		functionDecl(
				//hasAnyParameter(anything())
			).bind("FunctionExample")
	)
);

/*******************************************************************************/

class FunctExample : public MatchFinder::MatchCallback {
public :
  virtual void run(const MatchFinder::MatchResult &Result) {
	ASTContext *Context = Result.Context;
	Rewriter Rewrite;
	Rewrite.setSourceMgr(Context->getSourceManager(), Context->getLangOpts());
  	
	if (const FunctionDecl *UT = Result.Nodes.getNodeAs<clang::FunctionDecl>("FunctionExample")){
		
		FullSourceLoc FullLocation;
			
		FullLocation = Context->getFullLoc(UT->getLocStart());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation)){	
			    	
           	llvm::outs() << "Found declaration at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << "\n";

            llvm::outs() <<  UT->getNameInfo().getAsString() << "\n";
		}
	}
  }
};	

static llvm::cl::OptionCategory MyToolCategory("example");

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  FunctExample FE;
  MatchFinder Finder;
  Finder.addMatcher(FunctionExample, &FE);

  return Tool.run(newFrontendActionFactory(&Finder).get());
}
