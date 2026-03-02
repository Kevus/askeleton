#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

enum Framework { BOOST, CATCH, GTEST };

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
std::string extractFileName(const std::filesystem::path &fileRoute);

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
std::string getTodayString(const std::string &format = "%d-%m-%Y %H:%M:%S");

/**
 * @brief Read the content of a file.
 *
 * This function reads the entire content of the file at the specified path and
 * returns it as a string.
 *
 * @param filePath The path to the file to read.
 * @return std::string The content of the file.
 */
std::string readFromFile(const std::string &filePath);

/**
 * @brief Write content to a file.
 *
 * This function writes the specified content to the file at the specified path.
 * If the file already exists, its content will be overwritten.
 *
 * @param filePath The path to the file where the content will be written.
 * @param content The content to write to the file.
 */
void writeToFile(const std::string &filePath, const std::string &content);

/**
 * @brief Append content to a file.
 *
 * This function appends the specified content to the end of the file at the
 * specified path.
 *
 * @param filePath The path to the file where the content will be appended.
 * @param content The content to append to the file.
 */
void appendToFile(const std::string &filePath, const std::string &content);

/**
 * @brief Displays an error message when a file cannot be opened.
 *
 * This function outputs an error message to the standard error stream
 * indicating that a file could not be opened. It also provides the
 * specific error message associated with the failure.
 *
 * @param filePath The path of the file that could not be opened.
 *
 * @see errno
 */
void showOpenFileError(const std::string &filePath);

/**
 * @brief Get the path to the user's ASkeleTon path.
 *
 * This function returns the path to the user's ASkeleTon path.
 *
 * @return path The path to the user's ASkeleTon path.
 */
std::filesystem::path getAskeletonHome();
void refreshSystemFiles(bool force = true);

/**
 * @brief Searches for a file with one of the specified extensions.
 *
 * This function takes a base file path and a list of extensions, and checks if
 * a file with any of the given extensions exists. If such a file is found, its
 * full path is returned.
 *
 * @param filePath The base file path without extension.
 * @param extensions A vector of possible file extensions to check.
 * @return std::optional<std::string> The full path of the file if found,
 * otherwise std::nullopt.
 */
std::optional<std::string>
getFileWithExtensions(const std::string &filePath,
                      const std::vector<std::string> &extensions);

/**
 * @brief Retrieves the source file with specific extensions from the given file
 * path.
 *
 * This function attempts to find a source file with either a ".cpp" or ".c"
 * extension based on the provided file path.
 *
 * @param filePath The path to the file without the extension.
 * @return std::optional<string> The path to the source file if found, otherwise
 * std::nullopt.
 */
std::optional<std::string> getSourceFile(const std::string &filePath);

/**
 * @brief Retrieves the header file associated with a given file path.
 *
 * This function attempts to find a header file by appending common header
 * file extensions (e.g., .hpp, .h) to the provided file path and checking
 * for their existence.
 *
 * @param filePath The base file path for which to find the header file.
 * @return An optional string containing the path to the header file if found,
 *         or std::nullopt if no header file is found.
 */
std::optional<std::string> getHeaderFile(const std::string &filePath);

/**
 * @brief Retrieves the template items.
 *
 * This function returns the template items that have been loaded from the JSON
 * file.
 *
 * @return The template items as a JSON object.
 */
nlohmann::json &getTemplateItems();

/**
 * @brief Sets the framework to be used.
 *
 * This function assigns the specified framework to the global
 * variable SELECTED_FRAMEWORK, which determines the framework
 * that will be used by the system.
 *
 * @param framework The framework to be set.
 */
void setFramework(Framework framework);

/**
 * @brief Retrieves the currently selected framework.
 *
 * This function returns the framework that has been selected for use.
 *
 * @return Framework The selected framework.
 */
Framework getFramework();

/**
 * @brief Retrieves the configuration data.
 *
 * This function returns the configuration data that has been loaded from the
 * configuration file.
 *
 * @return The configuration data as a JSON object.
 */
nlohmann::json &getConfig();
