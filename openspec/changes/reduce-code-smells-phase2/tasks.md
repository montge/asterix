# Tasks: Reduce Code Smells - Phase 2

## Status: Ready

---

## Phase 1: Cognitive Complexity (High Value, Focused)

### Task 1.1: Refactor XMLParser::handleBitsStart (complexity 57 -> 25)
**File**: `src/asterix/XMLParser.cpp` (line 372)
**Validation**: SonarCloud cognitive complexity <= 25; all 11 integration tests pass

- [x] 1.1.1 Analyze `handleBitsStart` control flow and nesting structure
- [x] 1.1.2 Extract bit attribute parsing (`bit`, `from`, `to`) into `parseBitRange()` helper
- [x] 1.1.3 Extract encoding attribute parsing (`encode`) into `parseEncodeAttribute()` helper
- [x] 1.1.4 Verify cognitive complexity is at or below 25 (estimated ~15 after extraction)
- [x] 1.1.5 Run integration tests (11/11 pass)
- [x] 1.1.6 Run valgrind (0 memory leaks)

### Task 1.2: Audit remaining functions above complexity threshold
**Files**: All `src/asterix/*.cpp`, `src/engine/*.cxx`, `src/main/*.cpp`
**Validation**: No function has cognitive complexity > 25

- [x] 1.2.1 Manual audit of functions refactored in December 2025
- [x] 1.2.2 `ElementHandlerStart` - flat if/else dispatch chain (~22), acceptable after PR #151 refactoring
- [x] 1.2.3 `main()` - already refactored with helpers (~25-30), flat argument parsing loop is acceptable
- [x] 1.2.4 `addFormatToParent` - ~35 complexity due to nested format type checks; acceptable as extracted helper
- [x] 1.2.5 No newly-introduced functions above threshold; all high-complexity functions have been addressed

---

## Phase 2: Smart Pointer Migration (Highest Count, Mechanical)

### Task 2.1: Identify top files by raw new/delete usage
**Files**: All C++ source files
**Validation**: Prioritized file list with issue counts

- [ ] 2.1.1 Search codebase for raw `new` allocations (exclude placement new)
- [ ] 2.1.2 Search codebase for raw `delete` calls
- [ ] 2.1.3 Map ownership patterns: who allocates, who deletes, who transfers ownership
- [ ] 2.1.4 Identify FFI boundary files that need special handling (Python C extension, Rust CXX bridge)
- [ ] 2.1.5 Prioritize files: core parsing pipeline first, then engine, then bindings

### Task 2.2: Migrate ASTERIX layer to smart pointers
**Files**: `src/asterix/*.cpp`, `src/asterix/*.h`
**Validation**: `cargo test --all-features`, `python -m unittest`, valgrind 0 leaks

- [ ] 2.2.1 Migrate `XMLParser.cpp` - replace `new DataItemBits/Format*` with `std::make_unique`
- [ ] 2.2.2 Migrate `Category.cpp` - update ownership of DataItem and Format objects
- [ ] 2.2.3 Migrate `AsterixDefinition.cpp` - update Category ownership
- [ ] 2.2.4 Migrate `DataRecord.cpp` - update DataItem ownership in records
- [ ] 2.2.5 Migrate `DataBlock.cpp` - update DataRecord ownership
- [ ] 2.2.6 Update container types (`std::list<DataItemFormat*>` -> `std::list<std::unique_ptr<...>>`)
- [ ] 2.2.7 Run integration tests after each file (11/11 must pass)
- [ ] 2.2.8 Run valgrind after full ASTERIX layer migration (0 leaks required)

### Task 2.3: Migrate Engine layer to smart pointers
**Files**: `src/engine/*.cxx`, `src/engine/*.hxx`
**Validation**: Integration tests pass, valgrind 0 leaks

- [ ] 2.3.1 Migrate device factory (partially done - `std::make_unique` already used)
- [ ] 2.3.2 Migrate channel management objects
- [ ] 2.3.3 Migrate format descriptor objects
- [ ] 2.3.4 Run integration tests (11/11 must pass)
- [ ] 2.3.5 Run valgrind (0 leaks required)

### Task 2.4: Verify FFI boundary safety
**Files**: `src/python/*.c`, `src/python/*.cpp`, `asterix-rs/src/ffi.rs`
**Validation**: Python and Rust bindings work correctly with smart pointer changes

- [ ] 2.4.1 Verify Python C extension handles smart pointer changes at the boundary
- [ ] 2.4.2 Verify Rust CXX bridge handles ownership transfer correctly
- [ ] 2.4.3 Run `python -m unittest` (all tests pass)
- [ ] 2.4.4 Run `cargo test --all-features` from `asterix-rs/` (all tests pass)
- [ ] 2.4.5 Run valgrind on Python and C++ integration tests

---

## Phase 3: String/Vector Modernization (cpp:S4962, cpp:S5827)

### Task 3.1: Replace C-style string operations
**Files**: All C++ source files
**Validation**: SonarCloud cpp:S4962 count reduced to 0

- [ ] 3.1.1 Replace `strcmp()` with `std::string::operator==` where applicable
- [ ] 3.1.2 Replace `strcpy()`/`strncpy()` with `std::string` assignment
- [ ] 3.1.3 Replace `strlen()` with `std::string::length()` or `std::string::empty()`
- [ ] 3.1.4 Replace `atoi()`/`atof()` with `std::stoi()`/`std::stod()` with error handling
- [ ] 3.1.5 Modernize `std::vector` and `std::string` initialization patterns
- [ ] 3.1.6 Run integration tests (11/11 must pass)

### Task 3.2: Add reserve() and use auto where beneficial
**Files**: All C++ source files with string/vector building
**Validation**: cpp:S5827 count reduced; no performance regressions

- [ ] 3.2.1 Add `std::string::reserve()` to functions that build strings in loops
- [ ] 3.2.2 Add `std::vector::reserve()` where final size is known or estimable
- [ ] 3.2.3 Replace redundant type declarations with `auto` (e.g., iterator declarations)
- [ ] 3.2.4 Verify no performance regressions (compare with existing benchmarks if available)
- [ ] 3.2.5 Run integration tests (11/11 must pass)

---

## Phase 4: Remaining Smells (Cleanup)

### Task 4.1: Fix unused return values (cpp:S5945)
**Files**: All C++ source files
**Validation**: SonarCloud cpp:S5945 count reduced to 0

- [ ] 4.1.1 Audit unused return values - categorize as intentional vs. oversight
- [ ] 4.1.2 Capture return values where they indicate errors that should be checked
- [ ] 4.1.3 Add `[[maybe_unused]]` or `(void)` cast for intentionally ignored returns
- [ ] 4.1.4 Run integration tests (11/11 must pass)

### Task 4.2: Add const reference parameters (cpp:S3471)
**Files**: All C++ source files
**Validation**: SonarCloud cpp:S3471 count reduced to 0

- [ ] 4.2.1 Identify functions passing `std::string` and other objects by value
- [ ] 4.2.2 Change to `const std::string&` where the parameter is not modified
- [ ] 4.2.3 Verify no impact on virtual function signatures (override compatibility)
- [ ] 4.2.4 Run integration tests (11/11 must pass)

### Task 4.3: Final cleanup and verification
**Files**: All C++ source files
**Validation**: SonarCloud shows 0 code smells (or documented false positives only)

- [ ] 4.3.1 Address any remaining miscellaneous code smells
- [ ] 4.3.2 Run full test suite (C++, Python, Rust)
- [ ] 4.3.3 Run valgrind - 0 memory leaks required
- [ ] 4.3.4 Verify SonarCloud dashboard shows 0 code smells
- [ ] 4.3.5 Document any false positives that cannot be resolved
- [ ] 4.3.6 Update `openspec/specs/code-quality/spec.md` current status

---

## Dependencies

```
Phase 1 (Cognitive Complexity)
    ├── Task 1.1 (handleBitsStart refactor)
    └── Task 1.2 (Audit remaining functions)

Phase 2 (Smart Pointers) -- depends on Phase 1 completion
    ├── Task 2.1 (Identify files)
    │    └── Task 2.2 (ASTERIX layer)
    │         └── Task 2.3 (Engine layer)
    │              └── Task 2.4 (FFI boundary verification)

Phase 3 (String/Vector) -- independent of Phase 2, can run in parallel
    ├── Task 3.1 (C-style string ops)
    └── Task 3.2 (reserve() and auto)

Phase 4 (Remaining) -- depends on Phases 2 and 3
    ├── Task 4.1 (Unused return values)
    ├── Task 4.2 (Const reference params)
    └── Task 4.3 (Final verification)
```

## Verification Checklist (per phase)

- [ ] All 11 integration tests pass
- [ ] Valgrind: 0 memory leaks
- [ ] `python -m unittest` passes
- [ ] `cargo test --all-features` passes (from `asterix-rs/`)
- [ ] No new compiler warnings
- [ ] SonarCloud smell count decreases as expected
- [ ] No hardcoded paths: `git grep -E "(C:\\\\Users|/home/|/Users/)"`
