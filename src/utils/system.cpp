#include "utils/system.hpp"
#include "color.h"
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

bool fileExists(const string &filename) {
    return fs::exists(filename) && fs::is_regular_file(filename);
}

bool folderExists(const string &folder) {
    return fs::exists(folder) && fs::is_directory(folder);
}

string extractFileName(const fs::path &fileRoute) { return fileRoute.stem(); }

void exitWithError(const string &message) {
    cerr << message << "\n";
    cerr << "\nFatal error. Exiting...\n" << ANSI_RESET;
    exit(EXIT_FAILURE);
}

string getTodayString(const string &format) {
    ostringstream oss;
    auto t = time(nullptr);
    auto tm = *localtime(&t);

    oss << put_time(&tm, format.c_str());

    return oss.str();
}

void refreshSystemFiles(bool force) {
    const json &config = getConfig();
    fs::path askeletonHome = getAskeletonHome();
    fs::path outputPath = askeletonHome / config["system_files"].get<string>();

    if (!force && fs::exists(outputPath)) {
        return;
    }

    std::vector<std::string> frameworks;
    if (config.contains("frameworks") && config["frameworks"].is_array()) {
        for (const auto &fw : config["frameworks"]) {
            frameworks.push_back(fw.get<std::string>());
        }
    }
    if (frameworks.empty()) {
        frameworks = {"boost", "catch2", "gtest"};
    }

    std::vector<std::string> files;

    const auto &templateFiles = config["file"]["template"];
    for (auto it = templateFiles.begin(); it != templateFiles.end(); ++it) {
        const std::string key = it.key();
        const auto &value = it.value();

        if (key == "cfg_tpl") {
            files.push_back(
                (askeletonHome / "data" / "templates" / value.get<string>())
                    .string());
            continue;
        }

        if (key == "method" && value.is_object()) {
            for (auto methodIt = value.begin(); methodIt != value.end(); ++methodIt) {
                files.push_back(
                    (askeletonHome / "data" / "templates" /
                     methodIt.value().get<string>())
                        .string());
            }
            continue;
        }

        if (value.is_object()) {
            for (const auto &fw : frameworks) {
                for (auto subIt = value.begin(); subIt != value.end(); ++subIt) {
                    files.push_back(
                        (askeletonHome / "data" / "templates" / fw /
                         subIt.value().get<string>())
                            .string());
                }
            }
            continue;
        }

        if (value.is_string()) {
            for (const auto &fw : frameworks) {
                files.push_back(
                    (askeletonHome / "data" / "templates" / fw /
                     value.get<string>())
                        .string());
            }
        }
    }

    const auto &dataFiles = config["file"]["data"];
    for (auto it = dataFiles.begin(); it != dataFiles.end(); ++it) {
        files.push_back((askeletonHome / it.value().get<string>()).string());
    }

    json output = files;
    fs::create_directories(outputPath.parent_path());
    std::ofstream file(outputPath);
    if (file.is_open()) {
        file << output.dump(4) << "\n";
    }
}

string readFromFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open())
        showOpenFileError(filePath);

    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void writeToFile(const std::string &filePath, const std::string &content) {
    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
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
    cerr << ANSI_RED << "Error opening file: " << filePath << "\n";
    cerr << strerror(errno) << ANSI_RESET << "\n";
}

bool checkAskeletonHome() {
    const char *envHome = getenv("ASKELETON_HOME");
    bool isValid = true;

    if (!envHome || strcmp(envHome, "") == 0) {
        std::cerr << "WARNING: ASKELETON_HOME is not set.\n";
        isValid = false;
    } else {
        fs::path home = envHome;
        if (!fs::exists(home)) {
            std::cerr << "WARNING: ASKELETON_HOME is set to a non-existent folder.\n";
            std::cerr << "Current value: " << home << "\n";
            isValid = false;
        } else if (!fs::is_directory(home)) {
            std::cerr << "WARNING: ASKELETON_HOME is set but is not a directory.\n";
            std::cerr << "Current value: " << home << "\n";
            isValid = false;
        }
    }

    if (!isValid) {
        std::cerr << "Please set the ASKELETON_HOME environment variable to a valid "
                     "directory.\n";
        std::cerr << "ASkeleTon will use the current directory as the home directory.\n";
    }

    return isValid;
}

fs::path getAskeletonHome() {
    static fs::path home;

    if (home.empty()) {
        if (checkAskeletonHome())
            home = fs::path(getenv("ASKELETON_HOME"));
        else
            home = fs::current_path();
    }

    return home;
}

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
    return getFileWithExtensions(filePath, {".cpp", ".c", ".cc"});
}

optional<string> getHeaderFile(const string &filePath) {
    return getFileWithExtensions(filePath, {".hpp", ".h", ".hh", ".hxx"});
}

json &getTemplateItems() {
    static json templateItems;

    if (templateItems.empty()) {
        string tplItemFile = getConfig()["file"]["data"]["template_items"];
        fs::path tplItemPath = getAskeletonHome() / tplItemFile;

        ifstream file(tplItemPath);
        if (!file.is_open())
            exitWithError("Error opening file: " + tplItemPath.string());

        file >> templateItems;
    }

    return templateItems;
}

json &getConfig() {
    static json configData;
    static bool initialized = false;

    if (!initialized) {
        std::filesystem::path configPath = getAskeletonHome() / "data/configuration.json";

        if (!fileExists(configPath))
            throw std::runtime_error("Failed to open config file: " +
                                     configPath.string());

        std::ifstream file(configPath);
        if (!file.is_open())
            throw std::runtime_error("Failed to open config file: " +
                                     configPath.string());

        try {
            file >> configData;
        } catch (const json::parse_error &e) {
            throw std::runtime_error(
                "JSON parsing error in config file: " + configPath.string() +
                "\nError: " + std::string(e.what()));
        }

        initialized = true;
    }

    return configData;
}

void setFramework(Framework framework) {
    json &configData = getConfig();
    configData["framework"] = static_cast<int>(framework);
}

Framework getFramework() {
    json &configData = getConfig();
    try {
        return static_cast<Framework>(configData["framework"].get<int>());
    } catch (const json::exception &e) {
        cerr << "Error accessing framework configuration: " << e.what() << endl;
    }

    return {};
}
