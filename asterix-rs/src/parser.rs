//! High-level parsing API for ASTERIX data
//!
//! This module provides safe, Rust-idiomatic functions for parsing ASTERIX data.
//! It wraps the unsafe FFI layer and manages memory, error handling, and data conversion.

use crate::error::{AsterixError, Result};
use crate::ffi;
use crate::types::{AsterixRecord, DataItem, ParseOptions, ParseResult, ParsedValue};

use std::collections::BTreeMap;

// Safety limits for FFI boundary validation
const MAX_ASTERIX_MESSAGE_SIZE: usize = 65536; // 64 KB - reasonable max for ASTERIX message
const MAX_BLOCKS_PER_CALL: usize = 10000; // Maximum blocks to parse in single call

/// Parse raw ASTERIX data into structured records
///
/// This is the main entry point for parsing ASTERIX data. It accepts a byte slice
/// containing raw ASTERIX data and returns a vector of parsed records.
///
/// # Arguments
///
/// * `data` - Raw ASTERIX bytes (can be raw binary, PCAP, HDLC, etc.)
/// * `options` - Parsing configuration options
///
/// # Returns
///
/// A vector of parsed ASTERIX records on success, or an error if parsing fails.
///
/// # Example
///
/// ```no_run
/// # use asterix::*;
/// # fn main() -> Result<()> {
/// let data = std::fs::read("sample.asterix")?;
/// let options = ParseOptions {
///     verbose: true,
///     filter_category: Some(62),
///     max_records: Some(1000),
/// };
///
/// let records = parse(&data, options)?;
/// for record in records {
///     println!("Category {}: {} items", record.category, record.items.len());
/// }
/// # Ok(())
/// # }
/// ```
///
/// # Errors
///
/// Returns an error if:
/// - ASTERIX parser is not initialized
/// - Input data is malformed or invalid
/// - Requested category is not defined
/// - C++ parsing layer encounters an error
pub fn parse(data: &[u8], options: ParseOptions) -> Result<Vec<AsterixRecord>> {
    // CRITICAL-005 FIX: Validate input data length
    if data.is_empty() {
        return Err(AsterixError::InvalidData("Empty input data".to_string()));
    }

    if data.len() > MAX_ASTERIX_MESSAGE_SIZE {
        return Err(AsterixError::InvalidData(format!(
            "Input data too large: {} bytes (maximum {} bytes)",
            data.len(),
            MAX_ASTERIX_MESSAGE_SIZE
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

/// Parse ASTERIX data with offset and block count for incremental parsing
///
/// This function allows parsing large data streams incrementally, which is useful
/// for processing live data feeds or very large files without loading everything
/// into memory at once.
///
/// # Arguments
///
/// * `data` - Complete data buffer
/// * `offset` - Byte offset to start parsing from
/// * `blocks_count` - Maximum number of blocks to parse (0 = all)
/// * `options` - Parsing configuration
///
/// # Returns
///
/// A `ParseResult` containing parsed records, bytes consumed, and remaining blocks.
///
/// # Example
///
/// ```no_run
/// # use asterix::*;
/// # fn main() -> Result<()> {
/// let data = std::fs::read("large_file.asterix")?;
/// let mut offset = 0;
/// let mut all_records = Vec::new();
///
/// loop {
///     let result = parse_with_offset(&data, offset, 100, ParseOptions::default())?;
///     all_records.extend(result.records);
///     offset = result.bytes_consumed;
///
///     if result.remaining_blocks == 0 {
///         break;
///     }
/// }
/// # Ok(())
/// # }
/// ```
pub fn parse_with_offset(
    data: &[u8],
    offset: usize,
    blocks_count: usize,
    options: ParseOptions,
) -> Result<ParseResult> {
    // CRITICAL-005 FIX: Validate input data length (same as parse())
    if data.is_empty() {
        return Err(AsterixError::InvalidData("Empty input data".to_string()));
    }

    if data.len() > MAX_ASTERIX_MESSAGE_SIZE {
        return Err(AsterixError::InvalidData(format!(
            "Input data too large: {} bytes (maximum {} bytes)",
            data.len(),
            MAX_ASTERIX_MESSAGE_SIZE
        )));
    }

    // CRITICAL-004 FIX: Validate offset fits in u32 before casting
    if offset > u32::MAX as usize {
        return Err(AsterixError::ParseError {
            offset,
            message: format!(
                "Offset {} exceeds FFI maximum (u32::MAX = {})",
                offset,
                u32::MAX
            ),
        });
    }

    if offset >= data.len() {
        return Err(AsterixError::ParseError {
            offset,
            message: format!("Offset {} exceeds data length {}", offset, data.len()),
        });
    }

    // CRITICAL-004 FIX: Validate blocks_count fits in u32 before casting
    if blocks_count > u32::MAX as usize {
        return Err(AsterixError::InvalidData(format!(
            "blocks_count {} exceeds FFI maximum (u32::MAX = {})",
            blocks_count,
            u32::MAX
        )));
    }

    // HIGH-002 equivalent: Limit blocks_count to reasonable maximum
    if blocks_count > MAX_BLOCKS_PER_CALL {
        return Err(AsterixError::InvalidData(format!(
            "blocks_count {} exceeds maximum ({})",
            blocks_count, MAX_BLOCKS_PER_CALL
        )));
    }

    unsafe {
        // CRITICAL-004 FIX: Safe to cast after validation
        let offset_u32 = offset as u32;
        let blocks_count_u32 = blocks_count as u32;

        let data_ptr = ffi::ffi::asterix_parse_offset(
            data.as_ptr(),
            data.len(),
            offset_u32,
            blocks_count_u32,
            options.verbose,
        );

        if data_ptr.is_null() {
            return Err(AsterixError::NullPointer(
                "C++ parser returned null".to_string(),
            ));
        }

        let records = convert_asterix_data(data_ptr, &options)?;

        // Calculate bytes consumed (this is approximate - C++ wrapper should provide exact value)
        let bytes_consumed = if records.is_empty() {
            offset
        } else {
            offset + records.iter().map(|r| r.length as usize).sum::<usize>()
        };

        let remaining_blocks = if bytes_consumed >= data.len() {
            0
        } else {
            // Estimate remaining blocks (rough calculation)
            (data.len() - bytes_consumed) / 32 // Assume avg 32 bytes per block
        };

        ffi::ffi::asterix_free_data(data_ptr);

        Ok(ParseResult {
            records,
            bytes_consumed,
            remaining_blocks,
        })
    }
}

/// Convert C++ AsterixData to Rust structures
///
/// This internal function marshals data from the C++ side to Rust-native types.
/// It handles all memory management and type conversions.
unsafe fn convert_asterix_data(
    data_ptr: *mut ffi::ffi::AsterixDataWrapper,
    options: &ParseOptions,
) -> Result<Vec<AsterixRecord>> {
    let mut records = Vec::new();

    let block_count = ffi::ffi::asterix_data_block_count(data_ptr);

    for i in 0..block_count {
        let block_ptr = ffi::ffi::asterix_get_data_block(data_ptr, i);

        if block_ptr.is_null() {
            continue;
        }

        // Apply category filter if specified
        let category = ffi::ffi::asterix_block_category(block_ptr);
        if let Some(filter_cat) = options.filter_category {
            if category != filter_cat {
                continue;
            }
        }

        let record = convert_data_block(block_ptr)?;

        records.push(record);

        // Check max records limit
        if let Some(max) = options.max_records {
            if records.len() >= max {
                break;
            }
        }
    }

    Ok(records)
}

/// Convert a single C++ DataBlock to Rust AsterixRecord
unsafe fn convert_data_block(
    block_ptr: *const ffi::ffi::DataBlockWrapper,
) -> Result<AsterixRecord> {
    // HIGH-003 FIX: Validate block_ptr is not null
    if block_ptr.is_null() {
        return Err(AsterixError::NullPointer(
            "C++ returned null data block".to_string(),
        ));
    }

    let category = ffi::ffi::asterix_block_category(block_ptr);
    let length = ffi::ffi::asterix_block_length(block_ptr);
    let timestamp_ms = ffi::ffi::asterix_block_timestamp_ms(block_ptr);
    let crc = ffi::ffi::asterix_block_crc(block_ptr);

    // Get hex data
    let hex_ptr = ffi::ffi::asterix_block_hex_data(block_ptr);
    let hex_data = if !hex_ptr.is_null() {
        std::ffi::CStr::from_ptr(hex_ptr as *const std::os::raw::c_char)
            .to_string_lossy()
            .to_string()
    } else {
        String::new()
    };

    // Parse JSON to extract items
    let json_ptr = ffi::ffi::asterix_block_to_json(block_ptr);
    let items = if !json_ptr.is_null() {
        let json_str = ffi::c_string_to_rust(json_ptr)?;
        parse_items_from_json(&json_str)?
    } else {
        BTreeMap::new()
    };

    Ok(AsterixRecord {
        category,
        length,
        timestamp_ms,
        crc,
        hex_data,
        items,
    })
}

/// Parse data items from JSON representation
///
/// This is a temporary implementation that parses the JSON output from C++.
/// Future versions should use direct C++ struct access for better performance.
fn parse_items_from_json(json_str: &str) -> Result<BTreeMap<String, DataItem>> {
    #[cfg(feature = "serde")]
    {
        use serde_json::Value;

        // Handle empty or whitespace-only JSON (indicates no data was parsed by C++)
        let trimmed = json_str.trim();
        if trimmed.is_empty() || trimmed == "{}" || trimmed == "[]" {
            return Ok(BTreeMap::new());
        }

        // MEDIUM-006 FIX: Validate JSON before parsing
        // Check for obviously malformed JSON (unbalanced braces, control characters)
        let brace_count = json_str
            .chars()
            .fold((0i32, 0i32), |(open, close), c| match c {
                '{' => (open + 1, close),
                '}' => (open, close + 1),
                _ => (open, close),
            });

        if brace_count.0 != brace_count.1 {
            return Err(AsterixError::InvalidData(format!(
                "Malformed JSON from C++: unbalanced braces ({} open, {} close)",
                brace_count.0, brace_count.1
            )));
        }

        // MEDIUM-006 FIX: Parse JSON and return proper error on failure
        // Do not silently swallow JSON parsing errors
        let value: Value = serde_json::from_str(json_str).map_err(|e| {
            AsterixError::InvalidData(format!(
                "Failed to parse JSON from C++: {e}\nJSON snippet: {}",
                &json_str.chars().take(100).collect::<String>()
            ))
        })?;

        let mut items = BTreeMap::new();

        if let Some(obj) = value.as_object() {
            for (key, val) in obj {
                // Skip metadata fields
                if key == "id"
                    || key == "cat"
                    || key == "category"
                    || key == "length"
                    || key == "timestamp"
                    || key == "crc"
                    || key == "hexdata"
                {
                    continue;
                }

                // The actual ASTERIX items are nested under a key like "CAT048"
                // Check if this is a category object containing items
                if key.starts_with("CAT") && val.is_object() {
                    // Extract items from the nested category object
                    if let Some(cat_obj) = val.as_object() {
                        for (item_key, item_val) in cat_obj {
                            let data_item = json_value_to_data_item(item_val)?;
                            items.insert(item_key.clone(), data_item);
                        }
                    }
                } else {
                    // For backward compatibility, also handle top-level items
                    let data_item = json_value_to_data_item(val)?;
                    items.insert(key.clone(), data_item);
                }
            }
        }

        Ok(items)
    }

    #[cfg(not(feature = "serde"))]
    {
        // Without serde, we can't parse JSON - return empty items
        let _ = json_str;
        Ok(BTreeMap::new())
    }
}

/// Convert serde_json::Value to DataItem
#[cfg(feature = "serde")]
fn json_value_to_data_item(value: &serde_json::Value) -> Result<DataItem> {
    let mut data_item = DataItem::new(None);

    if let Some(obj) = value.as_object() {
        for (key, val) in obj {
            let parsed_val = json_value_to_parsed_value(val)?;
            data_item.fields.insert(key.clone(), parsed_val);
        }
    }

    Ok(data_item)
}

/// Convert serde_json::Value to ParsedValue
#[cfg(feature = "serde")]
fn json_value_to_parsed_value(value: &serde_json::Value) -> Result<ParsedValue> {
    use serde_json::Value;

    match value {
        Value::Number(n) => {
            if let Some(i) = n.as_i64() {
                Ok(ParsedValue::Integer(i))
            } else if let Some(f) = n.as_f64() {
                Ok(ParsedValue::Float(f))
            } else {
                Ok(ParsedValue::Integer(0))
            }
        }
        Value::String(s) => Ok(ParsedValue::String(s.clone())),
        Value::Bool(b) => Ok(ParsedValue::Boolean(*b)),
        Value::Array(arr) => {
            let mut parsed_arr = Vec::new();
            for item in arr {
                parsed_arr.push(json_value_to_parsed_value(item)?);
            }
            Ok(ParsedValue::Array(parsed_arr))
        }
        Value::Object(obj) => {
            let mut nested = BTreeMap::new();
            for (key, val) in obj {
                nested.insert(key.clone(), Box::new(json_value_to_parsed_value(val)?));
            }
            Ok(ParsedValue::Nested(nested))
        }
        Value::Null => Ok(ParsedValue::String("null".to_string())),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_empty_data() {
        let data: &[u8] = &[];
        let result = parse(data, ParseOptions::default());
        assert!(result.is_err());
        assert!(matches!(result, Err(AsterixError::InvalidData(_))));
    }

    #[test]
    fn test_parse_options_default() {
        let opts = ParseOptions::default();
        assert!(!opts.verbose);
        assert_eq!(opts.filter_category, None);
        assert_eq!(opts.max_records, None);
    }

    #[cfg(feature = "serde")]
    #[test]
    fn test_parse_items_from_json() {
        let json = r#"{"I062/010": {"SAC": 1, "SIC": 2}}"#;
        let items = parse_items_from_json(json).unwrap();
        assert!(items.contains_key("I062/010"));
    }
}
