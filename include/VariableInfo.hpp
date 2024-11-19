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
    bool isList() const;
    bool isMap() const;

    bool isPointer() const;
    bool isReference() const;

    bool isRecord() const;
    bool isEnum() const;

    bool isTemplateParametrized() const;

    InfoType getUnderlyingType() const;
    InfoVariable getTypeAsReturn() const;
    std::vector<InfoVariable> getRecordFields() const;
    std::string getFormattedNotParametrized() const;

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
    InfoVariable(const std::string &varName, const InfoType &type);
    InfoVariable(const std::string &name, const std::string &original,
                 const std::string &formatted);

    std::pair<InfoVariable, InfoVariable> getPointers() const;
    std::pair<std::string, std::string> getPointersVarName() const;

    std::string name;
    static unsigned NO_NAME_COUNT;
};