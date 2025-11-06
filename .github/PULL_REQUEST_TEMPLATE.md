# Pull Request

## Summary

<!-- Brief description of what this PR does -->

## Related Issues

<!-- Link to related issues using "Closes #123" or "Relates to #456" -->
<!-- REQUIRED for safety-critical changes: Reference requirement ID from docs/SAFETY_CRITICAL.md -->

## Changes Made

<!-- List the main changes in this PR -->

-
-
-

## Safety-Critical Assessment

**Does this PR modify safety-critical code?** (Select one)

- [ ] **YES** - Modifies core parser (`src/asterix/`, `src/engine/`)
- [ ] **YES** - Modifies FFI boundaries (`asterix-rs/src/ffi.rs`, `src/python/`)
- [ ] **PARTIAL** - Modifies safety-related code (bindings, CLI)
- [ ] **NO** - Only documentation, tests, CI, or non-critical code

**If YES or PARTIAL, complete the [Safety-Critical Checklist](#safety-critical-checklist) below.**

---

## Documentation Impact

**REQUIRED:** All PRs must include documentation updates where applicable.

### Files Updated

- [ ] README.md (if user-facing change)
- [ ] docs/ARCHITECTURE.md (if architectural change)
- [ ] docs/BUILD.md (if build process changes)
- [ ] docs/SAFETY_CRITICAL.md (if safety-critical change)
- [ ] CONTRIBUTING.md (if developer workflow changes)
- [ ] Language-specific READMEs (if binding-specific)
  - [ ] asterix/README.md (Python)
  - [ ] asterix-rs/README.md (Rust)
- [ ] CHANGELOG.md (for user-facing changes)
- [ ] API documentation (if public API changes)

### New Documentation

- [ ] User guide for new feature
- [ ] Architecture diagrams (if applicable)
- [ ] Code examples / tutorials
- [ ] Migration guide (if breaking change)

### Documentation Review

- [ ] All code examples compile and run
- [ ] Links validated (no 404s)
- [ ] Terminology consistent with existing docs
- [ ] Diagrams/images included (if applicable)
- [ ] Mermaid diagrams follow project style (use mermaid for all architectural diagrams)

**Additional Documentation Notes:**

<!-- Describe what documentation was added/updated -->

---

## Testing

### Test Coverage

- [ ] Unit tests added (>80% coverage for new code)
- [ ] Integration tests added (if applicable)
- [ ] All existing tests pass
- [ ] Manual testing completed

### Safety/Security Testing (if applicable)

- [ ] FFI boundary validation (if touches language bindings)
- [ ] Memory leak testing (Valgrind/ASAN clean)
- [ ] Fuzz testing (if core parser changes - minimum 1 hour)
- [ ] Static analysis passed (Clippy, CodeQL, cppcheck)
- [ ] Performance regression check (no significant slowdown >5%)
- [ ] Error injection tests (invalid inputs, null pointers, overflows)

**Test Results:**

<!-- Paste relevant test results or CI links -->

```
Example:
- Unit tests: 45/45 passed
- Integration tests: 11/11 passed
- Valgrind: 0 memory leaks, 0 errors
- Coverage: 92% (src/asterix/)
```

---

## Safety-Critical Checklist

**REQUIRED for all changes to safety-critical code** (core parser, FFI boundaries)

### Requirements Traceability

- [ ] Requirement ID assigned (e.g., REQ-XXX from Issue #YY)
- [ ] Code annotated with requirement ID and issue reference
- [ ] Tests tagged with requirement ID
- [ ] Traceability matrix updated (in PR description below)

**Traceability Matrix:**

| Requirement ID | Issue | Implementation Files | Test Files | Status |
|----------------|-------|---------------------|------------|--------|
| REQ-XXX | #YY | `path/to/file.cpp` | `path/to/test.cpp` | ✅ |

### Defensive Programming

- [ ] All external input validated before use
- [ ] Buffer lengths checked before memory operations
- [ ] Integer overflow prevention for arithmetic on untrusted input
- [ ] Null pointer checks before dereferencing
- [ ] Array bounds checked before indexing
- [ ] All malloc/calloc return values checked for NULL

**Example:**
```cpp
// GOOD: Validated before use
if (offset >= len) {
    Tracer::Error("Offset %u exceeds length %zu", offset, len);
    return 0;  // Fail-safe
}
```

### Error Handling

- [ ] All error conditions explicitly handled
- [ ] No silent failures (all errors logged via `Tracer::Error`)
- [ ] Fail-safe design (return 0/NULL/false on error)
- [ ] Error messages include context (offset, category, reason)
- [ ] FFI errors propagate to language binding (Python exceptions, Rust Result)

**Example:**
```cpp
if (!pUAP) {
    Tracer::Error("UAP not found for category %d", category);
    return;  // Fail-safe
}
```

### Memory Safety

- [ ] All loops have bounded iteration counts
- [ ] No raw pointer arithmetic without bounds checking
- [ ] RAII used for resource management (or explicit cleanup in destructor)
- [ ] No memory leaks (verified with Valgrind)
- [ ] No use-after-free (verified with ASAN)
- [ ] No uninitialized memory reads (verified with MSAN)
- [ ] No buffer overflows (verified with ASAN)

**Valgrind Output:**
```
Expected:
==12345== LEAK SUMMARY:
==12345==    definitely lost: 0 bytes in 0 blocks
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==      possibly lost: 0 bytes in 0 blocks
==12345==    still reachable: 0 bytes in 0 blocks
```

### FFI Boundary Validation (if applicable)

**Python C Extension:**
- [ ] All parameters validated (type, length, range)
- [ ] Offset < data length checked
- [ ] blocks_count limited to reasonable maximum (e.g., 10,000)
- [ ] Integer overflow checks before arithmetic
- [ ] Proper reference counting (Py_INCREF/DECREF balanced)
- [ ] Python exceptions set for all error conditions

**Rust FFI:**
- [ ] Empty data check (`data.is_empty()`)
- [ ] Maximum size check (prevent DoS - e.g., 64 KB limit)
- [ ] usize → u32 cast overflow check (`offset > u32::MAX as usize`)
- [ ] Null pointer validation before dereferencing
- [ ] Path traversal prevention (file paths)
- [ ] All FFI functions return `Result<T, E>`

**Example (Rust):**
```rust
// CRITICAL: Validate before usize → u32 cast
if offset > u32::MAX as usize {
    return Err(AsterixError::ParseError {
        offset,
        message: format!("Offset {} exceeds FFI max", offset),
    });
}
```

### Static Analysis

- [ ] No new warnings from compiler (`-Wall -Wextra -Werror`)
- [ ] cppcheck clean (no new issues)
- [ ] Clippy clean (`cargo clippy -- -D warnings`)
- [ ] CodeQL clean (no new security alerts)
- [ ] No unsafe functions (sprintf, strcpy, strcat) - use safe alternatives

**Compiler Output:**
```
Expected: 0 warnings, 0 errors
```

### Performance

- [ ] No unbounded allocations in hot path
- [ ] No dynamic allocation in parsing loop (if avoidable)
- [ ] Buffer reuse for repeated operations
- [ ] Performance benchmarked (no >5% regression)

**Benchmark Results:**
```
Before: 10,000 records/sec
After:  10,200 records/sec (+2%)
```

### Assertions and Logging

- [ ] Preconditions validated (function inputs)
- [ ] Postconditions validated (function outputs)
- [ ] Invariants checked (class state)
- [ ] All error paths logged with context

**Example:**
```cpp
// Precondition
if (pData == NULL) {
    Tracer::Error("pData is null at %s:%d", __FILE__, __LINE__);
    return 0;
}

// Postcondition
if (result <= 0 || result > MAX_SIZE) {
    Tracer::Error("Invalid result: %ld", result);
    return 0;
}
```

---

## Performance Impact

- [ ] No performance impact
- [ ] Performance improvement (attach benchmark results)
- [ ] Minor performance regression (<5%, justified in comments)
- [ ] Performance regression check needed

**Benchmark Results (if applicable):**

<!-- Paste benchmark results showing before/after performance -->

```
Example:
Baseline: 15,234 records/sec
This PR:  15,892 records/sec (+4.3% improvement)

Test file: install/test/testdata/cat062.raw (1 MB, 10,000 records)
Environment: Ubuntu 22.04, Intel i7-9700K @ 3.6GHz
```

---

## Breaking Changes

- [ ] No breaking changes
- [ ] Breaking changes (describe below and update CHANGELOG.md)

**Breaking Changes Description:**

<!-- If breaking changes, describe migration path for users -->

---

## Checklist

- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex logic
- [ ] Documentation updated (see Documentation Impact section)
- [ ] Tests added and passing
- [ ] CI/CD passing
- [ ] Commit messages follow convention (include issue #, REQ-ID if applicable)
- [ ] No merge conflicts
- [ ] Security considerations reviewed (if applicable)

---

## Additional Notes

<!-- Any other context, screenshots, or information -->

---

## Security Review (if applicable)

**For security-sensitive changes (input validation, parsing, FFI boundaries):**

### Threat Model

**What attack vectors does this PR address or introduce?**

<!-- Example:
- Addresses: Integer overflow in repetitive item parsing (VULN-001)
- Introduces: New FFI function with validated parameters
- Mitigations: Overflow detection, fail-safe returns, comprehensive logging
-->

### Security Testing

- [ ] Fuzzing completed (minimum 1 hour for core parser changes)
- [ ] Error injection tests (invalid offsets, null pointers, overflows)
- [ ] Boundary value analysis (0, 1, MAX-1, MAX)
- [ ] Attack scenarios considered and tested

**Fuzz Test Results:**
```
Example:
- Fuzzer: libFuzzer
- Duration: 3 hours
- Test cases: 1,245,032
- Crashes: 0
- Hangs: 0
- Unique paths: 3,421
```

### Reviewer Notes

**For security reviewers:**

<!-- Highlight specific areas that need security review -->

- Files requiring security review: `src/asterix/DataItemFormatRepetitive.cpp`
- Focus areas: Integer overflow prevention (lines 59-74)
- Test coverage: `tests/cpp/test_repetitive_overflow.cpp`

---

**Certification:** This PR includes appropriate documentation updates, testing, and safety-critical validation as specified above.

**Reviewer:** Please verify all applicable checklists are complete before approving.
