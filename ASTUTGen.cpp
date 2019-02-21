#include "ASTUTGen.hpp"

void ASTUTGen::run(const MatchFinder::MatchResult &Result)
{
	apply_FD1(Result);
	apply_MD1(Result);
	apply_CT1(Result);
	apply_CC1(Result);

	apply_DG1(Result);
	apply_DG2(Result);
}

//General method for testing functions
void ASTUTGen::generateFunctionTest(string source_file, string function_name, ArrayRef<ParmVarDecl *> parameters, string return_type, BoostGenerator bGen)
{
	ConfigGenerator cfg_gen(source_file);
	string function_cfg_name = function_name;

	//if( function_occurrences.find(function_name) == function_occurrences.end() )
	//	function_occurences.insert( pair<string, int> (function_name, 1));
	function_occurrences[function_name]++;

	if (function_occurrences[function_name] > 1)
		function_cfg_name += "_" + to_string(function_occurrences[function_name]);

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

    /*CustomGenerator cgen(source_file);
    cgen.generateTypesFile(function_name, param_type, insert_order, rtn_type);
    cgen.generateTestCasesFile(function_name, param_type, insert_order, rtn_type);*/

    cfg_gen.generateTestCase(function_cfg_name, param_type, insert_order, rtn_type);
    bGen.generateBoostAssert(source_file, function_name, function_cfg_name, param_type, insert_order, rtn_type);

}

//Method for constructing constructor test
void ASTUTGen::generateConstructorTest(string source, string constructor_name, ArrayRef<ParmVarDecl *> parameters, BoostGenerator bGen)
{
	ConfigGenerator cfg_gen(source);

	string constructor_cfg_name = constructor_name;
	function_occurrences[constructor_name]++;

	if (function_occurrences[constructor_name] > 1)
		constructor_cfg_name += "_" + to_string(function_occurrences[constructor_name]);

	//Get the parameters
	map<string, string> param_type;
	vector<string> insert_order;

	string tmp_type;

	for(auto i : parameters)
	{
		tmp_type = i->getOriginalType().getAsString();
    	tmp_type = cleanUnnecesaryChars(tmp_type);

    	param_type.insert(pair<string, string>(i->getQualifiedNameAsString(), tmp_type));
    	insert_order.push_back(i->getQualifiedNameAsString());
	}

	/**
	** We will add custom generator lates
	**/
	cfg_gen.generateConstructorTest(constructor_cfg_name, param_type, insert_order);
	bGen.generateBoostConstructorAssert(source, constructor_name, constructor_cfg_name, param_type, insert_order);

}

void ASTUTGen::generateCustomTypeFixture(string source, string type_name, vector<FieldDecl *> parameters, BoostGenerator bGen)
{
	ConfigGenerator cfg_gen(source);

	//Get the parameters
	map<string, string> param_type;
	vector<string> insert_order;

	string tmp_type;

	for(auto i : parameters)
	{
		tmp_type = i->getType().getAsString();
    	tmp_type = cleanUnnecesaryChars(tmp_type);

    	param_type.insert(pair<string, string>(i->getNameAsString(), tmp_type));
    	insert_order.push_back(i->getNameAsString());
	}

	bGen.addReadTypeToFixture(type_name, param_type, insert_order, false, false);
}

void ASTUTGen::generateTestData(string source, string function_name, string param, string type, string value)
{
	//function.value=a,b,c,d,e
	map<string, vector<string>> function_values;
	string outputPath = "Generated/UT/" + source + "/" + source + "_data.txt";
	vector<string> values;
	string fileContent;

	//First, we check the file
	//bool file_exists = fileExists(outputPath);

	/*if(!file_exists)
	{
		fileContent = function_name + "." + param + ":" + value + "\n";
		ofstream outputFile(outputPath);

		outputFile << fileContent;
	} else
	{*/
	ifstream tmp_output(outputPath);
	for(string line; getline(tmp_output, line); )
	{
		//for each line...
		auto delimiter = line.find(":");

		string fvalue = line.substr(0, delimiter);
		line = line.substr(delimiter + 1);

		delimiter = line.find(",");
		while(delimiter != string::npos)
		{
			auto key = line.substr(0, delimiter);
			line = line.substr(delimiter + 1);

			values.push_back(key);

			delimiter = line.find(",");
		}

		values.push_back(line);
		function_values.insert(std::pair<string, vector<string>>(fvalue, values));
		values.clear();
	}

	if(function_values.find(function_name + "." + param) != function_values.end()){
		values = function_values.at(function_name + "." + param);
	}

	//If the value is integer
	//Idea: Crear metodo que devuelva vector<string> con los valores
	/*int realv = boost::lexical_cast<int>(value);
	vector<int> realvalues{realv - 1, realv, realv + 1};
	for(auto it : realvalues)
		values.push_back(to_string(it));*/

	vector<string> realvalues = obtainTestData(type, value);
	for(auto it : realvalues)
		values.push_back(it);

	function_values[(function_name + "." + param)] = values;
	//function_values.insert(std::pair<string, vector<string>>((function_name + "." + param), values));

	stringstream ss;

	for (auto it : function_values)
	{
		ss << it.first << ":";

		for (unsigned long i = 0; i < it.second.size(); i++)
		{
			ss << it.second[i];
			if (i < it.second.size() - 1)
				ss << ",";
		}

		ss << "\n";
	}

	ofstream outputFile(outputPath);
	outputFile << ss.str();

	//}

}

vector<string> ASTUTGen::obtainTestData(string type, string value)
{
	vector<string> result;
	boost::replace_all(value, "\'", "");
	boost::replace_all(value, "\"", "");
	result.push_back(value);

	if(type == "bool" || type == "_Bool")
	{
		result.push_back(
			(value == "true") ? "false" : "true"
		);
	} else if(type == "string")
	{
		result.push_back(value + "_another");
	} else if(type == "char")
	{
		char res = boost::lexical_cast<char>(value);
		result.push_back(boost::lexical_cast<string>(res+1));
		result.push_back(boost::lexical_cast<string>(res-1));
	} else if(type == "int")
	{
		int res = boost::lexical_cast<int>(value);
		result.push_back(boost::lexical_cast<string>(res+1));
		result.push_back(boost::lexical_cast<string>(res-1));
	} else if(type == "double")
	{
		double res = boost::lexical_cast<double>(value);
		result.push_back(boost::lexical_cast<string>(res+1));
		result.push_back(boost::lexical_cast<string>(res-1));
	} else if(type == "float")
	{
		float res = boost::lexical_cast<float>(value);
		result.push_back(boost::lexical_cast<string>(res+1));
		result.push_back(boost::lexical_cast<string>(res-1));
	}

	return result;
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

			/*
			CXXRecordDecl* cl = ...;
			for (const auto& field : cl->fields) {
			    const auto& name = field->getName();
			    const auto field_cl = field->getType()->getAsCXXRecordDecl(); 
			}*/

			//Get the file name
			string source_file = Context->getSourceManager().getFilename(UT->getLocStart());
			unsigned first = source_file.find_last_of('/') + 1;
			unsigned last = source_file.find_last_of('.');

			string filename = source_file.substr(first, last-first);

			BoostGenerator bGen(source_file, filename, false);

			//We'll read the fields here
			vector<FieldDecl *> field_decl;

			for(auto i : UT->fields())
			{
				field_decl.push_back(i);
			}

			generateCustomTypeFixture(filename,
					UT->getNameAsString(),
					field_decl,
					bGen
				);

			//addReadTypeToFixture(string type_name, map<string, string> param_type, vector<string> insertion_order)

			//Print auxiliary ======================================================================
           	llvm::outs() << "Found CXXRecordDecl (struct-customtype) at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            llvm::outs() <<  UT->getNameAsString() << " in file " << filename << "\n";
            //Print auxiliary ======================================================================


			
		}
	}
}

void ASTUTGen::apply_CC1(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const CXXConstructorDecl *UT = Result.Nodes.getNodeAs<clang::CXXConstructorDecl>("CC1")){
		
		FullSourceLoc FullLocation;
			
		FullLocation = Context->getFullLoc(UT->getLocStart());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation)){	

			string source_file = Context->getSourceManager().getFilename(UT->getLocStart());
			string parentname = UT->getParent()->getName();

			BoostGenerator bGen(source_file, parentname, true);

			generateConstructorTest(
				 parentname,
				 parentname,
				 UT->parameters(),
				 bGen);

			//Print auxiliary ======================================================================
           	llvm::outs() << "Found CXXConstructorDecl at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            llvm::outs() <<  UT->getNameInfo().getAsString() << " from class " << parentname << "\n";
            //Print auxiliary ======================================================================


			
		}
	}
}

void ASTUTGen::apply_PD1(const MatchFinder::MatchResult &Result)
{
	//ASTContext *Context = Result.Context;
}

void ASTUTGen::apply_DG1(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const BinaryOperator *UT = Result.Nodes.getNodeAs<clang::BinaryOperator>("DG1")){

		const FunctionDecl *FD = Result.Nodes.getNodeAs<clang::FunctionDecl>("DG1b");
		FullSourceLoc FullLocation;
			
		FullLocation = Context->getFullLoc(UT->getLocStart());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation)){	
			
			string LHS_string = convertExpressionToString(UT->getLHS(), Context->getSourceManager());
			string RHS_string = convertExpressionToString(UT->getRHS(), Context->getSourceManager());
			string LHS_type = UT->getLHS()->getType().getAsString();
			string RHS_type = UT->getRHS()->getType().getAsString();

			//llvm::outs() << "LHS: " << LHS_string << " " << LHS_type << " - RHS: " << RHS_string << " " << RHS_type << "\n";

			string source_file = Context->getSourceManager().getFilename(UT->getLocStart());
			unsigned first = source_file.find_last_of('/') + 1;
			unsigned last = source_file.find_last_of('.');
			string filename = source_file.substr(first, last-first);

			string type;
			if(!isNumeric(LHS_string) && isNumeric(RHS_string) && isInParameters(LHS_string, FD->parameters(), type))
			{
				generateTestData(filename, FD->getName(), LHS_string, type, RHS_string);
			} else if (isNumeric(LHS_string) && !isNumeric(RHS_string) && isInParameters(RHS_string, FD->parameters(), type))
			{
				generateTestData(filename, FD->getName(), RHS_string, type, LHS_string);
			} else
			{
				llvm::outs() << "non-numeric condition\n";
			}

			//Print auxiliary ======================================================================
           	llvm::outs() << "Found BinaryOperator at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            llvm::outs() << " from function " << FD->getName() <<  "\n";
            //Print auxiliary ======================================================================


			
		}
	}
}

void ASTUTGen::apply_DG2(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const SwitchStmt *UT = Result.Nodes.getNodeAs<clang::SwitchStmt>("DG2")){

		const FunctionDecl *FD = Result.Nodes.getNodeAs<clang::FunctionDecl>("DG2b");
		FullSourceLoc FullLocation;
			
		FullLocation = Context->getFullLoc(UT->getLocStart());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation)){	
			
			string source_file = Context->getSourceManager().getFilename(UT->getLocStart());
			unsigned first = source_file.find_last_of('/') + 1;
			unsigned last = source_file.find_last_of('.');
			string filename = source_file.substr(first, last-first);


			//string test = UT->getCond()->getAsString();
			//llvm::outs() << "test: " << test << "\n";
			//string cname = UT->getCond()->getName();
			//string ctype = UT->getCond()->getType().getAsString();
			
			//llvm::outs() << /*"Cname: " << cname << */" Ctype: " << ctype << "\n";		


			/*string LHS_string = convertExpressionToString(UT->getLHS(), Context->getSourceManager());
			string RHS_string = convertExpressionToString(UT->getRHS(), Context->getSourceManager());
			string LHS_type = UT->getLHS()->getType().getAsString();
			string RHS_type = UT->getRHS()->getType().getAsString();

			//llvm::outs() << "LHS: " << LHS_string << " " << LHS_type << " - RHS: " << RHS_string << " " << RHS_type << "\n";

			string source_file = Context->getSourceManager().getFilename(UT->getLocStart());
			unsigned first = source_file.find_last_of('/') + 1;
			unsigned last = source_file.find_last_of('.');
			string filename = source_file.substr(first, last-first);

			string type;
			if(!isNumeric(LHS_string) && isNumeric(RHS_string) && isInParameters(LHS_string, FD->parameters(), type))
			{
				generateTestData(filename, FD->getName(), LHS_string, type, RHS_string);
			} else if (isNumeric(LHS_string) && !isNumeric(RHS_string) && isInParameters(RHS_string, FD->parameters(), type))
			{
				generateTestData(filename, FD->getName(), RHS_string, type, LHS_string);
			} else
			{
				llvm::outs() << "non-numeric condition\n";
			}*/

			//Print auxiliary ======================================================================
           	llvm::outs() << "Found SwitchStmt at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            llvm::outs() << " from function " << FD->getName() <<  "\n";
            //Print auxiliary ======================================================================


			
		}
	}
}
std::string ASTUTGen::convertExpressionToString(Expr *E, SourceManager &SM) {
  clang::LangOptions lopt;

  SourceLocation startLoc = E->getLocStart();
  SourceLocation _endLoc = E->getLocEnd();
  SourceLocation endLoc = clang::Lexer::getLocForEndOfToken(_endLoc, 0, SM, lopt);

  return std::string(SM.getCharacterData(startLoc), SM.getCharacterData(endLoc) - SM.getCharacterData(startLoc));
}

bool ASTUTGen::isInParameters(string name, ArrayRef<ParmVarDecl *> params, string& type)
{
	for(auto it : params)
	{
		if(it->getName() == name)
		{
			type = it->getOriginalType().getAsString();
			return true;
		}
	}

	return false;
}