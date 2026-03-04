Release Verification Checklist

Date: 2026-02-04

Recommended commands
1. Basic
`ASKELETON_HOME=$(pwd) ./askeleton -p examples examples/sut.cpp`

2. Rule-based data
`ASKELETON_HOME=$(pwd) ./askeleton --rule-data --rule-max-cases=3 -p examples examples/sut.cpp`

3. Profiles
- `ASKELETON_HOME=$(pwd) ./askeleton --profile=random -p examples examples/sut.cpp`
- `ASKELETON_HOME=$(pwd) ./askeleton --profile=boundary -p examples examples/sut.cpp`
- `ASKELETON_HOME=$(pwd) ./askeleton --profile=safe -p examples examples/sut.cpp`
- `ASKELETON_HOME=$(pwd) ./askeleton --profile=stress -p examples examples/sut.cpp`

4. Determinism
`ASKELETON_HOME=$(pwd) ./askeleton --seed=123 -p examples examples/sut.cpp`

Automated
- `scripts/check_all.sh`

Expected results
- Files are generated under `Generated/UT/<target>/`.
- The `.cfg` contains non-empty values for `random` and `safe`.
- `boundary` may intentionally include empty containers.
- Rule-based data generates multiple cases for supported comparisons.
- The default run and the explicit `--rule-data` run generate equivalent outputs
  (ignoring timestamps).
