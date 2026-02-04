#include "generators.hpp"

#include <string>

#include "utils/strings.hpp"

std::string askeleton::bt::generateVariableAssign(const std::string &funcCfgName,
                                                  const InfoVariable &variable) {
    return ("\n\t" + variable.original + " " + funcCfgName + "_" +
            variable.name + " = Read_" + variable.formatted + "(\"" +
            funcCfgName + "." + variable.name + "\");\n");
}

std::string askeleton::bt::generateVariableAssign(const std::string &funcCfgName,
                                                  const InfoType &type) {
    std::string name = type.formatted;
    replaceAll(name, "_", " ");
    removeAll(name, "&");

    return generateVariableAssign(funcCfgName,
                                  {name, type.original, type.formatted});
}

std::string askeleton::bt::generatePointerAssign(const std::string &funcCfgName,
                                                 const InfoVariable &variable) {
    return ("\t" + variable.original + funcCfgName + "_" + variable.name +
            " = Read_" + variable.formatted + "(\"" + funcCfgName + "." +
            variable.name + "_input\");");
}

std::string askeleton::bt::generateReferenceAssign(const std::string &functionName,
                                                   const InfoVariable &reference) {
    InfoType underlying = reference.getUnderlyingType();
    return ("\t" + underlying.original + " " + functionName + "_" +
            reference.name + " = " + "Read_" + underlying.formatted + "(\"" +
            functionName + "." + reference.name + "_input\");");
}

std::string askeleton::routes::generateSupportPath(const std::string &className) {
    return className;
}
