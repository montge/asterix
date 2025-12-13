## Why

SonarCloud analysis reveals 5,933 code quality issues including 16 critical and 31 major issues that need attention. Additionally, code coverage reporting to SonarCloud is not configured, and there are 11 Dependabot PRs pending merge that were blocked by CI failures.

## What Changes

- Address 16 critical SonarCloud issues (cognitive complexity, memory management, type safety)
- Configure SonarCloud code coverage reporting for C++, Python, and Rust
- Merge pending Dependabot dependency updates (11 PRs)
- **BREAKING**: Some fixes may change internal function signatures for better type safety

## Impact

- Affected specs: code-quality (new)
- Affected code:
  - `src/engine/cycloneddsdevice.cxx` - Global pointer const qualifications
  - `src/asterix/AsterixData.hpp` - Cognitive complexity refactoring
  - `asterix-node/src/ruby_parser.cpp` - Memory management fixes
  - `src/engine/devicefactory.cxx` - Replace raw `new` with smart pointers
  - `.github/workflows/` - Add SonarCloud coverage configuration
  - `asterix-rs/Cargo.toml` - Dependency updates
