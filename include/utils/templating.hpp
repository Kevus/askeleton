#pragma once

#include <filesystem>
#include <map>
#include <optional>
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
 *
 * @param sToReplace The string to be cleaned.
 * @return A new string with the unnecessary characters removed or replaced.
 */
std::string removeNamespaceQualifier(std::string sToReplace);

/**
 * @brief Removes template arguments from a given type string.
 *
 * This function takes a string representing a type and removes any template
 * arguments, returning the base type. For example, given "std::vector<int>",
 * it will return "std::vector".
 *
 * @param type The type string from which to remove template arguments.
 * @return A string representing the type without template arguments.
 */
std::string removeTemplateArguments(std::string type);

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

/**
 * @brief Creates a file from a template by replacing tokens with specified
 * values.
 *
 * This function reads the content of a template file, replaces tokens in the
 * content with the corresponding values provided in the replacements map, and
 * writes the modified content to an output file.
 *
 * @param templateFilePath The path to the template file.
 * @param outputFilePath The path where the output file will be created.
 * @param replacements A map containing token-value pairs for replacement in the
 * template.
 */
void createFileFromTemplate(
    const std::filesystem::path &templateFilePath,
    const std::filesystem::path &outputFilePath,
    const std::map<std::string, std::string> &replacements = {});

/**
 * @brief Generates a test object for a given target.
 *
 * This function generates a test object for a given target. The test object is
 * a string that contains the target name with the first letter in lowercase.
 *
 * @param target The class target for which to generate the test object.
 * @return std::string The generated test object.
 */
std::string generateTestObjectForTarget(const std::string &target);

std::optional<std::string> getEquivalentType(const std::string &type);
