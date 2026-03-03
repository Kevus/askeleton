**ASkeleTon**
Generate C/C++ unit test scaffolding from `compile_commands.json` with GoogleTest, Boost.Test, or Catch2.  
Outputs fixtures, tests, Makefiles, and `.cfg` data files with deterministic or rule-based values.

**Highlights**
- AST-driven discovery of functions, methods, and constructors.
- Profiles for data generation: `random`, `boundary`, `safe`, `stress`.
- Coverage policies: `strict`, `balanced`, `aggressive`.
- Rule-based values extracted from comparisons.
- Structured skip reasons in reports (`abstract_record`, `missing_instance_strategy`, etc.).
- Structured input support for `std::optional`, `std::pair`, and `std::tuple`.
- JSON report with per-target summary.
- Output inside the SUT repo by default.
- Clean, structured console output with optional JSON execution log.

**Quick Start**
```bash
ASKELETON_HOME=$(pwd) ./askeleton -p examples examples/sut.cpp
```

**Install (Ubuntu 24.04)**
```bash
sudo apt update
sudo apt install -y clang-18 llvm-18 llvm-18-dev llvm-18-tools libclang-18-dev build-essential
```
Optional frameworks:
```bash
sudo apt install -y libboost-dev libgtest-dev catch2
```
Verify:
```bash
clang++-18 --version
llvm-config-18 --version
```

**Install (Ubuntu 22.04)**
Ubuntu 22.04 ships older LLVM by default. Install LLVM/Clang 18 using the
official LLVM APT repo, then run the same package list as above:
```bash
sudo apt update
sudo apt install -y clang-18 llvm-18 llvm-18-dev llvm-18-tools libclang-18-dev build-essential
```

**Other distros**
Install LLVM/Clang 18 plus libclang development headers and a C++ toolchain.
Package names vary by distro; look for equivalents of:
`clang-18`, `llvm-18`, `llvm-18-dev`, `libclang-18-dev`, `build-essential`.

**Build**
```bash
make
```
`system_files.json` is refreshed automatically by the binary on each run. Use
`--no-system-files-refresh` to disable it.

**Usage**
```
askeleton [options] <source0> [... <sourceN>]
```
Key options:
- `-p <build-path>`: path to `compile_commands.json`.
- `--framework=<gtest|boost|catch>`: select test framework.
- `--profile=<random|boundary|safe|stress>`: data generation profile.
- `--coverage-mode=<strict|balanced|aggressive>`: generation coverage policy.
- `--oracle-mode=<mirror|explicit|property>`: expected-value strategy.
- `--seed=<N>`: deterministic data generation.
- `--rule-data`: enable rule-based values from AST.
- `--rule-max-cases=<N>`: limit rule-based test cases per function.
- `--out-dir=<path>`: output directory for generated tests.
- `--include-impl-under-include`: allow compiling `.c/.cc/.cpp` under `include/`.
- `--report=<path>`: write a JSON report of generated/skipped tests.
- `--report-json`: write a JSON report to `<out-dir>/askeleton_report.json`.
- `--log-json=<path>`: write an execution log with summary/warnings.
- `--no-system-files-refresh`: do not regenerate `data/system_files.json`.
- `--quiet`, `--verbose`, `--debug`: control console verbosity.
- `-extra-arg`, `-extra-arg-before`: pass extra compiler args to Clang tooling.

**Default Output**
By default, ASkeleTon writes output under `tests/generated` relative to the first
source file passed on the command line. Use `--out-dir` to override this.

**Console Output**
By default you get a concise progress view plus a final summary. Use:
- `--quiet` to print only errors.
- `--verbose` to include per-entity progress.
- `--debug` to include detailed parameter/return signatures.

To collect a machine-readable execution log (inputs, counts, warnings, timings),
use `--log-json=<path>`.

**Data Generation**
Rule-based values:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --rule-data --rule-max-cases=3 -p examples examples/sut.cpp
```
Deterministic run:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --seed=123 -p examples examples/sut.cpp
```
Profiled data:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --profile=boundary -p examples examples/sut.cpp
```
Strict coverage mode:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --coverage-mode=strict -p examples examples/sut.cpp
```
Explicit oracle mode:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --oracle-mode=explicit -p examples examples/sut.cpp
```

**Coverage Modes**
Coverage mode controls how aggressive ASkeleTon is when deciding whether to
generate a test for a callable. This is separate from the data-generation
`--profile`.

- `balanced`: default behavior. Uses the current materialization heuristics and
  generates tests for mutable pointer/reference parameters when supported.
- `strict`: favors conservative, easy-to-review scaffolding. It skips:
  - functions and methods that require mutable pointer/reference parameter
    handling
  - instance methods that require constructing the object with a non-default
    constructor
- `aggressive`: currently behaves like `balanced`, but is exposed as the forward
  compatibility mode for more permissive generation in future iterations.

**Expected Value Strategy**
ASkeleTon currently exposes three oracle modes:

- `mirror`: default behavior. Generated tests derive `expected` through a second
  isolated execution of the same function or method using the same case data.
- `explicit`: generated tests first look for `expected` in the `.cfg`; if the
  key is missing, they fall back to `mirror`. This keeps freshly generated tests
  passing while allowing users to override specific cases with independent
  expectations.
- `property`: currently reserved for the next phase and intentionally falls back
  to `mirror`.

What this means in practice:
- The `.cfg` file stores the source case data that users are expected to edit.
- In `explicit`, users may add `expected` (or structured keys such as
  `expected.x`) to override the mirrored value.
- Generated test code creates internal `oracle_*` variables by re-reading that same
  data into a second set of local variables.
- If no explicit override exists, `expected` is computed from that isolated
  mirror execution, not from a separate specification or golden value.
- This also lets the generated test compare side effects on pointer/reference
  parameters against the mirrored execution.
- Parameters inferred as pure `out` values are no longer persisted as editable
  `.cfg` inputs; they are initialized internally in generated test code.

What this strategy is good for:
- Stable, reproducible scaffold tests.
- Detecting accidental mutations or aliasing differences in generated checks.
- Keeping generated `.cfg` files simpler because users do not maintain a separate
  `return_*` field.

What this strategy is not:
- It is not an independent semantic oracle.
- It does not prove the SUT is correct if the same deterministic bug appears in
  both executions.

This is an intentional first-phase approach. Future phases may replace or extend
it with stronger domain-specific or property-based oracles.

**Reproducible Runs**
Use `--seed` to make data generation deterministic. For fully reproducible
outputs across machines, keep these inputs identical:
- LLVM/Clang major version (e.g., 18).
- The exact `compile_commands.json`.
- `data/type_factories.json` and `data/default_values.json`.
- `data/system_files.json` (or disable refresh with `--no-system-files-refresh`).

**Type Factories and Stubs**
Configure `data/type_factories.json` to control how complex types are initialized.
```json
{
  "types": {},
  "functions": {}
}
```
Quick example:
```json
{
  "types": {
    "User": { "strategy": "factory", "expr": "MakeUser(\"guest\")" },
    "Session": { "strategy": "zeroed" },
    "Address": { "strategy": "dummy" }
  },
  "functions": {
    "BuildAdminSession": {
      "types": {
        "User": { "strategy": "factory", "expr": "MakeAdminUser()" }
      }
    }
  }
}
```
Notes:
- `factory`: uses the expression in the generated `Read_<Type>()` fixture method.
- `zeroed`: returns `{}` for record types.
- `dummy`: uses in-class defaults, then `data/default_values.json`, then zero.
- `functions`: lets you override the factory for a type only when generating a
  specific function or method. Function-scoped factories currently support only
  explicit `factory` expressions.

**Supported Structured Inputs**
ASkeleTon now has first-class input support for some common C++ standard library
shapes in generated `.cfg` files and fixtures:
- `std::optional<T>`
- `std::pair<T, U>`
- `std::tuple<Ts...>`

These are stored in the `.cfg` as structured keys rather than flattened strings.
Examples:
```text
value.has_value=true;#bool
value.value=39;#int

pairValue.first=1;#int
pairValue.second=2;#long

tupleValue.0=4;#int
tupleValue.1=5;#short
tupleValue.2=6;#long long
```

When a callable has an `std::optional<T>` parameter, ASkeleTon now generates at
least two cases when possible so the generated data covers both:
- a present value (`has_value=true`)
- an empty optional (`has_value=false`)

**Report JSON**
Use `--report` or `--report-json` to generate a machine-readable summary with:
- Per-entity status: `generated` or `skipped`.
- Failure reason and type details when skipped.
- The selected `coverage_mode`.
- The selected `oracle_mode`.
- `summary` section with counts by status, kind, target, reason, and file.
- Coverage metrics (`generation_rate`, `skip_rate`) plus top skip reasons and
  targets with the most skips.

Common `reason` values include:
- `abstract_record`
- `coverage_policy_instance_construction`
- `coverage_policy_mutable_parameter`
- `missing_fixture_strategy`
- `missing_instance_strategy`
- `non_public_lifecycle`
- `unsupported_indirection`

**Troubleshooting**
- `compile_commands.json` not found: pass `-p <build-path>` to its directory.
- `compile_commands.json` uses relative paths: supported. ASkeleTon normalizes
  entries internally so relative entries still match absolute source paths.
- Headers not found in fixture: add the include manually in `*_fixture.hpp`.
- Empty containers in `boundary` profile: use `random` or `safe`.
- `llvm-config-18 not found`: install `llvm-18` and `llvm-18-tools`.
- Link errors mentioning `APINotesManager`: install `libclang-18-dev`.

**Docs**
Architecture overview: `doc/Architecture.md`  
Rule catalog: `doc/DataRules.md`  
Release checklist: `doc/ReleaseChecklist.md`  
Known issues: `doc/KnownIssues.md`  
Dependencies: `doc/Dependencies.md`  
Examples: `examples/README.md`

If you are working with C++ headers use the option `-xc++` at the end.

Author: Kevin J. Valle-Gomez (kevin.valle@uca.es)  
Acknowledgements: José Manuel Heredia Bravo for his constant maintenance, support and help.
