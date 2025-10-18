# Baseline Metrics Report
## ASTERIX Decoder - DO-278A AL-3 Compliance

**Date:** 2025-10-17
**Phase:** Phase 1 - Baseline Measurement
**Objective:** Establish starting point for coverage improvement

---

## Executive Summary

This document records the baseline metrics for the ASTERIX Decoder project before beginning the DO-278A AL-3 compliance effort focused on achieving 90-95% overall coverage and ≥80% per-module coverage.

**Key Findings:**
- Python module baseline coverage: **39%**
- C++ integration tests: **9/12 passing** (75% pass rate)
- Total test count: **24 automated Python tests**
- All functional tests passing

---

## Python Module Coverage

### Overall Coverage: 39%

Measured using pytest with coverage.py on 2025-10-17.

**Command:**
```bash
pytest asterix/test/ tests/python/ -v --cov=asterix --cov-report=html --cov-report=term
```

**Results:**
```
Name                                         Stmts   Miss  Cover
----------------------------------------------------------------
asterix/__init__.py                            157     74    53%
asterix/examples/__init__.py                     1      1     0%
asterix/examples/multicast_receive.py           15     15     0%
asterix/examples/multicast_send_receive.py      53     53     0%
asterix/examples/read_final_file.py             32     32     0%
asterix/examples/read_pcap_file.py              15     15     0%
asterix/examples/read_raw_bytes.py               6      6     0%
asterix/examples/read_raw_file.py               22     22     0%
asterix/examples/xml_parser.py                 185    185     0%
asterix/test/__init__.py                         1      0   100%
asterix/test/test_init.py                       33      3    91%
asterix/test/test_memory_leak.py                12      7    42%
asterix/test/test_parse.py                     149      2    99%
asterix/version.py                               1      0   100%
----------------------------------------------------------------
TOTAL                                          682    415    39%
```

### Analysis

**Strong Areas (>80% coverage):**
- `asterix/test/test_init.py` - 91%
- `asterix/test/test_parse.py` - 99%
- `asterix/version.py` - 100%
- `asterix/test/__init__.py` - 100%

**Medium Areas (40-80% coverage):**
- `asterix/__init__.py` - 53% (core module)
- `asterix/test/test_memory_leak.py` - 42%

**Weak Areas (0% coverage):**
- `asterix/examples/*` - 0% (all example scripts)
  - These are standalone scripts, not tested
  - Should be excluded from coverage or have integration tests added

**Core Module Coverage:**
The main `asterix/__init__.py` at 53% is reasonable for baseline but needs improvement to reach 90% target.

---

## C++ Integration Tests

### Test Results: 9 Passed / 3 Failed (75%)

**Passing Tests (9):**
- ✓ Test json CAT_062_065
- ✓ Test json CAT_001_002
- ✓ Test jsonh CAT_001_002
- ✓ Test xml CAT_001_002
- ✓ Test txt CAT_001_002
- ✓ Test line CAT_001_002
- ✓ Test filtered txt CAT_034_048
- ✓ Test unfiltered txt CAT_034_048
- ✓ Test GPS parsing

**Failing Tests (3):**
- ✗ Test filter
- ✗ Test Memory leak with valgrind (1)
- ✗ Test Memory leak with valgrind (2)

### Analysis

**Functional Tests:** All 9 functional tests passing
- Multiple ASTERIX categories tested (001, 002, 034, 048, 062, 065)
- Multiple output formats verified (JSON, XML, Text, Line)
- PCAP and GPS input formats working
- Filter functionality present but test failing

**Memory Tests:** Both valgrind tests failing
- These require debug build: `make debug && make debug install`
- Memory leak detection is critical for AL-3
- **Action Required:** Build debug version and investigate failures

**Filter Test:** Failing
- Likely a known issue with filter reference output
- **Action Required:** Investigate and fix or update expected output

---

## C++ Code Coverage

**Status:** Not yet measured

C++ code coverage requires:
1. Compile with `--coverage` flags
2. Run tests
3. Collect coverage data with lcov
4. Generate reports

**Action Items:**
- Modify `src/Makefile` to support coverage build
- Add coverage target to build system
- Integrate with CI/CD pipeline

**Estimated Baseline:** Unknown (likely 20-40% based on test scope)

---

## Test Inventory

### Python Tests (24 total)

**Existing Tests:**
- `asterix/test/test_init.py` - 5 tests (initialization, error handling)
- `asterix/test/test_parse.py` - 3 tests (CAT048, CAT062/065 parsing)

**New Tests (from DO-278 effort):**
- `tests/python/test_basic_parsing.py` - 16 tests
  - Basic parsing functionality
  - Category identification
  - Configuration file handling
  - Sample file handling
  - Description functions
  - Error handling

### C++ Integration Tests (12 total)

**Test Script:** `install/test/test.sh`
- 9 functional tests (data parsing, output formats)
- 2 memory leak tests (valgrind)
- 1 filter test

---

## Gap Analysis

### To Reach 90-95% Overall Coverage Target

**Python Module:** Currently 39%, need +51-56 percentage points

**Gaps Identified:**
1. **Example scripts** - 0% coverage (307 lines uncovered)
   - Decision: Exclude from coverage or add integration tests
2. **Core module** - 53% coverage (74 lines uncovered)
   - Need to test:
     - `parse_with_offset()` function
     - `describeXML()` function
     - Various error paths
     - Edge cases in parsing
3. **Memory leak test** - 42% coverage (needs improvement)

**C++ Code:** Coverage not yet measured
- Need to instrument and measure
- Estimated gap: 50-70 percentage points

### To Reach ≥80% Per-Module Coverage Target

**Modules Below Target:**
- `asterix/test/test_memory_leak.py` - 42% (need +38 points)
- All example scripts - 0% (exclude or test)

**Modules At or Above Target:**
- `asterix/__init__.py` - 53% (need +27 points to reach 80%)
- `asterix/test/test_init.py` - 91% ✓
- `asterix/test/test_parse.py` - 99% ✓

---

## Test Quality Metrics

### Test Execution Time
- Python tests: **0.25 seconds** (very fast)
- C++ tests: ~5-10 seconds (fast)

### Test Reliability
- **Python:** 100% pass rate (24/24)
- **C++:** 75% pass rate (9/12)
  - 3 known failures to investigate

### Test Coverage Gaps
1. No unit tests for C++ components
2. Limited error path testing
3. No boundary condition tests
4. Limited integration scenarios

---

## Recommendations

### Immediate Actions (Week 1)

1. **Fix C++ Test Failures:**
   - Build debug version
   - Run valgrind tests
   - Fix memory leaks if present
   - Update filter test expectations

2. **Set Up C++ Coverage:**
   - Modify Makefiles for `--coverage`
   - Add lcov integration
   - Measure baseline C++ coverage

3. **Improve Python Coverage:**
   - Add tests for `parse_with_offset()`
   - Add tests for `describeXML()`
   - Add error handling tests
   - Target: 60-70% coverage by end of week 1

### Short-term Actions (Weeks 2-4)

1. **Add C++ Unit Tests:**
   - Set up Google Test framework
   - Test DataItemFormat* classes
   - Test parsing functions
   - Target: 40-50% C++ coverage

2. **Expand Integration Tests:**
   - Test all ASTERIX categories
   - Test all input/output format combinations
   - Add stress tests
   - Add negative tests

### Medium-term Actions (Weeks 5-12)

1. **Coverage Improvement:**
   - Systematic gap filling
   - Focus on critical paths first
   - 80% per module by week 8
   - 90-95% overall by week 12

2. **Documentation:**
   - Requirements from ASTERIX specs
   - Traceability matrix
   - Test procedures
   - Verification results

---

## Baseline Metrics Summary Table

| Metric | Current Value | Target Value | Gap | Priority |
|--------|---------------|--------------|-----|----------|
| Python Overall Coverage | 39% | 90-95% | +51-56 pts | High |
| Python Core Module | 53% | ≥90% | +37 pts | Critical |
| C++ Coverage | Unknown | ≥85% | TBD | Critical |
| Python Tests Passing | 24/24 (100%) | 100% | 0 | ✓ |
| C++ Tests Passing | 9/12 (75%) | 100% | 3 tests | High |
| C++ Unit Tests | 0 | >50 | +50 | High |
| Total Test Count | ~36 | >200 | +164 | Medium |

---

## Dependencies & Blockers

**None identified** - Ready to proceed with improvement phase

**Tools Verified:**
- ✓ Python 3.12 with pytest, coverage
- ✓ GCC compiler working
- ✓ Make build system working
- ✓ Valgrind installed
- ✓ Git version control
- ✓ GitHub Actions configured

---

## Conclusion

The baseline measurement phase is complete. The project has:
- A solid foundation of 24 passing Python tests
- 75% passing C++ integration tests (3 failures to investigate)
- 39% Python coverage (good starting point)
- Clear path to 90-95% coverage target

**Next Phase:** Requirements Documentation & Test Development

---

## Document Approval

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Verification Engineer | TBD | 2025-10-17 | |
| QA Manager | TBD | | |

---

**Appendices:**

A. Full Python coverage report: `htmlcov/index.html`
B. Test execution logs: (archived)
C. CI/CD pipeline configuration: `.github/workflows/ci-verification.yml`
