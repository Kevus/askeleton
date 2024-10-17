BOOST_FIXTURE_TEST_CASE({target}_{function}_{number}, Fixture) {
    Date("Start");

    {class} {object};
{initializations}

    BOOST_CHECK{assertEnding}(
        {object}.{invocation}({parameters}),
        {return}
    );

    {pointers}

    Date("End");
}