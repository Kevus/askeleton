#include "ASKGen.hpp"
#include "ASKMatchers.hpp"
#include "color.h"
#include "framework/Generator.hpp"
#include "ConfigGenerator.hpp"
#include "Report.hpp"
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
cl::opt<bool> RuleDataOption(
    "rule-data",
    cl::desc("Generate basic rule-based data from simple AST comparisons"),
    cl::init(false), cl::cat(OptC));
cl::opt<unsigned> RuleMaxCasesOption(
    "rule-max-cases",
    cl::desc("Max number of rule-based cases to generate per function (default: 3)"),
    cl::init(3), cl::cat(OptC));
cl::opt<int> SeedOption(
    "seed",
    cl::desc("Seed for deterministic data generation (>= 0 enables it)"),
    cl::init(-1), cl::cat(OptC));
cl::opt<std::string> ProfileOption(
    "profile",
    cl::desc("Data generation profile (random, boundary, safe, stress)"),
    cl::init("random"), cl::cat(OptC));
cl::opt<bool> NoSystemFilesRefresh(
    "no-system-files-refresh",
    cl::desc("Do not refresh system_files.json before running"),
    cl::init(false), cl::cat(OptC));
cl::opt<std::string> ReportPathOption(
    "report",
    cl::desc("Write generation report to JSON file at the given path"),
    cl::value_desc("path"), cl::init(""), cl::cat(OptC));
cl::opt<bool> ReportJsonOption(
    "report-json", cl::desc("Write generation report to a default JSON path"),
    cl::init(false), cl::cat(OptC));

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

    if (!NoSystemFilesRefresh.getValue()) {
        refreshSystemFiles(true);
    }
    exitIfFolderDoesNotExist(getAskeletonHome() / config["route"]["templates"]);
    llvm::outs() << "Checking ASkeleTon files...\n";
    exitIfFilesDoNotExist();
    llvm::outs() << ANSI_GREEN << "Files checked successfully\n" << ANSI_RESET;

    Generator::MAX_DEPTH = DeepLevel.getValue();
    ConfigGenerator::setProfile(ProfileOption.getValue());
    if (SeedOption.getValue() >= 0) {
        ConfigGenerator::setSeed(static_cast<uint32_t>(SeedOption.getValue()));
    }
    if (SeedOption.getValue() >= 0) {
        llvm::outs() << "Data generation: profile=" << ProfileOption.getValue()
                     << " seed=" << SeedOption.getValue() << "\n";
    } else {
        llvm::outs() << "Data generation: profile=" << ProfileOption.getValue()
                     << "\n";
    }

    moveGeneratedFolderToLog();

    fs::create_directories(getAskeletonHome() / config["route"]["ut"]);

    llvm::outs() << ANSI_BOLD << "-------------------------------------\n"
                 << ANSI_BOLD_BLUE << "   Starting ASkeleTon UT Generator   \n"
                 << ANSI_RESET << ANSI_BOLD << "-------------------------------------\n"
                 << ANSI_RESET;

    std::string reportPath;
    if (!ReportPathOption.empty()) {
        reportPath = ReportPathOption;
    } else if (ReportJsonOption) {
        reportPath = (getAskeletonHome() / config["route"]["ut"] /
                      "askeleton_report.json")
                         .string();
    }

    Report report;
    if (!reportPath.empty()) {
        ReportMetadata meta;
        meta.generated_at = getTodayString("%Y-%m-%d %H:%M:%S");
        meta.profile = ProfileOption.getValue();
        if (SeedOption.getValue() >= 0) {
            meta.seed = static_cast<uint32_t>(SeedOption.getValue());
        }
        meta.rule_data = RuleDataOption.getValue();
        meta.rule_max_cases = RuleMaxCasesOption.getValue();
        if (selectedFramework.value() == Framework::GTEST) {
            meta.framework = "gtest";
        } else if (selectedFramework.value() == Framework::BOOST) {
            meta.framework = "boost";
        } else {
            meta.framework = "catch";
        }
        meta.sources = options->getSourcePathList();
        report.setMetadata(meta);
    }

    clang::ast_matchers::MatchFinder Finder;
    ASKGen Functionality(RuleDataOption, RuleMaxCasesOption,
                         reportPath.empty() ? nullptr : &report);
    for (auto i : createMapMatchers(RuleDataOption))
        Finder.addMatcher(i.second, &Functionality);

    ClangTool Tool(options->getCompilations(), options->getSourcePathList());
    int result = Tool.run(newFrontendActionFactory(&Finder).get());
    if (!reportPath.empty()) {
        std::filesystem::path outPath = reportPath;
        std::filesystem::create_directories(outPath.parent_path());
        report.write(reportPath);
    }
    return result;
}
