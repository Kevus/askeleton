#include "ConfigGenerator.hpp"
#include "VariableInfo.hpp"
#include <regex>

/**
 * @brief This class generates: BOOST asserts (test file) and reading methods
 * (fixture file).
 *
 */
class BoostGenerator {
public:
    BoostGenerator(string filePath, string cfgName, bool isFromClass);

    void generateBoostAssert(const string &classTest,
                             const string &functionName,
                             const string &funcCfgName,
                             const vector<InfoVariable> &params,
                             const InfoType &returnType);

    void generateBoostConstructorAssert(const string &classTest,
                                        const string &ctorName,
                                        const string &ctorCfgName,
                                        const vector<InfoVariable> &params);

    // Generates a reading method for the pointer given in type and its
    // underlying type.
    void addPointerReadToFixture(const InfoType &type);
    // Generates a reading method for the enum given in type.
    void addEnumReadToFixture(const InfoType &type);
    // Generates a struct/class method for the record given in type but also the
    // types of its fields.
    void addRecordReadToFixture(const InfoType &type);

    // Generates a reading method for a given type in a secure way (checks if
    // supported before the addition). It also mark the type as supported.
    void generateCustomTypeFixture(const InfoType &type);
    // Generates a reading method for a given type in a secure way (checks if
    // supported before the addition). It also mark the type as supported.
    void generateCustomTypeFixture(const string &filename,
                                   const InfoType &type);

    // Checks if a type was added to the supportedPath
    bool isTypeSupported(const InfoType &type);
    // Checks if a type was added in a given className path
    bool isTypeSupported(const InfoType &type, const string &className);

    // Marks a type as supported in the supportedPath file. Remind that THIS
    // DOES NOT GENERATE TYPE READ METHOD
    void addTypeToSupported(const std::pair<string, string> &type,
                            const string &supportedPath);

    /**
     * TODO: requiere revision y eliminacion de muchas funciones antiguas,
     * mantenidas para la generacion de casos de prueba contra los ctores
     * de las clases (BOOST_ASSERT)
     *
     */
    // @deprecated Manera original, sustituida por su version sobrecargada
    // generateBoostConstructorAssert que recibe solo un vector de InfoVariable
    void generateBoostConstructorAssert(string class_test,
                                        string constructor_name,
                                        string constructor_cfg_name,
                                        map<string, string> param_type,
                                        vector<string> insertion_order);

    // @deprecated Reemplazar por isTypeSupported
    bool checkIfSupported(const std::pair<string, string> &type,
                          const string &supportedPath);

    // @deprecated Reemplazar por su version sobrecargada con InfoType
    void
    addPointerReadToFixture(const std::pair<std::string, std::string> &type);
    // @deprecated Reemplazar por addRecordReadToFixture
    void addStructReadToFixture(string type_name,
                                map<string, string> param_type,
                                vector<string> insertion_order,
                                bool overloaded_eq, bool overloaded_flux);
    // @deprecated Reemplazar por su version sobrecargada con InfoType
    void addEnumReadToFixture(const std::pair<std::string, std::string> &type);

    // @deprecated Reemplazar por generateCustomTypeFixture
    void addNewTypeToFixture(string type_name, string fixture_path);
    // @deprecated Reemplazar por generateCustomTypeFixture
    void addNewTypeToFixture(const std::pair<string, string> &type,
                             string fixture_path);

private:
    void generateFixture(string outputPath) const;
    void generateMakefile(string outputPath);
    void generateSupported() const;

    void addReadObjectToFixture(const std::string &method);
    void addOverloadToFixture(const std::string &overload);

    // vector<string> fillDefaultTypes(string path);

    // TODO: estas funciones no son necesarias, una funcion que comprueba no
    // debe generar tipos.

    // @deprecated En su lugar, usar generateCustomTypeFixture, que lo
    // agrega de manera segura
    void checkTypes(string type, string support_path);
    // @deprecated En su lugar, usar generateCustomTypeFixture, que lo
    // agrega de manera segura
    void checkTypes(const std::pair<string, string> &type, string support_path);

    map<string, string> valuesToChange;
    bool isFromClass;

    string fixture_path, makefile_path, supported_path;
    string ASKELETON_HOME;
    string folderName;

    // vector<string> defaultTypes;
};