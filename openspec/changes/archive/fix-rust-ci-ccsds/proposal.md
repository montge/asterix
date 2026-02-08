# Proposal: Fix Broken Rust CI - CCSDS Test Compilation Errors

## Change ID
`fix-rust-ci-ccsds`

## Priority
**P0 - BLOCKER** - This blocks all 14 pending Dependabot PRs and any Rust CI validation.

## Related Issues
- Blocks: 14 Dependabot PRs (#158-#172)
- Affects: Rust Bindings CI/CD workflow, DO-278 Verification CI

## Summary

Fix the Rust CI pipeline which is currently broken on master. The `test_ccsds.rs` integration test has 19 compilation errors due to API drift between the CCSDS module source code and the test file. Additionally, the DO-278 CI has an lcov `exclude` pattern error.

## Root Cause Analysis

### Rust CI (19 compilation errors in `test_ccsds.rs`)

The `test_ccsds.rs` test references APIs that no longer match the `CcsdsConfig`, `CcsdsError`, and `CcsdsSample` types:

**Missing fields on `CcsdsConfig`:**
- `port`
- `interface`

**Missing methods on `CcsdsConfig`:**
- `with_base_apid()`
- `with_crc()`
- `with_multicast()`
- `with_interface()`

**Missing variants on `CcsdsError`:**
- `PacketTooShort`
- `InvalidHeader`
- `CrcMismatch`
- `FragmentationError`
- `IoError`
- `AsterixError`

**Missing field on `CcsdsSample`:**
- `timestamp_us`

### DO-278 CI (lcov error)
The lcov `exclude` pattern `*/test/*` is unused (no matching files), causing a hard error in strict mode. Fix: add `--ignore-errors unused` to the lcov exclude command.

## Scope

### In Scope
- Update `asterix-rs/tests/test_ccsds.rs` to match current CCSDS API
- Fix DO-278 CI lcov exclude pattern error
- Verify all Rust CI jobs pass on all 3 platforms (ubuntu, macos, windows)

### Out of Scope
- Changing the CCSDS module API itself
- Fixing unrelated test failures
- Merging the 14 blocked Dependabot PRs (separate follow-up)

## Success Criteria

- [ ] `cargo test --all-features` compiles and passes on all platforms
- [ ] Rust Bindings CI/CD workflow passes on master
- [ ] DO-278 Verification CI passes on master
- [ ] SonarCloud scan completes successfully
- [ ] At least 1 Dependabot PR is unblocked and can be merged
