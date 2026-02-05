#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP_DIR="/tmp/ask_sanity"

rm -rf "${TMP_DIR}"
mkdir -p "${TMP_DIR}"

cat > "${TMP_DIR}/sample.cpp" <<'EOF'
#include <cstdint>
#include <cstddef>
#include <vector>

int64_t add_i64(int64_t a, int64_t b) { return a + b; }
uint64_t add_u64(uint64_t a, uint64_t b) { return a + b; }
size_t next_size(size_t a) { return a + 1; }
std::vector<int> dup_vec(std::vector<int> v) { v.push_back(1); return v; }
EOF

cat > "${TMP_DIR}/compile_commands.json" <<EOF
[
  {
    "directory": "${TMP_DIR}",
    "command": "clang++-18 -std=c++20 -c ${TMP_DIR}/sample.cpp",
    "file": "${TMP_DIR}/sample.cpp"
  }
]
EOF

ASKELETON_HOME="${ROOT_DIR}" "${ROOT_DIR}/askeleton" -p "${TMP_DIR}" "${TMP_DIR}/sample.cpp"
echo "Sanity run completed. Output in ${ROOT_DIR}/Generated/UT/sample"
