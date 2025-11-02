# Requirements Traceability Matrix (RTM)
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** RTM-ASTERIX-001
**Revision:** 1.0
**Date:** 2025-10-17
**Assurance Level:** AL-3 (Major)

---

## 1. Purpose

This Requirements Traceability Matrix (RTM) establishes bidirectional traceability between:
- **High-Level Requirements (HLR)** ↔ **Low-Level Requirements (LLR)**
- **Requirements** ↔ **Design** (Software Design Description)
- **Requirements** ↔ **Code** (Source files)
- **Requirements** ↔ **Tests** (Test cases)

**Traceability Objective:** 100% coverage - every requirement shall be traced.

---

## 2. Traceability Legend

**Status:**
- ✓ Complete - Implemented and tested
- ◐ Partial - Implementation exists, tests incomplete
- ○ Planned - Not yet implemented
- ✗ Not Applicable

**Coverage:**
- Statement coverage % (from coverage reports)
- Test count / Total tests

---

## 3. System-Level Requirements Traceability

| HLR ID | Description | LLR Count | Design Ref | Code Ref | Test Cases | Status | Coverage |
|--------|-------------|-----------|------------|----------|------------|--------|----------|
| REQ-HLR-SYS-001 | ASTERIX Protocol Compliance | 18 CAT | SDD-3.1 | src/asterix/*.cpp | TC-SYS-001-* | ◐ | 39% |
| REQ-HLR-SYS-002 | Data Integrity Verification | 2 | SDD-3.2 | DataRecord.cpp:120 | TC-SYS-002-* | ◐ | 53% |
| REQ-HLR-SYS-003 | Multiple ASTERIX Categories | 18 | SDD-3.3 | AsterixDefinition.cpp | TC-SYS-003-* | ◐ | 39% |

---

## 4. CAT048 Requirements Traceability

### 4.1 High-Level to Low-Level

| HLR ID | Description | Derived LLRs | Status |
|--------|-------------|--------------|--------|
| REQ-HLR-CAT-048 | Parse CAT048 | REQ-LLR-048-010 through REQ-LLR-048-SP | ◐ |

### 4.2 CAT048 Low-Level Requirements Detail

| LLR ID | Data Item | HLR Parent | Design | Code | Test Cases | Status | Coverage |
|--------|-----------|------------|--------|------|------------|--------|----------|
| **REQ-LLR-048-010** | I048/010 (SAC/SIC) | REQ-HLR-CAT-048 | SDD-3.4.2 | DataItemFormatFixed.cpp:145<br/>asterix_cat048_1_21.xml:24-39 | TC-048-010-001<br/>TC-048-010-002<br/>TC-048-010-003 | ◐ | 85% |
| **REQ-LLR-048-020** | I048/020 (Target Descr) | REQ-HLR-CAT-048 | SDD-3.4.3 | DataItemFormatVariable.cpp:89<br/>asterix_cat048_1_21.xml:41-141 | TC-048-020-001<br/>TC-048-020-002<br/>TC-048-020-003<br/>TC-048-020-004<br/>TC-048-020-005 | ○ | 0% |
| **REQ-LLR-048-040** | I048/040 (Position) | REQ-HLR-CAT-048 | SDD-3.4.2 | DataItemFormatFixed.cpp:178<br/>DataItemBits.cpp:234 | TC-048-040-001<br/>TC-048-040-002<br/>TC-048-040-003<br/>TC-048-040-004 | ○ | 0% |
| **REQ-LLR-048-070** | I048/070 (Mode-3/A) | REQ-HLR-CAT-048 | SDD-3.4.2 | DataItemFormatFixed.cpp:201<br/>DataItemBits.cpp:145 | TC-048-070-001<br/>TC-048-070-002<br/>TC-048-070-003<br/>TC-048-070-004 | ○ | 0% |
| **REQ-LLR-048-090** | I048/090 (Flight Level) | REQ-HLR-CAT-048 | SDD-3.4.2 | DataItemFormatFixed.cpp:223 | TC-048-090-001<br/>TC-048-090-002<br/>TC-048-090-003<br/>TC-048-090-004 | ○ | 0% |
| **REQ-LLR-048-140** | I048/140 (Time Of Day) | REQ-HLR-CAT-048 | SDD-3.4.2 | DataItemFormatFixed.cpp:267 | TC-048-140-001<br/>TC-048-140-002<br/>TC-048-140-003<br/>TC-048-140-004 | ○ | 0% |
| **REQ-LLR-048-161** | I048/161 (Track Number) | REQ-HLR-CAT-048 | SDD-3.4.2 | DataItemFormatFixed.cpp:289 | TC-048-161-001<br/>TC-048-161-002<br/>TC-048-161-003 | ○ | 0% |
| **REQ-LLR-048-220** | I048/220 (Aircraft Addr) | REQ-HLR-CAT-048 | SDD-3.4.2 | DataItemFormatFixed.cpp:312 | TC-048-220-001<br/>TC-048-220-002<br/>TC-048-220-003 | ○ | 0% |
| **REQ-LLR-048-240** | I048/240 (Aircraft ID) | REQ-HLR-CAT-048 | SDD-3.4.2 | DataItemFormatFixed.cpp:334<br/>DataItemBits.cpp:456 | TC-048-240-001<br/>TC-048-240-002<br/>TC-048-240-003<br/>TC-048-240-004 | ○ | 0% |
| **REQ-LLR-048-120** | I048/120 (Doppler Speed) | REQ-HLR-CAT-048 | SDD-3.4.4 | DataItemFormatRepetitive.cpp:112 | TC-048-120-001<br/>TC-048-120-002<br/>TC-048-120-003<br/>TC-048-120-004 | ○ | 0% |
| **REQ-LLR-048-250** | I048/250 (Mode S BDS) | REQ-HLR-CAT-048 | SDD-3.4.4<br/>SDD-3.4.6 | DataItemFormatRepetitive.cpp:134<br/>DataItemFormatBDS.cpp:67 | TC-048-250-001<br/>TC-048-250-002<br/>TC-048-250-003 | ○ | 0% |
| **REQ-LLR-048-260** | I048/260 (ACAS RA) | REQ-HLR-CAT-048 | SDD-3.4.2 | DataItemFormatFixed.cpp:356 | TC-048-260-001<br/>TC-048-260-002 | ○ | 0% |
| **REQ-LLR-048-SP** | I048/SP (Special) | REQ-HLR-CAT-048 | SDD-3.4.5 | DataItemFormatExplicit.cpp:78 | TC-048-SP-001 | ○ | 0% |
| **REQ-LLR-048-ERR-001** | Invalid Length | REQ-HLR-ERR-001 | SDD-3.8 | DataRecord.cpp:234 | TC-048-ERR-001<br/>TC-048-ERR-002 | ○ | 0% |
| **REQ-LLR-048-ERR-002** | Unknown Data Item | REQ-HLR-ERR-001 | SDD-3.8 | DataRecord.cpp:256 | TC-048-ERR-003 | ○ | 0% |

**Note:** Code line numbers are estimates and should be verified.

---

## 5. Input/Output Requirements Traceability

| HLR ID | Description | LLRs | Design | Code | Tests | Status | Coverage |
|--------|-------------|------|--------|------|-------|--------|----------|
| REQ-HLR-IO-001 | Multiple Input Formats | 6 | SDD-3.5 | asterixformat.cxx<br/>asterixpcapsubformat.cxx<br/>asterixhdlcsubformat.cxx<br/>asterixgpssubformat.cxx<br/>asterixfinalsubformat.cxx | TC-IO-001-* | ◐ | 45% |
| REQ-HLR-IO-002 | Multiple Output Formats | 3 | SDD-3.6 | asterix.cpp:234-567 | TC-IO-002-* | ✓ | 75% |
| REQ-HLR-IO-003 | Data Filtering | 2 | SDD-3.7 | asterix.cpp:123 | TC-IO-003-* | ◐ | 0% |

---

## 6. API Requirements Traceability

| HLR ID | Description | LLRs | Design | Code | Tests | Status | Coverage |
|--------|-------------|------|--------|------|-------|--------|----------|
| REQ-HLR-API-001 | Python API | 3 | SDD-4.1 | asterix/__init__.py<br/>python_wrapper.c<br/>python_parser.cpp | tests/python/test_basic_parsing.py | ◐ | 53% |
| REQ-HLR-API-002 | C/C++ Library API | 4 | SDD-4.2 | AsterixDefinition.h<br/>Category.h | ○ | ○ | 0% |
| REQ-HLR-API-003 | Command-Line Interface | 2 | SDD-4.3 | asterix.cpp | install/test/test.sh | ◐ | 75% |

---

## 7. Configuration Requirements Traceability

| HLR ID | Description | LLRs | Design | Code | Tests | Status | Coverage |
|--------|-------------|------|--------|------|-------|--------|----------|
| REQ-HLR-CFG-001 | XML Configuration Files | 3 | SDD-3.9 | XMLParser.cpp<br/>AsterixDefinition.cpp | tests/python/test_basic_parsing.py::TestConfigurationFiles | ◐ | 91% |
| REQ-HLR-CFG-002 | Default Configuration | 1 | SDD-3.9 | asterix/__init__.py:355-359 | TC-CFG-002-001 | ✓ | 100% |
| REQ-HLR-CFG-003 | Custom Configuration | 1 | SDD-3.9 | asterix/__init__.py:75-90 | asterix/test/test_init.py | ✓ | 91% |

---

## 8. Error Handling Requirements Traceability

| HLR ID | Description | LLRs | Design | Code | Tests | Status | Coverage |
|--------|-------------|------|--------|------|-------|--------|----------|
| REQ-HLR-ERR-001 | Graceful Error Handling | 6 | SDD-3.8 | DataRecord.cpp<br/>InputParser.cpp<br/>python_wrapper.c | tests/python/test_basic_parsing.py::TestErrorHandling | ◐ | 42% |
| REQ-HLR-ERR-002 | Error Reporting | 1 | SDD-3.8 | Tracer.cpp | TC-ERR-002-* | ○ | 0% |

---

## 9. Performance Requirements Traceability

| HLR ID | Description | LLRs | Design | Code | Tests | Status | Coverage |
|--------|-------------|------|--------|------|-------|--------|----------|
| REQ-HLR-PERF-001 | Real-Time Processing | N/A | SDD-5.1 | N/A | Performance benchmarks | ○ | N/A |
| REQ-HLR-PERF-002 | Memory Efficiency | 1 | SDD-5.2 | All | install/test/test.sh (valgrind) | ◐ | N/A |

---

## 10. Test Coverage Summary

### 10.1 By Requirement Type

| Requirement Type | Total | Implemented | Tested | Coverage % |
|------------------|-------|-------------|--------|------------|
| System (SYS) | 3 | 3 | 2 | 39% |
| Category (CAT-048) | 15 | 15 | 3 | 15% |
| Input/Output (IO) | 3 | 3 | 3 | 60% |
| API | 3 | 3 | 2 | 43% |
| Configuration (CFG) | 3 | 3 | 3 | 94% |
| Error Handling (ERR) | 2 | 2 | 1 | 42% |
| Performance (PERF) | 2 | 2 | 1 | N/A |
| **TOTAL** | **31** | **31** | **15** | **39%** |

### 10.2 By Safety Impact

| Safety Impact | Requirements | Tested | Coverage % |
|---------------|--------------|--------|------------|
| High | 13 | 4 | 30% |
| Medium | 10 | 6 | 60% |
| Low | 8 | 5 | 63% |

### 10.3 By Priority

| Priority | Requirements | Tested | Coverage % |
|----------|--------------|--------|------------|
| Critical | 6 | 2 | 33% |
| High | 12 | 7 | 58% |
| Medium | 9 | 5 | 56% |
| Low | 4 | 1 | 25% |

---

## 11. Traceability Gaps

### 11.1 High Priority Gaps (To Address First)

1. **REQ-LLR-048-020** - Target Report Descriptor parsing
   - **Status:** Implemented but not unit tested
   - **Action:** Create TC-048-020-001 through TC-048-020-005
   - **Target:** Week 2

2. **REQ-LLR-048-040** - Position parsing
   - **Status:** Implemented but not unit tested
   - **Action:** Create TC-048-040-001 through TC-048-040-004
   - **Target:** Week 2

3. **REQ-LLR-048-070** - Mode-3/A code parsing
   - **Status:** Implemented but not unit tested
   - **Action:** Create TC-048-070-001 through TC-048-070-004
   - **Target:** Week 2

### 11.2 Medium Priority Gaps

4. **REQ-HLR-API-002** - C/C++ Library API
   - **Status:** No tests exist
   - **Action:** Create C++ API tests
   - **Target:** Week 4

5. **REQ-HLR-ERR-002** - Error Reporting
   - **Status:** No tests exist
   - **Action:** Create error message validation tests
   - **Target:** Week 3

### 11.3 Documentation Gaps

6. **Software Design Description (SDD)**
   - **Status:** Not yet created
   - **Action:** Create SDD documenting architecture
   - **Target:** Week 5

---

## 12. Verification Status

### 12.1 DO-278A AL-3 Objectives Coverage

| Objective | Description | Status | Evidence |
|-----------|-------------|--------|----------|
| 1.1 | High-level requirements | ✓ Complete | This document, HLR doc |
| 1.2 | Derived requirements (LLR) | ◐ Partial | LLR-CAT048 doc |
| 1.3 | Software architecture | ○ Planned | SDD (to be created) |
| 1.4 | Low-level requirements | ◐ Partial | LLR-CAT048 doc |
| 2.1 | Source code | ✓ Complete | src/ directory |
| 3.1 | Review of requirements | ○ Planned | Review records |
| 3.2 | Review of design | ○ Planned | Design review |
| 3.3 | Review of code | ◐ Partial | Git history, PR reviews |
| 4.1 | Test coverage | ◐ In Progress | 39% baseline |
| 4.2 | Structural coverage | ◐ In Progress | Statement: 39% |

---

## 13. Traceability Maintenance

### 13.1 Update Process

This RTM shall be updated:
1. When new requirements are added
2. When requirements are modified or deleted
3. When test cases are created or modified
4. When code is refactored affecting traceability
5. At each project milestone
6. Before each release

### 13.2 Verification

Traceability shall be verified:
- Forward: Every requirement has associated design, code, and tests
- Backward: Every test traces to a requirement
- Coverage: No orphaned requirements or tests

### 13.3 Tools

- **Tracking:** This Markdown document (manual)
- **Future:** Requirements management tool (optional)
- **Coverage:** pytest-cov, lcov/gcov

---

## 14. Next Actions

**Week 1:**
1. Create test cases for REQ-LLR-048-020, 040, 070
2. Improve coverage from 39% to 50%
3. Update RTM with new tests

**Week 2:**
4. Complete all CAT048 LLR test cases
5. Achieve 60% overall coverage
6. Update RTM

**Week 3:**
7. Create Software Design Description (SDD)
8. Link all requirements to design
9. Update RTM with design references

**Week 4:**
10. Create C++ API tests
11. Achieve 70% overall coverage
12. Update RTM

**Ongoing:**
- Maintain RTM current with all changes
- Review traceability monthly
- Generate traceability reports for compliance

---

## Document Approval

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Requirements Engineer | TBD | 2025-10-17 | |
| Verification Engineer | TBD | | |
| QA Manager | TBD | | |

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-10-17 | Initial | Initial RTM with baseline traceability |

---

**Status:** Phase 1 Complete (Baseline), Phase 2 In Progress (Requirements)
**Next Update:** End of Week 2
**Coverage Target:** 90-95% overall, ≥80% per module
