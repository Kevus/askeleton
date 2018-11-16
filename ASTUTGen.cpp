#include "ASTUTGen.hpp"

void ASTUTGen::run(const MatchFinder::MatchResult &Result)
{
	apply_FD1(Result);
}

void ASTUTGen::apply_FD1(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const FunctionDecl *UT = Result.Nodes.getNodeAs<clang::FunctionDecl>("FD1")){
		
		FullSourceLoc FullLocation;
			
		FullLocation = Context->getFullLoc(UT->getLocStart());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation)){	

			//In this case, we do not want class functions
			if(!isa<CXXMethodDecl>(UT))
			{
				string source_file = Context->getSourceManager().getFilename(UT->getLocStart());

				//Get the file name
				unsigned first = source_file.find_last_of('/') + 1;
				unsigned last = source_file.find_last_of('.');
				string filename = source_file.substr(first, last-first);


				//Print auxiliary ======================================================================
	           	llvm::outs() << "Found FunctionDecl at "
	                         << FullLocation.getSpellingLineNumber() << ":"
	                         << FullLocation.getSpellingColumnNumber() << " - ";

	            llvm::outs() <<  UT->getNameInfo().getAsString() << " in file " << filename << "\n";
	            //Print auxiliary ======================================================================

	            ConfigGenerator cfg_gen(filename);

	            //Get the parameters
	            map<string, string> param_type;
	            for(auto i : UT->parameters())
	            {
	            	param_type.insert(pair<string, string>(i->getQualifiedNameAsString(), i->getOriginalType().getAsString()));
	            }

	            cfg_gen.generateTestCase(UT->getName(), param_type, "return_" + UT->getReturnType().getAsString());


			}
		}
	}
}