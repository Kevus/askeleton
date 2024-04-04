#pragma once

#include "VariableInfo.hpp"
#include "constants.hpp"
#include <string>

namespace askeleton {
// Generators for Boost.Test
namespace bt {
string generateVariableAssign(const string &funcCfgName,
                              const InfoVariable &variable);
string generateVariableAssign(const string &funcCfgName, const InfoType &type);
string generatePointerAssign(const string &funcCfgName,
                             const InfoVariable &variable);
string generateReferenceAssign(const string &functionName,
							   const InfoVariable &reference);
string generateReferenceRemoval(const InfoVariable &reference);
} // namespace bt

namespace routes {
string generateSupportPath(const string &className);
}
} // namespace askeleton
