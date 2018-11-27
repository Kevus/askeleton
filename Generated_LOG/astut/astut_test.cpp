#include "astut_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(astut_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(CommonHelp(Read_int("CommonHelp.")),Read_int("CommonHelp.return_int"));
	BOOST_CHECK_EQUAL(main(Read_int("main.argc"),Read_const_char_**("main.argv")),Read_int("main.return_int"));
//{assert}
	

	Date("End");
}