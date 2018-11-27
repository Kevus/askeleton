#ifndef CONFIGGENERATOR_HPP
#define CONFIGGENERATOR_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iostream>
#include <fstream>

#include <iomanip>
#include <ctime>

#include <vector>
#include <map>

#include <stdio.h>
#include <experimental/filesystem>

//Boost libraries
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <sys/stat.h>

using namespace std;

bool fileExists(const std::string& filename);

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

	void generateTestCase(string funct_name, map<string, string> param_type, 
		vector<string> insert_order, string return_type);

private:
	string getCommentHeader();

	string f_Name;
	string f_CommentHeader;

	ofstream cfg_file;

	//Const default values. They will be deleted in future iterations, where
	//the default values will be calculated during execution.
	static map<string, string> defaultValues;
};

class BoostGenerator
{
public:
	BoostGenerator(string filePath, string cfgName, bool isFromClass);

	void generateBoostAssert(string class_test, string function_name, 
		map<string, string> param_type, vector<string> insertion_order, string return_type);

private:
	
	void generateFixture(string outputPath);
	map<string, string> valuesToChange;
	bool isFromClass;
};

#endif