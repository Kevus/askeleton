**Overview**
ASkeleTon generates C/C++ unit test skeletons (GoogleTest, Boost.Test, Catch2) from
`compile_commands.json`. It emits test files, fixtures, a Makefile, and `.cfg`
data files with configurable data generation (random, rule-based, or profiled).

**Requirements (Ubuntu/Debian)**
- clang-15
- llvm-15
- llvm-config-15
- libclang-15-dev
- Optional: g++-12 / gcc-12
- Optional: libboost-dev

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
    CLANGLIBS="$(llvm-config-15 --ldflags --system-libs --libs | tr '\n' ' ') -lclangFrontend -lclangSerialization -lclangDriver -lclangTooling -lclangParse -lclangSema -lclangAnalysis -lclangEdit -lclangAST -lclangASTMatchers -lclangLex -lclangBasic -lclangRewrite -lclangRewriteFrontend -lclangSupport -fsanitize=address"
```

**Quick Sanity Check**
```bash
scripts/sanity_types.sh
```

**Quick Start**
```bash
ASKELETON_HOME=$(pwd) ./askeleton -p examples examples/sut.cpp
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
- `-extra-arg`, `-extra-arg-before`: pass extra compiler args to Clang tooling.

**How `-p` Works**
`-p <build-path>` points to a directory containing `compile_commands.json`. If it
is omitted, ASkeleTon searches parent paths of the first input file.

**Scope and Limitations**
ASkeleTon focuses on generating compilable test scaffolding and data files. It
does not validate business logic or automatically create assertions beyond the
framework templates. Complex or opaque types may require manual fixture edits.

Architecture overview: see `doc/Arquitectura.md`.

If you are working with C++ headers use the option `-xc++` at the end.

Author: Kevin J. Valle-Gomez (kevin.valle@uca.es)  
Acknowledgements: José Manuel Heredia Bravo for his constant maintenance, support and help.
