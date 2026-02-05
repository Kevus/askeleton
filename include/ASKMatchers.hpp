#pragma once

#include <map>
#include <string>

#include "clang/ASTMatchers/ASTMatchers.h"

std::map<std::string, clang::ast_matchers::DeclarationMatcher>
createMapMatchers(bool includeDataMatchers);
