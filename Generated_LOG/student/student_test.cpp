#include "student_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(student_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(outsideMethod(),Read_int("outsideMethod.return_int"));
//{assert}
	

	Date("End");
}