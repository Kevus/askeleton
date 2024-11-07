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

const Config &ConfigGenerator::config = Config::getInstance();
RandomValuesGenerator ConfigGenerator::rvg;
std::map<std::string, std::string> ConfigGenerator::tplItems;

void ConfigGenerator::loadConfigurations() {
    json tplItemsJson = loadTemplateItems();
    tplItems = {
        {"date_of_generation", tplItemsJson["tplitem"]["date_of_generation"]},
        {"file_path", tplItemsJson["tplitem"]["file_path"]},
        {"target", tplItemsJson["tplitem"]["target"]}};
}

ConfigGenerator::ConfigGenerator(const string &target)
    : target(target), testFolder(fs::path(config.get("route.ut")) / target),
      configFilePath(fs::path(testFolder) / (target + ".cfg")) {
    fs::path configFileTemplate = getAskeletonHome() /
                                  config.get("route.templates") /
                                  config.get("file.template.cfg_tpl");

    map<string, string> replacements = {
        {tplItems["file_path"], target},
        {tplItems["target"], target},
        {tplItems["date_of_generation"], getTodayString()}};

    replaceTokensInFile(configFileTemplate, configFilePath, replacements);
}

void ConfigGenerator::generateTestCase(const string &functionName,
                                       const vector<InfoVariable> &params,
                                       const InfoType &returnType) const {
    stringstream ss;

    ss << functionName << ":\n{\n";
    ss << generateParams(params);
    ss << generateReturn(returnType);
    ss << "};\n\n";

    appendToConfigFile(ss.str());
}

void ConfigGenerator::generateConstructorTest(
    const string &ctorName, const vector<InfoVariable> &params) const {
    stringstream ss;

    ss << ctorName << ":\n{\n";
    ss << generateParams(params);
    ss << "};\n\n";

    appendToConfigFile(ss.str());
}

std::string
ConfigGenerator::generateParams(const vector<InfoVariable> &params) const {
    stringstream ss;

    for (const InfoVariable &param : params)
        ss << generateParam(param);

    return ss.str();
}

std::string ConfigGenerator::generateReturn(const InfoType &returnType) const {
    InfoType underlying = returnType.getUnderlyingType();
    stringstream ss;

    if (underlying.isRecord() && !underlying.isContainer())
        ss << generateReturnRecord(underlying);
    else {
        ss << "\treturn_"
           << (underlying.isContainer()
                   ? extractSubstringUntilCharacter(underlying.formatted, '<')
                   : underlying.formatted)
           << "=" << rvg.getRandomValue(underlying.formatted) << ";#"
           << underlying.original << "\n";
    }

    return ss.str();
}

std::string ConfigGenerator::generateParam(const InfoVariable &param) const {
    string original = param.original,
           name = param.isPointer() ? param.name + "_input" : param.name;
    InfoType underlying = param.getUnderlyingType();
    string value = rvg.getRandomValue(param.formatted);
    stringstream ss;

    if (underlying.isRecord() && !underlying.isContainer()) {
        ss << generateParamRecord(underlying, name);
        if (param.isPointer() || param.isReference()) {
            ss << generateParamRecord(underlying, param.name + "_output");
        }
    } else {
        ss << "\t" << name << "=" << value << ";#" << original << "\n";
    }

    return ss.str();
}

string ConfigGenerator::generateParamRecord(InfoType &underlying,
                                            const std::string &name) const {
    std::stringstream ss;
    for (const InfoVariable &field : underlying.getRecordFields()) {
        ss << "\t" << name << "." << field.name << "="
           << rvg.getRandomValue(field.formatted) << ";#" << field.original
           << "\n";
    }
    return ss.str();
}

std::string ConfigGenerator::generateReturnRecord(const InfoType &record,
                                                  const string &prefix) const {
    const string &name = prefix + record.formatted;
    stringstream ss;

    for (const InfoVariable &field : record.getRecordFields()) {
        if (field.isRecord())
            ss << generateReturnRecord(field, name + ".");
        else
            ss << "\t" << name << "." << field.name << "="
               << rvg.getRandomValue(field.formatted) << ";#" << field.original
               << "\n";
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
