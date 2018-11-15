#ifndef ASTUT_MATCHERS_HPP
#define ASTUT_MATCHERS_HPP

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersInternal.h"
#include "clang/ASTMatchers/ASTMatchersMacros.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <map>
#include <iostream>
#include <algorithm>

using namespace std;

using namespace clang;
using namespace clang::ast_matchers;

namespace clang{
	namespace ast_matchers{
	  using namespace clang::ast_matchers::internal;
	}
}

map<string, DeclarationMatcher> createMapMatchers();

#endif