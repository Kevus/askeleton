TEST_CASE_METHOD(Fixture, "Testing constructor {function} from class {target} #{number}", "[{target}]") {
    Date("Start");

{initializations}
    REQUIRE_NOTHROW({
        {class} generated_instance{{parameters}};
        (void)generated_instance;
    });
{pointers}
    Date("End");
}
