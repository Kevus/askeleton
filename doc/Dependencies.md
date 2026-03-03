Updated: 2026-02-04

Recommended (aligned with the current `Makefile`)
- `clang-18`
- `llvm-18`
- `llvm-config-18`
- `libclang-18-dev`

Optional
- `g++-12` and `gcc-12` (GCC 11 is not supported by this setup)
- `libboost-dev`

Notes
- LLVM/Clang versions should match the versions expected by the `Makefile`.
- You may not need every package in every environment.

Quick checks
- `scripts/sanity_types.sh`
