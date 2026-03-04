#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "VariableInfo.hpp"
#include "clang/AST/DeclCXX.h"

enum class InstancePlanKind {
    Configured,
    DefaultConstructor,
    Constructor,
    StaticFactory,
    FreeFactory,
    OwnerFactory,
};

enum class InstanceSubjectKind {
    Value,
    Pointer,
    Reference,
};

struct InstancePlan {
    InstancePlanKind kind = InstancePlanKind::DefaultConstructor;
    InstanceSubjectKind subjectKind = InstanceSubjectKind::Value;
    std::vector<InfoVariable> setupParams;
    std::string callableExpr;
    std::string initExpr;
    std::string ownerTypeName;
    std::shared_ptr<InstancePlan> ownerPlan;
    unsigned totalParameters = 0;

    bool usesDefaultConstructor() const {
        return kind == InstancePlanKind::DefaultConstructor;
    }

    bool requiresSetupInputs() const { return !setupParams.empty(); }
    bool usesCallable() const { return !callableExpr.empty(); }
    bool usesDirectExpression() const { return !initExpr.empty(); }
    bool usesOwner() const { return ownerPlan != nullptr; }
    bool usesMemberAccessArrow() const {
        return subjectKind == InstanceSubjectKind::Pointer;
    }
};

bool requiresMutableAliasHandling(const std::vector<InfoVariable> &params);
unsigned minimumCoverageInvocations(const std::vector<InfoVariable> &params);

void validateTypeMaterialization(const InfoType &type,
                                 const std::string &functionName = "");
void validateReturnTypeMaterialization(const InfoType &type,
                                       const std::string &functionName = "");
void validateTypesMaterialization(const std::vector<InfoVariable> &variables,
                                  const std::string &functionName = "");

std::optional<InstancePlan> resolveInstancePlan(const clang::CXXRecordDecl *record,
                                                const std::string &functionName);
