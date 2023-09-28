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
