#include "ConfigGenerator.hpp"
#include "constants.hpp"

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

void ConfigGenerator::generateParams(const vector<InfoVariable> &params) {
    for (const InfoVariable &param : params)
        generateParam(param);
}

void ConfigGenerator::generateReturn(const InfoType &returnType) {
    if (returnType.isRecord() && !returnType.isContainer())
        generateReturnRecord(returnType);
    else {
        std::string content =
            "\treturn_" +
            (returnType.isContainer()
                 ? extractSubstringUntilCharacter(returnType.formatted, '<')
                 : returnType.formatted) +
            "=" + rvg.getRandomValue(returnType.formatted) + ";#" +
            returnType.original + "\n";
        appendToConfigFile(content);
    }
}

void ConfigGenerator::generateParam(const InfoVariable &param) {
    const string &original = param.original, &name = param.name;
    string value = rvg.getRandomValue(param.formatted);

    cfg_file << "\t";
    if (param.isRecord() && !param.isContainer()) {
        for (const InfoVariable &field : param.getRecordFields()) {
            cfg_file << name << "." << field.name << "="
                     << rvg.getRandomValue(field.formatted) << ";#"
                     << field.original << "\n";
        }
    } else if (param.isPointer() || param.isReference()) {
        cfg_file << name << "_input=" << value << ";#" << original << "\n\t"
                 << name << "_output=" << value << ";#" << original << "\n";
    } else {
        cfg_file << name << "=" << value << ";#" << original << "\n";
    }
}

void ConfigGenerator::generateParamRecord(const InfoVariable &record,
                                          const string &prefix) {
    const string &name = prefix + record.name;

    for (const InfoVariable &field : record.getRecordFields()) {
        if (field.isRecord())
            generateParamRecord(field, name + ".");
        else
            cfg_file << "\t" << name << "." << field.name << "="
                     << rvg.getRandomValue(field.formatted) << ";#"
                     << field.original << "\n";
    }
}

void ConfigGenerator::generateReturnRecord(const InfoType &record,
                                           const string &prefix) {
    const string &name = prefix + record.formatted;

    for (const InfoVariable &field : record.getRecordFields()) {
        if (field.isRecord())
            generateReturnRecord(field, name + ".");
        else
            cfg_file << "\t" << name << "." << field.name << "="
                     << rvg.getRandomValue(field.formatted) << ";#"
                     << field.original << "\n";
    }
}

void ConfigGenerator::generateTestCase(const string &functionName,
                                       const vector<InfoVariable> &params,
                                       const InfoType &returnType) {
    if (!cfg_file.is_open())
        return;

    cfg_file << functionName << ":\n{\n";
    generateParams(params);
    generateReturn(returnType);
    cfg_file << "};\n\n";
}

void ConfigGenerator::generateConstructorTest(
    const string &ctorName, const vector<InfoVariable> &params) {
    if (!cfg_file.is_open())
        return;

    cfg_file << ctorName << ":\n{\n";
    generateParams(params);
    cfg_file << "};\n\n";
}

void ConfigGenerator::appendToConfigFile(const string &content) const {
    std::ofstream configfileStream(configFilePath, ios_base::app);
    if (configfileStream.is_open()) {
        configfileStream << content;
    } else {
        exitWithError("Error opening file: " + configFilePath);
    }
}

// ##############################################################
