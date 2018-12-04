#include "customType_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(customType_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(customType_test.(Read_struct_customType&(".")),Read_struct_customType(".return_struct_customType"));
//{assert}
	

	Date("End");
}