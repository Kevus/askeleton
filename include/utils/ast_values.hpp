#pragma once

#include <optional>
#include <string>

#include "clang/AST/Expr.h"

std::optional<long long> extractIntegerValue(const clang::Expr *expr);
std::optional<std::string> extractStringLiteral(const clang::Expr *expr);
std::optional<std::string> extractLiteralValue(const clang::Expr *expr);
