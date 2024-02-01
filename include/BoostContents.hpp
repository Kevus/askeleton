#pragma once

#include <string>
#include "VariableInfo.hpp"

namespace askeleton {
	namespace btcontents {
		string generateVariableAssign(const string &funcCfgName, const InfoVariable &variable);
		string generateVariableAssign(const string &funcCfgName, const InfoType &type);
	}
} // namespace askeleton
