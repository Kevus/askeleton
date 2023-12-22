#ifndef VARIABLE_INFO_HPP
#define VARIABLE_INFO_HPP

#include <map>
#include <set>
#include <string>

#include <clang/AST/Decl.h>

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

class VariableInfo {
public:
    VariableInfo(const std::string &originalType,
                 const std::string &varName = "");
    VariableInfo(const QualType &);
    VariableInfo(const ParmVarDecl *);

    bool isReturned() const;
    Parameter getAsParameter() const;
    const std::string &getName() const;

    bool isPrimitiveType() const;
    bool wasAddedToFixture(const std::string &fixture) const;
    void addToFixture(const std::string &fixture) const;

private:
    static std::map<std::string, std::set<std::string>> &getTypes();

    std::string varName, originalType, formattedType;
    bool isReturned_, isStruct_;
};

#endif /* VARIABLE_INFO_HPP */