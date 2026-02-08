# Proposal: Improve Test Coverage to 90% Target

## Change ID
`improve-test-coverage`

## Priority
**P1 - HIGH** - Coverage at 21.4% is the largest gap against the code-quality spec's 90% target and was explicitly deferred from the `fix-sonarcloud-issues` change.

## Related Issues
- Spec: `openspec/specs/code-quality/spec.md` (90% overall, 80% per-module targets)
- Deferred from: `fix-sonarcloud-issues` (Dec 2025) - coverage improvement marked out of scope
- Relates to: SonarCloud quality gate failure (coverage)

## Summary

The project's overall code coverage is **21.4%**, far below the 90% target defined in the code-quality specification. This change establishes a staged plan to close the gap through targeted test expansion across all three language bindings (C++, Python, Rust).

## Current State

| Binding | Coverage | Test Count | Test Framework | Notes |
|---------|----------|------------|----------------|-------|
| C++ | Very low (drives overall down) | 705 unit tests (Google Test) + 11 integration tests | Google Test + shell scripts | 37 test files in `tests/cpp/`, but many source files have shallow coverage |
| Python | Moderate | 927+ tests (19 skipped) | unittest + pytest | `asterix/test/` (19 files) + `asterix/radar_integration/test/` (19 files) |
| Rust | ~76.2% | Integration + unit tests | cargo test + tarpaulin | 20 test files in `asterix-rs/tests/`, inline unit tests in `src/` |
| **Overall** | **21.4%** | **~1,700+** | Mixed | SonarCloud aggregated metric |

### Why 21.4%?

The low overall figure is driven by:
1. **C++ engine layer** (`src/engine/`) has almost no unit test coverage - device classes (UDP, TCP, serial, disk) contain significant untested code
2. **C++ subformat handlers** (`asterixpcapsubformat`, `asterixgpssubformat`, etc.) are exercised only through integration tests, missing many code paths
3. **Python `radar_integration` module** is relatively new and some tests are skipped in CI (e.g., JSBSim tests requiring external dependencies)
4. **SonarCloud aggregates** all languages, so the large C++ codebase with low coverage dominates

### Existing Test Infrastructure

The project already has solid test infrastructure:
- **C++ Google Test**: 37 test files covering core ASTERIX classes, with CMake integration (`BUILD_TESTING=ON`)
- **C++ Integration Tests**: `install/test/test.sh` with 11 end-to-end scenarios
- **Python pytest/unittest**: Coverage reporting via `--cov` flags, uploaded to Codecov
- **Rust tarpaulin/llvm-cov**: Coverage in lcov format uploaded to SonarCloud
- **CI Pipelines**: `ci-verification.yml` runs Google Tests + integration tests with `ENABLE_COVERAGE=ON`; `sonarcloud.yml` aggregates all coverage reports

## Strategy: Staged Milestones

Rather than attempting to jump from 21.4% to 90% in one effort, this change uses incremental milestones:

### Milestone 1: 40% Overall Coverage
**Focus**: Expand C++ unit tests for core parsing classes (highest impact per test added).

The ASTERIX layer (`src/asterix/`) contains the most lines of code. Adding thorough unit tests for `DataItemFormat*`, `Category`, `DataRecord`, `XMLParser`, and `Utils` will have the greatest impact on the overall metric.

### Milestone 2: 60% Overall Coverage
**Focus**: Cover C++ engine layer + Python radar_integration module.

Add unit tests for device classes (`UdpDevice`, `TcpDevice`, `DiskDevice`, `StdDevice`) using mock/stub patterns. Enable and fix Python radar_integration tests that are currently skipped in CI.

### Milestone 3: 80% Overall Coverage
**Focus**: Cover remaining C++ subformat handlers + push Rust to 90%.

Add targeted tests for `asterixpcapsubformat`, `asterixgpssubformat`, `asterixhdlcsubformat`, `asterixfinalsubformat`. Identify uncovered Rust code paths and add tests.

### Milestone 4: 90% Overall Coverage
**Focus**: Cover edge cases, error paths, and output formatters.

Fill remaining gaps in all three bindings. Ensure per-module coverage meets the 80% minimum. Configure SonarCloud quality gate to enforce 90%.

## Scope

### In Scope
- Expanding C++ Google Test unit tests in `tests/cpp/`
- Adding new C++ test files for untested source files
- Expanding Python test coverage (both `asterix/test/` and `asterix/radar_integration/test/`)
- Pushing Rust coverage from ~76.2% to 90%+
- Verifying coverage metrics in CI (SonarCloud, Codecov)
- Updating CI workflows if needed to improve coverage measurement accuracy

### Out of Scope
- Refactoring production code for testability (separate change if needed)
- Smart pointer migration (tracked separately as cpp:S5025)
- Adding new ASTERIX categories or protocol support
- Changing the public API of any binding
- Node.js binding coverage (too new, tracked separately)

## Risks and Mitigations

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| C++ engine classes hard to unit test (network, serial, file I/O) | High | Medium | Use mock/stub patterns; focus on logic paths, not I/O calls |
| Coverage percentage may not improve linearly | Medium | Low | Track per-file coverage, not just overall; celebrate incremental wins |
| Tests may be fragile if tightly coupled to implementation | Medium | Medium | Write behavior-based tests, not implementation-based; avoid testing private internals |
| Rust feature-gated code (zenoh, dds, ccsds, can, dbus) hard to test in CI | High | Medium | Use conditional compilation; test each feature independently |
| radar_integration tests depend on external packages (JSBSim, NumPy) | Medium | Low | Mark external-dependency tests as optional; ensure CI installs dependencies |
| Large effort may stall without visible progress | Medium | High | Use staged milestones; merge incremental improvements; track in CI dashboard |

## Success Criteria

- [ ] Overall coverage reaches 90% as reported by SonarCloud
- [ ] Each module (C++, Python, Rust) meets 80% minimum line coverage
- [ ] All new tests pass on Linux, Windows, and macOS CI
- [ ] Valgrind shows 0 memory leaks with new C++ tests
- [ ] SonarCloud quality gate passes for coverage metric
- [ ] No regressions in existing test suites

## Estimated Effort

| Milestone | Primary Effort | Estimated Tests to Add |
|-----------|---------------|----------------------|
| 40% | C++ core class unit tests | ~200-300 new Google Test cases |
| 60% | C++ engine + Python radar | ~150-200 new tests across C++/Python |
| 80% | C++ subformats + Rust push | ~100-150 new tests across C++/Rust |
| 90% | Edge cases + quality gate | ~50-100 targeted tests |
