#include "utils/system.hpp"
#include "utils/strings.hpp"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <vector>

using namespace std;

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