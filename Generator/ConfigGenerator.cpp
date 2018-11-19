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
	{"char", ""},
	{"unsigned char", ""},
	{"signed char", ""},
	{"int", "1"},
	{"unsigned int", "1"},
	{"signed int", "1"},
	{"short int", "1"},
	{"unsigned short int", "1"},
	{"signed short int", "1"},
	{"long int", "1"},
	{"signed long int", "1"},
	{"unsigned long int", "1"},
	{"float", "1.0"},
	{"double", "1.0"},
	{"long double", "1.0"},
	{"std::string", ""},
	{"bool", "false"}
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

bool ConfigGenerator::fileExists(const std::string& filename)
{
  struct stat buffer;   
  return (stat (filename.c_str(), &buffer) == 0); 
}

string ConfigGenerator::deleteAllBeforeChar(string sToReplace, char cToFind)
{

	if ( sToReplace.find(cToFind) != string::npos )
		sToReplace = sToReplace.substr(sToReplace.find_last_of(cToFind) + 1, sToReplace.size());

	return sToReplace;

}

void ConfigGenerator::generateTestCase(string funct_name, map<string, string> param_type, string return_type)
{
	if(cfg_file.is_open())
	{
		cfg_file << funct_name << ":\n{\n";

		for(auto i : param_type)
		{
			cfg_file << "\t" << i.first << "=" << i.second << ",\n";
		}//for

		cfg_file << "\treturn_" << return_type << "=0\n}\n\n";
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