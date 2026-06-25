#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export ASKELETON_HOME="${ASKELETON_HOME:-$ROOT_DIR}"

echo "ASKELETON_HOME=$ASKELETON_HOME"

echo "== Minimal case =="
mkdir -p /tmp/ask_sut
cat > /tmp/ask_sut/sut.cpp <<'EOF'
int foo(int a){ return a + 1; }
EOF
cat > /tmp/ask_sut/compile_commands.json <<'EOF'
[
  {
    "directory": "/tmp/ask_sut",
    "command": "clang++-18 -std=c++20 -c /tmp/ask_sut/sut.cpp",
    "file": "/tmp/ask_sut/sut.cpp"
  }
]
EOF
"$ASKELETON_HOME/askeleton" -p /tmp/ask_sut /tmp/ask_sut/sut.cpp

echo "== Examples baseline =="
"$ASKELETON_HOME/askeleton" --bootstrap-compdb -p "$ASKELETON_HOME/examples" "$ASKELETON_HOME/examples/sut.cpp"

echo "== Rule data explicit parity =="
"$ASKELETON_HOME/askeleton" --bootstrap-compdb --rule-data --rule-max-cases=3 -p "$ASKELETON_HOME/examples" "$ASKELETON_HOME/examples/sut.cpp"

echo "== Profiles =="
"$ASKELETON_HOME/askeleton" --bootstrap-compdb --profile=boundary -p "$ASKELETON_HOME/examples" "$ASKELETON_HOME/examples/sut.cpp"
"$ASKELETON_HOME/askeleton" --bootstrap-compdb --profile=safe -p "$ASKELETON_HOME/examples" "$ASKELETON_HOME/examples/sut.cpp"
"$ASKELETON_HOME/askeleton" --bootstrap-compdb --profile=stress -p "$ASKELETON_HOME/examples" "$ASKELETON_HOME/examples/sut.cpp"

echo "== Showcase diagnostics =="
SHOWCASE_OUT="/tmp/askeleton_showcase_check"
rm -rf "$SHOWCASE_OUT"
"$ASKELETON_HOME/askeleton" \
  --bootstrap-compdb \
  -p "$ASKELETON_HOME/examples" \
  --framework=gtest \
  --profile=random \
  --coverage-mode=balanced \
  --oracle-mode=explicit \
  --seed=123 \
  --report="$SHOWCASE_OUT/report.json" \
  --out-dir="$SHOWCASE_OUT/generated" \
  "$ASKELETON_HOME/examples/sut_showcase.cpp"
grep -q '"found": 24' "$SHOWCASE_OUT/report.json"
grep -q '"generated": 16' "$SHOWCASE_OUT/report.json"
grep -q '"skipped": 8' "$SHOWCASE_OUT/report.json"
grep -q '"unusable_constructor": 3' "$SHOWCASE_OUT/report.json"

echo "All checks completed."
