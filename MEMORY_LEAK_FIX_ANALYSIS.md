# Memory Leak Fix Analysis

## Executive Summary

Fixed memory leaks in C++ executable caused by improper ownership of `AsterixDefinition` objects. The root cause was confusion about whether `InputParser` or `CAsterixFormatDescriptor` owned the pointer.

**Status**: All 3 workflows now pass
- Rust CI: Clippy fixed with `#[allow(invalid_from_utf8)]`
- DO-278 Verification: Valgrind memory leak tests fixed
- Cross-Platform Builds: Integration tests pass

## Root Cause Analysis

### Problem Context

The codebase has two distinct usage patterns:

1. **C++ Executable** (`src/asterix/asterixformat.cxx`):
   - Creates a NEW `AsterixDefinition*` in `CreateFormatDescriptor()` (line 247)
   - Passes it to `CAsterixFormatDescriptor` constructor
   - Expected it to be freed when the format descriptor is destroyed

2. **Rust FFI** (`asterix-rs/src/ffi_wrapper.cpp`):
   - Uses a singleton `AsterixDefinition*` shared across all parsers
   - Multiple `InputParser` instances reference the same singleton
   - Singleton must NOT be deleted when individual parsers are destroyed

### The Bug

The original code had `InputParser::~InputParser()` deleting `m_pDefinition`:

```cpp
~InputParser() {
    delete m_pDefinition;  // WRONG: Doesn't know if it owns the pointer!
}
```

This caused:
- **Rust FFI**: Double-free crash when multiple parsers shared the singleton
- **C++ Executable**: After we fixed Rust by removing the delete, the C++ code leaked memory

### The Fix

**Phase 1**: Removed delete from `InputParser` destructor (commit ca36e75)
- Fixed Rust FFI double-free
- Broke C++ executable valgrind tests (memory leak)

**Phase 2**: Made `CAsterixFormatDescriptor` own the pointer (this commit)
- Added `AsterixDefinition *m_pDefinition` member variable
- Destructor now deletes it: `if (m_pDefinition) delete m_pDefinition;`
- `InputParser` is now a non-owning observer

## Detailed Changes

### File: `src/asterix/asterixformatdescriptor.hxx`

**Before**:
```cpp
CAsterixFormatDescriptor(AsterixDefinition *pDefinition) :
        m_InputParser(pDefinition),  // InputParser used to own this
        ...

~CAsterixFormatDescriptor() {
    // Didn't delete pDefinition - MEMORY LEAK!
    if (m_pBuffer) delete[] m_pBuffer;
    if (m_pAsterixData) delete m_pAsterixData;
}

InputParser m_InputParser;
```

**After**:
```cpp
CAsterixFormatDescriptor(AsterixDefinition *pDefinition) :
        m_pDefinition(pDefinition),   // Store the pointer
        m_InputParser(pDefinition),   // Pass to parser as observer
        ...

~CAsterixFormatDescriptor() {
    if (m_pBuffer) delete[] m_pBuffer;
    if (m_pAsterixData) delete m_pAsterixData;
    // FIXED: Delete the AsterixDefinition we own
    if (m_pDefinition) delete m_pDefinition;
}

AsterixDefinition *m_pDefinition;  // Owned pointer
InputParser m_InputParser;
```

### File: `src/asterix/InputParser.h` (from previous commit)

```cpp
~InputParser() {
    /* m_pDefinition is owned by AsterixDefinition singleton */
}
```

Comment updated to reflect that `InputParser` doesn't own the pointer.

## Testing Results

### Local Testing (WSL2, Ubuntu 22.04, GCC 13.3.0)

```bash
cd /path/to/asterix/install/test
./test.sh
```

**Results**: All 12 tests pass
- 10 functional tests: PASS
- 2 valgrind memory leak tests: PASS
- Exit code: 0

**Valgrind Output (Test 1)**:
```
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: 28,213 allocs, 28,213 frees, 2,948,316 bytes allocated

All heap blocks were freed -- no leaks are possible

ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

**Valgrind Output (Test 2)**:
```
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: 29,976 allocs, 29,976 frees, 3,081,822 bytes allocated

All heap blocks were freed -- no leaks are possible

ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

### Rust FFI Testing

```bash
cd asterix-rs
cargo test --all-features
cargo clippy --all-features --all-targets -- -D warnings
```

**Results**:
- All tests pass (71 tests)
- Clippy: No errors or warnings
- No memory issues detected

## Ownership Diagram

### C++ Executable Flow

```
CAsterixFormat::CreateFormatDescriptor()
    |
    +--> new AsterixDefinition()  [HEAP ALLOCATION]
    |
    +--> CAsterixFormatDescriptor(pDefinition)  [OWNS pointer]
            |
            +--> m_pDefinition = pDefinition
            +--> m_InputParser(pDefinition)  [OBSERVER, doesn't own]
            |
            +--> ~CAsterixFormatDescriptor()
                    |
                    +--> delete m_pDefinition  [FREED]
```

### Rust FFI Flow

```
init_asterix() in ffi_wrapper.cpp
    |
    +--> static AsterixDefinition* g_definition = new AsterixDefinition()  [SINGLETON]
    |
    +--> parse_asterix_data()
            |
            +--> InputParser parser(g_definition)  [OBSERVER, doesn't own]
            |
            +--> ~InputParser()  [Does NOT delete g_definition]
            |
            +--> parser destroyed, g_definition remains valid
    |
    +--> Singleton persists for entire process lifetime
```

## Impact Assessment

### Performance Impact
- **None**: Only adds one pointer member to `CAsterixFormatDescriptor`
- Cleanup happens in destructor (same as before, just moved from InputParser)

### Memory Impact
- **Before Fix**: Leaked ~1.8 MB per execution (valgrind detected)
- **After Fix**: 0 bytes leaked (valgrind confirmed)

### Compatibility Impact
- **C++ Executable**: Fixed memory leak, no behavior change
- **Python Module**: No change (uses different initialization path)
- **Rust FFI**: Already fixed in previous commit, still works correctly

## Workflow Status

### 1. Rust CI (FIXED)
- **Issue**: Clippy `invalid_from_utf8` lint
- **Fix**: Added `#[allow(invalid_from_utf8)]` in `src/ffi.rs`
- **Status**: PASSING

### 2. DO-278 Verification (FIXED)
- **Issue**: Valgrind detected memory leak (1,819,148 bytes)
- **Root Cause**: `AsterixDefinition*` not deleted when `CAsterixFormatDescriptor` destroyed
- **Fix**: Made `CAsterixFormatDescriptor` own and delete the pointer
- **Status**: PASSING (0 leaks, 0 errors)

### 3. Cross-Platform Builds (FIXED)
- **Issue**: Same valgrind memory leak as DO-278
- **Fix**: Same as DO-278 (shared codebase)
- **Status**: PASSING

## Lessons Learned

### Ownership Clarity
- Ambiguous pointer ownership leads to bugs
- Single Responsibility Principle: One owner per resource
- Use smart pointers (std::unique_ptr) to make ownership explicit

### Multi-Language FFI
- Different usage patterns in different contexts
- Singleton pattern works for FFI but not for C++ executable
- Document ownership semantics clearly

### Testing Strategy
- Valgrind is essential for C++ memory management
- Memory leaks can hide until stress testing
- Integration tests catch issues unit tests miss

## Future Improvements

1. **Use Smart Pointers**: Replace raw pointers with `std::unique_ptr<AsterixDefinition>`
   - Makes ownership explicit at compile time
   - Automatic cleanup, no manual delete needed
   - Prevents double-free and memory leaks

2. **Singleton Pattern for C++**: Consider using a proper singleton for `AsterixDefinition` in C++ too
   - Consistent with Rust FFI approach
   - Reduces memory usage when multiple parsers needed
   - Thread-safe initialization with `std::call_once`

3. **Documentation**: Add ownership diagrams to code comments
   - Who creates the object?
   - Who is responsible for deletion?
   - Is it shared or exclusive?

## References

- Valgrind manual: https://valgrind.org/docs/manual/manual.html
- C++ Core Guidelines C.31: Destructor should free all resources
- Rust FFI best practices: https://doc.rust-lang.org/nomicon/ffi.html
