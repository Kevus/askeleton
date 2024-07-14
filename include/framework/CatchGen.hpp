#pragma once

#include "VariableInfo.hpp"
#include <string>
#include <vector>

class CatchGenerator {
public:
    CatchGenerator() = default;
    ~CatchGenerator() = default;

    CatchGenerator(const std::string &filePath, const std::string &cfgName);

    void generateFunctionAssert(const std::string &fileName,
                                const std::string &functionName,
                                const std::string &funcCfgName,
                                const std::vector<InfoVariable> &params,
                                const InfoType &returnType);
    void generateMethodAssert(const std::string &className,
                              const std::string &functionName,
                              const std::string &funcCfgName,
                              const std::vector<InfoVariable> &params,
                              const InfoType &returnType);
    void generateConstructorAssert(const std::string &className,
                                   const std::string &ctorName,
                                   const std::string &ctorCfgName,
                                   const std::vector<InfoVariable> &params);

private:
};