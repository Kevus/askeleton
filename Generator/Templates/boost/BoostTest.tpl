#include "{className}_fixture.hpp"

BOOST_FIXTURE_TEST_CASE({className}_ReadParams, Fixture)
{
	Date("Start");

	{pointerInitToken}
//{assert}
	{pointerDestroyToken}

	Date("End");
}