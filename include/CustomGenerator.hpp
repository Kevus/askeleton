#ifndef CUSTOMGENERATOR_HPP
#define CUSTOMGENERATOR_HPP

#include <fstream>
#include <map>
#include <string>
#include <vector>

void initialize_file(std::string source);

class CustomGenerator {
public:
    CustomGenerator(std::string filename = "ASKGeneratedTestsConfig");
    ~CustomGenerator();

    void generateTypesFile(std::string funct_name,
                           std::map<std::string, std::string> param_type,
                           std::vector<std::string> insert_order,
                           std::string return_type);
    void generateTestCasesFile(std::string funct_name,
                               std::map<std::string, std::string> param_type,
                               std::vector<std::string> insert_order,
                               std::string return_type);

private:
    std::string filename;

    std::ofstream types_file;
    std::ofstream testcases_file;
};
/*
class CustomReader
{
public:

    void generateExecutableTests(string types_path, string values_path);
private:
    bool isFromClass;

    ofstream types_file;
    ofstream values_file;
};*/

#endif