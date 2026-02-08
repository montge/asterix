# Proposal: Reduce Code Smells - Phase 2

## Change ID
`reduce-code-smells-phase2`

## Priority
**P2 - MEDIUM** - Quality improvement toward "model software" standards per `code-quality` spec.

## Related Issues
- Continues: `fix-sonarcloud-issues` (archived 2025-12-20, achieved 71% reduction)
- Continues: `improve-code-quality` (archived 2025-12-14, initial quality initiative)
- Spec: `openspec/specs/code-quality/spec.md` (target: 0 code smells, max 25 cognitive complexity)

## Summary

Eliminate the remaining ~1,400 code smells identified by SonarCloud to achieve the "model software" quality target of 0 code smells. The prior `fix-sonarcloud-issues` change reduced smells by 71% (5,097 to ~1,400), fixing all bugs, vulnerabilities, and security hotspots. This change addresses the deferred items and remaining systematic issues.

## Current State (as of 2026-02-07)

| Metric | Current | Target | Gap |
|--------|---------|--------|-----|
| Code Smells | ~1,400 | 0 | ~1,400 remaining |
| Cognitive Complexity Violations | 1 (XMLParser `handleBitsStart`, formerly `parseAttributes`, complexity 57) | max 25/function | 1 function |
| Bugs | 0 (2 false positives) | 0 | PASS |
| Vulnerabilities | 0 | 0 | PASS |
| Security Hotspots | 45 reviewed | All reviewed | PASS |
| Duplication | 1.8% | <3% | PASS |

### Remaining Code Smell Breakdown (by SonarCloud rule)

| Rule | Count | Description | Category |
|------|-------|-------------|----------|
| cpp:S5025 | ~578 | Use smart pointers instead of raw `new`/`delete` | Memory Management |
| cpp:S4962 | ~516 | Modernize `std::string`/`std::vector` initialization | Modernization |
| cpp:S5945 | ~441 | Unused return values from functions | Correctness |
| cpp:S5827 | ~248 | Use `auto` to avoid redundant type repetition | Modernization |
| cpp:S3471 | ~176 | Pass by const reference instead of by value | Performance |
| Misc | ~remaining | Naming conventions, other minor rules | Style |

### What Was Deferred (and Why)

From `fix-sonarcloud-issues`:

1. **Smart pointer migration (cpp:S5025)** - ~578 issues. Deferred because it is a large mechanical refactor touching core memory management across the engine and ASTERIX layers. Risk of regressions at FFI boundaries (Python C extension, Rust CXX bridge). Requires careful valgrind verification.

2. **Naming convention standardization** - Deferred due to large scope and low functional impact. Would require renaming member variables across the codebase, affecting all language bindings.

3. **Coverage improvement** - Explicitly out of scope; tracked separately in `improve-test-coverage`.

4. **XMLParser `handleBitsStart` cognitive complexity (57)** - The last remaining function above the 25 threshold. Was `parseAttributes` in the original analysis. Deferred from the December 2025 sessions due to time constraints.

## Approach

### Phased Execution

This change uses a four-phase approach, ordered by impact and risk:

1. **Phase 1: Cognitive Complexity** (high value, focused scope)
   - Refactor the single remaining violation (`handleBitsStart`, complexity 57 -> target 25)
   - Audit for any new functions above the threshold introduced since December 2025
   - Low risk: function extraction pattern proven in prior phases

2. **Phase 2: Smart Pointer Migration** (highest issue count, mechanical)
   - Systematic `new`/`delete` to `std::unique_ptr`/`std::shared_ptr` conversion
   - Prioritize highest-traffic files first (core parsing pipeline)
   - Verify each batch with valgrind (0 leaks required) and integration tests
   - Special care at FFI boundaries (Python C extension, Rust CXX bridge)

3. **Phase 3: String/Vector Modernization** (cpp:S4962, cpp:S5827)
   - Replace C-style string operations with `std::string` methods
   - Modernize initialization patterns
   - Add `reserve()` where beneficial for performance
   - Use `auto` to reduce type repetition where readability is maintained

4. **Phase 4: Remaining Smells** (cleanup)
   - Capture unused return values (cpp:S5945) or mark intentionally unused
   - Add const reference pass-by for function parameters (cpp:S3471)
   - Address remaining minor rules
   - Final SonarCloud verification pass

## Scope Boundaries

### In Scope
- All ~1,400 remaining SonarCloud code smells in C++ source
- XMLParser `handleBitsStart` cognitive complexity reduction
- Valgrind verification after each phase
- Integration test verification after each phase

### Out of Scope
- Test coverage improvement (tracked in `improve-test-coverage`)
- Python code smells (Python-specific quality tracked separately)
- Rust code smells (already at exemplary standard per clippy)
- SonarCloud quality gate UI configuration (requires manual SonarCloud admin action)
- Naming convention overhaul (large cross-cutting change, may warrant its own proposal)
- Generated files (AsterixData.hpp/cpp from Cyclone DDS IDL compiler, excluded in SonarCloud config)

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Smart pointer migration breaks FFI | Medium | High | Valgrind + integration tests after each file; test Python/Rust bindings |
| Regressions in parsing correctness | Low | High | 11 integration tests + valgrind verify each change batch |
| Build failures on MSVC/macOS | Low | Medium | CI runs on all 3 platforms; test locally where possible |
| SonarCloud count doesn't decrease | Low | Low | Verify exclusions config; run analysis after each phase |

## Success Criteria

- [ ] SonarCloud reports 0 code smells (or documented false positives only)
- [ ] Cognitive complexity: all functions at or below 25
- [ ] All integration tests pass (11/11)
- [ ] Valgrind: 0 memory leaks
- [ ] All CI pipelines pass on Linux, Windows, macOS
- [ ] No regressions in Python or Rust bindings
