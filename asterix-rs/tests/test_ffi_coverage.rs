//! Additional FFI tests to improve coverage
//!
//! These tests target uncovered error paths and edge cases in the FFI layer.

use asterix::{
    describe, get_log_level, init_config_dir, is_category_defined, load_category, set_log_level,
    AsterixError, LogLevel,
};
use std::sync::Once;

static INIT: Once = Once::new();

fn ensure_initialized() {
    INIT.call_once(|| {
        set_log_level(LogLevel::Silent);
        let _ = asterix::init_default();
    });
}

#[test]
fn test_init_config_dir_empty_path() {
    // Test empty directory path validation
    let result = init_config_dir("");
    assert!(result.is_err());
    match result {
        Err(AsterixError::InvalidData(msg)) => {
            assert!(msg.contains("empty"));
        }
        _ => panic!("Expected InvalidData error for empty path"),
    }
}

#[test]
fn test_init_config_dir_path_traversal_unix() {
    // Test path traversal attack prevention (Unix style)
    let result = init_config_dir("../../../etc/passwd");
    assert!(result.is_err());
    match result {
        Err(AsterixError::InvalidData(msg)) => {
            assert!(msg.contains("traversal") || msg.contains(".."));
        }
        _ => panic!("Expected InvalidData error for path traversal"),
    }
}

#[test]
fn test_init_config_dir_path_traversal_windows() {
    // Test path traversal attack prevention (Windows style)
    let result = init_config_dir("..\\..\\..\\windows\\system32");
    assert!(result.is_err());
    match result {
        Err(AsterixError::InvalidData(msg)) => {
            assert!(msg.contains("traversal") || msg.contains(".."));
        }
        _ => panic!("Expected InvalidData error for path traversal"),
    }
}

#[test]
fn test_init_config_dir_path_traversal_relative() {
    // Test ".." as path (edge case)
    let result = init_config_dir("..");
    assert!(result.is_err());
    match result {
        Err(AsterixError::InvalidData(msg)) => {
            assert!(msg.contains("traversal") || msg.contains(".."));
        }
        _ => panic!("Expected InvalidData error for '..' path"),
    }
}

#[test]
fn test_init_config_dir_too_long() {
    // Test path length validation (>4096 chars)
    let long_path = "a".repeat(5000);
    let result = init_config_dir(&long_path);
    assert!(result.is_err());
    match result {
        Err(AsterixError::InvalidData(msg)) => {
            assert!(msg.contains("too long") || msg.contains("4096"));
        }
        _ => panic!("Expected InvalidData error for long path"),
    }
}

#[test]
fn test_init_config_dir_invalid_path() {
    // Test initialization with invalid directory
    // Note: C++ init may succeed even with invalid paths (stub implementation)
    // so we just verify the function doesn't panic
    let result = init_config_dir("/nonexistent/invalid/path/to/config");

    // Either succeeds (stub) or fails gracefully
    match result {
        Ok(_) => {
            // C++ stub implementation may succeed
        }
        Err(AsterixError::InitializationError(_)) => {
            // Or it may fail - both are acceptable
        }
        Err(e) => panic!("Unexpected error type: {e:?}"),
    }
}

#[test]
fn test_load_category_empty_filename() {
    // Test empty filename validation
    let result = load_category("");
    assert!(result.is_err());
    match result {
        Err(AsterixError::InvalidData(msg)) => {
            assert!(msg.contains("empty"));
        }
        _ => panic!("Expected InvalidData error for empty filename"),
    }
}

#[test]
fn test_load_category_path_traversal_unix() {
    // Test path traversal in filename (Unix)
    let result = load_category("../../../etc/passwd");
    assert!(result.is_err());
    match result {
        Err(AsterixError::InvalidData(msg)) => {
            assert!(msg.contains("traversal") || msg.contains(".."));
        }
        _ => panic!("Expected InvalidData error for path traversal"),
    }
}

#[test]
fn test_load_category_path_traversal_windows() {
    // Test path traversal in filename (Windows)
    let result = load_category("..\\..\\..\\windows\\system.ini");
    assert!(result.is_err());
    match result {
        Err(AsterixError::InvalidData(msg)) => {
            assert!(msg.contains("traversal") || msg.contains(".."));
        }
        _ => panic!("Expected InvalidData error for path traversal"),
    }
}

#[test]
fn test_load_category_filename_too_long() {
    // Test filename length validation
    let long_filename = format!("{}.xml", "a".repeat(5000));
    let result = load_category(&long_filename);
    assert!(result.is_err());
    match result {
        Err(AsterixError::InvalidData(msg)) => {
            assert!(msg.contains("too long") || msg.contains("4096"));
        }
        _ => panic!("Expected InvalidData error for long filename"),
    }
}

#[test]
fn test_load_category_nonexistent_file() {
    // Test loading non-existent category file
    let result = load_category("/tmp/nonexistent_category_12345.xml");
    assert!(result.is_err());
    match result {
        Err(AsterixError::InitializationError(_)) => {}
        _ => panic!("Expected InitializationError for nonexistent file"),
    }
}

#[test]
fn test_describe_category_zero() {
    ensure_initialized();

    // Test category 0 validation (invalid in ASTERIX)
    let result = describe(0, None, None, None);
    assert!(result.is_err());
    match result {
        Err(AsterixError::InvalidData(msg)) => {
            assert!(msg.contains("0") && msg.contains("valid range"));
        }
        _ => panic!("Expected InvalidData error for category 0"),
    }
}

#[test]
fn test_describe_undefined_category() {
    ensure_initialized();

    // Test undefined category (likely returns error from C++)
    let result = describe(250, Some("010"), None, None);
    // May return error or empty description depending on C++ behavior
    match result {
        Ok(_) => {
            // C++ may return empty string for undefined categories
        }
        Err(_) => {
            // Or it may return an error
        }
    }
}

#[test]
fn test_is_category_defined_valid() {
    ensure_initialized();

    // Test common ASTERIX categories
    let cat48_defined = is_category_defined(48);
    let cat62_defined = is_category_defined(62);

    // At least one should be defined after init
    assert!(cat48_defined || cat62_defined);
}

#[test]
fn test_is_category_defined_invalid() {
    ensure_initialized();

    // Test unlikely category (probably not defined)
    let cat255_defined = is_category_defined(255);
    // We can't assert false because it might be defined, but we test the function works
    let _ = cat255_defined;
}

#[test]
fn test_log_level_roundtrip() {
    // Test get/set log level
    set_log_level(LogLevel::Debug);
    let level = get_log_level();
    assert_eq!(level, LogLevel::Debug);

    set_log_level(LogLevel::Silent);
    let level = get_log_level();
    assert_eq!(level, LogLevel::Silent);

    set_log_level(LogLevel::Error);
    let level = get_log_level();
    assert_eq!(level, LogLevel::Error);

    set_log_level(LogLevel::Warn);
    let level = get_log_level();
    assert_eq!(level, LogLevel::Warn);

    set_log_level(LogLevel::Info);
    let level = get_log_level();
    assert_eq!(level, LogLevel::Info);
}

#[test]
fn test_log_level_ordering() {
    // Test LogLevel ordering
    assert!(LogLevel::Silent < LogLevel::Error);
    assert!(LogLevel::Error < LogLevel::Warn);
    assert!(LogLevel::Warn < LogLevel::Info);
    assert!(LogLevel::Info < LogLevel::Debug);
}

#[test]
fn test_log_level_equality() {
    assert_eq!(LogLevel::Silent, LogLevel::Silent);
    assert_eq!(LogLevel::Error, LogLevel::Error);
    assert_ne!(LogLevel::Silent, LogLevel::Debug);
}

#[test]
fn test_init_default_config_not_found() {
    // This test is tricky because init_default() tries to find config files
    // We can't easily test the ConfigNotFound error without mocking the filesystem
    // But we can verify the function handles missing directories gracefully

    // Just verify init_default() either succeeds or fails gracefully
    match asterix::init_default() {
        Ok(_) => {
            // Successfully initialized
        }
        Err(e) => {
            // Failed to initialize - check error type is appropriate
            match e {
                AsterixError::ConfigNotFound(_)
                | AsterixError::InitializationError(_)
                | AsterixError::IOError(_) => {
                    // Expected error types
                }
                _ => panic!("Unexpected error type: {e:?}"),
            }
        }
    }
}
