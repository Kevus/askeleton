# Skip Reasons Guide

This guide explains the `reason` field used in ASkeleTon JSON reports when an
entity is marked as `skipped`.

Each section includes:

- what it means
- a minimal example
- typical fixes

## `abstract_record`

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

## `non_public_lifecycle`

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
- Provide a project-specific creation or ownership strategy.

## `missing_fixture_strategy`

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

## `missing_instance_strategy`

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

## `unsupported_framework_feature`

Meaning: the selected framework does not support a required generation feature
for this entity.

Example:

- A legacy or custom framework path requested a feature that is not emitted for
  that generator.

Typical fixes:

- Verify that you are using one of the built-in `gtest`, `boost`, or `catch`
  generators.
- If the reason appears unexpectedly, verify that you are running a current build.

## `unusable_constructor`

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

## `coverage_policy_mutable_parameter`

Meaning: under `--coverage-mode=strict`, mutable pointer or reference parameters
are skipped to keep scaffolding conservative.

Example:

```cpp
int f(int *outValue);
```

Typical fixes:

- Use `--coverage-mode=balanced`.
- Add explicit handling or factories for mutable parameters.

## `coverage_policy_instance_construction`

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

## `unsupported_indirection`

Meaning: the type requires unsupported pointer or reference depth, or nested
indirections.

Example:

```cpp
int f(char **argv);
```

Typical fixes:

- Wrap or flatten the API for test generation.
- Provide a higher-level adapter.

## `unsupported_pointer_pointee`

Meaning: the pointer pointee type is not currently materializable automatically.

Example:

```cpp
int g(CustomOpaque *p);
```

Typical fixes:

- Configure type factories or stubs.
- Replace opaque pointer inputs with supported wrappers in a test adapter.

## `unsupported_array_shape`

Meaning: the array shape is not supported by the automatic materialization path.

Example:

```cpp
void h(int matrix[4][4]);
```

Typical fixes:

- Adapt the signature to a container or wrapper type.
- Provide a custom reader or factory strategy.

## `unsupported_template_parameter`

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

## `unsupported_type`

Meaning: the type shape is not supported by the current model.

Example:

```cpp
int f(int (...)(double));
```

Typical fixes:

- Use adapter functions with simpler signatures.
- Add explicit support or a factory for that shape.

## `incomplete_type`

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

## `incomplete_record`

Meaning: a record was detected but its full definition is not available to the
generator.

Example:

```cpp
class Impl;
```

Typical fixes:

- Include headers providing the full definition.
- Prefer APIs using complete public data types for generated tests.

## Related Guides

- Type customization: [`TypeFactories.md`](TypeFactories.md)
- Instance construction: [`InstanceStrategies.md`](InstanceStrategies.md)
- CLI reference: [`CLI.md`](CLI.md)
