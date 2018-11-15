#ifndef ASTUTGEN_HPP
#define ASTUTGEN_HPP

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersInternal.h"
#include "clang/ASTMatchers/ASTMatchersMacros.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/SourceManager.h"

#include "llvm/Support/CommandLine.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "clang/Lex/Lexer.h"

#include <stdio.h>

using namespace clang::tooling;
using namespace llvm;

using namespace clang;
using namespace clang::ast_matchers;

using namespace std;

class ASTUTGen : public MatchFinder::MatchCallback
{
	virtual void run(const MatchFinder::MatchResult &Result);

	void apply_FunctionExample(const MatchFinder::MatchResult &Result);
};

#endif