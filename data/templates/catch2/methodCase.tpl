TEST_CASE_METHOD(Fixture, "Testing method {function} from class {target} #{number}", "[{target}]") {
    Date("Start");

    {class} {object};
{initializations}
    REQUIRE(
        {object}.{invocation}({parameters}) == {return}
    );
{pointers}
    Date("End");
}