# Tasks: Fix Broken Rust CI - CCSDS Test Compilation Errors

## Status: Ready

## Phase 1: Diagnose and Fix CCSDS Test

### Task 1.1: Audit current CCSDS API
**Files**: `asterix-rs/src/transport/ccsds.rs`, `asterix-rs/src/transport/mod.rs`
**Validation**: Document current `CcsdsConfig`, `CcsdsError`, `CcsdsSample` APIs

- [x] Read current `CcsdsConfig` struct fields and builder methods
- [x] Read current `CcsdsError` enum variants
- [x] Read current `CcsdsSample` struct fields
- [x] Compare against `test_ccsds.rs` expectations

### Task 1.2: Update test_ccsds.rs to match API
**File**: `asterix-rs/tests/test_ccsds.rs`
**Validation**: `cargo test --features ccsds` compiles without errors

- [x] Fix `CcsdsConfig` field references (port, interface)
- [x] Fix `CcsdsConfig` builder method calls (with_base_apid, with_crc, with_multicast, with_interface)
- [x] Fix `CcsdsError` variant references (PacketTooShort, InvalidHeader, CrcMismatch, FragmentationError, IoError, AsterixError)
- [x] Fix `CcsdsSample` field references (timestamp_us)
- [x] Ensure tests exercise the actual current API surface

---

## Phase 2: Fix DO-278 lcov Error

### Task 2.1: Fix lcov exclude pattern
**File**: `.github/workflows/do278-verification.yml` (or equivalent)
**Validation**: DO-278 CI C++ Coverage Analysis job passes

- [x] Add `--ignore-errors unused` to lcov exclude command
- [x] Verify coverage report still generates correctly

---

## Phase 3: Verify CI

### Task 3.1: Verify Rust CI passes
**Validation**: All Rust Bindings CI/CD jobs pass on master

- [x] `cargo test --all-features` passes locally
- [x] Push fix and verify CI on all 3 platforms (ubuntu, macos, windows) - Tests pass on all 3 platforms
- [x] Coverage jobs (tarpaulin, llvm-cov) pass
- [ ] Quality Checks pass - **BLOCKED**: 3 pre-existing clippy errors in parser.rs (not from CCSDS fix)
  - Line 531: `clippy::single_match` (match should be if let)
  - Lines 835, 837: `clippy::approx_constant` (3.14 triggers PI detection in Rust 1.93.0)

### Task 3.2: Verify Dependabot PRs unblocked
**Validation**: At least 1 Dependabot PR shows green CI

- [ ] Rebase or re-trigger CI on a Dependabot PR
- [ ] Confirm systemic failures are resolved

---

## Dependencies

```
Task 1.1 (Audit API)
    └── Task 1.2 (Fix tests)
         ├── Task 2.1 (Fix lcov) [parallel]
         └── Task 3.1 (Verify CI)
              └── Task 3.2 (Verify Dependabot unblocked)
```
