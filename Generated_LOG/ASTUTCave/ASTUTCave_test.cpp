#include "ASTUTCave_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(ASTUTCave_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(conditionalMethod(),Read_int("conditionalMethod.return_int"));
	BOOST_CHECK_EQUAL(outsideMethod2(),Read_int("outsideMethod2.return_int"));
	BOOST_CHECK_EQUAL(ctypetest(),Read_struct_customType("ctypetest.return_struct_customType"));
//{assert}
	

	Date("End");
}