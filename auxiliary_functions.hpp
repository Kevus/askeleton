#ifndef AUXILIARY_FUNCTIONS_HPP
#define AUXILIARY_FUNCTIONS_HPP

#include <iomanip>
#include <ctime>

#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <map>

//Boost libraries
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

//Clang libraries
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

#include <sys/stat.h>
#include <sys/types.h>

using namespace clang::tooling;
using namespace llvm;

using namespace clang;
using namespace clang::ast_matchers;

using namespace std;

bool fileExists(const string& filename);
bool folderExists(const string& folder);
string getCommentHeader(string filename);
string deleteAllBeforeChar(string sToReplace, char cToFind);
string cleanUnnecesaryChars(string sToReplace);
bool isNumeric(string query);
string convertExpressionToString(Expr *E, SourceManager &SM);
bool isInParameters(string name, ArrayRef<ParmVarDecl *> params, string& type);
void initializeFrameworks(bool BoostFramework, bool CatchFramework, bool GtestFramework);
void readFrameworks();
void replaceAll(string& str, const string& from, const string& to);
bool endsWith(const string& str, const string& suffix);

#endif
