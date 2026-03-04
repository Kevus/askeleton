#pragma once

#include <map>
#include <optional>
#include <string>

#include "GenerationEligibility.hpp"
#include "VariableInfo.hpp"

struct ConfiguredInstanceStrategy {
    std::string expr;
    std::string ownerType;
    std::string ownerExpr;
    std::string ownerCallable;
    InstanceSubjectKind subjectKind = InstanceSubjectKind::Value;
    InstanceSubjectKind ownerSubjectKind = InstanceSubjectKind::Value;
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
