#include "ASKGen.hpp"
#include "ASKMatchers.hpp"
#include "framework/Generator.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include <ctime>
#include <iostream>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

using namespace std;
namespace fs = std::filesystem;

std::optional<Framework> checkFramework(std::string framework) {
    framework = toLower(framework);
    if (set<string>({"gtest", "googletest", "google test", "google"})
            .contains(framework))
        return Framework::GTEST;
    else if (set<string>({"boost", "boost.test", "boosttest", "boost test"})
                 .contains(framework))
        return Framework::BOOST;
    else if (set<string>(
                 {"catch", "catch2", "catch.test", "catchtest", "catch test"})
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
    switch (framework) {
    case Framework::GTEST:
        cout << "Generating test for Google Test framework" << endl;
        break;
    case Framework::BOOST:
        cout << "Generating test for Boost.Test framework" << endl;
        break;
    case Framework::CATCH:
        cout << "Generating test for Catch2 framework" << endl;
        break;
    }
}

void setAskeletonHomeFromEnv() {
    if (getenv("ASKELETON_HOME") != NULL) {
        setAskeletonHome(getenv("ASKELETON_HOME"));
        cout << "ASKELETON_HOME set to " << getAskeletonHome() << endl;
    } else {
        setAskeletonHome(fs::current_path());
        cerr << "WARNING: ASKELETON_HOME is not set. Templates will not be "
                "accesible unless runing ASKELETON in the compilation "
                "folder.\n";
        cout << "Running ASKELETON from " << getAskeletonHome() << endl;
    }
}

void loadConfigurationFromConfigFile() {
    fs::path configFile = getAskeletonHome() / "data/configuration.json";
    if (!fs::exists(configFile))
        exitWithError("ERROR: Configuration file not found. Check " +
                      configFile.string());

    Config::getInstance().loadConfig(configFile);
    cout << "Configuration loaded from " << configFile << endl;
}

void exitIfFolderDoesNotExist(fs::path folder) {
    if (!fs::exists(folder))
        exitWithError("ERROR: Folder not found. Check " + folder.string());
}

void moveGeneratedFolderToLog(const Config &config) {
    fs::path utFolder = config.get("route.generated");
    if (fs::exists(utFolder)) {
        fs::path logFolder = fs::absolute(config.get("route.log"));
        if (!fs::exists(logFolder)) {
            create_directory(logFolder);
            cout << "Log folder created at " << logFolder << endl;
        }

        logFolder /= (config.get("route.generated") + "_" +
                      getTodayString("%d%m%Y_%H%M%S"));
        rename(utFolder, logFolder);
        cout << "Previous generated folder moved to " << logFolder << endl;
    }
}

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::extrahelp
    MoreHelp("\nIf you are working with C++ headers use the option -xc++ at "
             "the end.\nAuthor: Kevin J. Valle-Gomez (kevin.valle@uca.es)\n");
static llvm::cl::OptionCategory
    OptC("ASkeleTon - Unit Test Generator for C/C++");

cl::opt<std::string> FrameworkOption(
    "framework",
    cl::desc("Choose the testing framework (options: gtest, boost, catch)"),
    cl::value_desc("framework"), cl::init("gtest"), cl::cat(OptC));

cl::opt<unsigned> DeepLevel("deep-level",
                            cl::desc("Specify the maximum depth level"),
                            cl::value_desc("level"), cl::init(1),
                            cl::cat(OptC));

int main(int argc, const char **argv) {
    Expected<CommonOptionsParser> options =
        CommonOptionsParser::create(argc, argv, OptC);

    std::optional<Framework> selectedFramework =
        checkFramework(FrameworkOption);
    exitIfNotValidFramework(selectedFramework);
    selectFrameworkFromOption(selectedFramework.value());

    setAskeletonHomeFromEnv();

    loadConfigurationFromConfigFile();
    Config &config = Config::getInstance();
    ConfigGenerator::loadConfigurations();

    exitIfFolderDoesNotExist(getAskeletonHome() /
                             config.get("route.templates"));
    Generator::setTemplateItems();
    Generator::MAX_DEPTH = DeepLevel.getValue();

    moveGeneratedFolderToLog(config);

    fs::create_directories(getAskeletonHome() / config.get("route.ut"));

    clang::ast_matchers::MatchFinder Finder;
    ASKGen Functionality;
    for (auto i : createMapMatchers())
        Finder.addMatcher(i.second, &Functionality);

    ClangTool Tool(options->getCompilations(), options->getSourcePathList());
    return Tool.run(newFrontendActionFactory(&Finder).get());
}
