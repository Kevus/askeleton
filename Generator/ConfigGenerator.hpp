#ifndef CONFIGGENERATOR_HPP
#define CONFIGGENERATOR_HPP

#include "../auxiliary_functions.hpp"
#include "VariableInfo.hpp"

#include <map>
#include <vector>

#include <filesystem>
#include <stdio.h>

#include "RandomValuesGenerator.hpp"

using namespace std;

class ConfigGenerator {
public:
    ConfigGenerator(string f_Name = "ASKGeneratedTestsConfig");
    ~ConfigGenerator() {
        if (cfg_file.is_open())
            cfg_file.close();
    }

    vector<string> getGenerated();

    void generateTestCase(const string &functionName,
                          const vector<InfoVariable> &params,
                          const InfoType &returnType);

	void generateConstructorTest(const string &ctorName,
								const vector<InfoVariable> &params);

private:
    string f_Name;

    ofstream cfg_file;

    // Const default values. They will be deleted in future iterations, where
    // the default values will be calculated during execution.
    static map<string, string> defaultValues;

    // TEST
    RandomValuesGenerator rvg;
};

/*class TestDataGenerator
{
public:
    TestDataGenerator(string outputPath);



private:
    void createFile(string outputPath);

    map<string, string> method_value; //map<"method.param.type", "value">
};*/

#endif
