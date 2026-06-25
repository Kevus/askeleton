#!/usr/bin/env python3
"""Run the applicability evaluation for ASkeleTon."""

from __future__ import annotations

import argparse
import csv
import json
import os
import shutil
import subprocess
import sys
from collections import Counter
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path


FRAMEWORK = "gtest"
SEED = 123
RULE_MAX_CASES = 3
PROFILES = ["random", "boundary", "safe", "stress"]
COVERAGE_MODES = ["strict", "balanced", "aggressive"]
ORACLE_MODES = ["mirror", "explicit", "property"]
RULE_DATA_LEVELS = ["off", "on"]


@dataclass(frozen=True)
class ExternalSubject:
    key: str
    repo: str
    commit: str
    path: str
    description: str


@dataclass(frozen=True)
class Subject:
    key: str
    label: str
    subject_group: str
    experiment_plan: str
    source: str
    notes: str


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def run(command: list[str], cwd: Path, *, env: dict | None = None,
        capture_output: bool = False, check: bool = True) -> subprocess.CompletedProcess:
    return subprocess.run(
        command,
        cwd=cwd,
        env=env,
        text=True,
        capture_output=capture_output,
        check=check,
    )


def run_shell(command: str, cwd: Path, *, env: dict | None = None) -> None:
    run(["bash", "-lc", command], cwd, env=env, check=True)


def write_json(path: Path, payload) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def write_csv(path: Path, fieldnames: list[str], rows: list[dict]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for row in rows:
            writer.writerow(row)


def percent(value: float | None) -> str:
    if value is None:
        return ""
    return f"{value * 100:.2f}%"


def parse_float(value: str) -> float | None:
    if value == "":
        return None
    try:
        return float(value)
    except ValueError:
        return None


def bool_text(value: bool) -> str:
    return "true" if value else "false"


def external_subjects(root: Path) -> dict[str, ExternalSubject]:
    return {
        "tinyxml2": ExternalSubject(
            key="tinyxml2",
            repo="https://github.com/leethomason/tinyxml2.git",
            commit="3324d04d58de9d5db09327db6442f075e519f11b",
            path=str(root / "examples" / "tinyxml2"),
            description="tinyxml2 snapshot used for Chapter 4.",
        ),
        "sqlite": ExternalSubject(
            key="sqlite",
            repo="https://github.com/sqlite/sqlite",
            commit="c739d132175932cde2c7c2f38d625165991f2a5d",
            path=str(root / "examples" / "sqlite"),
            description="SQLite snapshot used for Chapter 4.",
        ),
        "openssl": ExternalSubject(
            key="openssl",
            repo="https://github.com/openssl/openssl",
            commit="0ed06337e38ec70e5beb043d5a1da9a6b6e8c57e",
            path=str(root / "examples" / "openssl"),
            description="OpenSSL snapshot used for Chapter 4.",
        ),
        "yamlcpp": ExternalSubject(
            key="yamlcpp",
            repo="https://github.com/jbeder/yaml-cpp",
            commit="05c050c6c14d5c3a82cbc368b50d985896922196",
            path=str(root / "examples" / "yaml-cpp"),
            description="yaml-cpp snapshot used for Chapter 4.",
        ),
    }


def evaluation_subjects() -> list[Subject]:
    return [
        Subject(
            key="sut_showcase",
            label="Illustrative showcase",
            subject_group="full_factorial",
            experiment_plan="full_factorial",
            source="examples/sut_showcase.cpp",
            notes="Controlled example with intended skips.",
        ),
        Subject(
            key="tinyxml2_core",
            label="tinyxml2 core",
            subject_group="full_factorial",
            experiment_plan="full_factorial",
            source="examples/tinyxml2/tinyxml2.cpp",
            notes="Real library TU with broad method surface.",
        ),
        Subject(
            key="tinyxml2_xmltest",
            label="tinyxml2 xmltest",
            subject_group="base_plus_coverage",
            experiment_plan="base_plus_coverage",
            source="examples/tinyxml2/xmltest.cpp",
            notes="Real program TU with high success rate.",
        ),
        Subject(
            key="yamlcpp_emitter",
            label="yaml-cpp emitter",
            subject_group="base_plus_coverage",
            experiment_plan="base_plus_coverage",
            source="examples/yaml-cpp/src/emitter.cpp",
            notes="Real C++ TU with richer type shapes.",
        ),
        Subject(
            key="openssl_ctype",
            label="OpenSSL ctype",
            subject_group="base_plus_coverage",
            experiment_plan="base_plus_coverage",
            source="examples/openssl/crypto/ctype.c",
            notes="Compact real C TU with full generation.",
        ),
        Subject(
            key="sqlite_util",
            label="SQLite util",
            subject_group="base_plus_coverage",
            experiment_plan="base_plus_coverage",
            source="examples/sqlite/src/util.c",
            notes="Real SQLite core TU with internal utility routines and moderate pointer-heavy skips.",
        ),
    ]


def select_subjects(subjects: list[Subject], selector: str) -> list[Subject]:
    if selector == "all":
        return subjects

    requested = {item.strip() for item in selector.split(",") if item.strip()}
    unknown = sorted(requested.difference(subject.key for subject in subjects))
    if unknown:
        raise SystemExit(f"Unknown subject(s): {', '.join(unknown)}")
    return [subject for subject in subjects if subject.key in requested]


def required_external_subject_keys(subjects: list[Subject]) -> set[str]:
    mapping = {
        "tinyxml2_core": "tinyxml2",
        "tinyxml2_xmltest": "tinyxml2",
        "yamlcpp_emitter": "yamlcpp",
        "openssl_ctype": "openssl",
        "sqlite_util": "sqlite",
    }
    return {mapping[subject.key] for subject in subjects if subject.key in mapping}


def clone_or_checkout(subject: ExternalSubject, *, prepare: bool) -> None:
    path = Path(subject.path)
    if not path.exists():
        if not prepare:
            raise SystemExit(
                f"Missing external subject '{subject.key}' at {path}.\n"
                "Re-run with --prepare-subjects to clone the recorded snapshot."
            )
        path.parent.mkdir(parents=True, exist_ok=True)
        run(["git", "clone", subject.repo, str(path)], cwd=path.parent)

    head = run(["git", "rev-parse", "HEAD"], cwd=path, capture_output=True).stdout.strip()
    if head == subject.commit:
        return

    if not prepare:
        raise SystemExit(
            f"External subject '{subject.key}' is at {head}, expected {subject.commit}.\n"
            "Re-run with --prepare-subjects to checkout the recorded snapshot."
        )

    run(["git", "fetch", "--all", "--tags"], cwd=path)
    run(["git", "checkout", subject.commit], cwd=path)


def ensure_tinyxml2_compile_db(root: Path) -> None:
    source = root / "examples" / "tinyxml2"
    build = source / "build"
    if (build / "compile_commands.json").exists():
        return
    run(
        [
            "cmake",
            "-S",
            str(source),
            "-B",
            str(build),
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        ],
        cwd=root,
    )


def ensure_yamlcpp_compile_db(root: Path) -> None:
    source = root / "examples" / "yaml-cpp"
    build = source / "build"
    if (build / "compile_commands.json").exists():
        return
    run(
        [
            "cmake",
            "-S",
            str(source),
            "-B",
            str(build),
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
            "-DYAML_CPP_BUILD_TESTS=OFF",
            "-DYAML_CPP_BUILD_TOOLS=OFF",
        ],
        cwd=root,
    )


def prepare_sqlite(sqlite_dir: Path) -> None:
    if not (sqlite_dir / "sqlite3.h").exists():
        run_shell("./configure >/tmp/askeleton_sqlite_configure.log 2>&1", sqlite_dir)
        run_shell("make sqlite3.h >/tmp/askeleton_sqlite_make.log 2>&1", sqlite_dir)

    opcodes_h = sqlite_dir / "opcodes.h"
    src_opcodes_h = sqlite_dir / "src" / "opcodes.h"
    if not opcodes_h.exists() and (sqlite_dir / "parse.h").exists():
        run_shell("cat parse.h src/vdbe.c | tclsh tool/mkopcodeh.tcl > opcodes.h", sqlite_dir)
    if opcodes_h.exists() and not src_opcodes_h.exists():
        shutil.copyfile(opcodes_h, src_opcodes_h)


def prepare_openssl(openssl_dir: Path) -> None:
    configuration_h = openssl_dir / "include" / "openssl" / "configuration.h"
    opensslv_h = openssl_dir / "include" / "openssl" / "opensslv.h"
    if not configuration_h.exists():
        run_shell("perl ./Configure >/tmp/askeleton_openssl_configure.log 2>&1", openssl_dir)
    if not opensslv_h.exists():
        run_shell("make include/openssl/opensslv.h >/tmp/askeleton_openssl_make.log 2>&1", openssl_dir)


def ensure_subject_inputs(root: Path, subjects: list[Subject], *, prepare_subjects: bool) -> None:
    externals = external_subjects(root)
    required = required_external_subject_keys(subjects)
    for key in sorted(required):
        clone_or_checkout(externals[key], prepare=prepare_subjects)

    if "tinyxml2" in required:
        ensure_tinyxml2_compile_db(root)
    if "yamlcpp" in required:
        ensure_yamlcpp_compile_db(root)
    if "sqlite" in required:
        prepare_sqlite(root / "examples" / "sqlite")
    if "openssl" in required:
        prepare_openssl(root / "examples" / "openssl")


def create_compdb_dir(path: Path, entry: dict) -> Path:
    path.mkdir(parents=True, exist_ok=True)
    write_json(path / "compile_commands.json", [entry])
    return path


def prepare_compdbs(root: Path, out_dir: Path, subjects: list[Subject]) -> dict[str, Path]:
    compdb_root = out_dir / "_compdb"
    examples_dir = root / "examples"
    subject_keys = {subject.key for subject in subjects}
    compdbs = {}
    if "sut_showcase" in subject_keys:
        compdbs["sut_showcase"] = create_compdb_dir(
            compdb_root / "examples",
            {
                "directory": str(examples_dir),
                "command": f"clang++-18 -std=c++20 -I{examples_dir} -c {examples_dir / 'sut_showcase.cpp'}",
                "file": str(examples_dir / "sut_showcase.cpp"),
            },
        )
    if "openssl_ctype" in subject_keys:
        compdbs["openssl_ctype"] = create_compdb_dir(
            compdb_root / "openssl",
            {
                "directory": str(root / "examples" / "openssl"),
                "command": (
                    f"clang-18 -std=c11 -I{root / 'examples' / 'openssl'} "
                    f"-I{root / 'examples' / 'openssl' / 'include'} "
                    f"-c {root / 'examples' / 'openssl' / 'crypto' / 'ctype.c'}"
                ),
                "file": str(root / "examples" / "openssl" / "crypto" / "ctype.c"),
            },
        )
    if "sqlite_util" in subject_keys:
        compdbs["sqlite_util"] = create_compdb_dir(
            compdb_root / "sqlite",
            {
                "directory": str(root / "examples" / "sqlite"),
                "command": (
                    f"clang-18 -std=c11 -I{root / 'examples' / 'sqlite'} "
                    f"-I{root / 'examples' / 'sqlite' / 'src'} "
                    f"-c {root / 'examples' / 'sqlite' / 'src' / 'util.c'}"
                ),
                "file": str(root / "examples" / "sqlite" / "src" / "util.c"),
            },
        )
    return compdbs


def resolve_build_path(root: Path, out_dir: Path, subject: Subject, compdbs: dict[str, Path]) -> Path:
    if subject.key in compdbs:
        return compdbs[subject.key]
    if subject.key in {"tinyxml2_core", "tinyxml2_xmltest"}:
        return root / "examples" / "tinyxml2" / "build"
    if subject.key == "yamlcpp_emitter":
        return root / "examples" / "yaml-cpp" / "build"
    raise KeyError(subject.key)


def full_factorial_runs(subject: Subject) -> list[dict]:
    runs = []
    for profile in PROFILES:
        for coverage_mode in COVERAGE_MODES:
            for oracle_mode in ORACLE_MODES:
                for rule_data in RULE_DATA_LEVELS:
                    runs.append(
                        {
                            "run_id": (
                                f"{subject.key}__full_factorial__profile-{profile}"
                                f"__coverage-{coverage_mode}"
                                f"__oracle-{oracle_mode}"
                                f"__rule-{rule_data}"
                            ),
                            "experiment_group": "full_factorial",
                            "profile": profile,
                            "coverage_mode": coverage_mode,
                            "oracle_mode": oracle_mode,
                            "rule_data": rule_data,
                        }
                    )
    return runs


def base_plus_coverage_runs(subject: Subject) -> list[dict]:
    return [
        {
            "run_id": f"{subject.key}__base__profile-random__coverage-balanced__oracle-explicit__rule-on",
            "experiment_group": "base",
            "profile": "random",
            "coverage_mode": "balanced",
            "oracle_mode": "explicit",
            "rule_data": "on",
        },
        {
            "run_id": f"{subject.key}__coverage_ablation__profile-random__coverage-strict__oracle-explicit__rule-on",
            "experiment_group": "coverage_ablation",
            "profile": "random",
            "coverage_mode": "strict",
            "oracle_mode": "explicit",
            "rule_data": "on",
        },
        {
            "run_id": f"{subject.key}__coverage_ablation__profile-random__coverage-aggressive__oracle-explicit__rule-on",
            "experiment_group": "coverage_ablation",
            "profile": "random",
            "coverage_mode": "aggressive",
            "oracle_mode": "explicit",
            "rule_data": "on",
        },
    ]


def planned_runs(subject: Subject) -> list[dict]:
    if subject.experiment_plan == "full_factorial":
        return full_factorial_runs(subject)
    if subject.experiment_plan == "base_plus_coverage":
        return base_plus_coverage_runs(subject)
    raise KeyError(subject.experiment_plan)


def parse_report(path: Path) -> dict | None:
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def run_one(root: Path, subject: Subject, build_path: Path, source_path: Path, out_dir: Path,
            plan: dict, total_index: int, total_runs: int) -> dict:
    print(f"[{total_index}/{total_runs}] {plan['run_id']}", flush=True)

    subject_dir = out_dir / subject.key
    report_path = subject_dir / "reports" / f"{plan['run_id']}.json"
    log_path = subject_dir / "logs" / f"{plan['run_id']}.log"
    generated_dir = subject_dir / "generated" / plan["run_id"]
    report_path.parent.mkdir(parents=True, exist_ok=True)
    log_path.parent.mkdir(parents=True, exist_ok=True)
    generated_dir.parent.mkdir(parents=True, exist_ok=True)

    command = [
        str(root / "askeleton"),
        "-p",
        str(build_path),
        "--framework",
        FRAMEWORK,
        "--seed",
        str(SEED),
        "--profile",
        plan["profile"],
        "--coverage-mode",
        plan["coverage_mode"],
        "--oracle-mode",
        plan["oracle_mode"],
        "--report",
        str(report_path),
        "--out-dir",
        str(generated_dir),
        str(source_path),
    ]
    if plan["rule_data"] == "on":
        command.extend(["--rule-data", "--rule-max-cases", str(RULE_MAX_CASES)])
    else:
        command.append("--no-rule-data")

    completed = run(
        command,
        cwd=root,
        env={**os.environ, "ASKELETON_HOME": str(root)},
        capture_output=True,
        check=False,
    )
    log_path.write_text(completed.stdout + completed.stderr, encoding="utf-8")

    report = parse_report(report_path) or {}
    summary = report.get("summary", {})
    coverage = summary.get("coverage", {})
    generation_success = completed.returncode == 0 and report_path.exists()
    row = {
        "run_id": plan["run_id"],
        "experiment_group": plan["experiment_group"],
        "subject": subject.key,
        "label": subject.label,
        "subject_group": subject.subject_group,
        "notes": subject.notes,
        "framework": FRAMEWORK,
        "seed": str(SEED),
        "profile": plan["profile"],
        "coverage_mode": plan["coverage_mode"],
        "oracle_mode": plan["oracle_mode"],
        "rule_data": plan["rule_data"],
        "rule_max_cases": str(RULE_MAX_CASES) if plan["rule_data"] == "on" else "",
        "exit_code": str(completed.returncode),
        "askeleton_exit_code": str(completed.returncode),
        "generation_success": bool_text(generation_success),
        "build_attempted": "false",
        "build_success": "",
        "execution_attempted": "false",
        "execution_success": "",
        "found": str(coverage.get("found", "")),
        "generated": str(coverage.get("generated", "")),
        "skipped": str(coverage.get("skipped", "")),
        "generation_rate": "" if coverage.get("generation_rate") is None else str(coverage.get("generation_rate")),
        "skip_rate": "" if coverage.get("skip_rate") is None else str(coverage.get("skip_rate")),
        "by_reason_json": json.dumps(summary.get("by_reason", {}), sort_keys=True),
        "by_kind_json": json.dumps(summary.get("by_kind", {}), sort_keys=True),
        "report_path": str(report_path),
        "log_path": str(log_path),
        "generated_dir": str(generated_dir),
        "generated_output_path": str(generated_dir),
    }
    return row


def row_lookup(rows: list[dict]) -> dict[tuple[str, str, str, str, str], dict]:
    lookup = {}
    for row in rows:
        lookup[
            (
                row["subject"],
                row["profile"],
                row["coverage_mode"],
                row["oracle_mode"],
                row["rule_data"],
            )
        ] = row
    return lookup


def baseline_summary_rows(subjects: list[Subject], rows: list[dict]) -> list[dict]:
    lookup = row_lookup(rows)
    result = []
    for subject in subjects:
        row = lookup[(subject.key, "random", "balanced", "explicit", "on")]
        result.append(
            {
                "subject": subject.key,
                "label": subject.label,
                "found": row["found"],
                "generated": row["generated"],
                "skipped": row["skipped"],
                "generation_rate": percent(parse_float(row["generation_rate"])),
            }
        )
    return result


def coverage_ablation_rows(subjects: list[Subject], rows: list[dict]) -> list[dict]:
    lookup = row_lookup(rows)
    result = []
    for subject in subjects:
        strict = lookup[(subject.key, "random", "strict", "explicit", "on")]
        balanced = lookup[(subject.key, "random", "balanced", "explicit", "on")]
        aggressive = lookup[(subject.key, "random", "aggressive", "explicit", "on")]
        result.append(
            {
                "subject": subject.key,
                "label": subject.label,
                "strict_rate": percent(parse_float(strict["generation_rate"])),
                "balanced_rate": percent(parse_float(balanced["generation_rate"])),
                "aggressive_rate": percent(parse_float(aggressive["generation_rate"])),
                "strict_generated": strict["generated"],
                "balanced_generated": balanced["generated"],
                "aggressive_generated": aggressive["generated"],
            }
        )
    return result


def full_factorial_sensitivity_rows(subjects: list[Subject], rows: list[dict]) -> list[dict]:
    result = []
    dimensions = [
        ("profile", PROFILES),
        ("coverage_mode", COVERAGE_MODES),
        ("oracle_mode", ORACLE_MODES),
        ("rule_data", RULE_DATA_LEVELS),
    ]

    for subject in subjects:
        subject_rows = [row for row in rows if row["subject"] == subject.key and row["experiment_group"] == "full_factorial"]
        for dimension, levels in dimensions:
            for level in levels:
                selected = [
                    value
                    for row in subject_rows
                    if row[dimension] == level
                    for value in [parse_float(row["generation_rate"])]
                    if value is not None
                ]
                if not selected:
                    result.append(
                        {
                            "subject": subject.key,
                            "dimension": dimension,
                            "level": level,
                            "avg_generation_rate": "",
                            "min_generation_rate": "",
                            "max_generation_rate": "",
                            "runs": "0",
                        }
                    )
                    continue
                result.append(
                    {
                        "subject": subject.key,
                        "dimension": dimension,
                        "level": level,
                        "avg_generation_rate": percent(sum(selected) / len(selected)),
                        "min_generation_rate": percent(min(selected)),
                        "max_generation_rate": percent(max(selected)),
                        "runs": str(len(selected)),
                    }
                )
    return result


def skip_reason_rows(rows: list[dict]) -> list[dict]:
    totals: Counter[str] = Counter()
    for row in rows:
        reasons = json.loads(row["by_reason_json"]) if row["by_reason_json"] else {}
        for reason, count in reasons.items():
            totals[reason] += int(count)
    return [{"reason": reason, "count": str(count)} for reason, count in totals.most_common()]


def practical_outcome_rows(subjects: list[Subject], rows: list[dict]) -> list[dict]:
    result = []
    for subject in subjects:
        subject_rows = [row for row in rows if row["subject"] == subject.key]
        build_attempted = [row for row in subject_rows if row.get("build_attempted") == "true"]
        build_success = [row for row in build_attempted if row.get("build_success") == "true"]
        execution_attempted = [row for row in subject_rows if row.get("execution_attempted") == "true"]
        execution_success = [row for row in execution_attempted if row.get("execution_success") == "true"]
        generation_success = [row for row in subject_rows if row.get("generation_success") == "true"]
        result.append(
            {
                "subject": subject.key,
                "label": subject.label,
                "runs": str(len(subject_rows)),
                "generation_success": str(len(generation_success)),
                "generation_failure": str(len(subject_rows) - len(generation_success)),
                "build_attempted": str(len(build_attempted)),
                "build_success": str(len(build_success)),
                "execution_attempted": str(len(execution_attempted)),
                "execution_success": str(len(execution_success)),
            }
        )
    return result


def write_evaluation_tables(out_dir: Path, baseline: list[dict], coverage: list[dict],
                            sensitivity: list[dict], skips: list[dict],
                            practical: list[dict]) -> None:
    lines = [
        "# Evaluation Summary",
        "",
        f"Raw data directory: `{out_dir}`",
        "",
        "## Baseline Configuration",
        "",
        f"- Framework: `{FRAMEWORK}`; seed: `{SEED}`; profile: `random`; coverage: `balanced`; oracle: `explicit`; rule-data: `on`.",
        "",
        "| Subject | Found | Generated | Skipped | Generation rate |",
        "| --- | ---: | ---: | ---: | ---: |",
    ]
    for row in baseline:
        lines.append(
            f"| {row['label']} | {row['found']} | {row['generated']} | {row['skipped']} | {row['generation_rate']} |"
        )

    lines.extend(
        [
            "",
            "## Practical Outcomes",
            "",
            "| Subject | Runs | Generation ok | Generation failed | Build attempted | Build ok | Execution attempted | Execution ok |",
            "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
        ]
    )
    for row in practical:
        lines.append(
            f"| {row['label']} | {row['runs']} | {row['generation_success']} | {row['generation_failure']} | "
            f"{row['build_attempted']} | {row['build_success']} | "
            f"{row['execution_attempted']} | {row['execution_success']} |"
        )

    lines.extend(
        [
            "",
            "## Coverage-Mode Ablation",
            "",
            "| Subject | Strict | Balanced | Aggressive | Strict gen | Balanced gen | Aggressive gen |",
            "| --- | ---: | ---: | ---: | ---: | ---: | ---: |",
        ]
    )
    for row in coverage:
        lines.append(
            f"| {row['label']} | {row['strict_rate']} | {row['balanced_rate']} | {row['aggressive_rate']} | "
            f"{row['strict_generated']} | {row['balanced_generated']} | {row['aggressive_generated']} |"
        )

    lines.extend(
        [
            "",
            "## Full-Factorial Sensitivity",
            "",
            "| Subject | Dimension | Level | Avg rate | Min rate | Max rate | Runs |",
            "| --- | --- | --- | ---: | ---: | ---: | ---: |",
        ]
    )
    for row in sensitivity:
        lines.append(
            f"| {row['subject']} | {row['dimension']} | {row['level']} | "
            f"{row['avg_generation_rate']} | {row['min_generation_rate']} | "
            f"{row['max_generation_rate']} | {row['runs']} |"
        )

    lines.extend(
        [
            "",
            "## Top Skip Reasons",
            "",
            "| Reason | Count |",
            "| --- | ---: |",
        ]
    )
    for row in skips:
        lines.append(f"| {row['reason']} | {row['count']} |")

    (out_dir / "evaluation_tables.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def update_latest_symlink(root: Path, out_dir: Path) -> None:
    latest = root / "analysis" / "eval_latest"
    latest.parent.mkdir(parents=True, exist_ok=True)
    if latest.exists() or latest.is_symlink():
        latest.unlink()
    target = out_dir.name if out_dir.parent == latest.parent else out_dir
    latest.symlink_to(target)


def write_run_metadata(root: Path, out_dir: Path, subjects: list[Subject]) -> None:
    selected_external_keys = required_external_subject_keys(subjects)
    externals = external_subjects(root)
    build_path_map = {
        "sut_showcase": out_dir / "_compdb" / "examples",
        "tinyxml2_core": root / "examples" / "tinyxml2" / "build",
        "tinyxml2_xmltest": root / "examples" / "tinyxml2" / "build",
        "yamlcpp_emitter": root / "examples" / "yaml-cpp" / "build",
        "openssl_ctype": out_dir / "_compdb" / "openssl",
        "sqlite_util": out_dir / "_compdb" / "sqlite",
    }
    metadata = {
        "framework": FRAMEWORK,
        "seed": SEED,
        "rule_max_cases": RULE_MAX_CASES,
        "full_factorial_subjects": [subject.key for subject in subjects if subject.subject_group == "full_factorial"],
        "base_plus_coverage_subjects": [subject.key for subject in subjects if subject.subject_group == "base_plus_coverage"],
        "total_runs": sum(len(planned_runs(subject)) for subject in subjects),
        "subjects": {
            subject.key: {
                "key": subject.key,
                "label": subject.label,
                "group": subject.subject_group,
                "build_path": str(build_path_map[subject.key]),
                "source": str(root / subject.source),
                "notes": subject.notes,
            }
            for subject in subjects
        },
        "raw_csv": str(out_dir / "raw_runs.csv"),
        "external_subjects": {
            key: {
                "repo": value.repo,
                "commit": value.commit,
                "path": value.path,
                "description": value.description,
            }
            for key, value in externals.items()
            if key in selected_external_keys
        },
    }
    write_json(out_dir / "run_metadata.json", metadata)


def build_viewer(root: Path, out_dir: Path) -> None:
    run([sys.executable, str(root / "scripts" / "build_eval_viewer.py"), str(out_dir)], cwd=root)


def parse_args(argv: list[str]) -> argparse.Namespace:
    default_out = repo_root() / "analysis" / f"eval_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out-dir", type=Path, default=default_out, help="Directory for evaluation outputs.")
    parser.add_argument(
        "--subjects",
        default="all",
        help="Comma-separated subset of evaluation subject keys, or 'all'.",
    )
    parser.add_argument("--prepare-subjects", action="store_true", help="Clone and checkout recorded external subject snapshots.")
    parser.add_argument("--build-viewer", action="store_true", help="Generate viewer.html after the evaluation completes.")
    parser.add_argument("--force", action="store_true", help="Delete --out-dir first if it already exists.")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv or sys.argv[1:])
    root = repo_root()
    out_dir = args.out_dir if args.out_dir.is_absolute() else root / args.out_dir

    if not (root / "askeleton").exists():
        raise SystemExit("Missing ./askeleton. Build first with: make CXX=clang++-18")

    if out_dir.exists():
        if not args.force:
            raise SystemExit(f"Output directory already exists: {out_dir}\nUse --force to overwrite it.")
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    subjects = select_subjects(evaluation_subjects(), args.subjects)
    ensure_subject_inputs(root, subjects, prepare_subjects=args.prepare_subjects)
    compdbs = prepare_compdbs(root, out_dir, subjects)
    total_runs = sum(len(planned_runs(subject)) for subject in subjects)

    rows: list[dict] = []
    index = 0
    for subject in subjects:
        build_path = resolve_build_path(root, out_dir, subject, compdbs)
        source_path = root / subject.source
        for plan in planned_runs(subject):
            index += 1
            rows.append(run_one(root, subject, build_path, source_path, out_dir, plan, index, total_runs))

    write_csv(
        out_dir / "raw_runs.csv",
        [
            "run_id",
            "experiment_group",
            "subject",
            "label",
            "subject_group",
            "notes",
            "framework",
            "seed",
            "profile",
            "coverage_mode",
            "oracle_mode",
            "rule_data",
            "rule_max_cases",
            "exit_code",
            "askeleton_exit_code",
            "generation_success",
            "build_attempted",
            "build_success",
            "execution_attempted",
            "execution_success",
            "found",
            "generated",
            "skipped",
            "generation_rate",
            "skip_rate",
            "by_reason_json",
            "by_kind_json",
            "report_path",
            "log_path",
            "generated_dir",
            "generated_output_path",
        ],
        rows,
    )

    baseline = baseline_summary_rows(subjects, rows)
    coverage = coverage_ablation_rows(subjects, rows)
    sensitivity = full_factorial_sensitivity_rows(
        [subject for subject in subjects if subject.subject_group == "full_factorial"],
        rows,
    )
    skips = skip_reason_rows(rows)
    practical = practical_outcome_rows(subjects, rows)

    write_csv(
        out_dir / "baseline_summary.csv",
        ["subject", "label", "found", "generated", "skipped", "generation_rate"],
        baseline,
    )
    write_csv(
        out_dir / "coverage_ablation.csv",
        [
            "subject",
            "label",
            "strict_rate",
            "balanced_rate",
            "aggressive_rate",
            "strict_generated",
            "balanced_generated",
            "aggressive_generated",
        ],
        coverage,
    )
    write_csv(
        out_dir / "full_factorial_sensitivity.csv",
        [
            "subject",
            "dimension",
            "level",
            "avg_generation_rate",
            "min_generation_rate",
            "max_generation_rate",
            "runs",
        ],
        sensitivity,
    )
    write_csv(out_dir / "skip_reason_summary.csv", ["reason", "count"], skips)
    write_evaluation_tables(out_dir, baseline, coverage, sensitivity, skips, practical)
    write_run_metadata(root, out_dir, subjects)
    update_latest_symlink(root, out_dir)

    if args.build_viewer:
        build_viewer(root, out_dir)

    print("")
    print(f"Evaluation complete: {out_dir}")
    print(f"Total runs: {total_runs}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
