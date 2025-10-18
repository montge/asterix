# DO-278A Compliance Documentation

This directory contains DO-278A (CNS/ATM Systems Software Integrity Assurance) compliance documentation for the ASTERIX Decoder project.

## Assurance Level: AL-3 (Major)

## Directory Structure

```
do-278/
├── README.md                           (this file)
├── plans/                              Software life cycle plans
│   ├── Software_Accomplishment_Summary.md
│   ├── Software_Verification_Plan.md
│   ├── Software_Development_Plan.md
│   ├── Software_Configuration_Management_Plan.md
│   └── Software_Quality_Assurance_Plan.md
├── requirements/                       Requirements data
│   ├── High_Level_Requirements.md
│   ├── Low_Level_Requirements.md
│   └── Requirements_Traceability_Matrix.md
├── design/                            Design data
│   ├── Software_Design_Description.md
│   └── Interface_Control_Documents/
├── verification/                      Verification data
│   ├── Test_Cases/
│   ├── Test_Procedures/
│   ├── Test_Results/
│   └── Coverage_Reports/
├── configuration-management/          CM records
│   ├── Software_Configuration_Index.md
│   └── Problem_Reports/
└── quality-assurance/                QA records
    └── Review_Records/
```

## Quick Start

### 1. Build the Project

```bash
# C++ executable
cd src
make
make install
make test

# Python module
python setup.py build
python setup.py install
python -m unittest
```

### 2. Run Verification

```bash
# CI pipeline (GitHub Actions)
git push  # Triggers CI automatically

# Local testing
cd install/test
./test.sh

# Memory check
./valgrind_test.sh
```

### 3. Coverage Analysis

```bash
# Python coverage
python -m pytest --cov=asterix --cov-report=html asterix/test/

# C++ coverage (TODO: needs instrumentation)
# See do-278/verification/ for reports
```

## Coverage Targets

- **Overall Project:** 90-95%
- **Per Module:** ≥80%
- **Critical Modules:** ≥90%

## Key Documents

### Planning
- **SAS** - Software Accomplishment Summary - Overview of compliance
- **SDP** - Software Development Plan - Development processes
- **SVP** - Software Verification Plan - Test strategy and coverage targets
- **SCMP** - Software Configuration Management Plan - Version control and CM
- **SQAP** - Software Quality Assurance Plan - QA processes

### Development
- **SRD** - Software Requirements Data - High-level and low-level requirements
- **SDD** - Software Design Description - Architecture and design
- **Source Code** - src/ directory
- **Build Scripts** - Makefiles, CMakeLists.txt, setup.py

### Verification
- **SVCP** - Software Verification Cases and Procedures - Test cases
- **SVR** - Software Verification Results - Test results and coverage
- **RTM** - Requirements Traceability Matrix - Bidirectional traceability

### Configuration Management
- **SCI** - Software Configuration Index - Baseline identification
- **SCMRs** - Software Configuration Management Records - Git history
- **PRs** - Problem Reports - GitHub issues

## DO-278A AL-3 Requirements Summary

For Assurance Level 3 (Major), the following are required:

### Development
- [x] High-level requirements defined
- [ ] Low-level requirements defined
- [ ] Software architecture documented
- [x] Source code developed
- [x] Executable object code built

### Verification
- [ ] Requirements reviewed
- [ ] Design reviewed
- [ ] Code reviewed
- [ ] Requirements-based testing
- [ ] Statement coverage analysis (≥95% target)
- [ ] Decision coverage analysis (≥95% target)

### Configuration Management
- [x] Version control (Git)
- [x] Build process (Make/CMake)
- [ ] Baseline identification
- [ ] Change tracking

### Quality Assurance
- [ ] Process compliance checks
- [ ] Review participation
- [ ] Records maintenance

## Test-Driven Development Approach

We are using a **retrofit TDD approach** for this existing codebase:

### Phase 1: Baseline ✓
- Build existing code
- Run existing tests
- Document current state

### Phase 2: Requirements (In Progress)
- Extract requirements from ASTERIX specs
- Document existing behavior
- Create traceability matrix

### Phase 3: Test Development (Next)
- Write test cases for requirements
- Achieve module coverage targets (≥80%)
- Refactor for testability

### Phase 4: Coverage Achievement
- Measure coverage
- Add tests for gaps
- Reach 90-95% overall coverage

### Phase 5: Documentation
- Complete all verification records
- Conduct reviews
- Archive results

## Continuous Integration

GitHub Actions workflows:
- `.github/workflows/ci-verification.yml` - Main verification CI
- `.github/workflows/codeql-analysis.yml` - Security scanning
- `.github/workflows/python-publish.yml` - PyPI publishing

## Contributing

When making changes:

1. **Create requirement** - Document what you're adding in `do-278/requirements/`
2. **Write tests first** - Follow TDD approach
3. **Implement** - Write the code
4. **Verify coverage** - Ensure targets met
5. **Update traceability** - Link requirement → design → code → test
6. **Submit PR** - CI will verify

## References

- DO-278A: Guidelines for CNS/ATM Systems Software Integrity Assurance
- EUROCONTROL ASTERIX Specifications: http://www.eurocontrol.int/services/asterix
- Project repository: https://github.com/montge/asterix
- Upstream: https://github.com/CroatiaControlLtd/asterix

## Contact

For DO-278 compliance questions, see the Software Accomplishment Summary or contact the project lead.

## Status

**Current Phase:** Requirements Documentation
**Overall Coverage:** TBD (baseline measurement pending)
**Target Date:** TBD

Last updated: 2025-10-17
