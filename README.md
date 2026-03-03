**ASkeleTon**
Generate C/C++ unit test scaffolding from `compile_commands.json` with GoogleTest, Boost.Test, or Catch2.  
Outputs fixtures, tests, Makefiles, and `.cfg` data files with deterministic or rule-based values.

**Highlights**
- AST-driven discovery of functions, methods, and constructors.
- Profiles for data generation: `random`, `boundary`, `safe`, `stress`.
- Rule-based values extracted from comparisons.
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

**Expected Value Strategy**
Today ASkeleTon does not generate a semantic oracle independent from the SUT.
Instead, generated tests derive `expected` through a second isolated execution of
the same function or method using the same case data.

What this means in practice:
- The `.cfg` file stores only the source case data that users are expected to edit.
- Generated test code creates internal `oracle_*` variables by re-reading that same
  data into a second set of local variables.
- `expected` is computed from that isolated mirror execution, not from a separate
  specification or golden value.
- This also lets the generated test compare side effects on pointer/reference
  parameters against the mirrored execution.

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
  "types": {
    "MyType": { "strategy": "factory", "expr": "MakeMyType()" },
    "OtherType": { "strategy": "zeroed" },
    "ThirdType": { "strategy": "dummy" }
  }
}
```
Quick example:
```json
{
  "types": {
    "User": { "strategy": "factory", "expr": "MakeUser(\"guest\")" },
    "Session": { "strategy": "zeroed" },
    "Address": { "strategy": "dummy" }
  }
}
```
Notes:
- `factory`: uses the expression in the generated `Read_<Type>()` fixture method.
- `zeroed`: returns `{}` for record types.
- `dummy`: uses in-class defaults, then `data/default_values.json`, then zero.

**Report JSON**
Use `--report` or `--report-json` to generate a machine-readable summary with:
- Per-entity status: `generated` or `skipped`.
- Failure reason and type details when skipped.
- `summary` section with counts by status, kind, and target.

**Troubleshooting**
- `compile_commands.json` not found: pass `-p <build-path>` to its directory.
- `compile_commands.json` uses relative paths: supported. ASkeleTon normalizes
  entries internally so relative entries still match absolute source paths.
- Headers not found in fixture: add the include manually in `*_fixture.hpp`.
- Empty containers in `boundary` profile: use `random` or `safe`.
- `llvm-config-18 not found`: install `llvm-18` and `llvm-18-tools`.
- Link errors mentioning `APINotesManager`: install `libclang-18-dev`.

**Docs**
Architecture overview: `doc/Arquitectura.md`  
Rule catalog: `doc/Reglas para datos`  
Release checklist: `doc/Checklist.md`  
Examples: `examples/README.md`

If you are working with C++ headers use the option `-xc++` at the end.

Author: Kevin J. Valle-Gomez (kevin.valle@uca.es)  
Acknowledgements: José Manuel Heredia Bravo for his constant maintenance, support and help.
