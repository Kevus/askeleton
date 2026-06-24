# Skip Reasons Guide

This guide explains the `reason` field used in ASkeleTon JSON reports when an
entity is marked as `skipped`. Skip reasons describe deliberate generation
boundaries or missing project-specific setup. They do not necessarily indicate
that the source code is wrong.

For the full report layout, see [`ReportSchema.md`](ReportSchema.md).

## How To Use Skip Reasons

1. Open the report written by `--report=<path>` or `--report-json`.
2. Find the skipped entry in `items`.
3. Use `signature`, `file`, `line`, `reason`, and `detail` to identify the
   unsupported type, callable, or policy boundary.
4. Apply one of the suggested remediations below, then regenerate.

Summary fields such as `summary.by_reason` help prioritize recurring causes.
Entity-level fields in `items` tell you what to fix for one callable.

## Quick Reference

| Reason | Meaning | User-fixable? | Common next step |
| --- | --- | --- | --- |
| `abstract_record` | A fixture or instance would require instantiating an abstract class. | Usually | Use a concrete wrapper or configure a factory/instance strategy. |
| `non_public_lifecycle` | Fixture lifetime would require a non-public or unusable destructor. | Usually | Use a public lifecycle type or configure project-specific construction. |
| `missing_fixture_strategy` | A record parameter cannot be default-constructed and has no configured factory. | Yes | Add a type factory in `data/type_factories.json`. |
| `missing_instance_strategy` | A non-static method has no usable `this` construction plan. | Yes | Add an instance strategy in `data/instance_strategies.json`. |
| `coverage_policy_mutable_parameter` | `strict` mode skipped mutable pointer/reference handling. | Yes | Use `balanced`, add an adapter, or manually refine the generated test. |
| `coverage_policy_instance_construction` | `strict` mode skipped non-default instance construction. | Yes | Use `balanced` or add an explicit instance strategy. |
| `unusable_constructor` | The constructor target is deleted, non-public, implicit-only, or copy/move-only. | Sometimes | Test methods instead, expose a public construction path, or add an instance strategy. |
| `unsupported_framework_feature` | The selected backend cannot emit a needed test form. | Sometimes | Try another built-in framework or inspect the generator support boundary. |
| `unsupported_indirection` | Pointer/reference nesting is deeper than ASkeleTon materializes automatically. | Usually | Add an adapter or simplify the generated-facing signature. |
| `unsupported_pointer_pointee` | A pointer pointee cannot be materialized automatically. | Usually | Add a type factory, stub, or adapter. |
| `unsupported_array_shape` | The array shape is outside the automatic materialization path. | Usually | Wrap the input in a supported container or adapter. |
| `unsupported_template_parameter` | A dependent template parameter remains unresolved. | Usually | Generate tests for concrete instantiations or wrapper functions. |
| `unsupported_type` | The type shape is outside the current support model. | Usually | Use an adapter, factory, or manual test. |
| `incomplete_type` | A type is incomplete where generated data would need a definition. | Yes | Include the full definition or adapt the API. |
| `incomplete_record` | A record declaration is visible but its full definition is not. | Yes | Include the defining header or adjust compile flags/include paths. |

## Detailed Guidance

### `abstract_record`

Meaning: a class or struct is abstract, so ASkeleTon cannot instantiate it for
fixture data.

Example:

```cpp
struct IParser {
    virtual int parse() = 0;
};
```

Typical fixes:

- Pass a concrete derived type instead of the abstract base.
- Add a factory in [`data/type_factories.json`](../data/type_factories.json) that returns a concrete instance.
- For non-static methods, add an instance strategy in
  [`data/instance_strategies.json`](../data/instance_strategies.json) when the
  method can be exercised through a concrete owner or factory.

### `non_public_lifecycle`

Meaning: the type has a non-public or deleted destructor, so fixture lifetime
management is unsafe.

Example:

```cpp
class Token {
private:
    ~Token() = default;
};
```

Typical fixes:

- Use a type with a public usable destructor for generated data.
- Provide a project-specific creation or ownership strategy when lifetime is
  managed outside the generated fixture.

### `missing_fixture_strategy`

Meaning: a record type has no usable default construction path and no explicit
factory strategy configured.

Example:

```cpp
struct User {
    User(int id);
};
```

Typical fixes:

- Add a factory in [`data/type_factories.json`](../data/type_factories.json).
- Expose a default constructor when appropriate.
- Use an adapter function that accepts simpler supported inputs when the record
  should not be generated directly.

### `missing_instance_strategy`

Meaning: for an instance method, ASkeleTon cannot build a valid instance plan
for `this`.

Example:

```cpp
class Service {
    Service() = delete;
public:
    int run();
};
```

Typical fixes:

- Add a strategy in [`data/instance_strategies.json`](../data/instance_strategies.json).
- Make a usable public construction path available.
- Use a static wrapper or free-function adapter when a method requires complex
  project setup.

### `unsupported_framework_feature`

Meaning: the selected framework does not support a required generation feature
for this entity.

Example:

- A legacy or custom framework path requested a feature that is not emitted for
  that generator.

Typical fixes:

- Verify that you are using one of the built-in `gtest`, `boost`, or `catch`
  generators.
- Try another framework backend if the callable shape is important.
- If the reason appears unexpectedly, verify that you are running a current build.

### `unusable_constructor`

Meaning: the constructor itself is not callable as a generated test target
because it is deleted, non-public, implicit-only, or copy/move-only.

Example:

```cpp
class Service {
public:
    Service() = delete;
};
```

Typical fixes:

- Expose a public usable constructor to test directly.
- Prefer method-level generation with an explicit instance strategy when the
  constructor should not be exercised directly.
- Leave the constructor ungenerated and add manual tests when the construction
  path is intentionally unavailable.

### `coverage_policy_mutable_parameter`

Meaning: under `--coverage-mode=strict`, mutable pointer or reference parameters
are skipped to keep scaffolding conservative.

Example:

```cpp
int f(int *outValue);
```

Typical fixes:

- Use `--coverage-mode=balanced`.
- Add explicit handling or factories for mutable parameters.
- Introduce a small adapter with value-style inputs and outputs when pointer or
  reference mutation is part of the production API.

### `coverage_policy_instance_construction`

Meaning: under `strict` mode, instance creation requires non-default
construction, so generation is skipped.

Example:

```cpp
class C {
public:
    C(int seed);
    int m();
};
```

Typical fixes:

- Use `balanced`.
- Add an explicit instance strategy with predictable setup.
- Keep `strict` for conservative sweeps, then run targeted `balanced` generation
  for callables that require richer setup.

### `unsupported_indirection`

Meaning: the type requires unsupported pointer or reference depth, or nested
indirections.

Example:

```cpp
int f(char **argv);
```

Typical fixes:

- Wrap or flatten the API for test generation.
- Provide a higher-level adapter.
- Add manual tests for APIs where nested pointer ownership is central to the
  behavior.

### `unsupported_pointer_pointee`

Meaning: the pointer pointee type is not currently materializable automatically.

Example:

```cpp
int g(CustomOpaque *p);
```

Typical fixes:

- Configure type factories or stubs.
- Replace opaque pointer inputs with supported wrappers in a test adapter.
- Add explicit include and link flags when the type is supported by a project
  library but not visible or linkable in the generated target.

### `unsupported_array_shape`

Meaning: the array shape is not supported by the automatic materialization path.

Example:

```cpp
void h(int matrix[4][4]);
```

Typical fixes:

- Adapt the signature to a container or wrapper type.
- Provide a custom reader or factory strategy.
- Manually refine generated tests when the exact array layout is important.

### `unsupported_template_parameter`

Meaning: a dependent template parameter cannot be materialized into concrete
test data in the current context.

Example:

```cpp
template <typename T>
T id(T value);
```

Typical fixes:

- Instantiate and test concrete wrappers such as `int` or `std::string`.
- Skip generic entry points and test the instantiated API surface instead.

### `unsupported_type`

Meaning: the type shape is not supported by the current model.

Example:

```cpp
int f(int (...)(double));
```

Typical fixes:

- Use adapter functions with simpler signatures.
- Add explicit support or a factory for that shape.
- Check `detail` for the exact type string ASkeleTon could not materialize.

### `incomplete_type`

Meaning: the type is forward-declared or incomplete where materialization is
needed.

Example:

```cpp
struct sqlite3;
int db_close(sqlite3 *db);
```

Typical fixes:

- Ensure the full definition is visible where generation requires it.
- Use an adapter layer so generated tests interact with complete types.
- Check `compile_commands.json`, include directories, and source selection.

### `incomplete_record`

Meaning: a record was detected but its full definition is not available to the
generator.

Example:

```cpp
class Impl;
```

Typical fixes:

- Include headers providing the full definition.
- Prefer APIs using complete public data types for generated tests.
- Add explicit include flags if the generated translation unit cannot see the
  same headers as the original source.

## Related Guides

- Report JSON schema: [`ReportSchema.md`](ReportSchema.md)
- Type customization: [`TypeFactories.md`](TypeFactories.md)
- Instance construction: [`InstanceStrategies.md`](InstanceStrategies.md)
- CLI reference: [`CLI.md`](CLI.md)
