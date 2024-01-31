#include "VariableInfo.hpp"
using std::string;

inline InfoType::InfoType(string original, string formatted) : 
	original(original), formatted(formatted) {}

inline bool InfoType::isContainer() const {
	return original.find("list") != string::npos ||
    	   original.find("vector") != string::npos ||
           original.find("map") != string::npos;
}

inline InfoType::isPointer() const {
	return original.find("*") != string::npos;
}

inline InfoVariable::InfoVariable(string name, string original, string formatted) :
		name(name), InfoType(original, formatted) {}
