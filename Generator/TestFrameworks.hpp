#include "ConfigGenerator.hpp"
#include <regex>

class BoostGenerator {
public:
    BoostGenerator(string filePath, string cfgName, bool isFromClass);

    // TODO: eliminar esta version
    void generateBoostAssert(string class_test, string function_name,
                             string function_cfg_name,
                             map<string, string> param_type,
                             vector<string> insertion_order,
                             string return_type);
    void generateBoostAssert(
        string class_test, string function_name, string function_cfg_name,
        const std::map<string, std::pair<string, string>> &param_type,
        const vector<string> &insertion_order,
        const std::pair<string, string> &return_type);

    void generateBoostConstructorAssert(string class_test,
                                        string constructor_name,
                                        string constructor_cfg_name,
                                        map<string, string> param_type,
                                        vector<string> insertion_order);

    //--
    void addStructReadToFixture(string type_name,
                                map<string, string> param_type,
                                vector<string> insertion_order,
                                bool overloaded_eq, bool overloaded_flux);
    void addNewTypeToFixture(string type_name, string fixture_path);
    void addNewTypeToFixture(const std::pair<string, string> &type,
                             string fixture_path);

private:
    void generateFixture(string outputPath);
    void generateMakefile(string outputPath);
    // vector<string> fillDefaultTypes(string path);
    // TODO: eliminar esta version
    void checkTypes(string type, string support_path);
    void checkTypes(const std::pair<string, string> &type, string support_path);

    map<string, string> valuesToChange;
    // vector<string> defaultTypes;
    bool isFromClass;

    string fixture_path, makefile_path;
    string ASKELETON_HOME;
};