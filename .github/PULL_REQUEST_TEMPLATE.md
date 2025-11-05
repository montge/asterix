# Pull Request

## Summary

<!-- Brief description of what this PR does -->

## Related Issues

<!-- Link to related issues using "Closes #123" or "Relates to #456" -->

## Changes Made

<!-- List the main changes in this PR -->

-
-
-

## Documentation Impact

**REQUIRED:** All PRs must include documentation updates where applicable.

### Files Updated

- [ ] README.md (if user-facing change)
- [ ] docs/ARCHITECTURE.md (if architectural change)
- [ ] docs/BUILD.md (if build process changes)
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

**Additional Documentation Notes:**

<!-- Describe what documentation was added/updated -->

## Testing

### Test Coverage

- [ ] Unit tests added (>80% coverage for new code)
- [ ] Integration tests added (if applicable)
- [ ] All existing tests pass
- [ ] Manual testing completed

### Safety/Security Testing (if applicable)

- [ ] FFI boundary validation (if touches language bindings)
- [ ] Memory leak testing (Valgrind/ASAN clean)
- [ ] Fuzz testing (if core parser changes)
- [ ] Static analysis passed (Clippy, CodeQL)
- [ ] Performance regression check (no significant slowdown)

**Test Results:**

<!-- Paste relevant test results or CI links -->

## Safety-Critical Checklist

**Only for changes to core parser or FFI boundaries:**

- [ ] All loops have bounded iteration count
- [ ] No dynamic allocation in hot path
- [ ] All buffer accesses bounds-checked
- [ ] Error handling for all failure cases
- [ ] No raw pointer arithmetic
- [ ] RAII for resource management
- [ ] No undefined behavior (ASAN/MSAN/UBSAN clean)

## Performance Impact

- [ ] No performance impact
- [ ] Performance improvement (attach benchmark results)
- [ ] Minor performance regression (<5%, justified in comments)
- [ ] Performance regression check needed

**Benchmark Results (if applicable):**

<!-- Paste benchmark results showing before/after performance -->

## Breaking Changes

- [ ] No breaking changes
- [ ] Breaking changes (describe below and update CHANGELOG.md)

**Breaking Changes Description:**

<!-- If breaking changes, describe migration path for users -->

## Checklist

- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex logic
- [ ] Documentation updated (see Documentation Impact section)
- [ ] Tests added and passing
- [ ] CI/CD passing
- [ ] Commit messages follow convention
- [ ] No merge conflicts

## Additional Notes

<!-- Any other context, screenshots, or information -->

---

**Documentation Review:** This PR includes appropriate documentation updates as specified in the Documentation Impact section above.
