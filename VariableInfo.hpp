#ifndef VARIABLE_INFO_HPP
#define VARIABLE_INFO_HPP

#include <map>
#include <set>
#include <string>

static const std::set<std::string> primitives = {"int", "char", "bool",
                                                 "unsigned"};

// A pair which represents the type and the name of a parameter
typedef std::pair<std::string, std::string> Parameter;

class VariableInfo {
public:
    VariableInfo(const std::string &originalType,
                 const std::string &varName = "");

    bool isReturned() const;
    Parameter getAsParameter() const;
    const std::string &getName() const;

    bool isPrimitiveType() const;
    bool wasAddedToFixture(const std::string &fixture) const;
    void addToFixture(const std::string &fixture) const;

private:
    static std::map<std::string, std::set<std::string>> &getTypes();

    std::string varName, originalType, formattedType;
    bool isReturned_;
};

#endif /* VARIABLE_INFO_HPP */