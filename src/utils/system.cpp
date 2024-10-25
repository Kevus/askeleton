#include "utils/system.hpp"

#include "Config.hpp"
#include "utils/strings.hpp"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <vector>

using namespace std;
namespace fs = filesystem;
using json = nlohmann::json;

fs::path ASKELETON_HOME;
Framework SELECTED_FRAMEWORK;

bool fileExists(const string &filename) {
    return fs::exists(filename) && fs::is_regular_file(filename);
}

bool folderExists(const string &folder) {
    return fs::exists(folder) && fs::is_directory(folder);
}

string extractFileName(const fs::path &fileRoute) { return fileRoute.stem(); }

void exitWithError(const string &message) {
    cerr << message << "\n";
    cerr << "Error code: " << errno << " - " << strerror(errno) << "\n";
    cerr << "\nFatal error. Exiting...\n";
    exit(EXIT_FAILURE);
}

string getTodayString(const string &format) {
    ostringstream oss;
    auto t = time(nullptr);
    auto tm = *localtime(&t);

    oss << put_time(&tm, format.c_str());

    return oss.str();
}

std::string createPath(const std::vector<std::string> &parts, bool isFile) {
    fs::path result;

    if (!parts.empty() && parts[0][0] == '/')
        result = "/";

    for (const std::string &part : parts) {
        result /= part;
    }

    return result;
}

string readFromFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open())
        showOpenFileError(filePath);

    return {std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
}

void writeToFile(const std::string &filePath, const std::string &content) {
    std::ofstream file(filePath);
    if (!file.is_open())
        showOpenFileError(filePath);

    file << content;
}

void appendToFile(const std::string &filePath, const std::string &content) {
    std::ofstream file(filePath, std::ios_base::app);
    if (!file.is_open())
        showOpenFileError(filePath);

    file << content;
}

void showOpenFileError(const string &filePath) {
    cerr << "Error opening file: " << filePath << "\n";
    cerr << "Error: " << strerror(errno) << "\n";
}

void setAskeletonHome(const fs::path &askPath) { ASKELETON_HOME = askPath; }

fs::path getAskeletonHome() { return ASKELETON_HOME; }

optional<string> getFileWithExtensions(const string &filePath,
                                       const vector<string> &extensions) {
    string basePath = filePath.substr(0, filePath.find_last_of("."));

    for (const auto &ext : extensions) {
        string fullPath = basePath + ext;
        if (fileExists(fullPath)) {
            return fullPath;
        }
    }
    return nullopt;
}

optional<string> getSourceFile(const string &filePath) {
    return getFileWithExtensions(filePath, {".cpp", ".c"});
}

optional<string> getHeaderFile(const string &filePath) {
    return getFileWithExtensions(filePath, {".hpp", ".h"});
}

json loadTemplateItems() {
    string tplItemFile = Config::getInstance().get("file.data.template_items");
    fs::path tplItemPath = getAskeletonHome() / tplItemFile;

    ifstream file(tplItemPath);
    if (!file.is_open())
        exitWithError("Error opening file: " + tplItemPath.string());

    json templateItems;
    file >> templateItems;

    return templateItems;
}

void setFramework(Framework framework) { SELECTED_FRAMEWORK = framework; }

Framework getFramework() { return SELECTED_FRAMEWORK; }