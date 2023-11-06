#include "ASTUTGen.hpp"

void ASTUTGen::run(const MatchFinder::MatchResult &Result)
{
	apply_FD1(Result);
	apply_MD1(Result);
	apply_CT1(Result); // Necessary for structs and classes
	apply_CC1(Result);

	// Kevin: dejamos estos fuera, nos interesa ahora solo las funciones, los datos vendrán por KLEE
	// apply_DG1(Result);
	// apply_DG2(Result);
}

// Method outside classes
void ASTUTGen::apply_FD1(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const FunctionDecl *UT = Result.Nodes.getNodeAs<clang::FunctionDecl>("FD1"))
	{

		FullSourceLoc FullLocation;

		FullLocation = Context->getFullLoc(UT->getBeginLoc());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation))
		{

			// In this case, we do not want class functions
			if (!isa<CXXMethodDecl>(UT))
			{

				// Get the file name
				string source_file = Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
				unsigned first = source_file.find_last_of('/') + 1;
				unsigned last = source_file.find_last_of('.');

				string filename = source_file.substr(first, last - first);

				// TO-DO: MODIFICAR PARA AÑADIR MAS O MENOS FRAMEWORKS
				BoostGenerator bGen(source_file, filename, false);

				generateFunctionTest(filename,
									 UT->getName().str(),
									 UT->parameters(),
									 UT->getReturnType().getAsString(),
									 bGen);
				// Print auxiliary ======================================================================
				llvm::outs() << "Found FunctionDecl at "
							 << FullLocation.getSpellingLineNumber() << ":"
							 << FullLocation.getSpellingColumnNumber() << " - ";

				llvm::outs() << UT->getNameInfo().getAsString() << " in file " << filename << "\n";
				// Print auxiliary ======================================================================
			}
		}
	}
}

void ASTUTGen::apply_MD1(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const CXXMethodDecl *UT = Result.Nodes.getNodeAs<clang::CXXMethodDecl>("MD1"))
	{

		FullSourceLoc FullLocation;

		FullLocation = Context->getFullLoc(UT->getBeginLoc());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation))
		{

			// In this case, we do not want class constructors
			if (!isa<CXXConstructorDecl>(UT))
			{
				string source_file = Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
				string parentname = UT->getParent()->getName().str();

				// TO-DO: MODIFICAR PARA AÑADIR MAS O MENOS FRAMEWORKS
				BoostGenerator bGen(source_file, parentname, true);

				generateFunctionTest(parentname,
									 UT->getName().str(),
									 UT->parameters(),
									 UT->getReturnType().getAsString(),
									 bGen);

				// Print auxiliary ======================================================================
				llvm::outs() << "Found CxxMethodDecl at "
							 << FullLocation.getSpellingLineNumber() << ":"
							 << FullLocation.getSpellingColumnNumber() << " - ";

				llvm::outs() << UT->getNameInfo().getAsString() << " from class " << parentname << "\n";
				// Print auxiliary ======================================================================
			}
		}
	}
}

void ASTUTGen::apply_CT1(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const CXXRecordDecl *UT = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("CT1"))
	{

		FullSourceLoc FullLocation;

		FullLocation = Context->getFullLoc(UT->getBeginLoc());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation))
		{

			// Get the file name
			string source_file = Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
			unsigned first = source_file.find_last_of('/') + 1;
			unsigned last = source_file.find_last_of('.');

			string filename = source_file.substr(first, last - first);

			// TO-DO: MODIFICAR PARA AÑADIR MAS O MENOS FRAMEWORKS
			BoostGenerator bGen(source_file, filename, false);

			// We'll read the fields here
			vector<FieldDecl *> field_decl;

			for (auto i : UT->fields())
				field_decl.push_back(i);

			bool overloadedEq = false;
			bool overloadedFlux = false;
			for (auto i : UT->methods())
			{
				// llvm::outs() << i->getNameAsString() << "\n";
				if (i->getNameAsString().find("operator==") != string::npos)
					overloadedEq = true;
				else if (i->getNameAsString().find("operator<<") != string::npos)
					overloadedFlux = true;
			}

			generateCustomTypeFixture(filename,
									  // UT->getNameAsString(),
									  UT->getQualifiedNameAsString(),
									  field_decl,
									  overloadedEq,
									  overloadedFlux,
									  bGen);

			// addReadTypeToFixture(string type_name, map<string, string> param_type, vector<string> insertion_order)

			// Print auxiliary ======================================================================
			llvm::outs() << "Found CXXRecordDecl (struct-customtype) at "
						 << FullLocation.getSpellingLineNumber() << ":"
						 << FullLocation.getSpellingColumnNumber() << " - ";

			llvm::outs() << UT->getNameAsString() << " in file " << filename << "\n";
			// Print auxiliary ======================================================================
		}
	}
}

void ASTUTGen::apply_CC1(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const CXXConstructorDecl *UT = Result.Nodes.getNodeAs<clang::CXXConstructorDecl>("CC1"))
	{

		FullSourceLoc FullLocation;

		FullLocation = Context->getFullLoc(UT->getBeginLoc());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation))
		{

			string source_file = Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
			string parentname = UT->getParent()->getName().str();

			// TO-DO: MODIFICAR PARA AÑADIR MAS O MENOS FRAMEWORKS
			BoostGenerator bGen(source_file, parentname, true);

			generateConstructorTest(
				parentname,
				parentname,
				UT->parameters(),
				bGen);

			// Print auxiliary ======================================================================
			llvm::outs() << "Found CXXConstructorDecl at "
						 << FullLocation.getSpellingLineNumber() << ":"
						 << FullLocation.getSpellingColumnNumber() << " - ";

			llvm::outs() << UT->getNameInfo().getAsString() << " from class " << parentname << "\n";
			// Print auxiliary ======================================================================
		}
	}
}

void ASTUTGen::apply_PD1(const MatchFinder::MatchResult &Result)
{
	// TO-DO: make this SHOW when a private member is called
}

void ASTUTGen::apply_DG1(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const BinaryOperator *UT = Result.Nodes.getNodeAs<clang::BinaryOperator>("DG1"))
	{

		const FunctionDecl *FD = Result.Nodes.getNodeAs<clang::FunctionDecl>("DG1b");
		FullSourceLoc FullLocation;

		FullLocation = Context->getFullLoc(UT->getBeginLoc());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation))
		{

			string LHS_string = convertExpressionToString(UT->getLHS(), Context->getSourceManager());
			string RHS_string = convertExpressionToString(UT->getRHS(), Context->getSourceManager());
			string LHS_type = UT->getLHS()->getType().getAsString();
			string RHS_type = UT->getRHS()->getType().getAsString();

			string source_file = Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
			unsigned first = source_file.find_last_of('/') + 1;
			unsigned last = source_file.find_last_of('.');
			string filename = source_file.substr(first, last - first);

			string type;

			if (!isNumeric(LHS_string) && isNumeric(RHS_string) && isInParameters(LHS_string, FD->parameters(), type))
			{
				generateTestData(filename, FD->getName().str(), LHS_string, type, RHS_string);
			}
			else if (isNumeric(LHS_string) && !isNumeric(RHS_string) && isInParameters(RHS_string, FD->parameters(), type))
			{
				generateTestData(filename, FD->getName().str(), RHS_string, type, LHS_string);
			}
			else
			{
				llvm::outs() << "non-numeric condition\n";
			}

			// Print auxiliary ======================================================================
			/*llvm::outs() << "Found BinaryOperator at "
						 << FullLocation.getSpellingLineNumber() << ":"
						 << FullLocation.getSpellingColumnNumber() << " - ";

			llvm::outs() << " from function " << FD->getName().str() <<  "\n";*/
			// Print auxiliary ======================================================================
		}
	}
}

void ASTUTGen::apply_DG2(const MatchFinder::MatchResult &Result)
{
	ASTContext *Context = Result.Context;

	if (const SwitchStmt *UT = Result.Nodes.getNodeAs<clang::SwitchStmt>("DG2"))
	{

		const FunctionDecl *FD = Result.Nodes.getNodeAs<clang::FunctionDecl>("DG2b");
		FullSourceLoc FullLocation;

		FullLocation = Context->getFullLoc(UT->getBeginLoc());

		if (FullLocation.isValid() && !Context->getSourceManager().isInSystemHeader(FullLocation))
		{

			string source_file = Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
			unsigned first = source_file.find_last_of('/') + 1;
			unsigned last = source_file.find_last_of('.');
			string filename = source_file.substr(first, last - first);

			// string test = UT->getCond()->getAsString();
			// llvm::outs() << "test: " << test << "\n";
			// string cname = UT->getCond()->getName().str();
			// string ctype = UT->getCond()->getType().getAsString();

			// llvm::outs() << /*"Cname: " << cname << */" Ctype: " << ctype << "\n";

			/*string LHS_string = convertExpressionToString(UT->getLHS(), Context->getSourceManager());
			string RHS_string = convertExpressionToString(UT->getRHS(), Context->getSourceManager());
			string LHS_type = UT->getLHS()->getType().getAsString();
			string RHS_type = UT->getRHS()->getType().getAsString();

			//llvm::outs() << "LHS: " << LHS_string << " " << LHS_type << " - RHS: " << RHS_string << " " << RHS_type << "\n";

			string source_file = Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
			unsigned first = source_file.find_last_of('/') + 1;
			unsigned last = source_file.find_last_of('.');
			string filename = source_file.substr(first, last-first);

			string type;
			if(!isNumeric(LHS_string) && isNumeric(RHS_string) && isInParameters(LHS_string, FD->parameters(), type))
			{
				generateTestData(filename, FD->getName().str(), LHS_string, type, RHS_string);
			} else if (isNumeric(LHS_string) && !isNumeric(RHS_string) && isInParameters(RHS_string, FD->parameters(), type))
			{
				generateTestData(filename, FD->getName().str(), RHS_string, type, LHS_string);
			} else
			{
				llvm::outs() << "non-numeric condition\n";
			}*/

			// Print auxiliary ======================================================================
			llvm::outs() << "Found SwitchStmt at "
						 << FullLocation.getSpellingLineNumber() << ":"
						 << FullLocation.getSpellingColumnNumber() << " - ";

			llvm::outs() << " from function " << FD->getName().str() << "\n";
			// Print auxiliary ======================================================================
		}
	}
}

// General method for testing functions
void ASTUTGen::generateFunctionTest(string source_file, string function_name, ArrayRef<ParmVarDecl *> parameters, string return_type, BoostGenerator bGen)
{

	ConfigGenerator cfg_gen(source_file);
	string function_cfg_name = function_name;

	// if( function_occurrences.find(function_name) == function_occurrences.end() )
	//	function_occurences.insert( pair<string, int> (function_name, 1));
	function_occurrences[function_name]++;

	if (function_occurrences[function_name] > 1)
		function_cfg_name += "_" + to_string(function_occurrences[function_name]);

	// Get the parameters
	map<string, string> param_type;
	vector<string> insert_order;

	string rtn_type = cleanUnnecesaryChars(return_type);
	string tmp_type;
	string tmp_name;

	int noname_count = 0;

	for (auto i : parameters)
	{
		tmp_type = i->getOriginalType().getAsString();
		tmp_name = i->getQualifiedNameAsString();

		// TEST: check if this solves anything
		/*if(tmp_name == "")
			abort_test = true;*/
		if (tmp_name == "")
		{
			tmp_name = tmp_type + "_" + to_string(noname_count);
			noname_count++;
		}

		tmp_type = cleanUnnecesaryChars(tmp_type);

		param_type.insert(pair<string, string>(tmp_name, tmp_type));
		insert_order.push_back(tmp_name);
	}

	/*CustomGenerator cgen(source_file);
	cgen.generateTypesFile(function_name, param_type, insert_order, rtn_type);
	cgen.generateTestCasesFile(function_name, param_type, insert_order, rtn_type);*/

	// if(!abort_test)
	//{
	cfg_gen.generateTestCase(function_cfg_name, param_type, insert_order, rtn_type);
	bGen.generateBoostAssert(source_file, function_name, function_cfg_name, param_type, insert_order, rtn_type);
	//}
}

// Method for constructing constructor test
void ASTUTGen::generateConstructorTest(string source, string constructor_name, ArrayRef<ParmVarDecl *> parameters, BoostGenerator bGen)
{
	ConfigGenerator cfg_gen(source);

	string constructor_cfg_name = constructor_name;
	function_occurrences[constructor_name]++;

	if (function_occurrences[constructor_name] > 1)
		constructor_cfg_name += "_" + to_string(function_occurrences[constructor_name]);

	// Get the parameters
	map<string, string> param_type;
	vector<string> insert_order;

	string tmp_type;
	string tmp_name;

	int noname_count = 0;

	for (auto i : parameters)
	{
		tmp_type = i->getOriginalType().getAsString();
		tmp_type = cleanUnnecesaryChars(tmp_type);

		tmp_name = i->getQualifiedNameAsString();

		if (tmp_name == "")
		{
			tmp_name = tmp_type + "_" + to_string(noname_count);
			noname_count++;
		}

		param_type.insert(pair<string, string>(tmp_name, tmp_type));
		insert_order.push_back(tmp_name);
	}

	/**
	** We will add custom generator lates
	**/
	cfg_gen.generateConstructorTest(constructor_cfg_name, param_type, insert_order);
	bGen.generateBoostConstructorAssert(source, constructor_name, constructor_cfg_name, param_type, insert_order);
}

void ASTUTGen::generateCustomTypeFixture(string source, string type_name, vector<FieldDecl *> parameters, bool overloadedEq, bool overloadedFlux, BoostGenerator bGen)
{
	ConfigGenerator cfg_gen(source);

	// Get the parameters
	map<string, string> param_type;
	vector<string> insert_order;

	string tmp_type;
	string tmp_name;

	int noname_count = 0;

	for (auto i : parameters)
	{
		tmp_type = i->getType().getAsString();
		tmp_type = cleanUnnecesaryChars(tmp_type);

		tmp_name = i->getNameAsString();

		if (tmp_name == "")
		{
			tmp_name = tmp_type + "_" + to_string(noname_count);
			noname_count++;
		}

		param_type.insert(pair<string, string>(i->getNameAsString(), tmp_type));
		insert_order.push_back(i->getNameAsString());
	}

	bGen.addStructReadToFixture(type_name, param_type, insert_order, overloadedEq, overloadedFlux);
}

void ASTUTGen::generateTestData(string source, string function_name, string param, string type, string value)
{
	// function.value=a,b,c,d,e
	map<string, vector<string>> function_values;
	string outputPath = "Generated/UT/" + source + "/" + source + "_data.txt";
	vector<string> values;
	string fileContent;

	ifstream tmp_output(outputPath);
	for (string line; getline(tmp_output, line);)
	{
		// for each line...
		auto delimiter = line.find(":");

		string fvalue = line.substr(0, delimiter);
		line = line.substr(delimiter + 1);

		delimiter = line.find(",");
		while (delimiter != string::npos)
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

	if (function_values.find(function_name + "." + param) != function_values.end())
	{
		values = function_values.at(function_name + "." + param);
	}

	vector<string> realvalues = obtainTestData(type, value);
	for (auto it : realvalues)
		values.push_back(it);

	function_values[(function_name + "." + param)] = values;

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
}

vector<string> ASTUTGen::obtainTestData(string type, string value)
{
	vector<string> result;
	boost::replace_all(value, "\'", "");
	boost::replace_all(value, "\"", "");
	result.push_back(value);

	// Although this looks repetitive, the lexical_cast<string> is necessary like this
	// because res is a different type depending on the conditional
	if (type.find("bool") != string::npos || type.find("_Bool") != string::npos)
	{
		result.push_back(
			(value == "true") ? "false" : "true");
	}
	else if (type.find("string") != string::npos)
	{
		result.push_back(value + "_another");
	}
	else if (type.find("char") != string::npos)
	{
		char res = boost::lexical_cast<char>(value);
		result.push_back(boost::lexical_cast<string>(res + 1));
		result.push_back(boost::lexical_cast<string>(res - 1));
	}
	else if (type.find("int") != string::npos)
	{
		int res = boost::lexical_cast<int>(value);
		result.push_back(boost::lexical_cast<string>(res + 1));
		result.push_back(boost::lexical_cast<string>(res - 1));
	}
	else if (type.find("double") != string::npos)
	{
		double res = boost::lexical_cast<double>(value);
		result.push_back(boost::lexical_cast<string>(res + 1));
		result.push_back(boost::lexical_cast<string>(res - 1));
	}
	else if (type.find("float") != string::npos)
	{
		float res = boost::lexical_cast<float>(value);
		result.push_back(boost::lexical_cast<string>(res + 1));
		result.push_back(boost::lexical_cast<string>(res - 1));
	}

	return result;
}
