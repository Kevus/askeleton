#pragma once

#include <map>
#include <string>

#include "clang/ASTMatchers/ASTMatchers.h"

namespace clang {
namespace ast_matchers {
using namespace clang::ast_matchers::internal;
}
} // namespace clang

std::map<std::string, clang::ast_matchers::DeclarationMatcher>
createMapMatchers(bool includeDataMatchers);
