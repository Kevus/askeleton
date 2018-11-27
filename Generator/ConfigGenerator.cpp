#include "ConfigGenerator.hpp"

bool fileExists(const std::string& filename)
{
  struct stat buffer;   
  return (stat (filename.c_str(), &buffer) == 0); 
}

/**
** Default values for generating config file.
** It can be seen that every value is a string,
** however, C++11 provide us with methods for
** transforming them.
**
** int: std::stoi( str )
** float: std::stof( str )
**
** more info: http://en.cppreference.com/w/cpp/string/basic_string/stol
**
**/
map<string, string> ConfigGenerator::defaultValues = 
{
	{"char", "a"},
	{"signed_char", "a"},
	{"unsigned_char", "a"},
	{"short", "0"},
	{"short_int", "0"},
	{"signed_short", "0"},
	{"signed_short_int", "0"},
	{"unsigned_short", "0"},
	{"unsigned_short_int", "0"},
	{"int", "0"},
	{"signed_int", "0"},
	{"unsigned", "0"},
	{"unsigned_int", "0"},
	{"long", "0"},
	{"long_int", "0"},
	{"signed_long", "0"},
	{"signed_long_int", "0"},
	{"unsigned_long", "0"},
	{"unsigned_long_int", "0"},
	{"long_long", "0"},
	{"long_long_int", "0"},
	{"signed_long_long", "0"},
	{"signed_long_long_int", "0"},
	{"unsigned_long_long", "0"},
	{"unsigned_long_long_int", "0"},
	{"float", "0.0"},
	{"double", "0.0"},
	{"long_double", "0.0"},
	{"string", "dflt"},
	{"std::string", "dflt"},
	{"std::__cxx11::string", "dflt"},
	{"bool", "false"},
	{"char_*", "a"},
	{"signed_char_*", "a"},
	{"unsigned_char_*", "a"},
	{"short_*", "0"},
	{"short_int_*", "0"},
	{"signed_short_*", "0"},
	{"signed_short_int_*", "0"},
	{"unsigned_short_*", "0"},
	{"unsigned_short_int_*", "0"},
	{"int_*", "0"},
	{"signed_int_*", "0"},
	{"unsigned_*", "0"},
	{"unsigned_int_*", "0"},
	{"long_*", "0"},
	{"long_int_*", "0"},
	{"signed_long_*", "0"},
	{"signed_long_int_*", "0"},
	{"unsigned_long_*", "0"},
	{"unsigned_long_int_*", "0"},
	{"long_long_*", "0"},
	{"long_long_int_*", "0"},
	{"signed_long_long_*", "0"},
	{"signed_long_long_int_*", "0"},
	{"unsigned_long_long_*", "0"},
	{"unsigned_long_long_int_*", "0"},
	{"float_*", "0.0"},
	{"double_*", "0.0"},
	{"long_double_*", "0.0"},
	{"string_*", "dflt"},
	{"std::string_*", "dflt"},
	{"std::__cxx11::string_*", "dflt"},
	{"bool_*", "false"}
};

ConfigGenerator::ConfigGenerator(string f_Name) :
	f_Name(f_Name)
{
	//We will create the file if it doesn't exist
	string sys_command = "mkdir -p Generated/UT/" + f_Name;
	system(sys_command.c_str());

	sys_command = "Generated/UT/" + f_Name + "/" + f_Name + ".cfg";
	bool file_exists = fileExists(sys_command);

	if (!file_exists)
		f_CommentHeader = getCommentHeader();
	else
		f_CommentHeader = "";

	//cfg_file.open(sys_command, (fileExists(sys_command) ? ios_base::app : ios_base::out));
	cfg_file.open(sys_command, ios_base::app);

	if(cfg_file.is_open())
		cfg_file << f_CommentHeader;
}

void ConfigGenerator::generateTestCase(string funct_name, map<string, string> param_type, vector<string> insert_order, string return_type)
{
	if(cfg_file.is_open())
	{
		cfg_file << funct_name << ":\n{\n"; 

		for(auto i : insert_order)
		{
			cfg_file << "\t" << i << "=" << defaultValues.find(param_type[i])->second << ";#" << param_type[i] << "\n";
		}//for

		cfg_file << "\treturn_" << return_type << "=" << defaultValues.find(return_type)->second << ";#" << return_type << "\n};\n\n";
	}//if
}

string ConfigGenerator::getCommentHeader()
{
	/**
	** Comment header, for visibility purposes
	**/
	stringstream buffer;

	// Time utilities
	auto t = time(nullptr);
	auto tm = *localtime(&t);

	// ASTUT Banner
	buffer << "#" << string(55, '/') << "\n"
		   << "#////" << string(47, ' ') << "////\n"
		   << "#//// AST-UT PROTOTYPE" << string(30, ' ') << "////\n"
		   << "#//// UNIVERSIDAD DE CADIZ - NAVANTIA SISTEMAS      ////\n"
		   << "#////" << string(47, ' ') << "////\n"
		   << "#" << string(55, '/') << "\n"
		   << "#File generated automatically by AST-UT.\n"
		   << "#File " << f_Name << ".cfg\n"
		   << "#Date " << put_time(&tm, "%d-%m-%Y %H:%M:%S") << "\n"
		   << "#" << string(55, '/') << "\n\n";

    //The banner should look like this:

    /**
    ** #///////////////////////////////////////////////////////
	** #////                                               ////
	** #//// AST-UT PROTOTYPE                              ////
	** #//// UNIVERSIDAD DE CADIZ - NAVANTIA SISTEMAS      ////
	** #////                                               ////
	** #///////////////////////////////////////////////////////
	** #File generated automatically by AST-UT.
	** #File <f_Name>.cfg
	** #Date dd-mm-yyyy hh:MM:ss
	** #///////////////////////////////////////////////////////
	**
	**/
	// *** END OF THE BANNER ***

	return buffer.str();
}

//##############################################################
BoostGenerator::BoostGenerator(string filePath, string cfgName, bool isFromClass) : isFromClass(isFromClass)
{
	// Time utilities
	auto t = time(nullptr);
	auto tm = *localtime(&t);
	ostringstream dateStream;

	valuesToChange.insert(pair<string, string>("{filePath}", filePath));
	valuesToChange.insert(pair<string, string>("{cfgName}", cfgName));

	dateStream << put_time(&tm, "%d-%m-%Y %H:%M:%S");

	valuesToChange.insert(pair<string, string>("{dateOfGeneration}", dateStream.str()));

	//==========================================================
	// These methods have not been implemented
	//==========================================================
	valuesToChange.insert(pair<string,string>("{includes}", ""));
	valuesToChange.insert(pair<string,string>("{namespaces}", ""));
	valuesToChange.insert(pair<string,string>("{readObject}", ""));
	valuesToChange.insert(pair<string,string>("{newMethods}", ""));
	//==========================================================
	//==========================================================

	if (isFromClass)
	{
		valuesToChange.insert(pair<string,string>("{className}", cfgName));
		valuesToChange.insert(pair<string,string>("{classNameTest}", cfgName + "_test;"));
	} else
	{
		valuesToChange.insert(pair<string,string>("{className}", ""));
		valuesToChange.insert(pair<string,string>("{classNameTest}", ""));
	}

	generateFixture("Generated/UT/" + cfgName + "/" + cfgName + "_fixture.hpp");
}

void BoostGenerator::generateFixture(string outputPath)
{
	if(!fileExists(outputPath))
	{
		string templatePath = "Generator/Templates/Fixture.tpl";
		string fileContent;

		ifstream tplFile (templatePath);
		ofstream outputFile (outputPath);

		if (tplFile.is_open())
		{
			//Read the entire template into memory
			fileContent = string( (istreambuf_iterator<char>(tplFile)),
		   				  istreambuf_iterator<char>() );

			for(auto i : valuesToChange)
				boost::replace_all(fileContent, i.first, i.second);
			
			outputFile << fileContent;

			outputFile.close();
			tplFile.close();
		}//if tpl
	}//if fileExist
}

void BoostGenerator::generateBoostAssert(string class_test, string function_name, map<string, string> param_type, vector<string> insertion_order, string return_type)
{
	string templatePath = "Generator/Templates/BoostTest.tpl";
	string outputPath = "Generated/UT/" + class_test + "/" + class_test + "_test.cpp";
	string fileContent;
	string ptype;

	stringstream test_case;

	bool existFlag = fileExists(outputPath);
	ifstream tplFile (templatePath);

	if (tplFile.is_open())
	{
		//If file doesnt exist, then replace common tags
		if(!existFlag)
		{
			fileContent = string( (istreambuf_iterator<char>(tplFile)),
						 		   istreambuf_iterator<char>() );

			boost::replace_all(fileContent, "{className}", class_test);

			//==========================================================
			// These tags are not used yet, so we will simply delete them
			//==========================================================
			boost::replace_all(fileContent, "{pointerInitToken}", "");
			boost::replace_all(fileContent, "{pointerDestroyToken}", "");
			//==========================================================
			//==========================================================

			//Copy makefile for compiling tests
			system(("cp -r Generator/Templates/makefile Generated/UT/" + class_test + "/").c_str());

		} else
		{
			ifstream tmp_output(outputPath);
			fileContent = string( (istreambuf_iterator<char>(tmp_output)),
						 		   istreambuf_iterator<char>() );
		}

		//Now we will create the assertion sentence
		test_case << "\tBOOST_CHECK_EQUAL(";
		if (isFromClass) test_case << class_test << "_test.";
		test_case << function_name << "(";

		for(auto i : insertion_order)
		{
			test_case << "Read_" << param_type[i] << "(\""
					  << function_name << "." << i << "\")";
			if (i != insertion_order.back()) test_case << ",";
		}

		//test_case << "));\n{assert}";
		test_case << "),Read_" << return_type << "(\"" << function_name << ".return_" << return_type << "\"));\n//{assert}";

		boost::replace_all(fileContent, "//{assert}", test_case.str());

		ofstream outputFile(outputPath);
		outputFile << fileContent;

		tplFile.close();
		outputFile.close();

	}

}