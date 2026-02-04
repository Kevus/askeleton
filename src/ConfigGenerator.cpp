#include "ConfigGenerator.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "constants.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "utils/templating.hpp"

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

RandomValuesGenerator ConfigGenerator::rvg;
const json &ConfigGenerator::config = getConfig();
const json &ConfigGenerator::tplItems = getTemplateItems();

ConfigGenerator::ConfigGenerator(const string &target)
    : target(target), testFolder(getAskeletonHome() / config["route"]["ut"] / target),
      configFilePath(fs::path(testFolder) / (target + ".cfg")) {
    fs::create_directories(testFolder);
    fs::path configFileTemplate = getAskeletonHome() / config["route"]["templates"] /
                                  config["file"]["template"]["cfg_tpl"];

    map<string, string> replacements = {
        {tplItems["tplitem"]["file_path"], target},
        {tplItems["tplitem"]["target"], target},
        {tplItems["tplitem"]["date_of_generation"], getTodayString()}};

    replaceTokensInFile(configFileTemplate, configFilePath, replacements);
}

void ConfigGenerator::generateTestCase(const string &functionName,
                                       const vector<InfoVariable> &params,
                                       const InfoType &returnType,
                                       unsigned invocationNumber) const {
    stringstream ss;
    const string returnVarName =
        returnType.getUnderlyingType().getFormattedNotParametrized();
    InfoVariable returnVar{returnVarName, returnType};
    const static string returnPrefix = tplItems["tplitem"]["return_prefix"];

    ss << functionName << "_" << invocationNumber << ":\n{\n";
    ss << generateParam(params);
    ss << generateParam(returnVar, false, returnPrefix);
    ss << "\n};\n\n";

    appendToConfigFile(ss.str());
}

void ConfigGenerator::generateConstructorTest(const string &ctorName,
                                              const vector<InfoVariable> &params,
                                              unsigned invocationNumber) const {
    stringstream ss;

    ss << ctorName << "_" << invocationNumber << ":\n{\n";
    ss << generateParam(params);
    ss << "\n};\n\n";

    appendToConfigFile(ss.str());
}

std::string ConfigGenerator::generateParam(const vector<InfoVariable> &params,
                                           bool generatePointers,
                                           const string &prefix) const {
    stringstream ss;

    for (const InfoVariable &param : params)
        ss << generateParam(param, generatePointers, prefix) << "\n";

    return ss.str();
}

std::string ConfigGenerator::generateParam(const InfoVariable &param,
                                           bool generatePointers,
                                           const string &prefix) const {
    pair<InfoVariable, InfoVariable> pointers = param.getPointers();
    InfoType underlying = param.getUnderlyingType();
    string value = rvg.getRandomValue(underlying.formatted);
    stringstream ss;

    if (underlying.isRecord() && !underlying.isContainer()) {
        ss << generateParam(pointers.first.getRecordFields(), generatePointers,
                            prefix + pointers.first.name + ".");
        if (generatePointers && (param.isPointer() || param.isReference()))
            ss << generateParam(pointers.second.getRecordFields(), generatePointers,
                                prefix + pointers.second.name + ".");
    } else {
        ss << "\t" << prefix + pointers.first.name << "=" << value << ";#"
           << param.original;
        if (generatePointers && (param.isPointer() || param.isReference()))
            ss << "\n\t" << prefix + pointers.second.name << "=" << value << ";#"
               << param.original;
    }

    return ss.str();
}

void ConfigGenerator::appendToConfigFile(const string &content) const {
    std::ofstream configfileStream(configFilePath, ios_base::app);
    if (configfileStream.is_open()) {
        configfileStream << content;
    } else {
        cerr << askeleton::errors::openFileError(configFilePath) << endl;
    }
}
