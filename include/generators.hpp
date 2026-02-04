#pragma once

#include "VariableInfo.hpp"
#include "constants.hpp"
#include <string>

namespace askeleton {
// Generators for Boost.Test
namespace bt {
std::string generateVariableAssign(const std::string &funcCfgName,
                                   const InfoVariable &variable);
std::string generateVariableAssign(const std::string &funcCfgName,
                                   const InfoType &type);
std::string generatePointerAssign(const std::string &funcCfgName,
                                  const InfoVariable &variable);
std::string generateReferenceAssign(const std::string &functionName,
                                    const InfoVariable &reference);
std::string generateReferenceRemoval(const InfoVariable &reference);
} // namespace bt

namespace routes {
std::string generateSupportPath(const std::string &className);
}
} // namespace askeleton
