TEST_CASE_METHOD(Fixture, "Testing method {function} from class {target} #{number}", "[{target}]") {
    Date("Start");
{initializations}
    REQUIRE(
        {object}.{invocation}({parameters}) == {return}
    );
{pointers}
    Date("End");
}