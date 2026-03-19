# Release Checklist

This checklist is meant for release verification and regression smoke tests.

## Recommended Commands

### Basic Run

```bash
ASKELETON_HOME=$(pwd) ./askeleton -p examples examples/sut.cpp
```

### Rule-Based Data

```bash
ASKELETON_HOME=$(pwd) ./askeleton --rule-data --rule-max-cases=3 -p examples examples/sut.cpp
```

### Profiles

```bash
ASKELETON_HOME=$(pwd) ./askeleton --profile=random -p examples examples/sut.cpp
ASKELETON_HOME=$(pwd) ./askeleton --profile=boundary -p examples examples/sut.cpp
ASKELETON_HOME=$(pwd) ./askeleton --profile=safe -p examples examples/sut.cpp
ASKELETON_HOME=$(pwd) ./askeleton --profile=stress -p examples examples/sut.cpp
```

### Determinism

```bash
ASKELETON_HOME=$(pwd) ./askeleton --seed=123 -p examples examples/sut.cpp
```

### Automated Checks

```bash
./scripts/check_all.sh
```

## Expected Results

- Files are generated under `tests/generated` relative to the analyzed source.
- The `.cfg` contains non-empty values for `random` and `safe`.
- `boundary` may intentionally include empty containers.
- Rule-based data generates multiple cases for supported comparisons.
- The default run and the explicit `--rule-data` run generate equivalent outputs,
  ignoring timestamps and similar metadata.

## Related Guides

- CLI reference: [`CLI.md`](CLI.md)
- Rule-based generation: [`DataRules.md`](DataRules.md)
