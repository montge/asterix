# Language Binding Guidelines

**Version:** 1.0
**Last Updated:** 2025-11-06
**Status:** Active Development Guidelines

---

## Table of Contents

1. [Overview](#overview)
2. [Core Principles](#core-principles)
3. [Required API Surface](#required-api-surface)
4. [FFI Boundary Design](#ffi-boundary-design)
5. [Testing Requirements](#testing-requirements)
6. [Security Requirements](#security-requirements)
7. [Documentation Requirements](#documentation-requirements)
8. [CI/CD Integration](#cicd-integration)
9. [Language-Specific Patterns](#language-specific-patterns)
10. [Reference Implementations](#reference-implementations)

---

## Overview

This document provides comprehensive guidelines for creating new language bindings for the ASTERIX decoder. All bindings share the same C++ core parser but expose idiomatic APIs for their target languages.

**Current Bindings:**
- C++ (native) - High-performance CLI
- Python 3.10-3.14 - Simple, intuitive API
- Rust 1.87+ - Type-safe, memory-safe bindings

**Planned Bindings:**
- Node.js (N-API/NAPI-RS)
- Go (CGO)
- Java (JNI)
- C# (.NET Interop)

---

## Core Principles

### 1. Safety First

All language bindings MUST implement a validated FFI boundary layer:

```
┌─────────────────────────────────────┐
│   Idiomatic Language API            │  ← User-facing, language-specific
│   (ergonomic, safe, convenient)     │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│   FFI Boundary Layer                │  ← VALIDATION LAYER (CRITICAL)
│   - Input validation                │
│   - Bounds checking                 │
│   - Type conversion                 │
│   - Error mapping                   │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│   C++ Core Parser                   │  ← Shared parsing engine
│   (libasterix)                      │
└─────────────────────────────────────┘
```

**Key Safety Requirements:**
- Validate ALL inputs at FFI boundary
- Check buffer bounds before passing to C++
- Prevent integer overflows in size/offset calculations
- Map C++ errors to language-specific exceptions
- No silent failures - all errors propagated

### 2. Zero-Copy Where Possible

Minimize data copying across FFI boundaries:

- Use buffer views/slices for input data (don't copy unless necessary)
- Return owned data structures (let language runtime manage lifetime)
- Avoid double-serialization (e.g., C++ → JSON → language struct)

**Example (Rust - zero-copy input):**
```rust
pub fn parse(data: &[u8]) -> Result<Vec<AsterixRecord>> {
    // Pass slice pointer directly to C++ (zero-copy input)
    unsafe { ffi::asterix_parse(data.as_ptr(), data.len()) }
    // Return owned Vec (Rust manages memory)
}
```

### 3. Idiomatic APIs

Each binding should feel natural to developers in that language:

| Language | Idiom | Example |
|----------|-------|---------|
| Python | Dicts, keyword args | `parse(data, verbose=True)` |
| Rust | Result types, builders | `Parser::new().build()?` |
| JavaScript | Promises, callbacks | `await parse(buffer)` |
| Go | Error values, channels | `records, err := Parse(data)` |
| Java | Exceptions, streams | `parser.parse(bytes).stream()` |

### 4. Comprehensive Error Handling

Errors should provide actionable information:

```python
# BAD: Generic error
raise RuntimeError("Parse failed")

# GOOD: Detailed error with context
raise AsterixParseError(
    offset=1024,
    category=48,
    message="Invalid FSPEC: unexpected FX bit",
    raw_data=hexdump(data[1024:1040])
)
```

---

## Required API Surface

All language bindings MUST provide these core APIs:

### 1. Initialization

**Purpose:** Load ASTERIX category definitions

```
init_default()                    - Load all default categories
init_from_path(config_dir)        - Load from custom directory
load_category(xml_file)           - Load single category file
is_category_defined(category)     - Check if category is loaded
```

**Python Example:**
```python
import asterix

# Auto-initializes on import
asterix.init("custom/config/path")
asterix.load_category("asterix_cat099.xml")
```

**Rust Example:**
```rust
use asterix::Parser;

let parser = Parser::new()
    .init_default()?
    .add_category("asterix_cat099.xml")?
    .build()?;
```

### 2. Parsing

**Purpose:** Parse ASTERIX data from bytes

```
parse(data: bytes) -> Vec<Record>
parse_with_offset(data, offset, count) -> ParseResult
```

**Return Type:**
```
Record {
    category: u8,
    length: u32,
    timestamp_ms: u64,
    crc: u32,
    hex_data: String,
    items: Map<String, DataItem>
}

ParseResult {
    records: Vec<Record>,
    bytes_consumed: usize,
    remaining_blocks: usize
}
```

**Python Example:**
```python
data = open("sample.pcap", "rb").read()
records = asterix.parse(data)

for record in records:
    print(f"Category {record['category']}: {len(record['items'])} items")
```

**Rust Example:**
```rust
let data = std::fs::read("sample.pcap")?;
let options = ParseOptions::default();
let records = parser.parse(&data, &options)?;

for record in records {
    println!("Category {}: {} items", record.category, record.items.len());
}
```

### 3. Metadata/Description

**Purpose:** Get human-readable descriptions

```
describe(category, item, field, value) -> String
```

**Example:**
```python
# Get category description
desc = asterix.describe(48, None, None, None)
# "Monoradar Target Reports"

# Get item description
desc = asterix.describe(48, "010", None, None)
# "Data Source Identifier"

# Get field value description
desc = asterix.describe(48, "010", "SAC", "7")
# "System Area Code: 7"
```

### 4. Configuration

**Purpose:** Control parser behavior

```
set_verbose(enabled: bool)
set_log_level(level: LogLevel)
get_log_level() -> LogLevel
```

**Rust Example:**
```rust
use asterix::{set_log_level, LogLevel};

set_log_level(LogLevel::Debug);  // Enable verbose output
let records = parser.parse(&data, &options)?;
set_log_level(LogLevel::Silent); // Silence output
```

---

## FFI Boundary Design

### Input Validation Checklist

ALL FFI functions MUST validate inputs:

```c
extern "C" int asterix_parse_safe(
    const uint8_t* data,
    size_t data_len,
    AsterixRecord** out_records,
    size_t* out_count,
    char* error_buffer,
    size_t error_buffer_size)
{
    // 1. NULL POINTER CHECKS
    if (!data || !out_records || !out_count) {
        snprintf(error_buffer, error_buffer_size, "Null pointer argument");
        return ERROR_INVALID_ARGUMENT;
    }

    // 2. EMPTY INPUT CHECK
    if (data_len == 0) {
        snprintf(error_buffer, error_buffer_size, "Empty input data");
        return ERROR_EMPTY_INPUT;
    }

    // 3. SIZE LIMITS
    if (data_len > MAX_MESSAGE_SIZE) {
        snprintf(error_buffer, error_buffer_size,
            "Input too large: %zu bytes (max %zu)", data_len, MAX_MESSAGE_SIZE);
        return ERROR_INPUT_TOO_LARGE;
    }

    // 4. CALL CORE PARSER (after validation)
    return asterix_parse_internal(data, data_len, out_records, out_count);
}
```

### Error Code Mapping

Map C++ error codes to language-specific exceptions:

**C++ Error Codes:**
```cpp
enum AsterixError {
    OK = 0,
    ERROR_EMPTY_INPUT = 1,
    ERROR_INVALID_CATEGORY = 2,
    ERROR_TRUNCATED_MESSAGE = 3,
    ERROR_BUFFER_OVERFLOW = 4,
    ERROR_INVALID_FSPEC = 5,
    ERROR_MALFORMED_DATA = 6,
    ERROR_OUT_OF_MEMORY = 7,
    ERROR_INVALID_ARGUMENT = 8,
};
```

**Python Mapping:**
```python
class AsterixError(Exception):
    """Base exception for ASTERIX decoder"""
    pass

class ParseError(AsterixError):
    """Parse-time error with offset context"""
    def __init__(self, offset, message):
        self.offset = offset
        self.message = message
        super().__init__(f"Parse error at offset {offset}: {message}")

class InvalidDataError(AsterixError):
    """Invalid input data"""
    pass

# Map C++ error codes to Python exceptions
ERROR_MAP = {
    1: InvalidDataError("Empty input data"),
    2: InvalidDataError("Invalid ASTERIX category"),
    3: ParseError,
    4: ParseError,
    # ...
}
```

**Rust Mapping:**
```rust
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum AsterixError {
    EmptyInput,
    InvalidCategory(u8),
    ParseError { offset: usize, message: String },
    BufferOverflow { offset: usize },
    InvalidFspec { offset: usize },
    MalformedData(String),
    OutOfMemory,
    InvalidArgument(String),
    IOError(String),
}

impl std::error::Error for AsterixError {}

impl std::fmt::Display for AsterixError {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            AsterixError::EmptyInput => write!(f, "Empty input data"),
            AsterixError::InvalidCategory(cat) => write!(f, "Invalid category: {}", cat),
            AsterixError::ParseError { offset, message } =>
                write!(f, "Parse error at offset {}: {}", offset, message),
            // ...
        }
    }
}
```

### Memory Management

**Ownership Rules:**

1. **Input buffers:** Borrowed (FFI receives pointer + length)
2. **Output data:** Owned by language runtime (C++ allocates, language frees)
3. **Error strings:** C++ allocates with malloc(), language calls free()

**Python Example (C Extension):**
```c
static PyObject* asterix_parse(PyObject* self, PyObject* args) {
    const char* buffer;
    Py_ssize_t length;

    // Parse args (borrows buffer, does NOT copy)
    if (!PyArg_ParseTuple(args, "y#", &buffer, &length)) {
        return NULL;
    }

    // Validate
    if (length == 0) {
        PyErr_SetString(PyExc_ValueError, "Empty input data");
        return NULL;
    }

    // Call C++ (buffer is borrowed)
    AsterixRecord* records = NULL;
    size_t count = 0;
    int result = asterix_parse_safe(
        (const uint8_t*)buffer, length, &records, &count, error_buf, 1024);

    if (result != 0) {
        PyErr_SetString(PyExc_RuntimeError, error_buf);
        return NULL;
    }

    // Convert to Python objects (Python now owns the data)
    PyObject* list = PyList_New(count);
    for (size_t i = 0; i < count; i++) {
        PyObject* record = build_record_object(&records[i]);
        PyList_SET_ITEM(list, i, record);
    }

    // Free C++ allocated memory
    asterix_free_records(records, count);

    return list;
}
```

**Rust Example (CXX Bridge):**
```rust
#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        fn asterix_parse(data: *const u8, len: usize) -> *mut AsterixDataWrapper;
        fn asterix_free_data(ptr: *mut AsterixDataWrapper);
    }
}

pub fn parse(data: &[u8]) -> Result<Vec<AsterixRecord>> {
    unsafe {
        // Borrow input (zero-copy)
        let data_ptr = ffi::asterix_parse(data.as_ptr(), data.len());

        if data_ptr.is_null() {
            return Err(AsterixError::NullPointer);
        }

        // Convert to Rust types (copies data, Rust now owns)
        let records = convert_asterix_data(data_ptr)?;

        // Free C++ memory
        ffi::asterix_free_data(data_ptr);

        Ok(records)
    }
}
```

---

## Testing Requirements

All language bindings MUST meet these testing requirements:

### 1. Test Coverage: >80%

**Metrics:**
- Line coverage: >80%
- Branch coverage: >75%
- Function coverage: >90%

**Tools:**
- Python: `coverage.py`, `pytest-cov`
- Rust: `cargo-tarpaulin`, `cargo-llvm-cov`
- Node.js: `nyc`, `jest --coverage`
- Go: `go test -cover`

**Example (Python):**
```bash
pytest --cov=asterix --cov-report=html --cov-report=term-missing
# Fails CI if coverage < 80%
coverage report --fail-under=80
```

### 2. Unit Tests

Test individual functions in isolation:

```python
# tests/test_ffi_validation.py
import pytest
import asterix

def test_parse_empty_input():
    """Empty input should raise ValueError"""
    with pytest.raises(ValueError, match="Empty input"):
        asterix.parse(b"")

def test_parse_invalid_category():
    """Invalid category should raise ValueError"""
    with pytest.raises(ValueError, match="Invalid category"):
        asterix.describe(0, None, None, None)  # Category 0 is invalid

def test_parse_buffer_overflow():
    """Offset beyond buffer should raise ParseError"""
    data = b"\x30\x00\x10" * 10  # 30 bytes
    with pytest.raises(asterix.ParseError):
        asterix.parse_with_offset(data, offset=50, count=1)
```

### 3. Integration Tests

Test end-to-end parsing with real ASTERIX data:

```python
# tests/test_integration.py
import asterix
import os

def test_parse_cat048_pcap():
    """Parse real CAT048 PCAP file"""
    test_file = "tests/data/cat048_sample.pcap"
    data = open(test_file, "rb").read()

    records = asterix.parse(data)

    assert len(records) > 0
    assert all(r['category'] == 48 for r in records)
    assert all('items' in r for r in records)

def test_incremental_parsing():
    """Parse large file incrementally"""
    test_file = "tests/data/large_file.asterix"
    data = open(test_file, "rb").read()

    all_records = []
    offset = 0

    while offset < len(data):
        result = asterix.parse_with_offset(data, offset, count=100)
        all_records.extend(result['records'])
        offset = result['bytes_consumed']

        if result['remaining_blocks'] == 0:
            break

    assert len(all_records) > 0
```

### 4. Property-Based Tests

Use fuzzing/property testing to discover edge cases:

**Python (Hypothesis):**
```python
from hypothesis import given, strategies as st
import asterix

@given(st.binary(min_size=1, max_size=1024))
def test_parse_never_crashes(data):
    """Parser should never crash, even on random input"""
    try:
        records = asterix.parse(data)
        # If parse succeeds, records should be valid
        assert isinstance(records, list)
    except (ValueError, RuntimeError):
        # Expected errors are OK
        pass
```

**Rust (proptest):**
```rust
use proptest::prelude::*;

proptest! {
    #[test]
    fn parse_never_panics(data: Vec<u8>) {
        // Parser should never panic
        let _ = parse(&data, ParseOptions::default());
    }

    #[test]
    fn offset_bounds_checked(data: Vec<u8>, offset in 0usize..10000) {
        // Out-of-bounds offset should return error, not panic
        if offset >= data.len() {
            assert!(parse_with_offset(&data, offset, 1, ParseOptions::default()).is_err());
        }
    }
}
```

### 5. Memory Leak Tests

Verify no memory leaks with repeated parsing:

**Python:**
```python
import gc
import tracemalloc

def test_no_memory_leaks():
    """Repeated parsing should not leak memory"""
    data = open("tests/data/sample.pcap", "rb").read()

    tracemalloc.start()
    baseline = tracemalloc.get_traced_memory()[0]

    # Parse 1000 times
    for _ in range(1000):
        records = asterix.parse(data)
        del records
        gc.collect()

    current = tracemalloc.get_traced_memory()[0]
    tracemalloc.stop()

    # Memory should not grow significantly (allow 10% variance)
    assert current < baseline * 1.1
```

**Rust:**
```bash
# Use Valgrind/ASAN for leak detection
RUSTFLAGS="-Z sanitizer=address" cargo test
valgrind --leak-check=full cargo test
```

---

## Security Requirements

All language bindings MUST pass security audits:

### 1. Static Analysis

**Required Tools:**

| Language | Tool | Purpose |
|----------|------|---------|
| Python | Bandit | Security linter |
| Python | Safety | Dependency vulnerability scanner |
| Rust | Clippy | Linter with security checks |
| Rust | cargo-audit | Dependency vulnerability scanner |
| Node.js | ESLint (security plugin) | Security linter |
| C++ | CodeQL | Semantic code analysis |

**Python Example:**
```bash
# Run Bandit (CI fails on HIGH severity issues)
bandit -r asterix/ -f json -o bandit-report.json

# Check dependencies for known vulnerabilities
safety check --json
```

**Rust Example:**
```bash
# Run Clippy with strict lints
cargo clippy --all-targets --all-features -- -D warnings

# Audit dependencies
cargo audit
```

### 2. Fuzzing

Integrate fuzzing for FFI boundary validation:

**Python (Atheris - libFuzzer for Python):**
```python
import atheris
import sys
import asterix

@atheris.instrument_func
def TestOneInput(data):
    """Fuzz parser with random input"""
    try:
        asterix.parse(bytes(data))
    except (ValueError, RuntimeError):
        pass  # Expected errors

atheris.Setup(sys.argv, TestOneInput)
atheris.Fuzz()
```

**Rust (cargo-fuzz):**
```rust
#![no_main]
use libfuzzer_sys::fuzz_target;
use asterix::{parse, ParseOptions};

fuzz_target!(|data: &[u8]| {
    let _ = parse(data, ParseOptions::default());
});
```

**Run Fuzzing (CI nightly):**
```bash
# Python
python -m atheris.fuzz_harness fuzz_parse.py

# Rust
cargo fuzz run fuzz_parse -- -max_total_time=3600
```

### 3. ASAN/MSAN/UBSAN

Run tests with sanitizers enabled:

**Rust:**
```bash
# Address Sanitizer (memory safety)
RUSTFLAGS="-Z sanitizer=address" cargo test

# Memory Sanitizer (uninitialized reads)
RUSTFLAGS="-Z sanitizer=memory" cargo test

# Undefined Behavior Sanitizer
RUSTFLAGS="-Z sanitizer=undefined" cargo test
```

**Python (C extension):**
```bash
# Compile with ASAN
CC="clang -fsanitize=address" python setup.py build_ext --inplace

# Run tests with ASAN
ASAN_OPTIONS=detect_leaks=1 python -m pytest
```

### 4. CodeQL Analysis

Integrate GitHub CodeQL for semantic analysis:

**.github/workflows/codeql.yml:**
```yaml
name: CodeQL Security Analysis

on:
  push:
    branches: [master]
  pull_request:
    branches: [master]

jobs:
  analyze:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v3

    - name: Initialize CodeQL
      uses: github/codeql-action/init@v2
      with:
        languages: python, cpp

    - name: Build
      run: python setup.py build_ext --inplace

    - name: Perform CodeQL Analysis
      uses: github/codeql-action/analyze@v2
```

---

## Documentation Requirements

All language bindings MUST provide comprehensive documentation:

### 1. API Reference

**Auto-generated from docstrings/comments:**

**Python (Sphinx):**
```python
def parse(data: bytes, verbose: bool = False) -> List[Dict]:
    """Parse ASTERIX data from bytes.

    Args:
        data: Raw ASTERIX bytes (raw, PCAP, HDLC, etc.)
        verbose: Enable verbose output (default: False)

    Returns:
        List of parsed ASTERIX records, each record is a dict with:
        - category (int): ASTERIX category (1-255)
        - length (int): Data block length in bytes
        - timestamp_ms (int): Timestamp in milliseconds
        - items (dict): Parsed data items

    Raises:
        ValueError: If input data is empty or invalid
        RuntimeError: If parser encounters an error

    Example:
        >>> data = open("sample.pcap", "rb").read()
        >>> records = asterix.parse(data)
        >>> print(records[0]['category'])
        48
    """
```

**Rust (rustdoc):**
```rust
/// Parse ASTERIX data from bytes.
///
/// # Arguments
///
/// * `data` - Raw ASTERIX bytes (can be raw binary, PCAP, HDLC, etc.)
/// * `options` - Parsing configuration options
///
/// # Returns
///
/// A vector of parsed ASTERIX records on success.
///
/// # Errors
///
/// Returns an error if:
/// - Input data is empty ([`AsterixError::EmptyInput`])
/// - Requested category is not defined ([`AsterixError::InvalidCategory`])
/// - Parser encounters malformed data ([`AsterixError::ParseError`])
///
/// # Example
///
/// ```no_run
/// use asterix::{parse, ParseOptions};
///
/// let data = std::fs::read("sample.pcap")?;
/// let options = ParseOptions::default();
/// let records = parse(&data, options)?;
///
/// for record in records {
///     println!("Category {}: {} items", record.category, record.items.len());
/// }
/// # Ok::<(), Box<dyn std::error::Error>>(())
/// ```
pub fn parse(data: &[u8], options: ParseOptions) -> Result<Vec<AsterixRecord>> {
    // ...
}
```

### 2. Tutorials

**Quickstart guide for each binding:**

```markdown
# Python Quickstart

## Installation

```bash
pip install asterix_decoder
```

## Basic Usage

```python
import asterix

# Parse ASTERIX data
data = open("sample.pcap", "rb").read()
records = asterix.parse(data)

# Print parsed records
for record in records:
    print(f"Category {record['category']}: {len(record['items'])} items")
    for item_name, item_data in record['items'].items():
        print(f"  {item_name}: {item_data}")
```

## Advanced Usage

### Incremental Parsing

```python
# Parse large files incrementally
offset = 0
while offset < len(data):
    result = asterix.parse_with_offset(data, offset, count=100)
    for record in result['records']:
        process(record)
    offset = result['bytes_consumed']
```

### Custom Categories

```python
# Load custom category definition
asterix.load_category("custom_category.xml")
```
```

### 3. Migration Guide

**From other bindings:**

```markdown
# Migrating from Python to Rust

## Python
```python
import asterix
data = open("file.pcap", "rb").read()
records = asterix.parse(data)
```

## Rust Equivalent
```rust
use asterix::{Parser, ParseOptions};

let data = std::fs::read("file.pcap")?;
let parser = Parser::new().init_default()?.build()?;
let options = ParseOptions::default();
let records = parser.parse(&data, &options)?;
```

## Key Differences

| Feature | Python | Rust |
|---------|--------|------|
| Initialization | Auto on import | Explicit `init_default()` |
| Error handling | Exceptions | `Result<T, E>` |
| Data ownership | GC managed | Explicit ownership |
| Type safety | Runtime | Compile-time |
```

---

## CI/CD Integration

All language bindings MUST integrate with CI/CD:

### 1. Build Matrix

Test on multiple OS/compiler/runtime versions:

**.github/workflows/binding-test.yml:**
```yaml
name: Language Binding Tests

on: [push, pull_request]

jobs:
  python:
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]
        python-version: ['3.10', '3.11', '3.12', '3.13', '3.14']

    runs-on: ${{ matrix.os }}

    steps:
    - uses: actions/checkout@v3

    - name: Set up Python
      uses: actions/setup-python@v4
      with:
        python-version: ${{ matrix.python-version }}

    - name: Install dependencies
      run: |
        pip install pytest pytest-cov hypothesis

    - name: Build extension
      run: python setup.py build_ext --inplace

    - name: Run tests
      run: pytest --cov=asterix --cov-report=xml

    - name: Upload coverage
      uses: codecov/codecov-action@v3

  rust:
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]
        rust: [stable, beta, nightly]

    runs-on: ${{ matrix.os }}

    steps:
    - uses: actions/checkout@v3

    - name: Install Rust
      uses: actions-rs/toolchain@v1
      with:
        toolchain: ${{ matrix.rust }}

    - name: Build
      run: cargo build --all-features

    - name: Run tests
      run: cargo test --all-features

    - name: Run clippy
      run: cargo clippy -- -D warnings
```

### 2. Release Automation

Automate package publishing:

**Python (PyPI):**
```yaml
- name: Build wheels
  run: python -m build

- name: Publish to PyPI
  uses: pypa/gh-action-pypi-publish@release/v1
  with:
    password: ${{ secrets.PYPI_API_TOKEN }}
```

**Rust (crates.io):**
```yaml
- name: Publish to crates.io
  run: cargo publish --token ${{ secrets.CARGO_REGISTRY_TOKEN }}
```

### 3. Performance Regression Detection

Track performance over time:

```yaml
- name: Run benchmarks
  run: |
    cargo bench --bench benchmarks -- --save-baseline master

- name: Compare with baseline
  run: |
    cargo bench --bench benchmarks -- --baseline master
```

---

## Language-Specific Patterns

### Python (C Extension)

**Structure:**
```
asterix/
├── __init__.py           # User-facing API
├── _asterix.so           # C extension (built from src/python/)
└── py.typed              # PEP 561 type stub marker

src/python/
├── asterix_wrapper.c     # Python C API wrapper
├── python_parser.cpp     # C++ bridge
└── python_parser.h       # Bridge header
```

**Type Stubs (PEP 484):**
```python
# asterix/__init__.pyi
from typing import Dict, List, Optional

def parse(data: bytes, verbose: bool = False) -> List[Dict]: ...
def parse_with_offset(
    data: bytes,
    offset: int,
    count: int,
    verbose: bool = False
) -> Dict: ...
def describe(
    category: int,
    item: Optional[str] = None,
    field: Optional[str] = None,
    value: Optional[str] = None
) -> str: ...
```

### Rust (CXX Bridge)

**Structure:**
```
asterix-rs/
├── Cargo.toml
├── build.rs              # Build script (compiles C++ via CMake)
├── src/
│   ├── lib.rs           # Public API
│   ├── ffi.rs           # CXX bridge
│   ├── parser.rs        # Safe parser wrapper
│   ├── types.rs         # Rust data types
│   └── error.rs         # Error types
├── tests/               # Integration tests
├── examples/            # Example programs
└── benches/             # Benchmarks
```

**CXX Bridge Pattern:**
```rust
#[cxx::bridge(namespace = "asterix")]
mod ffi {
    unsafe extern "C++" {
        include!("ffi_wrapper.h");

        type AsterixDataWrapper;

        fn asterix_init(config_dir: &str) -> bool;
        fn asterix_parse(data: *const u8, len: usize) -> *mut AsterixDataWrapper;
        fn asterix_free_data(ptr: *mut AsterixDataWrapper);
    }
}
```

### Node.js (N-API)

**Structure:**
```
asterix-node/
├── package.json
├── binding.gyp          # Native addon build config
├── lib/
│   └── index.js        # JavaScript API
├── src/
│   ├── addon.cpp       # N-API wrapper
│   └── parser_wrapper.cpp
└── test/
    └── test.js
```

**N-API Pattern:**
```cpp
Napi::Value Parse(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Validate arguments
    if (info.Length() < 1 || !info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Expected Buffer").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Get buffer
    Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
    const uint8_t* data = buffer.Data();
    size_t length = buffer.Length();

    // Call C++ parser
    AsterixRecord* records = NULL;
    size_t count = 0;
    int result = asterix_parse_safe(data, length, &records, &count, error_buf, 1024);

    if (result != 0) {
        Napi::Error::New(env, error_buf).ThrowAsJavaScriptException();
        return env.Null();
    }

    // Convert to JavaScript objects
    Napi::Array array = Napi::Array::New(env, count);
    for (size_t i = 0; i < count; i++) {
        array[i] = ConvertRecord(env, &records[i]);
    }

    asterix_free_records(records, count);
    return array;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("parse", Napi::Function::New(env, Parse));
    return exports;
}

NODE_API_MODULE(asterix, Init)
```

---

## Reference Implementations

### Python Binding

**File:** `src/python/asterix_wrapper.c`

**Key Features:**
- Comprehensive input validation (Issues #29, CRITICAL-002)
- Proper Python exception mapping
- Memory safety (refcounting)
- Integration with C++ core via `python_parser.cpp`

**Example Validation:**
```c
static PyObject* asterix_parse_with_offset(PyObject* self, PyObject* args) {
    const char* buffer;
    Py_ssize_t length;
    unsigned int offset;
    unsigned int blocks_count;
    int verbose = 0;

    if (!PyArg_ParseTuple(args, "y#II|p", &buffer, &length, &offset, &blocks_count, &verbose)) {
        return NULL;
    }

    // CRITICAL-002 FIX: Validate offset bounds
    if (offset >= (unsigned int)length) {
        PyErr_Format(PyExc_ValueError,
            "Offset %u exceeds data length %zd", offset, length);
        return NULL;
    }

    // Validate blocks_count (HIGH-002 equivalent)
    if (blocks_count > 10000) {
        PyErr_Format(PyExc_ValueError,
            "blocks_count %u exceeds maximum (10000)", blocks_count);
        return NULL;
    }

    return python_parse_with_offset(
        (const unsigned char*)buffer, length, offset, blocks_count, verbose);
}
```

### Rust Binding

**File:** `asterix-rs/src/ffi.rs`

**Key Features:**
- Type-safe FFI via CXX crate
- Comprehensive input validation (Issues #29, CRITICAL-004, CRITICAL-005)
- Zero unsafe code in public API
- Idiomatic Rust error handling (Result types)

**Example Validation:**
```rust
pub fn parse(data: &[u8], options: ParseOptions) -> Result<Vec<AsterixRecord>> {
    // CRITICAL-005 FIX: Validate input data length
    if data.is_empty() {
        return Err(AsterixError::InvalidData("Empty input data".to_string()));
    }

    if data.len() > MAX_ASTERIX_MESSAGE_SIZE {
        return Err(AsterixError::InvalidData(format!(
            "Input data too large: {} bytes (maximum {} bytes)",
            data.len(), MAX_ASTERIX_MESSAGE_SIZE
        )));
    }

    unsafe {
        let data_ptr = ffi::ffi::asterix_parse(data.as_ptr(), data.len(), options.verbose);

        if data_ptr.is_null() {
            return Err(AsterixError::NullPointer(
                "C++ parser returned null (check if ASTERIX is initialized)".to_string(),
            ));
        }

        let result = convert_asterix_data(data_ptr, &options);
        ffi::ffi::asterix_free_data(data_ptr);
        result
    }
}
```

---

## Checklist for New Bindings

Use this checklist when creating a new language binding:

- [ ] **FFI Boundary Layer**
  - [ ] Input validation (null checks, bounds, overflows)
  - [ ] Error code mapping to language exceptions
  - [ ] Memory ownership model documented
  - [ ] All FFI functions have validation

- [ ] **API Surface**
  - [ ] `init_default()` - Load default categories
  - [ ] `init_from_path()` - Load custom categories
  - [ ] `parse()` - Parse ASTERIX data
  - [ ] `parse_with_offset()` - Incremental parsing
  - [ ] `describe()` - Get metadata/descriptions
  - [ ] Idiomatic error handling

- [ ] **Testing**
  - [ ] Unit tests (>80% coverage)
  - [ ] Integration tests (real ASTERIX data)
  - [ ] Property-based tests (fuzzing)
  - [ ] Memory leak tests
  - [ ] CI/CD integration

- [ ] **Security**
  - [ ] Static analysis (linter)
  - [ ] Dependency scanning
  - [ ] ASAN/MSAN testing
  - [ ] Fuzz testing integrated
  - [ ] CodeQL analysis

- [ ] **Documentation**
  - [ ] API reference (auto-generated)
  - [ ] Quickstart tutorial
  - [ ] Example programs
  - [ ] Migration guide (from other bindings)

- [ ] **CI/CD**
  - [ ] Build matrix (OS/version)
  - [ ] Automated testing
  - [ ] Coverage reporting
  - [ ] Performance benchmarks
  - [ ] Release automation

---

## References

- **ARCHITECTURE.md:** Overall system architecture
- **PROTOCOL_INTEGRATION.md:** Adding new protocol adapters
- **CLAUDE.md:** Development guidelines
- **Issue #26:** Safety-critical design patterns
- **Issue #29:** FFI boundary security audit

---

**Document Version:** 1.0
**Last Updated:** 2025-11-06
**Maintainers:** ASTERIX Contributors
**License:** GPL-3.0-or-later
