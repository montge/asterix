# code-quality Specification

## Purpose

Define and enforce model software quality standards for the ASTERIX decoder - a safety-critical ATM (Air Traffic Management) application. This spec establishes measurable quality gates for static analysis, code coverage, complexity, memory safety, and dependency management across all three language bindings (C++, Python, Rust).

## Current Status (2026-02-07)

| Metric | Current | Target | Gap |
|--------|---------|--------|-----|
| Bugs | 0 (2 false positives) | 0 | PASS |
| Vulnerabilities | 0 | 0 | PASS |
| Code Smells | ~1,400 | 0 | ~1,400 remaining |
| Overall Coverage | 21.4% | 90% | Major gap |
| Duplication | 1.8% | <3% | PASS |
| Security Hotspots | 45 reviewed | All reviewed | PASS |
| Cognitive Complexity | 1 violation (XMLParser::parseAttributes at 57) | max 25/function | 1 violation |

### Known Deferred Items
- Smart pointer migration (cpp:S5025) - large refactor, ~578 issues
- Naming convention standardization - deferred from fix-sonarcloud-issues
- Coverage improvement 21.4% -> 90% - explicitly out of scope in prior changes, needs dedicated effort
- SonarCloud quality gate UI configuration - pending manual action
## Requirements
### Requirement: Model Software Quality Standards

The system SHALL achieve and maintain "model software" quality standards - exemplary code suitable for safety-critical ATM (Air Traffic Management) applications.

#### Scenario: Zero tolerance for quality issues
- **WHEN** the SonarCloud dashboard is reviewed
- **THEN** the project SHALL show 0 bugs, 0 vulnerabilities, and 0 code smells

#### Scenario: Technical debt elimination
- **WHEN** the SonarCloud dashboard is reviewed
- **THEN** technical debt SHALL be 0 days

#### Scenario: Quality ratings
- **WHEN** SonarCloud quality metrics are evaluated
- **THEN** all metrics (Reliability, Security, Maintainability) SHALL have "A" rating

### Requirement: Static Code Analysis Quality Gates

The system SHALL maintain code quality standards enforced by SonarCloud static analysis.

#### Scenario: No critical issues allowed
- **WHEN** code is submitted for review
- **THEN** SonarCloud analysis SHALL report zero critical issues

#### Scenario: No major issues allowed
- **WHEN** code is submitted for review
- **THEN** SonarCloud analysis SHALL report zero major issues

#### Scenario: No minor issues allowed
- **WHEN** code is submitted for review
- **THEN** SonarCloud analysis SHALL report zero minor issues

#### Scenario: No code smells allowed
- **WHEN** code is submitted for review
- **THEN** SonarCloud analysis SHALL report zero code smells

#### Scenario: New code quality gate
- **WHEN** new code is added to the project
- **THEN** the new code MUST not introduce any new issues of any severity

### Requirement: Code Coverage Reporting

The system SHALL report code coverage metrics to SonarCloud for all language bindings.

#### Scenario: C++ coverage reporting
- **WHEN** CI pipeline runs on a pull request
- **THEN** C++ code coverage data SHALL be uploaded to SonarCloud in lcov format

#### Scenario: Python coverage reporting
- **WHEN** CI pipeline runs on a pull request
- **THEN** Python code coverage data SHALL be uploaded to SonarCloud in XML format

#### Scenario: Rust coverage reporting
- **WHEN** CI pipeline runs on a pull request
- **THEN** Rust code coverage data SHALL be uploaded to SonarCloud

#### Scenario: Module coverage threshold
- **WHEN** code coverage is measured per module
- **THEN** each module SHALL have at least 80% line coverage

#### Scenario: Overall coverage threshold
- **WHEN** overall project code coverage is measured
- **THEN** the project SHALL have at least 90% line coverage

### Requirement: Code Duplication Standards

The system SHALL minimize code duplication to maintain DRY (Don't Repeat Yourself) principles.

#### Scenario: Duplication threshold
- **WHEN** code duplication metrics are measured
- **THEN** the project SHALL have less than 3% duplicated lines

#### Scenario: No new duplications
- **WHEN** new code is added
- **THEN** the new code MUST not introduce duplicated blocks

### Requirement: Cognitive Complexity Standards

The system SHALL maintain readable, maintainable code with controlled cognitive complexity.

#### Scenario: Function complexity limit
- **WHEN** any function is analyzed
- **THEN** the cognitive complexity SHALL not exceed 25

#### Scenario: Class complexity limit
- **WHEN** any class is analyzed
- **THEN** the total cognitive complexity of all methods SHALL not exceed 100

### Requirement: Memory Safety Standards

The system SHALL follow memory-safe coding practices as verified by static analysis.

#### Scenario: Smart pointer usage
- **WHEN** dynamic memory allocation is required
- **THEN** smart pointers (unique_ptr, shared_ptr) SHALL be used instead of raw new/delete

#### Scenario: Const correctness
- **WHEN** global or shared pointers are declared
- **THEN** appropriate const qualifications SHALL be applied

#### Scenario: Type safety
- **WHEN** pointers are used in function signatures
- **THEN** void pointers SHALL be replaced with meaningful types where possible

### Requirement: Dependency Management

The system SHALL maintain up-to-date dependencies with automated security monitoring.

#### Scenario: Dependabot updates
- **WHEN** Dependabot creates a dependency update PR
- **THEN** the PR SHALL be reviewed and merged within 2 weeks if CI passes

#### Scenario: Dependabot blocked by CI
- **WHEN** Dependabot PRs cannot be merged due to pre-existing CI failures
- **THEN** the CI failure SHALL be treated as P0 priority and fixed before other feature work

#### Scenario: Security vulnerabilities
- **WHEN** a dependency has known security vulnerabilities
- **THEN** the dependency SHALL be updated to a patched version within 1 week

