# Rust Bindings Implementation Plan for ASTERIX Decoder

## Executive Summary

This document provides a comprehensive plan for adding production-grade Rust bindings to the ASTERIX decoder project. The plan leverages existing C++ architecture and Python bindings as reference implementations while introducing Rust-idiomatic safety guarantees.

**Project Scope:**
- Target audience: Rust developers in ATM/surveillance systems
- Integration method: FFI bindings using `cxx` crate for safe C++/Rust interop
- Timeline estimate: 8-12 weeks for full implementation with testing
- Deliverable: Published crate on crates.io with 90%+ API coverage

---

## 1. Technology Choices and Rationale

### 1.1 FFI Binding Framework: CXX vs Alternatives

#### Chosen: **CXX Crate** (Primary)
**Rationale:**
- **Type-safe interop**: Generates safe C++ function signatures from Rust definitions
- **Minimal runtime overhead**: No garbage collection, predictable performance
- **Compile-time verification**: Invalid C++/Rust combinations caught at compile time
- **Active maintenance**: Well-maintained by Dtolnay and the Rust community
- **Error handling**: Native Rust `Result<T, E>` type support for C++ exceptions
- **Existing ecosystem**: Used by major projects (Pinterest, AWS, Google)

**Advantages:**
- `cxx::bridge` macro ensures C++ function signatures match Rust declarations
- Automatic code generation (less manual bindgen configuration)
- Better compiler errors than raw FFI
- Built-in support for shared ownership (`cxx::SharedPtr<T>`)
- Thread-safe semantics by default

**Trade-offs:**
- Requires writing `bridge` declarations (more verbose than bindgen but safer)
- Limited to function signatures (not raw class layouts)
- Smaller ecosystem compared to bindgen

#### Alternative: **Bindgen** (Secondary Consideration)
**Why not primary:**
- Generates unsafe Rust code directly from C++ headers
- Requires manual safety wrappers for every function
- More error-prone but faster for simple C APIs
- Would be used in conjunction with cxx if needed for complex types

**When to use bindgen:**
- For direct memory access patterns in performance-critical paths
- Legacy code requiring direct struct access

#### Not Recommended: **SWIG** (Deprecated)
- Mature but higher maintenance burden
- Not designed for modern C++ (pre-C++11)
- Complex code generation

---

### 1.2 Build System Integration

#### Cargo + Custom Build Script (build.rs)

**Rationale:**
- Single canonical build system for Rust developers
- `build.rs` handles C++ compilation, mirroring existing CMake/Make
- No duplicate build configuration
- Cross-platform support (Linux, macOS, Windows)

**Implementation Strategy:**
```rust
// asterix_rs/build.rs
fn main() {
    // 1. Detect platform and compiler
    // 2. Compile C++ core with cmake-rs or cxx-build
    // 3. Link against system libexpat or statically compile
    // 4. Generate FFI declarations via cxx
}
```

**Advantages:**
- Single `cargo build` command works everywhere
- Hermetic builds (reproducible, no system-wide assumptions)
- cxx crate integrates directly with build.rs via `cxx-build`

**Alternative: Separate Build**
If C++ library maintained independently:
```toml
# Cargo.toml
[dependencies]
asterix-sys = { path = "../asterix-sys" }  # Raw FFI
```

This is NOT recommended as it couples versions and adds complexity.

---

### 1.3 Memory Model and Ownership

**Architecture Decision: Owned Parsed Data**

Given the ASTERIX parsing pipeline structure, we'll use owned data structures:

```
Input bytes → Parse → Owned Rust Vec<AsterixRecord> → Return to user
```

**Rationale:**
- **Safety**: No lifetime management required; clear ownership
- **Simplicity**: User doesn't need to understand C++ pointer semantics
- **Performance**: Reasonable for typical message sizes (512 bytes - 64KB)
- **Zero-copy: Not feasible** because:
  - C++ structures (std::list, std::map) have different layouts than Rust
  - Parsed data contains nested structures incompatible with C++ memory models

**Memory Strategy:**

1. **Input data**: Borrowed (`&[u8]`)
2. **Parsing**: Allocates in C++, marshals result to Rust
3. **Output**: Owned `Vec<AsterixRecord>` in Rust
4. **Cleanup**: Automatic via Rust RAII when parsed data dropped

**Performance Characteristics:**
- Parsing: ~100-500 KB/s (dominated by C++ DOM walks, not marshaling)
- Memory allocation: Single contiguous Rust Vec (no fragmentation)
- Cache locality: Good for iteration

---

### 1.4 Error Handling Strategy

**Approach: Result<T, AsterixError> Throughout**

```rust
#[derive(Debug, Clone)]
pub enum AsterixError {
    ParseError(String),
    InvalidCategory(u8),
    FileNotFound(String),
    InitializationError(String),
    IOError(String),
}

impl std::fmt::Display for AsterixError { ... }
impl std::error::Error for AsterixError { ... }

// FFI mapping:
// C++ exception → Rust Result::Err()
// NULL pointer → InvalidData error
// XML parse fail → InitializationError
```

**Exception Handling in FFI:**
- Use `cxx` exception translation
- All C++ exceptions → Rust error variants
- No panics (unwrap only in tests)

---

## 2. Implementation Architecture

### 2.1 Project Structure

```
asterix-rs/                          # New crate (workspace member)
├── Cargo.toml
├── build.rs                         # C++ compilation logic
├── src/
│   ├── lib.rs                      # Library root, public API
│   ├── ffi.rs                      # CXX bridge declarations (unsafe)
│   ├── parser.rs                   # High-level parse API (safe)
│   ├── data_types.rs               # Rust data structures
│   ├── error.rs                    # Error types
│   ├── config.rs                   # Configuration management
│   └── utils.rs                    # Helper functions
├── tests/
│   ├── integration_tests.rs        # Full parsing tests with real PCAP
│   ├── benchmark.rs                # Perf comparison vs C++/Python
│   └── fixtures/
│       ├── cat048.raw
│       ├── cat062.pcap
│       └── cat065.raw
├── examples/
│   ├── parse_pcap.rs              # PCAP file parsing
│   ├── multicast_receiver.rs       # UDP multicast parsing
│   └── real_time_decoder.rs        # Streaming pipeline
└── docs/
    ├── ARCHITECTURE.md             # Design decisions
    ├── PERFORMANCE.md              # Benchmarks vs Python/C++
    └── MIGRATION_GUIDE.md          # Porting from Python
```

**Workspace Structure:**

```
asterix/
├── Cargo.toml                      # Workspace root
├── src/                            # C++ source (unchanged)
├── asterix/                        # Python module (unchanged)
├── asterix-rs/                     # Rust bindings (NEW)
└── asterix-sys/                    # Optional: Raw FFI layer
```

### 2.2 CXX Bridge Declaration

**File: `asterix-rs/src/ffi.rs`**

```rust
// Minimal safe interface between C++ and Rust
#[cxx::bridge(namespace = "asterix")]
mod ffi {
    // Opaque C++ types
    unsafe extern "C++" {
        include!("asterix/AsterixDefinition.h");
        include!("asterix/AsterixData.h");

        // C++ types as opaque from Rust perspective
        type AsterixDefinition;
        type AsterixData;
        type DataBlock;
        type DataRecord;
        type DataItem;

        // Singleton instance
        unsafe fn GetAsterixDefinition() -> *mut AsterixDefinition;

        // Parse functions
        unsafe fn parse_asterix_data(
            data: &[u8],
            verbose: bool
        ) -> *mut AsterixData;

        // Configuration
        unsafe fn init_category(filename: &str) -> Result<()>;

        // Metadata
        unsafe fn describe(
            category: u8,
            item: Option<&str>,
            field: Option<&str>,
            value: Option<&str>
        ) -> String;
    }

    // Rust types for safe sharing
    extern "Rust" {
        type AsterixRecord;
        type DataField;
        type ParsedValue;
    }
}
```

**Considerations:**
- FFI layer is minimal and unsafe
- All conversion logic goes in `parser.rs` (safe, public API)
- Uses `cxx::UniquePtr` for automatic cleanup
- No raw `*mut` pointers exposed to users

---

### 2.3 Safe Rust API Design

**File: `asterix-rs/src/lib.rs`**

```rust
//! Rust bindings for ASTERIX decoder
//!
//! # Quick Start
//!
//! ```no_run
//! use asterix_rs::parse;
//!
//! let data = std::fs::read("sample.raw")?;
//! let records = parse(&data, Default::default())?;
//!
//! for record in records {
//!     println!("Category {}: {} items", record.category, record.items.len());
//! }
//! # Ok::<(), Box<dyn std::error::Error>>(())
//! ```

pub mod parser;
pub mod data_types;
pub mod error;
pub mod config;

pub use parser::{parse, parse_with_offset, ParseOptions};
pub use data_types::{AsterixRecord, DataItem, ParsedValue};
pub use error::AsterixError;
pub use config::AsterixConfig;

/// Initialize decoder with custom category definition
pub fn init_category(path: impl AsRef<std::path::Path>) -> Result<(), AsterixError> {
    // Implementation
}

/// Describe a category, item, field, or value
pub fn describe(
    category: u8,
    item: Option<&str>,
    field: Option<&str>,
    value: Option<&str>,
) -> Result<String, AsterixError> {
    // Implementation
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_cat048() {
        let data = include_bytes!("../tests/fixtures/cat048.raw");
        let records = parse(data, Default::default()).unwrap();
        assert!(!records.is_empty());
    }
}
```

---

### 2.4 Data Type Definitions

**File: `asterix-rs/src/data_types.rs`**

```rust
use std::collections::BTreeMap;
use serde::{Serialize, Deserialize};

/// A single ASTERIX record (one data block)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AsterixRecord {
    pub category: u8,
    pub length: u32,
    pub timestamp_ms: u64,
    pub crc: u32,
    pub hex_data: String,
    pub items: BTreeMap<String, DataItem>,
}

/// Individual data item within a record
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DataItem {
    pub description: Option<String>,
    pub fields: BTreeMap<String, ParsedValue>,
}

/// A single parsed value (leaf node in data hierarchy)
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(untagged)]
pub enum ParsedValue {
    Integer(i64),
    Float(f64),
    String(String),
    Boolean(bool),
    Bytes(Vec<u8>),
    // Nested structures (for compound items)
    Nested(Box<ParsedValue>),
}

impl ParsedValue {
    pub fn as_i64(&self) -> Option<i64> {
        match self {
            ParsedValue::Integer(v) => Some(*v),
            _ => None,
        }
    }

    pub fn as_f64(&self) -> Option<f64> {
        match self {
            ParsedValue::Float(v) => Some(*v),
            _ => None,
        }
    }
}

/// Options for parsing
#[derive(Debug, Clone, Default)]
pub struct ParseOptions {
    pub verbose: bool,
    pub filter_category: Option<u8>,
    pub max_records: Option<usize>,
}

/// Incremental parsing result
pub struct ParseResult {
    pub records: Vec<AsterixRecord>,
    pub bytes_consumed: usize,
    pub remaining_blocks: usize,
}
```

---

### 2.5 Error Types

**File: `asterix-rs/src/error.rs`**

```rust
use std::fmt;
use std::error::Error;

#[derive(Debug, Clone)]
pub enum AsterixError {
    /// Parsing failed at given offset with description
    ParseError {
        offset: usize,
        message: String,
    },

    /// Invalid category number
    InvalidCategory(u8),

    /// Configuration file not found
    ConfigNotFound(String),

    /// Initialization failed
    InitializationError(String),

    /// IO error
    IOError(String),

    /// Unexpected end of input
    UnexpectedEOF,

    /// Internal error (usually from C++ side)
    InternalError(String),
}

impl fmt::Display for AsterixError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            AsterixError::ParseError { offset, message } => {
                write!(f, "Parse error at offset {}: {}", offset, message)
            }
            AsterixError::InvalidCategory(cat) => {
                write!(f, "Invalid ASTERIX category: {}", cat)
            }
            // ... other variants
        }
    }
}

impl Error for AsterixError {}

pub type Result<T> = std::result::Result<T, AsterixError>;
```

---

## 3. Step-by-Step Implementation Phases

### Phase 1: Foundation (Weeks 1-2)

**Deliverables:**
- `Cargo.toml` with dependencies
- `build.rs` that successfully compiles C++ core
- FFI declarations for core parsing functions
- CI/CD setup

**Tasks:**
1. Create `Cargo.toml` with metadata
2. Implement `build.rs` to compile C++ files
3. Link against libexpat
4. Create minimal FFI bridge
5. Set up GitHub Actions for CI
6. Verify compilation on Linux, macOS (Windows in Phase 3)

**Code Example - Cargo.toml:**

```toml
[package]
name = "asterix_decoder"
version = "0.1.0"
edition = "2021"
authors = ["ASTERIX Contributors"]
license = "GPL-3.0-or-later"
description = "Rust bindings for ASTERIX ATM surveillance protocol decoder"
repository = "https://github.com/your-org/asterix"
documentation = "https://docs.rs/asterix_decoder"
categories = ["aerospace", "aviation", "parser"]

[dependencies]
cxx = "1.0"
serde = { version = "1.0", features = ["derive"], optional = true }
serde_json = { version = "1.0", optional = true }

[dev-dependencies]
criterion = "0.5"
rstest = "0.18"
tempfile = "3.8"

[build-dependencies]
cxx-build = "1.0"
cmake = "0.1"

[features]
default = ["serde"]
serde = ["dep:serde", "dep:serde_json"]

[[bench]]
name = "parser_benchmark"
harness = false
```

---

### Phase 2: Core Parsing API (Weeks 3-4)

**Deliverables:**
- Safe `parse()` and `parse_with_offset()` functions
- Complete data type definitions
- Error handling
- 50+ unit tests
- Documentation

**Tasks:**
1. Implement parser.rs with core parsing logic
2. Define all data types (Rust-idiomatic)
3. Write comprehensive error handling
4. Add docstring comments to all public APIs
5. Create unit tests for each data type
6. Test with sample PCAP/raw files

**Code Example - Parser Module:**

```rust
// asterix-rs/src/parser.rs
use crate::{ffi, data_types::*, error::*};
use std::collections::BTreeMap;

/// Parse raw ASTERIX data
///
/// # Arguments
/// * `data` - Raw ASTERIX bytes
/// * `options` - Parsing configuration
///
/// # Returns
/// Vector of parsed ASTERIX records
///
/// # Errors
/// Returns `AsterixError` if parsing fails
///
/// # Example
/// ```no_run
/// # use asterix_decoder::parse;
/// let data = b"\x30\x00\x30..."; // ASTERIX data
/// let records = parse(data, Default::default())?;
/// # Ok::<(), asterix_decoder::AsterixError>(())
/// ```
pub fn parse(data: &[u8], options: ParseOptions) -> Result<Vec<AsterixRecord>> {
    unsafe {
        // Call C++ parser
        let result_ptr = ffi::parse_asterix_data(
            data,
            options.verbose,
        );

        if result_ptr.is_null() {
            return Err(AsterixError::InternalError(
                "C++ parser returned NULL".to_string()
            ));
        }

        // Marshal C++ data to Rust structures
        marshal_asterix_data(result_ptr, &options)
    }
}

/// Parse with offset and block count (incremental)
pub fn parse_with_offset(
    data: &[u8],
    offset: usize,
    blocks_count: usize,
    options: ParseOptions,
) -> Result<ParseResult> {
    // Similar implementation with offset tracking
    todo!()
}

// Internal: Convert C++ structures to Rust types
fn marshal_asterix_data(
    ptr: *mut ffi::AsterixData,
    options: &ParseOptions,
) -> Result<Vec<AsterixRecord>> {
    let mut records = Vec::new();

    // Iterate over C++ data blocks
    // Convert each to Rust AsterixRecord
    // Handle nested structures recursively

    Ok(records)
}
```

---

### Phase 3: Advanced Features (Weeks 5-6)

**Deliverables:**
- Configuration management API
- Metadata/describe function
- Window (PCAP, HDLC, FINAL) format support
- 100+ additional tests
- Cross-platform builds (Windows support)

**Tasks:**
1. Implement `init_category()` for custom configurations
2. Implement `describe()` with full metadata lookups
3. Add PCAP format auto-detection
4. Add thread-safe global state handling
5. Windows compilation setup
6. macOS M1/M2 support

**Code Example - Config Module:**

```rust
// asterix-rs/src/config.rs
pub struct AsterixConfig {
    config_dir: std::path::PathBuf,
    custom_categories: Vec<String>,
}

impl AsterixConfig {
    pub fn new() -> Self {
        Self {
            config_dir: default_config_dir(),
            custom_categories: Vec::new(),
        }
    }

    /// Add custom category definition file
    pub fn add_category(&mut self, path: impl AsRef<std::path::Path>) -> Result<()> {
        let path = path.as_ref();
        if !path.exists() {
            return Err(AsterixError::ConfigNotFound(
                path.display().to_string()
            ));
        }

        unsafe {
            ffi::init_category(
                path.to_str().ok_or_else(||
                    AsterixError::InitializationError("Invalid path".to_string())
                )?
            )?;
        }

        self.custom_categories.push(path.display().to_string());
        Ok(())
    }
}

impl Default for AsterixConfig {
    fn default() -> Self {
        Self::new()
    }
}

fn default_config_dir() -> std::path::PathBuf {
    // Return path to bundled configs
    // Or environment variable ASTERIX_CONFIG_DIR
}
```

---

### Phase 4: Performance & Optimization (Weeks 7-8)

**Deliverables:**
- Benchmarks vs Python/C++ implementations
- Performance profiling (perf, flamegraph)
- Zero-copy optimizations where applicable
- Async streaming API (optional)
- Memory profiling

**Tasks:**
1. Create benchmark suite comparing implementations
2. Profile hot paths with perf/flamegraph
3. Optimize string allocations
4. Add optional async parsing via `tokio`
5. Implement streaming parser for files >1GB
6. Memory leak tests (valgrind, MSAN)

**Benchmark Example:**

```rust
// asterix-rs/benches/parser_benchmark.rs
use criterion::{black_box, criterion_group, criterion_main, Criterion};
use asterix_decoder::parse;

fn benchmark_parsing(c: &mut Criterion) {
    let data = include_bytes!("../tests/fixtures/cat048.pcap");

    c.bench_function("parse_cat048_raw", |b| {
        b.iter(|| {
            parse(black_box(data), Default::default()).unwrap()
        })
    });
}

criterion_group!(benches, benchmark_parsing);
criterion_main!(benches);
```

**Expected Performance:**
- Rust: ~95% of C++ (within margin of marshaling overhead)
- Rust: ~2-3x faster than Python (direct C++ calling vs Python overhead)

---

### Phase 5: Testing & Documentation (Weeks 9-10)

**Deliverables:**
- 90%+ test coverage
- Full API documentation with examples
- Migration guide from Python
- Integration test suite
- Performance comparison report

**Tasks:**
1. Property-based testing (proptest)
2. Fuzzing with cargo-fuzz
3. Integration tests with real PCAP files
4. Full rustdoc with examples
5. Write MIGRATION_GUIDE.md
6. Create examples for each major feature
7. Run clippy and rustfmt

**Documentation Structure:**

```
docs/
├── ARCHITECTURE.md          # Design decisions, module architecture
├── PERFORMANCE.md           # Benchmarks, optimization notes
├── MIGRATION_GUIDE.md       # Python → Rust porting
├── SAFETY.md                # Memory safety guarantees
├── ERROR_HANDLING.md        # Error types and recovery
└── EXAMPLES.md              # Common usage patterns
```

---

### Phase 6: Publication & CI/CD (Weeks 11-12)

**Deliverables:**
- Published on crates.io
- Documentation on docs.rs
- Automated CI/CD pipeline
- Security audit
- Release notes

**Tasks:**
1. Add CHANGELOG.md
2. Semantic versioning (0.1.0)
3. Create release checklist
4. Set up automated crates.io publishing
5. Configure docs.rs features
6. Security audit (cargo-audit)
7. GitHub releases with binaries

---

## 4. Detailed Code Examples

### 4.1 FFI Bridge Implementation

**Complete CXX bridge declaration:**

```rust
// asterix-rs/src/ffi.rs
#[cxx::bridge(namespace = "asterix_ffi")]
pub mod ffi {
    unsafe extern "C++" {
        include!("asterix-rs/src/ffi_wrapper.h");

        type AsterixDataWrapper;

        // Initialize with category definitions
        unsafe fn asterix_init(config_dir: &str) -> bool;

        // Parse raw bytes
        unsafe fn asterix_parse(
            data: *const u8,
            len: usize,
            verbose: bool
        ) -> *mut AsterixDataWrapper;

        // Incremental parsing
        unsafe fn asterix_parse_offset(
            data: *const u8,
            len: usize,
            offset: u32,
            blocks: u32,
            verbose: bool
        ) -> *mut AsterixDataWrapper;

        // Metadata
        unsafe fn asterix_describe(
            category: u8,
            item: *const u8,
            item_len: usize
        ) -> *const u8;  // Return string pointer

        // Cleanup
        unsafe fn asterix_free(ptr: *mut AsterixDataWrapper);

        // Data access
        unsafe fn asterix_record_count(ptr: *const AsterixDataWrapper) -> u32;
        unsafe fn asterix_get_record(
            ptr: *const AsterixDataWrapper,
            index: u32
        ) -> *const AsterixRecordWrapper;
    }
}
```

**Corresponding C++ wrapper header:**

```cpp
// asterix-rs/src/ffi_wrapper.h
#ifndef ASTERIX_FFI_WRAPPER_H
#define ASTERIX_FFI_WRAPPER_H

#include <memory>
#include "AsterixData.h"

namespace asterix_ffi {

    struct AsterixRecordWrapper {
        uint8_t category;
        uint32_t length;
        uint64_t timestamp_ms;
        uint32_t crc;
        const char* hex_data;
        // Item count and access methods
    };

    struct AsterixDataWrapper {
        std::unique_ptr<AsterixData> data;
        // Helper methods for Rust access
    };

    // C++ implementations exported to Rust
    extern "C" {
        bool asterix_init(const char* config_dir);
        AsterixDataWrapper* asterix_parse(
            const uint8_t* data,
            size_t len,
            bool verbose
        );
        // ... other functions
    }
}

#endif
```

---

### 4.2 High-Level Parser Module

```rust
// asterix-rs/src/parser.rs (extended)

use crate::ffi;
use std::ffi::CStr;

pub struct Parser {
    initialized: bool,
    config_path: std::path::PathBuf,
}

impl Parser {
    /// Create new parser with default configuration
    pub fn new() -> Result<Self> {
        unsafe {
            let config_path = get_default_config_path();
            let config_cstr = std::ffi::CString::new(
                config_path.to_str().unwrap()
            )?;

            if !ffi::asterix_init(config_cstr.as_ptr()) {
                return Err(AsterixError::InitializationError(
                    "Failed to initialize ASTERIX parser".to_string()
                ));
            }
        }

        Ok(Parser {
            initialized: true,
            config_path,
        })
    }

    /// Parse raw ASTERIX bytes
    pub fn parse(
        &self,
        data: &[u8],
        options: ParseOptions,
    ) -> Result<Vec<AsterixRecord>> {
        if data.is_empty() {
            return Err(AsterixError::ParseError {
                offset: 0,
                message: "Empty input".to_string(),
            });
        }

        unsafe {
            let wrapper = ffi::asterix_parse(
                data.as_ptr(),
                data.len(),
                options.verbose as i32,
            );

            if wrapper.is_null() {
                return Err(AsterixError::InternalError(
                    "Failed to parse data".to_string()
                ));
            }

            // Convert C++ wrapper to Rust structs
            let records = self.convert_wrapper(wrapper, &options)?;

            // Cleanup
            ffi::asterix_free(wrapper);

            Ok(records)
        }
    }

    fn convert_wrapper(
        &self,
        wrapper: *const ffi::AsterixDataWrapper,
        _options: &ParseOptions,
    ) -> Result<Vec<AsterixRecord>> {
        unsafe {
            let mut records = Vec::new();
            let count = ffi::asterix_record_count(wrapper);

            for i in 0..count {
                let rec_ptr = ffi::asterix_get_record(wrapper, i);
                if rec_ptr.is_null() {
                    continue;
                }

                let record = self.convert_record(&*rec_ptr)?;
                records.push(record);
            }

            Ok(records)
        }
    }

    fn convert_record(
        &self,
        rec: &ffi::AsterixRecordWrapper,
    ) -> Result<AsterixRecord> {
        Ok(AsterixRecord {
            category: rec.category,
            length: rec.length,
            timestamp_ms: rec.timestamp_ms,
            crc: rec.crc,
            hex_data: unsafe {
                CStr::from_ptr(rec.hex_data)
                    .to_string_lossy()
                    .to_string()
            },
            items: Default::default(),
        })
    }
}

impl Default for Parser {
    fn default() -> Self {
        Self::new().expect("Failed to initialize default parser")
    }
}
```

---

### 4.3 Integration Test Example

```rust
// asterix-rs/tests/integration_tests.rs

use asterix_decoder::{parse, ParseOptions};

#[test]
fn test_parse_cat048_raw() {
    let data = include_bytes!("fixtures/cat048.raw");
    let options = ParseOptions {
        verbose: true,
        filter_category: None,
        max_records: None,
    };

    let records = parse(data, options).expect("Failed to parse");
    assert!(!records.is_empty());

    let first = &records[0];
    assert_eq!(first.category, 48);
    assert!(first.length > 0);
    assert!(!first.items.is_empty());
}

#[test]
fn test_parse_cat062_pcap() {
    let data = include_bytes!("fixtures/cat062_065.pcap");
    let records = parse(data, Default::default()).expect("Failed to parse");

    // Verify multiple records from PCAP
    assert!(records.len() > 1);

    // Check category distribution
    let cat62_count = records.iter().filter(|r| r.category == 62).count();
    let cat65_count = records.iter().filter(|r| r.category == 65).count();

    assert!(cat62_count > 0);
    assert!(cat65_count > 0);
}

#[test]
fn test_incremental_parsing() {
    use asterix_decoder::parse_with_offset;

    let data = include_bytes!("fixtures/large_file.pcap");

    let mut offset = 0;
    let mut total_records = 0;

    loop {
        let result = parse_with_offset(data, offset, 100, Default::default())
            .expect("Failed to parse");

        total_records += result.records.len();
        offset = result.bytes_consumed;

        if result.remaining_blocks == 0 {
            break;
        }
    }

    assert!(total_records > 0);
}

#[test]
fn test_error_handling() {
    let invalid_data = b"not asterix data";
    let result = parse(invalid_data, Default::default());

    assert!(result.is_err());
}
```

---

## 5. Build System Integration

### 5.1 Complete build.rs Implementation

```rust
// asterix-rs/build.rs

use std::env;
use std::path::PathBuf;

fn main() {
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap();
    let target_family = env::var("CARGO_CFG_TARGET_FAMILY").unwrap();

    // Determine if we need to compile C++ or use prebuilt library
    let use_system_lib = env::var("ASTERIX_USE_SYSTEM_LIB")
        .map(|v| v == "1")
        .unwrap_or(false);

    if use_system_lib {
        link_system_library();
    } else {
        compile_cpp_from_source();
    }

    // Generate CXX bridge
    cxx_build::bridge("src/ffi.rs")
        .file("src/ffi_wrapper.cpp")
        .include("../src/asterix")
        .include("../src/engine")
        .compile("asterix_ffi");

    println!("cargo:rustc-link-search=native={}/lib",
             env::var("OUT_DIR").unwrap());
}

fn compile_cpp_from_source() {
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let asterix_root = PathBuf::from(env::var("CARGO_MANIFEST_DIR")
        .unwrap())
        .parent()
        .unwrap()
        .to_path_buf();

    // Compile C++ files
    let mut cc = cc::Build::new();

    cc.cpp(true)
        .std("c++17")
        .flag_if_supported("-fPIC")
        .include(&asterix_root / "src" / "asterix")
        .include(&asterix_root / "src" / "engine")
        // Add security flags
        .flag_if_supported("-fstack-protector-strong")
        .flag_if_supported("-D_FORTIFY_SOURCE=2");

    // Add source files
    for file in glob::glob(
        (asterix_root / "src" / "asterix" / "*.cpp")
            .to_str()
            .unwrap()
    ).unwrap() {
        if let Ok(path) = file {
            cc.file(path);
        }
    }

    // Link against libexpat
    pkg_config::probe_library("expat").unwrap_or_else(|_| {
        println!("cargo:rustc-link-lib=expat");
        pkg_config::Library {
            libs: vec!["expat".to_string()],
            link_paths: vec![],
            include_paths: vec![],
            ld_library_paths: vec![],
            frameworks: vec![],
            framework_paths: vec![],
            defines: vec![],
            version: String::new(),
            cargo_metadata: false,
        }
    });

    cc.compile("asterix_core");
}

fn link_system_library() {
    // Use system-installed asterix library
    pkg_config::probe_library("asterix").unwrap();
}
```

### 5.2 Cargo.toml Dependencies

```toml
[dependencies]
cxx = "1.0"
thiserror = "1.0"
serde = { version = "1.0", features = ["derive"], optional = true }
serde_json = { version = "1.0", optional = true }
log = "0.4"
anyhow = "1.0"

[dev-dependencies]
criterion = "0.5"
proptest = "1.3"
rstest = "0.18"
tempfile = "3.8"
env_logger = "0.10"

[build-dependencies]
cxx-build = "1.0"
cc = "1.0"
pkg-config = "0.3"
cmake = "0.1"
glob = "0.3"

[features]
default = ["serde"]
serde = ["dep:serde", "dep:serde_json"]
async = []  # Future: tokio-based async parsing

[profile.release]
opt-level = 3
lto = true
codegen-units = 1
strip = true
```

---

## 6. Cross-Platform Considerations

### 6.1 Tested Platforms

| Platform | Compiler | C++ Standard | Status |
|----------|----------|-------------|--------|
| Linux (x86_64) | GCC 9+ | C++17 | Primary |
| Linux (ARM64) | GCC 11+ | C++17 | Primary |
| macOS 12+ | AppleClang 13+ | C++17 | Secondary |
| macOS M1/M2 | AppleClang 14+ | C++17 | Secondary |
| Windows 10+ | MSVC 2019+ | C++17 | Tertiary |
| FreeBSD 12+ | Clang 12+ | C++17 | Optional |

### 6.2 Platform-Specific Code

```rust
// src/config.rs - Platform-specific config dir

#[cfg(target_os = "windows")]
fn get_default_config_path() -> PathBuf {
    PathBuf::from(r"C:\Program Files\asterix\config")
}

#[cfg(target_os = "macos")]
fn get_default_config_path() -> PathBuf {
    if let Ok(home) = std::env::var("HOME") {
        PathBuf::from(home).join("Library/Application Support/asterix/config")
    } else {
        PathBuf::from("/usr/local/etc/asterix/config")
    }
}

#[cfg(target_os = "linux")]
fn get_default_config_path() -> PathBuf {
    if let Ok(xdg_config) = std::env::var("XDG_CONFIG_HOME") {
        PathBuf::from(xdg_config).join("asterix/config")
    } else if let Ok(home) = std::env::var("HOME") {
        PathBuf::from(home).join(".config/asterix/config")
    } else {
        PathBuf::from("/etc/asterix/config")
    }
}
```

---

## 7. Testing Strategy

### 7.1 Unit Test Coverage Plan

```
Target: 90%+ code coverage

src/
├── parser.rs          (50+ tests)
├── data_types.rs      (30+ tests)
├── error.rs           (10+ tests)
├── config.rs          (20+ tests)
└── utils.rs           (15+ tests)

tests/
├── integration_tests.rs (40+ integration tests)
├── fixtures/
│   ├── cat048.raw
│   ├── cat062_065.pcap
│   ├── cat034_048.pcap
│   └── edge_cases.raw
└── property_tests.rs (20+ property-based tests)
```

### 7.2 Fuzzing

```rust
// fuzz/fuzz_targets/parse_fuzzer.rs
#![no_main]
use libfuzzer_sys::fuzz_target;
use asterix_decoder::parse;

fuzz_target!(|data: &[u8]| {
    // Try to parse any input without panicking
    let _ = parse(data, Default::default());
});
```

### 7.3 Benchmark Structure

```rust
// benches/benchmarks.rs
use criterion::{criterion_group, criterion_main, Criterion};

fn bench_parsing_sizes(c: &mut Criterion) {
    for size in &["small", "medium", "large"] {
        c.bench_function(
            &format!("parse_{}", size),
            |b| {
                // Load fixture of given size
                // Parse it
            }
        );
    }
}

criterion_group!(benches, bench_parsing_sizes);
criterion_main!(benches);
```

**Expected Results:**
- Small (512 B): <1 ms
- Medium (64 KB): 10-50 ms
- Large (1 MB): 100-500 ms

---

## 8. Performance Analysis & Optimization

### 8.1 Expected Performance Profile

| Operation | Time (μs) | Notes |
|-----------|-----------|-------|
| Parse 512 B record | 50-100 | Dominated by DOM walks |
| Parse 64 KB PCAP | 500-1000 | Typical message |
| Parse 1 MB file | 10,000-50,000 | Streaming recommended |
| Describe category | 5-10 | Metadata lookup |
| Describe item | 10-20 | Full traversal |

### 8.2 Optimization Roadmap

**Phase 4 Optimizations:**
1. **String allocation**: Pre-allocate vectors
2. **Memory reuse**: Parser state caching
3. **SIMD**: Hex string conversion (optional)
4. **Lazy evaluation**: Only compute descriptions on demand
5. **Memory pooling**: Reuse allocated structures (advanced)

**Performance Targets:**
- Achieve 95%+ of raw C++ performance
- <5% overhead from FFI marshaling
- Sub-microsecond per-field parsing

---

## 9. Packaging and Distribution

### 9.1 crates.io Publication Process

**Checklist:**
```
Before publishing:
- [ ] 90%+ test coverage achieved
- [ ] All clippy warnings resolved
- [ ] rustfmt applied to all code
- [ ] CHANGELOG.md updated
- [ ] Version bumped to 0.1.0
- [ ] Documentation builds on docs.rs
- [ ] Security audit passed (cargo-audit clean)
- [ ] Benchmarks show acceptable performance
- [ ] README has quick-start example

Publish:
cargo publish --dry-run
cargo publish
```

**Semantic Versioning:**
- 0.1.0: Initial release (API may change)
- 0.x.y: Pre-1.0 (API stability improving)
- 1.0.0: Stable API guarantee

### 9.2 docs.rs Configuration

Create `Cargo.toml` metadata:
```toml
[package.metadata.docs.rs]
features = ["serde"]
rustdoc-args = ["--cfg", "docsrs"]
targets = ["x86_64-unknown-linux-gnu"]
all-features = true
```

### 9.3 GitHub Actions CI/CD

```yaml
name: CI/CD

on: [push, pull_request]

jobs:
  test:
    name: Tests
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]
        rust: [stable, nightly]
    steps:
      - uses: actions/checkout@v3
      - uses: dtolnay/rust-toolchain@stable
        with:
          toolchain: ${{ matrix.rust }}
      - run: cargo test --all-features
      - run: cargo clippy -- -D warnings

  coverage:
    name: Code Coverage
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - uses: taiki-e/install-action@cargo-tarpaulin
      - run: cargo tarpaulin --verbose --all-features --timeout 120 --out Xml
      - uses: codecov/codecov-action@v3

  security:
    name: Security Audit
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - uses: rustsec/audit-check-action@v1

  publish:
    name: Publish to crates.io
    if: startsWith(github.ref, 'refs/tags/')
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - run: cargo publish --token ${{ secrets.CARGO_TOKEN }}
```

---

## 10. Risk Assessment and Mitigation

### 10.1 Technical Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| **C++ ABI breaking changes** | Medium | High | Vendor entire C++ core as static lib |
| **Memory safety violations** | Low | Critical | Extensive fuzzing, valgrind tests |
| **Performance regression** | Low | Medium | Benchmarks in CI, 95% target |
| **libexpat availability** | Low | Medium | Vendored static build as fallback |
| **Windows build failures** | Medium | Medium | CI on Windows, early testing |
| **Binary compatibility across Rust versions** | Low | Low | MSRV policy (stable-2) |

### 10.2 Mitigation Strategies

**Memory Safety:**
- Run valgrind/MSAN on all CI runs
- Cargo-fuzz on parse path
- Address Sanitizer (ASAN) in debug builds
- No unwrap/panic in library code

**Build Stability:**
- Vendor C++ as static library copy in `src/asterix_embedded/`
- Don't depend on system libraries
- Test matrix: Linux (GCC, Clang), macOS (x86, ARM), Windows (MSVC)

**Performance:**
- Benchmark in every PR
- Required: >90% of C++ performance
- Profile with perf/flamegraph before release

**Version Compatibility:**
- MSRV: Rust 1.70 (2023 edition)
- Pin dependencies to major versions
- Semantic versioning from day one

---

## 11. Deployment Timeline

### Detailed Phase Schedule

```
Week 1-2: Foundation
├─ Cargo.toml setup (2 days)
├─ build.rs with C++ compilation (4 days)
├─ Basic FFI bridge (5 days)
├─ Initial CI setup (2 days)
└─ Verification (1 day)

Week 3-4: Core Parsing API
├─ Parser module (6 days)
├─ Data types (4 days)
├─ Error handling (3 days)
├─ Unit tests (3 days)
└─ Documentation (1 day)

Week 5-6: Advanced Features
├─ Config management (3 days)
├─ Describe function (3 days)
├─ Format detection (4 days)
├─ Windows/cross-platform (4 days)
└─ Additional tests (2 days)

Week 7-8: Performance & Optimization
├─ Benchmarking framework (3 days)
├─ Profiling & optimization (5 days)
├─ Async streaming API (4 days)
├─ Memory profiling (2 days)
└─ Documentation (2 days)

Week 9-10: Testing & Documentation
├─ Property-based testing (3 days)
├─ Fuzzing setup (2 days)
├─ Integration tests (4 days)
├─ Full API documentation (3 days)
├─ Migration guide (2 days)
└─ Examples (2 days)

Week 11-12: Publication & Release
├─ Security audit (2 days)
├─ Final testing (2 days)
├─ crates.io publishing (1 day)
├─ Release notes (2 days)
├─ GitHub release (1 day)
└─ Post-release monitoring (4 days)
```

**Critical Path:** Week 1-2 (Foundation) → Week 3-4 (Core API) → Week 11-12 (Release)

**Parallel Activities:**
- Week 5-6 and 7-8 can run in parallel with adjusted staffing
- Week 9-10 documentation can start in week 6

---

## 12. Success Metrics

### 12.1 Functional Success Criteria

- [ ] All major Python APIs available in Rust
  - parse(data) → Vec<AsterixRecord>
  - parse_with_offset() with full semantics
  - describe() with all lookups
  - init() for custom configs

- [ ] 90%+ test coverage (reported by tarpaulin)
- [ ] Zero panics in library code (except tests)
- [ ] Comprehensive error types (no generic errors)
- [ ] Full rustdoc with examples for every public API

### 12.2 Performance Criteria

- [ ] 95%+ of raw C++ performance
- [ ] <5% overhead from FFI marshaling
- [ ] Sub-microsecond per-record parsing
- [ ] <1 MB memory per 1 MB input

### 12.3 Quality Criteria

- [ ] Zero clippy warnings
- [ ] Code formatted with rustfmt
- [ ] All dependencies audited (cargo-audit)
- [ ] No unsafe code in public API surface (only in ffi module)
- [ ] MSAN/ASAN clean build
- [ ] Fuzzing campaign (1000s of inputs)

### 12.4 Adoption Criteria

- [ ] Published on crates.io
- [ ] Documentation on docs.rs
- [ ] CI/CD fully automated
- [ ] GitHub releases with install instructions
- [ ] Example code runnable as-is

---

## 13. Appendices

### 13.1 Comparison: FFI Technologies

| Aspect | CXX | Bindgen | cbindgen |
|--------|-----|---------|----------|
| **Safety** | Type-safe bridge | Unsafe wrapper | Unsafe wrapper |
| **Ease of use** | Moderate (bridge DSL) | High (auto-generate) | Medium |
| **Code generation** | Minimal | Full FFI | Limited |
| **C++ complexity** | Good | Limited | Limited |
| **Maintenance** | Active | Very active | Moderate |
| **Best for** | Complex C++ | C APIs | Exporting Rust to C |

**Recommendation:** CXX for primary, consider bindgen if raw struct access needed.

---

### 13.2 Python vs Rust API Equivalence

```
Python                              Rust
==================================================
asterix.parse(data)                 parse(&data, Default::default())
asterix.parse(data, verbose=False)  parse(&data, ParseOptions { verbose: false, .. })
asterix.parse_with_offset(...)      parse_with_offset(data, offset, blocks, opts)
asterix.describe(cat, ...)          describe(cat, item, field, value)
asterix.init(path)                  Parser::new().add_category(path)
asterix.list_sample_files()         [NOT PROVIDED - filesystem dependent]
asterix.get_configuration_file()    config module functions
```

**Breaking changes from Python:**
- No global state (parser requires explicit construction)
- Error types as Result<T> instead of exceptions
- No lazy describe evaluation (evaluate on demand)
- No XML output builders (use serde instead)

---

### 13.3 Recommended Reading

**FFI & Rust/C++ Interop:**
- CXX Book: https://cxx.rs/
- Rust FFI Guide: https://doc.rust-lang.org/nomicon/ffi.html
- Safe FFI Patterns: https://matklad.github.io/2021/03/04/fast-simple-rust-interner.html

**Performance:**
- Flamegraph: http://www.brendangregg.com/flamegraphs.html
- Criterion.rs: https://bheisler.github.io/criterion.rs/book/

**Security:**
- OWASP Rust Security: https://cheatsheetseries.owasp.org/cheatsheets/Rust_Security_Cheat_Sheet.html
- CWE-119: Buffer overflows (key concern in parsing)

---

### 13.4 Dependencies Analysis

**Essential:**
- `cxx`: FFI bridge (1.0, maintained)
- `thiserror`: Error derive (1.0, standard)

**Recommended:**
- `serde`/`serde_json`: Serialization (optional feature)
- `log`: Logging (standard)
- `anyhow`: Error handling (alternative to thiserror)

**Development:**
- `criterion`: Benchmarking (standard)
- `proptest`: Property testing
- `cargo-fuzz`: Fuzzing harness
- `cargo-audit`: Security audit

**CI/CD:**
- GitHub Actions (free, integrated)
- codecov (coverage tracking)
- cargo-deny (dependency security)

---

## 14. Conclusion

This plan provides a comprehensive roadmap for adding production-grade Rust bindings to the ASTERIX decoder project. The use of the `cxx` crate ensures type safety at the FFI boundary while maintaining near-native performance. The phased approach allows for incremental delivery with opportunities for feedback and adjustment.

### Key Success Factors:

1. **Foundation phase (1-2 weeks)**: Establish build system and FFI layer
2. **Core API (weeks 3-4)**: Deliver minimal viable product
3. **Polish (weeks 5-8)**: Performance and feature completeness
4. **Validation (weeks 9-10)**: Comprehensive testing and documentation
5. **Release (weeks 11-12)**: Publication and community readiness

### Next Steps:

1. **Immediate**: Create asterix-rs/ directory structure
2. **Week 1**: Begin foundation phase with build.rs
3. **Week 2**: Verify compilation on 3 platforms
4. **Week 3**: Start core parsing API implementation

The Rust bindings will enable:
- Type-safe access to ASTERIX data
- Zero-cost abstractions matching C++ performance
- Idiomatic Rust error handling
- Integration with modern Rust ecosystem (serde, async/await)
- New audience of Rust developers in ATM systems

---

**Document Version:** 1.0
**Created:** 2025-11-01
**Status:** Ready for Implementation
