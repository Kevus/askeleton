# `.cfg` Test Data Format

ASkeleTon writes one `.cfg` file next to each generated test fixture. The file
stores editable test data; the generated C++ fixture and test files store the
test logic that reads that data.

Use this file when you want to inspect generated inputs, adjust a generated
case, add an explicit expected value, or diagnose a generated test that fails
after a manual edit.

## Scope of this format guide

This guide documents:

- Syntax observed in generated `.cfg` files.
- Additional supported shapes based on current generated fixture/parser
  behavior.
- Current limitations that users should not rely on as stable design
  guarantees.

The safest source of truth for any generated test is the generated `.cfg`
together with the generated test and fixture source that read it.

## Where It Fits

Default output for `examples/sut.cpp` is:

```text
examples/tests/generated/sut/
  sut.cfg
  sut_fixture.hpp
  sut_test.cpp
  Makefile
```

The fixture parses `sut.cfg` at test runtime. Regenerating tests rewrites the
generated output, so keep manual `.cfg` edits in the generated output you intend
to run.

## Basic Syntax

Generated files use case blocks:

```text
case_name_1:
{
	key=value;#type

};
```

| Part | Meaning |
| --- | --- |
| `case_name_1:` | Case key. Generated keys usually use the callable name plus an invocation number. |
| `{` | Opens the case block. It must appear as a line by itself. |
| `key=value;#type` | Entry read by generated fixture helpers. |
| `};` | Closes the case block. It must appear as a line by itself. |

Entry details:

- The value is the text between `=` and the first `;`.
- The `#type` suffix is generated type metadata for readers. Runtime parsing
  does not validate the suffix against the C++ type.
- Full-line comments beginning with `#` are skipped by the parser.
- Generated header lines beginning with `////` are not user comments; avoid
  adding new `//` comment lines inside case blocks.
- The parser is simple. Keep case headers, `{`, and `};` flush with the left
  margin. Generated entries use a leading tab; tabs are stripped from keys, but
  spaces in keys are not.
- Blank lines are allowed.

## Primitive Values

Generated fixtures include read helpers for these scalar families:

| C++ type family | Example `.cfg` value |
| --- | --- |
| `bool`, `_Bool` | `flag=true;#bool` |
| character types | `c=a;#char` |
| signed and unsigned integer types | `n=-1;#int`, `count=4;#unsigned` |
| `short`, `long`, `long long` variants | `total=22;#long` |
| `float`, `double`, `long double` | `ratio=1.5;#double` |
| `std::string`, `string` | `name=abc;#std::string` |
| `char *` | `buffer=abc;#char *` |

ASkeleTon canonicalizes several aliases before generation, including fixed-width
integer aliases such as `int32_t` and `uint64_t`, and common signed/unsigned
spellings.

## Generated Example

The following case shape is observed in output from
`scripts/check_main_workflow.sh`:

```text
scale_vec_1:
{
	v={67};#std::vector<int>
	factor=71;#int

};
```

The same workflow also generates scalar inputs, record fields such as
`p.x=86;#int`, and an explicit expected value when the workflow refines
`add_i64_1`:

```text
add_i64_1:
{
	a=72;#long
	b=-25;#long

	expected=999;#int

};
```

## Supported Structured Shapes

Structured values are expanded into multiple keys. The generated fixture reads
the same key paths. The shapes below are supported by current generated
readers/generators and may be subject to the limitations listed later.

### Records

Public record fields are flattened with dot notation:

```text
translate_1:
{
	p.x=-69;#int
	p.y=-19;#int

	dx=2;#int
	dy=-75;#int

};
```

Record field syntax was observed in the generated workflow output.

### `std::optional<T>`

Optional values use a presence key plus a nested `value` key when present:

```text
value.has_value=true;#bool
value.value=39;#int
```

For an empty optional:

```text
value.has_value=false;#bool
```

Generated data alternates optional presence when multiple invocations are
created for the callable.

### `std::pair<T, U>`

Pairs use `first` and `second`:

```text
range.first=1;#int
range.second=10;#int
```

### `std::tuple<Ts...>`

Tuples use zero-based element keys:

```text
point.0=4;#int
point.1=5;#short
point.2=6;#long long
```

### Containers

Generated fixtures also include readers for `std::vector<T>`, `std::list<T>`,
and `std::map<K, V>` when the element types can be parsed by the generated
helpers.

Vectors and lists use brace-wrapped comma-separated values:

```text
scale_vec_1:
{
	v={-100,80,-30,-37,10,29,1,53};#std::vector<int>
	factor=23;#int

};
```

Maps use brace-wrapped `(key,value)` pairs:

```text
counts={(1,2),(3,4)};#std::map<int, int>
```

Container parsing is intentionally simple. Primitive element, key, and value
types are safest. Nested containers or complex nested structures should not be
assumed to work unless validated in the generated test.

## Expected Values

In the default `explicit` expected-value strategy, generated tests first check
whether the case has an `expected` entry for return types and generated tests
that support that path. When the generated test checks
`HasObject("<case>.expected")`, an `expected` entry can override the mirrored
result. If it is absent, the test falls back to an isolated replay of the same
callable with the same case data.
This fallback is a characterization baseline, not an independent correctness
oracle.

Before manual refinement:

```text
add_i64_1:
{
	a=22;#long
	b=25;#long

};
```

After adding an explicit expected value:

```text
add_i64_1:
{
	a=22;#long
	b=25;#long
	expected=47;#int

};
```

Notes:

- Use the key name `expected`.
- The type suffix should match the callable return type for readability, but
  the generated read helper is selected from the C++ return type.
- Explicit `expected` overrides are available for generated tests that check
  `HasObject("<case>.expected")`.
- Some generated tests, including certain container-return cases, may use
  mirrored execution directly and never read an `expected` key.
- Inspect the generated test source before adding expected values for
  non-scalar or complex return types.
- Explicit expected values are not used for `void`, list, or map return types
  in current generated code paths. They may also be unavailable if the return
  type contains an unsupported field or nested type.
- For pointer returns, generated explicit expected values represent pointer
  presence as `bool`.
- For C string returns, generated explicit expected values are read as
  `std::string`.
- For expected-value strategy details, see
  [`ExpectedValues.md`](ExpectedValues.md).

## Invalid Or Unsupported Values

Most `.cfg` validation happens when the generated test binary runs.

| Edit problem | Typical result |
| --- | --- |
| Missing ordinary input key | Current generated readers may fall back to empty or zero-like values for missing scalar keys. Users should not rely on this behavior. Generated keys should be preserved unless the corresponding generated test code is also inspected and updated. |
| Missing `expected` | In `explicit` mode, generated tests use the mirror fallback. |
| Invalid numeric text | Standard conversions such as `stoi`, `stol`, `stod`, or Boost lexical casts can throw and fail the generated test. |
| Invalid boolean text | `true` reads as `true`; any other value reads as `false`. |
| Bad case header or closing line whitespace | The parser may not recognize the case boundary. |
| Unsupported source type | ASkeleTon usually skips generation for that callable and records a skip reason in the report. |
| Unsupported explicit expected type | The generated test does not look for `expected`; mirror fallback is used. |

## Safe editing checklist

1. Confirm the block name matches the generated test case key, for example
   `add_i64_1`.
2. Keep `{` and `};` on their own left-aligned lines.
3. Check that each entry contains `=` and a terminating `;`.
4. Keep generated case names unchanged.
5. Keep generated keys unless inspecting generated test code.
6. Add `expected` only when the generated test checks for it.
7. Compare the edited block with the generated fixture read calls in
   `*_fixture.hpp` or the generated test body.
8. Re-run the generated test from its output directory so it reads the edited
   `.cfg` next to the binary.
9. Use the report and test failure messages to diagnose invalid edits.
10. If ASkeleTon skipped generation, inspect the JSON report or
    [`SkipReasons.md`](SkipReasons.md).
