#pragma once

#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"

#include <map>
#include <string>
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
    const clang::QualType type;

protected:
    static std::string formatType(const std::string &);

    bool isRecord_, isEnum_;
    std::vector<InfoVariable> recordFields;
};

struct InfoVariable : public InfoType {
    InfoVariable() = default;
    InfoVariable(const clang::ParmVarDecl *);
    InfoVariable(const clang::FieldDecl *);
    InfoVariable(const std::string &name, const std::string &original,
                 const std::string &formatted);

    std::string name;
    static unsigned NO_NAME_COUNT;
};