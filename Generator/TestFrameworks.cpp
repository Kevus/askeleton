#include "TestFrameworks.hpp"

string cleanClassIdentifier(string sToReplace) {
    string result = sToReplace;
    if (result.find("class") != string::npos) {
        replaceAll(result, "class_", "");
        replaceAll(result, "class ", "");
    }

    std::regex pattern("Read_(\\w+)::(\\w+)"); //\\((\\w+) (\\w+)\\)");
    std::string output = std::regex_replace(result, pattern, "Read_$1_$2");

    return output;
}

BoostGenerator::BoostGenerator(string filePath, string cfgName,
                               bool isFromClass)
    : isFromClass(isFromClass) {
    // Time utilities
    auto t = time(nullptr);
    auto tm = *localtime(&t);
    ostringstream dateStream;

    if (getenv("ASKELETON_HOME") != NULL) {
        ASKELETON_HOME = getenv("ASKELETON_HOME");
        if (!endsWith(ASKELETON_HOME, "/")) {
            ASKELETON_HOME += "/";
        }
    } else
        ASKELETON_HOME = "";

    valuesToChange.insert(pair<string, string>("{filePath}", filePath));
    valuesToChange.insert(pair<string, string>("{cfgName}", cfgName));

    // String obtained from 'filePath', containing only the name of the file
    // without extension
    string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
    fileName = fileName.substr(0, fileName.find_last_of("."));
    valuesToChange.insert(pair<string, string>("{fileName}", fileName));

    // Given the filePath to the .h or .hpp, it will look for the .c or .cpp in
    // the same directory. If it doesnt exist, it will show an error
    string cppPath = filePath.substr(0, filePath.find_last_of("."));
    if (fileExists(cppPath + ".cpp")) {
        valuesToChange.insert(
            pair<string, string>("{cppPath}", cppPath + ".cpp"));
    } else if (fileExists(cppPath + ".c")) {
        valuesToChange.insert(
            pair<string, string>("{cppPath}", cppPath + ".c"));
    } else {
        cout << "ERROR: .cpp file not found. It will not be included in the "
                "makefile"
             << endl;
        valuesToChange.insert(pair<string, string>("{cppPath}", filePath));
    }

    dateStream << put_time(&tm, "%d-%m-%Y %H:%M:%S");

    valuesToChange.insert(
        pair<string, string>("{dateOfGeneration}", dateStream.str()));

    //==========================================================
    // These methods have not been implemented
    //==========================================================
    valuesToChange.insert(pair<string, string>("{includes}", ""));
    valuesToChange.insert(pair<string, string>("{namespaces}", ""));
    // valuesToChange.insert(pair<string,string>("{readObject}", ""));
    valuesToChange.insert(pair<string, string>("{newMethods}", ""));
    //==========================================================
    //==========================================================

    if (isFromClass) {
        valuesToChange.insert(pair<string, string>("{className}", cfgName));
        valuesToChange.insert(
            pair<string, string>("{classNameTest}", cfgName + "_test;"));
    } else {
        valuesToChange.insert(pair<string, string>("{className}", ""));
        valuesToChange.insert(pair<string, string>("{classNameTest}", ""));
    }

    // We will create the folder if it doesn't exist
    if (!folderExists("Generated/UT/" + cfgName)) {
        string sys_command = "mkdir -p Generated/UT/" + cfgName;
        system(sys_command.c_str());
    }
    fixture_path = "Generated/UT/" + cfgName + "/" + cfgName + "_fixture.hpp";
    makefile_path = "Generated/UT/" + cfgName + "/makefile";

    generateFixture(fixture_path);
    generateMakefile(makefile_path);
}

void BoostGenerator::generateFixture(string outputPath) const {
    if (!fileExists(outputPath)) {
        string templatePath =
            ASKELETON_HOME + "Generator/Templates/Fixture.tpl";
        string fileContent;

        ifstream tplFile(templatePath);
        ofstream outputFile(outputPath);

        if (tplFile.is_open() && outputFile.is_open()) {
            // Read the entire template into memory
            fileContent = string((istreambuf_iterator<char>(tplFile)),
                                 istreambuf_iterator<char>());

            for (auto i : valuesToChange)
                replaceAll(fileContent, i.first, i.second);

            outputFile << fileContent;

            outputFile.close();
            tplFile.close();
        } else
            cout << "ERROR: COULDN'T WRITE FIXTURE FILE. IT WILL NOT BE CREATED"
                 << endl; // if tpl
    }                     // if fileExist
}

void BoostGenerator::generateMakefile(string outputPath) {
    if (!fileExists(outputPath)) {
        string templatePath =
            ASKELETON_HOME + "Generator/Templates/makefile.tpl";
        string fileContent;

        ifstream tplFile(templatePath);
        ofstream outputFile(outputPath);

        if (tplFile.is_open() && outputFile.is_open()) {
            // Read the entire template into memory
            fileContent = string((istreambuf_iterator<char>(tplFile)),
                                 istreambuf_iterator<char>());

            for (auto i : valuesToChange)
                replaceAll(fileContent, i.first, i.second);

            outputFile << fileContent;

            outputFile.close();
            tplFile.close();
        } else
            cout << "ERROR: COULDN'T WRITE MAKEFILE. IT WILL NOT BE CREATED"
                 << endl; // if tpl
    }                     // if fileExist
}

void BoostGenerator::generateBoostAssert(
    string class_test, string function_name, string function_cfg_name,
    const map<string, pair<string, string>> &param_type,
    const vector<string> &insertion_order,
    const pair<string, string> &return_type) {

    string templatePath = ASKELETON_HOME + "Generator/Templates/BoostTest.tpl";
    string outputPath =
        "Generated/UT/" + class_test + "/" + class_test + "_test.cpp";
    string fileContent, ptype;

    stringstream test_case;

    bool existFlag = fileExists(outputPath);
    ifstream tplFile(templatePath);

    if (tplFile.is_open()) {
        // If file doesnt exist, then replace common tags
        if (!existFlag) {
            fileContent = string((istreambuf_iterator<char>(tplFile)),
                                 istreambuf_iterator<char>());

            replaceAll(fileContent, "{className}", class_test);

            //==========================================================
            // These tags are not used yet, so we will simply delete them
            //==========================================================
            replaceAll(fileContent, "{pointerInitToken}", "");
            replaceAll(fileContent, "{pointerDestroyToken}", "");
            //==========================================================
            //==========================================================

            // Copy makefile and supported types for compiling tests
            // system(("cp -r " + ASKELETON_HOME + "Generator/Templates/makefile
            // Generated/UT/" + class_test + "/").c_str());
            system(("cp -r " + ASKELETON_HOME +
                    "Generator/Templates/SupportedTypes.txt Generated/UT/" +
                    class_test + "/")
                       .c_str());
        } else {
            ifstream tmp_output(outputPath);
            fileContent = string((istreambuf_iterator<char>(tmp_output)),
                                 istreambuf_iterator<char>());
        }

        // List, vector and map will have a different test case
        bool return_container =
            (return_type.first.find("list") != string::npos ||
             return_type.first.find("vector") != string::npos ||
             return_type.first.find("map") != string::npos);

        string formatted, original;
        // Lets check pointers...
        for (auto i : insertion_order) {
            tie(original, formatted) = param_type.at(i);
            // checkTypes(param_type.at(i),
            //            "Generated/UT/" + class_test + "/SupportedTypes.txt");
            replaceAll(ptype, "struct_", "");

            if (formatted.find("_&") != string::npos ||
                formatted.find("const_") != string::npos) {
                ptype = formatted;

                replaceAll(ptype, "_",
                           " "); // delete '_' for using it as a type
                replaceAll(ptype, " &", ""); // Then delete const and/or '&'

                test_case << "\n\t" << ptype << " ";

                // TODO: generalizar
                replaceAll(ptype, " ", "_");
                replaceAll(ptype, "const_", "");
                replaceAll(ptype, "*", "s");

                test_case << function_cfg_name << "_" << i << " = Read_"
                          << ptype << "(\"" << function_cfg_name << "." << i
                          << "\");\n";
            }
        }

        tie(original, formatted) = return_type;
        // checkTypes(return_type,
        //            "Generated/UT/" + class_test + "/SupportedTypes.txt");
        replaceAll(formatted, "struct_", "");

        if (formatted.find("_&") != string::npos ||
            formatted.find("const_") != string::npos) {
            ptype = formatted;
            replaceAll(ptype, "_", " ");
            replaceAll(ptype, " &", "");

            test_case << "\t" << ptype << " return_" << function_cfg_name;

            // TODO: generalizar
            replaceAll(ptype, " ", "_");
            replaceAll(ptype, "const_", "");
            replaceAll(ptype, "*", "s");

            test_case << " = Read_" << ptype << "(\"" << function_cfg_name
                      << ".return_" << formatted << "\");\n";
        }

        // Now we will create the assertion sentence
        if (!return_container)
            test_case << "\tBOOST_CHECK_EQUAL(";
        else
            test_case << "\tBOOST_CHECK((";

        if (isFromClass)
            test_case << class_test << "_test.";

        test_case << function_name << "(";

        for (auto i : insertion_order) {
            string formatted_type = param_type.at(i).second;
            replaceAll(formatted_type, "struct_", "");
            if (formatted_type.find("_&") == string::npos &&
                formatted_type.find("const_") == string::npos) {
                replaceAll(formatted_type, "*", "s");
                test_case << "Read_" << formatted_type << "(\""
                          << function_cfg_name << "." << i << "\")";
            } else {
                test_case << function_cfg_name << "_" << i;
            }

            if (i != insertion_order.back())
                test_case << ",";
        }

        // test_case << "));\n{assert}";
        if (!return_container)
            test_case << "),";
        else
            test_case << ") == ";

        if (formatted.find("_&") == string::npos &&
            formatted.find("const_") == string::npos) {
            string read = "Read_" + formatted;
            replaceAll(read, "*", "s");
            test_case << read << "(\"" << function_name << ".return_"
                      << formatted << "\"))";
        } else {
            test_case << "return_" << function_cfg_name << ")";
        }

        if (!return_container)
            test_case << ";";
        else
            test_case << ");";

        test_case << "\n//{assert}";

        string result_test_case = cleanClassIdentifier(test_case.str());
        replaceAll(result_test_case, "::", "_");
        replaceAll(fileContent, "//{assert}", result_test_case);

        ofstream outputFile(outputPath);
        outputFile << fileContent;

        tplFile.close();
        outputFile.close();
    }
}

void BoostGenerator::generateBoostAssert(string class_test,
                                         string function_name,
                                         string function_cfg_name,
                                         map<string, string> param_type,
                                         vector<string> insertion_order,
                                         string return_type) {
    string templatePath = ASKELETON_HOME + "Generator/Templates/BoostTest.tpl";
    string outputPath =
        "Generated/UT/" + class_test + "/" + class_test + "_test.cpp";
    string fileContent;
    string ptype;

    stringstream test_case;

    bool existFlag = fileExists(outputPath);
    ifstream tplFile(templatePath);

    if (tplFile.is_open()) {
        // If file doesnt exist, then replace common tags
        if (!existFlag) {
            fileContent = string((istreambuf_iterator<char>(tplFile)),
                                 istreambuf_iterator<char>());

            replaceAll(fileContent, "{className}", class_test);

            //==========================================================
            // These tags are not used yet, so we will simply delete them
            //==========================================================
            replaceAll(fileContent, "{pointerInitToken}", "");
            replaceAll(fileContent, "{pointerDestroyToken}", "");
            //==========================================================
            //==========================================================

            // Copy makefile and supported types for compiling tests
            // system(("cp -r " + ASKELETON_HOME + "Generator/Templates/makefile
            // Generated/UT/" + class_test + "/").c_str());
            system(("cp -r " + ASKELETON_HOME +
                    "Generator/Templates/SupportedTypes.txt Generated/UT/" +
                    class_test + "/")
                       .c_str());
        } else {
            ifstream tmp_output(outputPath);
            fileContent = string((istreambuf_iterator<char>(tmp_output)),
                                 istreambuf_iterator<char>());
        }

        // List, vector and map will have a different test case
        bool return_container = (return_type.find("list") != string::npos ||
                                 return_type.find("vector") != string::npos ||
                                 return_type.find("map") != string::npos);

        // Lets check pointers...
        for (auto i : insertion_order) {
            checkTypes(param_type[i],
                       "Generated/UT/" + class_test + "/SupportedTypes.txt");

            if (param_type[i].find("_&") != string::npos ||
                param_type[i].find("const_") != string::npos) {
                ptype = param_type[i];

                replaceAll(ptype, "_",
                           " "); // delete '_' for using it as a type
                replaceAll(ptype, " &", ""); // Then delete const and/or '&'

                test_case << "\n\t" << ptype << " ";

                // TODO: generalizar
                replaceAll(ptype, " ", "_");
                replaceAll(ptype, "const_", "");
                replaceAll(ptype, "*", "s");
                replaceAll(ptype, "struct_", "");

                test_case << function_cfg_name << "_" << i << " = Read_"
                          << ptype << "(\"" << function_cfg_name << "." << i
                          << "\");\n";
            }
        }

        checkTypes(return_type,
                   "Generated/UT/" + class_test + "/SupportedTypes.txt");
        if (return_type.find("_&") != string::npos ||
            return_type.find("const_") != string::npos) {
            ptype = return_type;
            replaceAll(ptype, "_", " ");
            replaceAll(ptype, " &", "");

            test_case << "\t" << ptype << " return_" << function_cfg_name;

            // TODO: generalizar
            replaceAll(ptype, " ", "_");
            replaceAll(ptype, "const_", "");
            replaceAll(ptype, "*", "s");
            replaceAll(ptype, "struct_", "");

            test_case << " = Read_" << ptype << "(\"" << function_cfg_name
                      << ".return_" << return_type << "\");\n";
        }

        // Now we will create the assertion sentence
        if (!return_container)
            test_case << "\tBOOST_CHECK_EQUAL(";
        else
            test_case << "\tBOOST_CHECK((";

        if (isFromClass)
            test_case << class_test << "_test.";

        test_case << function_name << "(";

        for (auto i : insertion_order) {
            if (param_type[i].find("_&") == string::npos &&
                param_type[i].find("const_") == string::npos) {
                replaceAll(param_type[i], "*", "s");
                test_case << "Read_" << param_type[i] << "(\""
                          << function_cfg_name << "." << i << "\")";
            } else {
                test_case << function_cfg_name << "_" << i;
            }

            if (i != insertion_order.back())
                test_case << ",";
        }

        // test_case << "));\n{assert}";
        if (!return_container)
            test_case << "),";
        else
            test_case << ") == ";

        if (return_type.find("_&") == string::npos &&
            return_type.find("const_") == string::npos) {
            string read = "Read_" + return_type;
            replaceAll(read, "*", "s");
            test_case << read << "(\"" << function_name << ".return_"
                      << return_type << "\"))";
        } else {
            test_case << "return_" << function_cfg_name << ")";
        }

        if (!return_container)
            test_case << ";";
        else
            test_case << ");";

        test_case << "\n//{assert}";

        string result_test_case = cleanClassIdentifier(test_case.str());
        replaceAll(result_test_case, "::", "_");
        replaceAll(fileContent, "//{assert}", result_test_case);

        ofstream outputFile(outputPath);
        outputFile << fileContent;

        tplFile.close();
        outputFile.close();
    }
}

void BoostGenerator::generateBoostConstructorAssert(
    string class_test, string constructor_name, string constructor_cfg_name,
    map<string, string> param_type, vector<string> insertion_order) {
    string templatePath = ASKELETON_HOME + "Generator/Templates/BoostTest.tpl";
    string outputPath =
        "Generated/UT/" + class_test + "/" + class_test + "_test.cpp";
    string fileContent;
    string ptype;

    stringstream test_case;

    bool existFlag = fileExists(outputPath);
    ifstream tplFile(templatePath);

    if (tplFile.is_open()) {
        // If file doesnt exist, then replace common tags
        if (!existFlag) {
            fileContent = string((istreambuf_iterator<char>(tplFile)),
                                 istreambuf_iterator<char>());

            replaceAll(fileContent, "{className}", class_test);

            //==========================================================
            // These tags are not used yet, so we will simply delete them
            //==========================================================
            replaceAll(fileContent, "{pointerInitToken}", "");
            replaceAll(fileContent, "{pointerDestroyToken}", "");
            //==========================================================
            //==========================================================

            // Copy makefile for compiling tests
            // system(("cp -r " + ASKELETON_HOME + "Generator/Templates/makefile
            // Generated/UT/" + class_test + "/").c_str());
            system(("cp -r " + ASKELETON_HOME +
                    "Generator/Templates/SupportedTypes.txt Generated/UT/" +
                    class_test + "/")
                       .c_str());
        } else {
            ifstream tmp_output(outputPath);
            fileContent = string((istreambuf_iterator<char>(tmp_output)),
                                 istreambuf_iterator<char>());
        }

        // Now we will create the assertion sentence
        test_case << "\tBOOST_CHECK(new ";
        test_case << constructor_name << "(";

        for (auto i : insertion_order) {
            checkTypes(param_type[i],
                       "Generated/UT/" + class_test + "/SupportedTypes.txt");

            if (param_type[i].find("_&") == string::npos &&
                param_type[i].find("const_") == string::npos) {
                test_case << "Read_" << param_type[i] << "(\""
                          << constructor_cfg_name << "." << i << "\")";
            } else {
                test_case << constructor_cfg_name << "_" << i;
            }

            if (i != insertion_order.back())
                test_case << ",";
        }

        test_case << "));\n//{assert}";
        // test_case << "),Read_" << return_type << "(\"" << function_name <<
        // ".return_" << return_type << "\"));\n//{assert}";

        string result_test_case = cleanClassIdentifier(test_case.str());
        replaceAll(result_test_case, "::", "_");
        replaceAll(fileContent, "//{assert}", result_test_case);

        ofstream outputFile(outputPath);
        outputFile << fileContent;

        tplFile.close();
        outputFile.close();
    }
}

void BoostGenerator::addStructReadToFixture(string type_name,
                                            map<string, string> param_type,
                                            vector<string> insertion_order,
                                            bool overloaded_eq,
                                            bool overloaded_flux) {
    bool existFlag = fileExists(fixture_path);

    if (!existFlag)
        generateFixture(fixture_path);

    ifstream fixture_file(fixture_path);
    string fileContent = string((istreambuf_iterator<char>(fixture_file)),
                                istreambuf_iterator<char>());

    // TO-DO: This will be a problem when using custom types...
    /**
    ** EXAMPLE
    **
    ** customType Read_struct_customType(string objectKey)
    {
        string object = readObject(objectKey);
        vector<string> values;

        //1,2,3,4
        auto delimiter = object.find(",");
        while(delimiter != string::npos)
        {
            auto key = object.substr(0, delimiter);
            object = object.substr(delimiter + 1);

            values.push_back(key);

            delimiter = object.find(",");
        }

        int number = boost::lexical_cast<int>(values[0]);
        string name = boost::lexical_cast<string>(values[1]);

        customType result;
        result.number = number;
        result.name = name;

        return result;
    }
    **
    **/

    stringstream overloaded_operators;
    stringstream read_method;

    // TODO: requiere revisión
    // Heredia: Eliminado el prefijo struct_
    read_method << type_name << " Read_" << type_name
                << "(string objectKey)\n\t{\n"
                << "\t\tstring object = readObject(objectKey);\n"
                << "\t\tboost::replace_all(object, \";\", \"\");\n"
                << "\t\tvector<string> values;\n\n"
                << "\t\tauto delimiter = object.find(\",\");\n"
                << "\t\twhile( delimiter != string::npos )\n\t\t{\n"
                << "\t\t\tauto key = object.substr(0, delimiter);\n"
                << "\t\t\tobject = object.substr(delimiter + 1);\n\n"
                << "\t\t\tvalues.push_back(key);\n\n"
                << "\t\t\tdelimiter = object.find(\",\");\n\t\t}\n"
                << "\t\tvalues.push_back(object);\n\n";

    //----
    read_method << "\t\t" << type_name << " result;\n";
    //----
    int pos = 0;
    int size = insertion_order.size();

    read_method << "\t\tif( values.size() >= " << size << ")\n\t\t{\n";
    for (auto i : insertion_order) {
        // read_method << "\t\t" << param_type[i] << " " << i << " = ";
        string support_path =
            fixture_path.substr(0, fixture_path.find_last_of("/")) +
            "/SupportedTypes.txt";
        checkTypes(param_type[i], support_path);

        read_method << "\t\t\tresult." << i << " = ";

        if (param_type[i] != "string") {
            read_method << "boost::lexical_cast<" << param_type[i]
                        << ">(values[" << pos << "]);\n";
        } else {
            read_method << "values[" << pos << "];\n";
        }

        pos++;
    }
    read_method << "\t\t}\n";

    read_method << "\n\t\treturn result;\n\t}\n\t//{readObject}";

    replaceAll(fileContent, "//{readObject}", read_method.str());

    // Now ovearload operators if needed
    if (!overloaded_eq) {
        overloaded_operators << "bool operator==(const " << type_name
                             << "& a, const " << type_name << "& b)\n{\n"
                             << "\tbool result = true;\n";

        int size = insertion_order.size();
        if (size > 0) {
            overloaded_operators << "\tresult = (";
            pos = 1;
            for (auto i : insertion_order) {
                overloaded_operators << "a." << i << " == b." << i;

                if (pos != size) {
                    overloaded_operators << ") && \n\t\t(";
                } else {
                    overloaded_operators << ");";
                }

                pos++;
            }
        }

        overloaded_operators << "\n\treturn result;\n}\n\n";

        if (overloaded_flux) {
            overloaded_operators << "//{overloadOperator}";
            replaceAll(fileContent, "//{overloadOperator}",
                       overloaded_operators.str());
        }
    }

    if (!overloaded_flux) {
        overloaded_operators << "ostream& operator<<(ostream& stream, "
                             << "const " << type_name << "& a)\n{\n";

        int size = insertion_order.size();
        if (size > 0) {
            for (auto i : insertion_order) {
                overloaded_operators << "\tstream << a." << i << " << endl;\n";
            }
        }

        overloaded_operators << "\n\treturn stream;\n}\n\n//{overloadOperator}";

        replaceAll(fileContent, "//{overloadOperator}",
                   overloaded_operators.str());
    }

    ofstream outputFile(fixture_path);
    fileContent = cleanClassIdentifier(fileContent);

    outputFile << fileContent;

    fixture_file.close();
    outputFile.close();
}

void BoostGenerator::addEnumReadToFixture(
    const std::pair<std::string, std::string> &type) {

    string original_type, formatted_type;
    tie(original_type, formatted_type) = type;
    bool existFlag = fileExists(fixture_path);

    if (!existFlag)
        generateFixture(fixture_path);

    ifstream fixture_file(fixture_path);
    string fileContent = string((istreambuf_iterator<char>(fixture_file)),
                                istreambuf_iterator<char>());

    /** For example:
        TOD_resultType Read_TOD_resultType(string objectKey) {
            try {
                return static_cast<TOD_resultType>(
                    std::stoi(readObject(objectKey)));
        } catch (const std::exception &e) {
                std::cerr << "Please, check the value of " << objectKey
                    << ". The conversion is invalid: " << e.what()
                    << "\nDefault value is returned instead\n";
                return TOD_resultType();
            }
        }
    */
    stringstream read_method;
    if (original_type.find("*") == string::npos) {
        read_method
            << original_type << " Read_" << formatted_type
            << "(string objectKey) {\n"
            << "\t\ttry {\n"
            << "\t\t\treturn static_cast<" << original_type
            << ">(std::stoi(readObject(objectKey)));\n"
            << "\t\t} catch (const std::exception &e) {\n"
            << "\t\t\tstd::cerr << \"Please, check the value of \" "
               "<< objectKey\n"
            << "\t\t\t\t<<\". The conversion is invalid: \" << e.what()\n"
            << "\t\t\t\t<< \"\\nDefault value is returned instead\\n\";\n"
            << "\t\t\treturn {};\n\t\t}\n\t}";
    } else {
        stringstream allocation_instruction;
        string method_name = original_type;
        string tmp_type = original_type;

        // unsigned_long_* -> unsigned_long_s
        replaceAll(formatted_type, "*", "s");
        // unsigned long * -> unsigned long
        replaceAll(method_name, " *", "");
        // unsigned long -> unsigned_long
        replaceAll(method_name, " ", "_");
        // unsigned long * -> unsigned long
        replaceAll(tmp_type, " *", "");
        // ¿?
        replaceAll(original_type, "class_", "");

        // e.g. int *val = (int*)malloc(sizeof(int))
        allocation_instruction << original_type << "result = (" << original_type
                               << ")malloc(sizeof(" << tmp_type << "))";

        read_method << original_type << " Read_" << formatted_type
                    << "(string objectKey)\n\t{\n"
                    << "\t\t" << allocation_instruction.str() << ";\n"
                    << "\t\tif(result == NULL) {\n"
                    << "\t\t\tcerr << \"Error in memory allocation\\n\";\n"
                    << "\t\t\texit(EXIT_FAILURE);\n"
                    << "\t\t}\n"
                    << "\t\t*result = Read_" << method_name << "(objectKey);\n"
                    << "\t\tpointers.push_back(result);\n"
                    << "\t\treturn result;\n\t}\n"
                    << "\t//{readObject}";
    }

    read_method << "\n//{readObject}";

    // This is necessary, as the AST info will carry the "class"
    // identificator up to this poiunt CHECK: es necesario todavia?
    string definitive_read_method = cleanClassIdentifier(read_method.str());

    replaceAll(fileContent, "//{readObject}", definitive_read_method);

    ofstream outputFile(fixture_path);
    outputFile << fileContent;

    fixture_file.close();
    outputFile.close();
}

void BoostGenerator::addPointerReadToFixture(
    const std::pair<std::string, std::string> &type) {
    string original_type, formatted_type;
    tie(original_type, formatted_type) = type;
    bool existFlag = fileExists(fixture_path);

    if (!existFlag)
        generateFixture(fixture_path);

    ifstream fixture_file(fixture_path);
    string fileContent = string((istreambuf_iterator<char>(fixture_file)),
                                istreambuf_iterator<char>());

    /** For example:
        int *Read_int_s(string objectKey) {
            int *result = (int*)malloc(sizeof(int));
            if(result == NULL) {
                cerr << "Error in memory allocation\n";
                exit(EXIT_FAILURE);
            }
            *result = Read_int(objectKey);
            pointers.push_back(result);
            return result;
        }

        *** PAY ATTENTION that we need the read method of the basic type ***
    */

    stringstream read_method, allocation_instruction;
    string method_name = original_type, tmp_type = original_type;

    // TODO: eliminar
    // unsigned_long_* -> unsigned_long_s
    replaceAll(formatted_type, "*", "s");
    // unsigned long * -> unsigned long
    replaceAll(method_name, " *", "");
    // unsigned long -> unsigned_long
    replaceAll(method_name, " ", "_");
    // unsigned long * -> unsigned long
    replaceAll(tmp_type, " *", "");
    // ¿?
    replaceAll(original_type, "class_", "");

    // e.g. int *val = (int*)malloc(sizeof(int))
    allocation_instruction << original_type << "result = (" << original_type
                           << ")malloc(sizeof(" << tmp_type << "))";

    read_method << original_type << " Read_" << formatted_type
                << "(string objectKey)\n\t{\n"
                << "\t\t" << allocation_instruction.str() << ";\n"
                << "\t\tif(result == NULL) {\n"
                << "\t\t\tcerr << \"Error in memory allocation\\n\";\n"
                << "\t\t\texit(EXIT_FAILURE);\n"
                << "\t\t}\n"
                << "\t\t*result = Read_" << method_name << "(objectKey);\n"
                << "\t\tpointers.push_back(result);\n"
                << "\t\treturn result;\n\t}\n"
                << "\t//{readObject}";

    // This is necessary, as the AST info will carry the "class"
    // identificator up to this poiunt CHECK: es necesario todavia?
    string definitive_read_method = cleanClassIdentifier(read_method.str());

    replaceAll(fileContent, "//{readObject}", definitive_read_method);

    ofstream outputFile(fixture_path);
    outputFile << fileContent;

    fixture_file.close();
    outputFile.close();
}

void BoostGenerator::addNewTypeToFixture(const std::pair<string, string> &type,
                                         string fixture_path) {
    /*** EXAMPLE ***
     ** <original_type> Read_<formatted_type>(string objectKey)
     ** {
     **		<original_type> result =
     *boost::lexical_cast<<original_type>>(result); return result; *	}
     ***************/
    bool existFlag = fileExists(fixture_path);
    string original_type, formatted_type, definitive_read_method;
    stringstream read_method;

    tie(original_type, formatted_type) = type;

    if (!existFlag)
        generateFixture(fixture_path);

    ifstream fixture_file(fixture_path);
    string fileContent = string((istreambuf_iterator<char>(fixture_file)),
                                istreambuf_iterator<char>());

    if (original_type.find("*") == string::npos) {
        read_method << original_type << " Read_" << formatted_type
                    << "(string objectKey)\n\t{\n\t\t" << original_type
                    << " result = boost::lexical_cast<" << original_type
                    << ">(objectKey);\n"
                    << "\t\treturn result;\n\t}\n"
                    << "\t//{readObject}";
    } else {
        stringstream allocation_instruction;
        string method_name = original_type;
        string tmp_type = original_type;

        // unsigned_long_* -> unsigned_long_s
        replaceAll(formatted_type, "*", "s");
        // unsigned long * -> unsigned long
        replaceAll(method_name, " *", "");
        // unsigned long -> unsigned_long
        replaceAll(method_name, " ", "_");
        // unsigned long * -> unsigned long
        replaceAll(tmp_type, " *", "");
        // ¿?
        replaceAll(original_type, "class_", "");

        // e.g. int *val = (int*)malloc(sizeof(int))
        allocation_instruction << original_type << "result = (" << original_type
                               << ")malloc(sizeof(" << tmp_type << "))";

        read_method << original_type << " Read_" << formatted_type
                    << "(string objectKey)\n\t{\n"
                    << "\t\t" << allocation_instruction.str() << ";\n"
                    << "\t\tif(result == NULL) {\n"
                    << "\t\t\tcerr << \"Error in memory allocation\\n\";\n"
                    << "\t\t\texit(EXIT_FAILURE);\n"
                    << "\t\t}\n"
                    << "\t\t*result = Read_" << method_name << "(objectKey);\n"
                    << "\t\tpointers.push_back(result);\n"
                    << "\t\treturn result;\n\t}\n"
                    << "\t//{readObject}";
    }

    // This is necessary, as the AST info will carry the "class" identificator
    // up to this poiunt
    definitive_read_method = cleanClassIdentifier(read_method.str());

    replaceAll(fileContent, "//{readObject}", definitive_read_method);

    ofstream outputFile(fixture_path);
    outputFile << fileContent;

    fixture_file.close();
    outputFile.close();
}

void BoostGenerator::addNewTypeToFixture(string type_name,
                                         string fixture_path) {
    /*** EXAMPLE ***
     ** <formatted_type> Read_<type_name>(string objectKey)
     ** {
     **		<formatted_type> result =
     *boost::lexical_cast<<formatted_type>>(result); *		return result;
     **	}
     ***************/
    bool existFlag = fileExists(fixture_path);

    if (!existFlag)
        generateFixture(fixture_path);

    ifstream fixture_file(fixture_path);
    string fileContent = string((istreambuf_iterator<char>(fixture_file)),
                                istreambuf_iterator<char>());

    string formatted_type, definitive_read_method;
    stringstream read_method;

    /*replaceAll(type_name, "_&", "");
    replaceAll(type_name, "const_", "");*/

    formatted_type = type_name;

    // replaceAll(formatted_type, "_*", "");
    replaceAll(formatted_type, "_", " ");

    if (type_name.find("*") == string::npos) {
        read_method << type_name << " Read_" << type_name
                    << "(string objectKey)\n\t{\n\t\t" << formatted_type
                    << " result = boost::lexical_cast<" << formatted_type
                    << ">(objectKey);\n"
                    << "\t\treturn result;\n\t}\n"
                    << "\t//{readObject}";
    } else {
        stringstream allocation_instruction;
        string method_name = formatted_type;
        string tmp_type = formatted_type;

        replaceAll(type_name, "*", "s");
        replaceAll(method_name, " *", "");
        replaceAll(tmp_type, " *", "");
        replaceAll(method_name, " ", "_");
        replaceAll(formatted_type, "class_", "");

        // Modified with @Heredia99 suggestions.
        // read_method << formatted_type << " Read_" << type_name << "(string
        // objectKey)\n\t{\n\t\t"
        //			<< tmp_type << " tmp = Read_" << method_name <<
        //"(objectKey);\n"
        //			<< "\t\t" << formatted_type << "result = &tmp;\n"
        //			<< "\t\treturn result;\n\t}\n"
        //			<< "\t//{readObject}";

        // e.g.: int *val = (int*)malloc(sizeof(int))
        allocation_instruction << formatted_type << "result = ("
                               << formatted_type << ")malloc(sizeof("
                               << tmp_type << "))";

        read_method << formatted_type << " Read_" << type_name
                    << "(string objectKey)\n\t{\n"
                    << "\t\t" << allocation_instruction.str() << ";\n"
                    << "\t\tif(result == NULL) {\n"
                    << "\t\t\tcerr << \"Error in memory allocation\\n\";\n"
                    << "\t\t\texit(EXIT_FAILURE);\n"
                    << "\t\t}\n"
                    << "\t\t*result = Read_" << method_name << "(objectKey);\n"
                    << "\t\tpointers.push_back(result);\n"
                    << "\t\treturn result;\n\t}\n"
                    << "\t//{readObject}";
    }

    // This is necessary, as the AST info will carry the "class" identificator
    // up to this poiunt
    definitive_read_method = cleanClassIdentifier(read_method.str());

    replaceAll(fileContent, "//{readObject}", definitive_read_method);

    ofstream outputFile(fixture_path);
    outputFile << fileContent;

    fixture_file.close();
    outputFile.close();
}

/*vector<string> BoostGenerator::fillDefaultTypes(string path)
{
    ifstream types_file(path);
    vector<string> result;

    if(types_file.is_open())
    {
        string line;
        while(getline(types_file, line))
        {
            result.push_back(line);
        }
        types_file.close();
    }

    return result;

}*/

/*void BoostGenerator::checkTypes(string type)
{
    replaceAll(type, "_&", "");
    replaceAll(type, "const_", "");

    if(find(defaultTypes.begin(), defaultTypes.end(), type) ==
defaultTypes.end())
    {
        if(type.find("struct_") == string::npos && type.find("char") ==
string::npos)
        {
            addNewTypeToFixture(type, fixture_path);
            defaultTypes.push_back(type);
        }
    }
}*/

// NOTE: LEE CORRECTAMENTE
// TODO: revisar eficiencia: abrir fichero + recorrerlo con O(n) + cerrarlo
bool BoostGenerator::checkIfSupported(const pair<string, string> &type,
                                      const string &supportedPath) {
    ifstream supportedFile("Generated/UT/" + supportedPath +
                           "/SupportedTypes.txt");

    string typeClean = type.first;
    replaceAll(typeClean, " ", "_");

    string line;
    bool found = false;
    while (getline(supportedFile, line) && !found) {
        if (line == typeClean)
            found = true;
    }

    cout << "checking " << typeClean << " in "
         << ("Generated/UT/" + supportedPath + "/SupportedTypes.txt") << ": "
         << (found ? "detected" : "not detected") << "\n";

    return found;
}

void BoostGenerator::addTypeToSupported(const pair<string, string> &type,
                                        const string &supportedPath) {
    const string fullSupportedPath =
        "Generated/UT/" + supportedPath + "/SupportedTypes.txt";

    ofstream supportedFile(fullSupportedPath, ios_base::app);

    if (!supportedFile.is_open()) {
        cout << "Supported types file " << supportedPath
             << " couldn't be open. Type " << type.second
             << " will not be supported\n";
        return;
    }

    string typeClean = type.first;
    replaceAll(typeClean, " ", "_");
    supportedFile << typeClean << endl;
    cout << "Added " << typeClean << " to supported in "
         << ("Generated/UT/" + supportedPath + "/SupportedTypes.txt") << "\n";
}

void BoostGenerator::checkTypes(const std::pair<string, string> &type,
                                string support_path) {
    string original_type, formatted_type;
    tie(original_type, formatted_type) = type;

    replaceAll(formatted_type, "_&", "");
    replaceAll(formatted_type, "const_", "");

    ifstream support_file(support_path);

    string line;
    bool found = false;

    // FIXME: ¿no comprueba para estos tipos?
    if (formatted_type.find("struct_") == string::npos &&
        formatted_type.find("char_*") == string::npos &&
        formatted_type.find("list") == string::npos &&
        formatted_type.find("vector") == string::npos &&
        formatted_type.find("map") == string::npos) {
        while (std::getline(support_file, line)) {
            // if (line.find(formatted_type) != string::npos) {
            if (line == formatted_type) {
                // cout << "line:" << line << " - type:" << type << endl;
                found = true;
            }
        }

        if (!found) {
            ofstream output(support_path, ios_base::app);
            output << formatted_type << endl;

            addNewTypeToFixture(type, fixture_path);
        }
    }
}

void BoostGenerator::checkTypes(string type, string support_path) {
    replaceAll(type, "_&", "");
    replaceAll(type, "const_", "");

    ifstream support_file(support_path);

    string line;
    bool found = false;

    if (type.find("struct_") == string::npos &&
        type.find("char_*") == string::npos &&
        type.find("list") == string::npos &&
        type.find("vector") == string::npos &&
        type.find("map") == string::npos) {
        while (std::getline(support_file, line)) {
            // if (line.find(type) != string::npos) {
            if (line == type) {
                // cout << "line:" << line << " - type:" << type << endl;
                found = true;
            }
        }

        if (!found) {
            ofstream output(support_path, ios_base::app);
            output << type << endl;

            addNewTypeToFixture(type, fixture_path);
        }
    }
}