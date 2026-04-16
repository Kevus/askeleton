# Reproducibility

This document describes how to reproduce the release smoke tests and the
publication evaluation workflow for ASkeleTon 1.0.0.

## Tested Environment

The release workflow is tested on:

- Ubuntu 22.04 with LLVM/Clang 18 from the official LLVM APT repository.
- Ubuntu 24.04 with LLVM/Clang 18 from the default Ubuntu repositories.
- C++20 compiler support.

Required core packages:

```bash
sudo apt update
sudo apt install -y clang-18 llvm-18 llvm-18-dev llvm-18-tools libclang-18-dev build-essential
```

Optional packages for generated-test validation:

```bash
sudo apt install -y libgtest-dev libboost-dev catch2
```

## Build From a Clean Clone

```bash
make CXX=clang++-18
./askeleton --version
```

Expected version output for this release:

```text
ASkeleTon 1.0.0
Built with LLVM 18.x.y
```

The LLVM patch version may vary across supported systems.

## Minimal Reproducible Run

The repository does not version `examples/compile_commands.json` because that
file contains machine-specific absolute paths. For the bundled examples, use
`--bootstrap-compdb` to create a local compilation database:

```bash
ASKELETON_HOME=$(pwd) ./askeleton \
  --bootstrap-compdb \
  -p examples \
  --seed=123 \
  --out-dir=/tmp/askeleton_sut \
  --report=/tmp/askeleton_sut/report.json \
  examples/sut.cpp
```

Expected summary:

```text
Found: 10
Generated: 10
Skipped: 0
Generation rate: 100.00%
```

## Automated Smoke Checks

Run:

```bash
./scripts/check_all.sh
```

This checks:

- A minimal temporary C++ source file.
- The bundled `examples/sut.cpp` baseline.
- Rule-data generation.
- `boundary`, `safe`, and `stress` profiles.

The script may create ignored local files such as `examples/compile_commands.json`
and generated outputs under `examples/tests/`.

## CI Validation

The GitHub Actions workflow builds ASkeleTon and validates generated tests for:

- GoogleTest.
- Boost.Test.
- Catch2.

See `.github/workflows/ci.yml`.

## publication Evaluation Workflow

The script `scripts/run_publication_eval.py` runs the full publication evaluation
campaign used for the release results. The complete campaign contains 407
ASkeleTon executions:

- `sut`: 82 runs.
- `sut_showcase`: 82 runs.
- `tinyxml2`: 81 runs.
- `sqlite`: 81 runs.
- `openssl`: 81 runs.

By default, outputs are written under:

```text
analysis/publication_eval_<timestamp>/
```

including:

- `campaign_metadata.json`
- `summaries/all_runs.csv`
- `summaries/all_runs.json`
- `summaries/case_overview.md`
- `summaries/case_overview.json`
- `summaries/<case>.csv`
- per-run reports, logs, commands, and generated outputs under `runs/`

Run the complete campaign:

```bash
scripts/run_publication_eval.py --prepare-subjects
```

Use `--out-dir` to choose an explicit output directory:

```bash
scripts/run_publication_eval.py \
  --prepare-subjects \
  --out-dir analysis/publication_eval_full
```

Run a subset for a quick check:

```bash
scripts/run_publication_eval.py --cases sut --out-dir /tmp/askeleton_eval_sut
```

The full evaluation uses external subject projects that are intentionally not
distributed inside the source repository. The script can clone missing subjects
at the recorded commits when `--prepare-subjects` is used:

- OpenSSL
- SQLite
- tinyxml2

Recorded external snapshots:

| Subject | Repository | Commit |
| --- | --- | --- |
| OpenSSL | `https://github.com/openssl/openssl.git` | `81cc6cb97ef83ad138eebd47129368b9e963e8cd` |
| SQLite | `https://github.com/sqlite/sqlite.git` | `140cbff0d2acc5b375109d567fba352a3be2663f` |
| tinyxml2 | `https://github.com/leethomason/tinyxml2.git` | `3dcad8e3c38e7091ae3771bf63020027f91715ce` |

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

- `./askeleton --version` reports `ASkeleTon 1.0.0`.
- `LICENSE.txt` is present.
- `CITATION.cff` is present and has the final repository URL and DOI.
- `.zenodo.json` is present and has the final repository URL.
- No external-project folders or generated artifacts are staged for commit.
