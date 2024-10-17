TEST_CASE("{target}_{function}_{number}", "[Fixture]") {
    Date("Start");

    {class} {object};
    {initializations}

    REQUIRE{assertEnding}(
        {object}.{invocation}({parameters}) == {return}
    );

    {pointers}

    Date("End");
}