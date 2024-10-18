#pragma once

#include "clang/AST/Decl.h"
#include <string>

/**
 * @brief Check if a string is numeric.
 *
 * This function checks if a string is numeric. It does so by attempting to
 * convert the string to a double. If the conversion is successful, the function
 * returns true. Otherwise, it checks if the string contains a character that
 * indicates a non-numeric value, such as a single quote ('), double quote ("),
 * or the strings "true" or "false". If any of these conditions are met, the
 * function returns true. Otherwise, it returns false.
 *
 * @param query The string to check.
 * @return true if the string is numeric, false otherwise.
 */
std::string getCommentHeader(std::string filename);

/**
 * @brief Check if a parameter is in a list of parameters.
 *
 * This function checks if a parameter with the specified name is present in a
 * list of parameters. If the parameter is found, the function saves the type of
 * the parameter in the provided string reference and returns true. Otherwise,
 * it returns false.
 *
 * @param name The name of the parameter to search for.
 * @param params The list of parameters to search in.
 * @param type A reference to a string where the type of the parameter will be
 * saved.
 * @return true if the parameter is found, false otherwise.
 */
std::string deleteAllBeforeChar(std::string sToReplace, char cToFind);

/**
 * @brief Check if a parameter is in a list of parameters.
 *
 * This function checks if a parameter with the specified name is present in a
 * list of parameters. If the parameter is found, the function saves the type of
 * the parameter in the provided string reference and returns true. Otherwise,
 * it returns false.
 *
 * @param name The name of the parameter to search for.
 * @param params The list of parameters to search in.
 * @param type A reference to a string where the type of the parameter will be
 * saved.
 * @return true if the parameter is found, false otherwise.
 */
std::string cleanUnnecesaryChars(std::string sToReplace);

/**
 * @brief Check if a parameter is in a list of parameters.
 *
 * This function checks if a parameter with the specified name is present in a
 * list of parameters. If the parameter is found, the function saves the type of
 * the parameter in the provided string reference and returns true. Otherwise,
 * it returns false.
 *
 * @param name The name of the parameter to search for.
 * @param params The list of parameters to search in.
 * @param type A reference to a string where the type of the parameter will be
 * saved.
 * @return true if the parameter is found, false otherwise.
 */
std::string convertExpressionToString(clang::Expr *E, clang::SourceManager &SM);

/**
 * @brief Check if a parameter is in a list of parameters.
 *
 * This function checks if a parameter with the specified name is present in a
 * list of parameters. If the parameter is found, the function saves the type of
 * the parameter in the provided string reference and returns true. Otherwise,
 * it returns false.
 *
 * @param name The name of the parameter to search for.
 * @param params The list of parameters to search in.
 * @param type A reference to a string where the type of the parameter will be
 * saved.
 * @return true if the parameter is found, false otherwise.
 */
void replaceAll(std::string &str, const std::string &from,
                const std::string &to);

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
void removeAll(std::string &originalString,
               const std::string &substringToRemove);

/**
 * @brief Removes all occurrences of specified substrings from the given string.
 *
 * This function iterates over the list of substrings and removes all
 * occurrences of each substring from the provided string.
 *
 * @param str The string from which to remove the substrings.
 * @param substrings The list of substrings to remove.
 */
void removeAll(std::string &originalString,
               const std::initializer_list<std::string> &substringToRemove);

/**
 * @brief Checks if a substring is present in the given string.
 *
 * This function searches for the specified substring within the main string.
 *
 * @param originalString The main string to search within.
 * @param substring The substring to search for.
 * @return True if the substring is found, false otherwise.
 */
bool containsSubstring(const std::string &originalString,
                       const std::string &substring);

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
bool containsAnySubstring(const std::string &originalString,
                          const std::initializer_list<std::string> &substrings);

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
std::string extractFileName(const std::string &fileRoute);

/**
 * @brief Remove specified type qualifiers from the given C++ type.
 *
 * This function removes the specified type qualifiers, such as "const," "enum,"
 * "class," and "struct," from the provided C++ type string.
 *
 * @param type The C++ type string from which to remove qualifiers.
 */
void removeTypeQualifiers(std::string &type);

/**
 * @brief Replace certain characters in the given C++ type.
 *
 * This function replaces '*' with 's' and '&' with 'r' in the provided C++ type
 * string.
 *
 * @param type The C++ type string in which to replace characters.
 */
void replaceTypeCharacters(std::string &type);

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

/**
 * @brief Checks if a given string ends with the specified suffix.
 *
 * This function compares the end of the input string with the provided suffix
 * and returns true if the string ends with the suffix, otherwise false.
 *
 * @param str The input string to be checked.
 * @param suffix The suffix to be checked against the end of the input string.
 * @return true if the input string ends with the specified suffix, false
 * otherwise.
 */
bool endsWith(const std::string &str, const std::string &suffix);