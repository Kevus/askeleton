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
    "command": "clang++-15 -std=c++20 -c /tmp/ask_sut/sut.cpp",
    "file": "/tmp/ask_sut/sut.cpp"
  }
]
EOF
"$ASKELETON_HOME/askeleton" -p /tmp/ask_sut /tmp/ask_sut/sut.cpp

echo "== Examples baseline =="
"$ASKELETON_HOME/askeleton" -p "$ASKELETON_HOME/examples" "$ASKELETON_HOME/examples/sut.cpp"

echo "== Rule data =="
"$ASKELETON_HOME/askeleton" --rule-data --rule-max-cases=3 -p "$ASKELETON_HOME/examples" "$ASKELETON_HOME/examples/sut.cpp"

echo "== Profiles =="
"$ASKELETON_HOME/askeleton" --profile=boundary -p "$ASKELETON_HOME/examples" "$ASKELETON_HOME/examples/sut.cpp"
"$ASKELETON_HOME/askeleton" --profile=safe -p "$ASKELETON_HOME/examples" "$ASKELETON_HOME/examples/sut.cpp"
"$ASKELETON_HOME/askeleton" --profile=stress -p "$ASKELETON_HOME/examples" "$ASKELETON_HOME/examples/sut.cpp"

echo "All checks completed."
