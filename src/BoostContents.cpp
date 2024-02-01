#include "BoostContents.hpp"

string askeleton::btcontents::generateVariableAssign(
		const string &funcCfgName, const InfoVariable &variable) {
	return ( 
		"\n\t" + variable.original + " " + funcCfgName + "_" + variable.name 
		+ " = Read_" + variable.formatted + "(\"" + funcCfgName + "." 
		+ variable.name + "\");\n"
	);
}

string askeleton::btcontents::generateVariableAssign(
		const string &funcCfgName, const InfoType &type) {
	string name = type.formatted;
	replaceAll(name, "_", " ");
	removeAll(name, "&");
	
	return generateVariableAssign(funcCfgName, {name, type.original, type.formatted});
}