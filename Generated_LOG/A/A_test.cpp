#include "A_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(A_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(A_test.B(),Read_int("B.return_int"));
	BOOST_CHECK_EQUAL(A_test.C(Read_int("C.par")),Read_int("C.return_int"));
	BOOST_CHECK_EQUAL(A_test.constFunction(),Read_const int("constFunction.return_const int"));
//{assert}
	

	Date("End");
}