//! FFI bridge declarations for C++ ASTERIX core
//!
//! This module defines the CXX bridge between Rust and C++. It uses the `cxx` crate
//! for type-safe interop, minimizing unsafe code and providing compile-time verification
//! of FFI contracts.
//!
//! # Safety
//!
//! All FFI functions are marked `unsafe` as they cross language boundaries. The safe
//! Rust API in `parser.rs` wraps these functions and ensures memory safety.

#[allow(clippy::module_inception)]
#[cxx::bridge(namespace = "asterix")]
pub mod ffi {
    // Opaque C++ types - we don't need to know their internal structure
    unsafe extern "C++" {
        include!("ffi_wrapper.h");

        // Opaque wrapper types for C++ objects
        type AsterixDataWrapper;
        type DataBlockWrapper;

        // Initialization
        // Initialize ASTERIX definitions from config directory
        unsafe fn asterix_init(config_dir: &str) -> bool;

        // Load a specific category definition file
        unsafe fn asterix_load_category(xml_path: &str) -> bool;

        // Parsing functions
        // Parse raw ASTERIX data and return wrapper
        unsafe fn asterix_parse(
            data: *const u8,
            len: usize,
            verbose: bool,
        ) -> *mut AsterixDataWrapper;

        // Parse with offset for incremental parsing
        unsafe fn asterix_parse_offset(
            data: *const u8,
            len: usize,
            offset: u32,
            blocks_count: u32,
            verbose: bool,
        ) -> *mut AsterixDataWrapper;

        // Cleanup
        unsafe fn asterix_free_data(ptr: *mut AsterixDataWrapper);

        // Data access methods
        // Get number of data blocks in parsed result
        unsafe fn asterix_data_block_count(data: *const AsterixDataWrapper) -> u32;

        // Get a specific data block by index
        unsafe fn asterix_get_data_block(
            data: *const AsterixDataWrapper,
            index: u32,
        ) -> *const DataBlockWrapper;

        // Data block properties
        unsafe fn asterix_block_category(block: *const DataBlockWrapper) -> u8;
        unsafe fn asterix_block_length(block: *const DataBlockWrapper) -> u32;
        unsafe fn asterix_block_timestamp_ms(block: *const DataBlockWrapper) -> u64;
        unsafe fn asterix_block_crc(block: *const DataBlockWrapper) -> u32;

        // Get hex representation of data block
        // Returns pointer to null-terminated string (lifetime tied to block)
        unsafe fn asterix_block_hex_data(block: *const DataBlockWrapper) -> *const u8;

        // Get JSON representation of data block
        // Returns pointer to null-terminated string (must be freed with asterix_free_string)
        unsafe fn asterix_block_to_json(block: *const DataBlockWrapper) -> *mut u8;

        // Get text representation of data block
        #[allow(dead_code)]
        unsafe fn asterix_block_to_text(block: *const DataBlockWrapper) -> *mut u8;

        // Free string allocated by C++ side
        unsafe fn asterix_free_string(ptr: *mut u8);

        // Metadata functions
        // Get description of category/item/field/value
        // Returns allocated string (must be freed with asterix_free_string)
        unsafe fn asterix_describe(
            category: u8,
            item: *const u8,
            item_len: usize,
            field: *const u8,
            field_len: usize,
            value: *const u8,
            value_len: usize,
        ) -> *mut u8;

        // Check if category is defined
        unsafe fn asterix_category_defined(category: u8) -> bool;

        // Log level control
        // Set log level: 0 = silent, 1 = errors only, 2 = warnings, 3 = info, 4 = debug
        unsafe fn asterix_set_log_level(level: i32);

        // Get current log level
        unsafe fn asterix_get_log_level() -> i32;
    }
}

// Safe wrapper functions for common operations
use crate::error::{AsterixError, Result};
use std::ffi::CStr;

/// Initialize ASTERIX with default config directory
pub fn init_default() -> Result<()> {
    use std::path::PathBuf;

    // First, initialize the AsterixDefinition object (stub implementation in C++)
    let config_dir = get_default_config_dir();
    init_config_dir(&config_dir)?;

    // Then, load all XML category files from the source tree
    // (since asterix_init is a stub that doesn't load files)
    let manifest_dir = env!("CARGO_MANIFEST_DIR");
    let config_path = PathBuf::from(manifest_dir)
        .parent()
        .ok_or_else(|| AsterixError::InitializationError("Invalid manifest dir".to_string()))?
        .join("asterix/config");

    if !config_path.exists() {
        return Err(AsterixError::ConfigNotFound(format!(
            "Config directory not found: {}",
            config_path.display()
        )));
    }

    // Load BDS definitions first (required by many categories)
    let bds_file = config_path.join("asterix_bds.xml");
    if bds_file.exists() {
        load_category(
            bds_file.to_str().ok_or_else(|| {
                AsterixError::InvalidData("Invalid UTF-8 in BDS path".to_string())
            })?,
        )?;
    }

    // Load all category XML files from the config directory
    let entries = std::fs::read_dir(&config_path)
        .map_err(|e| AsterixError::IOError(format!("Failed to read config directory: {e}")))?;

    let mut loaded_count = 0;
    for entry in entries {
        let entry = entry?;
        let path = entry.path();

        // Skip BDS file (already loaded) and non-XML files
        if path.file_name().and_then(|n| n.to_str()) == Some("asterix_bds.xml") {
            continue;
        }

        if path.extension().and_then(|e| e.to_str()) == Some("xml") {
            load_category(
                path.to_str().ok_or_else(|| {
                    AsterixError::InvalidData("Invalid UTF-8 in path".to_string())
                })?,
            )?;
            loaded_count += 1;
        }
    }

    if loaded_count == 0 {
        return Err(AsterixError::InitializationError(
            "No XML configuration files found in config directory".to_string(),
        ));
    }

    Ok(())
}

/// Initialize ASTERIX with specific config directory
pub fn init_config_dir(config_dir: &str) -> Result<()> {
    // MEDIUM-004 FIX: Validate directory path parameter
    if config_dir.is_empty() {
        return Err(AsterixError::InvalidData("Directory path cannot be empty".to_string()));
    }

    // MEDIUM-004 FIX: Check for path traversal attacks (Windows + Unix)
    // Only reject actual traversal patterns, not legitimate relative paths
    // Reject: starts with "../" or "..\", contains "/../" or "\..\", or is exactly ".."
    if config_dir.starts_with("../") ||
       config_dir.starts_with("..\\") ||
       config_dir == ".." ||
       config_dir.contains("/../") ||
       config_dir.contains("\\..\\") ||
       config_dir.contains("\\../") ||
       config_dir.contains("/..\\") {
        return Err(AsterixError::InvalidData(
            "Invalid directory path: path traversal detected (..)".to_string(),
        ));
    }

    // MEDIUM-004 FIX: Validate path length
    if config_dir.len() > 4096 {
        return Err(AsterixError::InvalidData(
            "Directory path too long (maximum 4096 characters)".to_string(),
        ));
    }

    let c_str = std::ffi::CString::new(config_dir)?;

    unsafe {
        if ffi::asterix_init(c_str.as_c_str().to_str().unwrap()) {
            Ok(())
        } else {
            Err(AsterixError::InitializationError(format!(
                "Failed to initialize ASTERIX with config dir: {config_dir}"
            )))
        }
    }
}

/// Load a category definition file
pub fn load_category(xml_path: &str) -> Result<()> {
    // MEDIUM-004 FIX: Validate filename parameter
    if xml_path.is_empty() {
        return Err(AsterixError::InvalidData("Filename cannot be empty".to_string()));
    }

    // MEDIUM-004 FIX: Check for path traversal attacks (Windows + Unix)
    // Only reject actual traversal patterns, not legitimate relative paths
    // Reject: starts with "../" or "..\", contains "/../" or "\..\", or is exactly ".."
    if xml_path.starts_with("../") ||
       xml_path.starts_with("..\\") ||
       xml_path == ".." ||
       xml_path.contains("/../") ||
       xml_path.contains("\\..\\") ||
       xml_path.contains("\\../") ||
       xml_path.contains("/..\\") {
        return Err(AsterixError::InvalidData(
            "Invalid filename: path traversal detected (..)".to_string(),
        ));
    }

    // MEDIUM-004 FIX: Validate filename length
    if xml_path.len() > 4096 {
        return Err(AsterixError::InvalidData(
            "Filename too long (maximum 4096 characters)".to_string(),
        ));
    }

    let c_str = std::ffi::CString::new(xml_path)?;

    unsafe {
        if ffi::asterix_load_category(c_str.as_c_str().to_str().unwrap()) {
            Ok(())
        } else {
            Err(AsterixError::InitializationError(format!(
                "Failed to load category file: {xml_path}"
            )))
        }
    }
}

/// Check if a category is defined
pub fn is_category_defined(category: u8) -> bool {
    unsafe { ffi::asterix_category_defined(category) }
}

/// Log level for ASTERIX parser output
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum LogLevel {
    /// Silent - no output
    Silent = 0,
    /// Errors only (default)
    Error = 1,
    /// Warnings and errors
    Warn = 2,
    /// Info, warnings, and errors
    Info = 3,
    /// Debug output (verbose)
    Debug = 4,
}

/// Set the log level for C++ output
///
/// Controls how much diagnostic output the ASTERIX parser produces.
/// By default, the log level is set to Error.
///
/// # Example
///
/// ```no_run
/// use asterix::{set_log_level, LogLevel};
///
/// // Silence all output (useful for tests)
/// set_log_level(LogLevel::Silent);
///
/// // Show only errors (default)
/// set_log_level(LogLevel::Error);
///
/// // Show debug output
/// set_log_level(LogLevel::Debug);
/// ```
pub fn set_log_level(level: LogLevel) {
    unsafe {
        ffi::asterix_set_log_level(level as i32);
    }
}

/// Get the current log level
pub fn get_log_level() -> LogLevel {
    let level = unsafe { ffi::asterix_get_log_level() };
    match level {
        0 => LogLevel::Silent,
        1 => LogLevel::Error,
        2 => LogLevel::Warn,
        3 => LogLevel::Info,
        4 => LogLevel::Debug,
        _ => LogLevel::Error, // Default fallback
    }
}

/// Get description for a category/item/field/value
pub fn describe(
    category: u8,
    item: Option<&str>,
    field: Option<&str>,
    value: Option<&str>,
) -> Result<String> {
    // MEDIUM-003 FIX: Validate category is within valid ASTERIX range
    // Category 0 is not valid in ASTERIX (valid range: 1-255)
    if category == 0 {
        return Err(AsterixError::InvalidData(
            "Invalid ASTERIX category: 0 (valid range: 1-255)".to_string(),
        ));
    }

    unsafe {
        let item_ptr = item.map(|s| s.as_ptr()).unwrap_or(std::ptr::null());
        let item_len = item.map(|s| s.len()).unwrap_or(0);

        let field_ptr = field.map(|s| s.as_ptr()).unwrap_or(std::ptr::null());
        let field_len = field.map(|s| s.len()).unwrap_or(0);

        let value_ptr = value.map(|s| s.as_ptr()).unwrap_or(std::ptr::null());
        let value_len = value.map(|s| s.len()).unwrap_or(0);

        let result_ptr = ffi::asterix_describe(
            category, item_ptr, item_len, field_ptr, field_len, value_ptr, value_len,
        );

        if result_ptr.is_null() {
            return Err(AsterixError::InternalError(
                "Failed to get description".to_string(),
            ));
        }

        let c_str = CStr::from_ptr(result_ptr as *const std::os::raw::c_char);
        let result = c_str.to_string_lossy().to_string();

        ffi::asterix_free_string(result_ptr);

        Ok(result)
    }
}

/// Helper to convert C string pointer to Rust String (and free it)
pub(crate) unsafe fn c_string_to_rust(ptr: *mut u8) -> Result<String> {
    if ptr.is_null() {
        return Err(AsterixError::NullPointer(
            "Got null string from C++".to_string(),
        ));
    }

    let c_str = CStr::from_ptr(ptr as *const std::os::raw::c_char);
    let result = c_str.to_string_lossy().to_string();

    ffi::asterix_free_string(ptr);

    Ok(result)
}

/// Get default config directory based on platform
fn get_default_config_dir() -> String {
    #[cfg(target_os = "linux")]
    {
        if let Ok(xdg_config) = std::env::var("XDG_CONFIG_HOME") {
            format!("{xdg_config}/asterix/config")
        } else if let Ok(home) = std::env::var("HOME") {
            format!("{home}/.config/asterix/config")
        } else {
            "/etc/asterix/config".to_string()
        }
    }

    #[cfg(target_os = "macos")]
    {
        if let Ok(home) = std::env::var("HOME") {
            format!("{}/Library/Application Support/asterix/config", home)
        } else {
            "/usr/local/etc/asterix/config".to_string()
        }
    }

    #[cfg(target_os = "windows")]
    {
        r"C:\Program Files\asterix\config".to_string()
    }

    #[cfg(not(any(target_os = "linux", target_os = "macos", target_os = "windows")))]
    {
        "/etc/asterix/config".to_string()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_default_config_dir() {
        let config_dir = get_default_config_dir();
        assert!(!config_dir.is_empty());
        assert!(config_dir.contains("asterix"));
    }

    #[test]
    fn test_c_string_creation() {
        let test_str = "test/path";
        let c_str = std::ffi::CString::new(test_str).unwrap();
        assert_eq!(c_str.to_str().unwrap(), test_str);
    }
}
