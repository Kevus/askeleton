////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
//// ASKELETON TEST GENERATOR                                               ////
//// UNIVERSIDAD DE CADIZ                           						////
//                                                                          ////
////////////////////////////////////////////////////////////////////////////////
// File generated automatically by ASKELETON
// Template originally created for ASKELETON
// File to test: {filePath}
// DESCRIPTION: This file sets tests cases for {target}
// DATE: {dateOfGeneration}
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "{headerPath}"

#include <boost/lexical_cast.hpp>
#include <catch2/catch.hpp>
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
{includes}
{namespaces}
using namespace std;

struct Fixture {
	Fixture() { getConfigParameters("{target}.cfg"); }

	void replaceAll(std::string &str, const std::string &from, const std::string &to) {
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // In case 'to' contains 'from', like
									// replacing 'x' with 'yx'
		}
	}

	bool getConfigParameters(std::string cfgPath) {
		std::ifstream configFile(cfgPath);
		std::string line, ext_key;
		std::map<std::string, std::string> paramValues;

		if(configFile.is_open()) {
			while(std::getline(configFile, line)) {

				if (line[0] == '#' || line == "{" || line.empty()) 
					continue;

				if (line[line.size() - 1] == ':') {
					ext_key = line.substr(0, line.size() - 1);
					continue;
				}

				if (line == "};") {
					configContent.insert(pair<string,map<string,string>>
						(ext_key, paramValues));

					ext_key = "";
					paramValues.clear();

					continue;
				}

				auto delimiter = line.find("=");
				auto key = line.substr(0, delimiter);
				auto value = line.substr(delimiter + 1);
				replaceAll(key, "\t", "");

				paramValues.insert({key, value});
			}

			return true;
		}
		
		return false;
	}

	std::string readObject(std::string objectKey) {
		size_t delimiter = objectKey.find(".");
		std::string key = objectKey.substr(0, delimiter);
		std::string value = objectKey.substr(delimiter + 1);

		auto outerIt = configContent.find(key);
		if (outerIt != configContent.end()) {
			auto innerIt = outerIt->second.find(value);

			if(innerIt != outerIt->second.end()) {
				std::string returnValue = innerIt->second;
				return returnValue.substr(0, returnValue.find(";"));
			} else
				return "0";
		} else
			return "0";

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
	bool Read_bool(std::string objectKey) {
		return (readObject(objectKey) == "true");
	}

	bool Read__Bool(std::string objectKey) {
		return Read_bool(objectKey);
	}

	std::string Read_string(std::string objectKey) {
		return readObject(objectKey);
	}

	char* Read_char_s(std::string objectKey) {
		std::string s_value = readObject(objectKey);

		return strdup(s_value.empty() ? "" : s_value.c_str());
	}

	//==========================================================
	// SPECIAL TYPES
	//==========================================================
	template <typename T>
	T convertFromString(const std::string& str) {
		std::istringstream iss(str);
		T result;
		
		if (!(iss >> result))
			throw std::invalid_argument("Error parsing string " + str);

		return result;
	}
	
	template <typename T>
	std::list<T> Read_list(std::string objectKey) {
		std::list<T> result_list;

		string unparsed_list = readObject(objectKey);
		//{1, 2, 3, 4}
		replaceAll(unparsed_list, "{", "");
		replaceAll(unparsed_list, "}", "");
		replaceAll(unparsed_list, ";", "");

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
	std::vector<T> Read_vector(std::string objectKey) {
		std::list<T> aux_list = Read_list<T>(objectKey);

		std::vector<T> result_vector(
								 aux_list.begin(),
								 aux_list.end()
							   );

		return result_vector;
	}

	template <typename T, typename Y>
	std::map<T,Y> Read_map(std::string objectKey) {
		std::map<T,Y> result_map;
		T aux_key;
		Y aux_value;

		std::string unparsed_map = readObject(objectKey);

		//{(1,2),(3,4),(5,6)};
		replaceAll(unparsed_map, "{", "");
		replaceAll(unparsed_map, "}", "");
		replaceAll(unparsed_map, ";", "");
		//(1,2),(3,4),(5,6)

		auto general_delimiter = unparsed_map.find(")");

		while(general_delimiter != std::string::npos) {
			auto key = unparsed_map.substr(1, general_delimiter); //1,2
			replaceAll(key, "(", "");
			replaceAll(key, ")", "");

			unparsed_map = unparsed_map.substr(general_delimiter + 1);//,(3,4),(5,6)

			auto inside_delimiter = key.find(",");

			if(inside_delimiter != std::string::npos) {
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

	void Date(std::string value) {
		time_t t;
		struct tm *tmt;
		t=time(NULL);
		tmt=gmtime(&t);
		std::stringstream ss;
		ss << value << ": " << tmt->tm_year+1900 << "-";
		ss << std::setw(2) << std::setfill('0') << tmt->tm_mon+1 << "-";
		ss << std::setw(2) << std::setfill('0') << tmt->tm_mday <<"T";
		ss << std::setw(2) << std::setfill('0') << tmt->tm_hour << ":";
		ss << std::setw(2) << std::setfill('0') << tmt->tm_min << ":";
		ss << std::setw(2) << std::setfill('0') << tmt->tm_sec << "+02:00";
		CAPTURE(ss.str());
	}
//{readObject}
{newMethods}
	int argc;
	char **argv;
{className} {classNameTest}
	map<string, map<string, string>> configContent;
	vector<void *> pointers;
};
//{overloadOperator}
