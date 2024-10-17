TEST_F(Fixture, {target}_{function}_{number}) {
    Date("Start");

    {initializations}

    EXPECT_TRUE(
        {class}({parameters})
    );

    {pointers}

    Date("End");
}
