TEST_CASE_METHOD(Fixture, "Function {function} from {target} #{number}", "[{target}]") {
    Date("Start");

{initializations}
    REQUIRE(
        {invocation}({parameters}) == {return}
    );
{pointers}
    Date("End");
}
