#include "ConfigGenerator.hpp"
#include "VariableInfo.hpp"
#include <regex>

class BoostGenerator {
public:
    BoostGenerator(string filePath, string cfgName, bool isFromClass);

    void generateBoostAssert(const string &classTest,
                             const string &functionName,
                             const string &funcCfgName,
                             const vector<InfoVariable> &params,
                             const InfoType &returnType);

    void generateBoostConstructorAssert(string class_test,
                                        string constructor_name,
                                        string constructor_cfg_name,
                                        map<string, string> param_type,
                                        vector<string> insertion_order);
    void generateBoostConstructorAssert(const string &classTest,
                                        const string &ctorName,
                                        const string &ctorCfgName,
                                        const vector<InfoVariable> &params);

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
    void addPointerReadToFixture(const InfoType &type);
    void addEnumReadToFixture(const InfoType &type);
    void addRecordReadToFixture(const InfoType &type);

    void generateCustomTypeFixture(const InfoType &type);
    void generateCustomTypeFixture(const string &filename,
                                   const InfoType &type);

    void addNewTypeToFixture(string type_name, string fixture_path);
    void addNewTypeToFixture(const std::pair<string, string> &type,
                             string fixture_path);

    //  Checks if a type was added to the supportedPath
    bool isTypeSupported(const InfoType &type);
    bool isTypeSupported(const InfoType &type, const string &className);
    bool checkIfSupported(const std::pair<string, string> &type,
                          const string &supportedPath);

    //  Add a type as supported in the supportedPath file
    void addTypeToSupported(const std::pair<string, string> &type,
                            const string &supportedPath);

private:
    void generateFixture(string outputPath) const;
    void generateMakefile(string outputPath);
    void generateSupported() const;
    // vector<string> fillDefaultTypes(string path);
    // TODO: eliminar esta version mas adelante
    void checkTypes(string type, string support_path);
    void checkTypes(const std::pair<string, string> &type, string support_path);
    void addReadObjectToFixture(const std::string &method);
    void addOverloadToFixture(const std::string &overload);

    map<string, string> valuesToChange;
    // vector<string> defaultTypes;
    bool isFromClass;

    string fixture_path, makefile_path, supported_path;
    string ASKELETON_HOME;
    string folderName;
};