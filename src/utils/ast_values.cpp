#include "utils/ast_values.hpp"

#include "clang/AST/ExprCXX.h"
#include "clang/AST/Decl.h"

using namespace clang;

std::optional<long long> extractIntegerValue(const Expr *expr) {
    if (!expr)
        return std::nullopt;

    const Expr *cleanExpr = expr->IgnoreParenImpCasts();
    if (const auto *intLiteral = dyn_cast<IntegerLiteral>(cleanExpr)) {
        return intLiteral->getValue().getSExtValue();
    }

    if (const auto *boolLiteral = dyn_cast<CXXBoolLiteralExpr>(cleanExpr)) {
        return boolLiteral->getValue() ? 1 : 0;
    }

    if (const auto *declRef = dyn_cast<DeclRefExpr>(cleanExpr)) {
        if (const auto *enumConstant = dyn_cast<EnumConstantDecl>(declRef->getDecl())) {
            return enumConstant->getInitVal().getSExtValue();
        }

        if (const auto *varDecl = dyn_cast<VarDecl>(declRef->getDecl())) {
            if (varDecl->hasInit()) {
                return extractIntegerValue(varDecl->getInit());
            }
        }
    }

    if (const auto *unary = dyn_cast<UnaryOperator>(cleanExpr)) {
        if (unary->getOpcode() == UO_Minus) {
            const Expr *subExpr = unary->getSubExpr()->IgnoreParenImpCasts();
            if (const auto *subInt = dyn_cast<IntegerLiteral>(subExpr)) {
                return -subInt->getValue().getSExtValue();
            }
        }
    }

    return std::nullopt;
}

std::optional<std::string> extractStringLiteral(const Expr *expr) {
    if (!expr)
        return std::nullopt;

    const Expr *cleanExpr = expr->IgnoreParenImpCasts();
    if (const auto *literal = dyn_cast<StringLiteral>(cleanExpr)) {
        return literal->getString().str();
    }

    if (const auto *materialize = dyn_cast<MaterializeTemporaryExpr>(cleanExpr)) {
        return extractStringLiteral(materialize->getSubExpr());
    }

    if (const auto *bindTemp = dyn_cast<CXXBindTemporaryExpr>(cleanExpr)) {
        return extractStringLiteral(bindTemp->getSubExpr());
    }

    if (const auto *construct = dyn_cast<CXXConstructExpr>(cleanExpr)) {
        for (const Expr *arg : construct->arguments()) {
            if (auto value = extractStringLiteral(arg)) {
                return value;
            }
        }
    }

    return std::nullopt;
}

std::optional<std::string> extractLiteralValue(const Expr *expr) {
    if (!expr)
        return std::nullopt;

    if (const auto *boolLiteral = dyn_cast<CXXBoolLiteralExpr>(expr)) {
        return boolLiteral->getValue() ? "true" : "false";
    }

    if (auto intValue = extractIntegerValue(expr)) {
        return std::to_string(*intValue);
    }

    if (auto strValue = extractStringLiteral(expr)) {
        return strValue;
    }

    return std::nullopt;
}
