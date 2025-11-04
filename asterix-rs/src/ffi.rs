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
    }
}

// Safe wrapper functions for common operations
use crate::error::{AsterixError, Result};
use std::ffi::CStr;

/// Initialize ASTERIX with default config directory
pub fn init_default() -> Result<()> {
    let config_dir = get_default_config_dir();
    init_config_dir(&config_dir)
}

/// Initialize ASTERIX with specific config directory
pub fn init_config_dir(config_dir: &str) -> Result<()> {
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

/// Get description for a category/item/field/value
pub fn describe(
    category: u8,
    item: Option<&str>,
    field: Option<&str>,
    value: Option<&str>,
) -> Result<String> {
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

        let c_str = CStr::from_ptr(result_ptr as *const i8);
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

    let c_str = CStr::from_ptr(ptr as *const i8);
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
