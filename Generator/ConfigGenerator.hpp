#ifndef CONFIGGENERATOR_HPP
#define CONFIGGENERATOR_HPP

#include "../auxiliary_functions.hpp"

#include <vector>
#include <map>

#include <stdio.h>
#include <experimental/filesystem>

#include "RandomValuesGenerator.hpp"

using namespace std;

class ConfigGenerator
{
public:

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

	string f_Name;

	ofstream cfg_file;

	//Const default values. They will be deleted in future iterations, where
	//the default values will be calculated during execution.
	static map<string, string> defaultValues;

	//TEST
	RandomValuesGenerator rvg;
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