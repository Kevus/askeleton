#pragma once

#include <string>

#include "clang/AST/Expr.h"

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
 * @brief Cleans unnecessary characters from a given string.
 *
 * This function performs the following operations on the input string:
 * - Removes occurrences of "std::".
 * - Removes occurrences of "__cxx11::".
 * - Replaces all spaces with underscores, but only if the string does not
 * contain the character '<'.
 *
 * @param sToReplace The string to be cleaned.
 * @return A new string with the unnecessary characters removed or replaced.
 */
std::string cleanUnnecesaryChars(std::string sToReplace);

/**
 * @brief Convert a clang expression to a string.
 *
 * This function converts a clang expression to a string. It uses the provided
 * source manager to get the character data of the expression and return it as a
 * string.
 *
 * @param E The clang expression to convert.
 * @param SM The source manager to use to get the character data.
 * @return std::string The expression as a string.
 */
std::string convertExpressionToString(clang::Expr *E, clang::SourceManager &SM);

/**
 * @brief Replace all occurrences of a substring in a string.
 *
 * This function replaces all occurrences of a substring in a string with a
 * specified replacement.
 *
 * @param str The string in which to replace substrings.
 * @param from The substring to replace.
 * @param to The replacement for the substring.
 */
void replaceTokensInText(
    std::string &text, const std::map<std::string, std::string> &replacements);

/**
 * @brief Replaces tokens in a file with specified replacements.
 *
 * This function reads the content of the input file, replaces all occurrences
 * of the tokens specified in the replacements map with their corresponding
 * values, and writes the modified content to the output file.
 *
 * @param inputFilePath The path to the input file containing the original
 * content.
 * @param outputFilePath The path to the output file where the modified content
 * will be written.
 * @param replacements A map where the keys are the tokens to be replaced and
 * the values are the replacements.
 */
void replaceTokensInFile(
    const std::string &inputFilePath, const std::string &outputFilePath,
    const std::map<std::string, std::string> &replacements);

/**
 * @brief Replaces tokens in a file with specified replacements.
 *
 * This function reads the content of the input file, replaces all occurrences
 * of the tokens specified in the replacements map with their corresponding
 * values, and returns the modified content.
 *
 * @param inputFilePath The path to the input file containing the original
 * content.
 * @param replacements A map where the keys are the tokens to be replaced and
 * the values are the replacements.
 * @return std::string The modified content.
 */
std::string
replaceTokensInFile(const std::string &inputFilePath,
                    const std::map<std::string, std::string> &replacements);
