#!/usr/bin/env bash
# Local release readiness checks for the ASkeleTon reproducible artifact.
# Run this from a working tree to validate repository metadata, docs, and
# the reproducibility workflows expected for the release/publication artifact.
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT"

FAIL=0

section() {
  echo
  echo "============================================================"
  echo "$1"
  echo "============================================================"
}

warn() {
  echo "WARNING: $*"
}

fail() {
  echo "ERROR: $*"
  FAIL=1
}

require_file() {
  local f="$1"
  [[ -f "$f" ]] || fail "Missing required file: $f"
}

section "1. Repository sanity"

require_file "README.md"
require_file "REPRODUCIBILITY.md"
require_file ".gitignore"
require_file "scripts/check_main_workflow.sh"

LICENSE_FILE=""
if [[ -f "LICENSE" ]]; then
  LICENSE_FILE="LICENSE"
elif [[ -f "LICENSE.txt" ]]; then
  LICENSE_FILE="LICENSE.txt"
else
  fail "Missing required license file: LICENSE or LICENSE.txt"
fi

if ! grep -Eq "Apache License" "$LICENSE_FILE" || ! grep -Eq "Version 2\.0" "$LICENSE_FILE"; then
  fail "$LICENSE_FILE does not appear to be Apache License 2.0"
else
  echo "License check passed: $LICENSE_FILE"
fi

if [[ ! -x scripts/check_main_workflow.sh ]]; then
  warn "scripts/check_main_workflow.sh is not executable. Fix with: chmod +x scripts/check_main_workflow.sh"
fi

echo "Git status:"
git status --short

UNTRACKED="$(git ls-files --others --exclude-standard)"
if [[ -n "$UNTRACKED" ]]; then
  fail "There are untracked non-ignored files:
$UNTRACKED"
fi

section "2. .gitignore safety"

if grep -Eq '^\*\.zip$|^doc/\*\.svg$|^\*\.svg$' .gitignore; then
  fail ".gitignore contains a broad pattern that may hide legitimate artifacts. Review .gitignore."
else
  echo ".gitignore broad-pattern check passed."
fi

if ! grep -Eq 'ASkeleTon.*\.zip|_.*ASkeleTon.*\.zip|\*ASkeleTon\*\.zip|manuscript.*\.zip|publication.*\.zip' .gitignore; then
  warn ".gitignore may not explicitly ignore local manuscript/publication ZIPs."
fi

section "3. Documentation wording / overclaim scan"

OVERCLAIMS=(
  "fully reproduces the paper"
  "complete evaluation"
  "deterministic in all cases"
  "all frameworks are validated"
  "exact manuscript example"
  "full .* claimed by the paper"
)

for file in README.md REPRODUCIBILITY.md examples/README.md; do
  [[ -f "$file" ]] || continue
  echo "Checking $file"
  for phrase in "${OVERCLAIMS[@]}"; do
    if grep -Eiq "$phrase" "$file"; then
      warn "Potential overclaim in $file matching: $phrase"
    fi
  done
done

if ! grep -Eiq "minimal.*end-to-end|minimal.*reproducible|main reproducibility workflow" README.md REPRODUCIBILITY.md; then
  warn "Docs may not clearly describe the workflow as a minimal reproducible workflow."
fi

section "4. Dependency wording checks"

if ! grep -Eiq "libgtest-dev|GoogleTest.*required|GTest.*required" REPRODUCIBILITY.md README.md; then
  fail "Docs do not clearly state that GoogleTest/libgtest-dev is required for the main reproducibility workflow."
fi

if ! grep -Eiq "Boost.*optional|Catch2.*optional|optional.*Boost|optional.*Catch2" REPRODUCIBILITY.md README.md; then
  warn "Docs may not clearly state that Boost.Test and Catch2 are optional backend dependencies."
fi

section "5. check_main_workflow.sh safety checks"

if ! grep -q "set -euo pipefail" scripts/check_main_workflow.sh; then
  fail "scripts/check_main_workflow.sh should use: set -euo pipefail"
fi

if grep -Eq 'rm -rf "\$\{?[A-Za-z_][A-Za-z0-9_]*\}?"|rm -rf \$[A-Za-z_][A-Za-z0-9_]*' scripts/check_main_workflow.sh; then
  warn "scripts/check_main_workflow.sh contains rm -rf on a variable. Review manually for safeguards."
fi

if ! grep -Eq -- "--profile|--coverage-mode|--oracle-mode|--report|--log-json" scripts/check_main_workflow.sh; then
  fail "scripts/check_main_workflow.sh does not appear to pass generation/report options explicitly."
fi

section "6. Build"

make clean
make CXX=clang++-18

section "7. Version"

./askeleton --version || fail "./askeleton --version failed"

section "8. Main reproducibility workflow"

TMP_MAIN="/tmp/askeleton_main_workflow_pre_release"
rm -rf "$TMP_MAIN"
./scripts/check_main_workflow.sh "$TMP_MAIN"

section "9. Quick showcase evaluation"

TMP_EVAL="/tmp/askeleton_eval_subset_pre_release"
rm -rf "$TMP_EVAL"
python3 scripts/run_eval.py \
  --subjects sut_showcase \
  --out-dir "$TMP_EVAL" \
  --build-viewer \
  --force

section "10. Check repository cleanliness after scripts"

echo "Git status after verification:"
git status --short

DIRTY="$(git status --short)"
if [[ -n "$DIRTY" ]]; then
  warn "Working tree has modifications. This is expected if you have staged source changes, but there should be no new generated files."
fi

UNTRACKED_AFTER="$(git ls-files --others --exclude-standard)"
if [[ -n "$UNTRACKED_AFTER" ]]; then
  fail "Verification created untracked non-ignored files:
$UNTRACKED_AFTER"
fi

section "11. Summary"

if [[ "$FAIL" -ne 0 ]]; then
  echo "Pre-release check FAILED. Fix the errors above before commit/tag."
  exit 1
fi

echo "Pre-release check PASSED."
echo
echo "Suggested next commands:"
echo "  git diff --stat HEAD"
echo "  git diff -- README.md REPRODUCIBILITY.md CITATION.cff .zenodo.json include/constants.hpp scripts/pre_release_check.sh"
echo "  git add README.md REPRODUCIBILITY.md CITATION.cff .zenodo.json include/constants.hpp scripts/pre_release_check.sh"
echo "  git commit -m \"Prepare ASkeleTon v1.0.1 release\""
echo
echo "After CI passes:"
echo "  git tag -a v1.0.1 -m \"ASkeleTon v1.0.1 release/publication artifact\""
echo "  git push origin main"
echo "  git push origin v1.0.1"
