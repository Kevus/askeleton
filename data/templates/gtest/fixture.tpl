////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
//// ASKELETON TEST GENERATOR                                               ////
//// UNIVERSIDAD DE CADIZ                           						////
//                                                                            ////
////////////////////////////////////////////////////////////////////////////////
//File generated automatically by ASKELETON
//Template originally created for LATEGEN
//File to test: {filePath}
//DESCRIPTION: This file sets tests cases for {target}.
//DATE: {dateOfGeneration}
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "{headerPath}"
#include <gtest/gtest.h>

#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iomanip>
#include <ctime>

#include <map>
#include <vector>
#include <list>

#include <experimental/filesystem>

// Aditional includes (only appears if needed)
{includes}

// Aditional namespaces (only appears if needed)
{namespaces}
using namespace std;

class Fixture : public ::testing::Test {
protected:
	Fixture() {
		getConfigParameters("{target}.cfg");
	}

	virtual ~Fixture() {
		// Cleanup any pointers or resources
		for (auto ptr : pointers) {
			free(ptr);
		}
	}

	// This method is called before each test
	void SetUp() override {
		// Initialization before each test
	}

	// This method is called after each test
	void TearDown() override {
		// Cleanup after each test
	}

	bool getConfigParameters(string cfgPath) {
		// Aquí se mantiene el código original del Fixture
		// para manejar la configuración
		// ...
	}

	// Métodos de lectura del Fixture (Read_char, Read_int, etc.)
	// ...

	// Método para mostrar fecha
	void Date(string value) {
		time_t t;
		struct tm *tmt;
		t=time(NULL);
		tmt=gmtime(&t);
		stringstream ss;
		ss << value << ": " << tmt->tm_year+1900 << "-";
		ss << std::setw(2) << std::setfill('0') << tmt->tm_mon+1 << "-";
		ss << std::setw(2) << std::setfill('0') << tmt->tm_mday <<"T";
		ss << std::setw(2) << std::setfill('0') << tmt->tm_hour << ":";
		ss << std::setw(2) << std::setfill('0') << tmt->tm_min << ":";
		ss << std::setw(2) << std::setfill('0') << tmt->tm_sec << "+02:00";
		ADD_FAILURE() << ss.str();
	}

	// Miembros adicionales
	int argc;
	char **argv;
	{className} {classNameTest}
	map<string, map<string, string>> configContent;
	vector<void *> pointers;
};
