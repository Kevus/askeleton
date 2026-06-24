# Expected Value Strategies

Generated tests need expected values so the test framework can compare the
callable result with something stable. ASkeleTon generates executable
scaffolding and editable test data, but it does not infer an independent
semantic correctness oracle for the system under test.

The current expected-value strategies are best understood as ways to create a
starting point:

- Explicit expected values supplied in `.cfg`.
- Mirrored execution of the same callable with isolated inputs.
- Property mode, currently implemented as repeatability-oriented replay.

## Strategy Summary

| CLI mode | Current behavior | Best use |
| --- | --- | --- |
| `--oracle-mode=explicit` | Generated tests check for `expected` in the `.cfg` when supported. If absent, they fall back to mirrored execution. | Default workflow: generate, run, then refine important cases with explicit expected values. |
| `--oracle-mode=mirror` | Generated tests derive `expected` from a second isolated execution of the same callable with the same case data. | Characterization baseline or smoke test scaffolding when explicit values are not ready yet. |
| `--oracle-mode=property` | Generated tests replay the same callable with isolated inputs and compare the observable result across both executions. | Repeatability-oriented checks. Treat current behavior as a characterization baseline, not a specification oracle. |

## Explicit Expected Values

In `explicit` mode, generated tests that support explicit expected values look
for an `expected` key in the matching `.cfg` case. For example:

```text
add_i64_1:
{
	a=72;#long
	b=-25;#long

	expected=47;#int

};
```

When the generated test checks `HasObject("<case>.expected")`, the value in the
`.cfg` overrides the mirrored fallback. This is the recommended way to turn
generated scaffolding into a more meaningful regression test:

1. Generate tests and inspect the `.cfg` inputs.
2. Decide the expected behavior for important cases.
3. Add or adjust the `expected` key.
4. Re-run the generated test.
5. Keep the refined `.cfg` with the generated test artifacts you intend to use.

For non-scalar or complex return types, inspect the generated test source before
adding `expected`. Some generated tests do not read `.cfg` expected values for
those return types.

## Mirrored Execution

Mirrored execution means the generated test initializes a second set of inputs
from the same `.cfg` case and calls the same callable again to compute the
comparison value. This can make generated tests executable immediately, and it
can serve as a characterization baseline for future changes.

Mirrored execution is not an independent correctness oracle. It asks whether
the current implementation behaves consistently for the generated case; it does
not decide whether that behavior is semantically correct.

Faults mirrored execution may fail to expose include:

- Deterministic wrong results that are reproduced by both calls.
- Missing or incorrect domain-specific requirements.
- Bugs that affect both the assertion call and the mirrored call in the same
  way.
- Incorrect behavior that only appears for inputs not generated in the `.cfg`.
- Problems masked by the generated fixture setup or by fallback values.

Mirrored execution may still expose useful issues, such as crashes,
unexpected mutations checked by generated assertions, or behavior that differs
between two isolated executions.

## Property Mode

`--oracle-mode=property` is currently implemented as repeatability-oriented
replay. The generated test replays the same callable with isolated inputs and
compares the observable result across both executions.

This mode is useful for smoke tests and repeatability checks, but it should not
be read as a proof of correctness. Like mirrored execution, it is a
characterization baseline unless the generated assertion matches a property the
user has reviewed and accepts.

## Recommended Usage

Default workflow:

1. Start with `--oracle-mode=explicit`, the default.
2. Build and run the generated tests.
3. Inspect the `.cfg` cases and generated test source.
4. Add explicit expected values for behavior that matters.
5. Re-run the tests and keep the refined data with the generated artifacts.

Add explicit expected values when:

- The expected result is known from requirements, examples, or trusted manual
  calculation.
- The case covers important boundary or regression behavior.
- The generated test checks `HasObject("<case>.expected")`.

Avoid relying only on mirrored behavior when:

- The callable implements business rules or domain-specific calculations.
- A deterministic bug could produce the same wrong answer twice.
- You need tests to detect semantic regressions, not only crashes or
  repeatability changes.
- The return type is a container or complex object and the generated test does
  not read explicit `.cfg` expected values.

## Related Guides

- CLI option details: [`CLI.md`](CLI.md)
- `.cfg` data format: [`CfgFormat.md`](CfgFormat.md)

