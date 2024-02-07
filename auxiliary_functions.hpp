#ifndef AUXILIARY_FUNCTIONS_HPP
#define AUXILIARY_FUNCTIONS_HPP

#include <ctime>
#include <iomanip>

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>

#include <map>

// Boost libraries
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

// Clang libraries
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersInternal.h"
#include "clang/ASTMatchers/ASTMatchersMacros.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/CommandLine.h"

#include "clang/Lex/Lexer.h"

#include <sys/stat.h>
#include <sys/types.h>

using namespace clang::tooling;
using namespace llvm;

using namespace clang;
using namespace clang::ast_matchers;

using namespace std;

bool fileExists(const string &filename);
bool folderExists(const string &folder);
string getCommentHeader(string filename);
string deleteAllBeforeChar(string sToReplace, char cToFind);
string cleanUnnecesaryChars(string sToReplace);
bool isNumeric(string query);
string convertExpressionToString(Expr *E, SourceManager &SM);
bool isInParameters(string name, ArrayRef<ParmVarDecl *> params, string &type);
void initializeFrameworks(bool BoostFramework, bool CatchFramework,
                          bool GtestFramework);
void readFrameworks();
void replaceAll(string &str, const string &from, const string &to);

/**
 * @brief Removes all occurrences of the specified substring from the given
 * string.
 *
 * @param str The string to be modified.
 * @param substringToRemove The substring to be removed from the original
 * string.
 *
 * @details This function performs an in-place modification of the input string.
 *          It uses std::regex_replace to replace all occurrences of the
 * specified substring with an empty string.
 *
 * @bug No funciona
 *
 * @see std::regex_replace
 */
void removeAll(string &originalString, const string &substringToRemove);

/**
 * @brief Removes all occurrences of specified substrings from the given string.
 *
 * This function iterates over the list of substrings and removes all
 * occurrences of each substring from the provided string.
 *
 * @param str The string from which to remove the substrings.
 * @param substrings The list of substrings to remove.
 *
 * @bug No funciona
 */
void removeAll(string &originalString,
               const initializer_list<string> &substringToRemove);

/**
 * @brief Checks if a substring is present in the given string.
 *
 * This function searches for the specified substring within the main string.
 *
 * @param originalString The main string to search within.
 * @param substring The substring to search for.
 * @return True if the substring is found, false otherwise.
 */
bool containsSubstring(const string &originalString, const string &substring);

/**
 * @brief Checks if any of the specified substrings are present in the given
 * string.
 *
 * This function searches for any of the specified substrings within the main
 * string.
 *
 * @param originalString The main string to search within.
 * @param substrings The list of substrings to search for.
 * @return True if at least one substring is found, false otherwise.
 */
bool containsAnySubstring(const string &originalString,
                          const initializer_list<string> &substrings);

/**
 * @brief Extracts the file name (without extension or path) from a given file
 * route.
 *
 * The `extractFileName` function takes a full file path and returns the name of
 * the file, excluding the extension and path. If no extension is found, it
 * returns the full file name. The function is platform-agnostic and can be used
 * on operating systems with different path conventions.
 *
 * @param fileRoute Full path of the file to be processed.
 * @return std::string The file name without the extension or path.
 */
string extractFileName(const string &fileRoute);

bool endsWith(const string &str, const string &suffix);

// TODO: eliminar
// Returns true if str2 is included in str1
bool includes(const string &str1, const string &str2);

// TODO: eliminar
// Returns true if str2 is included in str1
bool includes(const char *str1, const char *str2);

// TODO: eliminar
// Returns true if type is a struct
bool isStruct(const QualType &type);

// TODO: eliminar
// Returns the name of the type, which must be a struct
string getStructName(const QualType &type);

// TODO: eliminar
// Returns true if type is a enum
bool isEnum(const QualType &);

#endif
