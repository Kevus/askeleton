# Input Generation

ASkeleTon generates `.cfg` input data so emitted tests can compile and run
without requiring users to hand-write every fixture value. The generated data is
a starting point for executable scaffolding. Users can inspect and edit the
`.cfg` files, add explicit expected values, and keep refined generated artifacts
for regression workflows.

This guide is the conceptual overview. The detailed catalog of AST-derived
rules remains in [`DataRules.md`](DataRules.md).

## Scope

ASkeleTon uses Clang AST inspection plus profile-based fallback generation. It
does not implement symbolic execution, concolic execution, or path exploration.
Rule-derived values are lightweight candidates extracted from local syntax such
as comparisons, literals, defaults, and `switch` cases.

Some behavior below is observed in generated output from repository examples.
Implementation details such as rule precedence and candidate selection are based
on `ASKGen`, `ConfigGenerator`, and `RandomValuesGenerator`.

## Profiles

`--profile=<random|boundary|safe|stress>` controls fallback values when no
higher-priority source supplies a value for a parameter.

| Profile | Intent | Typical generated shape |
| --- | --- | --- |
| `random` | Broad default sampling. | Integers and floating values in a moderate `-100..100` range; strings and containers with varied sizes. |
| `boundary` | Edge-oriented values. | Values near `-100`, `0`, and `100`; strings and containers may be empty or size `1`. |
| `safe` | Conservative values. | Positive numeric values, `true` booleans, shorter strings, and small non-empty containers. |
| `stress` | Larger generated data. | Longer strings and larger containers; numeric ranges are still moderate unless a rule supplies a value. |

Profiles do not attempt to prove path coverage. They only affect generated
fallback values.

## Determinism and Seeds

`--seed=<N>` enables deterministic generated fallback values when `N >= 0`.
With a seed, ASkeleTon derives per-parameter random streams from the function
name, invocation number, key name, and type. This helps keep generated values
stable when the same inputs and configuration are used.

Without `--seed`, fallback values use the process random source and can vary
between runs.

Generated `.cfg` metadata records the selected profile and, when present, the
seed:

```text
//// DATA PROFILE: random
//// SEED: 123
```

## Rule-Based Values

Rule-based generation is enabled by default. `--rule-data` explicitly enables
the same behavior, and `--no-rule-data` disables it. Rule-derived values are
computed from local AST patterns in the analyzed callable.

Currently documented rule sources include:

- Value comparisons such as `a > 10`, `a == 0`, or `a < -5`.
- Equality and inequality checks.
- Ranges joined with `&&` or `||`.
- Assignments from integer literals or constant integer expressions.
- `switch` cases with integer case labels.
- Numeric constant default arguments.
- Basic string literal default arguments.
- Enum, `const`, `constexpr`, and integer macro values when they can be
  resolved.
- Division and modulo patterns that need non-zero divisor candidates.

See [`DataRules.md`](DataRules.md) for the detailed candidate catalog.

If no rule-derived value is available for a parameter, ASkeleTon uses the next
available source in the precedence order below.

## Precedence and Interaction

For each generated case, parameter values are selected in this order:

1. Function-scoped string rule values for string-like parameters.
2. Function-scoped numeric rule values for non-container, non-record,
   non-string parameters.
3. Configured type factories that use `dummy` or `zeroed` strategies.
4. Default field initializer values discovered from the AST.
5. Profile-generated fallback values.

Additional notes:

- Full factory expressions can initialize objects directly in generated code; in
  those cases a `.cfg` value may not be emitted for that object.
- `--no-rule-data` disables rule collection even if `--rule-data` is also
  present.
- `--rule-max-cases=N` is normalized to at least `1`.
- Rule candidates for each parameter are de-duplicated as they are collected.
- If a parameter has more rule candidates than `--rule-max-cases`, ASkeleTon
  keeps a deterministic representative subset. The current selection prefers
  `0` when present, otherwise the smallest value, then adds distant values up
  to the limit.
- The number of generated invocations for a callable is the maximum rule
  candidate count after limiting, combined with any minimum case count required
  by supported structured inputs such as `std::optional<T>`.
- String rule values can supply values for generated invocations, but string
  rules by themselves do not currently increase the invocation count.
- When a parameter has fewer rule values than the invocation count, generated
  data cycles through that parameter's rule values.
- Parameters without rule values in the same callable use profile fallback
  values for each generated invocation.

## Example: Comparison-Derived Values

`examples/sut.cpp` contains:

```cpp
int classify(int a) {
    if (a > 10) return 1;
    if (a == 0) return 2;
    if (a < -5) return 3;
    return 0;
}
```

With rule-based data enabled, ASkeleTon generates multiple `classify` cases
because the comparisons provide candidate values for `a`. A representative
generated `.cfg` can include:

```text
classify_1:
{
	a=-6;#int

};

classify_2:
{
	a=0;#int

};

classify_3:
{
	a=11;#int

};
```

This is observed behavior in the repository example workflow.

## Example: Profile Fallback Values

`examples/sut.cpp` also contains:

```cpp
int add_i64(int64_t a, int64_t b) { return static_cast<int>(a + b); }
```

There are no comparisons or other supported rule patterns inside this function,
so `a` and `b` use profile-generated fallback values. With
`--profile=random --seed=123`, generated output before manual expected-value
editing can include:

```text
add_i64_1:
{
	a=72;#long
	b=-25;#long

};
```

The exact fallback values depend on the profile, seed, ASkeleTon version, and
other reproducibility inputs listed below.

## Reproducibility Requirements

For reproducible generated input data, keep these inputs the same:

- ASkeleTon version or commit.
- LLVM/Clang major version, for example LLVM/Clang 18.
- The exact `compile_commands.json` used by Clang Tooling.
- Source files and include paths visible to the AST.
- `--profile`, `--rule-data` or `--no-rule-data`, `--rule-max-cases`, and
  `--seed`.
- Configuration files under `data/`, especially `configuration.json`,
  `default_values.json`, `equivalent_types.json`, `type_factories.json`, and
  `instance_strategies.json`.
- `data/system_files.json`, or use `--no-system-files-refresh` when you need to
  avoid local refresh changes.

## Known Limitations

- Rule extraction is syntax-driven and local to supported AST patterns.
- ASkeleTon does not explore paths symbolically or concolically.
- Rule values currently target numeric parameters, with limited support for
  string literal defaults.
- Containers, maps, complex records, and unsupported template shapes usually use
  profile fallback values, configured factories, defaults, or are skipped.
- Profile fallback values are generated from moderate fixed ranges and should
  not be treated as exhaustive.
- Seeded output is intended to be stable for the same toolchain, source,
  configuration, and ASkeleTon version; changes to any of those inputs can
  legitimately change generated `.cfg` values.

## Related Guides

- Rule catalog: [`DataRules.md`](DataRules.md)
- `.cfg` format: [`CfgFormat.md`](CfgFormat.md)
- CLI reference: [`CLI.md`](CLI.md)
- Type factories: [`TypeFactories.md`](TypeFactories.md)
