#include "ASKGen.hpp"

void ASKGen::run(const MatchFinder::MatchResult &Result) {
    apply_FD1(Result);
    apply_MD1(Result);
    apply_CT1(Result); // Necessary for structs and classes
    apply_CC1(Result);

    // Kevin: dejamos estos fuera, nos interesa ahora solo las funciones, los
    // datos vendrán por KLEE apply_DG1(Result); apply_DG2(Result);
}

// Method outside classes
void ASKGen::apply_FD1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const FunctionDecl *UT =
            Result.Nodes.getNodeAs<clang::FunctionDecl>("FD1")) {

        FullSourceLoc FullLocation;

        FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            // In this case, we do not want class functions
            if (!isa<CXXMethodDecl>(UT)) {

                // Get the file name
                string source_file = Context->getSourceManager()
                                         .getFilename(UT->getBeginLoc())
                                         .str();
                unsigned first = source_file.find_last_of('/') + 1;
                unsigned last = source_file.find_last_of('.');

                string filename = source_file.substr(first, last - first);

                // TO-DO: MODIFICAR PARA AÑADIR MAS O MENOS FRAMEWORKS
                BoostGenerator bGen(source_file, filename, false);
                generateFunctionTest(filename, UT->getName().str(),
                                     UT->parameters(), UT->getReturnType(),
                                     bGen);
                // Print auxiliary
                // ======================================================================
                llvm::outs() << "Found FunctionDecl at "
                             << FullLocation.getSpellingLineNumber() << ":"
                             << FullLocation.getSpellingColumnNumber() << " - ";

                llvm::outs() << UT->getNameInfo().getAsString() << " in file "
                             << filename << "\n";
                // Print auxiliary
                // ======================================================================
            }
        }
    }
}

void ASKGen::apply_MD1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const CXXMethodDecl *UT =
            Result.Nodes.getNodeAs<clang::CXXMethodDecl>("MD1")) {

        FullSourceLoc FullLocation;

        FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            // In this case, we do not want class constructors
            if (!isa<CXXConstructorDecl>(UT)) {
                string source_file = Context->getSourceManager()
                                         .getFilename(UT->getBeginLoc())
                                         .str();
                string parentname = UT->getParent()->getName().str();

                // TO-DO: MODIFICAR PARA AÑADIR MAS O MENOS FRAMEWORKS
                BoostGenerator bGen(source_file, parentname, true);
                generateFunctionTest(parentname, UT->getName().str(),
                                     UT->parameters(), UT->getReturnType(),
                                     bGen);

                // Print auxiliary
                // ======================================================================
                llvm::outs() << "Found CxxMethodDecl at "
                             << FullLocation.getSpellingLineNumber() << ":"
                             << FullLocation.getSpellingColumnNumber() << " - ";

                llvm::outs() << UT->getNameInfo().getAsString()
                             << " from class " << parentname << "\n";
                // Print auxiliary
                // ======================================================================
            }
        }
    }
}

void ASKGen::apply_CT1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const CXXRecordDecl *UT =
            Result.Nodes.getNodeAs<clang::CXXRecordDecl>("CT1")) {

        FullSourceLoc FullLocation;

        FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            // Get the file name
            string source_file = Context->getSourceManager()
                                     .getFilename(UT->getBeginLoc())
                                     .str();
            unsigned first = source_file.find_last_of('/') + 1;
            unsigned last = source_file.find_last_of('.');

            string filename = source_file.substr(first, last - first);

            // TO-DO: MODIFICAR PARA AÑADIR MAS O MENOS FRAMEWORKS
            BoostGenerator bGen(source_file, filename, false);

            InfoType record(QualType(UT->getTypeForDecl(), 0));

            if (!bGen.isTypeSupported(record, filename)) {
                bGen.addRecordReadToFixture(record);
                bGen.addTypeToSupported({record.original, record.formatted},
                                        filename);
            }

            // We'll read the fields here
            // vector<FieldDecl *> field_decl;

            // for (auto i : UT->fields())
            //     field_decl.push_back(i);

            // bool overloadedEq = false;
            // bool overloadedFlux = false;
            // for (auto i : UT->methods()) {
            //     // llvm::outs() << i->getNameAsString() << "\n";
            //     if (i->getNameAsString().find("operator==") != string::npos)
            //         overloadedEq = true;
            //     else if (i->getNameAsString().find("operator<<") !=
            //              string::npos)
            //         overloadedFlux = true;
            // }
            // // TODO: extraer en función
            // string record_name = UT->getQualifiedNameAsString();
            // if (record_name.find("anonymous") != string::npos)
            //     record_name =
            //         UT->getTypedefNameForAnonDecl()->getNameAsString();

            // // TODO: esto tiene que cambiarse por
            // // bGen.generateCustomTypeFixture(...)
            // generateCustomTypeFixture(filename, record_name, field_decl,
            //                           overloadedEq, overloadedFlux, bGen);

            // addReadTypeToFixture(type_name, pram_type, insertion_order)

            // Print auxiliary
            // ======================================================================
            llvm::outs() << "Found CXXRecordDecl (struct-customtype) at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            // llvm::outs() << record_name << " in file " << filename << "\n";
            llvm::outs() << record.original << " in file " << filename << "\n";
            // Print auxiliary
            // ======================================================================
        }
    }
}

void ASKGen::apply_CC1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const CXXConstructorDecl *UT =
            Result.Nodes.getNodeAs<clang::CXXConstructorDecl>("CC1")) {

        FullSourceLoc FullLocation;

        FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            string source_file = Context->getSourceManager()
                                     .getFilename(UT->getBeginLoc())
                                     .str();
            string parentname = UT->getParent()->getName().str();

            // TO-DO: MODIFICAR PARA AÑADIR MAS O MENOS FRAMEWORKS
            BoostGenerator bGen(source_file, parentname, true);

            generateConstructorTest(parentname, parentname, UT->parameters(),
                                    bGen);

            // Print auxiliary
            // ======================================================================
            llvm::outs() << "Found CXXConstructorDecl at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            llvm::outs() << UT->getNameInfo().getAsString() << " from class "
                         << parentname << "\n";
            // Print auxiliary
            // ======================================================================
        }
    }
}

void ASKGen::apply_PD1(const MatchFinder::MatchResult &Result) {
    // TO-DO: make this SHOW when a private member is called
}

void ASKGen::apply_DG1(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const BinaryOperator *UT =
            Result.Nodes.getNodeAs<clang::BinaryOperator>("DG1")) {

        const FunctionDecl *FD =
            Result.Nodes.getNodeAs<clang::FunctionDecl>("DG1b");
        FullSourceLoc FullLocation;

        FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            string LHS_string = convertExpressionToString(
                UT->getLHS(), Context->getSourceManager());
            string RHS_string = convertExpressionToString(
                UT->getRHS(), Context->getSourceManager());
            string LHS_type = UT->getLHS()->getType().getAsString();
            string RHS_type = UT->getRHS()->getType().getAsString();

            string source_file = Context->getSourceManager()
                                     .getFilename(UT->getBeginLoc())
                                     .str();
            unsigned first = source_file.find_last_of('/') + 1;
            unsigned last = source_file.find_last_of('.');
            string filename = source_file.substr(first, last - first);

            string type;

            if (!isNumeric(LHS_string) && isNumeric(RHS_string) &&
                isInParameters(LHS_string, FD->parameters(), type)) {
                generateTestData(filename, FD->getName().str(), LHS_string,
                                 type, RHS_string);
            } else if (isNumeric(LHS_string) && !isNumeric(RHS_string) &&
                       isInParameters(RHS_string, FD->parameters(), type)) {
                generateTestData(filename, FD->getName().str(), RHS_string,
                                 type, LHS_string);
            } else {
                llvm::outs() << "non-numeric condition\n";
            }

            // Print auxiliary
            // ======================================================================
            /*llvm::outs() << "Found BinaryOperator at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            llvm::outs() << " from function " << FD->getName().str() <<  "\n";*/
            // Print auxiliary
            // ======================================================================
        }
    }
}

void ASKGen::apply_DG2(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;

    if (const SwitchStmt *UT =
            Result.Nodes.getNodeAs<clang::SwitchStmt>("DG2")) {

        const FunctionDecl *FD =
            Result.Nodes.getNodeAs<clang::FunctionDecl>("DG2b");
        FullSourceLoc FullLocation;

        FullLocation = Context->getFullLoc(UT->getBeginLoc());

        if (FullLocation.isValid() &&
            !Context->getSourceManager().isInSystemHeader(FullLocation)) {

            string source_file = Context->getSourceManager()
                                     .getFilename(UT->getBeginLoc())
                                     .str();
            unsigned first = source_file.find_last_of('/') + 1;
            unsigned last = source_file.find_last_of('.');
            string filename = source_file.substr(first, last - first);

            // string test = UT->getCond()->getAsString();
            // llvm::outs() << "test: " << test << "\n";
            // string cname = UT->getCond()->getName().str();
            // string ctype = UT->getCond()->getType().getAsString();

            // llvm::outs() << /*"Cname: " << cname << */" Ctype: " << ctype <<
            // "\n";

            /*string LHS_string = convertExpressionToString(UT->getLHS(),
            Context->getSourceManager()); string RHS_string =
            convertExpressionToString(UT->getRHS(),
            Context->getSourceManager()); string LHS_type =
            UT->getLHS()->getType().getAsString(); string RHS_type =
            UT->getRHS()->getType().getAsString();

            //llvm::outs() << "LHS: " << LHS_string << " " << LHS_type << " -
            RHS: " << RHS_string << " " << RHS_type << "\n";

            string source_file =
            Context->getSourceManager().getFilename(UT->getBeginLoc()).str();
            unsigned first = source_file.find_last_of('/') + 1;
            unsigned last = source_file.find_last_of('.');
            string filename = source_file.substr(first, last-first);

            string type;
            if(!isNumeric(LHS_string) && isNumeric(RHS_string) &&
            isInParameters(LHS_string, FD->parameters(), type))
            {
                generateTestData(filename, FD->getName().str(), LHS_string,
            type, RHS_string); } else if (isNumeric(LHS_string) &&
            !isNumeric(RHS_string) && isInParameters(RHS_string,
            FD->parameters(), type))
            {
                generateTestData(filename, FD->getName().str(), RHS_string,
            type, LHS_string); } else
            {
                llvm::outs() << "non-numeric condition\n";
            }*/

            // Print auxiliary
            // ======================================================================
            llvm::outs() << "Found SwitchStmt at "
                         << FullLocation.getSpellingLineNumber() << ":"
                         << FullLocation.getSpellingColumnNumber() << " - ";

            llvm::outs() << " from function " << FD->getName().str() << "\n";
            // Print auxiliary
            // ======================================================================
        }
    }
}

// General method for testing functions
void ASKGen::generateFunctionTest(string sourceFile,
                                  string originalFunctionName,
                                  ArrayRef<ParmVarDecl *> originalParameters,
                                  QualType originalReturnType,
                                  BoostGenerator bGen) {
    ConfigGenerator cfg_gen(sourceFile);
    string functionName =
        function_occurrences[originalFunctionName]++ > 1
            ? originalFunctionName + "_" +
                  to_string(function_occurrences[originalFunctionName])
            : originalFunctionName;

    // Getting the parameters
    vector<InfoVariable> parameters;
    transform(originalParameters.begin(), originalParameters.end(),
              back_inserter(parameters),
              [](const ParmVarDecl *param) { return param; });

    // Getting the return type
    InfoType returnType(originalReturnType);

#ifdef FULL_DEBUG
    // ---------------------------------------
    // DISPLAY AUXILIARY INFORMATION
    // ---------------------------------------
    cout << "--------------\n";
    unsigned i = 0;
    cout << "Params list: (";
    for (auto &param : parameters) {
        cout << i++ << " - " << param.original << " " << param.name;
        if (&param != &parameters.back())
            cout << ", ";
    }

    cout << ")\nReturn type: " << returnType.original << "\n";
    // ---------------------------------------
    // END OF DISPLAY
    // ---------------------------------------
#endif /* FULL_DEBUG */

    // Checking if the parameters are supported
    for (const InfoType &param : parameters)
        bGen.generateCustomTypeFixture(sourceFile, param);

    // Checking if the return type is supported
    bGen.generateCustomTypeFixture(sourceFile, returnType);

    cfg_gen.generateTestCase(functionName, parameters, returnType);

    bGen.generateBoostAssert(sourceFile, originalFunctionName, functionName,
                             parameters, returnType);
}

// Method for constructing constructor test
void ASKGen::generateConstructorTest(string source, string constructor_name,
                                     ArrayRef<ParmVarDecl *> originalParameters,
                                     BoostGenerator bGen) {
    ConfigGenerator cfg_gen(source);

    string constructor_cfg_name = constructor_name;
    function_occurrences[constructor_name]++;

    if (function_occurrences[constructor_name] > 1)
        constructor_cfg_name +=
            "_" + to_string(function_occurrences[constructor_name]);

    // Getting the parameters
    vector<InfoVariable> parameters;
    transform(originalParameters.begin(), originalParameters.end(),
              back_inserter(parameters),
              [](const ParmVarDecl *param) { return param; });

    // Get the parameters
    // map<string, string> param_type;
    // vector<string> insert_order;

    // string tmp_type;
    // string tmp_name;

    // int noname_count = 0;
    // for (auto i : parameters) {
    //     tmp_type = i->getOriginalType().getAsString();
    //     tmp_type = cleanUnnecesaryChars(tmp_type);

    //     tmp_name = i->getQualifiedNameAsString();

    //     if (tmp_name == "") {
    //         tmp_name = tmp_type + "_" + to_string(noname_count);
    //         noname_count++;
    //     }

    //     param_type.insert(pair<string, string>(tmp_name, tmp_type));
    //     insert_order.push_back(tmp_name);
    // }

    /**
    ** We will add custom generator lates
    **/
    // cfg_gen.generateConstructorTest(constructor_cfg_name, param_type,
    //                                 insert_order);
    // bGen.generateBoostConstructorAssert(source, constructor_name,
    //                                     constructor_cfg_name, param_type,
    //                                     insert_order);
    cfg_gen.generateConstructorTest(constructor_cfg_name, parameters);
    bGen.generateBoostConstructorAssert(source, constructor_name,
                                        constructor_cfg_name, parameters);
}

void ASKGen::generateEnumTypeFixture(string source,
                                     const pair<string, string> &type,
                                     BoostGenerator &bGen) {

    if (bGen.checkIfSupported(type, source))
        return;

    bGen.addEnumReadToFixture(type);

    bGen.addTypeToSupported(type, source);
}

void ASKGen::generateCustomTypeFixture(string source, string type_name,
                                       vector<FieldDecl *> parameters,
                                       bool overloadedEq, bool overloadedFlux,
                                       BoostGenerator bGen) {

    if (bGen.checkIfSupported({type_name, ""}, source))
        return;

    // CHECK: se utiliza?
    // ConfigGenerator cfg_gen(source);

    // Get the parameters
    map<string, string> param_type;
    vector<string> insert_order;

    int noname_count = 0;
    for (auto i : parameters) {
        string tmp_type = i->getType().getAsString();
        tmp_type = cleanUnnecesaryChars(tmp_type);

        // CHECK: se utiliza?
        string tmp_name = i->getNameAsString();

        if (tmp_name == "") {
            tmp_name = tmp_type + "_" + to_string(noname_count);
            noname_count++;
        }

        param_type.insert(pair<string, string>(i->getNameAsString(), tmp_type));
        insert_order.push_back(i->getNameAsString());
    }

    bGen.addStructReadToFixture(type_name, param_type, insert_order,
                                overloadedEq, overloadedFlux);

    bGen.addTypeToSupported({type_name, ""}, source);
}

void ASKGen::generateCustomTypeFixture(
    string filename, const vector<const CXXRecordDecl *> &records,
    const vector<const EnumDecl *> &enums,
    const vector<pair<string, string>> &pointers, BoostGenerator &boostGen) {

    for (const CXXRecordDecl *record : records) {
        vector<FieldDecl *> field_decl;
        string record_name = record->getQualifiedNameAsString();
        if (record_name.find("anonymous") != string::npos)
            record_name =
                record->getTypedefNameForAnonDecl()->getNameAsString();

        for (FieldDecl *field : record->fields())
            field_decl.push_back(field);
        generateCustomTypeFixture(filename, record_name, field_decl, false,
                                  false, boostGen);
    }

    for (const EnumDecl *enumDecl : enums) {
        // string original = enumDecl->getNameAsString();
        string original = enumDecl->getQualifiedNameAsString();
        if (original.find("anonymous") != string::npos)
            original = enumDecl->getTypedefNameForAnonDecl()->getNameAsString();
        string formatted = cleanUnnecesaryChars(original);

        generateEnumTypeFixture(filename, {original, formatted}, boostGen);
    }

    for (const pair<string, string> &pointer : pointers) {
        if (!boostGen.checkIfSupported(pointer, filename)) {
            boostGen.addPointerReadToFixture(pointer);
            boostGen.addTypeToSupported(pointer, filename);
        }
    }
}

void ASKGen::generateTestData(string source, string function_name, string param,
                              string type, string value) {
    // function.value=a,b,c,d,e
    map<string, vector<string>> function_values;
    string outputPath = "Generated/UT/" + source + "/" + source + "_data.txt";
    vector<string> values;
    string fileContent;

    ifstream tmp_output(outputPath);
    for (string line; getline(tmp_output, line);) {
        // for each line...
        auto delimiter = line.find(":");

        string fvalue = line.substr(0, delimiter);
        line = line.substr(delimiter + 1);

        delimiter = line.find(",");
        while (delimiter != string::npos) {
            auto key = line.substr(0, delimiter);
            line = line.substr(delimiter + 1);

            values.push_back(key);

            delimiter = line.find(",");
        }

        values.push_back(line);
        function_values.insert(
            std::pair<string, vector<string>>(fvalue, values));
        values.clear();
    }

    if (function_values.find(function_name + "." + param) !=
        function_values.end()) {
        values = function_values.at(function_name + "." + param);
    }

    vector<string> realvalues = obtainTestData(type, value);
    for (auto it : realvalues)
        values.push_back(it);

    function_values[(function_name + "." + param)] = values;

    stringstream ss;

    for (auto it : function_values) {
        ss << it.first << ":";

        for (unsigned long i = 0; i < it.second.size(); i++) {
            ss << it.second[i];
            if (i < it.second.size() - 1)
                ss << ",";
        }

        ss << "\n";
    }

    ofstream outputFile(outputPath);
    outputFile << ss.str();
}

vector<string> ASKGen::obtainTestData(string type, string value) {
    vector<string> result;
    replaceAll(value, "\'", "");
    replaceAll(value, "\"", "");
    result.push_back(value);

    if (type.find("bool") != string::npos ||
        type.find("_Bool") != string::npos) {
        result.push_back((value == "true") ? "false" : "true");
    } else if (type.find("string") != string::npos) {
        result.push_back(value + "_another");
    } else if (type.find("char") != string::npos) {
        char res = value.c_str()[0];
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    } else if (type.find("int") != string::npos) {
        int res = stoi(value);
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    } else if (type.find("double") != string::npos) {
        double res = stod(value);
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    } else if (type.find("float") != string::npos) {
        float res = stof(value);
        result.push_back(to_string(res + 1));
        result.push_back(to_string(res - 1));
    }

    return result;
}

// string format_return_type(const QualType &type) {
//     string formatted = type.getCanonicalType().getAsString();
//     replaceAll(formatted, "*", "s");
//     return cleanUnnecesaryChars(formatted);
// }

// // TODO: revisar eficiencia
// void get_full_parameters(const ArrayRef<ParmVarDecl *> &original_parameters,
//                          map<string, pair<string, string>>
//                          &mapped_parameters, vector<string>
//                          &ordered_parameters, vector<const CXXRecordDecl *>
//                          &records, vector<const EnumDecl *> &enums,
//                          vector<pair<string, string>> &pointers) {

//     unsigned noname_count = 0;
//     for (ParmVarDecl *i : original_parameters) {
//         // QualType type = i->getType();
//         QualType originalType = i->getOriginalType();

//         // Heredia: usando el canonical type en vez de el original type para
//         NEO string tmp_type = originalType.getCanonicalType().getAsString();
//         string tmp_name = i->getQualifiedNameAsString();
//         replaceAll(tmp_type, "struct ", "");

//         if (tmp_name == "")
//             tmp_name = tmp_type + "_" + to_string(noname_count++);
//         string formatted_type = cleanUnnecesaryChars(tmp_type);

//         if (originalType->isPointerType()) {
//             replaceAll(formatted_type, "*", "s");
//             pointers.push_back({tmp_type, formatted_type});
//         } else if (const RecordType *recordType =
//                        originalType->getAs<RecordType>()) {
//             records.push_back(cast<CXXRecordDecl>(recordType->getDecl()));
//             // TODO: eliminar
//             // Por ahora, evitamos que se generen nuevos tipos con las
//             // estructuras
//             // formatted_type = "struct_" + formatted_type;
//         } else if (const Type *unqualifiedType =
//                        originalType.getUnqualifiedType().getTypePtrOrNull())
//                        {
//             if (const EnumType *enumType =
//             unqualifiedType->getAs<EnumType>())
//                 enums.push_back(enumType->getDecl());
//         }

//         mapped_parameters.insert(
//             make_pair(tmp_name, make_pair(tmp_type, formatted_type)));
//         ordered_parameters.push_back(tmp_name);
//     }
// }

// // General method for testing functions
// void ASKGen::generateFunctionTest(string source_file, string function_name,
//                                   ArrayRef<ParmVarDecl *> parameters,
//                                   QualType return_qtype, BoostGenerator bGen)
//                                   {
//     string return_type = format_return_type(return_qtype);
//     ConfigGenerator cfg_gen(source_file);
//     string function_cfg_name = function_name;

//     // if( function_occurrences.find(function_name) ==
//     // function_occurrences.end() )
//     //	function_occurences.insert( pair<string, int> (function_name, 1));
//     function_occurrences[function_name]++;

//     if (function_occurrences[function_name] > 1)
//         function_cfg_name +=
//             "_" + to_string(function_occurrences[function_name]);

//     // Order of the parameters
//     map<string, pair<string, string>> param_type;
//     vector<string> insert_order;

//     // Typed parameters
//     vector<const CXXRecordDecl *> records;
//     vector<const EnumDecl *> enums;
//     vector<pair<string, string>> pointers;

//     get_full_parameters(parameters, param_type, insert_order, records, enums,
//                         pointers);

//     // Processing return type
//     if (return_qtype->isPointerType()) {
//         string tmp_type = return_qtype.getCanonicalType().getAsString();
//         string formatted_type = cleanUnnecesaryChars(tmp_type);
//         replaceAll(formatted_type, "*", "s");
//         pointers.push_back({tmp_type, formatted_type});
//     } else if (const RecordType *recordType =
//                    return_qtype->getAs<RecordType>()) {
//         records.push_back(cast<CXXRecordDecl>(recordType->getDecl()));
//     } else if (const EnumType *enumType = return_qtype->getAs<EnumType>()) {
//         EnumDecl *enumDecl = enumType->getDecl();
//         if (enumDecl) {
//             string enumName = enumDecl->getQualifiedNameAsString();
//             enums.push_back(enumType->getDecl());
//         }
//     }
//     replaceAll(return_type, "struct_", "");

// #ifdef FULL_DEBUG
//     cout << "\n\n--------------\n";
//     unsigned i = 0;
//     cout << "Params list: (";
//     for (auto t : param_type)
//         cout << i++ << " - " << t.second.first << " " << t.first << ", ";
//     cout << ")\n";

//     i = 0;
//     cout << "Records list: (";
//     for (auto t : records) {
//         string record_name = t->getQualifiedNameAsString();
//         if (record_name.find("anonymous") != string::npos)
//             record_name = t->getTypedefNameForAnonDecl()->getNameAsString();
//         cout << i++ << " - " << record_name << ", ";
//     }
//     cout << ")\n";

//     cout << "Return type: " << return_type << "\n";

//     i = 0;
//     cout << "Enumeration list: (";
//     for (auto t : enums) {
//         string enum_name = t->getQualifiedNameAsString();
//         if (enum_name.find("anonymous") != string::npos)
//             enum_name = t->getTypedefNameForAnonDecl()->getNameAsString();
//         cout << i++ << " - " << enum_name << ", ";
//     }
//     cout << ")\n";

//     i = 0;
//     cout << "Pointers list: (";
//     for (const pair<string, string> &pointer : pointers) {
//         cout << i++ << " - " << pointer.first << ", ";
//     }
//     cout << ")\n";

// #endif /* FULL_DEBUG */

//     generateCustomTypeFixture(source_file, records, enums, pointers, bGen);

//     string return_type_string =
//     return_qtype.getCanonicalType().getAsString();
//     // cfg_gen.generateTestCase(function_cfg_name, param_type, insert_order,
//     //                          {return_type_string, return_type});

//     InfoType returnType(return_type_string, return_type);
//     vector<InfoVariable> params;
//     for (const auto &name : insert_order) {
//         const auto &[original, formatted] = param_type[name];
//         params.push_back({name, original, formatted});
//     }

//     cfg_gen.generateTestCase(function_cfg_name, params, returnType);
//     bGen.generateBoostAssert(source_file, function_name, function_cfg_name,
//                              params, returnType);
// }
