#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export ASKELETON_HOME="${ASKELETON_HOME:-$ROOT_DIR}"

OUT_DIR="${1:-$(mktemp -d /tmp/askeleton_main_workflow.XXXXXX)}"
GENERATED_DIR="$OUT_DIR/generated"
TARGET_DIR="$GENERATED_DIR/sut"
REPORT_PATH="$OUT_DIR/report.json"
LOG_PATH="$OUT_DIR/log.json"
FAIL_LOG="$OUT_DIR/cfg_override_failure.log"
EXAMPLES_COMPDB="$ASKELETON_HOME/examples/compile_commands.json"
EXAMPLES_COMPDB_BAK="$EXAMPLES_COMPDB.bak"
SYSTEM_FILES_JSON="$ASKELETON_HOME/data/system_files.json"

HAD_EXAMPLES_COMPDB=0
HAD_EXAMPLES_COMPDB_BAK=0
HAD_SYSTEM_FILES_JSON=0

if [[ -e "$EXAMPLES_COMPDB" ]]; then
  HAD_EXAMPLES_COMPDB=1
fi
if [[ -e "$EXAMPLES_COMPDB_BAK" ]]; then
  HAD_EXAMPLES_COMPDB_BAK=1
fi
if [[ -e "$SYSTEM_FILES_JSON" ]]; then
  HAD_SYSTEM_FILES_JSON=1
fi

normalize_path() {
  realpath -m -- "$1"
}

assert_safe_output_dir() {
  local candidate="$1"
  local resolved_root
  local resolved_home
  local resolved_candidate

  if [[ -z "$candidate" ]]; then
    echo "ERROR: refusing to remove an empty output path" >&2
    exit 1
  fi

  resolved_root="$(normalize_path "$ROOT_DIR")"
  resolved_home="$(normalize_path "$HOME")"
  resolved_candidate="$(normalize_path "$candidate")"

  if [[ "$resolved_candidate" == "/" ]]; then
    echo "ERROR: refusing to remove /" >&2
    exit 1
  fi

  if [[ "$resolved_candidate" == "$resolved_home" ]]; then
    echo "ERROR: refusing to remove \$HOME" >&2
    exit 1
  fi

  if [[ "$resolved_candidate" == "$resolved_root" ]]; then
    echo "ERROR: refusing to remove the repository root" >&2
    exit 1
  fi

  case "$resolved_candidate" in
    /tmp/askeleton_*) ;;
    *)
      echo "ERROR: refusing to remove output outside /tmp/askeleton_: $resolved_candidate" >&2
      exit 1
      ;;
  esac
}

cleanup() {
  if [[ $HAD_EXAMPLES_COMPDB -eq 0 ]]; then
    rm -f "$EXAMPLES_COMPDB"
  fi
  if [[ $HAD_EXAMPLES_COMPDB_BAK -eq 0 ]]; then
    rm -f "$EXAMPLES_COMPDB_BAK"
  fi
  if [[ $HAD_SYSTEM_FILES_JSON -eq 0 ]]; then
    rm -f "$SYSTEM_FILES_JSON"
  fi
}

trap cleanup EXIT

echo "ASKELETON_HOME=$ASKELETON_HOME"
echo "Workflow output: $OUT_DIR"

assert_safe_output_dir "$OUT_DIR"

mkdir -p "$OUT_DIR"
rm -r -f -- "$GENERATED_DIR"
rm -f -- "$REPORT_PATH" "$LOG_PATH" "$FAIL_LOG"

"$ASKELETON_HOME/askeleton" \
  --bootstrap-compdb \
  -p "$ASKELETON_HOME/examples" \
  --framework=gtest \
  --profile=random \
  --coverage-mode=balanced \
  --oracle-mode=explicit \
  --seed=123 \
  --report="$REPORT_PATH" \
  --log-json="$LOG_PATH" \
  --out-dir="$GENERATED_DIR" \
  "$ASKELETON_HOME/examples/sut.cpp"

make -C "$TARGET_DIR"

(
  cd "$TARGET_DIR"
  ./sut_test --gtest_filter=Fixture.sut_add_i64_1
)

grep -q '"framework": "gtest"' "$REPORT_PATH"
grep -q '"oracle_mode": "explicit"' "$REPORT_PATH"
grep -q '"generated": 10' "$LOG_PATH"

sed -i '0,/};/s//\texpected=999;#int\n\n};/' "$TARGET_DIR/sut.cfg"

if "$TARGET_DIR/sut_test" --gtest_filter=Fixture.sut_add_i64_1 >"$FAIL_LOG" 2>&1; then
  echo "ERROR: edited sut.cfg did not affect the generated test result" >&2
  exit 1
fi

grep -q 'Which is: 999' "$FAIL_LOG"

echo "Main workflow check passed."
echo "  Generated tests: $TARGET_DIR"
echo "  Report JSON:      $REPORT_PATH"
echo "  Execution log:    $LOG_PATH"
