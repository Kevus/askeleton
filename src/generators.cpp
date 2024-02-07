#include "generators.hpp"

string askeleton::bt::generateVariableAssign(const string &funcCfgName,
                                             const InfoVariable &variable) {
    return ("\n\t" + variable.original + " " + funcCfgName + "_" +
            variable.name + " = Read_" + variable.formatted + "(\"" +
            funcCfgName + "." + variable.name + "\");\n");
}

string askeleton::bt::generateVariableAssign(const string &funcCfgName,
                                             const InfoType &type) {
    string name = type.formatted;
    replaceAll(name, "_", " ");
    removeAll(name, "&");

    return generateVariableAssign(funcCfgName,
                                  {name, type.original, type.formatted});
}

string askeleton::bt::generatePointerAssign(const string &funcCfgName,
                                            const InfoVariable &variable) {
    return ("\n\t" + variable.original + funcCfgName + "_" + variable.name +
            " = Read_" + variable.formatted + "(\"" + funcCfgName + "." +
            variable.name + "_input\");\n");
}

string askeleton::routes::generateSupportPath(const string &className) {
    return routes::TEST_ROUTE + className + "/" + files::SUPPORTED_TYPES;
}