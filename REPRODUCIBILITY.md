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

The script `scripts/run_publication_eval.py` reproduces the Chapter 4
applicability evaluation described in the publication article.

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
analysis/publication_eval_<timestamp>/
```

including:

- `raw_runs.csv`
- `baseline_summary.csv`
- `coverage_ablation.csv`
- `full_factorial_sensitivity.csv`
- `skip_reason_summary.csv`
- `paper_ready_tables.md`
- `run_metadata.json`
- `viewer.html` when `--build-viewer` is used
- per-run reports, logs, and generated outputs under `<subject>/`

The following repository files are part of the official publication
reproducibility package:

- `scripts/run_publication_eval.py`
- `scripts/reproduce_publication.sh`
- `scripts/build_publication_viewer.py`

In addition, `figures/askeleton-workflow.svg` and
`figures/askeleton-workflow.drawio` are kept as source assets for the workflow
diagram used to explain the tool usage in the paper and repository materials.

Run the complete campaign:

```bash
python3 scripts/run_publication_eval.py --prepare-subjects --build-viewer
```

Or use the one-command wrapper that builds ASkeleTon first:

```bash
./scripts/reproduce_publication.sh
```

Use `--out-dir` to choose an explicit output directory:

```bash
python3 scripts/run_publication_eval.py \
  --prepare-subjects \
  --build-viewer \
  --out-dir analysis/publication_eval_full
```

Run a subset for a quick check:

```bash
python3 scripts/run_publication_eval.py \
  --subjects sut_showcase,openssl_ctype \
  --out-dir /tmp/askeleton_eval_subset \
  --build-viewer \
  --force
```

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

The generated tables match the values reported in the paper for:

- Table 4 applicability counts and generation rates
- Table 5 omission-category totals

The helper script `scripts/run_mode_matrix.py` is not part of the official
publication reproduction path described here. It can still be useful for
exploratory local analyses, but the paper-aligned entry points are the three
scripts listed above.

The script also updates:

- `analysis/publication_eval_latest` as a symlink to the latest campaign output

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
