TEST_CASE("{target}_{function}_{number}", "[Fixture]") {
    Date("Start");

    {initializations}
    REQUIRE{assertEnding}(
        {invocation}({parameters}) == {return}
    );

    {pointers}

    Date("End");
}
