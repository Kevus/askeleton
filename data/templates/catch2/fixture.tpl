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
#include <catch2/catch.hpp>

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

struct Fixture {
	Fixture() {
		stringstream file;
		file << Catch::Session().argc();
		getConfigParameters("{target}.cfg");
	}

	bool getConfigParameters(string cfgPath) {
		// Aquí se mantiene el código original del Fixture
		// para manejar la configuración
		// ...
	}

	// Aquí van los métodos de lectura del Fixture
	// como en el archivo original (Read_char, Read_int, etc.)
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
		CAPTURE(ss.str());
	}

	// Miembros adicionales
	int argc;
	char **argv;
	{className} {classNameTest}
	map<string, map<string, string>> configContent;
	vector<void *> pointers;
};