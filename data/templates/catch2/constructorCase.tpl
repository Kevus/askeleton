TEST_CASE("{target}_{function}_{number}", "[Fixture]") {
    Date("Start");

    {initializations}

    REQUIRE(
        {class}({parameters})
    );

    {pointers}

    Date("End");
}
