# DO-278A Implementation Guide
## ASTERIX Decoder - AL-3 Compliance

**Status:** Initial Setup Complete ✓
**Next Phase:** Test Development and Coverage Improvement
**Target:** 90-95% overall coverage, ≥80% per module

---

## Executive Summary

This guide outlines the implementation plan for achieving DO-278A Assurance Level 3 (AL-3) compliance for the ASTERIX Decoder project using a Test-Driven Development approach.

**Key Decisions:**
- **Assurance Level:** AL-3 (Major)
- **Approach:** Retrofit TDD with existing codebase
- **Coverage Target:** 90-95% overall, ≥80% per module
- **CI/CD:** GitHub Actions
- **Timeline:** Phased approach (12 weeks estimated)

---

## What Has Been Set Up

### ✓ 1. DO-278 Documentation Structure

Created complete directory structure in `do-278/`:
- **Plans:** SAS, SVP, SDP (in progress), SCMP, SQAP
- **Requirements:** Templates and structure
- **Verification:** Test case directories
- **Configuration Management:** CM records structure
- **Quality Assurance:** Review records structure

**Key Documents Created:**
- `do-278/plans/Software_Accomplishment_Summary.md` - Overall compliance summary
- `do-278/plans/Software_Verification_Plan.md` - Detailed test strategy with coverage targets
- `do-278/requirements/Requirements_Template.md` - How to write requirements
- `do-278/README.md` - Quick reference guide

### ✓ 2. GitHub Actions CI/CD Pipeline

Created `.github/workflows/ci-verification.yml` with:
- **C++ Build:** Make and CMake
- **Python Testing:** Matrix for Python 3.8-3.12
- **Integration Tests:** Existing test.sh integration
- **Memory Checks:** Valgrind leak detection
- **Static Analysis:** cppcheck and clang-tidy
- **Coverage:** Placeholder for gcov/lcov (needs instrumentation)

**Triggers:**
- Every push to master, develop, feature branches
- Every pull request
- Nightly scheduled runs

### ✓ 3. Test Infrastructure

Created `tests/` directory structure:
- `tests/cpp/` - C++ unit tests (Google Test/Catch2)
- `tests/python/` - Python unit tests (pytest)
- `tests/integration/` - Integration test scripts
- `tests/README.md` - Comprehensive testing guide

### ✓ 4. Coverage Framework

Defined coverage targets and measurement approach:
- Overall: 90-95%
- src/asterix/: ≥90% (critical path)
- src/engine/: ≥85%
- src/main/: ≥80%
- src/python/: ≥85%
- Python module: ≥90%

### ✓ 5. Updated CLAUDE.md

Added project architecture and development guidelines for future Claude Code instances.

---

## Next Steps (Recommended Order)

### Phase 1: Baseline Measurement (Week 1)

**Goal:** Understand current state

**Tasks:**
1. ✓ Build project successfully
2. ✓ Run existing tests
3. **TODO:** Measure current coverage
4. **TODO:** Document baseline metrics

**Commands:**
```bash
# Build C++
cd src && make clean && make && make install

# Run existing tests
cd install/test && ./test.sh

# Measure Python coverage
pip install coverage pytest pytest-cov
pytest asterix/test/ --cov=asterix --cov-report=html --cov-report=term

# View results
open htmlcov/index.html

# TODO: Add C++ coverage instrumentation
```

**Deliverable:** Baseline coverage report

### Phase 2: Requirements Documentation (Weeks 2-3)

**Goal:** Document what the software does

**Tasks:**
1. **Extract HLRs from ASTERIX specifications:**
   - One HLR per ASTERIX category (CAT001, CAT048, CAT062, etc.)
   - One HLR per major feature (parsing, I/O, formatting, Python API)

2. **Derive LLRs from HLRs:**
   - One LLR per ASTERIX data item
   - One LLR per algorithm/function

3. **Create Requirements Traceability Matrix:**
   - Start with spreadsheet or Markdown table
   - Link HLR ↔ LLR ↔ Code ↔ Test

**Example Requirements:**
```
REQ-HLR-048: Parse ASTERIX Category 048 Data Blocks
  ↓
REQ-LLR-048-010: Parse Data Item I048/010 (SAC/SIC)
REQ-LLR-048-020: Parse Data Item I048/020 (Target Report Descriptor)
REQ-LLR-048-040: Parse Data Item I048/040 (Position in WGS-84)
... (one per data item)
```

**Template:** `do-278/requirements/Requirements_Template.md`

**Deliverable:**
- `do-278/requirements/High_Level_Requirements.md`
- `do-278/requirements/Low_Level_Requirements.md`
- `do-278/requirements/Requirements_Traceability_Matrix.md`

### Phase 3: Unit Test Development (Weeks 4-7)

**Goal:** Achieve ≥80% per-module coverage

**Priority Order (Critical Path First):**

#### 3.1 Core Parser Tests (Weeks 4-5)
**Target:** src/asterix/ ≥90%

Start with most critical components:
```cpp
// tests/cpp/test_dataitem_format.cpp
#include <gtest/gtest.h>
#include "DataItemFormatFixed.h"

TEST(DataItemFormatFixed, ParseValidData) {
    // Test CAT048/I010 (SAC/SIC)
    unsigned char data[] = {0x10, 0x20}; // SAC=16, SIC=32
    DataItemFormatFixed parser;
    auto result = parser.parse(data, 2);
    EXPECT_EQ(result["SAC"], 16);
    EXPECT_EQ(result["SIC"], 32);
}
```

**Focus Areas:**
- DataItemBits parsing
- DataItemFormat* classes (Fixed, Variable, Compound, Repetitive, Explicit, BDS)
- Category parsing
- UAP processing
- CRC validation

#### 3.2 I/O Engine Tests (Week 6)
**Target:** src/engine/ ≥85%

Test device and format handling:
- Device factories
- Format parsers (PCAP, HDLC, etc.)
- Channel management

#### 3.3 Python Bindings Tests (Week 7)
**Target:** src/python/ ≥85%, asterix/ ≥90%

Expand existing tests:
```python
# tests/python/test_parse_categories.py
import pytest
import asterix

def test_parse_cat048_complete():
    """Test complete CAT048 parsing with all data items"""
    with open('install/sample_data/cat_034_048.pcap', 'rb') as f:
        data = f.read()

    results = asterix.parse(data)
    assert len(results) > 0

    # Verify all expected data items present
    cat048_records = [r for r in results if r['category'] == 48]
    assert len(cat048_records) > 0

    # Check specific data items
    record = cat048_records[0]
    assert 'I048/010' in record  # SAC/SIC
    assert 'I048/140' in record  # Time of Day
    # ... more assertions
```

**Deliverable:** ≥80% coverage per module

### Phase 4: Integration & System Testing (Weeks 8-9)

**Goal:** Verify end-to-end functionality

**Test Scenarios:**
1. CLI with all input formats (PCAP, Raw, HDLC, GPS, FINAL)
2. CLI with all output formats (JSON, XML, Text, Line)
3. Python API complete workflows
4. Multicast network input
5. Error handling and recovery
6. Performance and stress testing

**Expand existing integration tests:**
```bash
# tests/integration/test_all_categories.sh
#!/bin/bash
# Test parsing all supported ASTERIX categories

for cat in 001 002 004 010 011 019 020 021 023 025 034 048 062 065 240 252; do
    echo "Testing CAT$cat..."
    if [ -f "install/sample_data/cat_${cat}.pcap" ]; then
        ./install/asterix -P -f install/sample_data/cat_${cat}.pcap -j > /dev/null
        if [ $? -eq 0 ]; then
            echo "✓ CAT$cat OK"
        else
            echo "✗ CAT$cat FAILED"
        fi
    fi
done
```

**Deliverable:** Complete integration test suite

### Phase 5: Coverage Achievement (Weeks 10-11)

**Goal:** Reach 90-95% overall coverage

**Process:**
1. Generate coverage report
2. Identify gaps (red/yellow areas)
3. Prioritize by:
   - Safety impact (high priority)
   - Code complexity (high priority)
   - Frequency of use
4. Add targeted tests for gaps
5. Re-measure and iterate

**Tools:**
```bash
# C++ coverage
cd src
make clean
CXXFLAGS="--coverage" LDFLAGS="--coverage" make
make install
cd ../install/test && ./test.sh
lcov --capture --directory ../../src --output-file coverage.info
genhtml coverage.info --output-directory coverage_html

# Python coverage
pytest tests/python/ asterix/test/ --cov=asterix --cov-report=html

# Review
open coverage_html/index.html
open htmlcov/index.html
```

**Focus on:**
- Uncovered error paths
- Boundary conditions
- Edge cases
- Complex algorithms

**Deliverable:** 90-95% overall coverage achieved

### Phase 6: Documentation & Reviews (Week 12)

**Goal:** Complete all DO-278A records

**Tasks:**
1. **Complete Requirements:**
   - Finalize all HLRs and LLRs
   - Complete RTM (100% traceability)

2. **Software Design Description (SDD):**
   - Document architecture (use existing CLAUDE.md as base)
   - Document key algorithms
   - Document interfaces

3. **Verification Results:**
   - Document all test results
   - Generate final coverage report
   - Document any deviations/justifications

4. **Conduct Reviews:**
   - Requirements review
   - Design review
   - Code review
   - Test review

5. **Software Configuration Index:**
   - Identify baseline (git tag)
   - List all configuration items
   - Document build process

**Deliverable:** Complete DO-278A compliance package

---

## Quick Start Commands

### Build Everything
```bash
# C++ executable
cd src && make clean && make && make install

# Python module
python setup.py build && python setup.py install
```

### Run Tests
```bash
# Existing integration tests
cd install/test && ./test.sh

# Python tests with coverage
pytest asterix/test/ --cov=asterix --cov-report=html --cov-report=term

# Memory check
cd install/test && ./valgrind_test.sh
```

### Measure Coverage
```bash
# Python
pytest --cov=asterix --cov-report=html
open htmlcov/index.html

# C++ (TODO: needs Makefile updates for --coverage flags)
```

### Run CI Locally
```bash
# Install act (https://github.com/nektos/act)
brew install act  # macOS
# or
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash

# Run GitHub Actions locally
act push
```

---

## Key Files Reference

### Documentation
- `do-278/README.md` - DO-278 overview
- `do-278/plans/Software_Accomplishment_Summary.md` - Compliance summary
- `do-278/plans/Software_Verification_Plan.md` - Test strategy
- `do-278/requirements/Requirements_Template.md` - How to write requirements

### Code
- `src/asterix/` - Core parser (CRITICAL - target ≥90%)
- `src/engine/` - I/O engine (target ≥85%)
- `src/main/` - CLI application (target ≥80%)
- `src/python/` - Python bindings (target ≥85%)
- `asterix/` - Python module (target ≥90%)

### Tests
- `install/test/test.sh` - Existing integration tests
- `asterix/test/` - Existing Python tests
- `tests/cpp/` - New C++ unit tests
- `tests/python/` - New Python unit tests
- `tests/integration/` - New integration tests

### CI/CD
- `.github/workflows/ci-verification.yml` - Main verification pipeline
- `.github/workflows/codeql-analysis.yml` - Security analysis
- `.github/workflows/python-publish.yml` - PyPI release

### Configuration
- `asterix/config/asterix_cat*.xml` - ASTERIX category definitions
- `install/config/asterix.ini` - Configuration file list

---

## Recommended Tools

### Development
- **IDE:** VSCode with C/C++ and Python extensions
- **Compiler:** GCC with C++11 support
- **Build:** Make, CMake
- **Version Control:** Git

### Testing
- **C++ Unit Test:** Google Test or Catch2
- **Python Unit Test:** pytest, unittest
- **Coverage:** gcov/lcov (C++), coverage.py (Python)
- **Memory:** Valgrind
- **Static Analysis:** cppcheck, clang-tidy

### CI/CD
- **Platform:** GitHub Actions
- **Coverage Reporting:** Codecov or Coveralls

---

## Common Issues & Solutions

### Issue: Low coverage on error paths
**Solution:** Mock error conditions, force error paths with invalid data

### Issue: Can't test network multicast
**Solution:** Create mock/stub for network I/O, test with recorded data

### Issue: Complex algorithm hard to test
**Solution:** Refactor into smaller testable units, use test vectors from ASTERIX specs

### Issue: Legacy code not testable
**Solution:** Add seams, extract interfaces, refactor incrementally

---

## Success Criteria Checklist

- [ ] Overall coverage ≥90%
- [ ] All module coverage ≥80%
- [ ] All requirements documented and traced
- [ ] All tests passing
- [ ] No memory leaks (Valgrind clean)
- [ ] CI pipeline green
- [ ] All reviews complete
- [ ] DO-278A documentation complete

---

## Resources

### DO-278A
- [DO-278A Overview](https://afuzion.com/do-278a/)
- [LDRA DO-278 Guide](https://ldra.com/do-278/)

### Testing
- [Google Test](https://github.com/google/googletest)
- [pytest](https://docs.pytest.org/)
- [Code Coverage Best Practices](https://testing.googleblog.com/2020/08/code-coverage-best-practices.html)

### ASTERIX
- [EUROCONTROL ASTERIX](http://www.eurocontrol.int/services/asterix)
- [ASTERIX Specs (structured)](https://zoranbosnjak.github.io/asterix-specs/)

---

**Last Updated:** 2025-10-17
**Phase:** Initial Setup Complete
**Next:** Baseline Coverage Measurement
