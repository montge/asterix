//! Error types for ASTERIX parsing operations
//!
//! This module defines comprehensive error types for all failure modes that can
//! occur during ASTERIX data parsing, configuration, and FFI operations.

use std::fmt;

/// Result type alias for ASTERIX operations
pub type Result<T> = std::result::Result<T, AsterixError>;

/// The main error type for ASTERIX operations
///
/// All public API functions return `Result<T, AsterixError>` to handle errors
/// in a Rust-idiomatic way. This enum covers parsing errors, configuration issues,
/// FFI failures, and I/O problems.
///
/// # Example
/// ```
/// # use asterix::{parse, ParseOptions, AsterixError};
/// # let data = &[];
/// match parse(data, ParseOptions::default()) {
///     Ok(records) => println!("Parsed {} records", records.len()),
///     Err(AsterixError::ParseError { offset, message }) => {
///         eprintln!("Parse failed at byte {}: {}", offset, message);
///     },
///     Err(e) => eprintln!("Other error: {}", e),
/// }
/// ```
#[derive(Debug, Clone)]
pub enum AsterixError {
    /// Parsing failed at a specific byte offset
    ///
    /// This error occurs when the parser encounters invalid ASTERIX data,
    /// incorrect category definitions, or malformed data blocks.
    ParseError {
        /// Byte offset where parsing failed
        offset: usize,
        /// Detailed error message
        message: String,
    },

    /// Invalid or unsupported ASTERIX category
    ///
    /// The category number is not defined in the loaded configuration files.
    InvalidCategory {
        /// The category number that was requested
        category: u8,
        /// Additional context about why it's invalid
        reason: String,
    },

    /// Configuration file not found or inaccessible
    ///
    /// Occurs when trying to load a category definition XML file that
    /// doesn't exist or can't be read.
    ConfigNotFound(String),

    /// Failed to initialize ASTERIX parser
    ///
    /// This can happen due to XML parsing errors, missing required elements,
    /// or invalid category definitions.
    InitializationError(String),

    /// I/O error reading data
    ///
    /// Wraps standard I/O errors that occur when reading ASTERIX data from
    /// files or network sources.
    IOError(String),

    /// Unexpected end of input data
    ///
    /// The parser expected more bytes but reached the end of the input buffer.
    UnexpectedEOF {
        /// Byte offset where EOF was encountered
        offset: usize,
        /// How many bytes were expected
        expected: usize,
    },

    /// Internal error from C++ side
    ///
    /// An unexpected error occurred in the underlying C++ parsing library.
    /// This usually indicates a bug or unsupported edge case.
    InternalError(String),

    /// Invalid data format
    ///
    /// The data doesn't conform to ASTERIX protocol specifications
    /// (e.g., invalid length field, corrupted header).
    InvalidData(String),

    /// Null pointer returned from C++ FFI
    ///
    /// Failed to communicate with the C++ parsing library, typically due to
    /// null pointers, invalid conversions, or memory issues.
    NullPointer(String),

    /// FFI (Foreign Function Interface) error
    ///
    /// General FFI communication failure
    FFIError(String),

    /// XML parsing error in configuration file
    ///
    /// The category definition XML file is malformed or doesn't follow
    /// the ASTERIX DTD specification.
    XMLParseError {
        /// Path to the XML file
        file: String,
        /// Line number where error occurred (if available)
        line: Option<usize>,
        /// Error message from XML parser
        message: String,
    },
}

impl fmt::Display for AsterixError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            AsterixError::ParseError { offset, message } => {
                write!(f, "Parse error at byte offset {offset}: {message}")
            }
            AsterixError::InvalidCategory { category, reason } => {
                write!(f, "Invalid ASTERIX category {category}: {reason}")
            }
            AsterixError::ConfigNotFound(path) => {
                write!(f, "Configuration file not found: {path}")
            }
            AsterixError::InitializationError(msg) => {
                write!(f, "Initialization error: {msg}")
            }
            AsterixError::IOError(msg) => {
                write!(f, "I/O error: {msg}")
            }
            AsterixError::UnexpectedEOF { offset, expected } => {
                write!(
                    f,
                    "Unexpected end of input at byte {offset}: expected {expected} more bytes"
                )
            }
            AsterixError::InternalError(msg) => {
                write!(f, "Internal error: {msg}")
            }
            AsterixError::InvalidData(msg) => {
                write!(f, "Invalid data: {msg}")
            }
            AsterixError::NullPointer(msg) => {
                write!(f, "Null pointer: {msg}")
            }
            AsterixError::FFIError(msg) => {
                write!(f, "FFI error: {msg}")
            }
            AsterixError::XMLParseError {
                file,
                line,
                message,
            } => {
                if let Some(line_num) = line {
                    write!(f, "XML parse error in {file} at line {line_num}: {message}")
                } else {
                    write!(f, "XML parse error in {file}: {message}")
                }
            }
        }
    }
}

impl std::error::Error for AsterixError {}

impl From<std::io::Error> for AsterixError {
    fn from(err: std::io::Error) -> Self {
        AsterixError::IOError(err.to_string())
    }
}

impl From<std::ffi::NulError> for AsterixError {
    fn from(err: std::ffi::NulError) -> Self {
        AsterixError::InvalidData(format!("Null byte in string: {err}"))
    }
}

impl From<std::str::Utf8Error> for AsterixError {
    fn from(err: std::str::Utf8Error) -> Self {
        AsterixError::FFIError(format!("Invalid UTF-8: {err}"))
    }
}

// Helper functions for creating common errors
impl AsterixError {
    /// Create a parse error at a specific offset
    pub fn parse_error(offset: usize, message: impl Into<String>) -> Self {
        AsterixError::ParseError {
            offset,
            message: message.into(),
        }
    }

    /// Create an invalid category error
    pub fn invalid_category(category: u8, reason: impl Into<String>) -> Self {
        AsterixError::InvalidCategory {
            category,
            reason: reason.into(),
        }
    }

    /// Create an initialization error
    pub fn initialization_error(message: impl Into<String>) -> Self {
        AsterixError::InitializationError(message.into())
    }

    /// Create an internal error
    pub fn internal_error(message: impl Into<String>) -> Self {
        AsterixError::InternalError(message.into())
    }

    /// Create an FFI error
    pub fn ffi_error(message: impl Into<String>) -> Self {
        AsterixError::FFIError(message.into())
    }

    /// Create an XML parse error
    pub fn xml_parse_error(
        file: impl Into<String>,
        line: Option<usize>,
        message: impl Into<String>,
    ) -> Self {
        AsterixError::XMLParseError {
            file: file.into(),
            line,
            message: message.into(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_error_display() {
        let err = AsterixError::ParseError {
            offset: 42,
            message: "Invalid category".to_string(),
        };
        let display = err.to_string();
        assert!(display.contains("42"));
        assert!(display.contains("Invalid category"));
    }

    #[test]
    fn test_invalid_category_display() {
        let err = AsterixError::InvalidCategory {
            category: 255,
            reason: "Not defined".to_string(),
        };
        let display = err.to_string();
        assert!(display.contains("255"));
        assert!(display.contains("Not defined"));
    }

    #[test]
    fn test_error_conversion_io() {
        let io_err = std::io::Error::new(std::io::ErrorKind::NotFound, "file not found");
        let asterix_err: AsterixError = io_err.into();
        assert!(matches!(asterix_err, AsterixError::IOError(_)));
    }

    #[test]
    fn test_helper_methods() {
        let err = AsterixError::parse_error(100, "test");
        match err {
            AsterixError::ParseError { offset, .. } => assert_eq!(offset, 100),
            _ => panic!("Wrong error type"),
        }

        let err = AsterixError::invalid_category(62, "test");
        match err {
            AsterixError::InvalidCategory { category, .. } => assert_eq!(category, 62),
            _ => panic!("Wrong error type"),
        }
    }

    #[test]
    fn test_xml_parse_error() {
        let err = AsterixError::xml_parse_error("test.xml", Some(42), "Missing element");
        let display = err.to_string();
        assert!(display.contains("test.xml"));
        assert!(display.contains("42"));
        assert!(display.contains("Missing element"));
    }

    #[test]
    fn test_all_error_display_variants() {
        // ConfigNotFound
        let err = AsterixError::ConfigNotFound("/path/to/config".to_string());
        assert!(err.to_string().contains("/path/to/config"));

        // InitializationError
        let err = AsterixError::InitializationError("init failed".to_string());
        assert!(err.to_string().contains("init failed"));

        // IOError
        let err = AsterixError::IOError("file error".to_string());
        assert!(err.to_string().contains("file error"));

        // UnexpectedEOF
        let err = AsterixError::UnexpectedEOF {
            offset: 100,
            expected: 20,
        };
        let display = err.to_string();
        assert!(display.contains("100"));
        assert!(display.contains("20"));

        // InternalError
        let err = AsterixError::InternalError("internal".to_string());
        assert!(err.to_string().contains("internal"));

        // InvalidData
        let err = AsterixError::InvalidData("bad data".to_string());
        assert!(err.to_string().contains("bad data"));

        // NullPointer
        let err = AsterixError::NullPointer("null ptr".to_string());
        assert!(err.to_string().contains("null ptr"));

        // FFIError
        let err = AsterixError::FFIError("ffi failed".to_string());
        assert!(err.to_string().contains("ffi failed"));

        // XMLParseError without line number
        let err = AsterixError::XMLParseError {
            file: "test.xml".to_string(),
            line: None,
            message: "parse error".to_string(),
        };
        let display = err.to_string();
        assert!(display.contains("test.xml"));
        assert!(display.contains("parse error"));
        assert!(!display.contains("line"));
    }

    #[test]
    #[allow(invalid_from_utf8)]
    fn test_error_from_conversions() {
        // From NulError
        let null_err = std::ffi::CString::new("test\0test").unwrap_err();
        let asterix_err: AsterixError = null_err.into();
        assert!(matches!(asterix_err, AsterixError::InvalidData(_)));

        // From Utf8Error (deliberately invalid UTF-8 to test conversion)
        let bad_utf8: &[u8] = &[0xFF, 0xFE, 0xFD];
        let utf8_err = std::str::from_utf8(bad_utf8).unwrap_err();
        let asterix_err: AsterixError = utf8_err.into();
        assert!(matches!(asterix_err, AsterixError::FFIError(_)));
    }

    #[test]
    fn test_error_helper_constructors() {
        // initialization_error
        let err = AsterixError::initialization_error("init failed");
        assert!(matches!(err, AsterixError::InitializationError(_)));

        // internal_error
        let err = AsterixError::internal_error("internal");
        assert!(matches!(err, AsterixError::InternalError(_)));

        // ffi_error
        let err = AsterixError::ffi_error("ffi failed");
        assert!(matches!(err, AsterixError::FFIError(_)));
    }
}
