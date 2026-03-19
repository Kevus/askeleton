# Type Factories

This guide explains how to customize complex type initialization with
`data/type_factories.json`.

## When to Use This

Use type factories when ASkeleTon cannot build sensible fixture values for a
project-specific or non-trivial type on its own.

Typical cases:

- records without a usable default constructor
- domain types that require a factory function
- inputs that need stable project-specific dummy values

## File Shape

```json
{
  "types": {},
  "functions": {}
}
```

## Example

```json
{
  "types": {
    "User": { "strategy": "factory", "expr": "MakeUser(\"guest\")" },
    "Session": { "strategy": "zeroed" },
    "Address": { "strategy": "dummy" }
  },
  "functions": {
    "BuildAdminSession": {
      "types": {
        "User": { "strategy": "factory", "expr": "MakeAdminUser()" }
      }
    }
  }
}
```

## Supported Strategies

- `factory`: uses the expression in the generated `Read_<Type>()` fixture method.
- `zeroed`: returns `{}` for record types.
- `dummy`: uses in-class defaults, then `data/default_values.json`, then zero.

## Scope Rules

- `types` applies globally to that type unless a function-level override exists.
- `functions` lets you override a type strategy only for a specific function or
  method.
- Function-scoped factories currently support explicit `factory` expressions.

## Related Guides

- Skip reasons: [`SkipReasons.md`](SkipReasons.md)
- Instance construction: [`InstanceStrategies.md`](InstanceStrategies.md)
