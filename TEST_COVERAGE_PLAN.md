# Test Coverage Plan to Achieve 95%

**Document Version:** 1.0
**Date:** 2025-10-20
**Target:** DO-278A AL-3 Compliance (90-95% overall coverage)
**Current Status:** Analysis completed, roadmap defined

---

## Executive Summary

This document provides a comprehensive plan to achieve 95% test coverage across the ASTERIX project, meeting DO-278A Assurance Level 3 requirements for aviation software.

**Current Coverage Baseline:**
- **C++:** 45.5% (1,062/2,334 lines, 20/32 files tested)
- **Python:** 88% (135/153 lines)
- **Overall Project:** ~14.9% (estimated across all source files)

**Target Coverage:**
- **Overall:** 95%
- **Per Module:** ≥80%
- **Critical Path:** 100%

**Gap to Close:** +50.1 percentage points (+80.5% for C++)

---

## 1. Current Coverage Baseline

### 1.1 C++ Coverage (45.5%)

**Tested Files (20):**
```
✓ AsterixDefinition.cpp      - Configuration management (tested via unit tests)
✓ Category.cpp               - Category definitions (tested via unit tests)
✓ DataBlock.cpp              - Block parsing (tested via unit + integration)
✓ DataItem.cpp               - Item parsing (tested via unit tests)
✓ DataItemBits.cpp           - Bit field handling (tested via unit tests)
✓ DataItemFormat.cpp         - Format base class (tested via inheritance)
✓ DataItemFormatBDS.cpp      - BDS format parser (tested via unit tests)
✓ DataItemFormatCompound.cpp - Compound format parser (tested via unit tests)
✓ DataItemFormatExplicit.cpp - Explicit format parser (tested via unit tests)
✓ DataItemFormatFixed.cpp    - Fixed format parser (tested via unit tests)
✓ DataItemFormatRepetitive.cpp - Repetitive format parser (tested via unit tests)
✓ DataItemFormatVariable.cpp - Variable format parser (tested via unit tests)
✓ DataRecord.cpp             - Record parsing (tested via unit tests)
✓ InputParser.cpp            - Input parsing (tested via unit tests)
✓ Tracer.cpp                 - Debug tracing (tested via unit tests)
✓ Utils.cpp                  - Utility functions (tested via unit tests)
✓ XMLParser.cpp              - XML config parsing (tested via unit tests)
✓ asterixpcapsubformat.cxx   - PCAP format (tested via integration)
✓ asterixrawsubformat.cxx    - Raw format (tested via integration)
✓ asterixformat.cxx          - Format base (tested via inheritance)
```

**Untested Files (12) - HIGH PRIORITY:**
```
✗ AsterixData.cpp            - 110 lines - Data container
✗ DataItemDescription.cpp    - Unknown size - Descriptions
✗ UAP.cpp                    - Unknown size - User Application Profile
✗ UAPItem.cpp                - Unknown size - UAP items
✗ WiresharkWrapper.cpp       - 378 lines - Wireshark integration
✗ asterixfinalsubformat.cxx  - Unknown size - FINAL format
✗ asterixgpssubformat.cxx    - Unknown size - GPS format
✗ asterixhdlcsubformat.cxx   - Unknown size - HDLC format
✗ asterixhdlcparsing.c       - Unknown size - HDLC C parser
```

**Untested Engine Layer (9) - MEDIUM PRIORITY:**
```
✗ channelfactory.cxx         - Factory pattern
✗ converterengine.cxx        - Main engine
✗ descriptor.cxx             - Descriptors
✗ devicefactory.cxx          - Device factory
✗ diskdevice.cxx             - File I/O
✗ serialdevice.cxx           - Serial port
✗ stddevice.cxx              - Stdin/stdout
✗ tcpdevice.cxx              - TCP network
✗ udpdevice.cxx              - UDP/multicast
```

**Untested Application Layer (1) - LOW PRIORITY:**
```
✗ asterix.cpp                - Main CLI (tested manually via integration)
```

### 1.2 Python Coverage (88%)

**Current Status:** 135/153 lines covered

**Missing Coverage (18 lines):**
```
asterix/__init__.py:
  Lines 58-59     - Error handling branch (custom init)
  Lines 103-109   - describeXML edge cases
  Lines 120-126   - describeXML_spec edge cases
  Line 216        - describe() edge case
  Line 232        - Error handling branch
```

**Analysis:** Python coverage is good. Missing lines are primarily:
- Edge case error handling
- Optional parameter branches
- Exception handling paths

### 1.3 Integration Test Coverage

**Existing Tests (install/test/test.sh):**
```
✓ Filter functionality
✓ JSON output (CAT 062/065)
✓ JSON output with filter (CAT 001/002)
✓ JSON human-readable output
✓ XML output
✓ Text output
✓ Line-by-line output
✓ Filtered text output
✓ GPS parsing
✓ Memory leak detection (valgrind)
```

**Coverage:** ~9 test scenarios covering major input/output formats

---

## 2. Top 10 Untested Files/Functions

### Priority 1 - Critical Path (Must Reach 95%+)

1. **DataItemBits.cpp** (1,485 lines)
   - Current: Partially tested
   - Target: 95%
   - Gap: ~50% untested
   - Priority: CRITICAL - largest file, complex bit manipulation
   - Test Needs: Edge cases, boundary conditions, all bit patterns

2. **XMLParser.cpp** (798 lines)
   - Current: Basic parsing tested
   - Target: 95%
   - Gap: Error handling untested
   - Priority: CRITICAL - config loading, validation
   - Test Needs: Malformed XML, missing tags, invalid values

3. **DataRecord.cpp** (409 lines)
   - Current: Basic parsing tested
   - Target: 95%
   - Gap: Edge cases untested
   - Priority: HIGH - core parsing logic
   - Test Needs: Truncated records, invalid UAP, empty records

4. **DataItemFormatCompound.cpp** (404 lines)
   - Current: Basic parsing tested
   - Target: 95%
   - Gap: Complex nested structures untested
   - Priority: HIGH - complex data types
   - Test Needs: Deep nesting, all FX bit patterns

5. **WiresharkWrapper.cpp** (378 lines)
   - Current: UNTESTED
   - Target: 80% (non-critical)
   - Gap: 100%
   - Priority: MEDIUM - external integration
   - Test Needs: Mock Wireshark API, basic functionality

### Priority 2 - Important Modules (Must Reach 80%+)

6. **DataItemFormatVariable.cpp** (355 lines)
   - Current: Basic parsing tested
   - Target: 90%
   - Gap: FX extension bits untested
   - Priority: HIGH - variable length handling
   - Test Needs: All FX patterns, max length

7. **DataItemFormatFixed.cpp** (306 lines)
   - Current: Basic parsing tested
   - Target: 90%
   - Gap: Edge cases
   - Priority: HIGH - most common format
   - Test Needs: Boundary values, all sizes

8. **converterengine.cxx** (Engine layer)
   - Current: UNTESTED
   - Target: 85%
   - Gap: 100%
   - Priority: HIGH - main processing engine
   - Test Needs: Unit tests for engine lifecycle

9. **UAP.cpp + UAPItem.cpp**
   - Current: UNTESTED
   - Target: 90%
   - Gap: 100%
   - Priority: HIGH - UAP mapping critical for parsing
   - Test Needs: All UAP variations, invalid mappings

10. **AsterixData.cpp** (110 lines)
    - Current: UNTESTED
    - Target: 90%
    - Gap: 100%
    - Priority: HIGH - data container
    - Test Needs: Basic container operations

### Priority 3 - Format Handlers (Must Reach 80%+)

11. **asterixfinalsubformat.cxx**
    - Current: UNTESTED
    - Target: 85%
    - Priority: MEDIUM - FINAL format support
    - Test Needs: Integration test with FINAL sample data

12. **asterixhdlcsubformat.cxx + asterixhdlcparsing.c**
    - Current: UNTESTED
    - Target: 85%
    - Priority: MEDIUM - HDLC format support
    - Test Needs: Integration test with HDLC sample data

13. **asterixgpssubformat.cxx**
    - Current: Integration test exists
    - Target: 90%
    - Priority: MEDIUM - GPS format
    - Test Needs: More edge cases

---

## 3. Priority Test Cases to Add

### 3.1 Critical Path Coverage

**Goal:** 100% coverage of main execution paths

**Test Cases:**

1. **End-to-End Parsing Pipeline:**
   ```cpp
   Input → Format Detection → Data Block → Data Record → Data Items → Output
   ```
   - Test CAT 001, 002, 019, 020, 021, 023, 034, 048, 062, 063, 065
   - Test all input formats: PCAP, HDLC, FINAL, GPS, Raw
   - Test all output formats: JSON, XML, Text, Line

2. **UAP Processing:**
   - Test all UAP variations for each category
   - Test UAP with all FX bit patterns
   - Test invalid UAP references

3. **XML Configuration Loading:**
   - Test all category XML files
   - Test BDS definitions
   - Test DTD validation

### 3.2 Error Handling Coverage

**Goal:** 100% coverage of error paths

**Test Cases:**

1. **Invalid Input Data:**
   - Truncated data blocks
   - Invalid category numbers
   - Corrupted data items
   - Buffer overruns
   - NULL pointers

2. **Configuration Errors:**
   - Missing XML files
   - Malformed XML
   - Invalid category definitions
   - Missing UAP entries

3. **Resource Errors:**
   - File I/O failures
   - Network connection failures
   - Memory allocation failures (if applicable)
   - Invalid file descriptors

### 3.3 Edge Case Coverage

**Goal:** 95%+ coverage of boundary conditions

**Test Cases:**

1. **Data Boundaries:**
   - Zero-length data items
   - Maximum-length data items
   - Minimum/maximum numeric values
   - Empty data blocks/records

2. **Format Variations:**
   - All FX bit extension patterns
   - All Compound item subfield combinations
   - All Repetitive item count values (0, 1, max)
   - All BDS register types

3. **Bit Field Operations (DataItemBits.cpp):**
   - All bit sizes (1-64 bits)
   - All byte alignments
   - Signed/unsigned conversions
   - Float/double conversions
   - All LSB/MSB variations

### 3.4 Python Module Edge Cases

**Goal:** 95%+ Python coverage

**Test Cases:**

1. **Error Handling:**
   ```python
   # Test missing in asterix/__init__.py:
   - Custom init with invalid XML file (lines 58-59)
   - describeXML with missing lxml (lines 103-109)
   - describeXML_spec with invalid parameters (lines 120-126)
   - describe() with invalid category (line 216)
   - Exception handling branches (line 232)
   ```

2. **API Variants:**
   - Test all parameter combinations
   - Test with None/empty values
   - Test type checking

---

## 4. Test Organization Recommendations

### 4.1 Current Structure
```
tests/
├── cpp/                    # C++ unit tests (Google Test)
│   ├── test_*.cpp         # 20 unit test files
│   └── test_integration_*.cpp  # 5 integration tests
├── python/                 # Python unit tests (pytest)
│   ├── test_basic_parsing.py
│   ├── test_advanced_parsing.py
│   └── test_edge_cases.py
└── integration/            # Not yet created
```

### 4.2 Recommended Structure
```
tests/
├── cpp/
│   ├── unit/              # Unit tests (isolated)
│   │   ├── asterix/       # Asterix layer tests
│   │   │   ├── test_asterixdata.cpp        # NEW
│   │   │   ├── test_dataitemdescription.cpp # NEW
│   │   │   ├── test_uap.cpp                # NEW
│   │   │   ├── test_uapitem.cpp            # NEW
│   │   │   ├── test_wiresharkwrapper.cpp   # NEW
│   │   │   └── [existing tests]
│   │   ├── engine/        # Engine layer tests (NEW)
│   │   │   ├── test_converterengine.cpp
│   │   │   ├── test_channelfactory.cpp
│   │   │   ├── test_devicefactory.cpp
│   │   │   ├── test_diskdevice.cpp
│   │   │   ├── test_tcpdevice.cpp
│   │   │   ├── test_udpdevice.cpp
│   │   │   └── test_serialdevice.cpp
│   │   └── formats/       # Format handlers (NEW)
│   │       ├── test_finalsubformat.cpp
│   │       ├── test_hdlcsubformat.cpp
│   │       └── test_gpssubformat.cpp
│   ├── integration/       # Integration tests
│   │   ├── test_integration_cat*.cpp       # Existing
│   │   ├── test_integration_formats.cpp    # NEW
│   │   ├── test_integration_engine.cpp     # NEW
│   │   └── test_integration_pipeline.cpp   # NEW
│   └── coverage/          # Coverage-specific tests (NEW)
│       ├── test_error_paths.cpp            # Error handling
│       ├── test_edge_cases.cpp             # Boundary conditions
│       └── test_bit_operations.cpp         # DataItemBits edge cases
├── python/
│   ├── test_basic_parsing.py               # Existing
│   ├── test_advanced_parsing.py            # Existing
│   ├── test_edge_cases.py                  # Existing
│   ├── test_error_handling.py              # NEW - lines 58-59, 232
│   └── test_optional_features.py           # NEW - lines 103-126, 216
├── integration/           # System-level integration (NEW)
│   ├── test_cli_all_formats.sh             # Test all CLI options
│   ├── test_network_streams.sh             # Test UDP/TCP/multicast
│   └── test_real_data.sh                   # Test with production data
└── data/                  # Test data (NEW)
    ├── samples/           # Sample ASTERIX files
    ├── edge_cases/        # Edge case data
    ├── malformed/         # Invalid data for error testing
    └── expected/          # Expected output for validation
```

### 4.3 Test Naming Convention

**C++ Unit Tests:**
```cpp
// File: test_modulename.cpp
// Class: ModuleNameTest
// Format: TEST(ClassName, TestName)

TEST(DataItemBits, ParseSingleBit)
TEST(DataItemBits, ParseMaxBitWidth)
TEST(DataItemBits, ParseInvalidBitWidth)  // Error case
TEST(DataItemBits, BoundaryCondition_ZeroLength)  // Edge case
```

**Python Tests:**
```python
# File: test_feature.py
# Class: TestFeatureName
# Format: test_scenario_description

class TestErrorHandling:
    def test_init_with_invalid_xml_file(self):  # Line 58-59
    def test_describe_with_missing_category(self):  # Line 216
```

---

## 5. CI/CD Integration for Coverage Tracking

### 5.1 GitHub Actions Workflow Updates

**File:** `.github/workflows/ci-verification.yml`

**Enhancements Needed:**

```yaml
# Add to coverage-cpp job:
coverage-cpp:
  name: C++ Coverage Analysis
  steps:
    - name: Build with coverage flags
      run: |
        cd src
        make clean
        # Add coverage instrumentation
        CXXFLAGS="--coverage -g -O0" CFLAGS="--coverage -g -O0" LDFLAGS="--coverage" make
        make install

    - name: Run all tests
      run: |
        cd tests/cpp
        cmake -DENABLE_COVERAGE=ON .
        make
        ctest --output-on-failure

        # Also run integration tests
        cd ../../install/test
        ./test.sh

    - name: Generate coverage report
      run: |
        lcov --capture --directory . --output-file coverage.info
        lcov --remove coverage.info '/usr/*' '*/test/*' --output-file coverage.info
        lcov --list coverage.info
        genhtml coverage.info --output-directory coverage_html

    - name: Upload coverage to Codecov
      uses: codecov/codecov-action@v5
      with:
        files: ./coverage.info
        flags: cpp
        fail_ci_if_error: true  # ENABLE

    - name: Coverage report comment
      uses: romeovs/lcov-reporter-action@v0.3.1
      with:
        lcov-file: ./coverage.info
        github-token: ${{ secrets.GITHUB_TOKEN }}

# Add Python coverage threshold:
test-python:
  steps:
    - name: Run Python tests with coverage
      run: |
        pytest --cov=asterix --cov-report=xml --cov-report=html \
               --cov-fail-under=90  # REQUIRE 90%+

    - name: Upload to Codecov
      uses: codecov/codecov-action@v5
      with:
        files: ./coverage.xml
        flags: python
        fail_ci_if_error: true

# Add combined coverage job:
coverage-report:
  needs: [coverage-cpp, test-python]
  runs-on: ubuntu-latest
  steps:
    - name: Download coverage reports
      uses: actions/download-artifact@v4

    - name: Combine coverage
      run: |
        # Merge C++ and Python coverage
        # Generate combined report

    - name: Check coverage threshold
      run: |
        # Fail if overall < 95%
        # Fail if any module < 80%
```

### 5.2 Coverage Badges

Add to `README.md`:
```markdown
[![codecov](https://codecov.io/gh/USERNAME/asterix/branch/master/graph/badge.svg)](https://codecov.io/gh/USERNAME/asterix)
[![C++ Coverage](https://img.shields.io/badge/C++-95%25-brightgreen)](./coverage_html/index.html)
[![Python Coverage](https://img.shields.io/badge/Python-95%25-brightgreen)](./htmlcov/index.html)
```

### 5.3 Pre-commit Hooks

**File:** `.pre-commit-config.yaml` (NEW)

```yaml
repos:
  - repo: local
    hooks:
      - id: coverage-check
        name: Check test coverage
        entry: python scripts/check_coverage.py
        language: python
        pass_filenames: false
        always_run: true
```

**File:** `scripts/check_coverage.py` (NEW)

```python
#!/usr/bin/env python3
"""Pre-commit hook to check coverage thresholds"""
import sys
import subprocess

def check_python_coverage():
    result = subprocess.run(
        ['pytest', '--cov=asterix', '--cov-report=term', '--cov-fail-under=90'],
        capture_output=True
    )
    return result.returncode == 0

def check_cpp_coverage():
    # Run C++ tests and check coverage
    # Return True if >= 95%
    pass

if __name__ == '__main__':
    if not check_python_coverage():
        print("❌ Python coverage below 90%")
        sys.exit(1)
    print("✅ Coverage checks passed")
```

---

## 6. Roadmap to 95% (Phased Approach)

### Phase 1: Foundation (Weeks 1-2) - Target: 60%

**Goal:** Test critical untested modules, establish infrastructure

**Tasks:**
1. Create test infrastructure for engine layer
2. Add unit tests for untested ASTERIX modules:
   - AsterixData.cpp
   - UAP.cpp / UAPItem.cpp
   - DataItemDescription.cpp
3. Improve Python coverage to 95%
4. Set up CI coverage reporting with thresholds
5. Create test data repository

**Deliverables:**
- 15 new unit test files
- Python coverage: 88% → 95%
- C++ coverage: 45% → 60%
- CI configured with coverage gates

### Phase 2: Core Coverage (Weeks 3-4) - Target: 75%

**Goal:** Achieve 90%+ coverage on critical path

**Tasks:**
1. Add edge case tests for DataItemBits.cpp (priority #1)
2. Add error handling tests for XMLParser.cpp
3. Add comprehensive tests for DataRecord.cpp
4. Add format handler tests (FINAL, HDLC, GPS)
5. Add engine layer integration tests

**Deliverables:**
- 20+ new test cases for DataItemBits
- XMLParser error handling: 100%
- Format handlers: 85%+ each
- C++ coverage: 60% → 75%

### Phase 3: Edge Cases (Weeks 5-6) - Target: 85%

**Goal:** Cover error paths and edge cases

**Tasks:**
1. Add error handling tests for all modules
2. Add boundary condition tests
3. Add integration tests for all input/output combinations
4. Add tests for WiresharkWrapper.cpp
5. Add device layer tests (TCP, UDP, serial, disk)

**Deliverables:**
- Error path coverage: 95%+
- Boundary case coverage: 90%+
- Integration test suite: 30+ scenarios
- C++ coverage: 75% → 85%

### Phase 4: Refinement (Weeks 7-8) - Target: 95%

**Goal:** Achieve 95% overall coverage

**Tasks:**
1. Analyze coverage gaps with lcov
2. Add tests for remaining uncovered lines
3. Add stress tests and performance tests
4. Add memory leak tests (valgrind)
5. Document all test cases

**Deliverables:**
- Coverage report showing 95%+ overall
- All modules ≥80% coverage
- Critical path: 100% coverage
- Test documentation complete
- C++ coverage: 85% → 95%
- Python coverage: maintained at 95%

### Phase 5: Maintenance (Ongoing) - Target: Maintain 95%

**Goal:** Maintain coverage as code evolves

**Tasks:**
1. Require tests for all new code (CI gate)
2. Monitor coverage in pull requests
3. Regular coverage audits
4. Update tests when requirements change

**Policies:**
- No PR merge if coverage drops below 95%
- New features require tests first (TDD)
- Coverage report in every PR review

---

## 7. Success Metrics

### 7.1 Coverage Targets

| Component | Current | Week 2 | Week 4 | Week 6 | Week 8 | Target |
|-----------|---------|--------|--------|--------|--------|--------|
| **C++ Overall** | 45.5% | 60% | 75% | 85% | 95% | **95%** |
| DataItemBits | ~50% | 70% | 85% | 95% | 95% | 95% |
| XMLParser | ~60% | 80% | 90% | 95% | 95% | 95% |
| DataRecord | ~70% | 85% | 90% | 95% | 95% | 95% |
| Format Handlers | 0% | 40% | 70% | 85% | 90% | 90% |
| Engine Layer | 0% | 40% | 60% | 80% | 85% | 85% |
| **Python** | 88% | 95% | 95% | 95% | 95% | **95%** |
| **Overall Project** | 14.9% | 65% | 78% | 87% | 95% | **95%** |

### 7.2 Quality Gates

**Per Pull Request:**
- ✅ All new code has tests
- ✅ Coverage does not decrease
- ✅ All tests pass
- ✅ No new memory leaks (valgrind clean)

**Per Module:**
- ✅ Coverage ≥ 80% (minimum)
- ✅ Critical modules ≥ 95%
- ✅ All public APIs tested
- ✅ Error paths tested

**Overall Project:**
- ✅ C++ coverage ≥ 95%
- ✅ Python coverage ≥ 95%
- ✅ Integration tests pass
- ✅ DO-278A compliance documented

### 7.3 Test Metrics

**Quantity:**
- C++ unit tests: 38 → 120+ (target)
- C++ integration tests: 5 → 15+ (target)
- Python tests: 60 → 75+ (target)
- Total test cases: 103 → 210+ (target)

**Quality:**
- Test execution time: < 5 minutes (full suite)
- Test reliability: 100% (no flaky tests)
- Code review: All tests reviewed
- Documentation: All tests documented

---

## 8. Test Data Requirements

### 8.1 Sample Data Collection

**Required:**
- CAT 001, 002, 019, 020, 021, 023, 034, 048, 062, 063, 065 samples
- PCAP format samples
- HDLC format samples
- FINAL format samples
- GPS format samples
- Raw binary samples

**Edge Cases:**
- Truncated data
- Maximum length data items
- All FX bit patterns
- All BDS registers
- Malformed data for error testing

### 8.2 Expected Output Generation

For each sample input, generate expected output in:
- JSON (compact)
- JSON (human-readable)
- XML
- Text
- Line format

**Automation:**
```bash
# Generate expected outputs from known-good version
for format in json jsonh xml txt line; do
  ./asterix -f sample.pcap -$format > expected.$format
done
```

### 8.3 Test Oracle

Create validation scripts:
- `validate_json.py` - Validate JSON structure
- `validate_xml.py` - Validate XML against schema
- `compare_outputs.py` - Compare actual vs expected
- `check_memory.sh` - Run valgrind checks

---

## 9. Tools and Infrastructure

### 9.1 Coverage Tools

**C++ Coverage:**
- `lcov` - Line coverage measurement
- `gcov` - GCC coverage tool
- `gcovr` - Coverage report generator
- `genhtml` - HTML report generation

**Python Coverage:**
- `coverage.py` - Python coverage tool
- `pytest-cov` - Pytest integration
- Coverage HTML reports

**Combined:**
- Codecov - Cloud coverage service
- Coverage badges
- Coverage trends

### 9.2 Testing Frameworks

**C++:**
- Google Test (gtest) - Unit testing framework
- Google Mock (gmock) - Mocking framework
- Valgrind - Memory leak detection

**Python:**
- pytest - Testing framework
- unittest - Standard library
- mock - Mocking support

### 9.3 CI/CD

**GitHub Actions:**
- Build verification
- Test execution
- Coverage measurement
- Coverage reporting
- Quality gates

**Artifacts:**
- Coverage reports (HTML)
- Test results (XML)
- Binary builds
- Documentation

---

## 10. Risk Assessment

### 10.1 Risks

| Risk | Probability | Impact | Mitigation |
|------|------------|--------|------------|
| Complex bit operations hard to test | High | High | Create comprehensive bit pattern generator |
| Wireshark integration difficult to mock | Medium | Low | Use minimal stub, lower coverage target |
| Engine layer dependencies | Medium | Medium | Use dependency injection, mocking |
| Test data hard to create | Low | Medium | Automate generation from existing data |
| Time constraints | Medium | High | Phased approach, prioritize critical path |

### 10.2 Dependencies

**External:**
- libexpat-devel (already installed)
- lcov, gcov (already installed)
- Google Test (fetched via CMake)
- pytest, coverage (already installed)

**Internal:**
- Test data repository
- Coverage infrastructure
- CI/CD configuration

---

## 11. Documentation Requirements

### 11.1 Test Documentation

**Per Test File:**
- Purpose and scope
- Test cases covered
- Data sources
- Expected results
- Maintenance notes

**Per Test Case:**
```cpp
/**
 * @brief Test parsing of maximum-length data item
 *
 * @test_id TC_DATAITEM_001
 * @requirement REQ-HLR-PARSE-001
 * @coverage DataItem::parse() - branch 3
 *
 * Verifies that data items with maximum allowed length
 * (65535 bytes) are parsed correctly without buffer overflow.
 */
TEST(DataItem, ParseMaxLength) {
    // Test implementation
}
```

### 11.2 Coverage Reports

**Weekly:**
- Coverage summary report
- New tests added
- Coverage trends
- Issues/blockers

**Per Release:**
- Full coverage report
- Module-by-module breakdown
- DO-278A compliance statement
- Traceability matrix update

---

## 12. Next Steps

### Immediate Actions (This Week)

1. ✅ **Review this plan** - Team review and approval
2. ⬜ **Set up coverage infrastructure** - CI configuration
3. ⬜ **Create test data repository** - Organize sample files
4. ⬜ **Start Phase 1** - Begin untested module tests
5. ⬜ **Improve Python coverage** - Add missing test cases

### Week 1-2 Deliverables

- [ ] CI configured with coverage gates
- [ ] Python coverage at 95%
- [ ] 15 new C++ unit test files
- [ ] Test data repository organized
- [ ] C++ coverage at 60%

### Review Schedule

- **Weekly:** Coverage metrics review
- **Bi-weekly:** Test plan progress review
- **Monthly:** DO-278A compliance audit

---

## Appendices

### A. Coverage Measurement Commands

**C++ Coverage:**
```bash
# Build with coverage
cd src
make clean
CXXFLAGS="--coverage -g -O0" CFLAGS="--coverage -g -O0" LDFLAGS="--coverage" make
make install

# Run tests
cd tests/cpp
cmake -DENABLE_COVERAGE=ON .
make
ctest

# Generate report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/test/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

**Python Coverage:**
```bash
# Run with coverage
pytest --cov=asterix --cov-report=html --cov-report=term

# View report
open htmlcov/index.html
```

### B. Test Template

**C++ Unit Test Template:**
```cpp
#include <gtest/gtest.h>
#include "ModuleName.h"

class ModuleNameTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test fixtures
    }

    void TearDown() override {
        // Clean up
    }
};

TEST_F(ModuleNameTest, BasicFunctionality) {
    // Arrange
    ModuleName module;

    // Act
    auto result = module.method();

    // Assert
    EXPECT_EQ(expected, result);
}

TEST_F(ModuleNameTest, ErrorHandling) {
    ModuleName module;
    EXPECT_THROW(module.invalidMethod(), std::runtime_error);
}
```

**Python Test Template:**
```python
import pytest
import asterix

class TestFeatureName:
    """Test suite for feature description"""

    def setup_method(self):
        """Set up test fixtures"""
        pass

    def test_basic_functionality(self):
        """Test basic operation"""
        # Arrange
        data = bytearray([...])

        # Act
        result = asterix.parse(data)

        # Assert
        assert result is not None
        assert result[0]['category'] == 48
```

### C. References

- DO-278A: Software Integrity Assurance Considerations for Communication, Navigation, Surveillance and Air Traffic Management (CNS/ATM) Systems
- ASTERIX Protocol: http://www.eurocontrol.int/services/asterix
- Google Test: https://github.com/google/googletest
- Coverage.py: https://coverage.readthedocs.io
- lcov: http://ltp.sourceforge.net/coverage/lcov.php

---

**Document Status:** DRAFT
**Next Review:** Week 1 of implementation
**Approval:** Pending team review
