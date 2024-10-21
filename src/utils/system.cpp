#include "utils/system.hpp"
#include "utils/strings.hpp"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <vector>

using namespace std;

string ASKELETON_HOME;

bool fileExists(const string &filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

bool folderExists(const string &folder) {
    struct stat info;

    if (stat(folder.c_str(), &info) != 0)
        return false;
    else
        return (info.st_mode & S_IFDIR);
}

string extractFileName(const string &fileRoute) {
    unsigned first = fileRoute.find_last_of("/\\") + 1;
    unsigned last = fileRoute.find_last_of(".");

    return fileRoute.substr(first, last - first);
}

void exitWithError(const string &message) {
    cerr << message << "\n";
    cerr << "Error code: " << errno << " - " << strerror(errno) << "\n";
    cerr << "\nFatal error. Exiting...\n";
    exit(EXIT_FAILURE);
}

string getTodayString() {
    ostringstream oss;
    auto t = time(nullptr);
    auto tm = *localtime(&t);

    oss << put_time(&tm, "%d-%m-%Y %H:%M:%S");

    return oss.str();
}

std::string createPath(const std::vector<std::string> &parts, bool isFile) {
    std::string path;

    if (!parts.empty() && parts[0][0] == '/')
        path = "/";

    for (const std::string &part : parts) {
        path += part + "/";
    }

    replaceAll(path, "//", "/");
    if (isFile)
        path.pop_back();
    return path;
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

void setAskeletonHome(const string &path) { ASKELETON_HOME = path; }

string getAskeletonHome() { return ASKELETON_HOME; }

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