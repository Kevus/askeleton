#include "ConfigGenerator.hpp"

ConfigGenerator::ConfigGenerator(string f_Name) :
	f_Name(f_Name)
{
	//We will create the file if it doesn't exist
	string sys_command = "mkdir -p Generated/UT/" + f_Name;
	system(sys_command.c_str());

	cfg_file.open("Generated/UT/" + f_Name + "/" + f_Name + ".cfg");

	if(cfg_file.is_open())
	{
		f_CommentHeader = getCommentHeader();
		cfg_file << f_CommentHeader;
	}
}

void ConfigGenerator::generateTestCase(string funct_name, map<string, string> param_type, string return_type)
{
	if(cfg_file.is_open())
	{
		cfg_file << funct_name << ":\n{\n";
		int stop_at = param_type.size();
		int current_it = 0;

		if(stop_at == 0)
		{
			cfg_file << "\treturn_" << return_type << "=0\n}\n\n";
		}else
		{
			for(auto i : param_type)
			{

				if(current_it < stop_at)
				{
					cfg_file << "\t" << i.first << "=" << i.second << ",\n";
				}
				else{
					cfg_file << "\t" << return_type << "=0\n}\n\n";
				}

				current_it++;
			}//for
		}//if-else
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