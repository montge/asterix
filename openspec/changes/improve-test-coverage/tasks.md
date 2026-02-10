# Tasks: Improve Test Coverage

## Phase 1: C++ Unit Test Expansion (Target: 40% overall)

### 1.1 Audit Existing Test Coverage
- [ ] 1.1.1 Build with `ENABLE_COVERAGE=ON` and run all tests locally to establish baseline
- [ ] 1.1.2 Generate per-file lcov report to identify lowest-coverage source files
- [ ] 1.1.3 Document coverage per C++ source file in `.local/coverage/` baseline report
- [ ] 1.1.4 Prioritize files by lines-of-code times coverage-gap (highest impact first)
_Note: Coverage audit deferred - manual review of test files was used to identify gaps._

### 1.2 DataItemFormat* Classes (Core Parsing)
- [x] 1.2.1 Expand `test_dataitemformatfixed.cpp` - test all output formats (getText, JSON, XML), edge cases (zero-length data, max-length data), error paths
- [x] 1.2.2 Expand `test_dataitemformatvariable.cpp` - test FX bit handling, multi-octet sequences, boundary conditions
- [x] 1.2.3 Expand `test_dataitemformatcompound.cpp` - test nested format combinations, missing subfields, all parse paths
- [x] 1.2.4 Expand `test_dataitemformatrepetitive.cpp` - test zero repetitions, max repetitions, repetition factor parsing
- [x] 1.2.5 Expand `test_dataitemformatexplicit.cpp` - test length field parsing, oversized data, undersized data
- [x] 1.2.6 Expand `test_dataitemformatbds.cpp` - test BDS register parsing, unknown BDS codes
_Note: Already comprehensive with 188 tests across 6 files. No additional expansion needed._

### 1.3 Category and DataRecord Classes
- [x] 1.3.1 Expand `test_category.cpp` - test UAP lookup, filtering, description retrieval, Wireshark definitions
- [x] 1.3.2 Expand `test_datarecord.cpp` - test FSPEC parsing, multi-byte FSPEC, record with all items present, record with no items
- [x] 1.3.3 Expand `test_datablock.cpp` - test data block header parsing, category extraction, length validation, multi-record blocks
- [x] 1.3.4 Expand `test_dataitem.cpp` - test item creation, binary data storage, format delegation
_Note: Added 12 tests to test_category.cpp, 7 to test_dataitem.cpp. DataRecord (47 tests) and DataBlock (20 tests) already comprehensive._

### 1.4 XMLParser Tests
- [x] 1.4.1 Expand `test_xmlparser.cpp` - test loading valid category XML files
- [x] 1.4.2 Add tests for malformed XML handling (missing elements, invalid attributes)
- [x] 1.4.3 Add tests for all element handlers (handleCategoryStart, handleFixedStart, etc.)
- [x] 1.4.4 Test parseAttributes for all data item types (Fixed, Variable, Compound, Repetitive, Explicit, BDS)
_Note: Already has 57 comprehensive tests covering all formats, encodings, error paths, and attributes._

### 1.5 Utils and Supporting Classes
- [x] 1.5.1 Expand `test_utils.cpp` - test hex conversion, string utilities, all utility functions
- [x] 1.5.2 Expand `test_dataitembits.cpp` - test bit extraction, encoding, signed/unsigned values, string values
- [x] 1.5.3 Expand `test_tracer.cpp` - test all trace levels, output formatting
- [x] 1.5.4 Expand `test_asterixdefinition.cpp` - test category loading, multi-category init, category lookup
- [x] 1.5.5 Expand `test_asterixdata.cpp` - test data container operations
- [x] 1.5.6 Expand `test_inputparser.cpp` - test command-line argument parsing, all input format flags
- [x] 1.5.7 Expand `test_dataitemdescription.cpp` - test description creation, format association
- [x] 1.5.8 Expand `test_uap.cpp` and `test_uapitem.cpp` - test UAP item ordering, lookup by FRN
_Note: Added 11 tests to test_utils.cpp, 7 to test_tracer.cpp. Other files already have 18-70 tests each._

### 1.6 C++ Integration Test Expansion
- [x] 1.6.1 Add integration tests for additional ASTERIX categories (CAT001, CAT002, CAT010, CAT020, CAT021, CAT034)
- [x] 1.6.2 Add integration tests for JSON output format validation
- [x] 1.6.3 Add integration tests for XML output format validation
- [x] 1.6.4 Add integration tests for extensive JSON (`-je`) output format
_Note: Added 6 category integration test files (26 tests) and 1 output format test file (7 tests)._

## Phase 2: Python Module Coverage (Target: 60% overall)

### 2.1 radar_integration Module Tests
- [ ] 2.1.1 Ensure all existing `asterix/radar_integration/test/` tests pass in CI (fix skip conditions)
- [ ] 2.1.2 Add coverage for `test_jsbsim_converter.py` (currently skipped - add CI-compatible tests or mock JSBSim)
- [ ] 2.1.3 Expand `test_encoder.py` - test all ASTERIX category encoders
- [ ] 2.1.4 Expand `test_decoder.py` - test round-trip encode/decode for all supported categories
- [ ] 2.1.5 Add error path tests for radar_integration (invalid input, missing fields, malformed data)

### 2.2 Core Python Module Coverage Gaps
- [ ] 2.2.1 Expand `test_parse.py` - test all parse modes (raw, with_offset), edge cases (empty data, truncated data)
- [ ] 2.2.2 Expand `test_init.py` - test loading custom category definitions, invalid paths, re-initialization
- [ ] 2.2.3 Add tests for `asterix.describe()` function across multiple categories and fields
- [ ] 2.2.4 Add negative/error tests for all public Python API functions
- [ ] 2.2.5 Expand `test_edge_cases.py` and `test_ffi_security.py` for FFI boundary coverage

### 2.3 Python Integration Tests
- [ ] 2.3.1 Add cross-validation tests comparing Python output with C++ CLI output for same input data
- [ ] 2.3.2 Ensure `test_cross_binding_validation.py` covers all supported categories

## Phase 3: Rust Coverage Push (Target: 80% overall, Rust 90%)

### 3.1 Identify Uncovered Rust Code Paths
- [ ] 3.1.1 Run `cargo llvm-cov --all-features --html` to generate detailed coverage report
- [ ] 3.1.2 Document uncovered lines per module (lib.rs, parser.rs, ffi.rs, error.rs, types.rs)
- [ ] 3.1.3 Prioritize uncovered paths by importance (public API > internal helpers)

### 3.2 Expand Rust Unit Tests
- [ ] 3.2.1 Add unit tests for error type conversions and Display implementations (`error.rs`)
- [ ] 3.2.2 Add unit tests for data type constructors and accessors (`types.rs` / `data_types_tests.rs`)
- [ ] 3.2.3 Add unit tests for parser configuration and options (`parser.rs`)
- [ ] 3.2.4 Add FFI boundary tests for edge cases (`ffi.rs`)

### 3.3 Expand Rust Integration Tests
- [ ] 3.3.1 Add integration tests for all supported ASTERIX categories (expand from current cat048/062/065)
- [ ] 3.3.2 Add integration tests for incremental parsing (parse_with_offset)
- [ ] 3.3.3 Add error handling integration tests (malformed data, unsupported categories)

### 3.4 Feature-Gated Module Coverage
- [ ] 3.4.1 Improve coverage for Zenoh transport module (`test_zenoh.rs`)
- [ ] 3.4.2 Improve coverage for DDS transport module (`test_dds.rs`)
- [ ] 3.4.3 Improve coverage for D-Bus transport module (`test_dbus.rs`)
- [ ] 3.4.4 Improve coverage for CAN transport module (`test_can.rs`)
- [ ] 3.4.5 Fix and improve coverage for CCSDS module (`test_ccsds.rs` - blocked by `fix-rust-ci-ccsds`)

## Phase 4: Integration, Verification, and Quality Gates (Target: 90% overall)

### 4.1 Coverage Measurement Accuracy
- [ ] 4.1.1 Verify SonarCloud correctly aggregates C++, Python, and Rust coverage reports
- [ ] 4.1.2 Ensure lcov `--ignore-errors` flags do not silently discard valid coverage data
- [ ] 4.1.3 Cross-check SonarCloud coverage with Codecov to identify reporting discrepancies
- [ ] 4.1.4 Verify `tests/cpp/CMakeLists.txt` includes all test executables and discovers all tests

### 4.2 Fill Remaining Gaps
- [ ] 4.2.1 Add C++ tests for output formatters (JSON, XML, text output generation)
- [ ] 4.2.2 Add C++ tests for WiresharkWrapper functions
- [ ] 4.2.3 Add C++ tests for InputParser command-line parsing
- [ ] 4.2.4 Add targeted tests for any per-module coverage still below 80%

### 4.3 CI Quality Gate Configuration
- [ ] 4.3.1 Configure SonarCloud quality gate to enforce 90% overall coverage on new code
- [ ] 4.3.2 Configure SonarCloud quality gate to enforce 80% per-module minimum
- [ ] 4.3.3 Add coverage threshold check to `ci-verification.yml` (fail CI if coverage drops below milestone)
- [ ] 4.3.4 Add coverage badge to README.md

### 4.4 Final Verification
- [ ] 4.4.1 Run full test suite across all bindings (C++, Python, Rust)
- [ ] 4.4.2 Verify Valgrind shows 0 memory leaks with expanded test suite
- [ ] 4.4.3 Verify SonarCloud reports 90%+ overall coverage
- [ ] 4.4.4 Verify each module meets 80% minimum
- [ ] 4.4.5 Verify all CI pipelines pass on Linux, Windows, and macOS

## Progress Tracking

| Phase | Tasks | Completed | Coverage Target |
|-------|-------|-----------|-----------------|
| Phase 1: C++ Unit Tests | 28 | 24 | 40% |
| Phase 2: Python Coverage | 12 | 0 | 60% |
| Phase 3: Rust Push | 13 | 0 | 80% (Rust 90%) |
| Phase 4: Verification | 13 | 0 | 90% |
| **Total** | **66** | **24** | **90%** |

## Dependencies

- **fix-rust-ci-ccsds**: Must be resolved before Phase 3.4.5 (CCSDS test coverage)
- **add-can-socketcan-support**: Phase 3.4.4 (CAN coverage) depends on CAN module being stable
- **reduce-code-smells-phase2**: No blocking dependency, but refactored code may change test targets

## Key Files

### C++ Test Infrastructure
- `tests/cpp/CMakeLists.txt` - Google Test build configuration (37 test executables)
- `tests/cpp/test_*.cpp` - 37 existing test files
- `CMakeLists.txt` (lines 690+) - `BUILD_TESTING` conditional inclusion

### Python Test Infrastructure
- `asterix/test/test_*.py` - 19 core module test files
- `asterix/radar_integration/test/test_*.py` - 19 radar integration test files
- `.github/workflows/ci-verification.yml` (lines 152-215) - pytest + coverage configuration

### Rust Test Infrastructure
- `asterix-rs/tests/*.rs` - 20 integration test files
- `asterix-rs/src/*_tests.rs` - Inline unit test modules
- `asterix-rs/Cargo.toml` - Test and dev-dependency configuration

### Coverage Reporting
- `.github/workflows/sonarcloud.yml` - Aggregated coverage upload (C++ lcov, Python XML, Rust lcov)
- `.github/workflows/ci-verification.yml` - Per-language coverage with Codecov upload
