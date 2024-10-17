BOOST_FIXTURE_TEST_CASE({target}_{function}_{number}, Fixture) {
    Date("Start");

{initializations}
    BOOST_CHECK{assertEnding}(
        {invocation}({parameters}),
        {return}
    );
{pointers}
    Date("End");
}