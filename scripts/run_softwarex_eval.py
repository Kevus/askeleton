#!/usr/bin/env python3
"""Run the full publication evaluation campaign for ASkeleTon.

The campaign reproduced by this script is the one used for the existing
`campaign_all_options_no_frameworks` results: five subjects, a full semantic
option matrix, and additional operational runs.
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import shlex
import shutil
import subprocess
import sys
import time
from collections import Counter, defaultdict
from dataclasses import dataclass
from datetime import UTC, datetime
from pathlib import Path


PROFILES = ["random", "boundary", "safe", "stress"]
COVERAGE_MODES = ["balanced", "strict", "aggressive"]
ORACLE_MODES = ["explicit", "mirror", "property"]
RULE_MODES = [("rule_on", ["--rule-data"]), ("rule_off", ["--no-rule-data"])]
BASE_SEED = 123


@dataclass(frozen=True)
class ExternalSubject:
    key: str
    path: Path
    repo: str
    commit: str
    description: str


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def run_command(command: list[str], cwd: Path, *, check: bool = True) -> subprocess.CompletedProcess:
    return subprocess.run(command, cwd=cwd, check=check, text=True)


def run_shell(command: str, cwd: Path) -> None:
    subprocess.run(["bash", "-lc", command], cwd=cwd, check=True)


def write_json(path: Path, data) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as handle:
        json.dump(data, handle, indent=2, sort_keys=True)
        handle.write("\n")


def write_text(path: Path, data: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(data, encoding="utf-8")


def external_subjects(root: Path) -> dict[str, ExternalSubject]:
    examples = root / "examples"
    return {
        "tinyxml2": ExternalSubject(
            key="tinyxml2",
            path=examples / "tinyxml2",
            repo="https://github.com/leethomason/tinyxml2.git",
            commit="3dcad8e3c38e7091ae3771bf63020027f91715ce",
            description="tinyxml2 11.0.0 development snapshot",
        ),
        "sqlite": ExternalSubject(
            key="sqlite",
            path=examples / "sqlite",
            repo="https://github.com/sqlite/sqlite.git",
            commit="140cbff0d2acc5b375109d567fba352a3be2663f",
            description="SQLite 3.53.0 development snapshot",
        ),
        "openssl": ExternalSubject(
            key="openssl",
            path=examples / "openssl",
            repo="https://github.com/openssl/openssl.git",
            commit="81cc6cb97ef83ad138eebd47129368b9e963e8cd",
            description="OpenSSL 4.0.0-dev snapshot",
        ),
    }


def git_head(path: Path) -> str:
    if not (path / ".git").exists():
        return ""
    proc = subprocess.run(
        ["git", "rev-parse", "HEAD"],
        cwd=path,
        check=False,
        text=True,
        capture_output=True,
    )
    return proc.stdout.strip() if proc.returncode == 0 else ""


def clone_subject(subject: ExternalSubject) -> None:
    if subject.path.exists():
        return
    subject.path.parent.mkdir(parents=True, exist_ok=True)
    run_command(["git", "clone", "--no-checkout", subject.repo, str(subject.path)], cwd=subject.path.parent)
    run_command(["git", "checkout", subject.commit], cwd=subject.path)


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


def ensure_external_subjects(root: Path, prepare: bool, required_keys: set[str]) -> None:
    missing = []
    wrong_commit = []
    subjects = external_subjects(root)
    for key in sorted(required_keys):
        subject = subjects[key]
        if prepare:
            clone_subject(subject)
        if not subject.path.exists():
            missing.append(subject)
            continue
        head = git_head(subject.path)
        if head and head != subject.commit:
            wrong_commit.append((subject, head))

    if missing:
        lines = [
            "Missing external subjects required for the full publication campaign:",
            "",
        ]
        for subject in missing:
            lines.append(f"- {subject.key}: {subject.path}")
            lines.append(f"  repo: {subject.repo}")
            lines.append(f"  commit: {subject.commit}")
        lines.extend(
            [
                "",
                "Run with --prepare-subjects to clone the public repositories automatically,",
                "or place the exact snapshots at the paths above.",
            ]
        )
        raise SystemExit("\n".join(lines))

    if wrong_commit:
        lines = [
            "External subject commits do not match the recorded publication campaign:",
            "",
        ]
        for subject, head in wrong_commit:
            lines.append(f"- {subject.key}: expected {subject.commit}, found {head}")
        lines.append("")
        lines.append("Use --prepare-subjects in a clean tree, or checkout the recorded commits manually.")
        raise SystemExit("\n".join(lines))

    if "sqlite" in required_keys:
        prepare_sqlite(root / "examples" / "sqlite")
    if "openssl" in required_keys:
        prepare_openssl(root / "examples" / "openssl")


def build_cases(root: Path) -> dict:
    examples = root / "examples"
    return {
        "sut": {
            "source": examples / "sut.cpp",
            "compile_entry": {
                "directory": str(examples),
                "arguments": [
                    "clang++-18",
                    "-std=c++20",
                    f"-I{examples}",
                    "-c",
                    str(examples / "sut.cpp"),
                ],
                "file": str(examples / "sut.cpp"),
            },
            "bootstrap_ok": True,
        },
        "sut_showcase": {
            "source": examples / "sut_showcase.cpp",
            "compile_entry": {
                "directory": str(examples),
                "arguments": [
                    "clang++-18",
                    "-std=c++20",
                    f"-I{examples}",
                    "-c",
                    str(examples / "sut_showcase.cpp"),
                ],
                "file": str(examples / "sut_showcase.cpp"),
            },
            "bootstrap_ok": True,
        },
        "tinyxml2": {
            "source": examples / "tinyxml2" / "tinyxml2.cpp",
            "compile_entry": {
                "directory": str(examples / "tinyxml2"),
                "arguments": [
                    "clang++-18",
                    "-std=c++17",
                    f"-I{examples / 'tinyxml2'}",
                    "-c",
                    str(examples / "tinyxml2" / "tinyxml2.cpp"),
                ],
                "file": str(examples / "tinyxml2" / "tinyxml2.cpp"),
            },
            "bootstrap_ok": False,
        },
        "sqlite": {
            "source": examples / "sqlite" / "src" / "printf.c",
            "compile_entry": {
                "directory": str(examples / "sqlite" / "src"),
                "arguments": [
                    "clang-18",
                    "-std=c11",
                    f"-I{examples / 'sqlite' / 'src'}",
                    f"-I{examples / 'sqlite'}",
                    "-DSQLITE_THREADSAFE=1",
                    "-DSQLITE_OMIT_LOAD_EXTENSION=1",
                    "-c",
                    str(examples / "sqlite" / "src" / "printf.c"),
                ],
                "file": str(examples / "sqlite" / "src" / "printf.c"),
            },
            "bootstrap_ok": False,
        },
        "openssl": {
            "source": examples / "openssl" / "crypto" / "params.c",
            "compile_entry": {
                "directory": str(examples / "openssl"),
                "arguments": [
                    "clang-18",
                    "-std=c11",
                    f"-I{examples / 'openssl' / 'include'}",
                    f"-I{examples / 'openssl'}",
                    f"-I{examples / 'openssl' / 'crypto'}",
                    "-DOPENSSL_THREADS",
                    "-D_REENTRANT",
                    "-c",
                    str(examples / "openssl" / "crypto" / "params.c"),
                ],
                "file": str(examples / "openssl" / "crypto" / "params.c"),
            },
            "bootstrap_ok": False,
        },
    }


def prepare_compdbs(campaign_root: Path, cases: dict) -> None:
    compdb_root = campaign_root / "compdb"
    for name, case in cases.items():
        normal_dir = compdb_root / name / "normal"
        write_json(normal_dir / "compile_commands.json", [case["compile_entry"]])
        if case["bootstrap_ok"]:
            write_json(compdb_root / name / "bootstrap" / "compile_commands.json", [])


def semantic_runs() -> list[tuple[str, list[str]]]:
    runs = []
    for profile in PROFILES:
        for coverage in COVERAGE_MODES:
            for oracle in ORACLE_MODES:
                for rule_name, rule_flags in RULE_MODES:
                    run_name = (
                        f"matrix_profile-{profile}"
                        f"__coverage-{coverage}"
                        f"__oracle-{oracle}"
                        f"__{rule_name}"
                    )
                    flags = [
                        f"--profile={profile}",
                        f"--coverage-mode={coverage}",
                        f"--oracle-mode={oracle}",
                        f"--seed={BASE_SEED}",
                        *rule_flags,
                    ]
                    runs.append((run_name, flags))
    return runs


def operational_runs(bootstrap_ok: bool) -> list[tuple[str, list[str]]]:
    base_seeded = [
        "--profile=random",
        "--coverage-mode=balanced",
        "--oracle-mode=explicit",
        "--rule-data",
        f"--seed={BASE_SEED}",
    ]
    runs = [
        ("default_unseeded", []),
        ("rule_max_cases_5", [*base_seeded, "--rule-max-cases=5"]),
        ("deep_level_2", [*base_seeded, "--deep-level=2"]),
        (
            "extra_args",
            [
                *base_seeded,
                "--extra-arg=-Wno-unused-parameter",
                "--extra-arg-before=-DASKELETON_CAMPAIGN=1",
            ],
        ),
        ("no_system_files_refresh", [*base_seeded, "--no-system-files-refresh"]),
        ("include_impl_under_include", [*base_seeded, "--include-impl-under-include"]),
        ("quiet", [*base_seeded, "--quiet"]),
        ("verbose", [*base_seeded, "--verbose"]),
        ("debug", [*base_seeded, "--debug"]),
    ]
    if bootstrap_ok:
        runs.append(("bootstrap_compdb", ["--bootstrap-compdb"]))
    return runs


def parse_report(report_path: Path):
    if not report_path.exists():
        return None
    return json.loads(report_path.read_text(encoding="utf-8"))


def run_askeleton(root: Path, case_name: str, source_path: Path, compdb_dir: Path,
                  run_dir: Path, flags: list[str]) -> dict:
    generated_dir = run_dir / "generated"
    report_path = run_dir / "report.json"
    log_path = run_dir / "log.json"
    default_report_path = generated_dir / "askeleton_report.json"
    cmd = [
        str(root / "askeleton"),
        "-p",
        str(compdb_dir),
        "--report",
        str(report_path),
        "--report-json",
        "--log-json",
        str(log_path),
        "--out-dir",
        str(generated_dir),
        *flags,
        str(source_path),
    ]
    write_text(run_dir / "command.txt", " ".join(shlex.quote(part) for part in cmd) + "\n")
    started = time.time()
    completed = subprocess.run(
        cmd,
        cwd=str(root),
        env={**os.environ, "ASKELETON_HOME": str(root)},
        text=True,
        capture_output=True,
    )
    elapsed = time.time() - started
    write_text(run_dir / "stdout.txt", completed.stdout)
    write_text(run_dir / "stderr.txt", completed.stderr)
    report = parse_report(report_path)
    summary = (report or {}).get("summary", {})
    coverage = summary.get("coverage", {})
    row = {
        "case": case_name,
        "run_name": run_dir.name,
        "returncode": completed.returncode,
        "elapsed_seconds": round(elapsed, 3),
        "source": str(source_path),
        "compdb_dir": str(compdb_dir),
        "found": coverage.get("found"),
        "generated": coverage.get("generated"),
        "skipped": coverage.get("skipped"),
        "generation_rate": coverage.get("generation_rate"),
        "skip_reasons": summary.get("by_reason", {}),
        "report_path": str(report_path) if report_path.exists() else "",
        "report_json_path": str(default_report_path) if default_report_path.exists() else "",
        "log_path": str(log_path) if log_path.exists() else "",
        "generated_dir": str(generated_dir),
    }
    write_json(run_dir / "run_meta.json", row)
    return row


def selected_cases(all_cases: dict, requested: str) -> dict:
    if requested == "all":
        return all_cases
    names = [item.strip() for item in requested.split(",") if item.strip()]
    unknown = [name for name in names if name not in all_cases]
    if unknown:
        raise SystemExit(f"Unknown case(s): {', '.join(unknown)}")
    return {name: all_cases[name] for name in names}


def write_case_overview(summaries_dir: Path, by_case: dict[str, list[dict]]) -> None:
    overview = {}
    lines = ["# Case Overview", ""]
    for case_name in sorted(by_case):
        rows = by_case[case_name]
        by_run = {row["run_name"]: row for row in rows}
        default = by_run.get("default_unseeded")
        balanced = by_run.get("matrix_profile-random__coverage-balanced__oracle-explicit__rule_on")
        strict = by_run.get("matrix_profile-random__coverage-strict__oracle-explicit__rule_on")
        non_zero = sum(1 for row in rows if row["returncode"] != 0)
        overview[case_name] = {
            "runs": len(rows),
            "non_zero_returncodes": non_zero,
            "default_unseeded": default,
            "balanced_seeded_rule_on": balanced,
            "strict_seeded_rule_on": strict,
        }
        lines.append(f"## {case_name}")
        lines.append("")
        lines.append(f"- Runs: {len(rows)}")
        lines.append(f"- Non-zero return codes: {non_zero}")
        for label, row in [
            ("Default unseeded", default),
            ("Balanced seeded rule_on", balanced),
            ("Strict seeded rule_on", strict),
        ]:
            if row:
                lines.append(
                    f"- {label}: found={row['found']}, generated={row['generated']}, "
                    f"skipped={row['skipped']}, rate={row['generation_rate']:.4f}, "
                    f"skip_reasons={json.dumps(row['skip_reasons'], sort_keys=True)}"
                )
        lines.append("")
    write_json(summaries_dir / "case_overview.json", overview)
    write_text(summaries_dir / "case_overview.md", "\n".join(lines))


def write_summaries(campaign_root: Path, rows: list[dict]) -> None:
    summaries_dir = campaign_root / "summaries"
    summaries_dir.mkdir(parents=True, exist_ok=True)
    write_json(summaries_dir / "all_runs.json", rows)

    fieldnames = [
        "case",
        "run_name",
        "returncode",
        "elapsed_seconds",
        "found",
        "generated",
        "skipped",
        "generation_rate",
        "skip_reasons",
        "report_path",
        "report_json_path",
        "log_path",
        "generated_dir",
    ]
    with (summaries_dir / "all_runs.csv").open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for row in rows:
            out = {key: row.get(key, "") for key in fieldnames}
            out["skip_reasons"] = json.dumps(row["skip_reasons"], sort_keys=True)
            writer.writerow(out)

    by_case = defaultdict(list)
    for row in rows:
        by_case[row["case"]].append(row)

    lines = ["# ASkeleTon Campaign Results", ""]
    for case_name in sorted(by_case):
        case_rows = by_case[case_name]
        lines.append(f"## {case_name}")
        lines.append("")
        lines.append(f"- Runs: {len(case_rows)}")
        lines.append(f"- Successful runs: {sum(1 for row in case_rows if row['returncode'] == 0)}")
        lines.append("- Summary CSV: summaries/all_runs.csv")
        lines.append("")

        case_csv_path = summaries_dir / f"{case_name}.csv"
        with case_csv_path.open("w", newline="", encoding="utf-8") as handle:
            writer = csv.DictWriter(
                handle,
                fieldnames=[
                    "run_name",
                    "returncode",
                    "elapsed_seconds",
                    "found",
                    "generated",
                    "skipped",
                    "generation_rate",
                    "skip_reasons",
                    "report_path",
                    "log_path",
                    "generated_dir",
                ],
            )
            writer.writeheader()
            for row in case_rows:
                writer.writerow(
                    {
                        key: json.dumps(row[key], sort_keys=True)
                        if key == "skip_reasons"
                        else row[key]
                        for key in writer.fieldnames
                    }
                )

        lines.append("| run | rc | found | generated | skipped | rate | skip_reasons |")
        lines.append("| --- | --- | --- | --- | --- | --- | --- |")
        for row in sorted(case_rows, key=lambda item: item["run_name"]):
            rate = row["generation_rate"]
            rate_str = "" if rate is None else f"{rate:.4f}"
            reason_str = json.dumps(row["skip_reasons"], sort_keys=True)
            lines.append(
                f"| {row['run_name']} | {row['returncode']} | {row['found']} | "
                f"{row['generated']} | {row['skipped']} | {rate_str} | {reason_str} |"
            )
        lines.append("")

    write_text(summaries_dir / "README.md", "\n".join(lines) + "\n")
    write_case_overview(summaries_dir, by_case)


def write_campaign_metadata(root: Path, campaign_root: Path, cases: dict) -> None:
    metadata = {
        "askeleton_version": subprocess.check_output(
            [str(root / "askeleton"), "--version"],
            cwd=root,
            text=True,
            env={**os.environ, "ASKELETON_HOME": str(root)},
        ).strip(),
        "created_at": datetime.now(UTC).isoformat(timespec="seconds").replace("+00:00", "Z"),
        "profiles": PROFILES,
        "coverage_modes": COVERAGE_MODES,
        "oracle_modes": ORACLE_MODES,
        "seed": BASE_SEED,
        "cases": {
            name: {
                "source": str(case["source"]),
                "bootstrap_ok": case["bootstrap_ok"],
            }
            for name, case in cases.items()
        },
        "external_subjects": {
            key: {
                "path": str(subject.path),
                "repo": subject.repo,
                "commit": subject.commit,
                "description": subject.description,
            }
            for key, subject in external_subjects(root).items()
        },
    }
    write_json(campaign_root / "campaign_metadata.json", metadata)


def parse_args(argv: list[str]) -> argparse.Namespace:
    default_out = repo_root() / "analysis" / f"publication_eval_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out-dir", type=Path, default=default_out,
                        help="Directory for campaign outputs.")
    parser.add_argument("--cases", default="all",
                        help="Comma-separated case list, or 'all'.")
    parser.add_argument("--prepare-subjects", action="store_true",
                        help="Clone missing external subject repositories at the recorded commits.")
    parser.add_argument("--force", action="store_true",
                        help="Delete --out-dir first if it already exists.")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv or sys.argv[1:])
    root = repo_root()
    campaign_root = args.out_dir if args.out_dir.is_absolute() else root / args.out_dir

    if not (root / "askeleton").exists():
        raise SystemExit("Missing ./askeleton. Build first with: make CXX=clang++-18")

    if campaign_root.exists():
        if not args.force:
            raise SystemExit(f"Output directory already exists: {campaign_root}\nUse --force to overwrite it.")
        shutil.rmtree(campaign_root)
    campaign_root.mkdir(parents=True, exist_ok=True)

    cases = selected_cases(build_cases(root), args.cases)
    ensure_external_subjects(root, args.prepare_subjects, set(cases).intersection(external_subjects(root)))
    prepare_compdbs(campaign_root, cases)
    write_campaign_metadata(root, campaign_root, cases)

    rows = []
    total = sum(72 + len(operational_runs(case["bootstrap_ok"])) for case in cases.values())
    index = 0
    for case_name, case in cases.items():
        source = case["source"]
        normal_compdb = campaign_root / "compdb" / case_name / "normal"
        for run_name, flags in semantic_runs():
            index += 1
            print(f"[{index}/{total}] {case_name} {run_name}", flush=True)
            run_dir = campaign_root / "runs" / case_name / run_name
            run_dir.mkdir(parents=True, exist_ok=True)
            rows.append(run_askeleton(root, case_name, source, normal_compdb, run_dir, flags))
        for run_name, flags in operational_runs(case["bootstrap_ok"]):
            index += 1
            print(f"[{index}/{total}] {case_name} {run_name}", flush=True)
            compdb = campaign_root / "compdb" / case_name / "bootstrap" if run_name == "bootstrap_compdb" else normal_compdb
            run_dir = campaign_root / "runs" / case_name / run_name
            run_dir.mkdir(parents=True, exist_ok=True)
            rows.append(run_askeleton(root, case_name, source, compdb, run_dir, flags))

    write_summaries(campaign_root, rows)

    counts = Counter(row["case"] for row in rows)
    print("")
    print(f"Campaign complete: {campaign_root}")
    print(f"Total runs: {len(rows)}")
    for case_name in sorted(counts):
        print(f"  {case_name}: {counts[case_name]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
