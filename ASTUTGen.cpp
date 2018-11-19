#include "ASTUTGen.hpp"

void ASTUTGen::run(const MatchFinder::MatchResult &Result)
{
	apply_FD1(Result);
	apply_MD1(Result);
}

//General method for testing functions
void ASTUTGen::generateFunctionTest(string source_file, string function_name, ArrayRef<ParmVarDecl *> parameters, string return_type)
{
	//Get the file name
	unsigned first = source_file.find_last_of('/') + 1;
	unsigned last = source_file.find_last_of('.');
	string filename;

	if(first != string::npos && last != string::npos)
		filename = source_file.substr(first, last-first);
	else
		filename = source_file;

	ConfigGenerator cfg_gen(filename);

	//Get the parameters
	map<string, string> param_type;
	int order = 0;

	for(auto i : parameters)
    {
    	param_type.insert(pair<string, string>(to_string(order) + "_" + i->getQualifiedNameAsString(), i->getOriginalType().getAsString()));
    	order++;
    }

    cfg_gen.generateTestCase(function_name, param_type, return_type);


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
				generateFunctionTest(Context->getSourceManager().getFilename(UT->getLocStart()),
									 UT->getName(),
									 UT->parameters(),
									 UT->getReturnType().getAsString());

				//Print auxiliary ======================================================================
	           	llvm::outs() << "Found FunctionDecl at "
	                         << FullLocation.getSpellingLineNumber() << ":"
	                         << FullLocation.getSpellingColumnNumber() << " - ";

	            llvm::outs() <<  UT->getNameInfo().getAsString() /*<< " in file " << filename*/ << "\n";
	            //Print auxiliary ======================================================================

			}
		}
	}
}

void ASTUTGen::apply_MD1(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const CXXMethodDecl *UT = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("MD1")){
		
		FullSourceLoc FullLocation;
			
		FullLocation = Context->getFullLoc(UT->getLocStart());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation)){	

			//In this case, we do not want class functions
			if(!isa<CXXConstructorDecl>(UT))
			{

				generateFunctionTest(UT->getParent()->getName(),
					 UT->getName(),
					 UT->parameters(),
					 UT->getReturnType().getAsString());

				//Print auxiliary ======================================================================
	           	llvm::outs() << "Found FunctionDecl at "
	                         << FullLocation.getSpellingLineNumber() << ":"
	                         << FullLocation.getSpellingColumnNumber() << " - ";

	            llvm::outs() <<  UT->getNameInfo().getAsString() /*<< " in file " << filename*/ << "\n";
	            //Print auxiliary ======================================================================


			}
		}
	}
}