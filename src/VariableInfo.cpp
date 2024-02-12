#include "VariableInfo.hpp"
using std::string;

InfoType::InfoType(const clang::QualType &type) : 
		original(type.getCanonicalType().getAsString()),
		formatted(), isRecord_(false), isEnum_(false), recordFields() {
	if(const CXXRecordDecl *record = type->getAsCXXRecordDecl()) {
		isRecord_ = true;
		string original = record->getQualifiedNameAsString();
        if (original.find("anonymous") != string::npos)
            original = record->getTypedefNameForAnonDecl()->getNameAsString();
		transform(record->fields().begin(), record->fields().end(),
			back_inserter(recordFields),
			[](const FieldDecl *field) { return field; });

	} else if(const Type *unqualified = 
			type.getUnqualifiedType().getTypePtrOrNull()) {
		if(unqualified->isEnumeralType()) isEnum_ = true;
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
    : original(original), formatted(original), isRecord_(false), isEnum_(false) {
		removeTypeQualifiers(this->original);
		this->formatted = cleanUnnecesaryChars(this->original);
		replaceTypeCharacters(this->formatted);
	}

InfoType::InfoType(const string &original, const string &formatted)
    : original(original), formatted(formatted), isRecord_(false), isEnum_(false)  {
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
	if(!isReference() && !isPointer()) return *this;

    string original = this->original, formatted = this->formatted;
	removeAll(original, {" *", "&"});
	removeAll(formatted, {"_s", "_r"});

    return {original, formatted};
}

vector<InfoVariable> InfoType::getRecordFields() const {
	return recordFields;
}

string InfoType::formatType(const string &name) {
    string formatted = cleanUnnecesaryChars(name);
    replaceAll(formatted, "*", "s");
    return formatted;
}

InfoVariable::InfoVariable(const clang::ParmVarDecl *param) : 
		InfoType(param->getOriginalType()), name(param->getQualifiedNameAsString()) {
	if(name == "") name = formatted + "_" + to_string(NO_NAME_COUNT++);
}

InfoVariable::InfoVariable(const clang::FieldDecl *field) : 
		InfoType(field->getType()), name(field->getNameAsString()) {}

InfoVariable::InfoVariable(const string &name, const string &original, const string &formatted)
    : InfoType(original, formatted), name(name) {}

unsigned InfoVariable::NO_NAME_COUNT = 0;