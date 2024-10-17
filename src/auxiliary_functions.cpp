#include "auxiliary_functions.hpp"

#include <algorithm>
#include <ctime>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <streambuf>
#include <string>

#include "clang/AST/Expr.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"

#include <sys/stat.h>

using namespace clang;
using namespace std;

// Method to check if a file exists given a path
bool fileExists(const string &filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

// Method to check if a file exists given a folder
bool folderExists(const string &folder) {
    struct stat info;

    if (stat(folder.c_str(), &info) != 0)
        return false;
    else
        return (info.st_mode &
                S_IFDIR); // S_ISDIR() doesn't exist on my windows
    /*      printf( "%s is a directory\n", pathname );
      else
          printf( "%s is no directory\n", pathname );*/
}

// Aux. method to generate the header for the files
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

// Given a string and a charater, generates a new string without
// all the elements before the character.
string deleteAllBeforeChar(string sToReplace, char cToFind) {
    if (sToReplace.find(cToFind) != string::npos)
        sToReplace = sToReplace.substr(sToReplace.find_last_of(cToFind) + 1,
                                       sToReplace.size());

    return sToReplace;
}

// This method deletes some tags in order to get the types.
// This was required by the company
string cleanUnnecesaryChars(string sToReplace) {
    //==========================================================
    // We are formating types:
    //	std:: flag
    //  __cxx11:: flag
    //	All pointers will be treated as normal types
    // 	All spaces will be replaced with _
    //==========================================================
    replaceAll(sToReplace, "std::", "");
    replaceAll(sToReplace, "__cxx11::", "");
    // replaceAll(sToReplace, " &", "");

    if (sToReplace.find("<") == string::npos)
        replaceAll(sToReplace, " ", "_");

    return sToReplace;
}

bool isNumeric(string query) {
    try {
        stod(query);
        return true;
    } catch (std::invalid_argument &) {
        return (query.find('\'') != string::npos ||
                query.find('\"') != string::npos || query == "true" ||
                query == "false");
    }
}

// Given an expression and a sourcemanager, return the expression as a string
string convertExpressionToString(Expr *E, SourceManager &SM) {

    LangOptions langOpts;

    SourceLocation startLoc = E->getBeginLoc();
    SourceLocation _endLoc = E->getEndLoc();
    SourceLocation endLoc =
        Lexer::getLocForEndOfToken(_endLoc, 0, SM, langOpts);

    try {
        string result =
            string(SM.getCharacterData(startLoc),
                   SM.getCharacterData(endLoc) - SM.getCharacterData(startLoc));

        return result;
    } catch (std::bad_alloc &ba) {
        return "";
    }
}

// Check if a parameter is in a list of parameters. If so, saves the type in a
// pointer
bool isInParameters(string name, ArrayRef<ParmVarDecl *> params, string &type) {
    for (auto it : params) {
        if (it->getName().str() == name) {
            type = it->getOriginalType().getAsString();
            return true;
        }
    }

    return false;
}

// Receives three strings and returns the first string with all the occurences
// of the second string replaced by the third string
void replaceAll(string &str, const string &from, const string &to) {

    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like
                                  // replacing 'x' with 'yx'
    }
}

std::string extractSubstringUntilCharacter(const string &str, char c) {
    size_t pos = str.find(c);
    return pos != string::npos ? str.substr(0, pos) : str;
}

void removeAll(string &str, const string &substringToRemove) {
    replaceAll(str, substringToRemove, "");
}

void removeAll(string &str, const initializer_list<string> &substrings) {
    for (const auto &sub : substrings)
        removeAll(str, sub);
}

bool containsSubstring(const string &original, const string &substring) {
    return original.find(substring) != string::npos;
}

bool containsAnySubstring(const string &original,
                          const initializer_list<string> &substrings) {
    for (const auto &s : substrings)
        if (containsSubstring(original, s))
            return true;
    return false;
}

string extractFileName(const string &fileRoute) {
    unsigned first = fileRoute.find_last_of("/\\") + 1;
    unsigned last = fileRoute.find_last_of(".");

    return fileRoute.substr(first, last - first);
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

void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
}

void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            s.end());
}

void exitWithError(const string &message) {
    cerr << message << "\n";
    cerr << "Error code: " << errno << " - " << strerror(errno) << "\n";
    cerr << "\nFatal error. Exiting...\n";
    exit(EXIT_FAILURE);
}

string getTodayString() {
    ostringstream oss;
    auto t = time(nullptr);
    auto tm = *localtime(&t);

    oss << put_time(&tm, "%d-%m-%Y %H:%M:%S");

    return oss.str();
}

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);

    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string createPath(const std::vector<std::string> &parts, bool isFile) {
    std::string path;

    if (!parts.empty() && parts[0][0] == '/')
        path = "/";

    for (const std::string &part : parts) {
        path += part + "/";
    }

    replaceAll(path, "//", "/");
    if (isFile)
        path.pop_back();
    return path;
}

bool endsWith(std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(),
                                        ending.length(), ending));
    } else {
        return false;
    }
}

bool includes(const string &str1, const string &str2) {
    return str1.find(str2) != string::npos;
}

bool includes(const char *str1, const char *str2) {
    return strstr(str1, str2) != NULL;
}

bool isStruct(const QualType &type) {
    const clang::Type *unqualifiedType =
        type.getUnqualifiedType().getTypePtrOrNull();

    if (unqualifiedType)
        return clang::isa<clang::RecordType>(unqualifiedType);

    return false;
}

string getStructName(const QualType &type) {
    const Type *unqualifiedType = type.getUnqualifiedType().getTypePtrOrNull();

    if (unqualifiedType) {
        // Checking if typedef struct
        if (const TypedefType *typedefType =
                dyn_cast<TypedefType>(unqualifiedType))
            return typedefType->getDecl()->getNameAsString();

        // Checking if typical struct
        if (const RecordType *recordType =
                dyn_cast<RecordType>(unqualifiedType)) {
            if (const RecordDecl *recordDeclaration = recordType->getDecl()) {
                return recordDeclaration->hasNameForLinkage()
                           ? recordDeclaration->getNameAsString()
                           : "anonymous";
            }
        }
    }
    return {};
}

bool isEnum(const QualType &type) {
    const clang::Type *unqualifiedType =
        type.getUnqualifiedType().getTypePtrOrNull();

    return unqualifiedType ? unqualifiedType->isEnumeralType() : false;
}