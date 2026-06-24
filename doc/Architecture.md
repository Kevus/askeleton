# Architecture Overview

This document gives a high-level view of how ASkeleTon turns a
`compile_commands.json` entry into generated tests, fixtures, and `.cfg` data.

## End-to-End Flow

1. Clang Tooling analyzes the translation unit using `compile_commands.json`.
2. `ASKGen` discovers functions, methods, constructors, and AST-derived rule data.
3. A framework generator (`gtest`, `boost`, or `catch`) emits tests and fixtures.
4. `ConfigGenerator` writes the `.cfg` input data consumed by generated tests.
5. `RandomValuesGenerator` fills inputs using the selected profile.
6. Optional rule-based data augments generated cases with values inferred from
   comparisons, ranges, literals, and similar AST patterns.

## Main Components

- `src/askeleton.cpp`: CLI entry point, option parsing, and high-level orchestration.
- `src/ASKGen.cpp`: callable discovery, skip decisions, and AST-derived rule extraction.
- `src/Report.cpp`: generation report JSON assembly and aggregate diagnostics.
- `src/framework/*Gen.cpp`: framework-specific test and fixture generation.
- `src/ConfigGenerator.cpp`: `.cfg` generation and output layout.
- `src/RandomValuesGenerator.cpp`: profile-based value generation (`random`,
  `boundary`, `safe`, `stress`).

## Extension Points

- New AST rules: `ASKGen::collectRuleValuesFromFunction`
- New data profiles: `RandomValuesGenerator::setProfile`
- New framework emitters: classes under `src/framework/`

## Related Guides

- CLI options: [`CLI.md`](CLI.md)
- Input generation overview: [`InputGeneration.md`](InputGeneration.md)
- Rule-based generation details: [`DataRules.md`](DataRules.md)
- Report JSON schema: [`ReportSchema.md`](ReportSchema.md)
- Skip reasons: [`SkipReasons.md`](SkipReasons.md)
- Type customization: [`TypeFactories.md`](TypeFactories.md)
- Method instance setup: [`InstanceStrategies.md`](InstanceStrategies.md)
