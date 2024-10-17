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
 * @brief Extracts a substring until a specified character is encountered.
 *
 * This function takes a string and a character as input. It finds the position
 * of the specified character in the string and returns the substring from the
 * beginning of the string up to (but not including) that character.
 * If the character is not found, the entire original string is returned.
 *
 * @param str The input string.
 * @param c The character to search for.
 * @return The substring until the specified character.
 */
std::string extractSubstringUntilCharacter(const std::string &str, char c);

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
 * the file, excluding the extension and path. The function is platform-agnostic
 * and can be used on operating systems with different path conventions.
 *
 * @param fileRoute Full path of the file to be processed.
 * @return std::string The file name without the extension or path.
 */
string extractFileName(const string &fileRoute);

/**
 * @brief Remove specified type qualifiers from the given C++ type.
 *
 * This function removes the specified type qualifiers, such as "const," "enum,"
 * "class," and "struct," from the provided C++ type string.
 *
 * @param type The C++ type string from which to remove qualifiers.
 */
void removeTypeQualifiers(string &type);

/**
 * @brief Replace certain characters in the given C++ type.
 *
 * This function replaces '*' with 's' and '&' with 'r' in the provided C++ type
 * string.
 *
 * @param type The C++ type string in which to replace characters.
 */
void replaceTypeCharacters(string &type);

/**
 * @brief Remove leading whitespaces from the given string.
 *
 * This function removes leading whitespaces from the provided string by erasing
 * characters until a non-whitespace character is encountered.
 *
 * @param s The string from which to remove leading whitespaces.
 */
void ltrim(std::string &s);

/**
 * @brief Remove trailing whitespaces from the given string.
 *
 * This function removes trailing whitespaces from the provided string by
 * erasing characters starting from the end until a non-whitespace character is
 * encountered.
 *
 * @param s The string from which to remove trailing whitespaces.
 */
void rtrim(std::string &s);

/**
 * @brief Stop the program and display an error message.
 *
 * This function displays an error message and stops the program.
 * It is used to handle critical errors that prevent the program from
 * continuing.
 *
 * @param message The error message to display.
 * @see exit
 */
void exitWithError(const string &message);

/**
 * Returns a string representation of the current date and time in the format
 * "dd-mm-yyyy hh:mm:ss".
 *
 * @return A string representation of the current date and time.
 */
std::string getTodayString();

/**
 * @brief Split a string into a vector of substrings using a delimiter.
 *
 * This function splits a string into a vector of substrings using the specified
 * delimiter character. The delimiter character is not included in the resulting
 * substrings.
 *
 * @param s The string to split.
 * @param delimiter The character used to split the string.
 * @return A vector of substrings.
 */
std::vector<std::string> split(const std::string &s, char delimiter);

/**
 * @brief Create a path from a list of parts.
 *
 * This function concatenates a list of strings to create a path. The parts are
 * joined with a forward slash ('/') character. If the first part of the path
 * starts with a forward slash, the resulting path will also start with a
 * forward slash.
 *
 * @param parts A vector of strings representing the parts of the path.
 * @param isFile A boolean value indicating whether the path represents a file.
 * @return The concatenated path.
 */
std::string createPath(const std::vector<std::string> &parts,
                       bool isFile = false);

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
