# ASTERIX Decoder - Master Roadmap to DO-278A Certification
## Single Source of Truth for Project Planning

**Last Updated:** 2025-10-18
**Current Status:** Phase 1 Complete - Integration Testing Started
**Overall Progress:** 15% → Target: 90-95%

---

## 📊 Current Reality (Measured 2025-10-18)

### Coverage Status
| Component | Coverage | Tests | Status |
|-----------|----------|-------|--------|
| **Python** | 88% | 60 | ✅ Nearly complete |
| **C++** | **45.5%** | 38 | 🟡 In progress |
| **Overall** | **~15%** | 98 | 🔴 Needs major work |
| **Target** | 90-95% | TBD | 🎯 Goal |

### Key Files Tested (20/22 C++ files)
- ✅ XMLParser, InputParser, AsterixData, DataBlock, DataRecord (NEW!)
- ✅ All 5 data formats: Fixed, Variable, Compound, Repetitive, BDS
- ⚠️ Still need: Error paths, edge cases, more categories

### Critical Discovery
**One integration test = +14 percentage points!**
Testing the complete parsing pipeline is 10x more effective than isolated unit tests.

---

## 🎯 Strategic Vision

### Phase Goals (Prioritized for Success)
1. **Phase 1:** Reach 50% coverage (baseline competence) - **IN PROGRESS**
2. **Phase 2:** Reach 80% coverage (production ready) - 2-3 weeks
3. **Phase 3:** Standards integration (EUROCONTROL compliance) - 3-4 weeks
4. **Phase 4:** 90-95% coverage (certification ready) - 2-3 weeks
5. **Phase 5:** Formal verification (proof of correctness) - 3-4 weeks
6. **Phase 6:** DO-278A documentation & review - 1-2 weeks

**Total Timeline:** 12-16 weeks from now

---

## 📅 PHASE 1: Foundation (CURRENT - Week 1-2)

### Goal: 50% C++ Coverage
**Current:** 45.5% | **Target:** 50% | **Gap:** +4.5 points

### Tasks
- [x] Create first CAT048 integration test (DONE - gave us +14 points!)
- [ ] Add CAT062 integration test
- [ ] Add CAT065 integration test
- [ ] Run coverage analysis and validate 50%+

### Success Criteria
- ✅ 50% C++ coverage measured with lcov
- ✅ All 3 sample categories tested end-to-end
- ✅ 100% test pass rate maintained

### Deliverables
- CAT062 integration test (5 test cases)
- CAT065 integration test (5 test cases)
- Coverage report showing 50%+

**Estimated Completion:** End of Week 2

---

## 📅 PHASE 2: Core Coverage (Weeks 3-5)

### Goal: 80% C++ Coverage (Production Ready)
**Current:** 45.5% | **Target:** 80% | **Gap:** +34.5 points

### Strategy
Focus on high-value integration tests that exercise multiple components:

#### Week 3: More Categories
- CAT001, CAT002, CAT004 integration tests
- CAT010, CAT011, CAT019 integration tests
- Target: +10-15 points

#### Week 4: All Input Formats
- Test PCAP format parsing
- Test HDLC format parsing
- Test FINAL format parsing
- Test GPS format parsing
- Target: +10-15 points

#### Week 5: Error Handling & Edge Cases
- Invalid data handling
- Truncated packets
- CRC failures
- Malformed XML configs
- Target: +5-10 points

### Success Criteria
- ✅ 80% C++ coverage measured
- ✅ All 24 supported ASTERIX categories tested
- ✅ All input format types tested
- ✅ Error paths covered

### Deliverables
- 15-20 additional integration tests
- Coverage report showing 80%+
- Error handling test suite

**Estimated Completion:** End of Week 5

---

## 📅 PHASE 3: Standards Integration (Weeks 6-9)

### Goal: EUROCONTROL Official Specification Compliance

This is YOUR vision - critical for DO-278A traceability!

#### Week 6: Specification Acquisition & Analysis
**Objective:** Obtain and parse official EUROCONTROL documentation

##### Tasks
1. **Acquire Official Specs:**
   - EUROCONTROL ASTERIX website downloads
   - ED-117 ASTERIX Application Handbook
   - Category-specific specification PDFs
   - Current XML definitions (from zoranbosnjak/asterix-specs)

2. **Evaluate Parsing Tools:**
   - Test docling for PDF parsing
   - Test pdfplumber as alternative
   - Test pymupdf for structure extraction
   - Select best tool for EUROCONTROL PDFs

3. **Create Specification Database:**
   - Extract data item definitions
   - Extract UAP (User Application Profile) structures
   - Extract value meanings and valid ranges
   - Store in structured format (JSON/SQLite)

**Deliverables:**
- EUROCONTROL specification repository (local copy)
- Specification parsing tool evaluation report
- Structured specification database

#### Week 7: Test Oracle Generation
**Objective:** Convert specs into test expectations

##### Tasks
1. **Extract Test Vectors:**
   - Example data from EUROCONTROL specs
   - Expected parsing results
   - Valid value ranges
   - Invalid value examples

2. **Generate Test Data:**
   - Create binary ASTERIX samples from spec examples
   - Generate edge case test data (min/max values)
   - Create error case test data (invalid formats)

3. **Build Test Oracle:**
   - Spec → Expected output mapping
   - Automated test case generator
   - Verification oracle for assertions

**Deliverables:**
- Test data generation tool
- Test oracle database
- 50+ spec-based test cases

#### Week 8-9: Compliance Testing & Traceability
**Objective:** Prove implementation matches specification

##### Tasks
1. **Run Compliance Tests:**
   - Execute all spec-based tests
   - Compare implementation vs. spec
   - Document any deviations
   - Fix implementation bugs found

2. **Create Traceability Matrix:**
   - Spec section → Requirement → Code → Test
   - Every data item traceable to spec
   - Every test case linked to spec section

3. **Generate Compliance Report:**
   - Percentage compliance per category
   - List of deviations with justifications
   - Coverage of spec requirements

**Deliverables:**
- EUROCONTROL compliance test suite (100+ tests)
- Requirements Traceability Matrix (spec → code → test)
- Compliance report

### Success Criteria
- ✅ All data items traced to EUROCONTROL specs
- ✅ Test oracles generated from official specs
- ✅ Compliance >95% with documented deviations
- ✅ Full bidirectional traceability

**Estimated Completion:** End of Week 9

---

## 📅 PHASE 4: Coverage Finalization (Weeks 10-12)

### Goal: 90-95% Coverage (DO-278A Certification Ready)
**Current:** 80% (projected) | **Target:** 90-95% | **Gap:** +10-15 points

### Strategy
Systematic gap closure using lcov reports:

#### Week 10: Gap Analysis
1. Generate comprehensive coverage report
2. Identify all uncovered lines
3. Categorize gaps:
   - Dead code (document as untestable)
   - Error paths (add error injection tests)
   - Edge cases (add boundary tests)
   - Complex algorithms (add test vectors)

#### Week 11: Targeted Test Development
1. Add tests for highest-priority gaps
2. Focus on safety-critical modules (>95%)
3. Document untestable code with justification

#### Week 12: Final Validation
1. Re-measure coverage
2. Verify all targets met
3. Final test suite cleanup
4. Performance testing

### Success Criteria
- ✅ 90-95% overall coverage
- ✅ ≥80% per module
- ✅ ≥95% for critical modules (parser core)
- ✅ All gaps documented

### Deliverables
- Final coverage report (90-95%)
- Untestable code justification document
- Performance test results

**Estimated Completion:** End of Week 12

---

## 📅 PHASE 5: Formal Verification (Weeks 13-16)

### Goal: Mathematical Proof of Correctness

This is YOUR vision - provides certification confidence!

#### Week 13: Formal Specification
**Objective:** Create formal model of ASTERIX protocol

##### Tasks
1. **Select Formal Method:**
   - Evaluate: TLA+, Z notation, Alloy, or property-based testing
   - Choose based on tool support and team expertise
   - Recommendation: Start with property-based testing (hypothesis)

2. **Define Protocol Properties:**
   - Parsing invariants (e.g., "output size ≤ input size")
   - Data integrity properties (e.g., "CRC validation detects corruption")
   - Format properties (e.g., "UAP determines field order")

3. **Model Core Algorithms:**
   - Data item parsing rules
   - UAP processing
   - CRC calculation

**Deliverables:**
- Formal specification document (TLA+/Z notation)
- Property definitions for property-based testing

#### Week 14-15: Property-Based Testing
**Objective:** Prove properties hold for all inputs

##### Tasks
1. **Implement Property Tests:**
   ```python
   from hypothesis import given, strategies as st

   @given(st.binary(min_size=1, max_size=1000))
   def test_parse_never_crashes(data):
       """Property: Parser never crashes on any input"""
       try:
           asterix.parse(data)
       except ValueError:
           pass  # Expected for invalid data
       except Exception as e:
           assert False, f"Unexpected exception: {e}"

   @given(st.binary(min_size=1, max_size=1000))
   def test_parse_idempotent(data):
       """Property: Parsing is deterministic"""
       result1 = asterix.parse(data)
       result2 = asterix.parse(data)
       assert result1 == result2
   ```

2. **Verify Critical Properties:**
   - No buffer overflows (output bounded by input)
   - Data integrity (CRC catches all single-bit errors)
   - Parsing correctness (round-trip where applicable)

3. **Generate Counter-Examples:**
   - Use property-based testing to find edge cases
   - Document interesting failures
   - Add to regression test suite

**Deliverables:**
- Property-based test suite (hypothesis/QuickCheck)
- Counter-example database
- Proof certificates for verified properties

#### Week 16: Formal Verification Report
**Objective:** Document formal verification results

##### Tasks
1. **Document Verified Properties:**
   - List all proved properties
   - Show proof evidence (test runs, formal proofs)
   - Document assumptions and limitations

2. **Create Assurance Argument:**
   - Informal spec (EUROCONTROL) → Formal spec
   - Formal spec → Implementation
   - Implementation → Tests
   - Tests prove spec compliance

3. **Update DO-278A Documentation:**
   - Add formal verification section
   - Link to traceability matrix
   - Provide certification evidence

**Deliverables:**
- Formal verification report
- Assurance argument document
- DO-278A addendum

### Success Criteria
- ✅ Critical properties formally specified
- ✅ Properties verified via testing or proof
- ✅ Formal verification integrated into CI/CD
- ✅ Certification evidence package complete

**Estimated Completion:** End of Week 16

---

## 📅 PHASE 6: Certification Package (Weeks 17-18)

### Goal: Complete DO-278A AL-3 Documentation

### Week 17: Documentation Completion
1. Update all DO-278A plans with actual results
2. Complete Requirements Traceability Matrix
3. Generate final test reports
4. Document all reviews conducted
5. Create Software Configuration Index

### Week 18: Final Review & Submission
1. Internal review of all documents
2. Address review findings
3. Package all deliverables
4. Submit for certification audit

### Success Criteria
- ✅ All DO-278A documents complete
- ✅ All reviews passed
- ✅ Baseline identified (git tag)
- ✅ Ready for auditor review

**Estimated Completion:** End of Week 18

---

## 📋 Key Deliverables Summary

### Phase 1 (Weeks 1-2)
- [ ] 50% C++ coverage
- [ ] CAT048/062/065 integration tests

### Phase 2 (Weeks 3-5)
- [ ] 80% C++ coverage
- [ ] All 24 categories tested
- [ ] Error handling test suite

### Phase 3 (Weeks 6-9)
- [ ] EUROCONTROL spec database
- [ ] Spec-based test oracle
- [ ] Compliance test suite
- [ ] Requirements Traceability Matrix

### Phase 4 (Weeks 10-12)
- [ ] 90-95% coverage
- [ ] Final coverage report
- [ ] Performance test results

### Phase 5 (Weeks 13-16)
- [ ] Formal specification
- [ ] Property-based tests
- [ ] Formal verification report

### Phase 6 (Weeks 17-18)
- [ ] Complete DO-278A package
- [ ] All reviews complete
- [ ] Ready for certification

---

## 🎯 Success Metrics

### Technical Metrics
- **Coverage:** 90-95% overall, ≥80% per module
- **Test Pass Rate:** 100%
- **Performance:** No regression vs. baseline
- **Memory:** Zero leaks (Valgrind clean)

### Process Metrics
- **Traceability:** 100% bidirectional (req ↔ test)
- **Reviews:** 100% of plans, requirements, design, code, tests
- **CI/CD:** All checks passing on main branch

### Quality Metrics
- **Bug Density:** <0.1 bugs per KLOC
- **Compliance:** >95% with EUROCONTROL specs
- **Formal Verification:** >90% of critical properties proved

---

## 🚀 Next Actions (This Week)

### Immediate Tasks (Next 2-3 Days)
1. ✅ Create MASTER_ROADMAP.md (this document) - consolidate all plans
2. [ ] Add CAT062 integration test
3. [ ] Add CAT065 integration test
4. [ ] Validate 50% coverage achieved
5. [ ] Update DO-278 documentation with actual 45.5% baseline

### This Week
- Complete Phase 1 (50% coverage)
- Begin Phase 2 planning
- Archive outdated planning documents

---

## 📚 Document Consolidation

### This Document Replaces
- ~~COVERAGE_PROGRESS.md~~ (outdated estimates)
- ~~DO-278_IMPLEMENTATION_GUIDE.md~~ (consolidated here)
- ~~PHASE_1_COMPLETE.md~~ (archived)
- ~~PHASE_2_COMPLETE.md~~ (archived)
- ~~PROGRESS_SUMMARY.md~~ (consolidated here)
- ~~TODAY_ACCOMPLISHMENTS.md~~ (use git log)

### Keep These Documents
- ✅ SESSION_SUMMARY_2025-10-18.md (historical record)
- ✅ COVERAGE_REALITY_CHECK.md (critical findings)
- ✅ INSTALLATION_GUIDE.md (setup instructions)
- ✅ do-278/plans/*.md (formal DO-278A documents)
- ✅ CLAUDE.md (project instructions)

### Archive Location
Old planning docs moved to: `docs/archive/planning/`

---

## 🔄 Maintenance

This roadmap will be updated:
- Weekly during active development
- After each phase completion
- When major changes occur

**Owner:** Project Lead
**Reviews:** Weekly team sync

---

## 📞 Questions?

See:
- Technical details: `CLAUDE.md`
- Setup instructions: `INSTALLATION_GUIDE.md`
- DO-278A compliance: `do-278/README.md`
- Latest status: This document (MASTER_ROADMAP.md)

---

**Status:** Phase 1 In Progress (45.5% coverage → Target 50%)
**Next Milestone:** CAT062 integration test
**Timeline:** On track for Week 18 completion

*Single source of truth - all other planning docs are outdated or archived*
