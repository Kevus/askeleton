#pragma once

#include "auxiliary_functions.hpp"

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
                                                 "unsigned long long int"};

static const std::vector<std::string> containers = {"map", "list", "vector"};

// A pair which represents the type and the name of a parameter
typedef std::pair<std::string, std::string> Parameter;

struct InfoVariable;

struct InfoType {
    InfoType() = default;
	InfoType(const clang::QualType &);
    InfoType(const std::string &original);
    InfoType(const std::string &original, const std::string &formatted);

    bool isContainer() const;
    bool isPointer() const;
    bool isReference() const;
	bool isRecord() const;
	bool isEnum() const;

    InfoType getUnderlyingType() const;
	std::vector<InfoVariable> getRecordFields() const;

    std::string original, formatted;

protected:
    static std::string formatType(const std::string &);
	bool isRecord_, isEnum_;
	std::vector<InfoVariable> recordFields;
};

struct InfoVariable : public InfoType {
    InfoVariable() = default;
	InfoVariable(const clang::ParmVarDecl *);
	InfoVariable(const clang::FieldDecl *);
    InfoVariable(const std::string &name, const std::string &original, const std::string &formatted);

    std::string name;
	static unsigned NO_NAME_COUNT;
};