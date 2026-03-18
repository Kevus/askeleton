BOOST_FIXTURE_TEST_CASE({target}_{function}_{number}, Fixture) {
    Date("Start");
{initializations}
    BOOST_CHECK_NO_THROW({
        {class} generated_instance{{parameters}};
        (void)generated_instance;
    });
{pointers}
    Date("End");
}
