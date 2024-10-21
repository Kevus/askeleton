#pragma once

#include <string>

#include "clang/AST/Decl.h"

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
bool isNumeric(const std::string &query);

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
bool isInParameters(const std::string &name,
                    const llvm::ArrayRef<clang::ParmVarDecl *> &params,
                    std::string &type);