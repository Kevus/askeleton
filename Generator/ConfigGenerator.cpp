#include "ConfigGenerator.hpp"

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
};


ConfigGenerator::ConfigGenerator(string f_Name) :
	f_Name(f_Name)
{
	string comment_header = "";

	//We will create the file if it doesn't exist
	string sys_command = "mkdir -p Generated/UT/" + f_Name;
	system(sys_command.c_str());

	sys_command = "Generated/UT/" + f_Name + "/" + f_Name + ".cfg";
	bool file_exists = fileExists(sys_command);

	if (!file_exists)
		comment_header = getCommentHeader(f_Name);

	//cfg_file.open(sys_command, (fileExists(sys_command) ? ios_base::app : ios_base::out));
	cfg_file.open(sys_command, ios_base::app);

	if(cfg_file.is_open())
		cfg_file << comment_header;
}

void ConfigGenerator::generateTestCase(string funct_name, map<string, string> param_type, vector<string> insert_order, string return_type)
{
	if(cfg_file.is_open())
	{
		cfg_file << funct_name << ":\n{\n"; 

		for(auto i : insert_order)
		{
			//cfg_file << "\t" << i << "=" << defaultValues.find(param_type[i])->second << ";#" << param_type[i] << "\n";
			cfg_file << "\t" << i << "=" << rvg.getRandomValue(param_type[i]) << ";#" << param_type[i] << "\n";
		}//for

		cfg_file << "\treturn_" << return_type << "=" << rvg.getRandomValue(return_type) << ";#" << return_type << "\n};\n\n";
	}//if
}

void ConfigGenerator::generateConstructorTest(string constructor_name, map<string, string> param_type, vector<string> insert_order)
{
	if(cfg_file.is_open())
	{
		cfg_file << constructor_name << ":\n{\n";

		for(auto i : insert_order)
			cfg_file << "\t" << i << "=" << rvg.getRandomValue(param_type[i]) << ";#" << param_type[i] << "\n";

		cfg_file << "};\n\n";
	}
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
	//valuesToChange.insert(pair<string,string>("{readObject}", ""));
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
	fixture_path = "Generated/UT/" + cfgName + "/" + cfgName + "_fixture.hpp";

	generateFixture(fixture_path);

	if(defaultTypes.size() == 0)
		defaultTypes = fillDefaultTypes("Config/DefaultTypes");
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

void BoostGenerator::generateBoostAssert(string class_test, string function_name, string function_cfg_name, map<string, string> param_type, vector<string> insertion_order, string return_type)
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

		//List, vector and map will have a different test case
		bool return_container = (return_type.find("list") != string::npos ||
							     return_type.find("vector") != string::npos ||
							     return_type.find("map") != string::npos);

		//Lets check pointers...
		for(auto i : insertion_order)
		{
			checkTypes(param_type[i]);

			if(param_type[i].find("_&") != string::npos ||
			   param_type[i].find("const_") != string::npos)
			{
				ptype = param_type[i];

				boost::replace_all(ptype, "_", " "); //delete '_' for using it as a type
				boost::replace_all(ptype, " &", ""); //Then delete const and/or '&'

				test_case << "\n\t" << ptype << " ";

				boost::replace_all(ptype, " ", "_");
				boost::replace_all(ptype, "const_", "");
				boost::replace_all(ptype, "*", "s");

				test_case << function_cfg_name << "_" << i << " = Read_" << ptype
						  << "(\"" << function_cfg_name << "." << i << "\");\n"; 

			}
		}

		checkTypes(return_type);
		if(return_type.find("_&") != string::npos ||
			return_type.find("const_") != string::npos)
		{
			ptype = return_type;
			boost::replace_all(ptype, "_", " "); 
			boost::replace_all(ptype, " &", "");

			test_case << "\t" << ptype << " return_" << function_cfg_name;

			boost::replace_all(ptype, " ", "_");
			boost::replace_all(ptype, "const_", "");
			boost::replace_all(ptype, "*", "s");

			test_case << " = Read_" << ptype << "(\"" << function_cfg_name << ".return_" << return_type << "\");\n";
		}

		//Now we will create the assertion sentence
		if(!return_container)
			test_case << "\tBOOST_CHECK_EQUAL(";
		else
			test_case << "\tBOOST_CHECK((";

		if (isFromClass) 
			test_case << class_test << "_test.";
		
		test_case << function_name << "(";


		for(auto i : insertion_order)
		{
			if(param_type[i].find("_&") == string::npos && param_type[i].find("const_") == string::npos)
			{
				boost::replace_all(param_type[i], "*", "s");
				test_case << "Read_" << param_type[i] << "(\""
						  << function_cfg_name << "." << i << "\")";
			} else
			{
				test_case << function_cfg_name << "_" << i;
			}

			if (i != insertion_order.back()) test_case << ",";
		}

		//test_case << "));\n{assert}";
		if(!return_container)
			test_case << "),";
		else
			test_case << ") == ";

		if(return_type.find("_&") == string::npos && return_type.find("const_") == string::npos)
		{
			string read = "Read_" + return_type;
			boost::replace_all(read, "*", "s");
			test_case << read << "(\"" << function_name << ".return_" << return_type << "\"))";
		}
		else {
			test_case << "return_" << function_cfg_name << ")";
		}

		if(!return_container)
			test_case << ";";
		else 
			test_case << ");";

		test_case << "\n//{assert}";

		boost::replace_all(fileContent, "//{assert}", test_case.str());

		ofstream outputFile(outputPath);
		outputFile << fileContent;

		tplFile.close();
		outputFile.close();

	}

}

void BoostGenerator::generateBoostConstructorAssert(string class_test, string constructor_name, string constructor_cfg_name, map<string, string> param_type, vector<string> insertion_order)
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
		test_case << "\tBOOST_CHECK(new ";
		test_case << constructor_name << "(";

		for(auto i : insertion_order)
		{
			checkTypes(param_type[i]);

			if(param_type[i].find("_&") == string::npos && param_type[i].find("const_") == string::npos)
			{
				test_case << "Read_" << param_type[i] << "(\""
						  << constructor_cfg_name << "." << i << "\")";
			} else
			{
				test_case << constructor_cfg_name << "_" << i;
			}

			if (i != insertion_order.back()) test_case << ",";
		}

		test_case << "));\n//{assert}";
		//test_case << "),Read_" << return_type << "(\"" << function_name << ".return_" << return_type << "\"));\n//{assert}";

		boost::replace_all(fileContent, "//{assert}", test_case.str());

		ofstream outputFile(outputPath);
		outputFile << fileContent;

		tplFile.close();
		outputFile.close();

	}
}

void BoostGenerator::addStructReadToFixture(string type_name, map<string, string> param_type, vector<string> insertion_order, bool overloaded_eq, bool overloaded_flux)
{
	bool existFlag = fileExists(fixture_path);

	if(!existFlag)
		generateFixture(fixture_path);

	ifstream fixture_file(fixture_path);
	string fileContent = string( (istreambuf_iterator<char>(fixture_file)),
						 		   istreambuf_iterator<char>() );
	

	/**
	** EXAMPLE
	**
	** customType Read_struct_customType(string objectKey)
	{
		string object = readObject(objectKey);
		vector<string> values;

		//1,2,3,4
		auto delimiter = object.find(",");
		while(delimiter != string::npos)
		{
			auto key = object.substr(0, delimiter);
			object = object.substr(delimiter + 1);

			values.push_back(key);

			delimiter = object.find(",");
		}

		int number = boost::lexical_cast<int>(values[0]);
		string name = boost::lexical_cast<string>(values[1]);

		customType result;
		result.number = number;
		result.name = name;

		return result;
	}
	**
	**/

	stringstream overloaded_operators;
	stringstream read_method;

	read_method << type_name << " Read_"
				<< "struct_" << type_name << "(string objectKey)\n\t{\n"
				<< "\t\tstring object = readObject(objectKey);\n"
				<< "\t\tboost::replace_all(object, \";\", \"\");\n"
				<< "\t\tvector<string> values;\n\n"
				<< "\t\tauto delimiter = object.find(\",\");\n"
				<< "\t\twhile( delimiter != string::npos )\n\t\t{\n"
				<< "\t\t\tauto key = object.substr(0, delimiter);\n"
				<< "\t\t\tobject = object.substr(delimiter + 1);\n\n"
				<< "\t\t\tvalues.push_back(key);\n\n"
				<< "\t\t\tdelimiter = object.find(\",\");\n\t\t}\n"
				<< "\t\tvalues.push_back(object);\n\n";

	//----
	read_method << "\t\t" << type_name << " result;\n";
	//----
	int pos = 0;
	int size = insertion_order.size();

	read_method << "\t\tif( values.size() >= " << size << ")\n\t\t{\n";
	for(auto i : insertion_order)
	{
		//read_method << "\t\t" << param_type[i] << " " << i << " = ";
		checkTypes(param_type[i]);
		read_method << "\t\t\tresult." << i << " = ";

		if(param_type[i] != "string")
		{
			read_method	<< "boost::lexical_cast<" << param_type[i] << ">(values["
						<< pos << "]);\n";
		} else
		{
			read_method << "values[" << pos << "];\n";
		}

		
		pos++;
	}
	read_method << "\t\t}\n";

	read_method << "\n\t\treturn result;\n\t}\n\t//{readObject}";

	boost::replace_all(fileContent, "//{readObject}", read_method.str());

	//Now ovearload operators if needed
	if(!overloaded_eq)
	{
		overloaded_operators << "bool operator==(const "
							 << type_name << "& a, const "
							 << type_name << "& b)\n{\n"
							 << "\tbool result = true;\n";

		int size = insertion_order.size();
		if(size > 0)
		{
			overloaded_operators << "\tresult = (";
			pos = 1;
			for(auto i : insertion_order)
			{
				overloaded_operators << "a." << i << " == b." << i;

				if(pos != size)
				{
					overloaded_operators << ") && \n\t\t(";
				} else 
				{
					overloaded_operators << ");";
				}

				pos++;
			}
		}

		overloaded_operators << "\n\treturn result;\n}\n\n";

		if(overloaded_flux)
		{
			overloaded_operators << "//{overloadOperator}";
			boost::replace_all(fileContent, "//{overloadOperator}", overloaded_operators.str());
		}
		
	}

	if(!overloaded_flux)
	{
		overloaded_operators << "ostream& operator<<(ostream& stream, "
							 << "const " << type_name << "& a)\n{\n";

		int size = insertion_order.size();
		if(size > 0)
		{
			for(auto i : insertion_order)
			{
				overloaded_operators << "\tstream << a." << i << " << endl;\n";
			}
		}

		overloaded_operators << "\n\treturn stream;\n}\n\n//{overloadOperator}";

		boost::replace_all(fileContent, "//{overloadOperator}", overloaded_operators.str());
	}

	ofstream outputFile(fixture_path);
	outputFile << fileContent;

	fixture_file.close();
	outputFile.close();

	defaultTypes.push_back(type_name);
}

void BoostGenerator::addNewTypeToFixture(string type_name, string fixture_path)
{
	/*** EXAMPLE ***
	 ** <formatted_type> Read_<type_name>(string objectKey)
	 ** {
	 **		<formatted_type> result = boost::lexical_cast<<formatted_type>>(result);
	 **		return result;
	 **	}
	 ***************/
	bool existFlag = fileExists(fixture_path);

	if(!existFlag)
		generateFixture(fixture_path);

	ifstream fixture_file(fixture_path);
	string fileContent = string( (istreambuf_iterator<char>(fixture_file)),
						 		   istreambuf_iterator<char>() );

	string formatted_type;
	stringstream read_method;

	/*boost::replace_all(type_name, "_&", "");
	boost::replace_all(type_name, "const_", "");*/

	formatted_type = type_name;

	//boost::replace_all(formatted_type, "_*", "");
	boost::replace_all(formatted_type, "_", " ");

	if(type_name.find("*") == string::npos)
	{
		read_method << formatted_type << " Read_" << type_name << "(string objectKey)\n\t{\n\t\t"
					<< formatted_type << " result = boost::lexical_cast<" << formatted_type << ">(result);\n"
					<< "\t\treturn result;\n\t}\n"
					<< "\t//{readObject}";
	}else {
		boost::replace_all(type_name, "*", "s");

		read_method << formatted_type << " Read_" << type_name << "(string objectKey)\n\t{\n\t\t"
					<< formatted_type << " result = Read_";

		boost::replace_all(formatted_type, "*", "s");
		boost::replace_all(formatted_type, " ", "_");

		read_method << formatted_type << "(objectKey);\n"
		            << "\t\treturn result;\n\t}\n"
					<< "\t//{readObject}";
	}
		

	boost::replace_all(fileContent, "//{readObject}", read_method.str());

	ofstream outputFile(fixture_path);
	outputFile << fileContent;

	fixture_file.close();
	outputFile.close();
	
}

vector<string> BoostGenerator::fillDefaultTypes(string path)
{
	ifstream types_file(path);
	vector<string> result;
	
	if(types_file.is_open())
	{
		string line;
		while(getline(types_file, line))
		{
			result.push_back(line);
		}
		types_file.close();
	}

	return result;

}

void BoostGenerator::checkTypes(string type)
{
	boost::replace_all(type, "_&", "");
	boost::replace_all(type, "const_", "");

	cout << defaultTypes.size() << endl;

	if(find(defaultTypes.begin(), defaultTypes.end(), type) == defaultTypes.end())
	{
		if(type.find("struct_") == string::npos && type.find("char") == string::npos)
		{
			addNewTypeToFixture(type, fixture_path);
			defaultTypes.push_back(type);
		}
	}
}
