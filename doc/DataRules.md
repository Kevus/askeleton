# Data Rules

This guide describes the AST-derived rules used by ASkeleTon when rule-based
generation is enabled.

For the broader interaction between profiles, rule data, seeds, fallback values,
and reproducibility, see [`InputGeneration.md`](InputGeneration.md).

## Activation

- Basic AST-derived rules are enabled by default.
- `--rule-data` explicitly enables the same behavior. This is mostly useful for
  parity checks in scripts or CI.
- `--no-rule-data` disables AST-derived rules and falls back to other data
  sources such as profiles, factories, and defaults.
- `--rule-max-cases=N` limits how many rule-generated cases are kept per function
  when multiple candidates are available. The default is `3`.
- Candidate selection is deterministic and keeps a representative subset when
  more candidates are detected than `--rule-max-cases` allows.

Example:

```bash
askeleton --rule-data --rule-max-cases=3 -p build src/foo.cpp
```

## Rule Catalog

### Value Comparisons

Example:
- `if (a > 10)`
- `if (a == 0)`

Generated candidates:
- `9`, `10`, `11`, and `0`

### Ranges with `&&` or `||`

Example:
- `if (a >= 10 && a < 20)`
- `if (a <= -2 || a >= 6)`

Generated candidates:
- `10`, `11`, `19`, `18` for the bounded range
- `-3`, `-2`, `-1` and `6`, `7` for the `OR` case

### Assignments with Literal Values

Example:
- `a = 2 + 2;`

Generated candidates:
- `3`, `4`, `5`

### `switch` Cases

Example:
- `switch(a) { case 1: ... case 3: ... }`

Generated candidates:
- `1`, `3`, and `(max + 1)`

### Default Parameters

Status:
- Implemented for numeric constant defaults and basic string literals.

Example:
- `int f(int a = 5)`

Generated candidates:
- `4`, `5`, `6`

String example:
- `int f(std::string s = "foo")`

Generated candidates:
- `"foo"`, `"foo_alt"`

### Constants and Enums

Example:
- `if (mode == ModeOn)`
- `if (v >= kLimit)`

Behavior:
- Resolves enums, `const` and `constexpr` values, plus integer macros.

### Modulo and Division Operators

Example:
- `if (a % 2 == 0)`
- `if (a % 3 != 0)`
- `x = 10 / b`

Generated candidates:
- `modulo == 0` -> `0`, `d`, `2d`
- `modulo != 0` -> `1`, `d + 1`
- When the divisor is a parameter, division and modulo avoid `0` with values
  such as `1`, `2`, and `-1`

## Notes and Limits

- These rules currently apply only to non-container numeric parameters unless a
  specialized string rule is available.
- Strings, maps, and complex records still fall back to profile-based generation
  or configured factories when no rule applies.

## Related Guides

- Input generation overview: [`InputGeneration.md`](InputGeneration.md)
- CLI options: [`CLI.md`](CLI.md)
- Type customization: [`TypeFactories.md`](TypeFactories.md)
