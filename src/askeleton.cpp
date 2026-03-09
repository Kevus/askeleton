#include "ASKGen.hpp"
#include "ASKMatchers.hpp"
#include "color.h"
#include "CoverageMode.hpp"
#include "ConfigGenerator.hpp"
#include "Logging.hpp"
#include "OracleMode.hpp"
#include "Report.hpp"
#include "RunStats.hpp"
#include "framework/Generator.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include <chrono>
#include <algorithm>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <unistd.h>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {

double computeRatio(unsigned numerator, unsigned denominator) {
    if (denominator == 0) {
        return 0.0;
    }
    return static_cast<double>(numerator) / static_cast<double>(denominator);
}

json topCounts(const std::map<std::string, unsigned> &counts, std::size_t limit = 5) {
    std::vector<std::pair<std::string, unsigned>> items(counts.begin(), counts.end());
    std::sort(items.begin(), items.end(),
              [](const auto &lhs, const auto &rhs) {
                  if (lhs.second != rhs.second) {
                      return lhs.second > rhs.second;
                  }
                  return lhs.first < rhs.first;
              });

    json result = json::array();
    const std::size_t count = std::min(limit, items.size());
    for (std::size_t i = 0; i < count; ++i) {
        result.push_back({{"name", items[i].first}, {"count", items[i].second}});
    }
    return result;
}

class NormalizedCompilationDatabase : public CompilationDatabase {
  public:
    explicit NormalizedCompilationDatabase(const CompilationDatabase &base) {
        const auto cmds = base.getAllCompileCommands();
        commands_.reserve(cmds.size());
        allCommands_.reserve(cmds.size());
        allFiles_.reserve(cmds.size());
        std::unordered_set<std::string> seenFiles;

        for (const auto &cmd : cmds) {
            fs::path dir = cmd.Directory;
            fs::path dirAbs = dir.is_relative() ? fs::absolute(dir) : dir;
            fs::path file = cmd.Filename;
            fs::path absFile = file.is_relative() ? (dirAbs / file) : file;
            std::string absFileStr = fs::absolute(absFile).string();

            CompileCommand normalized = cmd;
            normalized.Directory = dirAbs.string();
            normalized.Filename = absFileStr;

            if (seenFiles.insert(absFileStr).second) {
                commands_[absFileStr].push_back(normalized);
                allCommands_.push_back(normalized);
                allFiles_.push_back(absFileStr);
            }
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

static bool shouldKeepCompileArg(llvm::StringRef arg) {
    if (arg.empty() || arg == "-c" || arg == "-o") {
        return false;
    }
    return arg.starts_with("-I") || arg.starts_with("-isystem") || arg.starts_with("-D") ||
           arg.starts_with("-iquote") || arg.starts_with("-stdlib=") ||
           arg.starts_with("-fPIC") ||
           arg.starts_with("-fvisibility") || arg.starts_with("-pthread") ||
           arg == "-include";
}

static std::string absolutizePathIfRelative(llvm::StringRef value,
                                            const fs::path &baseDir) {
    fs::path p(value.str());
    if (p.empty() || p.is_absolute()) {
        return value.str();
    }
    return fs::absolute(baseDir / p).string();
}

static std::string normalizePathFlagValue(llvm::StringRef option,
                                          llvm::StringRef value,
                                          const fs::path &baseDir) {
    return option.str() + absolutizePathIfRelative(value, baseDir);
}

static std::string shellEscapeArg(llvm::StringRef arg) {
    std::string s = arg.str();
    if (s.find_first_of(" \t\"'\\$`()") == std::string::npos) {
        return s;
    }
    std::string escaped = "'";
    for (char c : s) {
        if (c == '\'') {
            escaped += "'\\''";
        } else {
            escaped.push_back(c);
        }
    }
    escaped.push_back('\'');
    return escaped;
}

static std::map<std::string, std::string> collectCompileFlagsBySourcePath(
    const CompilationDatabase &compdb, const std::vector<std::string> &sourcesForTool) {
    std::map<std::string, std::string> result;

    for (const auto &source : sourcesForTool) {
        const auto commands = compdb.getCompileCommands(source);
        if (commands.empty()) {
            continue;
        }

        const auto &cmd = commands.front();
        fs::path cmdDir = cmd.Directory;
        if (cmdDir.is_relative()) {
            cmdDir = fs::absolute(cmdDir);
        }
        std::vector<std::string> kept;
        for (std::size_t i = 1; i < cmd.CommandLine.size(); ++i) {
            const std::string &arg = cmd.CommandLine[i];
            if (!shouldKeepCompileArg(arg)) {
                continue;
            }

            if (arg == "-I" || arg == "-isystem" || arg == "-iquote" || arg == "-include") {
                if (i + 1 < cmd.CommandLine.size()) {
                    kept.push_back(shellEscapeArg(arg));
                    const std::string value =
                        absolutizePathIfRelative(cmd.CommandLine[++i], cmdDir);
                    kept.push_back(shellEscapeArg(value));
                }
                continue;
            }

            if (arg.rfind("-I", 0) == 0 && arg.size() > 2) {
                kept.push_back(shellEscapeArg(
                    normalizePathFlagValue("-I", llvm::StringRef(arg).substr(2), cmdDir)));
                continue;
            }

            if (arg.rfind("-isystem", 0) == 0 && arg.size() > 8) {
                kept.push_back(shellEscapeArg(normalizePathFlagValue(
                    "-isystem", llvm::StringRef(arg).substr(8), cmdDir)));
                continue;
            }

            if (arg.rfind("-iquote", 0) == 0 && arg.size() > 7) {
                kept.push_back(shellEscapeArg(
                    normalizePathFlagValue("-iquote", llvm::StringRef(arg).substr(7), cmdDir)));
                continue;
            }

            if (arg.rfind("-include", 0) == 0 && arg.size() > 8) {
                kept.push_back(shellEscapeArg(normalizePathFlagValue(
                    "-include", llvm::StringRef(arg).substr(8), cmdDir)));
                continue;
            }

            kept.push_back(shellEscapeArg(arg));
        }

        std::ostringstream ss;
        for (std::size_t i = 0; i < kept.size(); ++i) {
            if (i > 0) {
                ss << " ";
            }
            ss << kept[i];
        }
        result[source] = ss.str();
    }

    return result;
}

static std::map<std::string, std::vector<std::string>> collectCompanionSourcesBySourcePath(
    const CompilationDatabase &compdb, const std::vector<std::string> &sourcesForTool) {
    std::unordered_map<std::string, std::vector<std::string>> groups;
    for (const auto &cmd : compdb.getAllCompileCommands()) {
        std::string source = cmd.Filename;
        std::string groupKey = source;
        if (!cmd.Output.empty()) {
            groupKey = fs::path(cmd.Output).parent_path().string();
        }
        groups[groupKey].push_back(source);
    }

    std::map<std::string, std::vector<std::string>> result;
    for (const auto &source : sourcesForTool) {
        const auto commands = compdb.getCompileCommands(source);
        if (commands.empty()) {
            continue;
        }
        const auto &cmd = commands.front();
        std::string groupKey = source;
        if (!cmd.Output.empty()) {
            groupKey = fs::path(cmd.Output).parent_path().string();
        }

        std::vector<std::string> companions;
        for (const auto &candidate : groups[groupKey]) {
            if (candidate != source) {
                companions.push_back(candidate);
            }
        }
        result[source] = companions;
    }

    return result;
}

static std::optional<std::string> findBuildPathArg(int argc, const char **argv) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "-p" || a == "--build-path") && i + 1 < argc) {
            return std::string(argv[i + 1]);
        }
        if (a.rfind("--build-path=", 0) == 0) {
            return a.substr(std::string("--build-path=").size());
        }
        if (a.rfind("-p=", 0) == 0) {
            return a.substr(std::string("-p=").size());
        }
    }
    return std::nullopt;
}

static bool hasFlagArg(int argc, const char **argv, llvm::StringRef flag) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == flag) {
            return true;
        }
    }
    return false;
}

static fs::path compdbFilePathFromArgsOrCwd(int argc, const char **argv) {
    auto bp = findBuildPathArg(argc, argv);
    fs::path dir = bp ? fs::path(*bp) : fs::current_path();
    if (dir.is_relative()) dir = fs::absolute(dir);
    return dir / "compile_commands.json";
}

class ScopedStderrSilencer {
  public:
    ScopedStderrSilencer() {
        nullFd_ = ::open("/dev/null", O_WRONLY);
        if (nullFd_ == -1) {
            return;
        }

        savedFd_ = ::dup(STDERR_FILENO);
        if (savedFd_ == -1) {
            ::close(nullFd_);
            nullFd_ = -1;
            return;
        }

        if (::dup2(nullFd_, STDERR_FILENO) == -1) {
            ::close(savedFd_);
            ::close(nullFd_);
            savedFd_ = -1;
            nullFd_ = -1;
        }
    }

    ~ScopedStderrSilencer() {
        if (savedFd_ != -1) {
            ::dup2(savedFd_, STDERR_FILENO);
            ::close(savedFd_);
        }
        if (nullFd_ != -1) {
            ::close(nullFd_);
        }
    }

    ScopedStderrSilencer(const ScopedStderrSilencer &) = delete;
    ScopedStderrSilencer &operator=(const ScopedStderrSilencer &) = delete;

  private:
    int savedFd_ = -1;
    int nullFd_ = -1;
};

static bool shouldSuppressCompdbProbeNoise(int argc, const char **argv) {
    if (!hasFlagArg(argc, argv, "--bootstrap-compdb")) {
        return false;
    }
    return !fs::exists(compdbFilePathFromArgsOrCwd(argc, argv));
}

static std::vector<std::string> minimalCompileArgumentsForFile(
    const fs::path &absFile) {
    const std::string ext = absFile.extension().string();
    // Minimal but reasonable defaults for "single-file" bootstrapping.
    if (ext == ".c") {
        return {"clang", "-std=c11", "-I.", "-c", absFile.string()};
    }
    // default to C++
    return {"clang++", "-std=c++20", "-I.", "-c", absFile.string()};
}

static std::optional<std::string> parseProfileName(std::string_view value) {
    std::string normalized(value);
    normalized = toLower(normalized);

    if (normalized == "random" || normalized == "boundary" ||
        normalized == "safe" || normalized == "stress") {
        return normalized;
    }
    return std::nullopt;
}

static std::optional<fs::path>
findNearestCompilationDatabasePath(const std::vector<std::string> &sourcesForTool) {
    for (const auto &src : sourcesForTool) {
        fs::path current = fs::absolute(src).parent_path();
        while (!current.empty()) {
            fs::path candidate = current / "compile_commands.json";
            if (fs::exists(candidate)) {
                return candidate;
            }
            if (current == current.root_path()) {
                break;
            }
            current = current.parent_path();
        }
    }
    return std::nullopt;
}

static bool loadCompilationDatabaseJson(const fs::path &compdbPath, json &db) {
    db = json::array();
    // Load existing compile_commands.json if present and valid.
    if (fs::exists(compdbPath)) {
        try {
            std::ifstream in(compdbPath);
            in >> db;
            if (!db.is_array()) {
                return false;
            }
        } catch (...) {
            // If file is corrupt/unreadable, do NOT destroy it silently.
            return false;
        }
    }
    return true;
}

static bool hasCompileCommandEntry(const json &db, const fs::path &absFile) {
    for (const auto &entry : db) {
        if (!entry.is_object()) continue;
        if (entry.contains("file") && entry["file"].is_string()) {
            fs::path entryFile = fs::path(entry["file"].get<std::string>());
            if (entryFile.is_relative() && entry.contains("directory") &&
                entry["directory"].is_string()) {
                entryFile = fs::path(entry["directory"].get<std::string>()) / entryFile;
            }
            if (fs::absolute(entryFile) == absFile) return true;
        }
    }
    return false;
}

static void appendMinimalCompileCommandEntry(json &db, const fs::path &absFile) {
    json e;
    e["directory"] = absFile.parent_path().string();
    e["arguments"] = minimalCompileArgumentsForFile(absFile);
    e["file"] = absFile.string();
    db.push_back(e);
}

static bool writeCompilationDatabaseJson(const fs::path &compdbPath, const json &db) {
    // Backup existing compdb before writing, if it existed.
    if (fs::exists(compdbPath)) {
        fs::path bak = compdbPath;
        bak += ".bak";
        std::error_code ec;
        fs::copy_file(compdbPath, bak, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            return false;
        }
    }

    try {
        return writeJsonFileAtomically(compdbPath, db, 2);
    } catch (...) {
        return false;
    }
}

static bool ensureCompileCommandEntries(const fs::path &compdbPath,
                                        const std::vector<fs::path> &absFiles) {
    json db;
    if (!loadCompilationDatabaseJson(compdbPath, db)) {
        return false;
    }

    bool changed = false;
    for (const auto &absFile : absFiles) {
        if (hasCompileCommandEntry(db, absFile)) {
            continue;
        }
        appendMinimalCompileCommandEntry(db, absFile);
        changed = true;
    }

    if (!changed) {
        return true;
    }
    return writeCompilationDatabaseJson(compdbPath, db);
}

} // namespace

json &config = getConfig();

void exitIfFilesDoNotExist() {
    for (const auto &fileString : getSystemFilesToCheck(getFramework())) {
        if (!fileExists(fileString))
            exitWithError("ERROR: File not found. Check " + fileString);
    }
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
    llvm::outs() << frameworkDisplayName(framework);
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

        const fs::path generatedPath = config["route"]["generated"].get<string>();
        const std::string baseName = generatedPath.filename().string();
        fs::path archivePath =
            logFolder / (baseName + "_" + getTodayString("%d%m%Y_%H%M%S"));
        for (unsigned suffix = 1; fs::exists(archivePath); ++suffix) {
            archivePath = logFolder /
                          (baseName + "_" + getTodayString("%d%m%Y_%H%M%S") + "_" +
                           std::to_string(suffix));
        }
        rename(utFolder, archivePath);
        if (Logger::instance().level() >= LogLevel::Normal)
            llvm::outs() << "Previous generated folder moved to " << archivePath
                         << "\n";
    }
}

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::extrahelp
    MoreHelp("\nIf you are working with C++ headers use the option -xc++ at "
             "the end.\nAuthor: Kevin J. Valle-Gomez (kevin.valle@uca.es)\n");
static cl::extrahelp DetailedHelp(R"(

CLI GUIDE (DETAILED)
====================

Quick examples:
  askeleton -p build src/foo.cpp
  askeleton -p build --framework=boost src/foo.cpp
  askeleton -p build --coverage-mode=strict --report=out/report.json src/foo.cpp
  askeleton -p build --oracle-mode=property --seed=123 src/foo.cpp
  askeleton -p build --rule-data --rule-max-cases=5 src/foo.cpp
  askeleton --bootstrap-compdb -p . ./sut.cpp

Option details:

  -p <build-path>
    Path to compile_commands.json directory. Required unless discoverable.

  --framework=<gtest|boost|catch>  (default: gtest)
    Selects the generated test framework templates and assertions.

  --profile=<random|boundary|safe|stress>  (default: random)
    Controls data generation style:
      random   -> broad random sampling
      boundary -> emphasizes edge-like values
      safe     -> conservative values
      stress   -> larger/more extreme values

  --coverage-mode=<balanced|strict|aggressive>  (default: balanced)
    Controls generation policy:
      balanced   -> default practical generation
      strict     -> conservative skips for risky/mutable construction paths
      aggressive -> forward-compatible permissive mode (currently close to balanced)

  --oracle-mode=<explicit|mirror|property>  (default: explicit)
    Controls expected-value strategy:
      explicit -> read expected from cfg if present, otherwise mirror replay
      mirror   -> derive expected from isolated replay
      property -> repeatability-oriented replay oracle

  --rule-data / --no-rule-data  (default: enabled)
    Enable/disable AST-guided rule-based candidate values from comparisons.

  --rule-max-cases=<N>  (default: 3)
    Maximum rule-based generated cases per function when rule-data is active.

  --seed=<N>
    Enables deterministic generation when N >= 0.

  --out-dir=<path>
    Overrides default output folder for generated tests.

  --report=<path>
    Writes generation report JSON to the given path.

  --report-json
    Writes generation report JSON to: <out-dir>/askeleton_report.json

  --log-json=<path>
    Writes execution log JSON (counts/warnings/timings) to the given path.

  --quiet | --verbose | --debug
    Console verbosity controls (errors only / extra progress / detailed debug).

  --deep-level=<level>  (default: 1)
    Sets maximum generation depth used by internal generators.

  --extra-arg=<arg>
    Appends one extra compile flag for Clang tooling.
    Example: --extra-arg=-Wno-unused-parameter

  --extra-arg-before=<arg>
    Prepends one extra compile flag for Clang tooling.
    Example: --extra-arg-before=-DUSE_DEMO=1

  --include-impl-under-include
    Allows .c/.cc/.cpp under include/ to be processed (normally filtered out).

  --no-system-files-refresh
    Skips automatic system_files.json refresh/check update step.

  --bootstrap-compdb
    If an input source has no compile_commands entry, append a minimal one and
    continue. Useful for single-file demos/quick starts.

Notes:
  - For full CLI/manual documentation with examples, see doc/CLI.md.
  - For skip reason semantics and fixes, see doc/SkipReasons.md.
)");
static llvm::cl::OptionCategory OptC("ASkeleTon - Unit Test Generator for C/C++");

cl::opt<std::string> FrameworkOption(
    "framework", cl::desc("Choose the testing framework (options: gtest, boost, catch)"),
    cl::value_desc("framework"), cl::init("gtest"), cl::cat(OptC));

cl::opt<unsigned> DeepLevel("deep-level", cl::desc("Specify the maximum depth level"),
                            cl::value_desc("level"), cl::init(1), cl::cat(OptC));
cl::opt<bool> RuleDataOption(
    "rule-data",
    cl::desc("Generate basic rule-based data from simple AST comparisons"),
    cl::init(true), cl::cat(OptC));
cl::opt<bool> NoRuleDataOption(
    "no-rule-data",
    cl::desc("Disable the default AST-guided rule-based data generation"),
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
cl::opt<std::string> CoverageModeOption(
    "coverage-mode",
    cl::desc("Coverage policy (strict, balanced, aggressive)"),
    cl::init("balanced"), cl::cat(OptC));
cl::opt<std::string> OracleModeOption(
    "oracle-mode",
    cl::desc("Oracle strategy (mirror, explicit, property)"),
    cl::init("explicit"), cl::cat(OptC));
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
cl::opt<bool> BootstrapCompdbOption(
    "bootstrap-compdb",
    cl::desc("If a source file has no entry in compile_commands.json, create/append a minimal one automatically."),
    cl::init(false), cl::cat(OptC));

static ReportMetadata buildReportMetadata(Framework framework,
                                          CoverageMode coverageMode,
                                          OracleMode oracleMode,
                                          const std::string &profileName,
                                          bool ruleDataEnabled,
                                          const std::vector<std::string> &sources) {
    ReportMetadata meta;
    meta.generated_at = getTodayString("%Y-%m-%d %H:%M:%S");
    meta.profile = profileName;
    meta.coverage_mode = coverageModeName(coverageMode);
    meta.oracle_mode = oracleModeName(oracleMode);
    if (SeedOption.getValue() >= 0) {
        meta.seed = static_cast<uint32_t>(SeedOption.getValue());
    }
    meta.rule_data = ruleDataEnabled;
    meta.rule_max_cases = RuleMaxCasesOption.getValue();
    meta.framework = std::string(frameworkKey(framework));
    meta.sources = sources;
    return meta;
}

static void configureLoggingFromOptions() {
    LogLevel level = LogLevel::Normal;
    if (QuietOption)
        level = LogLevel::Quiet;
    if (VerboseOption)
        level = LogLevel::Verbose;
    if (DebugOption)
        level = LogLevel::Debug;
    Logger::instance().setLevel(level);
}

static std::string resolveOutputDirectory(const CommonOptionsParser &options) {
    if (!OutDirOption.empty()) {
        return OutDirOption;
    }

    const auto &sources = options.getSourcePathList();
    if (!sources.empty()) {
        std::filesystem::path sourcePath = sources.front();
        if (sourcePath.is_relative()) {
            sourcePath = std::filesystem::absolute(sourcePath);
        }
        return (sourcePath.parent_path() / "tests" / "generated").string();
    }

    return (getAskeletonHome() / config["route"]["ut"]).string();
}

static void applyOutputDirectoryOverride(const std::string &outDir) {
    if (outDir.empty()) {
        return;
    }

    std::filesystem::path outPath = outDir;
    if (outPath.is_relative()) {
        outPath = std::filesystem::absolute(outPath);
    }
    config["route"]["ut"] = outPath.string();
    config["route"]["generated"] = outPath.string();
    config["route"]["log"] = (outPath.parent_path() / "Generated_log").string();
}

static std::string resolveReportPath() {
    if (!ReportPathOption.empty()) {
        return ReportPathOption;
    }
    if (ReportJsonOption) {
        return (getAskeletonHome() / config["route"]["ut"] / "askeleton_report.json")
            .string();
    }
    return "";
}

static void printRunSummary(const RunStats &stats, const std::string &reportPath,
                            CoverageMode coverageMode, OracleMode oracleMode,
                            const std::string &profileName,
                            const std::chrono::steady_clock::time_point &tool_start,
                            const std::chrono::steady_clock::time_point &tool_end,
                            const std::optional<long long> &refresh_ms) {
    if (Logger::instance().level() < LogLevel::Normal) {
        return;
    }

    llvm::outs() << "\nSummary:\n";
    llvm::outs() << "  Found: " << stats.found << "\n";
    llvm::outs() << "  Generated: " << stats.generated << "\n";
    llvm::outs() << "  Skipped: " << stats.skipped << "\n";
    std::ostringstream generationRate;
    generationRate << std::fixed << std::setprecision(2)
                   << (computeRatio(stats.generated, stats.found) * 100.0) << "%";
    llvm::outs() << "  Generation rate: " << generationRate.str() << "\n";
    if (!stats.skipped_by_reason.empty()) {
        llvm::outs() << "  Skipped by reason:\n";
        for (const auto &it : stats.skipped_by_reason) {
            llvm::outs() << "    - " << it.first << ": " << it.second << "\n";
        }
    }
    llvm::outs() << "  Coverage mode: " << coverageModeName(coverageMode) << "\n";
    llvm::outs() << "  Oracle mode: " << oracleModeName(oracleMode) << "\n";
    llvm::outs() << "  Profile: " << profileName << "\n";
    llvm::outs() << "  Output: " << config["route"]["ut"].get<std::string>() << "\n";
    if (!reportPath.empty()) {
        llvm::outs() << "  Report: " << reportPath << "\n";
    }
    const auto tool_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(tool_end - tool_start)
            .count();
    llvm::outs() << "  Time: " << tool_ms << " ms";
    if (refresh_ms.has_value()) {
        llvm::outs() << " (system files: " << refresh_ms.value() << " ms)";
    }
    llvm::outs() << "\n";
}

static void writeExecutionLogJson(
    const RunStats &stats, const std::chrono::steady_clock::time_point &time_start,
    const std::chrono::steady_clock::time_point &tool_start,
    const std::chrono::steady_clock::time_point &tool_end,
    const std::optional<long long> &refresh_ms) {
    if (LogJsonOption.empty()) {
        return;
    }

    json log;
    log["summary"] = {
        {"found", stats.found},
        {"generated", stats.generated},
        {"skipped", stats.skipped},
        {"skipped_by_reason", stats.skipped_by_reason},
        {"generation_rate", computeRatio(stats.generated, stats.found)},
        {"skip_rate", computeRatio(stats.skipped, stats.found)},
        {"top_skip_reasons", topCounts(stats.skipped_by_reason)},
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
    log["timings_ms"]["tool_run"] =
        std::chrono::duration_cast<std::chrono::milliseconds>(tool_end - tool_start)
            .count();
    if (refresh_ms.has_value()) {
        log["timings_ms"]["system_files_refresh"] = refresh_ms.value();
    }
    log["timings_ms"]["total"] =
        std::chrono::duration_cast<std::chrono::milliseconds>(tool_end - time_start)
            .count();

    const std::filesystem::path logPath = LogJsonOption.getValue();
    if (!logPath.parent_path().empty()) {
        std::filesystem::create_directories(logPath.parent_path());
    }
    (void)writeJsonFileAtomically(logPath, log, 2);
}

int main(int argc, const char **argv) {
    system("");

    if (argc == 1) {
        llvm::outs() << "No arguments provided. Use --help for more information\n";
        return 1;
    }

    const auto time_start = std::chrono::steady_clock::now();

    std::optional<ScopedStderrSilencer> silentProbe;
    if (shouldSuppressCompdbProbeNoise(argc, argv)) {
        silentProbe.emplace();
    }

    Expected<CommonOptionsParser> options = CommonOptionsParser::create(argc, argv, OptC);
    silentProbe.reset();

    if (!options) {
        llvm::errs() << options.takeError();
        return 1;
    }

    configureLoggingFromOptions();

    std::optional<Framework> selectedFramework = parseFramework(FrameworkOption);
    exitIfNotValidFramework(selectedFramework);
    selectFrameworkFromOption(selectedFramework.value());
    std::optional<CoverageMode> coverageMode = parseCoverageMode(CoverageModeOption);
    if (!coverageMode.has_value()) {
        exitWithError("ERROR: Invalid coverage mode. Please use one of the "
                      "following: strict, balanced, aggressive\n");
    }
    std::optional<OracleMode> selectedOracleMode = parseOracleMode(OracleModeOption);
    if (!selectedOracleMode.has_value()) {
        exitWithError("ERROR: Invalid oracle mode. Please use one of the "
                      "following: mirror, explicit, property\n");
    }
    std::optional<std::string> profileName = parseProfileName(ProfileOption.getValue());
    if (!profileName.has_value()) {
        exitWithError("ERROR: Invalid profile. Please use one of the "
                      "following: random, boundary, safe, stress\n");
    }
    const bool ruleDataEnabled = RuleDataOption.getValue() && !NoRuleDataOption.getValue();
    const OracleMode resolvedOracleMode =
        effectiveOracleMode(selectedOracleMode.value());

    std::optional<long long> refresh_ms;
    if (!NoSystemFilesRefresh.getValue()) {
        auto t0 = std::chrono::steady_clock::now();
        // Regenerate only when missing to avoid machine-local churn on every run.
        refreshSystemFiles(false);
        auto t1 = std::chrono::steady_clock::now();
        refresh_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    }

    applyOutputDirectoryOverride(resolveOutputDirectory(*options));

    exitIfFolderDoesNotExist(getAskeletonHome() / config["route"]["templates"]);
    Logger::instance().info("Checking ASkeleTon files...");
    exitIfFilesDoNotExist();
    if (Logger::instance().level() >= LogLevel::Normal)
        llvm::outs() << ANSI_GREEN << "Files checked successfully\n" << ANSI_RESET;

    Generator::MAX_DEPTH = DeepLevel.getValue();
    ConfigGenerator::setProfile(profileName.value());
    ConfigGenerator::setOracleMode(selectedOracleMode.value());
    Generator::setOracleMode(selectedOracleMode.value());
    if (SeedOption.getValue() >= 0) {
        ConfigGenerator::setSeed(static_cast<uint32_t>(SeedOption.getValue()));
    }
    if (Logger::instance().level() >= LogLevel::Normal) {
        if (SeedOption.getValue() >= 0) {
            llvm::outs() << "Data generation: profile=" << profileName.value()
                         << " seed=" << SeedOption.getValue()
                         << " coverage=" << coverageModeName(coverageMode.value())
                         << " oracle=" << oracleModeName(resolvedOracleMode)
                         << "\n";
        } else {
            llvm::outs() << "Data generation: profile=" << profileName.value()
                         << " coverage=" << coverageModeName(coverageMode.value())
                         << " oracle=" << oracleModeName(resolvedOracleMode)
                         << "\n";
        }
        if (selectedOracleMode.value() == OracleMode::Property &&
            resolvedOracleMode != OracleMode::Property) {
            llvm::outs()
                << "Oracle mode 'property' currently falls back to mirror execution.\n";
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

    std::string reportPath = resolveReportPath();

    RunStats stats;
    Report report;
    if (!reportPath.empty()) {
        report.setMetadata(
            buildReportMetadata(selectedFramework.value(), coverageMode.value(),
                                resolvedOracleMode, profileName.value(),
                                ruleDataEnabled,
                                options->getSourcePathList()));
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
            report.setMetadata(
                buildReportMetadata(selectedFramework.value(), coverageMode.value(),
                                    resolvedOracleMode, profileName.value(),
                                    ruleDataEnabled, sources));
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
    ASKGen Functionality(ruleDataEnabled, RuleMaxCasesOption,
                         reportPath.empty() ? nullptr : &report, &stats,
                         coverageMode.value());
    for (auto i : createMapMatchers(ruleDataEnabled))
        Finder.addMatcher(i.second, &Functionality);


    // Pre-check: every source must have a compile command.
    // Otherwise ClangTool will silently skip it and the UX is terrible.
    std::vector<std::string> missing;
    for (const auto &absSrc : sourcesForTool) {
        auto cmds = normalizedCompdb.getCompileCommands(absSrc);
        if (cmds.empty()) {
            missing.push_back(absSrc);
        }
    }

    const CompilationDatabase *toolCompdb = &normalizedCompdb;
    std::unique_ptr<NormalizedCompilationDatabase> reloadedNormalizedCompdb;

    if (!missing.empty()) {
        fs::path compdbPath;
        if (auto bp = findBuildPathArg(argc, argv)) {
            compdbPath = fs::path(*bp) / "compile_commands.json";
        } else if (auto discovered = findNearestCompilationDatabasePath(sourcesForTool)) {
            compdbPath = *discovered;
        } else {
            compdbPath = compdbFilePathFromArgsOrCwd(argc, argv);
        }

        if (BootstrapCompdbOption.getValue()) {
            std::vector<fs::path> missingPaths;
            missingPaths.reserve(missing.size());
            for (const auto &m : missing) {
                fs::path absFile = fs::path(m);
                if (absFile.is_relative()) absFile = fs::absolute(absFile);
                missingPaths.push_back(absFile);
            }

            if (!ensureCompileCommandEntries(compdbPath, missingPaths)) {
                llvm::errs()
                    << "ERROR: --bootstrap-compdb failed. Existing compile_commands.json may be invalid or not writable.\n"
                    << "Tried: " << compdbPath.string() << "\n";
                return 2;
            }

            llvm::outs()
                << "Bootstrapped compilation database at:\n  " << compdbPath.string() << "\n"
                << "Re-loading compilation database and continuing...\n";

            std::string errMsg;
            auto newDb = JSONCompilationDatabase::loadFromFile(compdbPath.string(), errMsg,
                                                              JSONCommandLineSyntax::AutoDetect);
            if (!newDb) {
                llvm::errs()
                    << "ERROR: Could not load compilation database after bootstrapping:\n"
                    << errMsg << "\n";
                return 2;
            }

            reloadedNormalizedCompdb =
                std::make_unique<NormalizedCompilationDatabase>(*newDb);
            toolCompdb = reloadedNormalizedCompdb.get();
        } else {
            llvm::errs()
                << "ERROR: No compile command found for one or more source files.\n\n"
                << "ASkeleTon requires a valid compile_commands.json entry for each file because it uses Clang AST parsing.\n\n"
                << "Missing entries:\n";
            for (const auto &m : missing) {
                llvm::errs() << "  - " << m << "\n";
            }
            llvm::errs()
                << "\nHow to fix:\n"
                << "  • CMake: cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON\n"
                << "  • Make:  bear -- make\n"
                << "  • Quick test (single file): re-run with --bootstrap-compdb\n\n"
                << "Example:\n"
                << "  askeleton -p . sut.cpp --framework=gtest --bootstrap-compdb\n";
            return 2;
        }
    }

    Generator::setCompileFlags(
        collectCompileFlagsBySourcePath(*toolCompdb, sourcesForTool));
    Generator::setCompanionSources(
        collectCompanionSourcesBySourcePath(*toolCompdb, sourcesForTool));

    ClangTool Tool(*toolCompdb, sourcesForTool);
    auto tool_start = std::chrono::steady_clock::now();
    int result = Tool.run(newFrontendActionFactory(&Finder).get());
    auto tool_end = std::chrono::steady_clock::now();
    if (!reportPath.empty()) {
        std::filesystem::path outPath = reportPath;
        if (!outPath.parent_path().empty()) {
            std::filesystem::create_directories(outPath.parent_path());
        }
        if (!report.write(reportPath)) {
            llvm::errs() << "ERROR: Could not write report to " << reportPath << "\n";
            return 2;
        }
    }

    Logger::instance().printWarningsSummary();

    printRunSummary(stats, reportPath, coverageMode.value(), resolvedOracleMode,
                    profileName.value(),
                    tool_start, tool_end,
                    refresh_ms);
    writeExecutionLogJson(stats, time_start, tool_start, tool_end, refresh_ms);
    return result;
}
