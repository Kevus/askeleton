#include "auxiliary_functions.hpp"

bool fileExists(const string& filename)
{
  struct stat buffer;   
  return (stat (filename.c_str(), &buffer) == 0); 
}


string getCommentHeader(string filename)
{
	/**
	** Comment header, for visibility purposes
	**/
	stringstream buffer;

	// Time utilities
	auto t = time(nullptr);
	auto tm = *localtime(&t);

	// ASTUT Banner
	buffer << "#" << string(55, '/') << "\n"
		   << "#////" << string(47, ' ') << "////\n"
		   << "#//// AST-UT PROTOTYPE" << string(30, ' ') << "////\n"
		   << "#//// UNIVERSIDAD DE CADIZ - NAVANTIA SISTEMAS      ////\n"
		   << "#////" << string(47, ' ') << "////\n"
		   << "#" << string(55, '/') << "\n"
		   << "#File generated automatically by AST-UT.\n"
		   << "#File " << filename << "\n"
		   << "#Date " << put_time(&tm, "%d-%m-%Y %H:%M:%S") << "\n"
		   << "#" << string(55, '/') << "\n\n";

    //The banner should look like this:

    /**
    ** #///////////////////////////////////////////////////////
	** #////                                               ////
	** #//// AST-UT PROTOTYPE                              ////
	** #//// UNIVERSIDAD DE CADIZ - NAVANTIA SISTEMAS      ////
	** #////                                               ////
	** #///////////////////////////////////////////////////////
	** #File generated automatically by AST-UT.
	** #File <filename>.<extension>
	** #Date dd-mm-yyyy hh:MM:ss
	** #///////////////////////////////////////////////////////
	**
	**/
	// *** END OF THE BANNER ***

	return buffer.str();
}

string deleteAllBeforeChar(string sToReplace, char cToFind)
{
	if ( sToReplace.find(cToFind) != string::npos )
		sToReplace = sToReplace.substr(sToReplace.find_last_of(cToFind) + 1, sToReplace.size());

	return sToReplace;
}

string cleanUnnecesaryChars(string sToReplace)
{
	//==========================================================
	// We are formating types:
	//	std:: flag
	//  __cxx11:: flag
	//	All pointers will be treated as normal types
	// 	All spaces will be replaced with _
	//==========================================================
	boost::replace_all(sToReplace, "std::", "");
	boost::replace_all(sToReplace, "__cxx11::", "");
	boost::replace_all(sToReplace, " &", "");

	if(sToReplace.find("<") == string::npos)
		boost::replace_all(sToReplace, " ", "_");

	return sToReplace;
}