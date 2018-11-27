#include "ConfigGenerator_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(ConfigGenerator_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(fileExists(Read_string_&("fileExists.filename")),Read__Bool("fileExists.return__Bool"));
	BOOST_CHECK_EQUAL(ConfigGenerator_test.getGenerated(),Read_int("getGenerated.return_int"));
//{assert}
	

	Date("End");
}