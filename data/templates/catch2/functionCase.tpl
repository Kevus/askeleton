TEST_CASE_METHOD(Fixture, "{function} from {target} #{number}", "[{target}]") {
    Date("Start");
{initializations}
    {returnType} expected = {returnReadMethod};

    REQUIRE({invocation}({parameters}) == expected);
{pointers}
    Date("End");
}