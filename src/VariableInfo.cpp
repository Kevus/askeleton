#include "VariableInfo.hpp"
using std::string;

InfoType::InfoType(string original, string formatted)
    : original(original), formatted(formatted) {}

InfoType::InfoType(string original)
    : original(original), formatted(formatType(original)) {}

bool InfoType::isContainer() const {
    return containsAnySubstring(original, {"list", "vector", "map"});
}

bool InfoType::isPointer() const { return containsSubstring(original, "*"); }

bool InfoType::isReference() const {
    return containsAnySubstring(original, {"&", "const_"});
}

InfoType InfoType::getUnderlyingType() const {
    string original = this->original, formatted = this->formatted;

    replaceAll(original, " *", "");
    replaceAll(formatted, "_s", "");

    return {original, formatted};
}

string InfoType::formatType(const string &name) {
    string formatted = cleanUnnecesaryChars(name);
    replaceAll(formatted, "*", "s");
    return formatted;
}

InfoVariable::InfoVariable(string name, string original, string formatted)
    : InfoType(original, formatted), name(name) {}
