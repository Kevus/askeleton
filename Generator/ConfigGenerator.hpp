#ifndef CONFIGGENERATOR_HPP
#define CONFIGGENERATOR_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <iomanip>
#include <ctime>

#include <vector>
#include <map>

using namespace std;

class ConfigGenerator
{
public:
	/**
	** This constructor needs the container generated from the AST analysis.
	** We will also provide the name of the output file, but a default one
	** is set, for use it if necessary.
	**/
	ConfigGenerator(string f_Name = "ASTUTGeneratedTestsConfig");
	~ConfigGenerator(){
		if(cfg_file.is_open()) 
			cfg_file.close();
	}

	void generateTestCases();
	vector<string> getGenerated();

	void generateTestCase(string funct_name, map<string, string> param_type, string return_type);

private:
	string getCommentHeader();
	string deleteAllBeforeChar(string sToReplace, char cToFind);

	string f_Name;
	string f_CommentHeader;
	string f_TestCase;

	ofstream cfg_file;
	vector<string> generated_configs;

	//Const default values. They will be deleted in future iterations, where
	//the default values will be calculated during execution.
	static map<string, string> defaultValues;
};

#endif