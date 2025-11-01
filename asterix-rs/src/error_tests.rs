//! Unit tests for error type conversion and handling
//!
//! These tests ensure proper error propagation and conversion
//! between C++ exceptions and Rust Result types.

#[cfg(test)]
mod tests {
    use crate::error::{AsterixError, Result};
    use std::error::Error as StdError;

    #[test]
    fn test_parse_error_creation() {
        let err = AsterixError::ParseError {
            offset: 42,
            message: "Invalid data".to_string(),
        };

        assert!(matches!(err, AsterixError::ParseError { offset: 42, .. }));

        let display = format!("{}", err);
        assert!(display.contains("42"));
        assert!(display.contains("Invalid data"));
    }

    #[test]
    fn test_invalid_category_error() {
        let err = AsterixError::InvalidCategory(255);

        assert!(matches!(err, AsterixError::InvalidCategory(255)));

        let display = format!("{}", err);
        assert!(display.contains("255"));
    }

    #[test]
    fn test_config_not_found_error() {
        let path = "/nonexistent/config.xml";
        let err = AsterixError::ConfigNotFound(path.to_string());

        let display = format!("{}", err);
        assert!(display.contains(path));
    }

    #[test]
    fn test_initialization_error() {
        let msg = "Failed to initialize parser";
        let err = AsterixError::InitializationError(msg.to_string());

        let display = format!("{}", err);
        assert!(display.contains(msg));
    }

    #[test]
    fn test_io_error() {
        let msg = "Failed to read file";
        let err = AsterixError::IOError(msg.to_string());

        let display = format!("{}", err);
        assert!(display.contains(msg));
    }

    #[test]
    fn test_unexpected_eof() {
        let err = AsterixError::UnexpectedEOF;

        let display = format!("{}", err);
        assert!(display.contains("EOF") || display.contains("end of"));
    }

    #[test]
    fn test_internal_error() {
        let msg = "C++ parser crashed";
        let err = AsterixError::InternalError(msg.to_string());

        let display = format!("{}", err);
        assert!(display.contains(msg));
    }

    #[test]
    fn test_error_trait_implementation() {
        let err = AsterixError::ParseError {
            offset: 10,
            message: "Test".to_string(),
        };

        // Should implement std::error::Error
        let _: &dyn StdError = &err;

        // Should have source (None in this case)
        assert!(err.source().is_none());
    }

    #[test]
    fn test_error_debug_format() {
        let err = AsterixError::ParseError {
            offset: 100,
            message: "Test error".to_string(),
        };

        let debug = format!("{:?}", err);
        assert!(debug.contains("ParseError"));
        assert!(debug.contains("100"));
    }

    #[test]
    fn test_error_clone() {
        let err1 = AsterixError::InvalidCategory(42);
        let err2 = err1.clone();

        assert!(matches!(err2, AsterixError::InvalidCategory(42)));
    }

    #[test]
    fn test_result_type_alias() {
        fn returns_error() -> Result<i32> {
            Err(AsterixError::InvalidCategory(1))
        }

        let result = returns_error();
        assert!(result.is_err());
    }

    #[test]
    fn test_error_conversion_patterns() {
        // Test ? operator compatibility
        fn fallible_function() -> Result<()> {
            let _x = returns_error_helper()?;
            Ok(())
        }

        fn returns_error_helper() -> Result<i32> {
            Err(AsterixError::UnexpectedEOF)
        }

        let result = fallible_function();
        assert!(result.is_err());
    }

    #[test]
    fn test_error_equality() {
        // Errors should be comparable for testing
        let err1 = AsterixError::InvalidCategory(42);
        let err2 = AsterixError::InvalidCategory(42);
        let err3 = AsterixError::InvalidCategory(43);

        assert!(matches!(err1, AsterixError::InvalidCategory(42)));
        assert!(matches!(err2, AsterixError::InvalidCategory(42)));
        assert!(!matches!(err3, AsterixError::InvalidCategory(42)));
    }

    #[test]
    fn test_error_message_formatting() {
        let errors = vec![
            AsterixError::ParseError {
                offset: 0,
                message: "msg".to_string(),
            },
            AsterixError::InvalidCategory(1),
            AsterixError::ConfigNotFound("path".to_string()),
            AsterixError::InitializationError("init".to_string()),
            AsterixError::IOError("io".to_string()),
            AsterixError::UnexpectedEOF,
            AsterixError::InternalError("internal".to_string()),
        ];

        // All errors should have non-empty display strings
        for err in errors {
            let display = format!("{}", err);
            assert!(!display.is_empty(), "Error display should not be empty");
        }
    }

    #[test]
    fn test_parse_error_with_context() {
        let err = AsterixError::ParseError {
            offset: 1024,
            message: "Invalid category byte: expected 1-255, got 0".to_string(),
        };

        let display = format!("{}", err);
        assert!(display.contains("1024"));
        assert!(display.contains("Invalid category"));
    }

    #[test]
    fn test_error_propagation() {
        fn inner_function() -> Result<()> {
            Err(AsterixError::InvalidCategory(99))
        }

        fn outer_function() -> Result<()> {
            inner_function()?;
            Ok(())
        }

        match outer_function() {
            Err(AsterixError::InvalidCategory(99)) => { /* expected */ }
            _ => panic!("Error not propagated correctly"),
        }
    }
}
