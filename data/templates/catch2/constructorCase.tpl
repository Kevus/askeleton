TEST_CASE_METHOD(Fixture, "Testing constructor {function} from class {target} #{number}", "[{target}]") {
    Date("Start");

{initializations}
    REQUIRE(
        {class}({parameters})
    );
{pointers}
    Date("End");
}
