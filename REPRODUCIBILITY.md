# Reproducibility

This document describes how to reproduce the release smoke tests, the main
clean-clone example workflow, and the applicability evaluation workflow for
ASkeleTon 1.0.1.

## Tested Environment

The release workflow is tested on:

- Ubuntu 22.04 with LLVM/Clang 18 from the official LLVM APT repository.
- Ubuntu 24.04 with LLVM/Clang 18 from the default Ubuntu repositories.
- C++20 compiler support.

Required core packages to build ASkeleTon:

```bash
sudo apt update
sudo apt install -y clang-18 llvm-18 llvm-18-dev llvm-18-tools libclang-18-dev build-essential
```

Required for the main reproducibility workflow in this document
(`./scripts/check_main_workflow.sh`):

```bash
sudo apt install -y libgtest-dev
```

GoogleTest (`libgtest-dev`) is required for the main reproducibility workflow.
Boost.Test and Catch2 are optional backend dependencies; they are only
required when generating, building, or running scaffolding for those
backends.

Optional backend packages for the additional framework checks:

```bash
sudo apt install -y libboost-dev catch2
```

## Build From a Clean Clone

```bash
make CXX=clang++-18
./askeleton --version
```

Expected version output for this release:

```text
ASkeleTon 1.0.1
Built with LLVM 18.x.y
```

The LLVM patch version may vary across supported systems.

## Minimal Reproducible Run

The repository does not version `examples/compile_commands.json` because that
file contains machine-specific absolute paths. For the bundled examples, use
`--bootstrap-compdb` to create a local compilation database.

The shortest end-to-end verification is:

```bash
./scripts/check_main_workflow.sh
```

This script generates tests for `examples/sut.cpp`, builds them, runs them,
edits the generated `sut.cfg`, confirms that the rerun observes the edited
expected value, and writes `report.json` plus `log.json`.

This is the compact reproducibility subject used for clean-clone verification.
The manuscript's illustrative walkthrough example uses `examples/sut_showcase.cpp`.

Manual equivalent:

```bash
ASKELETON_HOME=$(pwd) ./askeleton \
  --bootstrap-compdb \
  -p examples \
  --framework=gtest \
  --profile=random \
  --coverage-mode=balanced \
  --oracle-mode=explicit \
  --seed=123 \
  --out-dir=/tmp/askeleton_sut/generated \
  --report=/tmp/askeleton_sut/report.json \
  --log-json=/tmp/askeleton_sut/log.json \
  examples/sut.cpp
make -C /tmp/askeleton_sut/generated/sut
/tmp/askeleton_sut/generated/sut/sut_test
```

Expected summary:

```text
Found: 10
Generated: 10
Skipped: 0
Generation rate: 100.00%
```

Expected generated artifacts:

- `/tmp/askeleton_sut/generated/sut/sut_fixture.hpp`
- `/tmp/askeleton_sut/generated/sut/sut_test.cpp`
- `/tmp/askeleton_sut/generated/sut/sut.cfg`
- `/tmp/askeleton_sut/generated/sut/Makefile`
- `/tmp/askeleton_sut/report.json`
- `/tmp/askeleton_sut/log.json`

## Automated Smoke Checks

Run:

```bash
./scripts/check_main_workflow.sh
./scripts/check_all.sh
```

This checks:

- The full documented `examples/sut.cpp` workflow, including `.cfg` refinement.
- A minimal temporary C++ source file.
- The bundled `examples/sut.cpp` baseline.
- Rule-data generation.
- `boundary`, `safe`, and `stress` profiles.

`check_main_workflow.sh` removes the temporary bootstrap files it creates under
the repository (for example `examples/compile_commands.json`) before exiting.
`check_all.sh` may still create ignored local files such as
`examples/compile_commands.json` and generated outputs under `examples/tests/`.

## CI Validation

The GitHub Actions workflow builds ASkeleTon and validates generated tests for:

- The full clean-clone `examples/sut.cpp` workflow.
- GoogleTest.
- Boost.Test.
- Catch2.
- Preservation of a C++20 compile standard from `compile_commands.json`.

See `.github/workflows/ci.yml`.

## Applicability Evaluation Workflow

The script `scripts/run_eval.py` reproduces the applicability evaluation
workflow for this release.

The complete campaign contains 156 ASkeleTon executions:

- `sut_showcase`: 72 runs (`full_factorial`)
- `tinyxml2_core`: 72 runs (`full_factorial`)
- `tinyxml2_xmltest`: 3 runs (`base_plus_coverage`)
- `yamlcpp_emitter`: 3 runs (`base_plus_coverage`)
- `openssl_ctype`: 3 runs (`base_plus_coverage`)
- `sqlite_util`: 3 runs (`base_plus_coverage`)

The two `full_factorial` subjects are evaluated across:

- 4 profiles: `random`, `boundary`, `safe`, `stress`
- 3 coverage modes: `strict`, `balanced`, `aggressive`
- 3 oracle modes: `mirror`, `explicit`, `property`
- 2 rule-data settings: `off`, `on`

This yields `4 * 3 * 3 * 2 = 72` runs per full-factorial subject.

The four `base_plus_coverage` subjects are evaluated with:

- 1 baseline run: `random + balanced + explicit + rule-data on`
- 2 coverage ablation runs: `strict` and `aggressive`

By default, outputs are written under:

```text
analysis/eval_<timestamp>/
```

including:

- `raw_runs.csv`
- `baseline_summary.csv`
- `coverage_ablation.csv`
- `full_factorial_sensitivity.csv`
- `skip_reason_summary.csv`
- `evaluation_tables.md`
- `run_metadata.json`
- `viewer.html` when `--build-viewer` is used
- per-run reports, logs, and generated outputs under `<subject>/`

The following repository files are part of the release reproducibility
workflow:

- `scripts/run_eval.py`
- `scripts/reproduce_eval.sh`
- `scripts/build_eval_viewer.py`

Additional tools required only for the external-subject evaluation workflow:

```bash
sudo apt install -y cmake git perl tcl
```

Run the complete campaign:

```bash
python3 scripts/run_eval.py --prepare-subjects --build-viewer
```

Or use the one-command wrapper that builds ASkeleTon first:

```bash
./scripts/reproduce_eval.sh
```

Use `--out-dir` to choose an explicit output directory:

```bash
python3 scripts/run_eval.py \
  --prepare-subjects \
  --build-viewer \
  --out-dir analysis/eval_full
```

Run a subset for a quick check:

```bash
python3 scripts/run_eval.py \
  --subjects sut_showcase \
  --out-dir /tmp/askeleton_eval_subset \
  --build-viewer \
  --force
```

The `sut_showcase` subset above is the clean-clone-safe quick check. External
subjects are only required when the selected subject list includes them.

The evaluation uses external subject projects that are intentionally not
versioned as part of the release branch. The script can clone missing subjects
and checkout the recorded snapshots when `--prepare-subjects` is used:

- TinyXML2
- yaml-cpp
- OpenSSL
- SQLite

Recorded external snapshots:

| Subject | Repository | Commit |
| --- | --- | --- |
| TinyXML2 | `https://github.com/leethomason/tinyxml2.git` | `3324d04d58de9d5db09327db6442f075e519f11b` |
| yaml-cpp | `https://github.com/jbeder/yaml-cpp` | `05c050c6c14d5c3a82cbc368b50d985896922196` |
| OpenSSL | `https://github.com/openssl/openssl` | `0ed06337e38ec70e5beb043d5a1da9a6b6e8c57e` |
| SQLite | `https://github.com/sqlite/sqlite` | `c739d132175932cde2c7c2f38d625165991f2a5d` |

The generated tables provide the reference applicability counts and omission
category totals for this evaluation workflow.

The helper script `scripts/run_mode_matrix.py` is not part of the release
reproduction path described here. It can still be useful for
exploratory local analyses, but the evaluation-focused entry points are the three
scripts listed above.

The script also updates:

- `analysis/eval_latest` as a symlink to the latest campaign output

For archival publication, these external subjects and generated campaign
outputs should also be provided by one of:

- A Zenodo dataset with exact source snapshots and generated artifacts.
- The public repositories plus the exact commits above and this campaign script.

## Files Intentionally Excluded From Git

The following are local evaluation assets, generated files, or machine-specific
files and are intentionally ignored:

- `artifacts/`
- `.campaign/`
- `.campaign_probe/`
- `examples/openssl/`
- `examples/sqlite/`
- `examples/tinyxml2/`
- `examples/matclpro/`
- `examples/compile_commands.json`
- `examples/tests/`
- `data/system_files.json`
- `askeleton`
- `*.o`
- `*.d`

## Release Consistency Checks

Before publishing a release archive:

```bash
make clean
make CXX=clang++-18
./askeleton --version
./scripts/check_all.sh
```

Confirm that:

- `./askeleton --version` reports `ASkeleTon 1.0.1`.
- `LICENSE.txt` is present.
- `CITATION.cff` is present and has the final repository URL.
- `CITATION.cff` is updated with the final DOI once the archival record is minted.
- `.zenodo.json` is present and has the final repository URL.
- No external-project folders or generated artifacts are staged for commit.
