#include "utils/templating.hpp"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "clang/AST/Expr.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"

using namespace clang;
using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

string getCommentHeader(string filename) {
    /**
    ** Comment header, for visibility purposes
    **/
    stringstream buffer;

    // Time utilities
    auto t = time(nullptr);
    auto tm = *localtime(&t);

    // ASK Banner
    buffer << "#" << string(55, '/') << "\n"
           << "#////" << string(47, ' ') << "////\n"
           << "#//// ASKELETON TEST GENERATOR" << string(30, ' ') << "////\n"
           << "#//// UNIVERSIDAD DE CADIZ - UCASE RESEARCH GROUP ////\n"
           << "#////" << string(47, ' ') << "////\n"
           << "#" << string(55, '/') << "\n"
           << "#File generated automatically by ASKELETON.\n"
           << "#File " << filename << "\n"
           << "#Date " << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << "\n"
           << "#" << string(55, '/') << "\n\n";

    // The banner should look like this:

    /**
    ** #///////////////////////////////////////////////////////
    ** #////                                               ////
    ** #//// ASKELETON TEST GENERATOR                              ////
    ** #//// UNIVERSIDAD DE CADIZ - UCASE RESEARCH GROUP   ////
    ** #////                                               ////
    ** #///////////////////////////////////////////////////////
    ** #File generated automatically by ASKELETON.
    ** #File <filename>.<extension>
    ** #Date dd-mm-yyyy hh:MM:ss
    ** #///////////////////////////////////////////////////////
    **
    **/
    // *** END OF THE BANNER ***

    return buffer.str();
}

void removeTypeQualifiers(string &type) {
    removeAll(type, "const");
    removeAll(type, "enum");
    removeAll(type, "class");
    removeAll(type, "struct");

    ltrim(type);
}

void replaceTypeCharacters(string &type) {
    replaceAll(type, "*", "s");
    replaceAll(type, "&", "r");
}

string removeNamespaceQualifier(string stringToReplace) {
    removeAll(stringToReplace, {"std::", "__cxx11::"});
    // replaceAll(stringToReplace, " ", "_");

    return stringToReplace;
}

string removeTemplateArguments(string type) {
    size_t pos = type.find("<");
    if (pos != string::npos) {
        return type.substr(0, pos);
    }
    return type;
}

string convertExpressionToString(Expr *E, SourceManager &SM) {

    LangOptions langOpts;

    SourceLocation startLoc = E->getBeginLoc();
    SourceLocation _endLoc = E->getEndLoc();
    SourceLocation endLoc = Lexer::getLocForEndOfToken(_endLoc, 0, SM, langOpts);

    try {
        string result =
            string(SM.getCharacterData(startLoc),
                   SM.getCharacterData(endLoc) - SM.getCharacterData(startLoc));

        return result;
    } catch (std::bad_alloc &ba) {
        return "";
    }
}

void replaceTokensInText(string &text,
                         const std::map<std::string, std::string> &replacements) {

    for (const auto &[key, value] : replacements)
        replaceAll(text, key, value);
}

void replaceTokensInFile(const std::string &templateFilePath,
                         const std::string &outputFilePath,
                         const std::map<std::string, std::string> &replacements) {

    string fileContent = readFromFile(templateFilePath);
    replaceTokensInText(fileContent, replacements);
    writeToFile(outputFilePath, fileContent);
}

string replaceTokensInFile(const string &inputFilePath,
                           const map<string, string> &replacements) {

    string fileContent = readFromFile(inputFilePath);
    replaceTokensInText(fileContent, replacements);
    return fileContent;
}

void createFileFromTemplate(const fs::path &templateFilePath,
                            const fs::path &outputFilePath,
                            const map<string, string> &replacements) {
    string fileContent = replaceTokensInFile(templateFilePath, replacements);
    writeToFile(outputFilePath, fileContent);
}

string generateTestObjectForTarget(const string &target) {
    string objectTest = target;
    objectTest[0] = tolower(objectTest[0]);
    return objectTest + "_test";
}

map<string, string> readEquivalentTypes() {
    const fs::path filename =
        getAskeletonHome() / getConfig()["file"]["data"]["equivalent_types"];
    map<string, string> equivalentTypes;
    ifstream jsonFile(filename);

    if (!jsonFile.is_open())
        showOpenFileError(filename);
    else {
        json j;
        jsonFile >> j;
        jsonFile.close();

        for (auto &[key, value] : j.items()) {
            equivalentTypes[key] = value;
        }
    }
    return equivalentTypes;
}

optional<string> getEquivalentType(const string &type) {
    static map<string, string> equivalentTypes = readEquivalentTypes();
    auto it = equivalentTypes.find(type);
    return it == equivalentTypes.end() ? nullopt : optional<string>(it->second);
}