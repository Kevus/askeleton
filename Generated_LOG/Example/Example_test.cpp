#include "Example_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(Example_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(Example_test.referencedMethod(Read_unsigned_int("referencedMethod.param1"),Read_float("referencedMethod.param2")),Read_int("referencedMethod.return_int"));
	BOOST_CHECK_EQUAL(Example_test.notAConstructor(Read_double("notAConstructor.param3"),Read_string("notAConstructor.param4")),Read_int("notAConstructor.return_int"));
//{assert}
	

	Date("End");
}