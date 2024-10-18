#pragma once

#include <string>

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