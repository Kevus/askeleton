#ifndef AUXILIARY_FUNCTIONS_HPP
#define AUXILIARY_FUNCTIONS_HPP

#include <iomanip>
#include <ctime>

#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iostream>
#include <fstream>

#include <map>

//Boost libraries
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>

#include <sys/stat.h>

using namespace std;

bool fileExists(const string& filename);
string getCommentHeader(string filename);
string deleteAllBeforeChar(string sToReplace, char cToFind);
string cleanUnnecesaryChars(string sToReplace);
bool isNumeric(string query);

#endif