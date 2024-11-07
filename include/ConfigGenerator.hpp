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

    static void loadConfigurations();

private:
    const std::string target, testFolder, configFilePath;

    std::string generateParams(const std::vector<InfoVariable> &params) const;
    std::string generateReturn(const InfoType &returnType) const;
    std::string generateParam(const InfoVariable &param) const;
    std::string generateParamRecord(InfoType &underlying,
                                    const std::string &name) const;
    std::string
    generateReturnRecord(const InfoType &record,
                         const std::string &prefix = "return_") const;

    void appendToConfigFile(const std::string &content) const;

    static const Config &config;
    static std::map<std::string, std::string> tplItems;
    static RandomValuesGenerator rvg;
};