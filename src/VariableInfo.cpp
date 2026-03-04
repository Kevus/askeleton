#include "VariableInfo.hpp"

#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "utils/templating.hpp"
#include "utils/ast_values.hpp"
#include "clang/AST/DeclCXX.h"

#include <algorithm>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

using namespace clang;
using std::string;

namespace {
bool startsWithToken(const std::string &value, const std::string &token) {
    if (value == token) {
        return true;
    }
    if (value.size() <= token.size()) {
        return false;
    }
    return value.rfind(token + "_", 0) == 0;
}

bool looksLikeOutputName(std::string name) {
    name = toLower(name);
    const std::vector<std::string> hints = {"out", "output", "result", "retval",
                                            "dst", "dest", "buf", "buffer"};
    for (const auto &hint : hints) {
        if (startsWithToken(name, hint)) {
            return true;
        }
    }
    return false;
}

ParameterIntent inferIntent(const ParmVarDecl *param) {
    if (!param) {
        return ParameterIntent::Input;
    }

    const std::string originalType = param->getOriginalType().getAsString();
    const bool isPointer = containsSubstring(originalType, "*");
    const bool isReference = containsSubstring(originalType, "&");
    if (!isPointer && !isReference) {
        return ParameterIntent::Input;
    }

    if (containsSubstring(originalType, "const")) {
        return ParameterIntent::Input;
    }

    if (looksLikeOutputName(param->getNameAsString())) {
        return ParameterIntent::Output;
    }

    return ParameterIntent::InOut;
}

std::optional<std::string> getEquivalentWrappedType(clang::QualType type) {
    if (type.isNull()) {
        return std::nullopt;
    }

    std::string suffix;
    while (!type.isNull() && (type->isPointerType() || type->isReferenceType())) {
        if (type->isPointerType()) {
            suffix += " *";
            type = type->getPointeeType();
        } else {
            suffix += " &";
            type = type->getPointeeType();
        }
    }

    if (type.isNull()) {
        return std::nullopt;
    }

    std::string base = type.getCanonicalType().getAsString();
    removeTypeQualifiers(base);
    auto equivalent = getEquivalentType(base);
    if (!equivalent.has_value()) {
        return std::nullopt;
    }

    return equivalent.value() + suffix;
}
} // namespace

ComplexTypeException::ComplexTypeException(const string &complexType)
    : ComplexTypeException("unsupported_type_shape", complexType) {}

ComplexTypeException::ComplexTypeException(const string &reasonCode,
                                           const string &complexType)
    : std::runtime_error("Type is too complex for ASkeleTon [" + reasonCode +
                         "]: " + complexType),
      reasonCode(reasonCode), type(complexType) {}

InfoType::InfoType(const clang::QualType &type)
    : original(type.getCanonicalType().getAsString()), formatted(), type(type),
      isRecord_(false), isEnum_(false), recordFields() {

    auto equivalent = getEquivalentType(original);
    if (!equivalent.has_value()) {
        std::string normalized = original;
        removeTypeQualifiers(normalized);
        equivalent = getEquivalentType(normalized);
    }
    if (!equivalent.has_value()) {
        equivalent = getEquivalentWrappedType(type);
    }
    if (equivalent.has_value()) {
        original = equivalent.value();
    } else if (type->isArrayType()) {
        isArray_ = true;
        if (const auto *arrayType = dyn_cast<clang::ArrayType>(type.getTypePtr())) {
            QualType elemType = arrayType->getElementType();
            std::string elemStr = elemType.getCanonicalType().getAsString();
            if (containsSubstring(elemStr, "char")) {
                original = elemStr + " *";
            } else {
                throw ComplexTypeException(original);
            }
        } else {
            throw ComplexTypeException("unsupported_array_shape", original);
        }
    } else if (const CXXRecordDecl *record = type->getAsCXXRecordDecl()) {
        isRecord_ = true;
        // Preserve the original type string (which may include template args),
        // only overriding it for anonymous records with a typedef name.
        if (this->original.find("anonymous") != string::npos) {
            if (const auto *typedefDecl = record->getTypedefNameForAnonDecl()) {
                this->original = typedefDecl->getNameAsString();
            }
        }
        for (const auto *field : record->fields()) {
            if (!field || field->getAccess() != AS_public) {
                continue;
            }

            // Fixed-size C arrays cannot be assigned or compared as plain
            // fields in the generated record helpers, so omit them instead of
            // rejecting the whole record type.
            if (field->getType()->isArrayType()) {
                continue;
            }

            recordFields.emplace_back(field);
        }
    } else if (typeIsComplex(original)) {
        throw ComplexTypeException(original);
    } else if (const Type *unqualified = type.getUnqualifiedType().getTypePtrOrNull()) {
        if (unqualified->isEnumeralType())
            isEnum_ = true;
    }

    // const int * -> int *
    // const int & -> int &
    // const std::string & -> std::string &
    removeTypeQualifiers(original);

    // int * -> int *
    // int & -> int &
    // std::string & -> string &
    formatted = removeNamespaceQualifier(original);

    // int * -> int_*
    // int & -> int_&
    // string & -> string_&
    if (!isTemplateParametrized())
        replaceAll(formatted, " ", "_");

    // int_* -> int_s
    // int_& -> int_r
    // string & -> string_r
    replaceTypeCharacters(formatted);
}

InfoType::InfoType(const string &original)
    : original(original), formatted(original), type(), isRecord_(false), isEnum_(false),
      recordFields() {
    removeTypeQualifiers(this->original);
    this->formatted = removeNamespaceQualifier(this->original);
    if (!isTemplateParametrized()) {
        replaceAll(this->formatted, " ", "_");
    }
    replaceTypeCharacters(this->formatted);
}

InfoType::InfoType(const string &original, const string &formatted)
    : original(original), formatted(formatted), type(), isRecord_(false), isEnum_(false),
      recordFields() {
    removeTypeQualifiers(this->original);
}

bool InfoType::isContainer() const {
    return containsAnySubstring(original, {"list", "vector", "map"});
}

bool InfoType::isList() const {
    return !isMap() && containsAnySubstring(original, {"list", "vector"});
}

bool InfoType::isMap() const { return containsSubstring(original, "map"); }

bool InfoType::isOptional() const { return containsSubstring(original, "optional<"); }

bool InfoType::isPair() const { return containsSubstring(original, "pair<"); }

bool InfoType::isTuple() const { return containsSubstring(original, "tuple<"); }

bool InfoType::isPointer() const {
    return containsSubstring(original, "*") && !isArray_;
}

bool InfoType::isReference() const { return containsSubstring(original, "&"); }

bool InfoType::isArray() const { return isArray_; }

bool InfoType::isRecord() const { return isRecord_; }

bool InfoType::isEnum() const { return isEnum_; }

bool InfoType::isTemplateParametrized() const {
    return std::regex_search(original, std::regex(R"(<.+>)"));
}

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

InfoVariable InfoType::getTypeAsReturn() const {
    const static string returnPrefix = getTemplateItems()["tplitem"]["return_prefix"];
    return {returnPrefix + getFormattedNotParametrized(), *this};
}

string InfoType::formatType(const string &name) {
    string formatted = removeNamespaceQualifier(name);
    replaceAll(formatted, "*", "s");
    return formatted;
}

bool InfoType::typeIsComplex(const string &type) {
    if (containsAnySubstring(type, {"[", "]", "..."})) {
        return true;
    }

    string normalized = removeTemplateArguments(removeNamespaceQualifier(type));
    removeTypeQualifiers(normalized);
    return !normalized.empty() && normalized[0] == '_';
}

std::vector<InfoVariable> InfoType::getRecordFields() const { return recordFields; }

std::vector<InfoType> InfoType::getTemplateArguments() const {
    std::vector<InfoType> args;

    const size_t left = original.find('<');
    const size_t right = original.find_last_of('>');
    if (left == std::string::npos || right == std::string::npos || right <= left) {
        return args;
    }

    std::string current;
    int depth = 0;
    for (size_t i = left + 1; i < right; ++i) {
        const char c = original[i];
        if (c == '<') {
            ++depth;
            current.push_back(c);
        } else if (c == '>') {
            --depth;
            current.push_back(c);
        } else if (c == ',' && depth == 0) {
            ltrim(current);
            rtrim(current);
            if (!current.empty()) {
                args.emplace_back(current);
            }
            current.clear();
        } else {
            current.push_back(c);
        }
    }

    ltrim(current);
    rtrim(current);
    if (!current.empty()) {
        args.emplace_back(current);
    }

    return args;
}

InfoVariable::InfoVariable(const clang::ParmVarDecl *param)
    : InfoType(param->getOriginalType()), name(param->getQualifiedNameAsString()) {
    if (name == "")
        name = formatted + "_" + std::to_string(NO_NAME_COUNT++);
    intent = inferIntent(param);
}

InfoVariable::InfoVariable(const clang::FieldDecl *field)
    : InfoType(field->getType()), name(field->getNameAsString()) {
    if (field && field->hasInClassInitializer()) {
        defaultValue = extractLiteralValue(field->getInClassInitializer());
    }
}

InfoVariable::InfoVariable(const string &name, const string &original,
                           const string &formatted)
    : InfoType(original, formatted), name(name) {}

InfoVariable::InfoVariable(const string &varName, const InfoType &type)
    : InfoType(type), name(varName) {
    if (name == "")
        name = formatted + "_" + std::to_string(NO_NAME_COUNT++);
}

std::pair<InfoVariable, InfoVariable> InfoVariable::getPointers() const {
    InfoType underlying = getUnderlyingType();
    InfoVariable input{name, underlying};
    InfoVariable output{name + "_output", underlying};
    input.intent = intent;
    output.intent = intent;
    return {input, output};
}

std::pair<string, string> InfoVariable::getPointersVarName() const {
    return {name, name + "_output"};
}

bool InfoVariable::isInputOnly() const { return intent == ParameterIntent::Input; }

bool InfoVariable::isOutputOnly() const { return intent == ParameterIntent::Output; }

bool InfoVariable::isInputOutput() const { return intent == ParameterIntent::InOut; }

bool InfoVariable::isMutableOutput() const {
    return isOutputOnly() || isInputOutput();
}

string InfoType::getFormattedNotParametrized() const {
    return isTemplateParametrized() ? extractSubstringUntilCharacter(formatted, '<')
                                    : formatted;
}

unsigned InfoVariable::NO_NAME_COUNT = 0;
