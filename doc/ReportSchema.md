# Report JSON Schema

ASkeleTon can write a generation report with `--report=<path>` or
`--report-json`. The report is intended for humans and scripts: it records the
run configuration, every discovered entity, aggregate counts, and skip reason
summaries.

These fields are scaffolding-generation diagnostics. They do not measure code
coverage, fault detection, semantic adequacy, or test quality.

This document describes the current report shape emitted by `src/Report.cpp`.
Fields are stable JSON keys, but consumers should tolerate extra fields in
future ASkeleTon versions.

## How To Generate A Report

```bash
ASKELETON_HOME=$(pwd) ./askeleton \
  --bootstrap-compdb \
  -p examples \
  --seed=123 \
  --coverage-mode=balanced \
  --oracle-mode=explicit \
  --report=/tmp/askeleton_report.json \
  examples/sut.cpp
```

`--report=<path>` writes to the given path. `--report-json` writes to
`<out-dir>/askeleton_report.json`.

## Top-Level Fields

| Field | Type | Meaning |
| --- | --- | --- |
| `generated_at` | string | Local timestamp for the ASkeleTon run. |
| `profile` | string | Fallback input-data profile, such as `random`, `boundary`, `safe`, or `stress`. |
| `coverage_mode` | string | Generation eligibility policy: `strict`, `balanced`, or `aggressive`. |
| `oracle_mode` | string | Expected-value strategy selected by `--oracle-mode`. |
| `seed` | number | Present only when a non-negative `--seed` was supplied. |
| `rule_data` | boolean | Whether AST-guided rule data was enabled for the run. |
| `rule_max_cases` | number | Effective rule-case limit used by the run. |
| `framework` | string | Generated test framework, such as `gtest`, `boost`, or `catch`. |
| `sources` | array | Source files passed to ASkeleTon. |
| `items` | array | Per-entity generation diagnostics. |
| `summary` | object | Aggregate counts and grouped summaries. |

## Per-Entity Fields

Each object in `items` describes one discovered entity. An entity may be a
free function, method, or constructor.

| Field | Type | Meaning |
| --- | --- | --- |
| `kind` | string | Entity kind, currently `function`, `method`, or `constructor`. |
| `name` | string | Short callable or class name. |
| `qualified_name` | string | Fully qualified callable name when available. |
| `file` | string | Source file path reported by Clang. |
| `line` | number | 1-based source line. |
| `column` | number | 1-based source column. |
| `target` | string | Generated target group, usually the source stem or class name. |
| `is_class` | boolean | `true` for class-scoped methods or constructors. |
| `status` | string | `generated` or `skipped`. |
| `signature` | string | Callable signature when available. |
| `test_cases` | number | Present for generated entities when at least one test case was emitted. |
| `reason` | string | Present for skipped entities. Machine-readable skip reason code. |
| `detail` | string | Present for skipped entities when ASkeleTon has a specific diagnostic detail. |

`reason` is intended for grouping and automation. `detail` is intended for
triage; it may include a type, callable, or policy-specific explanation.

## Summary Fields

`summary` contains grouped counts derived from `items`.

| Field | Type | Meaning |
| --- | --- | --- |
| `by_status` | object | Counts by item status. |
| `by_kind` | object | Counts by entity kind. |
| `by_target` | object | Counts by generated target group. |
| `by_reason` | object | Counts by skip reason. Empty when there are no skips. |
| `by_file` | object | Per-source status counts. |
| `coverage` | object | Found/generated/skipped scaffolding counts and rates. The key name is retained for compatibility. |
| `top_skip_reasons` | array | Up to five most frequent skip reasons, sorted by count then name. |
| `targets_with_most_skips` | array | Up to five targets with skipped entities. |

`coverage.found` is the number of discovered entities recorded in the report.
`coverage.generated` is the number with `status: "generated"`.
`coverage.skipped` is the number with `status: "skipped"`.
`coverage.generation_rate` is `generated / found` as a number from `0.0` to
`1.0`; console output formats the same ratio as a percentage.
`coverage.skip_rate` is `skipped / found`.

Despite the `coverage` key name, these values describe scaffolding-generation
outcomes. They are not line, branch, or path coverage measurements and do not
evaluate test quality.

## Summary-Level Vs Entity-Level Diagnostics

Use summary fields to answer broad questions:

- How many entities were generated?
- Which skip reasons are most common?
- Which target has the most skipped entities?
- Did a generation eligibility mode change affect scaffolding generation rate?

Use `items` to decide what to do next for a specific callable:

- which function, method, or constructor was skipped;
- where it appears in the source;
- which signature ASkeleTon saw;
- the exact skip reason and detail string.

For remediation guidance by reason code, see
[`SkipReasons.md`](SkipReasons.md).

## Example Generated Entry

This generated entry is based on the report written by
`./scripts/check_main_workflow.sh` for `examples/sut.cpp`:

```json
{
  "column": 1,
  "file": "<repo>/examples/sut.cpp",
  "is_class": false,
  "kind": "function",
  "line": 11,
  "name": "add_i64",
  "qualified_name": "add_i64",
  "signature": "int add_i64(int64_t, int64_t)",
  "status": "generated",
  "target": "sut",
  "test_cases": 1
}
```

## Example Skipped Entry

This skipped entry is based on a `sut_showcase` evaluation report generated
with `--coverage-mode=strict`:

```json
{
  "column": 1,
  "detail": "strict coverage mode skips mutable pointer/reference parameters: showcase::sum_ptrs",
  "file": "<repo>/examples/sut_showcase.cpp",
  "is_class": false,
  "kind": "function",
  "line": 103,
  "name": "sum_ptrs",
  "qualified_name": "showcase::sum_ptrs",
  "reason": "coverage_policy_mutable_parameter",
  "signature": "int showcase::sum_ptrs(const int *, int *)",
  "status": "skipped",
  "target": "sut_showcase"
}
```

## Example Summary Fragment

This summary fragment is based on a `sut_showcase` evaluation report generated
with `--coverage-mode=balanced`:

```json
{
  "coverage": {
    "found": 24,
    "generated": 16,
    "generation_rate": 0.6666666666666666,
    "skip_rate": 0.3333333333333333,
    "skipped": 8
  },
  "by_reason": {
    "abstract_record": 1,
    "missing_instance_strategy": 2,
    "unsupported_indirection": 1,
    "unsupported_type": 1,
    "unusable_constructor": 3
  }
}
```

## Compatibility Notes

- Report JSON is machine-readable and stable enough for CI summaries.
- Consumers should treat unknown future fields as additive.
- Omitted optional fields are meaningful. For example, generated entries do
  not include `reason`, and reports without `--seed` do not include `seed`.
- Report paths may be absolute and machine-specific.
