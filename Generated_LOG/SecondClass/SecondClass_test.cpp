#include "SecondClass_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(SecondClass_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK_EQUAL(SecondClass_test.testPointer1(Read_int_*("testPointer1.a")),Read_int *("testPointer1.return_int *"));
	BOOST_CHECK_EQUAL(SecondClass_test.testPointer2(Read_int_*("testPointer2.b")),Read_int *("testPointer2.return_int *"));
	BOOST_CHECK_EQUAL(SecondClass_test.testPointer3(Read_int_*("testPointer3.c")),Read_int *("testPointer3.return_int *"));
//{assert}
	

	Date("End");
}