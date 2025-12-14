# Change: Fix SonarCloud Issues

## Why

The code-quality spec requires 0 bugs, 0 vulnerabilities, and 0 code smells. Current SonarCloud analysis shows:
- **234 bugs** remaining
- **112 security hotspots** requiring review
- **~5,097 code smells** remaining

These issues must be addressed to meet the "model software" quality standards required for safety-critical ATM applications.

## What Changes

### Phase 1: Bug Fixes (Priority: CRITICAL)
Address all 234 bugs identified by SonarCloud, categorized by:
- Null pointer dereferences
- Resource leaks
- Buffer overflows
- Uninitialized variables
- Logic errors

### Phase 2: Security Hotspot Review (Priority: HIGH)
Review and resolve 112 security hotspots:
- Mark as safe with documented justification, OR
- Fix actual security issues

### Phase 3: High-Impact Code Smells (Priority: MEDIUM)
Focus on code smells that affect maintainability:
- Cognitive complexity violations (functions >25)
- Code duplication (>3% threshold)
- Unused code removal
- Naming convention violations

### Phase 4: Remaining Code Smells (Priority: LOW)
Address remaining code smells incrementally:
- Minor style issues
- Documentation gaps
- Refactoring opportunities

## Impact

- **Affected specs**: `code-quality` (existing requirements will be satisfied)
- **Affected code**: Primarily `src/asterix/`, `src/engine/`, language bindings
- **Testing**: All changes require passing existing test suites + valgrind
- **CI/CD**: SonarCloud quality gate will become enforceable

## Risk Assessment

- **Low risk**: Code smell fixes (style, naming)
- **Medium risk**: Bug fixes (may change behavior)
- **High risk**: Security hotspot changes (require careful review)

## Success Criteria

1. SonarCloud shows 0 bugs
2. SonarCloud shows 0 vulnerabilities
3. All security hotspots reviewed and resolved
4. Code smells reduced by >90%
5. All existing tests pass
6. No memory leaks (valgrind)

## Known Limitations

### SonarCloud Free Tier - Single Branch Only

**Current State (Dec 2024):** SonarCloud's free tier for open source projects only supports scanning the default branch (master). Branch analysis (including gitflow `develop` branch) is not available.

**Impact:**
- All SonarCloud analysis must target the `master` branch
- Feature branches and `develop` branch cannot be scanned until merged
- Pull request decoration works for PRs targeting master

**Future Enhancement (Expected 2026):**
SonarCloud plans to introduce branch support for open source projects. When available:
- [ ] Enable `develop` branch scanning
- [ ] Configure gitflow branch patterns in SonarCloud
- [ ] Update `sonar.newCode.referenceBranch` to `develop`
- [ ] Add branch-specific quality gates

**Workaround:** Run local SonarQube instance for branch analysis during development if needed.
