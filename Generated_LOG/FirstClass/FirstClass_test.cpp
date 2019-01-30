#include "FirstClass_fixture.hpp"

BOOST_FIXTURE_TEST_CASE(FirstClass_ReadParams, Fixture)
{
	Date("Start");

	
	BOOST_CHECK(new FirstClass());
	BOOST_CHECK(new FirstClass(Read_int("FirstClass_2.i_param"),Read_string("FirstClass_2.s_param")));
	BOOST_CHECK_EQUAL(FirstClass_test.rareMethod(Read_unsigned_int("rareMethod.ui_param"),Read_float("rareMethod.a_param"),Read_double("rareMethod.d_param")),Read_int("rareMethod.return_int"));
	BOOST_CHECK_EQUAL(FirstClass_test.whoReturnsAChar(),Read_char("whoReturnsAChar.return_char"));
	BOOST_CHECK((FirstClass_test.allLists(Read_list<int>("allLists.l_param")) == Read_list<int>("allLists.return_list<int>")));
	BOOST_CHECK((FirstClass_test.allVector(Read_vector<int>("allVector.v_param")) == Read_vector<int>("allVector.return_vector<int>")));
	BOOST_CHECK((FirstClass_test.allMap(Read_map<int, int>("allMap.m_param")) == Read_map<int, int>("allMap.return_map<int, int>")));
	BOOST_CHECK_EQUAL(FirstClass_test.iObtainAString(Read_string("iObtainAString.s_param")),Read_char("iObtainAString.return_char"));
//{assert}
	

	Date("End");
}