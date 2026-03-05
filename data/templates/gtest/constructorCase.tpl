TEST_F(Fixture, {target}_{function}_{number}) {
    Date("Start");

{initializations}

    EXPECT_NO_THROW({
        {class} generated_instance({parameters});
        (void)generated_instance;
    });

    {pointers}

    Date("End");
}
