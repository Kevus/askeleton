#include "ConfigGenerator.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "auxiliary_functions.hpp"
#include "constants.hpp"

using namespace std;

RandomValuesGenerator ConfigGenerator::rvg;

ConfigGenerator::ConfigGenerator(const string &target)
    : target(target), testFolder(askeleton::routes::TEST_ROUTE + target + "/"),
      configFilePath(testFolder + target + ".cfg") {

    try {
        std::filesystem::create_directory(testFolder);
    } catch (const std::filesystem::filesystem_error &e) {
        exitWithError("Error creating directory: " + string(e.what()));
    }

    std::ofstream configfileStream(configFilePath, ios_base::app);
    if (configfileStream.is_open()) {
        configfileStream << getCommentHeader(target);
    } else {
        exitWithError("Error opening file: " + configFilePath);
    }
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
    stringstream ss;

    if (returnType.isRecord() && !returnType.isContainer())
        ss << generateReturnRecord(returnType);
    else {
        ss << "\treturn_"
           << (returnType.isContainer()
                   ? extractSubstringUntilCharacter(returnType.formatted, '<')
                   : returnType.formatted)
           << "=" << rvg.getRandomValue(returnType.formatted) << ";#"
           << returnType.original << "\n";
    }

    return ss.str();
}

std::string ConfigGenerator::generateParam(const InfoVariable &param) const {
    const string &original = param.original, &name = param.name;
    string value = rvg.getRandomValue(param.formatted);
    stringstream ss;

    if (param.isRecord() && !param.isContainer()) {
        for (const InfoVariable &field : param.getRecordFields()) {
            ss << "\t" << name << "." << field.name << "="
               << rvg.getRandomValue(field.formatted) << ";#" << field.original
               << "\n";
        }
    } else if (param.isPointer() || param.isReference()) {
        ss << "\t" << name << "_input=" << value << ";#" << original << "\n\t"
           << name << "_output=" << value << ";#" << original << "\n";
    } else {
        ss << "\t" << name << "=" << value << ";#" << original << "\n";
    }

    return ss.str();
}

std::string ConfigGenerator::generateParamRecord(const InfoVariable &record,
                                                 const string &prefix) const {
    const string &name = prefix + record.name;
    stringstream ss;

    for (const InfoVariable &field : record.getRecordFields()) {
        if (field.isRecord())
            ss << generateParamRecord(field, name + ".");
        else
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
