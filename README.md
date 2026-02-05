**Overview**
ASkeleTon generates C/C++ unit test skeletons (GoogleTest, Boost.Test, Catch2) from
`compile_commands.json`. It emits test files, fixtures, a Makefile, and `.cfg`
data files with configurable data generation (random, rule-based, or profiled).

**Requirements (Ubuntu/Debian)**
- clang-18
- llvm-18
- llvm-config-18
- libclang-18-dev
- Optional: g++-12 / gcc-12
- Optional: libboost-dev (Boost.Test)
- Optional: libgtest-dev (GoogleTest)
- Optional: catch2 (Catch2)

**Install Dependencies (Ubuntu 24.04)**
```bash
sudo apt update
sudo apt install -y clang-18 llvm-18 llvm-18-dev llvm-18-tools libclang-18-dev build-essential
```
Optional (Boost.Test framework):
```bash
sudo apt install -y libboost-dev
```
Optional (GoogleTest/Catch2 frameworks):
```bash
sudo apt install -y libgtest-dev catch2
```
Quick verify:
```bash
clang++-18 --version
llvm-config-18 --version
```
**Troubleshooting (Ubuntu 24.04)**
- `llvm-config-18: command not found`: ensure `llvm-18` and `llvm-18-tools` are installed.
- Link errors mentioning `APINotesManager` or `libclangAPINotes`: confirm `libclang-18-dev` is installed.
- `clang++-18: command not found`: install `clang-18` (or set `CXX=clang++-18`).

**Environment**
Set `ASKELETON_HOME` to the repository root.

**Build**
```bash
make
```

**Optional Sanitizer Build (AddressSanitizer)**
```bash
make clean
make CXXFLAGS='-std=c++20 -Wall -Wextra -Wcast-qual -Wwrite-strings -Wno-unused-parameter -Wdelete-non-virtual-dtor -fPIC -ffunction-sections -fdata-sections -fsanitize=address -fno-omit-frame-pointer' \
    DEBUG_FLAGS='' OPTIMIZATION_FLAGS='-O1' \
    CLANGLIBS="$(llvm-config-18 --ldflags --system-libs --libs | tr '\n' ' ') -lclangFrontend -lclangAPINotes -lclangSerialization -lclangDriver -lclangTooling -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangASTMatchers -lclangLex -lclangBasic -lclangRewrite -lclangRewriteFrontend -lclangSupport -fsanitize=address"
```

**Quick Sanity Check**
```bash
scripts/sanity_types.sh
```

**Quick Start**
```bash
ASKELETON_HOME=$(pwd) ./askeleton -p examples examples/sut.cpp
```

**Sample Output**
Example snippet from `Generated/UT/sut/sut.cfg`:
```cfg
classify_1:
{
    a=9;#int
    return_int=42;#int
};
```

**Data Generation**
Rule-based values from simple AST comparisons:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --rule-data --rule-max-cases=3 -p examples examples/sut.cpp
```
Limit rule-based cases:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --rule-data --rule-max-cases=2 -p examples examples/sut.cpp
```
Deterministic runs:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --seed=123 -p examples examples/sut.cpp
```
Profiles (random | boundary | safe | stress):
```bash
ASKELETON_HOME=$(pwd) ./askeleton --profile=boundary -p examples examples/sut.cpp
```
The generated `.cfg` header records the profile and optional seed used.

**Usage**
```
askeleton [options] <source0> [... <sourceN>]
```
Key options:
- `-p <build-path>`: path to `compile_commands.json`.
- `--framework=<gtest|boost|catch>`: select test framework.
- `--rule-data`: enable rule-based values derived from AST.
- `--rule-max-cases=<N>`: limit rule-based test cases per function.
- `--seed=<N>`: deterministic data generation.
- `--profile=<random|boundary|safe|stress>`: data generation profile.
- `--report=<path>`: write a JSON report of generated/skipped tests.
- `--report-json`: write a JSON report to `Generated/UT/askeleton_report.json`.
- `-extra-arg`, `-extra-arg-before`: pass extra compiler args to Clang tooling.

**How `-p` Works**
`-p <build-path>` points to a directory containing `compile_commands.json`. If it
is omitted, ASkeleTon searches parent paths of the first input file.

**Type Factories and Stubs**
Configure `data/type_factories.json` to control how complex types are initialized:
```json
{
  "types": {
    "MyType": { "strategy": "factory", "expr": "MakeMyType()" },
    "OtherType": { "strategy": "zeroed" },
    "ThirdType": { "strategy": "dummy" }
  }
}
```
Notes:
- `factory`: uses the given expression in the generated fixture `Read_<Type>()`.
- `zeroed`: returns `{}` for record types.
- `dummy`: uses in-class default initializers when available; otherwise uses
  `data/default_values.json` for primitive fields (falls back to zero values).

**Scope and Limitations**
ASkeleTon focuses on generating compilable test scaffolding and data files. It
does not validate business logic or automatically create assertions beyond the
framework templates. Complex or opaque types may require manual fixture edits.

**Compatibility**
Validated with clang/llvm 18. Other versions may work, but are not guaranteed.

**Troubleshooting**
- `compile_commands.json` not found: pass `-p <build-path>` pointing to the build
  directory that contains it.
- Headers not found in the fixture: add the correct include manually in the
  generated `*_fixture.hpp`.
- Empty containers: expected in `boundary` profile; use `random` or `safe` to avoid.

Architecture overview: see `doc/Arquitectura.md`.
Rule catalog: see `doc/Reglas para datos`.
Release checklist: see `doc/Checklist.md`.
Examples: see `examples/README.md`.

If you are working with C++ headers use the option `-xc++` at the end.

Author: Kevin J. Valle-Gomez (kevin.valle@uca.es)  
Acknowledgements: José Manuel Heredia Bravo for his constant maintenance, support and help.
