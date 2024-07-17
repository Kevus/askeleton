BOOST_FIXTURE_TEST_CASE({target}_{function}_{number}, Fixture) {
    Date("Start");

{initializations}

    BOOST_CHECK(
        {class}({parameters})
    );

    {pointers}

    Date("End");
}