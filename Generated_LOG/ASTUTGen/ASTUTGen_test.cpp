#include "ASTUTGen_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(ASTUTGen_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(deleteAllBeforeChar(Read_string("deleteAllBeforeChar.sToReplace"),Read_char("deleteAllBeforeChar.cToFind")),Read_string("deleteAllBeforeChar.return_string"));
//{assert}
	

	Date("End");
}