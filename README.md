<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="doc/logo/ASkeleTon-logo-horizontal-dark.svg">
    <img src="doc/logo/ASkeleTon-logo-horizontal.svg" alt="ASkeleTon" width="560">
  </picture>
</p>

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
ASKELETON_HOME=$(pwd) ./askeleton --bootstrap-compdb -p examples examples/sut.cpp
```

By default, ASkeleTon generates GoogleTest scaffolding and writes output under
`tests/generated` relative to the first source file passed on the command line.

**Install (Ubuntu)**
ASkeleTon requires LLVM/Clang 18, libclang development headers, and a C/C++
build toolchain.

On Ubuntu 24.04, these packages are available directly from the default
repositories. On Ubuntu 22.04, install LLVM/Clang 18 from the official LLVM
APT repository first:
[https://apt.llvm.org/](https://apt.llvm.org/)

Then install:
```bash
sudo apt update
sudo apt install -y clang-18 llvm-18 llvm-18-dev llvm-18-tools libclang-18-dev build-essential
```

GoogleTest (`libgtest-dev`) is required for the main reproducibility workflow.
Boost.Test and Catch2 are optional backend dependencies; they are only
required when generating, building, or running scaffolding for those
backends.

Install GoogleTest for the default setup and the minimal end-to-end
reproducibility workflow:
```bash
sudo apt install -y libgtest-dev
```

If you want to generate tests for Boost.Test or Catch2, install the
corresponding optional backend libraries:
```bash
sudo apt install -y libboost-dev catch2
```

To install support for all available frameworks:
```bash
sudo apt install -y libboost-dev libgtest-dev catch2
```

**Other distros**
Install equivalents of LLVM/Clang 18, libclang development headers, and a C/C++ build toolchain.

**Build**
```bash
make CXX=clang++-18
./askeleton --version
```
`system_files.json` is created automatically if missing. Use
`--no-system-files-refresh` to skip this check.

**Minimal End-to-End Reproducibility Workflow**
For a compact end-to-end generate-build-run-refine workflow, run:

```bash
./scripts/check_main_workflow.sh
```

This script performs a minimal `examples/sut.cpp` workflow aligned with that
usage path:
- bootstrap `compile_commands.json`
- generate GoogleTest scaffolding plus `.cfg` data
- build the generated test
- execute the generated binary
- edit `sut.cfg` and confirm the rerun changes behavior
- emit `report.json` and `log.json`

The broader walkthrough input is `examples/sut_showcase.cpp`; the script above
is the smaller reproducibility check intended for clean clones.

To inspect the generated files yourself, pass an explicit output directory:

```bash
./scripts/check_main_workflow.sh /tmp/askeleton_main_workflow
```

**Usage**
```text
askeleton [options] <source0> [... <sourceN>]
```

Use `--out-dir` to override the default output location.

Key options:
- `-p <build-path>`: path to `compile_commands.json`.
- `--bootstrap-compdb`: auto-create/append minimal compile commands for missing source entries.
- `--framework=<gtest|boost|catch>`: select test framework.
- `--profile=<random|boundary|safe|stress>`: data generation profile.
- `--coverage-mode=<strict|balanced|aggressive>`: generation coverage policy.
- `--oracle-mode=<mirror|explicit|property>`: expected-value strategy.
- `--seed=<N>`: deterministic data generation.
- `--rule-data`: explicitly enable the default AST-guided rule-based values.
- `--no-rule-data`: disable AST-guided rule-based values and use fallback-only data generation.
- `--rule-max-cases=<N>`: limit rule-based test cases per function.
- `--out-dir=<path>`: output directory for generated tests.
- `--include-impl-under-include`: allow compiling `.c/.cc/.cpp` under `include/`.
- `--report=<path>`: write a JSON report of generated/skipped tests.
- `--report-json`: write a JSON report to `<out-dir>/askeleton_report.json`.
- `--log-json=<path>`: write an execution log with summary/warnings.
- `--no-system-files-refresh`: do not auto-create `data/system_files.json` if missing.
- `--quiet`, `--verbose`, `--debug`: control console verbosity.
- `--extra-arg`, `--extra-arg-before`: pass extra compiler args to Clang tooling.

For exhaustive option semantics and examples, see:
- [`doc/CLI.md`](doc/CLI.md)

For input generation profiles, rule data, seeds, and reproducibility, see:
- [`doc/InputGeneration.md`](doc/InputGeneration.md)

For the editable generated test data format, see:
- [`doc/CfgFormat.md`](doc/CfgFormat.md)

For evaluation output tables and local validation, see:
- [`REPRODUCIBILITY.md`](REPRODUCIBILITY.md)

**Console Output**
By default you get a concise progress view plus a final summary. Use:
- `--quiet` to print only errors.
- `--verbose` to include per-entity progress.
- `--debug` to include detailed parameter/return signatures.

To collect a JSON execution log with inputs, counts, warnings, and timings,
use `--log-json=<path>`.

**Generated Makefiles**
Generated Makefiles follow a consistent structure across frameworks:
- C/C++ split compilation: `.c` dependencies use `CC`, test translation units use `CXX`.
- Extra flags/libraries are overridable without editing templates:
  - `EXTRA_CPPFLAGS`, `EXTRA_CFLAGS`, `EXTRA_CXXFLAGS`
  - `EXTRA_LDFLAGS`, `EXTRA_LIBS`
- Base link libraries are exposed as `LIBS` in all framework Makefiles.
- Standard targets are available in all frameworks: `all`, `test`, `clean`, `compilation`.

Example:
```bash
make EXTRA_LIBS="-lcrypto" EXTRA_LDFLAGS="-L/path/to/lib"
```

**Data Generation**
These examples cover the most common ways to adjust generation behavior:

Default generation:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --bootstrap-compdb -p examples examples/sut.cpp
```

Deterministic generation:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --bootstrap-compdb --seed=123 -p examples examples/sut.cpp
```

Boundary-focused inputs:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --bootstrap-compdb --profile=boundary -p examples examples/sut.cpp
```

Conservative generation policy:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --bootstrap-compdb --coverage-mode=strict -p examples examples/sut.cpp
```

For more option combinations, see [`doc/CLI.md`](doc/CLI.md).
For the full input generation model, see
[`doc/InputGeneration.md`](doc/InputGeneration.md).

**Coverage Modes**
Coverage mode controls how selective ASkeleTon is when deciding whether to
generate a test for a callable. This is separate from the data-generation
`--profile`.

- `balanced`: default behavior and the recommended starting point.
- `strict`: favors conservative, easy-to-review scaffolding and skips callables
  that require mutable pointer/reference handling or non-default instance
  construction.
- `aggressive`: accepted for compatibility and experimentation; currently close
  to `balanced`, but kept explicit so repository workflows can reference it
  directly.

If a callable is skipped, check the report for the recorded reason.

**Expected Value Strategy**
ASkeleTon supports three expected-value strategies through the existing
`--oracle-mode` compatibility flag:

- `explicit`: default behavior. Generated tests first look for `expected` in the
  `.cfg`; if no override is present, they fall back to a mirrored execution.
- `mirror`: generated tests derive `expected` from a second isolated execution
  of the same callable with the same case data.
- `property`: generated tests replay the same callable with isolated inputs and
  compare the observable result across both executions.

In practice, the `.cfg` file stores the editable case data, and `explicit`
lets users override expected values when needed. These modes help keep
generated scaffolding stable and reproducible, but they are not an independent
semantic correctness oracle for the SUT. Mirrored execution and property mode
are characterization baselines unless users refine the generated tests with
reviewed explicit expected values.

For details and recommended usage, see [`doc/ExpectedValues.md`](doc/ExpectedValues.md).

**Type Factories and Stubs**
Use `data/type_factories.json` to customize how complex types are initialized
in generated fixtures and tests.

For the configuration format, supported strategies, and examples, see
[`doc/TypeFactories.md`](doc/TypeFactories.md).

**Instance Resolution**
For non-static methods, ASkeleTon tries to resolve a usable instance
automatically. When the AST cannot infer a safe construction path, configure
`data/instance_strategies.json`.

For the full resolution order, configuration format, and examples, see
[`doc/InstanceStrategies.md`](doc/InstanceStrategies.md).

**Supported Structured Inputs**
ASkeleTon supports structured input for some common C++ standard library
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

When a callable has an `std::optional<T>` parameter, ASkeleTon tries to
generate at least two cases when possible so the generated data covers both:
- a present value (`has_value=true`)
- an empty optional (`has_value=false`)

For complete `.cfg` syntax, expected-value overrides, and troubleshooting, see
[`doc/CfgFormat.md`](doc/CfgFormat.md).

**Reproducible Runs**
Use `--seed` to make data generation deterministic. For fully reproducible
outputs across machines, keep these inputs identical:
- ASkeleTon version or commit.
- LLVM/Clang major version (e.g., 18).
- The exact `compile_commands.json`.
- `data/type_factories.json` and `data/default_values.json`.
- `data/system_files.json` (or disable auto-create with `--no-system-files-refresh`).
For the complete reproducibility checklist, see
[`doc/InputGeneration.md`](doc/InputGeneration.md).

**Report JSON**
Use `--report` or `--report-json` to generate a JSON summary of the run with:
- Per-entity status: `generated` or `skipped`.
- Skip reasons and related type details when generation is not possible.
- The selected `coverage_mode`.
- The selected `oracle_mode`.
- Aggregate counts and coverage metrics.

Common `reason` values:
- `abstract_record`: type is abstract and cannot be instantiated for fixture setup.
- `non_public_lifecycle`: destructor/lifecycle is not publicly usable.
- `missing_fixture_strategy`: record cannot be constructed and no factory is configured.
- `missing_instance_strategy`: no valid instance plan for method invocation.
- `coverage_policy_mutable_parameter`: skipped by strict coverage policy.
- `unsupported_pointer_pointee`: pointee type is not auto-materializable.

Full guide with examples and fixes:
- [`doc/ReportSchema.md`](doc/ReportSchema.md)
- [`doc/SkipReasons.md`](doc/SkipReasons.md)

**Repository Evidence**
Key capabilities map to repository artifacts as follows:
- `compile_commands.json` + Clang AST analysis: [`src/askeleton.cpp`](src/askeleton.cpp), [`doc/Architecture.md`](doc/Architecture.md)
- Supported backends (`gtest`, `boost`, `catch`): [`src/framework/`](src/framework), [`data/templates/`](data/templates)
- Separate editable test data and test logic: [`scripts/check_main_workflow.sh`](scripts/check_main_workflow.sh)
- JSON report and execution log: [`src/Report.cpp`](src/Report.cpp), `--report`, `--log-json`
- Explicit expected-value refinement in the minimal reproducibility workflow: [`scripts/check_main_workflow.sh`](scripts/check_main_workflow.sh), [`REPRODUCIBILITY.md`](REPRODUCIBILITY.md)
- Seeded generation and the available profiles/expected-value modes: [`doc/CLI.md`](doc/CLI.md), `--seed`, `--profile`, `--oracle-mode`
- Framework extensibility points: [`include/framework/Generator.hpp`](include/framework/Generator.hpp), [`doc/Architecture.md`](doc/Architecture.md)

**Troubleshooting**
- `compile_commands.json` not found: pass `-p <build-path>` to its directory.
- `compile_commands.json` uses relative paths: supported. ASkeleTon normalizes
  entries internally so relative entries still match absolute source paths.
- Headers not found in fixture: add the include manually in `*_fixture.hpp`.
- Empty containers in `boundary` profile: use `random` or `safe`.
- `llvm-config-18 not found`: install `llvm-18` and `llvm-18-tools`.
- Link errors mentioning `APINotesManager`: install `libclang-18-dev`.
- Link errors for external symbols (e.g. OpenSSL internals): the generated test
  may need extra project libraries during link; this is project-specific and
  must be provided in the generated Makefile/link flags.
- If a constructor or method is skipped, check the report reason and the
  guidance in [`doc/SkipReasons.md`](doc/SkipReasons.md).

**Docs**
- CLI reference: [`doc/CLI.md`](doc/CLI.md)
- Architecture overview: [`doc/Architecture.md`](doc/Architecture.md)
- Input generation: [`doc/InputGeneration.md`](doc/InputGeneration.md)
- Rule catalog: [`doc/DataRules.md`](doc/DataRules.md)
- Expected value strategies: [`doc/ExpectedValues.md`](doc/ExpectedValues.md)
- Generated `.cfg` format: [`doc/CfgFormat.md`](doc/CfgFormat.md)
- Release checklist: [`doc/ReleaseChecklist.md`](doc/ReleaseChecklist.md)
- Known issues: [`doc/KnownIssues.md`](doc/KnownIssues.md)
- Skip reasons guide: [`doc/SkipReasons.md`](doc/SkipReasons.md)
- Report schema: [`doc/ReportSchema.md`](doc/ReportSchema.md)
- Instance strategies: [`doc/InstanceStrategies.md`](doc/InstanceStrategies.md)
- Type factories: [`doc/TypeFactories.md`](doc/TypeFactories.md)
- Dependencies: [`doc/Dependencies.md`](doc/Dependencies.md)
- Examples: [`examples/README.md`](examples/README.md)
- Reproducibility: [`REPRODUCIBILITY.md`](REPRODUCIBILITY.md)

**Citation**
Use [`CITATION.cff`](CITATION.cff) to cite the software version in use. Release
metadata for archival deposition is tracked in [`.zenodo.json`](.zenodo.json)
and should be updated with the final DOI when the archival record is minted.

**License**
ASkeleTon is released under the Apache License 2.0. See
[`LICENSE.txt`](LICENSE.txt).

Bundled third-party code retains its own license notices. In particular,
the vendored `nlohmann/json` headers under `include/nlohmann/` retain their
upstream SPDX notices.

If you are working with C++ headers, use the option `-xc++`.

**Authors**
- Kevin J. Valle-Gómez (kevin.valle@uca.es)
- Pedro Delgado-Pérez (pedro.delgado@uca.es)
- Inmaculada Medina-Bulo (inmaculada.medina@uca.es)

Acknowledgements: José Manuel Heredia Bravo for his constant maintenance, support and help.
