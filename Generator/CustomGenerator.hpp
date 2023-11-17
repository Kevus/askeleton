#ifndef CUSTOMGENERATOR_HPP
#define CUSTOMGENERATOR_HPP

#include "../auxiliary_functions.hpp"

#include <vector>
#include <map>

#include <stdio.h>
#include <experimental/filesystem>

using namespace std;

void initialize_file(string source);

class CustomGenerator
{
public:
	CustomGenerator(string filename = "ASKGeneratedTestsConfig");
	~CustomGenerator();

	void generateTypesFile(string funct_name, map<string, string> param_type, vector<string> insert_order, string return_type);
	void generateTestCasesFile(string funct_name, map<string, string> param_type, vector<string> insert_order, string return_type);
private:
	string filename;

	ofstream types_file;
	ofstream testcases_file;
};
/*
class CustomReader
{
public:

	void generateExecutableTests(string types_path, string values_path);
private:
	bool isFromClass;

	ofstream types_file;
	ofstream values_file;
};*/

#endif