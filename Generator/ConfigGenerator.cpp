#include "ConfigGenerator.hpp"

/**
** Default values for generating config file.
** It can be seen that every value is a string,
** however, C++11 provide us with methods for
** transforming them.
**
** int: std::stoi( str )
** float: std::stof( str )
**
** more info: http://en.cppreference.com/w/cpp/string/basic_string/stol
**
**/

ConfigGenerator::ConfigGenerator(string f_Name) : f_Name(f_Name) {
    string comment_header = "";

    // We will create the file if it doesn't exist
    string sys_command = "mkdir -p Generated/UT/" + f_Name;
    system(sys_command.c_str());

    sys_command = "Generated/UT/" + f_Name + "/" + f_Name + ".cfg";
    bool file_exists = fileExists(sys_command);

    if (!file_exists)
        comment_header = getCommentHeader(f_Name);

    // cfg_file.open(sys_command, (fileExists(sys_command) ? ios_base::app :
    // ios_base::out));
    cfg_file.open(sys_command, ios_base::app);

    if (cfg_file.is_open())
        cfg_file << comment_header;
}

void ConfigGenerator::generateParams(const vector<InfoVariable> &params) {
    for (const InfoVariable &param : params)
        generateParam(param);
}

void ConfigGenerator::generateReturn(const InfoType &returnType) {
    if (returnType.isRecord() && !returnType.isContainer())
        generateReturnRecord(returnType);
    else
        cfg_file << "\treturn_"
                 << (returnType.isContainer() ? extractSubstringUntilCharacter(
                                                    returnType.formatted, '<')
                                              : returnType.formatted)
                 << "=" << rvg.getRandomValue(returnType.formatted) << ";#"
                 << returnType.original << "\n";
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

// ##############################################################
