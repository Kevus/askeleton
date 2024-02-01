#include "VariableInfo.hpp"
using std::string;

InfoType::InfoType(string original, string formatted) : 
	original(original), formatted(formatted) {}

bool InfoType::isContainer() const {
	return containsAnySubstring(original, {"list", "vector", "map"});
}

bool InfoType::isPointer() const {
	return containsSubstring(original, "*");
}

bool InfoType::isReference() const {
	return containsAnySubstring(original, {"&", "const_"});
}

InfoVariable::InfoVariable(string name, string original, string formatted) :
		InfoType(original, formatted), name(name) {}
