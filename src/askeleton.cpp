#include "ASKGen.hpp"
#include "ASKMatchers.hpp"
#include "color.h"
#include "framework/Generator.hpp"
#include "ConfigGenerator.hpp"
#include "Logging.hpp"
#include "Report.hpp"
#include "RunStats.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <unordered_map>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {

class NormalizedCompilationDatabase : public CompilationDatabase {
  public:
    explicit NormalizedCompilationDatabase(const CompilationDatabase &base) {
        const auto cmds = base.getAllCompileCommands();
        commands_.reserve(cmds.size());
        allCommands_.reserve(cmds.size());
        allFiles_.reserve(cmds.size());

        for (const auto &cmd : cmds) {
            fs::path dir = cmd.Directory;
            fs::path dirAbs = dir.is_relative() ? fs::absolute(dir) : dir;
            fs::path file = cmd.Filename;
            fs::path absFile = file.is_relative() ? (dirAbs / file) : file;
            std::string absFileStr = fs::absolute(absFile).string();

            CompileCommand normalized = cmd;
            normalized.Directory = dirAbs.string();
            normalized.Filename = absFileStr;

            commands_[absFileStr].push_back(normalized);
            allCommands_.push_back(normalized);
            allFiles_.push_back(absFileStr);
        }
    }

    std::vector<CompileCommand> getCompileCommands(StringRef filePath) const override {
        fs::path abs = fs::absolute(filePath.str());
        auto it = commands_.find(abs.string());
        if (it != commands_.end()) {
            return it->second;
        }
        return {};
    }

    std::vector<std::string> getAllFiles() const override { return allFiles_; }

    std::vector<CompileCommand> getAllCompileCommands() const override {
        return allCommands_;
    }

  private:
    std::unordered_map<std::string, std::vector<CompileCommand>> commands_;
    std::vector<CompileCommand> allCommands_;
    std::vector<std::string> allFiles_;
};

} // namespace

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
    if (Logger::instance().level() < LogLevel::Normal)
        return;
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
            if (Logger::instance().level() >= LogLevel::Normal) {
                llvm::outs() << ANSI_BLUE << "Log folder created at " << logFolder
                             << ANSI_RESET << "\n";
            }
        }

        logFolder /= (config["route"]["generated"].get<string>() + "_" +
                      getTodayString("%d%m%Y_%H%M%S"));
        rename(utFolder, logFolder);
        if (Logger::instance().level() >= LogLevel::Normal)
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
cl::opt<std::string> OutDirOption(
    "out-dir",
    cl::desc("Output directory for generated tests (overrides default)"),
    cl::value_desc("path"), cl::init(""), cl::cat(OptC));
cl::opt<bool> IncludeImplUnderInclude(
    "include-impl-under-include",
    cl::desc("Allow compiling .c/.cc/.cpp files under include/ directories"),
    cl::init(false), cl::cat(OptC));
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
cl::opt<bool> QuietOption(
    "quiet", cl::desc("Reduce output to errors only"), cl::init(false), cl::cat(OptC));
cl::opt<bool> VerboseOption(
    "verbose", cl::desc("Increase verbosity"), cl::init(false), cl::cat(OptC));
cl::opt<bool> DebugOption(
    "debug", cl::desc("Enable debug verbosity"), cl::init(false), cl::cat(OptC));
cl::opt<std::string> LogJsonOption(
    "log-json", cl::desc("Write execution log to JSON file at the given path"),
    cl::value_desc("path"), cl::init(""), cl::cat(OptC));

int main(int argc, const char **argv) {
    system("");

    if (argc == 1) {
        llvm::outs() << "No arguments provided. Use --help for more information\n";
        return 1;
    }

    const auto time_start = std::chrono::steady_clock::now();

    Expected<CommonOptionsParser> options = CommonOptionsParser::create(argc, argv, OptC);

    if (!options) {
        llvm::errs() << options.takeError();
        return 1;
    }

    LogLevel level = LogLevel::Normal;
    if (QuietOption)
        level = LogLevel::Quiet;
    if (VerboseOption)
        level = LogLevel::Verbose;
    if (DebugOption)
        level = LogLevel::Debug;
    Logger::instance().setLevel(level);

    std::optional<Framework> selectedFramework = checkFramework(FrameworkOption);
    exitIfNotValidFramework(selectedFramework);
    selectFrameworkFromOption(selectedFramework.value());

    std::optional<long long> refresh_ms;
    if (!NoSystemFilesRefresh.getValue()) {
        auto t0 = std::chrono::steady_clock::now();
        refreshSystemFiles(true);
        auto t1 = std::chrono::steady_clock::now();
        refresh_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    }

    std::string outDir;
    if (!OutDirOption.empty()) {
        outDir = OutDirOption;
    } else {
        const auto &sources = options->getSourcePathList();
        if (!sources.empty()) {
            std::filesystem::path sourcePath = sources.front();
            if (sourcePath.is_relative()) {
                sourcePath = std::filesystem::absolute(sourcePath);
            }
            std::filesystem::path baseDir = sourcePath.parent_path();
            outDir = (baseDir / "tests" / "generated").string();
        } else {
            outDir = (getAskeletonHome() / config["route"]["ut"]).string();
        }
    }

    if (!outDir.empty()) {
        std::filesystem::path outPath = outDir;
        if (outPath.is_relative()) {
            outPath = std::filesystem::absolute(outPath);
        }
        config["route"]["ut"] = outPath.string();
        config["route"]["generated"] = outPath.string();
        config["route"]["log"] = (outPath.parent_path() / "Generated_log").string();
    }

    exitIfFolderDoesNotExist(getAskeletonHome() / config["route"]["templates"]);
    Logger::instance().info("Checking ASkeleTon files...");
    exitIfFilesDoNotExist();
    if (Logger::instance().level() >= LogLevel::Normal)
        llvm::outs() << ANSI_GREEN << "Files checked successfully\n" << ANSI_RESET;

    Generator::MAX_DEPTH = DeepLevel.getValue();
    ConfigGenerator::setProfile(ProfileOption.getValue());
    if (SeedOption.getValue() >= 0) {
        ConfigGenerator::setSeed(static_cast<uint32_t>(SeedOption.getValue()));
    }
    if (Logger::instance().level() >= LogLevel::Normal) {
        if (SeedOption.getValue() >= 0) {
            llvm::outs() << "Data generation: profile=" << ProfileOption.getValue()
                         << " seed=" << SeedOption.getValue() << "\n";
        } else {
            llvm::outs() << "Data generation: profile=" << ProfileOption.getValue()
                         << "\n";
        }
    }

    moveGeneratedFolderToLog();

    fs::create_directories(getAskeletonHome() / config["route"]["ut"]);

    if (Logger::instance().level() >= LogLevel::Normal) {
        llvm::outs() << ANSI_BOLD << "-------------------------------------\n"
                     << ANSI_BOLD_BLUE << "   Starting ASkeleTon UT Generator   \n"
                     << ANSI_RESET << ANSI_BOLD << "-------------------------------------\n"
                     << ANSI_RESET;
    }

    std::string reportPath;
    if (!ReportPathOption.empty()) {
        reportPath = ReportPathOption;
    } else if (ReportJsonOption) {
        reportPath = (getAskeletonHome() / config["route"]["ut"] /
                      "askeleton_report.json")
                         .string();
    }

    RunStats stats;
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

    std::vector<std::string> sources = options->getSourcePathList();
    const auto &compdb = options->getCompilations();
    NormalizedCompilationDatabase normalizedCompdb(compdb);
    if (!IncludeImplUnderInclude.getValue()) {
        std::vector<std::string> filtered;
        for (const auto &src : sources) {
            std::filesystem::path p = src;
            std::filesystem::path absPath = p;
            if (absPath.is_relative()) {
                absPath = std::filesystem::absolute(absPath);
            }
            const std::string ext = p.extension().string();
            if ((ext == ".c" || ext == ".cc" || ext == ".cpp") &&
                absPath.string().find("/include/") != std::string::npos) {
                llvm::outs()
                    << "Skipping " << absPath.string()
                    << " (implementation under include/). Use "
                       "--include-impl-under-include to force.\n";
                continue;
            }
            filtered.push_back(src);
        }
        sources = std::move(filtered);
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
            meta.sources = sources;
            report.setMetadata(meta);
        }
    }

    std::vector<std::string> sourcesForTool;
    sourcesForTool.reserve(sources.size());
    for (const auto &src : sources) {
        fs::path abs = fs::path(src);
        if (abs.is_relative())
            abs = fs::absolute(abs);
        sourcesForTool.push_back(abs.string());
    }

    clang::ast_matchers::MatchFinder Finder;
    ASKGen Functionality(RuleDataOption, RuleMaxCasesOption,
                         reportPath.empty() ? nullptr : &report, &stats);
    for (auto i : createMapMatchers(RuleDataOption))
        Finder.addMatcher(i.second, &Functionality);

    ClangTool Tool(normalizedCompdb, sourcesForTool);
    auto tool_start = std::chrono::steady_clock::now();
    int result = Tool.run(newFrontendActionFactory(&Finder).get());
    auto tool_end = std::chrono::steady_clock::now();
    if (!reportPath.empty()) {
        std::filesystem::path outPath = reportPath;
        std::filesystem::create_directories(outPath.parent_path());
        report.write(reportPath);
    }

    Logger::instance().printWarningsSummary();

    if (Logger::instance().level() >= LogLevel::Normal) {
        llvm::outs() << "\nSummary:\n";
        llvm::outs() << "  Found: " << stats.found << "\n";
        llvm::outs() << "  Generated: " << stats.generated << "\n";
        llvm::outs() << "  Skipped: " << stats.skipped << "\n";
        if (!stats.skipped_by_reason.empty()) {
            llvm::outs() << "  Skipped by reason:\n";
            for (const auto &it : stats.skipped_by_reason) {
                llvm::outs() << "    - " << it.first << ": " << it.second << "\n";
            }
        }
        llvm::outs() << "  Output: " << config["route"]["ut"].get<std::string>() << "\n";
        if (!reportPath.empty()) {
            llvm::outs() << "  Report: " << reportPath << "\n";
        }
        auto tool_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(tool_end - tool_start)
                .count();
        llvm::outs() << "  Time: " << tool_ms << " ms";
        if (refresh_ms.has_value())
            llvm::outs() << " (system files: " << refresh_ms.value() << " ms)";
        llvm::outs() << "\n";
    }

    if (!LogJsonOption.empty()) {
        json log;
        log["summary"] = {
            {"found", stats.found},
            {"generated", stats.generated},
            {"skipped", stats.skipped},
            {"skipped_by_reason", stats.skipped_by_reason},
        };
        log["by_kind"] = stats.by_kind;
        log["by_target"] = stats.by_target;
        log["warnings"]["missing_source"] =
            std::vector<std::string>(Logger::instance().missingSourceFiles().begin(),
                                     Logger::instance().missingSourceFiles().end());
        log["warnings"]["missing_header"] =
            std::vector<std::string>(Logger::instance().missingHeaderFiles().begin(),
                                     Logger::instance().missingHeaderFiles().end());
        log["files_seen"] =
            std::vector<std::string>(Logger::instance().filesSeen().begin(),
                                     Logger::instance().filesSeen().end());
        auto tool_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(tool_end - tool_start)
                .count();
        log["timings_ms"]["tool_run"] = tool_ms;
        if (refresh_ms.has_value())
            log["timings_ms"]["system_files_refresh"] = refresh_ms.value();
        auto total_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(tool_end - time_start)
                .count();
        log["timings_ms"]["total"] = total_ms;

        std::filesystem::path logPath = LogJsonOption.getValue();
        std::filesystem::create_directories(logPath.parent_path());
        std::ofstream out(logPath);
        if (out.is_open())
            out << log.dump(2) << "\n";
    }
    return result;
}
