# Software Verification Plan (SVP)
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** SVP-ASTERIX-001
**Revision:** 1.0
**Date:** 2025-10-17
**Assurance Level:** AL-3 (Major)

---

## 1. Introduction

### 1.1 Purpose
This Software Verification Plan defines the verification activities, methods, and criteria for the ASTERIX Decoder software to comply with DO-278A requirements for Assurance Level AL-3.

### 1.2 Scope
This plan covers:
- Requirements-based testing
- Structural coverage analysis
- Integration testing
- Verification independence
- Test-Driven Development (TDD) approach

### 1.3 Assurance Level
**AL-3 (Major)** - Requires:
- Requirements-based testing
- Design review
- Code review
- Structural coverage analysis at statement and decision level

---

## 2. Verification Strategy

### 2.1 Test-Driven Development Approach

Given the existing codebase, we will employ a **retrofit TDD approach**:

1. **Phase 1: Baseline Establishment**
   - Build existing code successfully
   - Run existing tests
   - Measure current coverage
   - Document current functionality as requirements

2. **Phase 2: Requirements Documentation**
   - Extract requirements from ASTERIX specifications
   - Document existing behavior as derived requirements
   - Create Requirements Traceability Matrix

3. **Phase 3: Test Development**
   - Write test cases for each requirement
   - Achieve coverage targets progressively
   - Refactor code as needed for testability

4. **Phase 4: Coverage Achievement**
   - Measure and report coverage
   - Identify gaps
   - Add tests until targets met
   - Document untestable/dead code

### 2.2 Coverage Targets

**Overall Project Goals:**
- **Overall Coverage: 90-95%** across all modules
- **Module Coverage: >80%** for each individual module/subsystem

**Specific Coverage Metrics for AL-3:**
- **Statement Coverage:** ≥95% overall, ≥80% per module
- **Decision Coverage:** ≥95% overall, ≥80% per module
- **MC/DC Coverage:** Not required for AL-3 (only AL-1, AL-2)

**Module Breakdown:**
- Core Parser (`src/asterix/`): ≥90% (critical path)
- I/O Engine (`src/engine/`): ≥85%
- CLI Application (`src/main/`): ≥80%
- Python Bindings (`src/python/`): ≥85%

---

## 3. Verification Methods

### 3.1 Requirements-Based Testing

**Objective:** Verify that all requirements are tested and pass.

**Activities:**
- Review requirements for testability
- Develop test cases for each requirement
- Execute tests and record results
- Maintain Requirements Traceability Matrix (RTM)

**Test Categories:**
1. **Functional Tests** - Verify correct parsing of ASTERIX data
2. **Error Handling Tests** - Verify robustness
3. **Boundary Tests** - Verify edge cases
4. **Interface Tests** - Verify API contracts

### 3.2 Structural Coverage Analysis

**Objective:** Ensure code is adequately exercised.

**Tools:**
- **C/C++ Coverage:** gcov, lcov
- **Python Coverage:** coverage.py
- **Reporting:** HTML coverage reports, CI integration

**Process:**
1. Instrument code for coverage
2. Run all test suites
3. Generate coverage reports
4. Identify uncovered code
5. Add tests or justify exclusions
6. Document results

**Exclusion Criteria:**
- Dead code (to be removed)
- Defensive code proven unreachable
- Debug/diagnostic code
- Generated code (documented)

### 3.3 Reviews

**Design Review:**
- Review software architecture
- Review data structures
- Review algorithms
- Review interfaces

**Code Review:**
- Review against coding standards
- Review for complexity
- Review for testability
- Review error handling

**Test Review:**
- Review test cases for completeness
- Review test procedures
- Review test results

---

## 4. Test Infrastructure

### 4.1 Test Frameworks

**C/C++ Testing:**
- Framework: Google Test (gtest/gmock) or Catch2
- Location: `tests/cpp/`
- Build: CMake integration

**Python Testing:**
- Framework: unittest (existing), pytest (additional)
- Location: `tests/python/`, `asterix/test/`
- Build: setup.py test

**Integration Testing:**
- Location: `install/test/` (existing)
- Scripts: Bash test harness
- Data: Sample ASTERIX files

### 4.2 Continuous Integration

**GitHub Actions Workflow:**
- Build on: Ubuntu (primary), macOS, Windows (Cygwin)
- Triggers: Push, Pull Request, Scheduled
- Matrix: Python 3.8, 3.9, 3.10, 3.11, 3.12

**CI Pipeline:**
```
Build → Unit Tests → Integration Tests → Coverage → Report
```

**Artifacts:**
- Coverage reports (HTML, Cobertura XML)
- Test results (JUnit XML)
- Build logs

### 4.3 Test Data

**ASTERIX Sample Data:**
- CAT001, CAT002, CAT004, CAT008, CAT010, CAT011
- CAT019, CAT020, CAT021, CAT023, CAT025
- CAT034, CAT048, CAT062, CAT065, CAT240, CAT252
- PCAP files, raw binary, GPS format, HDLC format

**Test Data Sources:**
- Existing: `install/sample_data/`
- Expected outputs: `install/sample_output/`
- Additional: EUROCONTROL specifications

---

## 5. Test Levels

### 5.1 Unit Testing

**Scope:** Individual functions and classes

**Test Cases:**
- DataItemBits parsing functions
- DataItemFormat* classes (Fixed, Variable, Compound, etc.)
- Category parsing
- UAP processing
- Utils functions
- XML parsing

**Success Criteria:**
- All unit tests pass
- ≥95% statement coverage
- ≥95% decision coverage
- ≥80% per-file coverage

### 5.2 Integration Testing

**Scope:** Component interactions

**Test Cases:**
- Parser + Format handlers
- Input devices + Parser
- Parser + Output formatters
- Python bindings + C++ parser
- Configuration loading + Parsing

**Success Criteria:**
- All integration tests pass
- End-to-end scenarios work
- Interface contracts verified

### 5.3 System Testing

**Scope:** Complete system validation

**Test Cases:**
- CLI with various options
- Multiple input formats (PCAP, Raw, HDLC, GPS, FINAL)
- Multiple output formats (JSON, XML, Text)
- Network multicast input
- Python module API
- Error conditions and recovery

**Success Criteria:**
- All system tests pass
- Performance acceptable
- No memory leaks (Valgrind clean)

### 5.4 Regression Testing

**Scope:** Prevent introduction of defects

**Process:**
- Run full test suite on every commit (CI)
- Compare outputs against known good baseline
- Existing regression tests in `install/test/test.sh`

---

## 6. Verification Procedures

### 6.1 Test Execution Process

1. **Pre-Test:**
   - Verify test environment setup
   - Build software under test
   - Prepare test data

2. **Test Execution:**
   - Run automated test suite
   - Record results
   - Capture coverage data

3. **Post-Test:**
   - Analyze results
   - Generate coverage report
   - Document failures
   - Create problem reports for defects

### 6.2 Test Pass/Fail Criteria

**Pass Criteria:**
- All test cases execute successfully
- Coverage targets met
- No unresolved high-severity defects
- Traceability complete

**Fail Criteria:**
- Any test case fails
- Coverage below targets
- High-severity defects exist
- Traceability gaps exist

### 6.3 Problem Reporting

**Process:**
- Create GitHub issue for each defect
- Classify by severity (Critical, Major, Minor)
- Track to resolution
- Verify fix with regression test
- Document in Problem Report

**Severity Definitions:**
- **Critical:** System crash, data corruption, safety impact
- **Major:** Incorrect results, functional failure
- **Minor:** Cosmetic, performance, usability

---

## 7. Coverage Analysis

### 7.1 Coverage Measurement

**Tools:**
- C/C++: gcov + lcov
- Python: coverage.py
- Integration: Codecov or Coveralls

**Reporting:**
- Per-file coverage
- Per-function coverage
- Per-module coverage
- Overall coverage
- Trend tracking

### 7.2 Coverage Targets by Module

| Module | Component | Statement | Decision | Notes |
|--------|-----------|-----------|----------|-------|
| Core Parser | src/asterix/*.cpp | ≥90% | ≥90% | Critical |
| I/O Engine | src/engine/*.cpp | ≥85% | ≥85% | Important |
| CLI | src/main/*.cpp | ≥80% | ≥80% | Standard |
| Python | src/python/*.c | ≥85% | ≥85% | Important |
| **Overall** | **All** | **≥90%** | **≥90%** | **Target** |

### 7.3 Coverage Improvement Process

1. Run tests with coverage enabled
2. Generate coverage report
3. Identify uncovered code:
   - **Red (0%)**: High priority - add tests
   - **Yellow (<80%)**: Medium priority - improve
   - **Green (≥80%)**: Good - maintain
4. Write tests for uncovered code
5. Repeat until targets met
6. Document justified exclusions

---

## 8. Traceability

### 8.1 Requirements Traceability Matrix (RTM)

**Format:** Bidirectional traceability

| Requirement ID | Description | Design | Code | Test Case(s) | Status |
|----------------|-------------|--------|------|--------------|--------|
| REQ-001 | Parse CAT048 | SDD-3.2 | DataItemFormat*.cpp | TC-048-001 | Pass |

**Tool:** Spreadsheet or requirements management tool

### 8.2 Traceability Objectives

- Every requirement has ≥1 test case
- Every test case traces to ≥1 requirement
- Every module traces to requirements
- 100% traceability coverage

---

## 9. Test Schedule

### 9.1 Phased Approach

**Phase 1: Foundation (Weeks 1-2)**
- Set up test infrastructure
- Configure CI/CD
- Establish baseline coverage

**Phase 2: Unit Testing (Weeks 3-6)**
- Write unit tests
- Achieve 80% module coverage
- Document requirements

**Phase 3: Integration Testing (Weeks 7-8)**
- Write integration tests
- Achieve 85% overall coverage

**Phase 4: System Testing (Weeks 9-10)**
- Write system tests
- Achieve 90-95% overall coverage

**Phase 5: Documentation (Weeks 11-12)**
- Complete traceability
- Generate verification reports
- Conduct reviews

### 9.2 Test Execution Frequency

- **Commit:** Smoke tests (fast unit tests)
- **Pull Request:** Full unit + integration tests
- **Nightly:** Full test suite + coverage analysis
- **Release:** Complete verification + reviews

---

## 10. Verification Independence

### 10.1 DO-278A AL-3 Requirements

For AL-3, verification may be performed by:
- Developer with oversight (acceptable)
- Independent verification team (preferred)

### 10.2 Independence Strategy

**Current Approach:**
- Developer performs testing
- Code review by different developer
- CI automation provides independence
- QA reviews results

**Review Requirements:**
- Requirements reviewed by stakeholder
- Design reviewed by peer
- Code reviewed by peer
- Tests reviewed by QA

---

## 11. Tool Qualification

### 11.1 Verification Tools Assessment

| Tool | Purpose | Qualification Needed? | Rationale |
|------|---------|----------------------|-----------|
| GCC | Compiler | No | Industry standard |
| gcov/lcov | Coverage | No | Results verified |
| Valgrind | Memory check | No | Results verified |
| gtest | Unit test | No | Test framework only |
| GitHub Actions | CI | No | Execution environment |

### 11.2 Tool Validation

- Coverage tools: Manually verify sample coverage
- Test frameworks: Review test execution logs
- CI: Review build and test artifacts

---

## 12. Verification Records

### 12.1 Documents to Produce

- Software Verification Cases and Procedures (SVCP)
- Software Verification Results (SVR)
- Coverage Analysis Report
- Requirements Traceability Matrix
- Problem Reports and Resolutions
- Review Records

### 12.2 Record Retention

- All verification records maintained in Git
- Test results archived per release
- Coverage reports archived per release
- Traceability matrix version-controlled

---

## 13. Success Criteria

### 13.1 Verification Complete When:

- [ ] All requirements have test cases
- [ ] All tests pass
- [ ] Overall coverage ≥90%
- [ ] All module coverage ≥80%
- [ ] RTM 100% complete
- [ ] All reviews completed
- [ ] No open critical/major defects
- [ ] All verification records complete

### 13.2 Acceptance

Verification is complete and acceptable when all success criteria are met and documented in the Software Verification Results (SVR).

---

## Document Approval

| Role | Name | Signature | Date |
|------|------|-----------|------|
| Project Lead | TBD | | |
| Verification Lead | TBD | | |
| QA Manager | TBD | | |

---

**Document History**

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-10-17 | Initial | Initial SVP creation |
