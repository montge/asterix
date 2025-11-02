# C++ Coverage Improvement - Quick Reference
## Fast Reference Guide for 90% Coverage Plan

**Last Updated:** 2025-11-01
**Full Plan:** See `CPP_COVERAGE_IMPROVEMENT_PLAN.md`

---

## Current Status at a Glance

| Metric | Value | Target | Gap |
|--------|-------|--------|-----|
| **Overall (ASTERIX layer)** | 77.7% | 90% | +12.3 pts |
| **Overall (PROJECT)** | ~20% | 90% | +70 pts |
| **Modules ≥80%** | 3/20 | 20/20 | 17 modules |
| **Days to 90%** | - | 20-29 days | - |

---

## Quick Wins (Week 1)

Execute these for fastest path to 80% overall:

| Quick Win | Module | Current | Target | Gain | Days |
|-----------|--------|---------|--------|------|------|
| #1 | Verify agent work | - | - | 0% | 1 |
| #2 | Category.cpp | 38.7% | 80% | +1.6% | 1-2 |
| #3 | DataBlock.cpp | 63.0% | 85% | +0.5% | 1 |
| #4 | UAPItem.cpp | 26.3% | 80% | +0.4% | 1 |
| #5 | AsterixDefinition.cpp | 46.7% | 80% | +0.6% | 1 |

**Total:** 5-6 days → 80.8% coverage ✅

---

## Module Priority List

### Priority 1: CRITICAL (Must fix)
1. **DataItemBits.cpp** - 77.9% → 85% (+1.7%) - 2 days
2. **XMLParser.cpp** - 67.8% → 85% (+3.3%) - 3 days
3. **Category.cpp** - 38.7% → 80% (+1.6%) - 2 days
4. **DataRecord.cpp** - 62.9% → 85% (+1.4%) - 2 days

### Priority 2: HIGH
5. **DataItemFormatFixed.cpp** - 51.1% → 85% (+1.9%) - 2 days
6. **DataItemFormatCompound.cpp** - 51.1%* → 85% (+2.1%) - 2 days
7. **InputParser.cpp** - 56.5% → 85% (+0.9%) - 1 day

*May already be at 98.62% - verify first

### Priority 3: MODERATE
8. **AsterixDefinition.cpp** - 46.7% → 80% (+0.6%) - 1 day
9. **DataItem.cpp** - 48.5% → 80% (+0.9%) - 2 days
10. **DataBlock.cpp** - 63.0% → 85% (+0.5%) - 1 day
11. **UAPItem.cpp** - 26.3% → 80% (+0.4%) - 1 day

### Priority 4: ENGINE LAYER
12. **converterengine.cxx** - 0% → 70% (+4.9%) - 4 days
13. **udpdevice.cxx** - 0% → 75% (+2.6%) - 3 days
14. **tcpdevice.cxx** - 0% → 70% (+1.8%) - 2 days
15. **devicefactory.cxx** - 0% → 80% (+1.4%) - 1 day
16. Other engine files - 0% → 60% (+7.7%) - 5 days

---

## Phase Timeline

```
Week 1: Quick Wins → 80% ✅
├── Verify agent work (1 day)
└── 4 small files (4-5 days)

Week 2: High-Impact → 86% ✅
├── XMLParser (3 days)
├── DataItemBits (2 days)
└── Others (2 days)

Week 3: Completion → 88% ✅
├── Remaining formats (3 days)
└── Integration tests (4 days)

Week 4-5: Engine → 90%+ ✅
├── Core engine (4 days)
├── Devices (6 days)
└── Polish (3 days)
```

---

## Agent Deployment Strategy

### Wave 1: Parallel Quick Wins (Days 2-3)
- **Agent A:** Category.cpp → 80%
- **Agent B:** AsterixDefinition.cpp → 80%
- **Agent C:** UAPItem.cpp → 80%
- **Expected:** +2.6% overall

### Wave 2: High-Impact (Days 4-7)
- **Agent D:** XMLParser.cpp → 85%
- **Agent E:** DataItemBits.cpp → 85%
- **Expected:** +5.0% overall

### Wave 3: Format Completion (Days 8-12)
- **Agent F:** DataItemFormatFixed → 85%
- **Agent G:** InputParser → 85%
- **Expected:** +2.8% overall

### Wave 4: Engine Layer (Days 20-29)
- **Agents H-K:** Engine components
- **Expected:** +70% overall (project-wide)

**Total Agents:** 11-15 autonomous agents
**Success Rate:** 100% (proven track record)

---

## Commands Reference

### Build and Test
```bash
# Build project
cd /tmp/asterix-restored
cmake .
make -j4

# Run all tests
ctest -j4

# Run specific test
./bin/test_{module}
```

### Coverage Analysis
```bash
# Generate coverage
lcov --capture --directory . --output-file coverage.info

# Filter coverage
lcov --remove coverage.info '/usr/*' '*/_deps/*' '*/test_*' -o filtered.info

# Generate HTML report
genhtml filtered.info --output-directory coverage_html

# View summary
lcov --list filtered.info

# View specific file
gcov src/asterix/{Module}.cpp
```

### Verify Agent Work
```bash
# Check current coverage state
cd /tmp/asterix-restored
cmake --build . --target all_tests
ctest
lcov --capture --directory . -o coverage_current.info
lcov --remove coverage_current.info '/usr/*' '*/_deps/*' '*/test_*' -o coverage_filtered.info
lcov --list coverage_filtered.info | grep -E "\.cpp|\.cxx" | grep -v test_
```

---

## Test Template Quick Start

### Create New Test File
```bash
cd /tmp/asterix-restored/tests/cpp
cp test_category.cpp test_{newmodule}.cpp
# Edit test_{newmodule}.cpp
```

### Add to CMakeLists.txt
```cmake
add_executable(test_{newmodule} test_{newmodule}.cpp)
target_link_libraries(test_{newmodule}
    GTest::gtest_main
    asterix_static
    ${EXPAT_LIBRARIES})
target_compile_options(test_{newmodule} PRIVATE --coverage)
target_link_options(test_{newmodule} PRIVATE --coverage)
gtest_discover_tests(test_{newmodule})
```

### Build and Run
```bash
cd /tmp/asterix-restored
cmake --build . --target test_{newmodule}
./bin/test_{newmodule} --gtest_color=yes
```

---

## Coverage Targets by Phase

| Phase | Days | Target | Modules Fixed |
|-------|------|--------|---------------|
| **Quick Wins** | 5-7 | 80% | 4 modules |
| **Phase 1** | 7 total | 82% | 7 modules |
| **Phase 2** | 12 total | 86% | 11 modules |
| **Phase 3** | 19 total | 88% | 15 modules |
| **Phase 4** | 29 total | 90%+ | All modules |

---

## Success Criteria Checklist

### Week 1 (80% Target)
- [ ] Overall coverage ≥ 80%
- [ ] 4+ modules at 80%+
- [ ] All tests passing
- [ ] Zero memory leaks
- [ ] Coverage report generated

### Week 2 (86% Target)
- [ ] Overall coverage ≥ 86%
- [ ] All DataItemFormat* at 85%+
- [ ] XMLParser at 85%+
- [ ] All tests passing
- [ ] Documentation updated

### Week 3 (88% Target)
- [ ] Overall coverage ≥ 88%
- [ ] All ASTERIX modules at 80%+
- [ ] Integration tests complete
- [ ] Error paths tested
- [ ] All tests passing

### Week 5 (90% Target)
- [ ] Overall PROJECT coverage ≥ 90%
- [ ] Engine layer ≥ 65%
- [ ] All modules ≥ 80%
- [ ] DO-278A compliant
- [ ] Final report complete

---

## Risk Mitigation Quick List

| Risk | Mitigation |
|------|------------|
| Agent work lost | Verify first (Quick Win #1) |
| Engine complexity | Use mocks, accept 65% target |
| Time overrun | Front-load quick wins, use parallel agents |
| Coverage gaps | Use HTML reports for precision |
| Test maintenance | Follow templates, consistent naming |

---

## Key Files Reference

| File | Purpose |
|------|---------|
| `CPP_COVERAGE_IMPROVEMENT_PLAN.md` | Complete detailed plan |
| `SESSION_SUMMARY_77_PERCENT.md` | Latest coverage snapshot |
| `COVERAGE_ANALYSIS_PHASE2.md` | Module-by-module breakdown |
| `tests/cpp/CMakeLists.txt` | Test build configuration |
| `tests/cpp/test_*.cpp` | Test implementations |

---

## Agent Prompt Template

```
Create comprehensive unit tests for {MODULE}.cpp to achieve {TARGET}% coverage.

Current: {CURRENT}% ({LINES_COVERED}/{TOTAL_LINES} lines)
Target: {TARGET}% ({TARGET_LINES}/{TOTAL_LINES} lines)

Test requirements:
1. {Category 1} ({N} tests)
2. {Category 2} ({N} tests)
3. {Category 3} ({N} tests)
4. All 7 output formats (7 tests)
5. Edge cases and error handling ({N} tests)

Build with:
cd /tmp/asterix-restored
cmake --build . --target test_{module}
./bin/test_{module}

Measure coverage with:
lcov --capture --directory . -o coverage.info
lcov --list coverage.info | grep {MODULE}.cpp

Report: File coverage, overall project impact, test count, all passing.
```

---

## Expected Outcomes

### Week 1
- **Coverage:** 77.7% → 80.8%
- **Tests:** 270 → 300
- **Modules Fixed:** 4
- **Status:** Quick wins complete ✅

### Week 2
- **Coverage:** 80.8% → 86%
- **Tests:** 300 → 360
- **Modules Fixed:** 7 more
- **Status:** Critical modules complete ✅

### Week 3
- **Coverage:** 86% → 88%
- **Tests:** 360 → 400
- **Modules Fixed:** 4 more
- **Status:** ASTERIX layer complete ✅

### Week 5
- **Coverage:** 88% → 90%+
- **Tests:** 400 → 500
- **Modules Fixed:** All
- **Status:** DO-278A compliant ✅

---

## Quick Decision Tree

**Need to reach 80% fast?**
→ Execute Quick Wins (5-7 days)

**Need per-module compliance?**
→ Follow Phase 1-3 (3 weeks)

**Need full DO-278A compliance?**
→ Execute all 4 phases (5 weeks)

**Need to verify current state?**
→ Run Quick Win #1 (1 day)

**Need to test specific module?**
→ Use test template, deploy agent

---

## Contact / Questions

**Plan Author:** DO-278A Compliance Team
**Document:** `CPP_COVERAGE_IMPROVEMENT_PLAN.md`
**Quick Ref:** This document
**Status:** Ready for execution

🚀 **Start with Quick Win #1: Coverage Verification (1 day)**

---

**Document Version:** 1.0
**Last Updated:** 2025-11-01
**Next Review:** After Week 1 completion
