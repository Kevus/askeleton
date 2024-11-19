TEST_F(Fixture, {target}_{function}_{number}) {
    Date("Start");
{initializations}
    {returnType} expected = {returnReadMethod};
    
    EXPECT_EQ(
        {invocation}({parameters}),
        expected
    );
{pointers}
    Date("End");
}