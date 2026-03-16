# Skip Reasons Guide

This guide explains the `reason` field used in ASkeleTon JSON reports when an
entity is marked as `skipped`.

Each section includes:
- What it means
- A minimal example
- Typical fixes

## `abstract_record`
Meaning: A class/struct is abstract (has pure virtual methods), so ASkeleTon
cannot instantiate it for fixture data.

Example:
```cpp
struct IParser {
    virtual int parse() = 0;
};
```

Typical fixes:
- Pass a concrete derived type instead of the abstract base.
- Add a factory in `data/type_factories.json` that returns a concrete instance.

## `non_public_lifecycle`
Meaning: The type has a non-public or deleted destructor, so fixture lifetime
management is unsafe.

Example:
```cpp
class Token {
private:
    ~Token() = default;
};
```

Typical fixes:
- Use a type with public usable destructor for generated data.
- Provide project-specific creation/ownership strategy.

## `missing_fixture_strategy`
Meaning: A record type has no usable default construction path and no explicit
factory strategy configured.

Example:
```cpp
struct User {
    User(int id);
};
```

Typical fixes:
- Add a factory in `data/type_factories.json`.
- Expose a default constructor when appropriate.

## `missing_instance_strategy`
Meaning: For an instance method, ASkeleTon cannot build a valid instance plan
for `this` (constructor/factory path not found or not usable).

Example:
```cpp
class Service {
    Service() = delete;
public:
    int run();
};
```

Typical fixes:
- Add constructor/factory strategy in `data/instance_strategies.json`.
- Make a usable public construction path available.

## `unsupported_framework_feature`
Meaning: The selected framework does not support a required generation feature
for this entity.

Example:
- Legacy/custom framework path requested a feature that is not emitted for that
  generator.

Typical fixes:
- For the built-in `gtest`, `boost`, and `catch` generators, constructor tests
  are supported in the current codebase, so this reason should be uncommon.
- If it appears, verify that you are running the current generator build and
  not an older binary or downstream/custom framework variant.

## `coverage_policy_mutable_parameter`
Meaning: Under `--coverage-mode=strict`, mutable pointer/reference params are
skipped to keep scaffolding conservative.

Example:
```cpp
int f(int *outValue);
```

Typical fixes:
- Use `--coverage-mode=balanced` or `--coverage-mode=aggressive`.
- Add explicit handling/factories for mutable parameters.

## `coverage_policy_instance_construction`
Meaning: Under `strict` mode, instance creation requires non-default
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
- Use `balanced`/`aggressive`.
- Add explicit instance strategy with predictable setup.

## `unsupported_indirection`
Meaning: Type requires unsupported pointer/reference depth or nested
indirections.

Example:
```cpp
int f(char **argv);
```

Typical fixes:
- Wrap/flatten API for test generation.
- Provide higher-level adapters.

## `unsupported_pointer_pointee`
Meaning: Pointer pointee type is not currently materializable automatically.

Example:
```cpp
int g(CustomOpaque *p);
```

Typical fixes:
- Configure type factories/stubs.
- Replace opaque pointer inputs with supported wrappers in a test adapter.

## `unsupported_array_shape`
Meaning: Array shape is not supported by automatic materialization path.

Example:
```cpp
void h(int matrix[4][4]);
```

Typical fixes:
- Adapt signature to container/wrapper type.
- Provide custom reader/factory strategy.

## `unsupported_template_parameter`
Meaning: Dependent template parameter cannot be materialized into concrete test
data in the current context.

Example:
```cpp
template <typename T>
T id(T value);
```

Typical fixes:
- Instantiate and test concrete wrappers (`int`, `std::string`, etc.).
- Skip generic entry points and test instantiated API surface.

## `unsupported_type_shape`
Meaning: Type shape is not supported by current model (complex/irregular case).

Example:
```cpp
int f(int (...)(double)); // complex function type
```

Typical fixes:
- Use adapter functions with simpler signatures.
- Add explicit support/factory for that shape.

## `incomplete_type`
Meaning: The type is forward-declared or incomplete where materialization is
needed.

Example:
```cpp
struct sqlite3; // forward declaration only
int db_close(sqlite3 *db);
```

Typical fixes:
- Ensure full definition is visible where generation requires it.
- Use adapter layer so generated tests interact with complete types.

## `incomplete_record`
Meaning: Record was detected but its full definition is not available to the
generator.

Example:
```cpp
class Impl; // no definition in translation unit
```

Typical fixes:
- Include headers providing full definition.
- Prefer APIs using complete/public data types for generated tests.
