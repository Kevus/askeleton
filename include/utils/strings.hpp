#pragma once

#include <string>

#include "clang/AST/Decl.h"

/**
 * @brief Deletes all characters in a string before a specified character.
 *
 * This function searches for the first occurrence of a specified character in a
 * string and deletes all characters before (and including) that character. If
 * the character is not found, the original string is returned unchanged.
 *
 * @param sToReplace The input string to be modified.
 * @param cToFind The character to search for.
 * @return A new string with all characters before the specified character
 * removed.
 */
std::string deleteAllBeforeChar(std::string sToReplace, char cToFind);

/**
 * @brief Replace all occurrences of a substring in a string.
 *
 * This function replaces all occurrences of a substring in a string with a new
 * substring.
 *
 * @param str The string to modify.
 * @param from The substring to replace.
 * @param to The new substring.
 */
void replaceAll(std::string &str, const std::string &from,
                const std::string &to);

/**
 * Joins a vector of strings into a single string with a delimiter.
 *
 * @param vec The vector of strings to join.
 * @param delimiter The character to use as a delimiter.
 * @return The joined string.
 */
std::string join(const std::vector<std::string> &vec, char delimiter);

/**
 * @brief Extracts a substring until a specified character is encountered.
 *
 * This function takes a string and a character as input. It finds the
 * position of the specified character in the string and returns the
 * substring from the beginning of the string up to (but not including) that
 * character. If the character is not found, the entire original string is
 * returned.
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

/**
 * @brief Check if a string includes another string.
 *
 * This function checks if a string includes another string.
 *
 * @param str1 The string to search within.
 * @param str2 The string to search for.
 * @return True if the string is found, false otherwise.
 */
bool includes(const std::string &str1, const std::string &str2);

/**
 * @brief Check if a string includes another string.
 *
 * This function checks if a string includes another string.
 *
 * @param str1 The string to search within.
 * @param str2 The string to search for.
 * @return True if the string is found, false otherwise.
 */
bool includes(const char *str1, const char *str2);

/**
 * @brief Convert a string to lowercase.
 *
 * This function converts a string to lowercase.
 *
 * @param str The string to convert.
 * @return The lowercase version of the input string.
 */
std::string toLower(const std::string &str);

/**
 * @brief Convert a string to uppercase.
 *
 * This function converts a string to uppercase.
 *
 * @param str The string to convert.
 * @return The uppercase version of the input string.
 */
std::string toUpper(const std::string &str);