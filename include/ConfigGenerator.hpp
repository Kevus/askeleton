#pragma once

#include <map>
#include <string>
#include <vector>

#include "Config.hpp"
#include "RandomValuesGenerator.hpp"
#include "VariableInfo.hpp"

class ConfigGenerator {
public:
    explicit ConfigGenerator(const std::string &target);
    ConfigGenerator(const ConfigGenerator &) = default;

    void generateTestCase(const std::string &functionName,
                          const std::vector<InfoVariable> &params,
                          const InfoType &returnType) const;
    void generateConstructorTest(const std::string &ctorName,
                                 const std::vector<InfoVariable> &params) const;

private:
    const std::string target, testFolder, configFilePath;

    std::string generateParams(const std::vector<InfoVariable> &params) const;
    std::string generateReturn(const InfoType &returnType) const;
    std::string generateParam(const InfoVariable &param) const;
    std::string generateParamRecord(const InfoVariable &record,
                                    const std::string &prefix = "") const;
    std::string
    generateReturnRecord(const InfoType &record,
                         const std::string &prefix = "return_") const;

    void appendToConfigFile(const std::string &content) const;

    static const Config &config;

    // TEST
    static RandomValuesGenerator rvg;
};