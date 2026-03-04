#include "utils/system.hpp"
#include "color.h"
#include "utils/strings.hpp"

#include <array>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <vector>
#include <unistd.h>

using namespace std;
namespace fs = filesystem;
using json = nlohmann::json;

namespace {

fs::path detectExecutableHome() {
    std::error_code ec;
    fs::path executable = fs::read_symlink("/proc/self/exe", ec);
    if (ec || executable.empty())
        return {};

    fs::path home = executable.parent_path();
    if (!home.empty() && fs::exists(home / "data" / "configuration.json"))
        return home;

    return {};
}

bool readJsonFile(const fs::path &filePath, json &output, std::string &error) {
    if (!fileExists(filePath.string())) {
        error = "ERROR: File not found. Check " + filePath.string();
        return false;
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        error = "Error opening file: " + filePath.string();
        return false;
    }

    try {
        file >> output;
        return true;
    } catch (const json::parse_error &e) {
        error = "JSON parsing error in file: " + filePath.string() +
                "\nError: " + std::string(e.what());
        return false;
    }
}

bool writeTextFileAtomicallyImpl(const fs::path &filePath, const std::string &content) {
    if (!filePath.parent_path().empty()) {
        std::error_code ec;
        fs::create_directories(filePath.parent_path(), ec);
        if (ec) {
            return false;
        }
    }

    fs::path tempPath = filePath;
    tempPath += ".tmp.";
    tempPath += std::to_string(::getpid());

    {
        std::ofstream file(tempPath, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            return false;
        }
        file << content;
        if (!file.good()) {
            file.close();
            std::error_code ec;
            fs::remove(tempPath, ec);
            return false;
        }
    }

    std::error_code ec;
    fs::rename(tempPath, filePath, ec);
    if (!ec) {
        return true;
    }

    if (fs::exists(filePath)) {
        fs::remove(filePath, ec);
        if (ec) {
            std::error_code cleanupEc;
            fs::remove(tempPath, cleanupEc);
            return false;
        }
        fs::rename(tempPath, filePath, ec);
        if (!ec) {
            return true;
        }
    }

    std::error_code cleanupEc;
    fs::remove(tempPath, cleanupEc);
    return false;
}

std::string_view frameworkTemplateDirImpl(Framework framework) {
    switch (framework) {
    case Framework::BOOST:
        return "/data/templates/boost/";
    case Framework::CATCH:
        return "/data/templates/catch2/";
    case Framework::GTEST:
        return "/data/templates/gtest/";
    }
    return "";
}

bool isTemplateFileForOtherFramework(const std::string &path, Framework framework) {
    static const std::array<std::string, 3> templateDirs = {
        "/data/templates/boost/",
        "/data/templates/catch2/",
        "/data/templates/gtest/",
    };

    const std::string_view currentDir = frameworkTemplateDirImpl(framework);
    for (const auto &dir : templateDirs) {
        if (path.find(dir) == std::string::npos) {
            continue;
        }
        return dir != currentDir;
    }
    return false;
}

} // namespace

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

    std::set<std::string> files;

    const auto &templateFiles = config["file"]["template"];
    for (auto it = templateFiles.begin(); it != templateFiles.end(); ++it) {
        const std::string key = it.key();
        const auto &value = it.value();

        if (key == "cfg_tpl") {
            files.insert(
                (askeletonHome / "data" / "templates" / value.get<string>())
                    .string());
            continue;
        }

        if (key == "method" && value.is_object()) {
            for (auto methodIt = value.begin(); methodIt != value.end(); ++methodIt) {
                files.insert(
                    (askeletonHome / "data" / "templates" /
                     methodIt.value().get<string>())
                        .string());
            }
            continue;
        }

        if (value.is_object()) {
            for (const auto &fw : frameworks) {
                for (auto subIt = value.begin(); subIt != value.end(); ++subIt) {
                    files.insert(
                        (askeletonHome / "data" / "templates" / fw /
                         subIt.value().get<string>())
                            .string());
                }
            }
            continue;
        }

        if (value.is_string()) {
            for (const auto &fw : frameworks) {
                files.insert(
                    (askeletonHome / "data" / "templates" / fw /
                     value.get<string>())
                        .string());
            }
        }
    }

    const auto &dataFiles = config["file"]["data"];
    for (auto it = dataFiles.begin(); it != dataFiles.end(); ++it) {
        files.insert((askeletonHome / it.value().get<string>()).string());
    }

    json output = json::array();
    for (const auto &path : files) {
        output.push_back(path);
    }
    (void)writeJsonFileAtomically(outputPath, output, 4);
}

string readFromFile(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open())
        showOpenFileError(filePath);

    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void writeToFile(const std::string &filePath, const std::string &content) {
    if (!writeTextFileAtomically(filePath, content)) {
        showOpenFileError(filePath);
    }
}

bool writeTextFileAtomically(const fs::path &filePath, const std::string &content) {
    return writeTextFileAtomicallyImpl(filePath, content);
}

bool writeJsonFileAtomically(const fs::path &filePath, const json &content,
                             int indent) {
    return writeTextFileAtomicallyImpl(filePath, content.dump(indent) + "\n");
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
        else {
            home = detectExecutableHome();
            if (home.empty())
                home = fs::current_path();
        }
    }

    return home;
}

std::vector<std::string> getSystemFilesToCheck(Framework framework) {
    const json &config = getConfig();
    fs::path fileSystemPath = getAskeletonHome() / config["system_files"].get<string>();
    json filesToCheck;
    std::string error;
    if (!readJsonFile(fileSystemPath, filesToCheck, error)) {
        exitWithError(error);
    }

    if (!filesToCheck.is_array()) {
        exitWithError("ERROR: Invalid system files manifest format. Check " +
                      fileSystemPath.string());
    }

    std::vector<std::string> result;
    result.reserve(filesToCheck.size());
    for (const auto &file : filesToCheck) {
        if (!file.is_string()) {
            continue;
        }
        const std::string fileString = file.get<std::string>();
        if (isTemplateFileForOtherFramework(fileString, framework)) {
            continue;
        }
        result.push_back(fileString);
    }
    return result;
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
    static const vector<string> headerExtensions = {".hpp", ".h", ".hh", ".hxx"};

    if (auto direct = getFileWithExtensions(filePath, headerExtensions)) {
        return direct;
    }

    fs::path inputPath = fs::absolute(filePath);
    const string stem = inputPath.stem().string();

    fs::path current = inputPath.parent_path();
    while (!current.empty()) {
        fs::path includeDir = current / "include";
        if (fs::exists(includeDir) && fs::is_directory(includeDir)) {
            std::optional<std::string> best;
            for (const auto &entry : fs::recursive_directory_iterator(includeDir)) {
                if (!entry.is_regular_file()) {
                    continue;
                }
                const fs::path candidate = entry.path();
                if (candidate.stem() != stem) {
                    continue;
                }
                if (std::find(headerExtensions.begin(), headerExtensions.end(),
                              candidate.extension().string()) == headerExtensions.end()) {
                    continue;
                }
                if (!best.has_value() ||
                    candidate.string().size() < best->size()) {
                    best = candidate.string();
                }
            }
            if (best.has_value()) {
                return best;
            }
        }

        if (current == current.root_path()) {
            break;
        }
        current = current.parent_path();
    }

    return nullopt;
}

json &getTemplateItems() {
    static json templateItems;

    if (templateItems.empty()) {
        string tplItemFile = getConfig()["file"]["data"]["template_items"];
        fs::path tplItemPath = getAskeletonHome() / tplItemFile;
        std::string error;
        if (!readJsonFile(tplItemPath, templateItems, error)) {
            exitWithError(error);
        }
    }

    return templateItems;
}

json &getConfig() {
    static json configData;
    static bool initialized = false;

    if (!initialized) {
        std::filesystem::path configPath = getAskeletonHome() / "data/configuration.json";
        std::string error;
        if (!readJsonFile(configPath, configData, error)) {
            throw std::runtime_error(error);
        }

        initialized = true;
    }

    return configData;
}

void setFramework(Framework framework) {
    json &configData = getConfig();
    configData["framework"] = static_cast<int>(framework);
}

std::optional<Framework> parseFramework(std::string_view framework) {
    const std::string normalized = toLower(std::string(framework));
    if (set<string>({"gtest", "googletest", "google test", "google"})
            .contains(normalized)) {
        return Framework::GTEST;
    }
    if (set<string>({"boost", "boost.test", "boosttest", "boost test"})
            .contains(normalized)) {
        return Framework::BOOST;
    }
    if (set<string>({"catch", "catch2", "catch.test", "catchtest", "catch test"})
            .contains(normalized)) {
        return Framework::CATCH;
    }
    return std::nullopt;
}

std::string_view frameworkKey(Framework framework) {
    switch (framework) {
    case Framework::GTEST:
        return "gtest";
    case Framework::BOOST:
        return "boost";
    case Framework::CATCH:
        return "catch";
    }
    return "gtest";
}

std::string_view frameworkDisplayName(Framework framework) {
    switch (framework) {
    case Framework::GTEST:
        return "Google Test";
    case Framework::BOOST:
        return "Boost.Test";
    case Framework::CATCH:
        return "Catch2";
    }
    return "Google Test";
}

std::string_view frameworkTemplateDir(Framework framework) {
    return frameworkTemplateDirImpl(framework);
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
