#include "ASTUTMatchers_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(ASTUTMatchers_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(createMapMatchers(),Read_int("createMapMatchers.return_int"));
//{assert}
	

	Date("End");
}