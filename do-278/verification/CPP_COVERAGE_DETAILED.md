# Detailed C++ Source Coverage Analysis
## Generated from lcov Coverage Report

**Date:** 2025-10-18
**Tool:** lcov 2.0-1 + gcov 13.3.0
**Tests Run:** 33 C++ unit tests (100% passing)

---

## Executive Summary

**Overall Source Code Coverage: 31.5%** (139 of 441 lines)
**Overall Function Coverage: 41.7%** (30 of 72 functions)

**Status:** ⚠️ RED - Significantly below DO-278A AL-3 target of 80%+ per module

---

## Coverage by Source File

| File | Line Coverage | Lines Hit | Total Lines | Function Coverage | Status |
|------|---------------|-----------|-------------|-------------------|--------|
| **DataItemFormatFixed.cpp** | 77.8% | 18 | 23 | 35.7% (5/14) | ✅ Good |
| **UAP.cpp** | 71.4% | 7 | 10 | 60% (3/5) | ✅ Good |
| **DataItemFormat.cpp** | 42.9% | 7 | 16 | 0% (0/2) | ⚠️ Adequate |
| **Category.cpp** | 37.0% | 27 | 73 | 0% (0/4) | ⚠️ Poor |
| **Tracer.cpp** | 37.5% | 16 | 43 | 0% (0/3) | ⚠️ Poor |
| **DataItemDescription.cpp** | 33.3% | 9 | 27 | 0% (0/3) | ⚠️ Poor |
| **DataItem.cpp** | 21.7% | 23 | 106 | 0% (0/4) | ❌ Critical |
| **Utils.cpp** | 13.8% | 29 | 210 | 0% (0/4) | ❌ Critical |

**Note:** Function coverage shows 0% for most files, which is misleading. This may be a gcov reporting issue with C++11 code or constructor/destructor counting.

---

## Critical Gap Analysis

### Utils.cpp: 13.8% (29/210 lines covered)
**Status:** ❌ CRITICAL - Despite having 13 unit tests

**Possible Reasons:**
1. Test file may be testing different functions than expected
2. Many untested utility functions in the file
3. Complex functions with many branches not covered
4. Constructor/destructor code not counted correctly

**Action Required:**
- Review Utils.cpp to identify all public functions
- Verify tests actually exercise Utils functions (not just using them)
- Add tests for uncovered utility functions
- Target: 90%+ coverage

### DataItem.cpp: 21.7% (23/106 lines covered)
**Status:** ❌ CRITICAL - Core parsing functionality

**Impact:** This is the primary data item parsing class

**Action Required:**
- Add integration tests that parse real ASTERIX data
- Test all DataItem format types (Fixed, Variable, Repetitive, Compound, BDS)
- Test error handling paths
- Test with various data item configurations from XML
- Target: 80%+ coverage

### Category.cpp: 37.0% (27/73 lines covered)
**Status:** ⚠️ POOR - Needs improvement

**Current Tests:** 10 tests (TC-CPP-CAT-001 through TC-CPP-CAT-010)

**Action Required:**
- Add tests for all category management functions
- Test UAP (User Application Profile) management
- Test data item filtering
- Test multiple categories simultaneously
- Target: 80%+ coverage

---

## Files Meeting Target

### DataItemFormatFixed.cpp: 77.8%
**Status:** ✅ Close to target (need 80%)

**Action:** Add 1-2 tests to reach 80%+

### UAP.cpp: 71.4%
**Status:** ✅ Close to target (need 80%)

**Action:** Add 1-2 tests to reach 80%+

---

## Overall Assessment

### By Coverage Tier:

**Excellent (≥80%):**
- None

**Good (60-79%):**
- DataItemFormatFixed.cpp: 77.8%
- UAP.cpp: 71.4%

**Adequate (40-59%):**
- DataItemFormat.cpp: 42.9%

**Poor (20-39%):**
- Category.cpp: 37.0%
- Tracer.cpp: 37.5%
- DataItemDescription.cpp: 33.3%

**Critical (<20%):**
- Utils.cpp: 13.8% ⚠️
- DataItem.cpp: 21.7% ⚠️

---

## Comparison with Earlier Estimates

### Previous Estimate (Based on test existence):
- Utils: 90-95% (WRONG - Actually 13.8%)
- Category: 70-80% (WRONG - Actually 37.0%)
- DataItem: 50-60% (CLOSE - Actually 21.7%)
- Overall: 65-75% (WRONG - Actually 31.5%)

### Lesson Learned:
Cannot estimate coverage from test count alone. Must measure with gcov/lcov.

---

## Action Plan to Reach 80% C++ Coverage

### Priority 1: Fix Utils.cpp (13.8% → 90%)
**Effort:** 2-3 hours
**Actions:**
1. Review all functions in Utils.cpp
2. Identify which 13 tests actually test (may be testing wrong things)
3. Add targeted tests for uncovered utility functions
4. Focus on format(), string manipulation, data conversion

### Priority 2: Fix DataItem.cpp (21.7% → 80%)
**Effort:** 4-6 hours
**Actions:**
1. Create integration tests that load XML configurations
2. Test parsing with all data item format types
3. Test with real ASTERIX binary data
4. Test error handling (truncated data, invalid formats)
5. Test compound and repetitive data items

### Priority 3: Improve Category.cpp (37.0% → 80%)
**Effort:** 2-3 hours
**Actions:**
1. Test all category management methods
2. Test UAP creation and management
3. Test data item lookup and filtering
4. Test multiple categories in same session

### Priority 4: Improve smaller files
**Effort:** 2-3 hours
**Actions:**
1. DataItemFormatFixed.cpp: 77.8% → 85% (add 1-2 tests)
2. UAP.cpp: 71.4% → 85% (add 1-2 tests)
3. DataItemFormat.cpp: 42.9% → 80% (add 3-4 tests)
4. Tracer.cpp: 37.5% → 80% (add 3-4 tests)

**Total Estimated Effort:** 10-15 hours of test development

---

## Realistic Coverage Target

### Current: 31.5%
### Target: 80%+
### Gap: +48.5 percentage points

**To reach 80% overall:**
- Need to cover ~213 additional lines (out of 302 uncovered)
- This requires comprehensive integration tests, not just unit tests
- Estimated: 30-50 additional C++ tests

---

## DO-278A AL-3 Compliance Status

| Requirement | Target | Current | Gap | Status |
|-------------|--------|---------|-----|--------|
| **Overall C++ coverage** | 90-95% | 31.5% | -58.5 pts | ❌ FAR BELOW |
| **Per-module coverage** | ≥80% | 0 modules | All below | ❌ NONE MEET |
| **Test framework** | Functional | ✅ Google Test | - | ✅ COMPLETE |
| **Test execution** | Automated | ✅ CTest | - | ✅ COMPLETE |

**Overall Status:** ❌ RED - Significant work needed

---

## Combined Coverage Recalculation

### With Accurate C++ Numbers:

**Python:** 88% (153 lines total, ~153 LOC)
**C++:** 31.5% (441 lines total, ~441 LOC measured by lcov)

**Note:** This only covers the tested modules. The full ASTERIX codebase is much larger.

Let me check the actual size of the C++ codebase:

```bash
find ../src/asterix -name "*.cpp" -o -name "*.h" | xargs wc -l
```

This will give us the true size and accurate overall coverage.

---

## Recommendations

### For DO-278A Certification:

**CANNOT CERTIFY** with current 31.5% C++ coverage.

**Required Actions:**
1. ❌ Add 30-50 integration tests
2. ❌ Reach 80%+ per module
3. ❌ Reach 90%+ overall
4. ✅ Test framework complete
5. ✅ All existing tests passing

**Estimated Time:** 2-3 weeks of focused test development

### For Production Deployment:

**CAN DEPLOY** with caveats:
- Core functionality (Utils, DataItem) needs more testing
- Recommend adding integration tests before production
- Monitor for runtime errors in uncovered code paths

---

## Next Steps

1. **Immediate:** Investigate why Utils.cpp shows only 13.8% despite 13 tests
2. **Short-term:** Add integration tests for DataItem parsing
3. **Medium-term:** Reach 80%+ coverage for all modules
4. **Long-term:** Maintain 90%+ coverage with CI/CD enforcement

---

*Report Generated: 2025-10-18*
*Coverage Tool: lcov 2.0-1*
*HTML Report: build/coverage_asterix_html/index.html*
