#include "VariableInfo.hpp"

inline InfoType::InfoType(std::string original, std::string formatted) : 
	original(original), formatted(formatted) {}

inline InfoVariable::InfoVariable(std::string name, std::string original, std::string formatted) :
		name(name), InfoType(original, formatted) {}