#include "FirstClass_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(FirstClass_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(FirstClass_test.rareMethod(Read_unsigned_int("rareMethod.ui_param"),Read_float("rareMethod.a_param"),Read_double("rareMethod.d_param")),Read_int("rareMethod.return_int"));
	BOOST_CHECK_EQUAL(FirstClass_test.whoReturnsAChar(),Read_char("whoReturnsAChar.return_char"));
	BOOST_CHECK_EQUAL(FirstClass_test.iObtainAString(Read_string("iObtainAString.s_param")),Read_char("iObtainAString.return_char"));
//{assert}
	

	Date("End");
}