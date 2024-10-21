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

static string GENERATION_FOLDER = "Generated";

int main(int argc, const char **argv) {
    // Setting the ASKELETON_HOME static member
    if (getenv("ASKELETON_HOME") != NULL) {
        Generator::ASKELETON_HOME = getenv("ASKELETON_HOME");
    } else {
        Generator::ASKELETON_HOME = ".";
        cerr
            << "WARNING: ASKELETON_HOME is not set. Templates will not be "
               "accesible unless runing ASKELETON in the compilation folder.\n";
    }
    Generator::ASKELETON_HOME += "/";
    Generator::MAX_DEPTH = DeepLevel.getValue();
    Config::getInstance().loadConfig(createPath(
        {Generator::ASKELETON_HOME, "data/configuration.json"}, true));

    if (CatchFramework)
        Generator::FRAMEWORK = CATCH;
    else if (GtestFramework)
        Generator::FRAMEWORK = GTEST;
    else
        Generator::FRAMEWORK = BOOST;

    // CommonOptionsParser OptionsParser(argc, argv, OptC);
    //  Esto se ha quedado 'deprecated', usando esta solucion temporal
    Expected<CommonOptionsParser> options =
        CommonOptionsParser::create(argc, argv, OptC);
    ClangTool Tool(options->getCompilations(), options->getSourcePathList());

    clang::ast_matchers::MatchFinder Finder;
    ASKGen Functionality;

    for (auto i : createMapMatchers())
        Finder.addMatcher(i.second, &Functionality);

    // We'll get the time for the LOG folder
    if (folderExists(GENERATION_FOLDER)) {
        auto t = time(nullptr);
        auto tm = *localtime(&t);

        ostringstream oss;
        oss << put_time(&tm, "%d%m%Y%H%M%S");
        string today = oss.str();

        // DEBUG: Generation of log folders is temporarily disabled
        // string system_op = "mkdir Generated_LOG" + today + "/ && cp -r " +
        // GENERATION_FOLDER + "/UT/* Generated_LOG" + today + "/ && " +
        //"rm -R " + GENERATION_FOLDER + "/UT/*";
        string system_op = "rm -R " + GENERATION_FOLDER + "/UT/*";

        // Reset the results Folder
        system(system_op.c_str());
    }

    return Tool.run(newFrontendActionFactory(&Finder).get());
}
