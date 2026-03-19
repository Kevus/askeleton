# Instance Strategies

This guide explains how ASkeleTon resolves instances for non-static methods and
how to override that behavior with `data/instance_strategies.json`.

## When to Use This

Use `data/instance_strategies.json` when ASkeleTon cannot infer a safe way to
construct `this` for an instance method.

This is especially useful when:

- the target type has no usable public constructor
- object creation must go through a factory
- method setup depends on an owner object or factory chain

## Resolution Order

For non-static methods, ASkeleTon resolves the test instance using this
cascade:

1. `data/instance_strategies.json` override for the exact method
2. `data/instance_strategies.json` override for the target type
3. Public usable constructor on the class
4. Public `static` factory on the class returning the same type by value
5. Externally visible free factory in the same scope returning the same type by value
6. Public instance method on a resolvable owner type returning the same type by value

The first usable strategy wins.

## Configuration File

Example:

```json
{
  "types": {
    "Widget": { "expr": "MakeWidget()", "subject": "pointer" }
  },
  "functions": {
    "Widget::Run": {
      "instance": {
        "owner_type": "WidgetFactory",
        "owner_expr": "WidgetFactory{}",
        "callable": "CreateReady()",
        "owner_subject": "value",
        "subject": "pointer"
      }
    }
  }
}
```

## Field Notes

- `types` applies to all instance methods of that class unless a function-level
  override exists.
- `functions` applies to the exact method key and takes precedence over `types`.
- `expr` is emitted as the direct initialization expression for the test object.
- `subject` may be `value`, `pointer`, or `reference` and controls whether the
  generated method call uses `.` or `->`.
- `owner_type`, `owner_expr`, and `callable` define an explicit owner factory chain.
- `owner_subject` controls whether the owner itself is treated as a value or pointer subject.
- Owner factories are inferred only after direct strategies fail, so constructor
  and direct factory paths still win when they are available.

## Related Guides

- Skip reasons: [`SkipReasons.md`](SkipReasons.md)
- Type customization: [`TypeFactories.md`](TypeFactories.md)
