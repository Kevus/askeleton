////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
//// AST-UT PROTOTYPE                                                       ////
//// UNIVERSIDAD DE CADIZ - NAVANTIA SISTEMAS                               ////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
//File generated automatically by AST-UT                                  
//Template originally created for LATEGEN
//File to test: /home/kevus/git-fixed/matcher/astutmatcher/Examples/ASTUTCave.cpp
//DESCRIPTION: This file sets tests cases for FirstClass.
//DATE: 21-11-2018 08:57:53
////////////////////////////////////////////////////////////////////////////////

#include "/home/kevus/git-fixed/matcher/astutmatcher/Examples/ASTUTCave.cpp"
#define BOOST_TEST_MODULE FirstClass
#include <boost/test/included/unit_test.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iomanip>
#include <ctime>

#include <map>

#include <experimental/filesystem>

//Boost libraries
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>

//Aditional includes (only appears if needed)


//Aditional namespaces (only appears if needed)

using namespace std;
using namespace boost;

struct Fixture {
	Fixture():argc(boost::unit_test::framework::master_test_suite().argc),
	argv(boost::unit_test::framework::master_test_suite().argv) {
		
		stringstream file;
		file << argv[1];
		getConfigParameters("FirstClass.cfg");
	}

	~Fixture() {}

	bool getConfigParameters(string cfgPath)
	{
		ifstream configFile (cfgPath);
		string line;

		string ext_key;
		map<string, string> paramValues;

		bool result = false;

		if(configFile.is_open())
		{
			//We will read line by line this file, and storing the information we need
			while(getline(configFile, line))
			{
				//All spaces will be removed from the line
				line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());

				//If it is a comment, it will be ignored
				if (line[0] == '#' || line == "{" || line.empty())
				{
					continue;	
				}

				//If it has two points at the end of the line, it means that we have a key
				if (line[line.size() - 1] == ':')
				{
					//Deleting the two points, then continue
					ext_key = line.substr(0, line.size() - 1);
					continue;
				}

				//If we reached the '};', it means that we have the whole element
				if (line == "};")
				{
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

				//Value will have the form [ value;#type ]
				paramValues.insert( pair<string,string>(key, value) );

			}

			//As we have finished reading the file, it will be closed
			configFile.close();

			//File readen successfully
			result = true;

		}

		return result;
	}

	//Read the value from the CFG file. It will always return string
	string readObject(string objectKey)
	{
		string returnValue;

		//Divide the object key by point
		auto delimiter = objectKey.find(".");
		auto key = objectKey.substr(0, delimiter);
		auto value = objectKey.substr(delimiter + 1);

		//And now we get the iterators
		map<string, map<string, string> >::iterator outerIt = configContent.find(key);
		map<string, string>::iterator innerIt; //We will wait to initialize this

		if (outerIt != configContent.end())
		{
			//Now we check if we have the attribute
			innerIt = outerIt->second.find(value);

			//Make the same checks
			if(innerIt != outerIt->second.end())
			{
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

	//Date
	void Date(string value)
	{
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

	
	

	int argc:
	char **argv;

	FirstClass FirstClass_test;

	map<string, map<string, string> > configContent;

};