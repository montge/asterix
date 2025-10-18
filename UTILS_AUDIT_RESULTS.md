# Utils.cpp Audit Results

## Summary
Utils.cpp has **90.6% coverage** (29/32 lines covered), NOT the 14% initially suspected.

## Uncovered Lines (3 lines total)
All uncovered lines are **legacy error handling paths** for vsnprintf failures:

### Line 30-31: format_arg_list() error path
```cpp
if (size < 0) {
    delete[] buffer;  // <-- UNCOVERED
    return std::string();  // <-- UNCOVERED
}
```
**Why uncovered:** vsnprintf only returns negative on very old glibc (< 2.0.6).
Modern systems always return the number of bytes that would have been written.

### Line 45: get_format_len() error path
```cpp
if (size < 0) {
    size = 4096;  // <-- UNCOVERED
}
```
**Why uncovered:** Same reason - legacy glibc < 2.0.6 compatibility code.

## Conclusion
- Utils.cpp coverage is **excellent** (90.6%)
- The 3 uncovered lines are untestable on modern systems
- **Not a priority** for coverage improvement

## Real Coverage Gaps (Lowest Coverage Files)
Files with **much lower** coverage that should be prioritized:

| File | Coverage | Lines Covered | Priority |
|------|----------|--------------|----------|
| **DataRecord.cpp** | 5.3% | 95/? | **CRITICAL** |
| **DataItemBits.cpp** | 7.2% | 250/? | **CRITICAL** |
| **DataBlock.cpp** | 11.8% | 34/? | **HIGH** |
| **DataItemFormatCompound.cpp** | 13.9% | 72/? | **HIGH** |
| **DataItem.cpp** | 15.2% | 33/? | **HIGH** |
| **DataItemFormatVariable.cpp** | 20.0% | 50/? | **MEDIUM** |
| **DataItemFormatFixed.cpp** | 20.9% | 67/? | **MEDIUM** |

## Recommendation
**Shift focus from Utils.cpp to critical files above.**

Testing DataRecord.cpp and DataItemBits.cpp could potentially gain +10-15 percentage points
since they have large line counts and very low coverage.
