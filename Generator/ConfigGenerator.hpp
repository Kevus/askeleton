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

    void generateTestCase(const string &functionName,
                          const vector<InfoVariable> &params,
                          const InfoType &returnType) const;
    void generateConstructorTest(const string &ctorName,
                                 const vector<InfoVariable> &params) const;

private:
    const string target, testFolder, configFilePath;

    std::string generateParams(const vector<InfoVariable> &params) const;
    std::string generateReturn(const InfoType &returnType) const;
    std::string generateParam(const InfoVariable &param) const;
    std::string generateParamRecord(const InfoVariable &record,
                                    const string &prefix = "") const;
    std::string generateReturnRecord(const InfoType &record,
                                     const string &prefix = "return_") const;

    void appendToConfigFile(const string &content) const;

    // TEST
    static RandomValuesGenerator rvg;
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
