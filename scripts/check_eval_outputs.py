#!/usr/bin/env python3
"""Validate ASkeleTon evaluation output tables."""

from __future__ import annotations

import argparse
import csv
import sys
from collections import Counter
from pathlib import Path


EXPECTED_FILES = [
    "raw_runs.csv",
    "baseline_summary.csv",
    "coverage_ablation.csv",
    "full_factorial_sensitivity.csv",
    "skip_reason_summary.csv",
    "evaluation_tables.md",
    "run_metadata.json",
]

REQUIRED_RAW_COLUMNS = [
    "run_id",
    "subject",
    "profile",
    "coverage_mode",
    "oracle_mode",
    "rule_data",
    "found",
    "generated",
    "skipped",
    "generation_rate",
    "askeleton_exit_code",
    "generation_success",
    "build_attempted",
    "build_success",
    "execution_attempted",
    "execution_success",
    "report_path",
    "log_path",
    "generated_output_path",
]


def load_rows(path: Path) -> tuple[list[dict[str, str]], list[str]]:
    with path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        rows = list(reader)
        return rows, list(reader.fieldnames or [])


def is_true(value: str) -> bool:
    return value.strip().lower() == "true"


def count_true(rows: list[dict[str, str]], column: str) -> int:
    return sum(1 for row in rows if is_true(row.get(column, "")))


def count_attempt_success(rows: list[dict[str, str]], attempted: str,
                          success: str) -> tuple[int, int]:
    attempted_rows = [row for row in rows if is_true(row.get(attempted, ""))]
    success_count = sum(1 for row in attempted_rows if is_true(row.get(success, "")))
    return len(attempted_rows), success_count


def format_attempt_summary(label: str, attempted: int, success: int) -> str:
    if attempted == 0:
        return f"{label}: not attempted"
    return f"{label}: attempted={attempted} success={success}"


def missing_paths(rows: list[dict[str, str]], column: str) -> list[str]:
    missing = []
    for row in rows:
        value = row.get(column, "")
        if not value:
            missing.append(f"{row.get('run_id', '<unknown>')}: <empty>")
            continue
        if not Path(value).exists():
            missing.append(f"{row.get('run_id', '<unknown>')}: {value}")
    return missing


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output_dir", type=Path, help="Evaluation output directory")
    args = parser.parse_args(argv)

    out_dir = args.output_dir.resolve()
    errors: list[str] = []

    if not out_dir.exists():
        errors.append(f"missing output directory: {out_dir}")
    elif not out_dir.is_dir():
        errors.append(f"not a directory: {out_dir}")

    for name in EXPECTED_FILES:
        path = out_dir / name
        if not path.exists():
            errors.append(f"missing expected file: {path}")

    raw_path = out_dir / "raw_runs.csv"
    rows: list[dict[str, str]] = []
    fieldnames: list[str] = []
    if raw_path.exists():
        rows, fieldnames = load_rows(raw_path)
        missing_columns = [
            column for column in REQUIRED_RAW_COLUMNS if column not in fieldnames
        ]
        for column in missing_columns:
            errors.append(f"missing raw_runs.csv column: {column}")

    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    subjects = sorted({row.get("subject", "") for row in rows if row.get("subject")})
    exit_codes = Counter(row.get("askeleton_exit_code", "") for row in rows)
    generation_success = count_true(rows, "generation_success")
    build_attempted, build_success = count_attempt_success(
        rows, "build_attempted", "build_success"
    )
    execution_attempted, execution_success = count_attempt_success(
        rows, "execution_attempted", "execution_success"
    )
    missing_reports = missing_paths(rows, "report_path")
    missing_logs = missing_paths(rows, "log_path")

    print(f"Evaluation output: {out_dir}")
    print(f"Rows: {len(rows)}")
    print(f"Subjects: {', '.join(subjects) if subjects else '<none>'}")
    print(
        "ASkeleTon exit codes: "
        + ", ".join(f"{code or '<empty>'}={count}" for code, count in sorted(exit_codes.items()))
    )
    print(
        f"Generation: success={generation_success} "
        f"failure={len(rows) - generation_success}"
    )
    print(format_attempt_summary("Build", build_attempted, build_success))
    print(format_attempt_summary("Execution", execution_attempted, execution_success))
    print(f"Missing reports: {len(missing_reports)}")
    for item in missing_reports[:10]:
        print(f"  {item}")
    if len(missing_reports) > 10:
        print(f"  ... {len(missing_reports) - 10} more")
    print(f"Missing logs: {len(missing_logs)}")
    for item in missing_logs[:10]:
        print(f"  {item}")
    if len(missing_logs) > 10:
        print(f"  ... {len(missing_logs) - 10} more")

    if missing_reports or missing_logs:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
