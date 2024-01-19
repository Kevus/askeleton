#include "ConfigGenerator.hpp"
#include <regex>

class BoostGenerator {
public:
    BoostGenerator(string filePath, string cfgName, bool isFromClass);

    // TODO: eliminar esta version mas adelante
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

    // Add the enum defined in type to the Fixture
    void addEnumReadToFixture(const std::pair<std::string, std::string> &type);

    // Add the pointer defined in type to the Fixture
    void
    addPointerReadToFixture(const std::pair<std::string, std::string> &type);

    // CHECK: por que se pide el fixture_path si ya esta como miembro?
    void addNewTypeToFixture(string type_name, string fixture_path);
    void addNewTypeToFixture(const std::pair<string, string> &type,
                             string fixture_path);

    //  Checks if a type was added to the supportedPath
    bool checkIfSupported(const std::pair<string, string> &type,
                          const string &supportedPath);

    //  Add a type as supported in the supportedPath file
    void addTypeToSupported(const std::pair<string, string> &type,
                            const string &supportedPath);

private:
    void generateFixture(string outputPath) const;
    void generateMakefile(string outputPath);
    // vector<string> fillDefaultTypes(string path);
    // TODO: eliminar esta version mas adelante
    void checkTypes(string type, string support_path);
    void checkTypes(const std::pair<string, string> &type, string support_path);

    map<string, string> valuesToChange;
    // vector<string> defaultTypes;
    bool isFromClass;

    // TODO: agregar el supported_path ?
    string fixture_path, makefile_path;
    string ASKELETON_HOME;
};