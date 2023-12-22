#include "VariableInfo.hpp"
#include "ASKGen.hpp"
#include "auxiliary_functions.hpp"
#include <fstream>
#include <ios>

using namespace clang;
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

VariableInfo::VariableInfo(const QualType &type)
    : varName(), originalType(type.getCanonicalType().getAsString()),
      formattedType(), isReturned_(true), isStruct_(false) {

    if (const RecordType *recordType = type->getAs<RecordType>()) {
        isStruct_ = true;
        formattedType = "struct_" + cleanUnnecesaryChars(originalType);
    } else
        formattedType = cleanUnnecesaryChars(originalType);
}

VariableInfo::VariableInfo(const ParmVarDecl *type)
    : varName(type->getQualifiedNameAsString()),
      originalType(type->getOriginalType().getCanonicalType().getAsString()),
      formattedType(), isReturned_(false), isStruct_(false) {

    QualType qualType = type->getType();
    if (const RecordType *recordType = qualType->getAs<RecordType>()) {
        isStruct_ = true;
        formattedType = "struct_" + cleanUnnecesaryChars(originalType);
    } else
        formattedType = cleanUnnecesaryChars(originalType);
}

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
    if (wasAddedToFixture(fixture))
        return;

    typesMap &types = getTypes();
    std::ofstream supportedsFile(fixture, std::ios_base::app);

    supportedsFile << originalType << "\n";
    types[fixture].insert(originalType);

    supportedsFile.close();
}