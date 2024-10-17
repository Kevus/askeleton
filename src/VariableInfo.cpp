#include "VariableInfo.hpp"

#include "EquivalentTypesManager.hpp"
#include "auxiliary_functions.hpp"

#include "clang/AST/DeclCXX.h"

#include <string>
#include <vector>

using namespace clang;
using std::string;

InfoType::InfoType(const clang::QualType &type)
    : original(type.getCanonicalType().getAsString()), formatted(), type(type),
      isRecord_(false), isEnum_(false), recordFields() {

    EquivalentTypesManager &manager = EquivalentTypesManager::getInstance();
    auto equivalent = manager.getEquivalentType(original);
    if (equivalent.has_value()) {
        original = equivalent.value();
    } else if (const CXXRecordDecl *record = type->getAsCXXRecordDecl()) {
        isRecord_ = true;
        string original = record->getQualifiedNameAsString();
        if (original.find("anonymous") != string::npos)
            original = record->getTypedefNameForAnonDecl()->getNameAsString();
        copy_if(record->fields().begin(), record->fields().end(),
                back_inserter(recordFields), [](const FieldDecl *field) {
                    return field->getAccess() == AS_public;
                });

    } else if (const Type *unqualified =
                   type.getUnqualifiedType().getTypePtrOrNull()) {
        if (unqualified->isEnumeralType())
            isEnum_ = true;
    }

    // const int * -> int *
    // const int & -> int &
    removeTypeQualifiers(original);

    // int * -> int_*
    // int & -> int_&
    formatted = cleanUnnecesaryChars(original);

    // int_* -> int_s
    // int_& -> int_r
    replaceTypeCharacters(formatted);
}

InfoType::InfoType(const string &original)
    : original(original), formatted(original), type(), isRecord_(false),
      isEnum_(false), recordFields() {
    removeTypeQualifiers(this->original);
    this->formatted = cleanUnnecesaryChars(this->original);
    replaceTypeCharacters(this->formatted);
}

InfoType::InfoType(const string &original, const string &formatted)
    : original(original), formatted(formatted), type(), isRecord_(false),
      isEnum_(false), recordFields() {
    removeTypeQualifiers(this->original);
}

bool InfoType::isContainer() const {
    return containsAnySubstring(original, {"list", "vector", "map"});
}

bool InfoType::isPointer() const { return containsSubstring(original, "*"); }

bool InfoType::isReference() const { return containsSubstring(original, "&"); }

bool InfoType::isRecord() const { return isRecord_; }

bool InfoType::isEnum() const { return isEnum_; }

InfoType InfoType::getUnderlyingType() const {
    if (type.isNull()) {
        std::string original = this->original;
        std::string formatted = this->formatted;
        removeAll(original, {" *", "&"});
        removeAll(formatted, {"_s", "_r"});

        return {original, formatted};
    }

    if (type->isPointerType() || type->isReferenceType()) {
        if (type->getPointeeType().isNull()) {
            return *this;
        }
        return {type->getPointeeType()};
    }

    return *this;
}

string InfoType::formatType(const string &name) {
    string formatted = cleanUnnecesaryChars(name);
    replaceAll(formatted, "*", "s");
    return formatted;
}

std::vector<InfoVariable> InfoType::getRecordFields() const {
    return recordFields;
}

InfoVariable::InfoVariable(const clang::ParmVarDecl *param)
    : InfoType(param->getOriginalType()),
      name(param->getQualifiedNameAsString()) {
    if (name == "")
        name = formatted + "_" + std::to_string(NO_NAME_COUNT++);
}

InfoVariable::InfoVariable(const clang::FieldDecl *field)
    : InfoType(field->getType()), name(field->getNameAsString()) {}

InfoVariable::InfoVariable(const string &name, const string &original,
                           const string &formatted)
    : InfoType(original, formatted), name(name) {}

unsigned InfoVariable::NO_NAME_COUNT = 0;