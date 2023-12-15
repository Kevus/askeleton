#include "VariableInfo.hpp"

using std::map;
using std::set;
using std::string;

typedef map<string, set<string>> typesMap;

typesMap &VariableInfo::getTypes() {
    static map<string, set<string>> types;
    return types;
}

VariableInfo::VariableInfo(const string &originalType, const string &varName)
    : varName(varName), originalType(originalType), formattedType(originalType),
      isReturned_(varName == "") {}

bool VariableInfo::isPrimitiveType() const {
    primitives.find(originalType) != primitives.end();
}

bool VariableInfo::isReturned() const { return isReturned_; }

Parameter VariableInfo::getAsParameter() const {
    return {originalType, varName};
}

const string &VariableInfo::getName() const { return varName; }

bool VariableInfo::wasAddedToFixture(const string &fixture) const {
    typesMap &types = getTypes();
    auto it = types.find(fixture);

    return it != types.end() &&
           (it->second.find(originalType) != it->second.end());
}

void VariableInfo::addToFixture(const string &fixture) const {
    typesMap &types = getTypes();

    if (!wasAddedToFixture(fixture)) {
        types[fixture].insert(originalType);
        // TODO: procesar al agregar
    }
}