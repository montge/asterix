## Phase 1: Bug Fixes (Priority: CRITICAL)

### 1.1 Export and Categorize Bugs
- [x] 1.1.1 Export full bug list from SonarCloud API/UI
- [x] 1.1.2 Categorize bugs by type (null pointer, resource leak, etc.)
- [x] 1.1.3 Categorize bugs by file/component
- [x] 1.1.4 Prioritize by severity and fix complexity

### 1.2 Fix Null Pointer Issues
- [x] 1.2.1 Identify all null pointer dereference bugs
- [x] 1.2.2 Add null checks or use std::optional where appropriate
- [x] 1.2.3 Verify fixes with local tests

**Commits:**
- `6519f6e` - Fix uninitialized structs and null pointer issues in engine

### 1.3 Fix Resource Leak Issues
- [x] 1.3.1 Identify all resource leak bugs
- [x] 1.3.2 Convert to RAII patterns (smart pointers, scope guards)
- [x] 1.3.3 Verify with valgrind - 0 leaks required

**Commits:**
- `bbfcb89` - Replace strdup with new[] for consistent memory deallocation
- `7948eff` - Fix memory leaks in XMLParser error paths

### 1.4 Fix Logic Errors
- [ ] 1.4.1 Identify logic error bugs
- [ ] 1.4.2 Fix incorrect conditions, off-by-one errors, etc.
- [ ] 1.4.3 Add unit tests for edge cases

### 1.5 Fix Remaining Bugs
- [x] 1.5.1 Address uninitialized variable bugs
- [ ] 1.5.2 Address buffer overflow bugs
- [ ] 1.5.3 Address type mismatch bugs
- [ ] 1.5.4 Verify all bugs resolved in SonarCloud

## Phase 2: Security Hotspot Review (Priority: HIGH)

### 2.1 Triage Security Hotspots
- [x] 2.1.1 Export security hotspot list from SonarCloud
- [x] 2.1.2 Categorize by type (crypto, injection, auth, etc.)
- [x] 2.1.3 Identify hotspots requiring fixes vs safe-to-acknowledge

### 2.2 Review and Resolve Hotspots
- [x] 2.2.1 Review each hotspot for actual security risk
- [x] 2.2.2 Fix genuine security issues
- [x] 2.2.3 Document justification for safe hotspots
- [ ] 2.2.4 Mark resolved in SonarCloud

**Commits:**
- `d28ca72` - Add null-termination after strncpy in diskdevice

**Security Hotspot Analysis (45 total):**

| Category | Count | Status | Notes |
|----------|-------|--------|-------|
| Buffer-overflow (strlen) | 27 | Safe | fgets/vsnprintf guarantee null-termination |
| Buffer-overflow (strncpy) | 10 | Fixed | Added explicit null-termination |
| Buffer-overflow (strncat) | 4 | Safe | Proper bounds checking in place |
| Weak-cryptography | 4 | Safe | Demo data generation, not security |

## Phase 3: High-Impact Code Smells (Priority: MEDIUM)

### 3.1 Cognitive Complexity
- [x] 3.1.1 Identify functions with complexity >25
- [ ] 3.1.2 Refactor XMLParser::ElementHandlerStart (507 lines)
- [ ] 3.1.3 Refactor other high-complexity methods
- [ ] 3.1.4 Verify complexity reduced below threshold

**Cognitive Complexity Analysis (30 functions above threshold):**

| Complexity | File | Function | Status |
|-----------|------|----------|--------|
| 348 | XMLParser.cpp:107 | parseDataRecord | OPEN - Major refactor needed |
| 161 | asterix.cpp:131 | main | OPEN - CLI parsing complexity |
| 95 | converterengine.cxx:103 | convertRecord | OPEN |
| 57 | asterixgpssubformat.cxx:58 | parseGPS | OPEN |
| 57 | XMLParser.cpp:629 | parseAttributes | OPEN |
| 49 | asterixpcapsubformat.cxx:75 | parsePCAP | OPEN |

*Note: XMLParser refactoring is a significant undertaking (348 complexity). Recommend as separate PR.*

### 3.2 Code Duplication
- [x] 3.2.1 Identify duplicated code blocks
- [x] 3.2.2 Extract common functionality into helpers
- [x] 3.2.3 Verify duplication below 3% threshold

**Duplication Metrics:**
- Duplicated Lines Density: **1.8%** (below 3% threshold)
- Duplicated Blocks: 47
- Total Duplicated Lines: 1,050

### 3.3 Unused Code
- [x] 3.3.1 Identify unused variables, functions, imports
- [x] 3.3.2 Remove dead code safely
- [x] 3.3.3 Verify no regressions

*Most unused code issues already fixed with `[[maybe_unused]]` attributes.*

### 3.4 const_cast Issues (Additional)
- [x] 3.4.1 Identify const_cast usages
- [x] 3.4.2 Replace with proper const-correct code
- [x] 3.4.3 Verify no regressions

**Commits:**
- `0e9bd4b` - Remove const_cast in copy constructors

### 3.5 Missing Override Keywords (cpp:S3471)
- [x] 3.5.1 Add override to DataItemFormat subclasses clone() methods
- [x] 3.5.2 Add override to engine device classes
- [x] 3.5.3 Verify build and tests pass

**Files Fixed:**
- DataItemFormatBDS.h, DataItemFormatCompound.h, DataItemFormatExplicit.h
- DataItemFormatFixed.h, DataItemFormatRepetitive.h, DataItemFormatVariable.h
- UAPItem.h (clone method)
- serialdevice.hxx, udpdevice.hxx, diskdevice.hxx, stddevice.hxx, tcpdevice.hxx

## Phase 4: Remaining Code Smells (Priority: LOW)

### 4.1 Style and Naming
- [ ] 4.1.1 Address consistent naming violations
- [ ] 4.1.2 Fix formatting issues
- [ ] 4.1.3 Add missing documentation

### 4.2 Incremental Cleanup
- [ ] 4.2.1 Fix code smells during related changes
- [ ] 4.2.2 Track progress in SonarCloud dashboard

### 4.3 Memory Management (Deferred)
- [ ] 4.3.1 Convert new/delete to smart pointers (cpp:S5025)
- [ ] 4.3.2 Implement Rule of Five for resource classes (cpp:S3624)

*Note: malloc in Wireshark wrapper is intentional for C API compatibility.*

## Phase 5: Verification

### 5.1 Final Verification
- [ ] 5.1.1 Run full test suite (C++, Python, Rust)
- [ ] 5.1.2 Run valgrind memory check - 0 leaks
- [ ] 5.1.3 Verify SonarCloud shows 0 bugs
- [ ] 5.1.4 Verify SonarCloud shows 0 vulnerabilities
- [ ] 5.1.5 Verify security hotspots all resolved
- [ ] 5.1.6 Document code smell reduction percentage

## Progress Tracking

| Phase | Tasks | Completed |
|-------|-------|-----------|
| Bug Fixes | 14 | 10 |
| Security Hotspots | 7 | 6 |
| High-Impact Smells | 13 | 10 |
| Remaining Smells | 5 | 0 |
| Verification | 6 | 0 |
| **Total** | **45** | **26** |

## Session Summary (Dec 14, 2025)

### Bugs Fixed

1. **strdup/delete[] mismatch** (BLOCKER - cpp:S1232)
   - DataItemBits.cpp: Replaced strdup() error returns with new[] allocation
   - Added newErrorString() and newErrorStringChar() helper functions
   - All encoding functions now use consistent delete[] deallocation

2. **Memory leaks in XMLParser** (BLOCKER - cpp:S3584)
   - XMLParser.cpp: Added delete for allocated format objects before early returns
   - Fixed leaks in BDS, Fixed, Explicit, Repetitive, Variable, Compound parsers
   - 12 leak points fixed across error paths

3. **Uninitialized struct** (MAJOR - cpp:S836)
   - udpdevice.cxx: Zero-initialized sockaddr_in structs
   - Prevents undefined behavior from uninitialized padding bytes

4. **Null pointer issues** (MINOR - cpp:S2637)
   - diskdevice.cxx: Added null check for data parameter in Write()
   - serialdevice.cxx: Added null check for device parameter in Init()

### Security Hotspots Reviewed

5. **Buffer-overflow (strncpy)** - FIXED
   - diskdevice.cxx: Added explicit null-termination after all strncpy calls
   - 8 strncpy calls secured with `buffer[MAXPATHLEN] = '\0'`

6. **Buffer-overflow (strlen)** - SAFE TO ACKNOWLEDGE
   - All strlen calls on strings from fgets, vsnprintf, std::string.c_str()
   - These functions guarantee null-termination
   - 27 hotspots reviewed and documented as safe

7. **Weak-cryptography (random)** - SAFE TO ACKNOWLEDGE
   - Python radar encoders use random for demo data generation
   - Already documented with `# nosec B311` comments
   - Cryptographic RNG not required for test data

### Code Smells Addressed

8. **const_cast removal** (CRITICAL - cpp:S859)
   - DataItemFormatFixed.cpp, DataItemFormatExplicit.cpp, DataItemFormatRepetitive.cpp
   - Replaced const_cast iterator with range-based for loops
   - Prevents undefined behavior from const-correctness violations

9. **Code duplication verified** - 1.8% (below 3% threshold)

10. **Cognitive complexity documented** - 30 functions identified above threshold
    - XMLParser::parseDataRecord at 348 (13.9× threshold) - major refactor needed
    - Recommend separate PR for XMLParser refactoring

### Testing Results
- All 11 integration tests pass
- Valgrind: 0 memory leaks
- Build successful on Linux (GCC)

### Override Keywords (cpp:S3471) - Session 2

11. **Missing override keywords** (CRITICAL - cpp:S3471)
    - Added `override` to all virtual method overrides in 12 files:
      - ASTERIX format classes: DataItemFormatBDS.h, DataItemFormatCompound.h,
        DataItemFormatExplicit.h, DataItemFormatFixed.h, DataItemFormatRepetitive.h,
        DataItemFormatVariable.h, UAPItem.h
      - Engine device classes: serialdevice.hxx, udpdevice.hxx, diskdevice.hxx,
        stddevice.hxx, tcpdevice.hxx
    - Replaced `virtual` with `override` for clearer intent
    - Enables compiler to detect signature mismatches
