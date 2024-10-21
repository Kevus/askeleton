#pragma once

#include <string>
#include <vector>

/**
 * @brief Check if a file exists.
 *
 * This function checks if a file exists given a path.
 *
 * @param filename The path to the file.
 * @return true if the file exists, false otherwise.
 */
bool fileExists(const std::string &filename);

/**
 * @brief Check if a folder exists.
 *
 * This function checks if a folder exists given a path.
 *
 * @param folder The path to the folder.
 * @return true if the folder exists, false otherwise.
 */
bool folderExists(const std::string &folder);

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
 * @brief Stop the program and display an error message.
 *
 * This function displays an error message and stops the program.
 * It is used to handle critical errors that prevent the program from
 * continuing.
 *
 * @param message The error message to display.
 * @see exit
 */
void exitWithError(const std::string &message);

/**
 * Returns a string representation of the current date and time in the format
 * "dd-mm-yyyy hh:mm:ss".
 *
 * @return A string representation of the current date and time.
 */
std::string getTodayString();

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
