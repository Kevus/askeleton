#include "ASTUTGen.hpp"

void ASTUTGen::run(const MatchFinder::MatchResult &Result)
{
	apply_FunctionExample(Result);
}

void ASTUTGen::apply_FunctionExample(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

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