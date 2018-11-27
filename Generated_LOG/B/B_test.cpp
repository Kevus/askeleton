#include "B_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(B_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(B_test.imfromB(),Read_string("imfromB.return_string"));
//{assert}
	

	Date("End");
}