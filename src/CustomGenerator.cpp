#include "CustomGenerator.hpp"

#include "utils/system.hpp"
#include "utils/templating.hpp"
#include <filesystem>

using namespace std;

void initialize_file(string source) {
    string comment_header = "";
    if (!fileExists(source))
        comment_header = getCommentHeader(source);

    ofstream file;
    file.open(source, ios_base::app);
    if (file.is_open()) {
        file << comment_header;
        file.close();
    }
}

CustomGenerator::CustomGenerator(string filename) : filename(filename) {
    // We will create the file if it doesn't exist
    string sys_command = "mkdir -p Generated/UT/" + filename;
    system(sys_command.c_str());

    string source = "Generated/UT/" + filename + "/" + filename + "_types";
    initialize_file(source);
    types_file.open(source, ios_base::app);

    source = "Generated/UT/" + filename + "/" + filename + "_testcases";
    initialize_file(source);
    testcases_file.open(source, ios_base::app);
}

CustomGenerator::~CustomGenerator() {
    if (types_file.is_open())
        types_file.close();

    if (testcases_file.is_open())
        testcases_file.close();
}

void CustomGenerator::generateTypesFile(string funct_name,
                                        map<string, string> param_type,
                                        vector<string> insert_order,
                                        string return_type) {
    if (types_file.is_open()) {
        types_file << funct_name << ":";

        for (auto i : insert_order)
            types_file << param_type[i] << ",";

        types_file << "return_" << return_type << "\n";
    }
}

void CustomGenerator::generateTestCasesFile(string funct_name,
                                            map<string, string> param_type,
                                            vector<string> insert_order,
                                            string return_type) {
    if (testcases_file.is_open()) {
        testcases_file << funct_name << ":";

        for (auto i : insert_order)
            testcases_file << "0" << ",";

        testcases_file << "0" << "\n";
    }
}

//%-------------------------------------------------------------------------------
/*
bool CustomReader::generateExecutableTests(string class_test, string types_path,
string values_path, string output_path)
{
    if(!fileExists(types_path) || !fileExists(values_path))
    {
        return false;
    } else
    {
        string fileContent;
        stringstream test_case;

        //=================================================================
        // Generation of final test file
        //=================================================================
        if(!fileExists(output_path))
        {
            string template_path = "Generator/Templates/BoostTest.tpl";
            ifstream tplFile (template_path);

            if(tplFile.is_open())
            {

                fileContent = string( (istreambuf_iterator<char>(tplFile)),
                                   istreambuf_iterator<char>() );

                replaceAll(fileContent, "{className}", class_test);

                //==========================================================
                // These tags are not used yet, so we will simply delete them
                //==========================================================
                replaceAll(fileContent, "{pointerInitToken}", "");
                replaceAll(fileContent, "{pointerDestroyToken}", "");
                //==========================================================
                //==========================================================
            }
        } else
        {
            ifstream tmp_output(outputPath);
            fileContent = string( (istreambuf_iterator<char>(tmp_output)),
                                   istreambuf_iterator<char>() );
        }
        //==================================================================

        //==================================================================
        // Generation of test cases
        //==================================================================

        ifstream types_file (types_path);
        ifstream values_file (values_path);

        map<string, vector<string> > method_types = getMapValues(types_file);
        map<string, vector<string> > method_values = getMapValues(values_path);

        for(auto i : method_values)
        {
            test_case << "\tBOOST_CHECK_EQUAL(";
            if (isFromClass) test_case << class_test << "_test.";
            test_case << i->first << "(";

            for(auto j : i->second)
            {

            }


        }

        vector<string> types;
        vector<string> values;

        string method_name;
        string line;

        if (types_file.is_open())
        {
            while(getline(types_file, line))
            {
                //All spaces will be removed from the line
                line.erase(remove_if(line.begin(), line.end(), ::isspace),
line.end());

                //If it is a comment, it will be ignored
                if (line[0] == '#' || line.empty())
                {
                    continue;
                } else
                {
                    auto delimiter = line.find(":");
                    auto key = line.substr(0, delimiter);
                    auto value = line.substr(delimiter + 1);

                    //Now we have the name
                    method_name = key;

                    delimiter = value.find(",");

                    while( delimiter != string::npos )
                    {
                        key = value.substr(0, delimiter);
                        value = value.substr(delimiter + 1);

                        types.push_back(key);

                        delimiter = value.find(",");
                    }

                    method_types.insert(pair<string, vector<string>(method_name,
types)); types.clear();
                }
            }
        }

        //==================================================================
    }
}

map<string, vector<string>> CustomReader getMapValues(ifstream file)
{
        map<string, vector<string> > method_values;

        vector<string> values;

        string method_name;
        string line;

        if (file.is_open())
        {
            while(getline(file, line))
            {
                //All spaces will be removed from the line
                line.erase(remove_if(line.begin(), line.end(), ::isspace),
line.end());

                //If it is a comment, it will be ignored
                if (line[0] == '#' || line.empty())
                {
                    continue;
                } else
                {
                    auto delimiter = line.find(":");
                    auto key = line.substr(0, delimiter);
                    auto value = line.substr(delimiter + 1);

                    //Now we have the name
                    method_name = key;

                    delimiter = value.find(",");

                    while( delimiter != string::npos )
                    {
                        key = value.substr(0, delimiter);
                        value = value.substr(delimiter + 1);

                        values.push_back(key);

                        delimiter = value.find(",");
                    }

                    method_values.insert(pair<string,
vector<string>(method_name, values)); values.clear();
                }
            }
        }
}*/