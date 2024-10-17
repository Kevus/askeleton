TEST_F(Fixture, {target}_{function}_{number}) {
    Date("Start");

    {class} {object};
    {initializations}

    EXPECT_EQ(
        {object}.{invocation}({parameters}),
        {return}
    );

    {pointers}

    Date("End");
}
