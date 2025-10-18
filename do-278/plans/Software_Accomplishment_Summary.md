# Software Accomplishment Summary (SAS)
## ASTERIX Decoder - DO-278A Compliance

**Document ID:** SAS-ASTERIX-001
**Revision:** 1.0
**Date:** 2025-10-17
**Assurance Level:** AL-3 (Major)

---

## 1. System Overview

### 1.1 Purpose
The ASTERIX Decoder is a software component for decoding and parsing EUROCONTROL ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) protocol data used in CNS/ATM systems.

### 1.2 Application Domain
- **Type:** Ground-based CNS/ATM System Software
- **Function:** Surveillance data decoding and parsing
- **Standard:** DO-278A / ED-109
- **Assurance Level:** AL-3

### 1.3 Assurance Level Justification

**AL-3 (Major)** was selected because:
- Software processes critical surveillance data for Air Traffic Management
- Incorrect parsing could lead to erroneous track information
- Failure could impair controller efficiency and situational awareness
- System operates within ground-based ATM infrastructure with redundancy
- Errors are within operational capabilities to detect and manage

**Safety Impact:** Malfunction could cause incorrect surveillance data presentation, potentially leading to reduced safety margins in air traffic control operations.

---

## 2. Software Overview

### 2.1 Software Identification
- **Name:** ASTERIX Decoder
- **Version:** TBD (tracking via git tags)
- **Repository:** https://github.com/montge/asterix (fork of CroatiaControlLtd/asterix)
- **Languages:** C++ (core), Python (bindings), C (interface)
- **Build System:** Make, CMake, Python setuptools

### 2.2 Functionality
- Parse ASTERIX binary protocol data (Categories 001-252)
- Support multiple input formats (Raw, PCAP, HDLC, FINAL, GPS)
- Support multiple output formats (Text, JSON, XML)
- Validate data integrity (CRC checks)
- Handle network multicast streams
- Provide Python API for integration

### 2.3 Software Components
1. **Core Parser Engine** (`src/asterix/`) - ASTERIX protocol implementation
2. **I/O Engine** (`src/engine/`) - Device and format abstraction
3. **CLI Application** (`src/main/`) - Command-line interface
4. **Python Bindings** (`src/python/`) - Python C extension
5. **Configuration** (`asterix/config/`, `install/config/`) - XML category definitions

---

## 3. Software Life Cycle Data

### 3.1 Planning Data
- Software Development Plan (SDP)
- Software Verification Plan (SVP)
- Software Configuration Management Plan (SCMP)
- Software Quality Assurance Plan (SQAP)

### 3.2 Development Data
- Software Requirements Standards
- Software Design Standards
- Software Code Standards
- Software Requirements Data (SRD)
- Software Design Description (SDD)
- Source Code
- Executable Object Code

### 3.3 Verification Data
- Software Verification Cases and Procedures (SVCP)
- Software Verification Results (SVR)
- Test Coverage Analysis
- Requirements Traceability Matrix

### 3.4 Configuration Management Data
- Software Configuration Index (SCI)
- Problem Reports
- Software Configuration Management Records
- Archive/Retrieval records

### 3.5 Quality Assurance Data
- Software Quality Assurance Records
- Software Conformity Review results
- Software Life Cycle Environment Configuration Index

---

## 4. Compliance Statement

This Software Accomplishment Summary documents the software life cycle processes and compliance with DO-278A for Assurance Level AL-3.

### 4.1 DO-278A Objectives Compliance

For AL-3, the following objectives are addressed:

| Objective | Description | Status |
|-----------|-------------|--------|
| 1.1 | High-level requirements | In Progress |
| 1.2 | Derived requirements | In Progress |
| 1.3 | Software architecture | Planned |
| 1.4 | Low-level requirements | Planned |
| 2.1 | Source code | Exists |
| 2.2 | Executable object code | Exists |
| 3.1 | Review of requirements | Planned |
| 3.2 | Review of design | Planned |
| 3.3 | Review of code | Planned |
| 3.4 | Review of integration | Planned |
| 4.1 | Test coverage | Planned |
| 4.2 | Structural coverage | Planned |

### 4.2 Certification Approach

**Test-Driven Development (TDD) Approach:**
1. Establish baseline with existing code
2. Document requirements from ASTERIX specifications
3. Create verification test cases
4. Measure and improve test coverage
5. Perform structural coverage analysis
6. Document traceability

---

## 5. Tool Qualification

### 5.1 Development Tools
- **Compiler:** GCC (system standard compiler)
- **Build System:** GNU Make, CMake
- **Version Control:** Git
- **CI/CD:** GitHub Actions

### 5.2 Verification Tools
- **Unit Testing:** Python unittest, C++ test framework (TBD)
- **Coverage Analysis:** gcov/lcov (planned)
- **Static Analysis:** cppcheck, clang-tidy (planned)
- **Memory Analysis:** Valgrind (existing)

---

## 6. Known Deviations and Issues

### 6.1 Current State
This is a retrofit compliance effort for existing open-source software. The following activities are in progress:

- Requirements documentation (from ASTERIX specifications)
- Formal design documentation
- Test case development and coverage improvement
- Traceability establishment

### 6.2 Legacy Code Considerations
- Core parsing logic exists and is operational
- Test infrastructure exists (install/test/)
- Configuration is based on EUROCONTROL ASTERIX specifications
- Upstream maintenance via CroatiaControlLtd/asterix

---

## 7. References

### 7.1 Standards
- DO-278A: Guidelines for Communication, Navigation, Surveillance and Air Traffic Management (CNS/ATM) Systems Software Integrity Assurance
- ED-109: Same as DO-278A (EUROCAE)
- EUROCONTROL ASTERIX Specifications

### 7.2 Project Documents
- Software Development Plan (SDP-ASTERIX-001)
- Software Verification Plan (SVP-ASTERIX-001)
- Software Configuration Management Plan (SCMP-ASTERIX-001)
- Software Quality Assurance Plan (SQAP-ASTERIX-001)

---

## Document Approval

| Role | Name | Signature | Date |
|------|------|-----------|------|
| Project Lead | TBD | | |
| QA Manager | TBD | | |
| Certification Authority | TBD | | |

---

**Document History**

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-10-17 | Initial | Initial SAS creation |
