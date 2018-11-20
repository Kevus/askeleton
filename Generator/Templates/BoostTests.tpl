////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
//// AST-UT PROTOTYPE                                                       ////
//// UNIVERSIDAD DE CADIZ - NAVANTIA SISTEMAS                               ////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////
//File generated automatically by AST-UT                                  
//Template originally created for LATEGEN
//File to test: {fileToTest}
//DESCRIPTION: This file sets tests cases for {className} class.
//DATE: {dateOfGeneration}
////////////////////////////////////////////////////////////////////////////////

#include "{className}_Fixture.hpp"

BOOST_FIXTURE_TEST_CASE({className}_ReadParams, Fixture)
{
	Date("Start");

	{pointerInitToken}
	{assert}
	{pointerDestroyToken}

	Date("End");
}