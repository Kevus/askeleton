Updated: 2026-02-04

- When a `.h` file is analyzed instead of a `.hpp`, Clang may parse it as C
  rather than C++. Workaround: add `-xc++`. Status: active.
- Although the main generation directory is configurable from the CLI, older
  generator assumptions may still need confirmation in edge cases. Status: to be
  confirmed.
- Intermittent `.cfg` open errors when the output folder did not exist. Status:
  mitigated (`ConfigGenerator` now creates the directory, validated on
  2026-02-04).
- Crash when processing container types without visible template arguments (for
  example `std::vector` without `<T>`). Status: mitigated (guards in
  `RandomValuesGenerator` and template-argument preservation in `InfoType`,
  validated on 2026-02-04).
- `map` reads with extra comparators/allocators in template signatures. Status:
  mitigated (`Read_map` now uses only key/value, validated on 2026-02-04).
- Value generation for `map` with `vector` values. Status: mitigated (`map`
  detection now runs before `vector`, validated on 2026-02-04).
- Types such as `int64_t` being transformed into malformed names like `int64 t`.
  Status: mitigated (equivalences added and validated with a minimal case on
  2026-02-04).
