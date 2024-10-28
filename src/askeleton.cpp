#include "ASKGen.hpp"
#include "ASKMatchers.hpp"
#include "framework/Generator.hpp"
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

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp
    MoreHelp("\nIf you are working with C++ headers use the option -xc++ at "
             "the end.\nAuthor: Kevin J. Valle-Gomez (kevin.valle@uca.es)\n");
static llvm::cl::OptionCategory
    OptC("ASkeleTon - Unit Test Generator for C/C++");

cl::opt<bool> BoostFramework("boost", cl::desc("Enable boost"), cl::init(true));
cl::opt<bool> CatchFramework("catch2", cl::desc("Enable catch"),
                             cl::init(false));
cl::opt<bool> GtestFramework("gtest", cl::desc("Enable google test"),
                             cl::init(false));
cl::opt<int> DeepLevel("deep_level",
                       cl::desc("Specify the maximum depth level"),
                       cl::value_desc("level"), cl::init(1), cl::cat(OptC));

int main(int argc, const char **argv) {
    if (CatchFramework)
        setFramework(CATCH);
    else if (GtestFramework)
        setFramework(GTEST);
    else
        setFramework(BOOST);

    if (getenv("ASKELETON_HOME") != NULL) {
        setAskeletonHome(getenv("ASKELETON_HOME"));
    } else {
        setAskeletonHome(fs::current_path());
        cerr
            << "WARNING: ASKELETON_HOME is not set. Templates will not be "
               "accesible unless runing ASKELETON in the compilation folder.\n";
    }

    fs::path configFile = getAskeletonHome() / "data/configuration.json";
    if (!fs::exists(configFile))
        exitWithError("ERROR: Configuration file not found. Check " +
                      configFile.string());

    Config &config = Config::getInstance();
    config.loadConfig(configFile);

    fs::path templateFolder =
        getAskeletonHome() / config.get("route.templates");
    if (!fs::exists(templateFolder))
        exitWithError("ERROR: Template folder not found. Check " +
                      templateFolder.string());

    Generator::MAX_DEPTH = DeepLevel.getValue();
    Generator::setTemplateItems();

    fs::path utFolder = config.get("route.generated");
    if (fs::exists(utFolder)) {
        fs::path logFolder = fs::path(config.get("route.log"));
        if (!fs::exists(logFolder))
            create_directory(logFolder);

        logFolder /= (config.get("route.generated") + "_" +
                      getTodayString("%d%m%Y_%H%M%S"));
        rename(utFolder, logFolder);
    }

    fs::create_directories(getAskeletonHome() / config.get("route.ut"));

    // CommonOptionsParser OptionsParser(argc, argv, OptC);
    //  Esto se ha quedado 'deprecated', usando esta solucion temporal
    Expected<CommonOptionsParser> options =
        CommonOptionsParser::create(argc, argv, OptC);
    ClangTool Tool(options->getCompilations(), options->getSourcePathList());

    clang::ast_matchers::MatchFinder Finder;
    ASKGen Functionality;
    for (auto i : createMapMatchers())
        Finder.addMatcher(i.second, &Functionality);

    return Tool.run(newFrontendActionFactory(&Finder).get());
}
