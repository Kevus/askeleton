#include "Example2_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(Example2_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(OutsideFunction(Read_int("OutsideFunction.par")),Read_int("OutsideFunction.return_int"));
//{assert}
	

	Date("End");
}