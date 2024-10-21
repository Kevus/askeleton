#include "utils/templating.hpp"
#include "utils/strings.hpp"

#include <algorithm>
#include <ctime>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "clang/AST/Expr.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"

using namespace clang;
using namespace std;

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
