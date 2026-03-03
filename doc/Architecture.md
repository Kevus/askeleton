ASkeleTon - Architecture Overview

General flow
1. Clang Tooling analyzes the AST using `compile_commands.json`.
2. `ASKGen` discovers functions, methods, and constructors.
3. `Generator` (Boost/Catch/GTest) creates test and fixture files.
4. `ConfigGenerator` writes the `.cfg` input data.
5. `RandomValuesGenerator` fills data using random values or a selected profile.
6. `--rule-data` rules inject values derived from AST patterns when applicable.

Main components
- `src/askeleton.cpp`: CLI and global orchestration.
- `src/ASKGen.cpp`: callable discovery plus AST-derived rules.
- `src/framework/*Gen.cpp`: framework-specific test generators.
- `src/ConfigGenerator.cpp`: `.cfg` generation.
- `src/RandomValuesGenerator.cpp`: data generation (`random`, `boundary`, `safe`, `stress`).

Extension points
- New AST rules: `ASKGen::collectRuleValuesFromFunction`.
- New data profiles: `RandomValuesGenerator::setProfile`.
- New output formats: framework classes under `src/framework/`.
