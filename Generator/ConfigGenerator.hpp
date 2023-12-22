#ifndef CONFIGGENERATOR_HPP
#define CONFIGGENERATOR_HPP

#include "../auxiliary_functions.hpp"

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

    void generateTestCases();
    vector<string> getGenerated();

    void generateTestCase(string funct_name, map<string, string> param_type,
                          vector<string> insert_order, string return_type);
    void generateTestCase(string funct_name,
                          const map<string, pair<string, string>> &param_type,
                          const vector<string> &insert_order,
                          const pair<string, string> &return_type);

    void generateConstructorTest(string constructor_name,
                                 map<string, string> param_type,
                                 vector<string> insert_order);

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
