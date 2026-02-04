BOOST_FIXTURE_TEST_CASE({target}_{function}_{number}, Fixture) {
    Date("Start");
{initializations}
{returnType} expected = {returnReadMethod};
    BOOST_CHECK_EQUAL(
        {object}.{invocation}({parameters}),
        expected
    );
{pointers}
    Date("End");
}