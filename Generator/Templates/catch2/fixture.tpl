#include "{className}_fixture.hpp"

TEST_CASE("{className} ReadParams", "[{className}]")
{
    Date("Start");

    {pointerInitToken}
    // {assert}
    {pointerDestroyToken}

    Date("End");
}