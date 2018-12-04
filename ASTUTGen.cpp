#include "ASTUTGen.hpp"

void ASTUTGen::run(const MatchFinder::MatchResult &Result)
{
	apply_FD1(Result);
	apply_MD1(Result);
	apply_CT1(Result);
}

//General method for testing functions
void ASTUTGen::generateFunctionTest(string source_file, string function_name, ArrayRef<ParmVarDecl *> parameters, string return_type, BoostGenerator bGen)
{
	ConfigGenerator cfg_gen(source_file);

	//Get the parameters
	map<string, string> param_type;
	vector<string> insert_order;

	string rtn_type = cleanUnnecesaryChars(return_type);
	string tmp_type;

	for(auto i : parameters)
    {
    	tmp_type = i->getOriginalType().getAsString();
    	tmp_type = cleanUnnecesaryChars(tmp_type);


    	param_type.insert(pair<string, string>(i->getQualifiedNameAsString(), tmp_type));
    	insert_order.push_back(i->getQualifiedNameAsString());
    }

    CustomGenerator cgen(source_file);
    cgen.generateTypesFile(function_name, param_type, insert_order, rtn_type);
    cgen.generateTestCasesFile(function_name, param_type, insert_order, rtn_type);

    cfg_gen.generateTestCase(function_name, param_type, insert_order, rtn_type);
    bGen.generateBoostAssert(source_file, function_name, param_type, insert_order, rtn_type);

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

				//Get the file name
				string source_file = Context->getSourceManager().getFilename(UT->getLocStart());
				unsigned first = source_file.find_last_of('/') + 1;
				unsigned last = source_file.find_last_of('.');

				string filename = source_file.substr(first, last-first);

				BoostGenerator bGen(source_file, filename, false);

				generateFunctionTest(filename,
									 UT->getName(),
									 UT->parameters(),
									 UT->getReturnType().getAsString(),
									 bGen);
				//Print auxiliary ======================================================================
	           	llvm::outs() << "Found FunctionDecl at "
	                         << FullLocation.getSpellingLineNumber() << ":"
	                         << FullLocation.getSpellingColumnNumber() << " - ";

	            llvm::outs() <<  UT->getNameInfo().getAsString() << " in file " << filename << "\n";
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

			//In this case, we do not want class constructors
			if(!isa<CXXConstructorDecl>(UT))
			{
				string source_file = Context->getSourceManager().getFilename(UT->getLocStart());
				string parentname = UT->getParent()->getName();

				BoostGenerator bGen(source_file, parentname, true);

				generateFunctionTest(parentname,
					 UT->getName(),
					 UT->parameters(),
					 UT->getReturnType().getAsString(),
					 bGen);

				//Print auxiliary ======================================================================
	           	llvm::outs() << "Found CxxMethodDecl at "
	                         << FullLocation.getSpellingLineNumber() << ":"
	                         << FullLocation.getSpellingColumnNumber() << " - ";

	            llvm::outs() <<  UT->getNameInfo().getAsString() << " from class " << parentname << "\n";
	            //Print auxiliary ======================================================================


			}
		}
	}
}

void ASTUTGen::apply_CT1(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const CXXRecordDecl *UT = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("CT1")){
		
		FullSourceLoc FullLocation;
			
		FullLocation = Context->getFullLoc(UT->getLocStart());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation)){	


				//Print auxiliary ======================================================================
	           	llvm::outs() << "Found CxxRecordDecl at "
	                         << FullLocation.getSpellingLineNumber() << ":"
	                         << FullLocation.getSpellingColumnNumber() << " - ";

	           // llvm::outs() <<  UT->getName().getAsString() << " from class " << parentname << "\n";
	            //Print auxiliary ======================================================================


			
		}
	}
}