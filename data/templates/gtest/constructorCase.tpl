TEST_F(Fixture, {target}_{function}_{number}) {
    Date("Start");

{initializations}

    auto construct_generated_instance = [&]() {
        {class} generated_instance{{parameters}};
        (void)generated_instance;
    };
    EXPECT_NO_THROW(construct_generated_instance());

    {pointers}

    Date("End");
}
