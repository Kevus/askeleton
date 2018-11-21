#include "FirstClass_Fixture.hpp"

BOOST_FIXTURE_TEST_CASE(FirstClass_ReadParams, Fixture)
{
	Date("Start");

	
		BOOST_CHECK_EQUAL(FirstClass.rareMethod(Read_unsigned_int("rareMethod.ui_param"),Read_float("rareMethod.a_param"),Read_double("rareMethod.d_param")),Read_int("rareMethod.return_int"));
		BOOST_CHECK_EQUAL(FirstClass.whoReturnsAChar(),Read_char("whoReturnsAChar.return_char"));
		BOOST_CHECK_EQUAL(FirstClass.whoReturnsAChar2(),Read_char("whoReturnsAChar2.return_char"));
//{assert}
	

	Date("End");
}