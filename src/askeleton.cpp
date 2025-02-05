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
#include <nlohmann/json.hpp>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

json &config = getConfig();

void exitIfFilesDoNotExit(const json &node, const fs::path &basePath) {
	for (const auto &item : node.items()) {
		if(item.value().is_string()) {
			fs::path filePath = basePath / item.value().get<string>();
			if (!fileExists(filePath))
				exitWithError("ERROR: File not found. Check " + filePath.string());
			else
				cout << "File checked: " << filePath << endl;
		}
		else if(item.value().is_object()) {
			exitIfFilesDoNotExit(item.value(), basePath);
		}
	}
}

void exitIfFilesDoNotExist() {
	fs::path basePath = getAskeletonHome();
	switch(getFramework()) {
		case Framework::GTEST:
			exitIfFilesDoNotExit(config["file"]["template"], basePath / config["route"]["gtest_templates"]);
			break;
		case Framework::BOOST:
			exitIfFilesDoNotExit(config["file"]["template"], basePath / config["route"]["boost_templates"]);
			break;
		case Framework::CATCH:
			exitIfFilesDoNotExit(config["file"]["template"], basePath / config["route"]["catch_templates"]);
			break;
	}
	exitIfFilesDoNotExit(config["file"]["data"], basePath);
}

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
            cout << "Log folder created at " << logFolder << endl;
        }

        logFolder /= (config["route"]["generated"].get<string>() + "_" +
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

    exitIfFolderDoesNotExist(getAskeletonHome() / config["route"]["templates"]);
	// llvm::outs() << "Checking ASkeleTon files...\n";
	// exitIfFilesDoNotExist();
	// llvm::outs() << "Files checked successfully\n";

    Generator::MAX_DEPTH = DeepLevel.getValue();

    moveGeneratedFolderToLog();

    fs::create_directories(getAskeletonHome() / config["route"]["ut"]);

	llvm::outs() << "-------------------------------------\n" 
				 << "   Starting ASkeleTon UT Generator   \n"
				 << "-------------------------------------\n";

    clang::ast_matchers::MatchFinder Finder;
    ASKGen Functionality;
    for (auto i : createMapMatchers())
        Finder.addMatcher(i.second, &Functionality);

    ClangTool Tool(options->getCompilations(), options->getSourcePathList());
    return Tool.run(newFrontendActionFactory(&Finder).get());
}
