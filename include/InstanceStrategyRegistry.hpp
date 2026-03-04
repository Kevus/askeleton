#pragma once

#include <map>
#include <optional>
#include <string>

#include "VariableInfo.hpp"

struct ConfiguredInstanceStrategy {
    std::string expr;
};

class InstanceStrategyRegistry {
public:
    static const InstanceStrategyRegistry &get();

    std::optional<ConfiguredInstanceStrategy>
    find(const InfoType &type, const std::string &functionName = "") const;

private:
    InstanceStrategyRegistry();
    void load();

    std::map<std::string, ConfiguredInstanceStrategy> typeStrategies;
    std::map<std::string, ConfiguredInstanceStrategy> functionStrategies;
};
