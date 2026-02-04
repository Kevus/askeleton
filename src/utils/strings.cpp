#include "utils/strings.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

string deleteAllBeforeChar(string sToReplace, char cToFind) {
    if (sToReplace.find(cToFind) != string::npos)
        sToReplace = sToReplace.substr(sToReplace.find_last_of(cToFind) + 1,
                                       sToReplace.size());

    return sToReplace;
}

void replaceAll(string &str, const string &from, const string &to) {

    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like
                                  // replacing 'x' with 'yx'
    }
}

std::string join(const std::vector<std::string> &vec, char delimiter) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) {
            oss << delimiter;
        }
        oss << vec[i];
    }
    return oss.str();
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

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);

    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
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

string toLower(const std::string &str) {
    string result = str;
    transform(str.begin(), str.end(), result.begin(), ::tolower);
    return result;
}

string toUpper(const std::string &str) {
    string result = str;
    transform(str.begin(), str.end(), result.begin(), ::toupper);
    return result;
}
