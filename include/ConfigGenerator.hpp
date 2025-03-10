#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "RandomValuesGenerator.hpp"
#include "VariableInfo.hpp"

class ConfigGenerator {
public:
    explicit ConfigGenerator(const std::string &target);
    ConfigGenerator(const ConfigGenerator &) = default;

    void generateTestCase(const std::string &functionName,
                          const std::vector<InfoVariable> &params,
                          const InfoType &returnType,
                          unsigned invocationNumber = 1) const;
    void generateConstructorTest(const std::string &ctorName,
                                 const std::vector<InfoVariable> &params,
                                 unsigned invocationNumber = 1) const;

private:
    const std::string target, testFolder, configFilePath;

    std::string generateParam(const InfoVariable &param, bool generatePointers = true,
                              const std::string &prefix = "") const;
    std::string generateParam(const std::vector<InfoVariable> &params,
                              bool generatePointers = true,
                              const std::string &prefix = "") const;
    std::string generateReturn(const InfoType &returnType) const;

    void appendToConfigFile(const std::string &content) const;

    static const nlohmann::json &config;
    static const nlohmann::json &tplItems;
    static RandomValuesGenerator rvg;
};