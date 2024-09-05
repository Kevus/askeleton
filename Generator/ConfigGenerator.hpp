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
    explicit ConfigGenerator(const string &target);
    ConfigGenerator(const ConfigGenerator &) = default;

    ~ConfigGenerator() {
        if (cfg_file.is_open())
            cfg_file.close();
    }

    void generateTestCase(const string &functionName,
                          const vector<InfoVariable> &params,
                          const InfoType &returnType);
    void generateConstructorTest(const string &ctorName,
                                 const vector<InfoVariable> &params);

private:
    const string target, testFolder, configFilePath;
    ofstream cfg_file;

    void generateParams(const vector<InfoVariable> &params);
    void generateReturn(const InfoType &returnType);
    void generateParam(const InfoVariable &param);
    void generateParamRecord(const InfoVariable &record,
                             const string &prefix = "");
    void generateReturnRecord(const InfoType &record,
                              const string &prefix = "return_");

    void appendToConfigFile(const string &content) const;

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
