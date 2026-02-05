**ASkeleTon**
Generate C/C++ unit test scaffolding from `compile_commands.json` with GoogleTest, Boost.Test, or Catch2.  
Outputs fixtures, tests, Makefiles, and `.cfg` data files with deterministic or rule-based values.

**Highlights**
- AST-driven discovery of functions, methods, and constructors.
- Profiles for data generation: `random`, `boundary`, `safe`, `stress`.
- Rule-based values extracted from comparisons.
- JSON report with per-target summary.
- Output inside the SUT repo by default.

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
- `--report=<path>`: write a JSON report of generated/skipped tests.
- `--report-json`: write a JSON report to `<out-dir>/askeleton_report.json`.
- `--no-system-files-refresh`: do not regenerate `data/system_files.json`.
- `-extra-arg`, `-extra-arg-before`: pass extra compiler args to Clang tooling.

**Default Output**
By default, ASkeleTon writes output under `tests/generated` relative to the first
source file passed on the command line. Use `--out-dir` to override this.

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
