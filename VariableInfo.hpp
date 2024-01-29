#ifndef VARIABLE_INFO_HPP
#define VARIABLE_INFO_HPP

#include <set>
#include <string>

static const std::set<std::string> primitives = {"_Bool",
                                                 "bool",
                                                 "char",
                                                 "char *",
                                                 "double",
                                                 "float",
                                                 "int",
                                                 "long",
                                                 "long double",
                                                 "long int",
                                                 "long long",
                                                 "long long int",
                                                 "short",
                                                 "short int",
                                                 "signed",
                                                 "signed char",
                                                 "signed int",
                                                 "signed long",
                                                 "signed long int",
                                                 "signed long long",
                                                 "signed long long int",
                                                 "signed short",
                                                 "signed short int",
                                                 "size_t",
                                                 "std::__cxx11::string",
                                                 "std::string",
                                                 "string",
                                                 "unsigned",
                                                 "unsigned char",
                                                 "unsigned int",
                                                 "unsigned long",
                                                 "unsigned long int",
                                                 "unsigned long long",
                                                 "unsigned long long int"},
                                   complexExcluded = {"map", "list", "vector"};

// A pair which represents the type and the name of a parameter
typedef std::pair<std::string, std::string> Parameter;

struct InfoType {
	InfoType() = default;
	InfoType(std::string original, std::string formatted);
	
	std::string original, formatted;
};

struct InfoVariable: public InfoType {
	InfoVariable() = default;
	InfoVariable(std::string name, std::string original, std::string formatted);

	std::string name;
};

#endif /* VARIABLE_INFO_HPP */