#include "ASKGen.hpp"
#include "ASKMatchers.hpp"
#include "color.h"
#include "framework/Generator.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

json &config = getConfig();

void exitIfFilesDoNotExist() {
    fs::path fileSystemPath = getAskeletonHome() / config["system_files"];
    ifstream file(fileSystemPath);
    json filesToCheck;

    if (!fileExists(fileSystemPath))
        exitWithError("ERROR: File not found. Check " + fileSystemPath.string());

    if (!file.is_open())
        exitWithError("Error opening file: " + fileSystemPath.string());

    file >> filesToCheck;

    for (auto &file : filesToCheck) {
        string fileString = file;
        if (!fileExists(fileString))
            exitWithError("ERROR: File not found. Check " + fileString);
        // else
        //     cout << "File checked: " << fileString << endl;
    }
}

std::optional<Framework> checkFramework(std::string framework) {
    framework = toLower(framework);
    if (set<string>({"gtest", "googletest", "google test", "google"}).contains(framework))
        return Framework::GTEST;
    else if (set<string>({"boost", "boost.test", "boosttest", "boost test"})
                 .contains(framework))
        return Framework::BOOST;
    else if (set<string>({"catch", "catch2", "catch.test", "catchtest", "catch test"})
                 .contains(framework))
        return Framework::CATCH;
    else
        return std::nullopt;
}

void exitIfNotValidFramework(std::optional<Framework> framework) {
    if (!framework) {
        exitWithError("ERROR: Invalid framework option. Please use one of the "
                      "following: gtest, boost, catch\n");
    }
}

void selectFrameworkFromOption(Framework framework) {
    setFramework(framework);
    llvm::outs() << "Generating test for " << ANSI_BOLD;
    switch (framework) {
    case Framework::GTEST:
        llvm::outs() << "Google Test";
        break;
    case Framework::BOOST:
        llvm::outs() << "Boost.Test";
        break;
    case Framework::CATCH:
        llvm::outs() << "Catch2";
        break;
    }
    llvm::outs() << ANSI_RESET << " framework\n";
}

void exitIfFolderDoesNotExist(fs::path folder) {
    if (!fs::exists(folder))
        exitWithError("ERROR: Folder not found. Check " + folder.string());
}

void moveGeneratedFolderToLog() {
    fs::path utFolder = getAskeletonHome() / config["route"]["generated"];
    if (fs::exists(utFolder)) {
        fs::path logFolder = getAskeletonHome() / config["route"]["log"];
        if (!fs::exists(logFolder)) {
            create_directory(logFolder);
            llvm::outs() << ANSI_BLUE << "Log folder created at " << logFolder
                         << ANSI_RESET << "\n";
        }

        logFolder /= (config["route"]["generated"].get<string>() + "_" +
                      getTodayString("%d%m%Y_%H%M%S"));
        rename(utFolder, logFolder);
        llvm::outs() << "Previous generated folder moved to " << logFolder << "\n";
    }
}

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::extrahelp
    MoreHelp("\nIf you are working with C++ headers use the option -xc++ at "
             "the end.\nAuthor: Kevin J. Valle-Gomez (kevin.valle@uca.es)\n");
static llvm::cl::OptionCategory OptC("ASkeleTon - Unit Test Generator for C/C++");

cl::opt<std::string> FrameworkOption(
    "framework", cl::desc("Choose the testing framework (options: gtest, boost, catch)"),
    cl::value_desc("framework"), cl::init("gtest"), cl::cat(OptC));

cl::opt<unsigned> DeepLevel("deep-level", cl::desc("Specify the maximum depth level"),
                            cl::value_desc("level"), cl::init(1), cl::cat(OptC));

int main(int argc, const char **argv) {
    system("");

    if (argc == 1) {
        llvm::outs() << "No arguments provided. Use --help for more information\n";
        return 1;
    }

    Expected<CommonOptionsParser> options = CommonOptionsParser::create(argc, argv, OptC);

    if (!options) {
        llvm::errs() << options.takeError();
        return 1;
    }

    std::optional<Framework> selectedFramework = checkFramework(FrameworkOption);
    exitIfNotValidFramework(selectedFramework);
    selectFrameworkFromOption(selectedFramework.value());

    exitIfFolderDoesNotExist(getAskeletonHome() / config["route"]["templates"]);
    llvm::outs() << "Checking ASkeleTon files...\n";
    exitIfFilesDoNotExist();
    llvm::outs() << ANSI_GREEN << "Files checked successfully\n" << ANSI_RESET;

    Generator::MAX_DEPTH = DeepLevel.getValue();

    moveGeneratedFolderToLog();

    fs::create_directories(getAskeletonHome() / config["route"]["ut"]);

    llvm::outs() << ANSI_BOLD << "-------------------------------------\n"
                 << ANSI_BOLD_BLUE << "   Starting ASkeleTon UT Generator   \n"
                 << ANSI_RESET << ANSI_BOLD << "-------------------------------------\n"
                 << ANSI_RESET;

    clang::ast_matchers::MatchFinder Finder;
    ASKGen Functionality;
    for (auto i : createMapMatchers())
        Finder.addMatcher(i.second, &Functionality);

    ClangTool Tool(options->getCompilations(), options->getSourcePathList());
    return Tool.run(newFrontendActionFactory(&Finder).get());
}
