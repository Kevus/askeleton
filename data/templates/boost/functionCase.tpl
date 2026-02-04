BOOST_FIXTURE_TEST_CASE({target}_{function}_{number}, Fixture) {
    Date("Start");
{initializations}
    {returnType} expected = {returnReadMethod};
    
    {assert}
{pointers}
    Date("End");
}