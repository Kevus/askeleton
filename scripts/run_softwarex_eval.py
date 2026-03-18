#!/usr/bin/env python3
import csv
import itertools
import json
import os
import shutil
import subprocess
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path


PROFILES = ["random", "boundary", "safe", "stress"]
COVERAGE_MODES = ["strict", "balanced", "aggressive"]
ORACLE_MODES = ["mirror", "explicit", "property"]
RULE_DATA = [False, True]
BASE_FRAMEWORK = "gtest"
BASE_PROFILE = "random"
BASE_ORACLE = "explicit"
BASE_COVERAGE = "balanced"
BASE_RULE_DATA = True
BASE_SEED = 123
RULE_MAX_CASES = 3


@dataclass(frozen=True)
class Subject:
    key: str
    label: str
    group: str
    build_path: str
    source: str
    notes: str


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def run_shell(command: str, cwd: Path) -> None:
    subprocess.run(["bash", "-lc", command], cwd=cwd, check=True)


def ensure_sqlite_headers(root: Path) -> None:
    sqlite_dir = root / "examples" / "sqlite"
    sqlite3_h = sqlite_dir / "sqlite3.h"
    if not sqlite3_h.exists():
        run_shell("./configure >/tmp/askeleton_sqlite_configure.log 2>&1", sqlite_dir)
        run_shell("make sqlite3.h >/tmp/askeleton_sqlite_make.log 2>&1", sqlite_dir)

    opcodes_h = sqlite_dir / "opcodes.h"
    src_opcodes_h = sqlite_dir / "src" / "opcodes.h"
    if not opcodes_h.exists():
        run_shell("cat parse.h src/vdbe.c | tclsh tool/mkopcodeh.tcl > opcodes.h",
                  sqlite_dir)
    if opcodes_h.exists() and not src_opcodes_h.exists():
        shutil.copyfile(opcodes_h, src_opcodes_h)


def ensure_openssl_headers(root: Path) -> None:
    openssl_dir = root / "examples" / "openssl"
    configuration_h = openssl_dir / "include" / "openssl" / "configuration.h"
    opensslv_h = openssl_dir / "include" / "openssl" / "opensslv.h"
    if not configuration_h.exists():
        run_shell("perl ./Configure >/tmp/askeleton_openssl_configure.log 2>&1", openssl_dir)
    if not opensslv_h.exists():
        run_shell("make include/openssl/opensslv.h >/tmp/askeleton_openssl_make.log 2>&1", openssl_dir)


def write_compdb(path: Path, entries: list[dict]) -> Path:
    path.mkdir(parents=True, exist_ok=True)
    compdb = path / "compile_commands.json"
    compdb.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
    return path


def prepare_compdbs(root: Path, out_dir: Path) -> dict[str, str]:
    ensure_openssl_headers(root)
    ensure_sqlite_headers(root)
    tmp_dir = out_dir / "_compdb"

    examples_dir = root / "examples"
    examples_entries = []
    for rel in ("sut_showcase.cpp",):
        src = examples_dir / rel
        examples_entries.append(
            {
                "directory": str(examples_dir),
                "command": f"clang++ -std=c++20 -I. -c {src}",
                "file": str(src),
            }
        )
    mapping = {
        "examples_local": str(write_compdb(tmp_dir / "examples", examples_entries)),
        "openssl_local": str(
            write_compdb(
                tmp_dir / "openssl",
                [
                    {
                        "directory": str(root / "examples" / "openssl"),
                        "command": (
                            "clang -std=c11 "
                            f"-I{root}/examples/openssl "
                            f"-I{root}/examples/openssl/include "
                            f"-c {root}/examples/openssl/crypto/ctype.c"
                        ),
                        "file": str(root / "examples" / "openssl" / "crypto" / "ctype.c"),
                    }
                ],
            )
        ),
        "sqlite_local": str(
            write_compdb(
                tmp_dir / "sqlite",
                [
                    {
                        "directory": str(root / "examples" / "sqlite"),
                        "command": (
                            "clang -std=c11 "
                            f"-I{root}/examples/sqlite "
                            f"-I{root}/examples/sqlite/src "
                            f"-c {root}/examples/sqlite/src/util.c"
                        ),
                        "file": str(root / "examples" / "sqlite" / "src" / "util.c"),
                    }
                ],
            )
        ),
    }
    return mapping


def subjects(root: Path, compdb_map: dict[str, str]) -> dict[str, Subject]:
    return {
        "sut_showcase": Subject(
            key="sut_showcase",
            label="Illustrative showcase",
            group="full_factorial",
            build_path=compdb_map["examples_local"],
            source=str(root / "examples" / "sut_showcase.cpp"),
            notes="Controlled example with intended skips.",
        ),
        "tinyxml2_core": Subject(
            key="tinyxml2_core",
            label="tinyxml2 core",
            group="full_factorial",
            build_path=str(root / "examples" / "tinyxml2" / "build"),
            source=str(root / "examples" / "tinyxml2" / "tinyxml2.cpp"),
            notes="Real library TU with broad method surface.",
        ),
        "tinyxml2_xmltest": Subject(
            key="tinyxml2_xmltest",
            label="tinyxml2 xmltest",
            group="base_plus_coverage",
            build_path=str(root / "examples" / "tinyxml2" / "build"),
            source=str(root / "examples" / "tinyxml2" / "xmltest.cpp"),
            notes="Real program TU with high success rate.",
        ),
        "yamlcpp_emitter": Subject(
            key="yamlcpp_emitter",
            label="yaml-cpp emitter",
            group="base_plus_coverage",
            build_path=str(root / "examples" / "yaml-cpp" / "build"),
            source=str(root / "examples" / "yaml-cpp" / "src" / "emitter.cpp"),
            notes="Real C++ TU with richer type shapes.",
        ),
        "openssl_ctype": Subject(
            key="openssl_ctype",
            label="OpenSSL ctype",
            group="base_plus_coverage",
            build_path=compdb_map["openssl_local"],
            source=str(root / "examples" / "openssl" / "crypto" / "ctype.c"),
            notes="Compact real C TU with full generation.",
        ),
        "sqlite_util": Subject(
            key="sqlite_util",
            label="SQLite util",
            group="base_plus_coverage",
            build_path=compdb_map["sqlite_local"],
            source=str(root / "examples" / "sqlite" / "src" / "util.c"),
            notes="Real SQLite core TU with internal utility routines and moderate pointer-heavy skips.",
        ),
    }


def full_factorial_runs(subject: Subject) -> list[dict]:
    runs = []
    for profile, coverage, oracle, rule_data in itertools.product(
        PROFILES, COVERAGE_MODES, ORACLE_MODES, RULE_DATA
    ):
        runs.append(
            {
                "experiment_group": "full_factorial",
                "subject": subject,
                "profile": profile,
                "coverage_mode": coverage,
                "oracle_mode": oracle,
                "rule_data": rule_data,
            }
        )
    return runs


def base_plus_coverage_runs(subject: Subject) -> list[dict]:
    runs = [
        {
            "experiment_group": "base",
            "subject": subject,
            "profile": BASE_PROFILE,
            "coverage_mode": BASE_COVERAGE,
            "oracle_mode": BASE_ORACLE,
            "rule_data": BASE_RULE_DATA,
        }
    ]
    for coverage in ("strict", "aggressive"):
        runs.append(
            {
                "experiment_group": "coverage_ablation",
                "subject": subject,
                "profile": BASE_PROFILE,
                "coverage_mode": coverage,
                "oracle_mode": BASE_ORACLE,
                "rule_data": BASE_RULE_DATA,
            }
        )
    return runs


def run_case(root: Path, out_dir: Path, run_spec: dict) -> dict:
    subject = run_spec["subject"]
    profile = run_spec["profile"]
    coverage = run_spec["coverage_mode"]
    oracle = run_spec["oracle_mode"]
    rule_data = run_spec["rule_data"]

    run_id = (
        f"{subject.key}__{run_spec['experiment_group']}"
        f"__profile-{profile}"
        f"__coverage-{coverage}"
        f"__oracle-{oracle}"
        f"__rule-{'on' if rule_data else 'off'}"
    )
    subject_dir = out_dir / subject.key
    report_path = subject_dir / "reports" / f"{run_id}.json"
    log_path = subject_dir / "logs" / f"{run_id}.log"
    generated_dir = subject_dir / "generated" / run_id
    report_path.parent.mkdir(parents=True, exist_ok=True)
    log_path.parent.mkdir(parents=True, exist_ok=True)
    generated_dir.parent.mkdir(parents=True, exist_ok=True)

    cmd = [
        str(root / "askeleton"),
        "-p",
        subject.build_path,
        "--framework",
        BASE_FRAMEWORK,
        "--seed",
        str(BASE_SEED),
        "--profile",
        profile,
        "--coverage-mode",
        coverage,
        "--oracle-mode",
        oracle,
        "--report",
        str(report_path),
        "--out-dir",
        str(generated_dir),
        subject.source,
    ]
    if rule_data:
        cmd.extend(["--rule-data", "--rule-max-cases", str(RULE_MAX_CASES)])
    else:
        cmd.append("--no-rule-data")

    env = os.environ.copy()
    env["ASKELETON_HOME"] = str(root)
    with log_path.open("w", encoding="utf-8") as fh:
        proc = subprocess.run(cmd, cwd=root, env=env, stdout=fh, stderr=subprocess.STDOUT, text=True)

    report = {}
    if report_path.exists():
        report = json.loads(report_path.read_text(encoding="utf-8"))
    summary = report.get("summary", {})
    coverage_summary = summary.get("coverage", {})
    return {
        "run_id": run_id,
        "experiment_group": run_spec["experiment_group"],
        "subject": subject.key,
        "label": subject.label,
        "subject_group": subject.group,
        "notes": subject.notes,
        "framework": BASE_FRAMEWORK,
        "seed": BASE_SEED,
        "profile": profile,
        "coverage_mode": coverage,
        "oracle_mode": oracle,
        "rule_data": "on" if rule_data else "off",
        "rule_max_cases": RULE_MAX_CASES if rule_data else "",
        "exit_code": proc.returncode,
        "found": coverage_summary.get("found", 0),
        "generated": coverage_summary.get("generated", 0),
        "skipped": coverage_summary.get("skipped", 0),
        "generation_rate": coverage_summary.get("generation_rate", 0.0),
        "skip_rate": coverage_summary.get("skip_rate", 0.0),
        "by_reason_json": json.dumps(summary.get("by_reason", {}), sort_keys=True),
        "by_kind_json": json.dumps(summary.get("by_kind", {}), sort_keys=True),
        "report_path": str(report_path),
        "log_path": str(log_path),
        "generated_dir": str(generated_dir),
    }


def write_csv(path: Path, rows: list[dict]) -> None:
    if not rows:
        return
    with path.open("w", encoding="utf-8", newline="") as fh:
        writer = csv.DictWriter(fh, fieldnames=list(rows[0].keys()))
        writer.writeheader()
        writer.writerows(rows)


def baseline_table(rows: list[dict]) -> list[dict]:
    selected = []
    for row in rows:
        if (
            row["profile"] == BASE_PROFILE
            and row["coverage_mode"] == BASE_COVERAGE
            and row["oracle_mode"] == BASE_ORACLE
            and row["rule_data"] == "on"
        ):
            selected.append(
                {
                    "subject": row["subject"],
                    "label": row["label"],
                    "found": row["found"],
                    "generated": row["generated"],
                    "skipped": row["skipped"],
                    "generation_rate": f"{100 * float(row['generation_rate']):.2f}%",
                }
            )
    return selected


def coverage_ablation_table(rows: list[dict], subject_keys: list[str]) -> list[dict]:
    table = []
    for subject_key in subject_keys:
        subset = [row for row in rows if row["subject"] == subject_key]
        by_mode = {row["coverage_mode"]: row for row in subset if row["profile"] == BASE_PROFILE and row["oracle_mode"] == BASE_ORACLE and row["rule_data"] == "on"}
        strict = by_mode.get("strict")
        balanced = by_mode.get("balanced")
        aggressive = by_mode.get("aggressive")
        label = subset[0]["label"] if subset else subject_key
        table.append(
            {
                "subject": subject_key,
                "label": label,
                "strict_rate": f"{100 * float(strict['generation_rate']):.2f}%" if strict else "",
                "balanced_rate": f"{100 * float(balanced['generation_rate']):.2f}%" if balanced else "",
                "aggressive_rate": f"{100 * float(aggressive['generation_rate']):.2f}%" if aggressive else "",
                "strict_generated": strict["generated"] if strict else "",
                "balanced_generated": balanced["generated"] if balanced else "",
                "aggressive_generated": aggressive["generated"] if aggressive else "",
            }
        )
    return table


def sensitivity_table(rows: list[dict], subject_key: str) -> list[dict]:
    subset = [row for row in rows if row["subject"] == subject_key]
    table = []
    for dimension, levels in [
        ("profile", PROFILES),
        ("coverage_mode", COVERAGE_MODES),
        ("oracle_mode", ORACLE_MODES),
        ("rule_data", ["off", "on"]),
    ]:
        for level in levels:
            rates = [float(row["generation_rate"]) for row in subset if row[dimension] == level]
            table.append(
                {
                    "subject": subject_key,
                    "dimension": dimension,
                    "level": level,
                    "avg_generation_rate": f"{100 * (sum(rates) / len(rates)):.2f}%",
                    "min_generation_rate": f"{100 * min(rates):.2f}%",
                    "max_generation_rate": f"{100 * max(rates):.2f}%",
                    "runs": len(rates),
                }
            )
    return table


def skip_reason_table(rows: list[dict]) -> list[dict]:
    counter = Counter()
    for row in rows:
        counter.update(json.loads(row["by_reason_json"]))
    return [{"reason": reason, "count": count} for reason, count in counter.most_common()]


def write_markdown(
    path: Path,
    baseline_rows: list[dict],
    coverage_rows: list[dict],
    sensitivity_rows: list[dict],
    skip_rows: list[dict],
    out_dir: Path,
) -> None:
    lines = []
    lines.append("# publication Evaluation Summary")
    lines.append("")
    lines.append(f"Raw data directory: `{out_dir}`")
    lines.append("")
    lines.append("## Baseline Configuration")
    lines.append("")
    lines.append(
        f"- Framework: `{BASE_FRAMEWORK}`; seed: `{BASE_SEED}`; profile: `{BASE_PROFILE}`; "
        f"coverage: `{BASE_COVERAGE}`; oracle: `{BASE_ORACLE}`; rule-data: `on`."
    )
    lines.append("")
    lines.append("| Subject | Found | Generated | Skipped | Generation rate |")
    lines.append("| --- | ---: | ---: | ---: | ---: |")
    for row in baseline_rows:
        lines.append(
            f"| {row['label']} | {row['found']} | {row['generated']} | {row['skipped']} | {row['generation_rate']} |"
        )
    lines.append("")
    lines.append("## Coverage-Mode Ablation")
    lines.append("")
    lines.append("| Subject | Strict | Balanced | Aggressive | Strict gen | Balanced gen | Aggressive gen |")
    lines.append("| --- | ---: | ---: | ---: | ---: | ---: | ---: |")
    for row in coverage_rows:
        lines.append(
            f"| {row['label']} | {row['strict_rate']} | {row['balanced_rate']} | {row['aggressive_rate']} | "
            f"{row['strict_generated']} | {row['balanced_generated']} | {row['aggressive_generated']} |"
        )
    lines.append("")
    lines.append("## Full-Factorial Sensitivity")
    lines.append("")
    lines.append("| Subject | Dimension | Level | Avg rate | Min rate | Max rate | Runs |")
    lines.append("| --- | --- | --- | ---: | ---: | ---: | ---: |")
    for row in sensitivity_rows:
        lines.append(
            f"| {row['subject']} | {row['dimension']} | {row['level']} | "
            f"{row['avg_generation_rate']} | {row['min_generation_rate']} | {row['max_generation_rate']} | {row['runs']} |"
        )
    lines.append("")
    lines.append("## Top Skip Reasons")
    lines.append("")
    lines.append("| Reason | Count |")
    lines.append("| --- | ---: |")
    for row in skip_rows[:12]:
        lines.append(f"| {row['reason']} | {row['count']} |")
    lines.append("")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    root = repo_root()
    timestamp = subprocess.check_output(["date", "+%Y%m%d_%H%M%S"], text=True, cwd=root).strip()
    out_dir = root / "analysis" / f"publication_eval_{timestamp}"
    if out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.mkdir(parents=True)

    compdb_map = prepare_compdbs(root, out_dir)
    subject_map = subjects(root, compdb_map)

    run_specs = []
    for key in ("sut_showcase", "tinyxml2_core"):
        run_specs.extend(full_factorial_runs(subject_map[key]))
    for key in ("tinyxml2_xmltest", "yamlcpp_emitter", "openssl_ctype", "sqlite_util"):
        run_specs.extend(base_plus_coverage_runs(subject_map[key]))

    rows = []
    total = len(run_specs)
    for idx, run_spec in enumerate(run_specs, start=1):
        subject = run_spec["subject"].key
        print(f"[{idx}/{total}] {subject} {run_spec['experiment_group']} {run_spec['coverage_mode']}", flush=True)
        rows.append(run_case(root, out_dir, run_spec))

    raw_csv = out_dir / "raw_runs.csv"
    write_csv(raw_csv, rows)

    baseline_rows = baseline_table(rows)
    coverage_rows = coverage_ablation_table(
        rows,
        ["sut_showcase", "tinyxml2_core", "tinyxml2_xmltest", "yamlcpp_emitter", "openssl_ctype", "sqlite_util"],
    )
    sensitivity_rows = sensitivity_table(rows, "sut_showcase") + sensitivity_table(rows, "tinyxml2_core")
    skip_rows = skip_reason_table(rows)

    write_csv(out_dir / "baseline_summary.csv", baseline_rows)
    write_csv(out_dir / "coverage_ablation.csv", coverage_rows)
    write_csv(out_dir / "full_factorial_sensitivity.csv", sensitivity_rows)
    write_csv(out_dir / "skip_reason_summary.csv", skip_rows)

    metadata = {
        "framework": BASE_FRAMEWORK,
        "seed": BASE_SEED,
        "rule_max_cases": RULE_MAX_CASES,
        "full_factorial_subjects": ["sut_showcase", "tinyxml2_core"],
        "base_plus_coverage_subjects": ["tinyxml2_xmltest", "yamlcpp_emitter", "openssl_ctype", "sqlite_util"],
        "total_runs": len(rows),
        "subjects": {key: subject.__dict__ for key, subject in subject_map.items()},
        "raw_csv": str(raw_csv),
    }
    (out_dir / "run_metadata.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")

    write_markdown(
        out_dir / "paper_ready_tables.md",
        baseline_rows,
        coverage_rows,
        sensitivity_rows,
        skip_rows,
        out_dir,
    )

    latest = root / "analysis" / "publication_eval_latest"
    if latest.exists() or latest.is_symlink():
        latest.unlink()
    latest.symlink_to(out_dir.name)

    print(f"Evaluation complete: {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
