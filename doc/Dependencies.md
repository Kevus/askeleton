# Dependencies

This guide summarizes the toolchain and optional libraries commonly used to
build and run ASkeleTon.

## Recommended Core Packages

These package names match the current `Makefile` expectations on Ubuntu-based
systems:

- `clang-18`
- `llvm-18`
- `llvm-config-18`
- `libclang-18-dev`

## Build Toolchain

- `g++-12` and `gcc-12` are the preferred GCC toolchain for this setup.
- GCC 11 is not supported by the current build configuration.

## Test Framework Packages

- `libgtest-dev`: enough for the default setup
- `libboost-dev`: needed for Boost.Test output
- `catch2`: needed for Catch2 output

## Notes

- LLVM and Clang versions should match the versions expected by the `Makefile`.
- Not every environment needs every optional package.

## Useful Scripts

- Sanity checks: [`scripts/sanity_types.sh`](../scripts/sanity_types.sh)
- Broader verification: [`scripts/check_all.sh`](../scripts/check_all.sh)

## Related Guides

- Main install overview: [`README.md`](../README.md)
- CLI reference: [`CLI.md`](CLI.md)
