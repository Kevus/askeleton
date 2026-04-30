#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CXX_BIN="${CXX:-clang++-18}"

cd "$ROOT_DIR"
make CXX="$CXX_BIN"
python3 scripts/run_eval.py --prepare-subjects --build-viewer "$@"
