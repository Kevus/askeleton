#pragma once

#include <string>
#include <map>
#include <vector>

#include "ConfigGenerator.hpp"
#include "VariableInfo.hpp"

class BoostGenerator {
public:
    BoostGenerator(std::string filePath, std::string cfgName, bool isFromClass);

    void generateBoostAssert(const std::string &classTest,
                             const std::string &functionName,
                             const std::string &funcCfgName,
                             const std::vector<InfoVariable> &params,
                             const InfoType &returnType);

    void
    generateBoostConstructorAssert(const std::string &classTest,
                                   const std::string &ctorName,
                                   const std::string &ctorCfgName,
                                   const std::vector<InfoVariable> &params);

    // Generates a reading method for the pointer given in type and its
    // underlying type.
    void addPointerReadToFixture(const InfoType &type);
    // Generates a reading method for the enum given in type.
    void addEnumReadToFixture(const InfoType &type);
    // Generates a struct/class method for the record given in type but also the
    // types of its fields.
    void addRecordReadToFixture(const InfoType &type, unsigned level = 0);

    // Generates a reading method for a given type in a secure way (checks if
    // supported before the addition). It also mark the type as supported.
    void generateCustomTypeFixture(const InfoType &type, unsigned level = 0);
    // Generates a reading method for a given type in a secure way (checks if
    // supported before the addition). It also mark the type as supported.
    void generateCustomTypeFixture(const std::string &filename,
                                   const InfoType &type, unsigned level = 0);

    // Checks if a type was added to the supportedPath
    bool isTypeSupported(const InfoType &type);
    // Checks if a type was added in a given className path
    bool isTypeSupported(const InfoType &type, const std::string &className);

    // Marks a type as supported in the supportedPath file. Remind that THIS
    // DOES NOT GENERATE TYPE READ METHOD
    void addTypeToSupported(const std::pair<std::string, std::string> &type,
                            const std::string &supportedPath);

    /**
     * TODO: requiere revision y eliminacion de muchas funciones antiguas,
     * mantenidas para la generacion de casos de prueba contra los ctores
     * de las clases (BOOST_ASSERT)
     *
     */
    // @deprecated Manera original, sustituida por su version sobrecargada
    // generateBoostConstructorAssert que recibe solo un vector de InfoVariable
    void
    generateBoostConstructorAssert(std::string class_test,
                                   std::string constructor_name,
                                   std::string constructor_cfg_name,
                                   std::map<std::string, std::string> param_type,
                                   std::vector<std::string> insertion_order);

    // @deprecated Reemplazar por isTypeSupported
    bool checkIfSupported(const std::pair<std::string, std::string> &type,
                          const std::string &supportedPath);

    // @deprecated Reemplazar por su version sobrecargada con InfoType
    void
    addPointerReadToFixture(const std::pair<std::string, std::string> &type);
    // @deprecated Reemplazar por addRecordReadToFixture
    void addStructReadToFixture(std::string type_name,
                                std::map<std::string, std::string> param_type,
                                std::vector<std::string> insertion_order,
                                bool overloaded_eq, bool overloaded_flux);
    // @deprecated Reemplazar por su version sobrecargada con InfoType
    void addEnumReadToFixture(const std::pair<std::string, std::string> &type);

    // @deprecated Reemplazar por generateCustomTypeFixture
    void addNewTypeToFixture(std::string type_name, std::string fixture_path);
    // @deprecated Reemplazar por generateCustomTypeFixture
    void addNewTypeToFixture(const std::pair<std::string, std::string> &type,
                             std::string fixture_path);

private:
    void generateFixture(std::string outputPath) const;
    void generateMakefile(std::string outputPath);
    void generateSupported() const;

    void addReadObjectToFixture(const std::string &method);
    void addOverloadToFixture(const std::string &overload);

    // std::vector<string> fillDefaultTypes(string path);

    // TODO: estas funciones no son necesarias, una funcion que comprueba no
    // debe generar tipos.

    // @deprecated En su lugar, usar generateCustomTypeFixture, que lo
    // agrega de manera segura
    void checkTypes(std::string type, std::string support_path);
    // @deprecated En su lugar, usar generateCustomTypeFixture, que lo
    // agrega de manera segura
    void checkTypes(const std::pair<std::string, std::string> &type,
                    std::string support_path);

    std::map<std::string, std::string> valuesToChange;
    bool isFromClass;

    std::string fixture_path, makefile_path, supported_path;
    std::string ASKELETON_HOME;
    std::string folderName;

    // std::vector<string> defaultTypes;
};