////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
//// ASKELETON TEST GENERATOR                                               ////
//// UNIVERSIDAD DE CADIZ                           						////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
// File generated automatically by ASKELETON
// Template originally created for ASKELETON
// File to test: {filePath}
// DESCRIPTION: This file sets tests cases for {target}.
// DATE: {dateOfGeneration}
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "{headerPath}"
#define BOOST_TEST_MODULE {target}_TEST

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/included/unit_test.hpp>
#include <cctype>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

// Aditional includes (only appears if needed)
{includes}

// Aditional namespaces (only appears if needed)
{namespaces}
using namespace std;
using namespace boost;

struct Fixture {
	Fixture():argc(boost::unit_test::framework::master_test_suite().argc),
	argv(boost::unit_test::framework::master_test_suite().argv) {

		stringstream file;
		file << argv[1];
		getConfigParameters("{target}.cfg");
	}

	bool getConfigParameters(string cfgPath) {
		ifstream configFile (cfgPath);
		string line;

		string ext_key;
		map<string, string> paramValues;

		bool result = false;

		if(configFile.is_open()) {
			//We will read line by line this file, and storing the information we need
			while(getline(configFile, line)) {
				//All spaces will be removed from the line
				//line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());

				//If it is a comment, it will be ignored
				if (line[0] == '#' || line == "{" || line.empty()) {
					continue;
				}

				//If it has two points at the end of the line, it means that we have a key
				if (line[line.size() - 1] == ':') {
					//Deleting the two points, then continue
					ext_key = line.substr(0, line.size() - 1);
					continue;
				}

				//If we reached the '};', it means that we have the whole element
				if (line == "};") {
					//Push the elements to the result and clear the auxiliars
					configContent.insert( pair<string,map<string,string>>(ext_key, paramValues) );

					//Clean the variables
					ext_key = "";
					paramValues.clear();

					continue;
				}

				/**
				** If we reach this point, then we can store the values
				** in the intermediate state
				**/
				auto delimiter = line.find("=");
				auto key = line.substr(0, delimiter);
				auto value = line.substr(delimiter + 1);
				boost::replace_all(key, "\t", "");

				//Value will have the form [ value;#type ]
				paramValues.insert({key, value});
			}

			//As we have finished reading the file, it will be closed
			configFile.close();

			//File readen successfully
			result = true;

		}

		return result;
	}

	//Read the value from the CFG file. It will always return string
	string readObject(string objectKey) {
		string returnValue;

		//Divide the object key by point
		auto delimiter = objectKey.find(".");
		auto key = objectKey.substr(0, delimiter);
		auto value = objectKey.substr(delimiter + 1);

		//And now we get the iterators
		map<string, map<string, string> >::iterator outerIt = configContent.find(key);
		map<string, string>::iterator innerIt; //We will wait to initialize this

		if (outerIt != configContent.end()) {
			//Now we check if we have the attribute
			innerIt = outerIt->second.find(value);

			//Make the same checks
			if(innerIt != outerIt->second.end()) {
				returnValue = innerIt->second;
				returnValue = returnValue.substr(0, returnValue.find(";"));
			}
			else {
				//No value found
				returnValue = "0";
			}

		}
		else {
			//No value found
			returnValue = "0";
		}

		return returnValue;

	}

	//==========================================================
	// EQUIVALENT TYPES:
	// char
	// signed char
	// unsigned char
	//==========================================================
	char Read_char(string objectKey) {
		return readObject(objectKey)[0];
	}

	signed char Read_signed_char(string objectKey) {
		return Read_char(objectKey);
	}

	unsigned char Read_unsigned_char(string objectKey) {
		return Read_char(objectKey);
	}

	//==========================================================
	// EQUIVALENT TYPES:
	// short
	// short int
	// signed short
	// signed short int
	//==========================================================
	short Read_short(string objectKey) {
		return stoi(readObject(objectKey));
	}

	short int Read_short_int(string objectKey) {
		return Read_short(objectKey);
	}

	signed short Read_signed_short(string objectKey) {
		return Read_short(objectKey);
	}

	signed short int Read_signed_short_int(string objectKey) {
		return Read_short(objectKey);
	}

	//==========================================================
	// EQUIVALENT TYPES:
	// unsigned short
	// unsigned short int
	//==========================================================
	unsigned short Read_unsigned_short(string objectKey) {
		return stoul(readObject(objectKey));
	}

	unsigned short int Read_unsigned_short_int(string objectKey) {
		return Read_unsigned_short(objectKey);
	}

	//==========================================================
	// EQUIVALENT TYPES:
	// int
	// signed
	// signed int
	//==========================================================
	int Read_int(string objectKey) {
		return stoi(readObject(objectKey));
	}

	signed Read_signed(string objectKey) {
		return Read_int(objectKey);
	}

	signed int Read_signed_int(string objectKey) {
		return Read_int(objectKey);
	}

	//==========================================================
	// EQUIVALENT TYPES
	// unsigned
	// unsigned int
	// size_t
	//==========================================================
	unsigned Read_unsigned_int(string objectKey) {
		return stoul(readObject(objectKey));
	}

	unsigned int Read_unsigned(string objectKey) {
		return Read_unsigned_int(objectKey);
	}

	size_t Read_size_t(string objectKey) {
		return Read_unsigned_int(objectKey);
	}

	//==========================================================
	// EQUIVALENT TYPES:
	// long
	// long int
	// signed long
	// signed long int
	//==========================================================
	long Read_long(string objectKey) {
		return stol(readObject(objectKey));
	}

	long int Read_long_int(string objectKey) {
		return Read_long(objectKey);
	}

	signed long Read_signed_long(string objectKey) {
		return Read_long(objectKey);
	}

	signed long int Read_signed_long_int(string objectKey) {
		return Read_long(objectKey);
	}

	//========================================================== 
	// EQUIVALENT TYPES:
	// unsigned long
	// unsigned long int
	//==========================================================
	unsigned long Read_unsigned_long(string objectKey) {
		return stoul(readObject(objectKey));
	}

	unsigned long int Read_unsigned_long_int(string objectKey) {
		return Read_unsigned_long(objectKey);
	}

	//==========================================================
	// EQUIVALENT TYPES:
	// long long
	// long long int
	// signed long long
	// signed long long int
	//==========================================================
	long long Read_long_long(string objectKey) {
		return stoll(readObject(objectKey));
	}

	long long int Read_long_long_int(string objectKey) {
		return Read_long_long(objectKey);
	}

	signed long long Read_signed_long_long(string objectKey) {
		return Read_long_long(objectKey);
	}

	signed long long int Read_signed_long_long_int(string objectKey) {
		return Read_long_long(objectKey);
	}

	//==========================================================
	// EQUIVALENT TYPES:
	// unsigned long long
	// unsigned long long int
	//==========================================================
	unsigned long long Read_unsigned_long_long(string objectKey) {
		return stoull(readObject(objectKey));
	}

	unsigned long long int Read_unsigned_long_long_int(string objectKey) {
		return Read_unsigned_long_long(objectKey);
	}

	//==========================================================
	// EQUIVALENT TYPES:
	// double
	// long double
	//==========================================================
	double Read_double(string objectKey) {
		return stod(readObject(objectKey));
	}

	long double Read_long_double(string objectKey) {
		return Read_double(objectKey);
	}

	//==========================================================
	// EQUIVALENT TYPES:
	// float
	//==========================================================
	float Read_float(string objectKey) {
		return stof(readObject(objectKey));
	}

	//==========================================================
	// MIXED TYPES
	//==========================================================

	bool Read_bool(string objectKey) {
		return (readObject(objectKey) == "true");
	}

	bool Read__Bool(string objectKey) {
		return Read_bool(objectKey);
	}

	string Read_string(string objectKey) {
		return readObject(objectKey);
	}

	char* Read_char_s(string objectKey) {
		string s_value = readObject(objectKey);
		
		char *result = strdup(s_value.empty() ? "" : s_value.c_str());
		pointers.push_back(result);

		return result;
	}

	//==========================================================
	// SPECIAL TYPES
	//==========================================================
	template <typename T>
	T convertFromString(const std::string& str) {
		std::istringstream iss(str);
		T result;
		
		if (!(iss >> result)) {
			throw std::invalid_argument("No se puede convertir la cadena " + str);
		}

		return result;
	}
	
	template <typename T>
	list<T> Read_list(string objectKey) {
		list<T> result_list;

		string unparsed_list = readObject(objectKey);
		//{1, 2, 3, 4}
		boost::replace_all(unparsed_list, "{", "");
		boost::replace_all(unparsed_list, "}", "");
		boost::replace_all(unparsed_list, ";", "");

		auto delimiter = unparsed_list.find(",");

		while(delimiter != string::npos) {
			auto key = unparsed_list.substr(0, delimiter);
			unparsed_list = unparsed_list.substr(delimiter + 1);

			T insert_value = boost::lexical_cast<T>(key);
			result_list.push_back(insert_value);

			delimiter = unparsed_list.find(",");

			if(delimiter == string::npos) {
				insert_value = boost::lexical_cast<T>(unparsed_list);
				result_list.push_back(insert_value);
			}

		}

		return result_list;

	}

	template <typename T>
	vector<T> Read_vector(string objectKey) {
		list<T> aux_list = Read_list<T>(objectKey);

		vector<T> result_vector(
								 aux_list.begin(),
								 aux_list.end()
							   );

		return result_vector;
	}

	template <typename T, typename Y>
	map<T,Y> Read_map(string objectKey) {
		map<T,Y> result_map;
		T aux_key;
		Y aux_value;

		string unparsed_map = readObject(objectKey);

		//{(1,2),(3,4),(5,6)};
		boost::replace_all(unparsed_map, "{", "");
		boost::replace_all(unparsed_map, "}", "");
		boost::replace_all(unparsed_map, ";", "");
		//(1,2),(3,4),(5,6)

		auto general_delimiter = unparsed_map.find(")");

		while(general_delimiter != string::npos) {
			auto key = unparsed_map.substr(1, general_delimiter); //1,2
			boost::replace_all(key, "(", "");
			boost::replace_all(key, ")", "");

			unparsed_map = unparsed_map.substr(general_delimiter + 1);//,(3,4),(5,6)

			auto inside_delimiter = key.find(",");

			if(inside_delimiter != string::npos) {
				auto inside_key = key.substr(0, inside_delimiter);
				auto inside_value = key.substr(inside_delimiter + 1);

				//cout << inside_key << " --- " << inside_value << "\n";

				aux_key = boost::lexical_cast<T>(inside_key);
				aux_value = boost::lexical_cast<Y>(inside_value);

				result_map.insert(pair<T,Y>(aux_key, aux_value));
			}

			general_delimiter = unparsed_map.find(")");

		}

		return result_map;
	}

	//Date
	void Date(string value) {
		time_t t;
		struct tm *tmt;
		t=time(NULL);
		tmt=gmtime(&t);
		stringstream ss;
		ss <<value<< ": "<< tmt->tm_year+1900 << "-";
		ss << std::setw(2) << std::setfill('0') <<tmt->tm_mon+1 << "-";
		ss << std::setw(2) << std::setfill('0') <<tmt->tm_mday <<"T";
		ss << std::setw(2) << std::setfill('0') <<tmt->tm_hour << ":";
		ss << std::setw(2) << std::setfill('0') <<tmt->tm_min << ":";
		ss << std::setw(2) << std::setfill('0') <<tmt->tm_sec<<"+02:00";
		BOOST_TEST_MESSAGE(ss.str());
	}
//{readObject}
{newMethods}
	int argc;
	char **argv;
{classMemberDecl}
	map<string, map<string, string> > configContent;
	vector<void *> pointers;
};
//{overloadOperator}
