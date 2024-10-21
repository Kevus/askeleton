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