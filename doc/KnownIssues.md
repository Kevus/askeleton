# Known Issues

This page tracks active issues and caveats that may affect users of ASkeleTon.

## Active Issues

- When a `.h` file is analyzed instead of a `.hpp`, Clang may parse it as C
  rather than C++. Workaround: add `-xc++`.

## Under Observation

- Although the main generation directory is configurable from the CLI, some
  older generator assumptions may still need confirmation in edge cases.

## Notes

- Issues that are already mitigated are intentionally not listed here to keep
  this page focused on current user-facing caveats.

## Related Guides

- CLI reference: [`CLI.md`](CLI.md)
- Troubleshooting overview: [`README.md`](../README.md)
