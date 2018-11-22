#include "ASTUTCave_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(ASTUTCave_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(outsideMethod(),Read_int("outsideMethod.return_int"));
	BOOST_CHECK_EQUAL(outsideMethod2(),Read_int("outsideMethod2.return_int"));
//{assert}
	

	Date("End");
}