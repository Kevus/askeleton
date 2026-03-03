#pragma once

#include <optional>
#include <string>
#include <vector>

#include "VariableInfo.hpp"
#include "clang/AST/DeclCXX.h"

struct ConstructorSelection {
    std::vector<InfoVariable> params;
    bool useDefaultConstructor = false;
    unsigned totalParameters = 0;
};

bool requiresMutableAliasHandling(const std::vector<InfoVariable> &params);
unsigned minimumCoverageInvocations(const std::vector<InfoVariable> &params);

void validateTypeMaterialization(const InfoType &type,
                                 const std::string &functionName = "");
void validateTypesMaterialization(const std::vector<InfoVariable> &variables,
                                  const std::string &functionName = "");

std::optional<ConstructorSelection>
selectConstructorForInstantiation(const clang::CXXRecordDecl *record,
                                  const std::string &functionName);
