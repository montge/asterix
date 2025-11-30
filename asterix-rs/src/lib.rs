//! Rust bindings for the ASTERIX ATM surveillance protocol decoder
//!
//! This crate provides safe, idiomatic Rust bindings to the C++ ASTERIX decoder library.
//! ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is
//! a binary protocol used for Air Traffic Management (ATM) surveillance data exchange.
//!
//! # Features
//!
//! - Type-safe parsing of ASTERIX categories (48, 62, 65, etc.)
//! - Support for multiple input formats (raw, PCAP, HDLC, FINAL, GPS)
//! - Zero-copy parsing where possible
//! - Comprehensive error handling
//! - Optional serde support for JSON serialization
//! - Memory-safe FFI layer using the `cxx` crate
//!
//! # Thread Safety
//!
//! **WARNING: This library is NOT thread-safe.**
//!
//! The underlying C++ ASTERIX decoder uses a global singleton (`AsterixDefinition`)
//! to manage category definitions. This means:
//!
//! - Concurrent calls to `parse()`, `init_default()`, `load_category()`, or `describe()`
//!   from multiple threads will cause race conditions and undefined behavior
//! - The library does not use internal locking or synchronization
//! - Rust's memory safety guarantees do NOT extend to C++ global state
//!
//! ## Safe Usage Patterns
//!
//! If you need to parse ASTERIX data from multiple threads, use one of these approaches:
//!
//! 1. **Single-threaded parsing** (recommended):
//!    ```ignore
//!    use asterix::{parse, ParseOptions, init_default};
//!
//!    # fn main() -> Result<(), Box<dyn std::error::Error>> {
//!    init_default()?;
//!    let data = std::fs::read("sample.asterix")?;
//!    let records = parse(&data, ParseOptions::default())?;
//!
//!    // Process records in parallel AFTER parsing completes
//!    use rayon::prelude::*;
//!    records.par_iter().for_each(|record| {
//!        // Safe: only reading parsed data
//!        println!("Category: {}", record.category);
//!    });
//!    # Ok(())
//!    # }
//!    ```
//!
//! 2. **Mutex-protected access**:
//!    ```ignore
//!    use asterix::{parse, ParseOptions};
//!    use std::sync::Mutex;
//!
//!    lazy_static::lazy_static! {
//!        static ref ASTERIX_LOCK: Mutex<()> = Mutex::new(());
//!    }
//!
//!    # fn parse_data(data: &[u8]) -> Result<(), Box<dyn std::error::Error>> {
//!    let _guard = ASTERIX_LOCK.lock().unwrap();
//!    let records = parse(data, ParseOptions::default())?;
//!    // Process records while holding lock
//!    # Ok(())
//!    # }
//!    ```
//!
//! 3. **Process-based parallelism**:
//!    Use separate processes instead of threads (e.g., with `rayon`'s process pool
//!    or manual process spawning).
//!
//! # Quick Start
//!
//! ```no_run
//! use asterix::{parse, ParseOptions, init_default};
//!
//! # fn main() -> Result<(), Box<dyn std::error::Error>> {
//! // Initialize ASTERIX with default config
//! init_default()?;
//!
//! // Read ASTERIX data
//! let data = std::fs::read("sample.asterix")?;
//!
//! // Parse with default options
//! let records = parse(&data, ParseOptions::default())?;
//!
//! // Process parsed records
//! for record in records {
//!     println!("Category {}: {} items", record.category, record.items.len());
//!
//!     // Access specific data items
//!     if let Some(item) = record.get_item("I062/010") {
//!         println!("  SAC/SIC: {:?}", item.fields);
//!     }
//! }
//! # Ok(())
//! # }
//! ```
//!
//! # Incremental Parsing
//!
//! For large files or streams, use `parse_with_offset`:
//!
//! ```no_run
//! use asterix::{parse_with_offset, ParseOptions, init_default};
//!
//! # fn main() -> Result<(), Box<dyn std::error::Error>> {
//! init_default()?;
//! let data = std::fs::read("large_file.asterix")?;
//!
//! let mut offset = 0;
//! let mut all_records = Vec::new();
//!
//! loop {
//!     let result = parse_with_offset(&data, offset, 100, ParseOptions::default())?;
//!     all_records.extend(result.records);
//!     offset = result.bytes_consumed;
//!
//!     if result.remaining_blocks == 0 {
//!         break;
//!     }
//! }
//!
//! println!("Parsed {} total records", all_records.len());
//! # Ok(())
//! # }
//! ```
//!
//! # Custom Configuration
//!
//! Load custom category definitions:
//!
//! ```no_run
//! use asterix::{load_category, init_config_dir};
//!
//! # fn main() -> Result<(), Box<dyn std::error::Error>> {
//! // Initialize with custom config directory
//! init_config_dir("/path/to/asterix/config")?;
//!
//! // Or load a specific category file
//! load_category("/path/to/asterix_cat062_1_18.xml")?;
//! # Ok(())
//! # }
//! ```
//!
//! # Metadata Queries
//!
//! Get descriptions for categories, items, and fields:
//!
//! ```no_run
//! use asterix::describe;
//!
//! # fn main() -> Result<(), Box<dyn std::error::Error>> {
//! // Describe a category
//! let cat_desc = describe(62, None, None, None)?;
//! println!("Category 62: {}", cat_desc);
//!
//! // Describe a specific item
//! let item_desc = describe(62, Some("010"), None, None)?;
//! println!("Item I062/010: {}", item_desc);
//!
//! // Describe a field value
//! let value_desc = describe(62, Some("010"), Some("SAC"), Some("1"))?;
//! println!("SAC=1: {}", value_desc);
//! # Ok(())
//! # }
//! ```
//!
//! # Error Handling
//!
//! All public functions return `Result<T, AsterixError>`:
//!
//! ```no_run
//! use asterix::{parse, ParseOptions, AsterixError, init_default};
//!
//! # fn main() {
//! init_default().expect("Failed to initialize");
//! let data = b"\x30\x00\x10invalid";
//!
//! match parse(data, ParseOptions::default()) {
//!     Ok(records) => println!("Success: {} records", records.len()),
//!     Err(AsterixError::ParseError { offset, message }) => {
//!         eprintln!("Parse failed at byte {}: {}", offset, message);
//!     }
//!     Err(AsterixError::InvalidCategory { category, reason }) => {
//!         eprintln!("Category {} not supported: {}", category, reason);
//!     }
//!     Err(e) => eprintln!("Other error: {}", e),
//! }
//! # }
//! ```
//!
//! # Feature Flags
//!
//! - `serde` (default): Enable JSON serialization/deserialization
//! - `async`: Enable async parsing support (future)
//!
//! # Platform Support
//!
//! - Linux (x86_64, ARM64) - Primary
//! - macOS (x86_64, Apple Silicon) - Secondary
//! - Windows (x86_64) - Tertiary
//!
//! # Safety
//!
//! This crate uses unsafe FFI internally but provides a completely safe public API.
//! All memory management is handled automatically, and all C++ exceptions are
//! converted to Rust `Result` types.

#![warn(missing_docs)]
#![warn(rustdoc::missing_crate_level_docs)]
#![cfg_attr(docsrs, feature(doc_cfg))]

// Public modules
pub mod error;
pub mod parser;
pub mod types;

// Transport modules (feature-gated)
#[cfg(feature = "zenoh")]
#[cfg_attr(docsrs, doc(cfg(feature = "zenoh")))]
pub mod transport;

// Private FFI module
mod ffi;

// Re-export main types and functions for convenience
pub use error::{AsterixError, Result};
pub use parser::{parse, parse_with_offset};
pub use types::{AsterixRecord, DataItem, ParseOptions, ParseResult, ParsedValue};

// Re-export FFI initialization functions
pub use ffi::{
    describe, get_log_level, init_config_dir, init_default, is_category_defined, load_category,
    set_log_level, LogLevel,
};

// Version information
/// Crate version
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

/// Crate name
pub const CRATE_NAME: &str = env!("CARGO_PKG_NAME");

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[allow(clippy::const_is_empty)]
    fn test_version_info() {
        assert!(!VERSION.is_empty());
        assert_eq!(CRATE_NAME, "asterix");
    }

    #[test]
    fn test_parse_options_default() {
        let opts = ParseOptions::default();
        assert!(!opts.verbose);
        assert_eq!(opts.filter_category, None);
        assert_eq!(opts.max_records, None);
    }

    #[test]
    fn test_asterix_record_creation() {
        let record = AsterixRecord::default();
        assert_eq!(record.category, 0);
        assert_eq!(record.length, 0);
        assert!(record.items.is_empty());
    }

    #[test]
    fn test_parsed_value_conversions() {
        let int_val = ParsedValue::Integer(42);
        assert_eq!(int_val.as_i64(), Some(42));
        assert_eq!(int_val.as_f64(), Some(42.0));

        let str_val = ParsedValue::String("test".to_string());
        assert_eq!(str_val.as_str(), Some("test"));
        assert_eq!(str_val.as_i64(), None);
    }

    #[test]
    fn test_error_display() {
        let err = AsterixError::ParseError {
            offset: 100,
            message: "Invalid data".to_string(),
        };
        let display = format!("{err}");
        assert!(display.contains("100"));
        assert!(display.contains("Invalid data"));
    }
}
