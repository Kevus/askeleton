#include "VariableInfo.hpp"
using std::string;

InfoType::InfoType(const clang::QualType &type)
    : original(type.getCanonicalType().getAsString()), formatted(), type(type),
      isRecord_(false), isEnum_(false), recordFields() {
    if (InfoType::isExcludedType(original)) {
        original = InfoType::excludedTypes.at(original);
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

    if (type->isPointerType() || type->isReferenceType())
        return {type->getPointeeType()};

    return *this;

    // const clang::Type *rawType = type.getTypePtrOrNull();
    // if (rawType == nullptr) {
    //     std::cerr << "Error: rawType es nullptr" << std::endl;
    //     return *this;
    // }

    // std::cout << "rawType no es nullptr" << std::endl;

    // clang::QualType qualType = type.getUnqualifiedType();

    // if (qualType.isNull()) {
    //     std::cerr << "Error: qualType es nulo después de
    //     getUnqualifiedType()"
    //               << std::endl;
    //     return *this;
    // }

    // std::cout << "Gola 3" << std::endl;

    // // Depuración: imprimir el tipo canónico
    // std::cout << "El tipo es: " << qualType.getAsString() << std::endl;
    // std::cout << "Canonical type: " <<
    // qualType.getCanonicalType().getAsString()
    //           << std::endl;

    // // Verifica si es un puntero
    // bool isPointer = qualType->isPointerType();
    // std::cout << "isPointerType: " << isPointer << std::endl;

    // if (isPointer) {
    //     clang::QualType pointeeType = qualType->getPointeeType();
    //     std::cout << "Es puntero. Pointee type: " <<
    //     pointeeType.getAsString()
    //               << std::endl;
    //     return InfoType(pointeeType);
    // }

    // // Verifica si es una referencia
    // bool isReference = qualType->isReferenceType();
    // std::cout << "isReferenceType: " << isReference << std::endl;

    // if (isReference) {
    //     clang::QualType pointeeType = qualType->getPointeeType();
    //     std::cout << "Es referencia. Pointee type: "
    //               << pointeeType.getAsString() << std::endl;
    //     return InfoType(pointeeType);
    // }

    // // Retorna el objeto actual si no es un puntero ni una referencia
    // return *this;
}

// clang::QualType underlying = *type;

// if (isPointer() || isReference())
//     underlying = underlying->getPointeeType();

// return {underlying};
// if (!isReference() && !isPointer())
//     return *this;

// string original = this->original, formatted = this->formatted;
// removeAll(original, {" *", "&"});
// removeAll(formatted, {"_s", "_r"});

// return {original, formatted};

string InfoType::formatType(const string &name) {
    string formatted = cleanUnnecesaryChars(name);
    replaceAll(formatted, "*", "s");
    return formatted;
}

vector<InfoVariable> InfoType::getRecordFields() const { return recordFields; }

const std::map<std::string, std::string> InfoType::excludedTypes = {
    {"class std::basic_string<char>", "std::string"},
    {"const char *", "const char *"},
    {"char *", "char *"}};

bool InfoType::isExcludedType(const std::string &type) {
    return InfoType::excludedTypes.contains(type);
}

InfoVariable::InfoVariable(const clang::ParmVarDecl *param)
    : InfoType(param->getOriginalType()),
      name(param->getQualifiedNameAsString()) {
    if (name == "")
        name = formatted + "_" + to_string(NO_NAME_COUNT++);
}

InfoVariable::InfoVariable(const clang::FieldDecl *field)
    : InfoType(field->getType()), name(field->getNameAsString()) {}

InfoVariable::InfoVariable(const string &name, const string &original,
                           const string &formatted)
    : InfoType(original, formatted), name(name) {}

unsigned InfoVariable::NO_NAME_COUNT = 0;