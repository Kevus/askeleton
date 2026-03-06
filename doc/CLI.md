# CLI Reference

This document is the exhaustive command-line reference for `askeleton`.

## Synopsis

```bash
askeleton [options] <source0> [... <sourceN>]
```

## Core Requirement

ASkeleTon needs a valid `compile_commands.json` entry per input source.
Use `-p <build-path>` to point to the folder containing that file.

Quick bootstrap example for single-file demos:
```bash
ASKELETON_HOME=$(pwd) ./askeleton --bootstrap-compdb -p . ./sut.cpp
```

## Option Reference

### Build/Inputs

`-p <build-path>`
- Meaning: directory containing `compile_commands.json`.
- Typical use:
```bash
./askeleton -p build src/foo.cpp
```

`--bootstrap-compdb`
- Meaning: if some input source has no compile command, append a minimal entry and continue.
- Best for quick demos/single-file experiments.
- Example:
```bash
./askeleton --bootstrap-compdb -p . ./sut.cpp
```

`--include-impl-under-include`
- Meaning: include `.c/.cc/.cpp` files placed under `include/` (normally skipped).
- Example:
```bash
./askeleton -p build --include-impl-under-include include/impl.cpp
```

`--extra-arg=<arg>`
- Meaning: append one compiler arg for Clang tooling.
- Example:
```bash
./askeleton -p build --extra-arg=-Wno-unused-parameter src/foo.cpp
```

`--extra-arg-before=<arg>`
- Meaning: prepend one compiler arg for Clang tooling.
- Example:
```bash
./askeleton -p build --extra-arg-before=-DUSE_DEMO=1 src/foo.cpp
```

### Generation Targets

`--framework=<gtest|boost|catch>` (default: `gtest`)
- Meaning: generated test framework template family.
- Example:
```bash
./askeleton -p build --framework=boost src/foo.cpp
```

`--out-dir=<path>`
- Meaning: explicit output directory for generated tests.
- Example:
```bash
./askeleton -p build --out-dir /tmp/generated_ut src/foo.cpp
```

### Data Generation

`--profile=<random|boundary|safe|stress>` (default: `random`)
- Meaning: how input values are produced.
- Modes:
  - `random`: broad random coverage.
  - `boundary`: edge-oriented values.
  - `safe`: conservative values.
  - `stress`: more extreme values.
- Example:
```bash
./askeleton -p build --profile=boundary src/foo.cpp
```

`--seed=<N>`
- Meaning: deterministic generation when `N >= 0`.
- Example:
```bash
./askeleton -p build --seed=123 src/foo.cpp
```

`--rule-data` / `--no-rule-data`
- Meaning: enable/disable AST-guided rule-based values from simple comparisons.
- Default behavior: rule-data is enabled.
- Examples:
```bash
./askeleton -p build --rule-data src/foo.cpp
./askeleton -p build --no-rule-data src/foo.cpp
```

`--rule-max-cases=<N>` (default: `3`)
- Meaning: maximum rule-generated cases per function.
- Example:
```bash
./askeleton -p build --rule-data --rule-max-cases=5 src/foo.cpp
```

### Coverage and Oracle Strategy

`--coverage-mode=<strict|balanced|aggressive>` (default: `balanced`)
- Meaning: policy controlling generation aggressiveness.
- Modes:
  - `balanced`: practical default.
  - `strict`: conservative; skips mutable/complex instance paths.
  - `aggressive`: forward-compatible permissive mode (currently close to balanced).
- Example:
```bash
./askeleton -p build --coverage-mode=strict src/foo.cpp
```

`--oracle-mode=<mirror|explicit|property>` (default: `explicit`)
- Meaning: expected-value strategy in generated tests.
- Modes:
  - `mirror`: isolated replay derives expected.
  - `explicit`: cfg expected override with mirror fallback.
  - `property`: repeatability-oriented replay oracle.
- Example:
```bash
./askeleton -p build --oracle-mode=property src/foo.cpp
```

### Reports and Logging

`--report=<path>`
- Meaning: write generation report JSON to explicit path.
- Example:
```bash
./askeleton -p build --report=/tmp/report.json src/foo.cpp
```

`--report-json`
- Meaning: write report to `<out-dir>/askeleton_report.json`.
- Example:
```bash
./askeleton -p build --report-json --out-dir /tmp/out src/foo.cpp
```

`--log-json=<path>`
- Meaning: write execution log JSON (counts, warnings, timings).
- Example:
```bash
./askeleton -p build --log-json=/tmp/run.log.json src/foo.cpp
```

### Verbosity and Runtime Controls

`--quiet`
- Meaning: errors-only console output.

`--verbose`
- Meaning: more progress detail.

`--debug`
- Meaning: detailed debug verbosity.

`--deep-level=<level>` (default: `1`)
- Meaning: internal generator maximum depth.

`--no-system-files-refresh`
- Meaning: skip system files refresh/update step.

## Interaction Notes

- `--no-rule-data` overrides `--rule-data` if both are passed.
- `--report` and `--report-json` both enable report output; `--report` path is explicit.
- `--seed` affects generated data, and is included in metadata when enabled.
- In strict coverage mode, some entities are intentionally skipped by policy.

## Skip Reasons

Skip reason meaning and remediation is documented in:
- `doc/SkipReasons.md`

## Practical End-to-End Examples

Default run:
```bash
ASKELETON_HOME=$(pwd) ./askeleton -p examples examples/sut_showcase.cpp
```

Deterministic strict run with report/log:
```bash
ASKELETON_HOME=$(pwd) ./askeleton \
  -p examples \
  --seed=123 \
  --coverage-mode=strict \
  --oracle-mode=explicit \
  --report=/tmp/askeleton_report.json \
  --log-json=/tmp/askeleton_log.json \
  examples/sut_showcase.cpp
```

Boost framework with boundary profile:
```bash
ASKELETON_HOME=$(pwd) ./askeleton \
  -p examples \
  --framework=boost \
  --profile=boundary \
  --out-dir /tmp/askeleton_boost \
  examples/sut_showcase.cpp
```
