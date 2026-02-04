#include "utils/checks.hpp"

#include <stdexcept>
#include <string>

#include "clang/AST/Decl.h"

using namespace std;
using namespace clang;

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

bool isInParameters(string name, ArrayRef<ParmVarDecl *> params, string &type) {
    for (auto it : params) {
        if (it->getName().str() == name) {
            type = it->getOriginalType().getAsString();
            return true;
        }
    }

    return false;
}
