Data Rules (ASkeleTon)

Activation
- Basic AST-derived rules are enabled by default.
- `--rule-data` explicitly enables the same behavior (useful for parity checks in
  scripts and CI).
- `--rule-max-cases=N` limits the number of generated cases per function when
  multiple candidates are available (default: 3).
- Candidate selection is deterministic and keeps a representative subset when
  more candidates are detected than `--rule-max-cases` allows.

Example
`askeleton --rule-data --rule-max-cases=3 -p build src/foo.cpp`

Rule 1: Value comparisons
Example:
- `if (a > 10)`
- `if (a == 0)`

Generated candidates:
- `9`, `10`, `11`, and `0`

Rule 2: Ranges with `&&` or `||`
Example:
- `if (a >= 10 && a < 20)`
- `if (a <= -2 || a >= 6)`

Generated candidates:
- `10`, `11`, `19`, `18` for the bounded range
- `-3`, `-2`, `-1` and `6`, `7` for the `OR` case

Rule 3: Assignments with literal values
Example:
- `a = 2 + 2;`

Generated candidates:
- `3`, `4`, `5`

Rule 4: `switch` cases
Example:
- `switch(a){ case 1: ... case 3: ... }`

Generated candidates:
- `1`, `3`, and `(max + 1)`

Rule 5: Default parameters
Status:
- Implemented for numeric constant defaults.

Example:
- `int f(int a = 5)`

Generated candidates:
- `4`, `5`, `6`

For string literals:
- `int f(std::string s = "foo")`

Generated candidates:
- `"foo"`, `"foo_alt"`

Rule 6: Constants and enums
Example:
- `if (mode == ModeOn)`
- `if (v >= kLimit)`

Behavior:
- Resolves enums, `const`/`constexpr` values, and integer macros.

Rule 7: Modulo and division operators
Example:
- `if (a % 2 == 0)`
- `if (a % 3 != 0)`
- `x = 10 / b`

Generated candidates:
- `modulo == 0` -> `0`, `d`, `2d`
- `modulo != 0` -> `1`, `d + 1`
- division/modulo when the divisor is a parameter -> avoids `0` with `{1, 2, -1}`

Notes
- These rules apply only to non-container numeric parameters.
- Strings, maps, and complex records still fall back to random/profile-driven
  generation unless another strategy is configured.
