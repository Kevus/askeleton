# Examples

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../doc/logo/ASkeleTon-symbol.svg">
    <img src="../doc/logo/ASkeleTon-symbol.svg" alt="ASkeleTon symbol" width="96">
  </picture>
</p>

This folder contains small, self-contained inputs to demonstrate ASkeleTon.
External projects used for evaluation (e.g., third-party libraries) are not
distributed as part of ASkeleTon. If they exist in your local tree, they are
for internal testing only.

Quick run:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --bootstrap-compdb -p examples examples/sut.cpp
```

Minimal end-to-end reproducibility workflow
(generate -> build -> execute -> edit `.cfg` -> rerun):
```bash
./scripts/check_main_workflow.sh
```

For the generated `.cfg` syntax and expected-value overrides, see
[`../doc/CfgFormat.md`](../doc/CfgFormat.md).

Showcase run (broader feature surface + intentional skips in report):
```bash
ASKELETON_HOME=$(pwd) ./askeleton --bootstrap-compdb -p examples --report=/tmp/sut_showcase_report.json examples/sut_showcase.cpp
```

## Compact Showcase Workflow

`examples/sut_showcase.cpp` is the recommended compact example when you want to
inspect more than a single primitive-argument function. It includes:

- free functions with branch conditions used for rule-derived data;
- `std::optional<T>`, `std::pair<T, U>`, `std::tuple<Ts...>`, vectors, and maps;
- classes with constructors, instance methods, const methods, and static methods;
- deliberate generation boundaries that appear as skip reasons in the report.

Generate GoogleTest scaffolding with deterministic data and a report:

```bash
rm -rf /tmp/askeleton_showcase_example
ASKELETON_HOME=$(pwd) ./askeleton \
  --bootstrap-compdb \
  -p examples \
  --framework=gtest \
  --profile=random \
  --coverage-mode=balanced \
  --oracle-mode=explicit \
  --seed=123 \
  --report=/tmp/askeleton_showcase_example/report.json \
  --out-dir=/tmp/askeleton_showcase_example/generated \
  examples/sut_showcase.cpp
```

Build and run the generated free-function tests:

```bash
make -C /tmp/askeleton_showcase_example/generated/sut_showcase
/tmp/askeleton_showcase_example/generated/sut_showcase/sut_showcase_test
```

Build and run one generated class target:

```bash
make -C /tmp/askeleton_showcase_example/generated/Counter
/tmp/askeleton_showcase_example/generated/Counter/Counter_test
```

Useful generated artifacts:

- Free-function `.cfg` data:
  `/tmp/askeleton_showcase_example/generated/sut_showcase/sut_showcase.cfg`
- Class `.cfg` data:
  `/tmp/askeleton_showcase_example/generated/Counter/Counter.cfg`
- Report JSON:
  `/tmp/askeleton_showcase_example/report.json`

To add an explicit expected value, edit `sut_showcase.cfg` and add
`expected=47;#int` inside `add_i64_1`, because the generated seeded inputs are
`a=72` and `b=-25`:

```text
add_i64_1:
{
    a=72;#long
    b=-25;#long
    expected=47;#int

};
```

Then rerun:

```bash
/tmp/askeleton_showcase_example/generated/sut_showcase/sut_showcase_test \
  --gtest_filter=Fixture.sut_showcase_add_i64_1
```

The report records both generated and skipped entities. Skips in this example
are intentional generation boundaries, such as deleted constructors, missing
instance strategies, abstract records, unsupported indirection, and unsupported
array/reference shapes.

Related documentation:

- Generated `.cfg` format: [`../doc/CfgFormat.md`](../doc/CfgFormat.md)
- Report JSON schema: [`../doc/ReportSchema.md`](../doc/ReportSchema.md)
- Skip reason guidance: [`../doc/SkipReasons.md`](../doc/SkipReasons.md)

Rule-based data:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --bootstrap-compdb --rule-data --rule-max-cases=3 -p examples examples/sut.cpp
```

Profiles:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --bootstrap-compdb --profile=boundary -p examples examples/sut.cpp
```

Output is written to:
examples/tests/generated/sut/

Interactive demo runner:
```bash
./scripts/demo_askeleton.sh
```
