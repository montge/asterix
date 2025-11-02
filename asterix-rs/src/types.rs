//! Core data types for ASTERIX records and parsing results
//!
//! This module defines the Rust data structures that represent parsed ASTERIX data.
//! The structures mirror the C++ implementation and Python module API while providing
//! Rust-idiomatic ergonomics.

use serde::{Deserialize, Serialize};
use std::collections::BTreeMap;

/// A single ASTERIX data block containing one or more data records
///
/// # Example
/// ```no_run
/// # use asterix_decoder::AsterixRecord;
/// let record = AsterixRecord {
///     category: 48,
///     length: 256,
///     timestamp_ms: 1234567890000,
///     crc: 0xABCD1234,
///     hex_data: "30003048...".to_string(),
///     items: Default::default(),
/// };
/// println!("Category: {}", record.category);
/// ```
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AsterixRecord {
    /// ASTERIX category number (e.g., 48, 62, 65)
    pub category: u8,

    /// Total length of the data block in bytes
    pub length: u32,

    /// Timestamp in milliseconds since Unix epoch
    pub timestamp_ms: u64,

    /// CRC32 checksum of the data block
    pub crc: u32,

    /// Hexadecimal representation of raw data
    pub hex_data: String,

    /// Map of data item IDs (e.g., "I062/010") to their parsed values
    pub items: BTreeMap<String, DataItem>,
}

impl AsterixRecord {
    /// Get a data item by its ID (e.g., "I062/010")
    ///
    /// # Example
    /// ```no_run
    /// # use asterix_decoder::AsterixRecord;
    /// # let record = AsterixRecord::default();
    /// if let Some(item) = record.get_item("I062/010") {
    ///     println!("Track number: {:?}", item.fields.get("TRK"));
    /// }
    /// ```
    pub fn get_item(&self, item_id: &str) -> Option<&DataItem> {
        self.items.get(item_id)
    }

    /// Check if this record contains a specific data item
    pub fn has_item(&self, item_id: &str) -> bool {
        self.items.contains_key(item_id)
    }

    /// Get the number of data items in this record
    pub fn item_count(&self) -> usize {
        self.items.len()
    }
}

impl Default for AsterixRecord {
    fn default() -> Self {
        Self {
            category: 0,
            length: 0,
            timestamp_ms: 0,
            crc: 0,
            hex_data: String::new(),
            items: BTreeMap::new(),
        }
    }
}

/// A data item within an ASTERIX record
///
/// Data items represent individual fields within an ASTERIX message, such as
/// aircraft position, velocity, identification, etc. Each item contains a
/// description and a map of fields.
///
/// # Structure
/// - Simple items: Single value in fields map
/// - Compound items: Multiple fields in nested structure
/// - Repetitive items: Represented as Vec<DataItem>
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DataItem {
    /// Human-readable description of this data item (if verbose mode enabled)
    #[serde(skip_serializing_if = "Option::is_none")]
    pub description: Option<String>,

    /// Map of field names to their parsed values
    pub fields: BTreeMap<String, ParsedValue>,
}

impl DataItem {
    /// Create a new data item with optional description
    pub fn new(description: Option<String>) -> Self {
        Self {
            description,
            fields: BTreeMap::new(),
        }
    }

    /// Get a field value by name
    pub fn get_field(&self, field_name: &str) -> Option<&ParsedValue> {
        self.fields.get(field_name)
    }

    /// Insert a field value
    pub fn insert_field(&mut self, field_name: String, value: ParsedValue) {
        self.fields.insert(field_name, value);
    }
}

/// Parsed value representing a single data field
///
/// ASTERIX data can contain various types of values: integers, floats, strings,
/// bytes, booleans, as well as nested structures (for compound items) and
/// arrays (for repetitive items).
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(untagged)]
pub enum ParsedValue {
    /// Integer value (signed)
    Integer(i64),

    /// Floating point value
    Float(f64),

    /// String value (callsigns, addresses, etc.)
    String(String),

    /// Boolean value (flags)
    Boolean(bool),

    /// Raw byte array (for binary data)
    Bytes(Vec<u8>),

    /// Nested structure (for compound items)
    Nested(BTreeMap<String, Box<ParsedValue>>),

    /// Array of values (for repetitive items)
    Array(Vec<ParsedValue>),
}

impl ParsedValue {
    /// Try to extract as signed integer
    ///
    /// # Example
    /// ```
    /// # use asterix_decoder::ParsedValue;
    /// let val = ParsedValue::Integer(42);
    /// assert_eq!(val.as_i64(), Some(42));
    /// ```
    pub fn as_i64(&self) -> Option<i64> {
        match self {
            ParsedValue::Integer(v) => Some(*v),
            _ => None,
        }
    }

    /// Try to extract as floating point
    pub fn as_f64(&self) -> Option<f64> {
        match self {
            ParsedValue::Float(v) => Some(*v),
            ParsedValue::Integer(v) => Some(*v as f64),
            _ => None,
        }
    }

    /// Try to extract as string slice
    pub fn as_str(&self) -> Option<&str> {
        match self {
            ParsedValue::String(s) => Some(s.as_str()),
            _ => None,
        }
    }

    /// Try to extract as boolean
    pub fn as_bool(&self) -> Option<bool> {
        match self {
            ParsedValue::Boolean(b) => Some(*b),
            _ => None,
        }
    }

    /// Try to extract as byte slice
    pub fn as_bytes(&self) -> Option<&[u8]> {
        match self {
            ParsedValue::Bytes(b) => Some(b.as_slice()),
            _ => None,
        }
    }

    /// Check if this is a nested structure
    pub fn is_nested(&self) -> bool {
        matches!(self, ParsedValue::Nested(_))
    }

    /// Check if this is an array
    pub fn is_array(&self) -> bool {
        matches!(self, ParsedValue::Array(_))
    }
}

/// Options for parsing ASTERIX data
///
/// # Example
/// ```
/// # use asterix_decoder::ParseOptions;
/// let opts = ParseOptions {
///     verbose: true,
///     filter_category: Some(62),
///     max_records: Some(1000),
/// };
/// ```
#[derive(Debug, Clone)]
pub struct ParseOptions {
    /// Include descriptions and metadata in output (default: false)
    pub verbose: bool,

    /// Only parse records of this category (None = all categories)
    pub filter_category: Option<u8>,

    /// Maximum number of records to parse (None = unlimited)
    pub max_records: Option<usize>,
}

impl Default for ParseOptions {
    fn default() -> Self {
        Self {
            verbose: false,
            filter_category: None,
            max_records: None,
        }
    }
}

/// Result of incremental parsing
///
/// When parsing large data streams, this structure allows tracking progress
/// and resuming parsing from where it left off.
///
/// # Example
/// ```no_run
/// # use asterix_decoder::{parse_with_offset, ParseOptions};
/// # let data = &[];
/// let mut offset = 0;
/// let mut all_records = Vec::new();
///
/// loop {
///     let result = parse_with_offset(data, offset, 100, ParseOptions::default())?;
///     all_records.extend(result.records);
///     offset = result.bytes_consumed;
///
///     if result.remaining_blocks == 0 {
///         break;
///     }
/// }
/// # Ok::<(), asterix_decoder::AsterixError>(())
/// ```
#[derive(Debug)]
pub struct ParseResult {
    /// Parsed ASTERIX records
    pub records: Vec<AsterixRecord>,

    /// Total bytes consumed from input (can be used as next offset)
    pub bytes_consumed: usize,

    /// Estimated number of remaining blocks in input
    pub remaining_blocks: usize,
}

impl ParseResult {
    /// Create a new parse result
    pub fn new(
        records: Vec<AsterixRecord>,
        bytes_consumed: usize,
        remaining_blocks: usize,
    ) -> Self {
        Self {
            records,
            bytes_consumed,
            remaining_blocks,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_asterix_record_default() {
        let record = AsterixRecord::default();
        assert_eq!(record.category, 0);
        assert_eq!(record.length, 0);
        assert!(record.items.is_empty());
    }

    #[test]
    fn test_data_item_new() {
        let item = DataItem::new(Some("Test item".to_string()));
        assert_eq!(item.description, Some("Test item".to_string()));
        assert!(item.fields.is_empty());
    }

    #[test]
    fn test_parsed_value_integer() {
        let val = ParsedValue::Integer(42);
        assert_eq!(val.as_i64(), Some(42));
        assert_eq!(val.as_f64(), Some(42.0));
        assert_eq!(val.as_str(), None);
    }

    #[test]
    fn test_parsed_value_float() {
        let val = ParsedValue::Float(3.14);
        assert_eq!(val.as_f64(), Some(3.14));
        assert_eq!(val.as_i64(), None);
    }

    #[test]
    fn test_parsed_value_string() {
        let val = ParsedValue::String("ABC123".to_string());
        assert_eq!(val.as_str(), Some("ABC123"));
        assert_eq!(val.as_i64(), None);
    }

    #[test]
    fn test_parsed_value_boolean() {
        let val = ParsedValue::Boolean(true);
        assert_eq!(val.as_bool(), Some(true));
    }

    #[test]
    fn test_parsed_value_bytes() {
        let val = ParsedValue::Bytes(vec![0x01, 0x02, 0x03]);
        assert_eq!(val.as_bytes(), Some(&[0x01, 0x02, 0x03][..]));
    }

    #[test]
    fn test_parsed_value_nested() {
        let mut nested = BTreeMap::new();
        nested.insert("field1".to_string(), Box::new(ParsedValue::Integer(10)));
        let val = ParsedValue::Nested(nested);
        assert!(val.is_nested());
        assert!(!val.is_array());
    }

    #[test]
    fn test_parse_options_default() {
        let opts = ParseOptions::default();
        assert!(!opts.verbose);
        assert_eq!(opts.filter_category, None);
        assert_eq!(opts.max_records, None);
    }

    #[test]
    fn test_asterix_record_methods() {
        let mut record = AsterixRecord::default();
        record.category = 48;

        let mut item = DataItem::new(Some("Test".to_string()));
        item.insert_field("SAC".to_string(), ParsedValue::Integer(1));

        record.items.insert("I048/010".to_string(), item);

        assert_eq!(record.item_count(), 1);
        assert!(record.has_item("I048/010"));
        assert!(!record.has_item("I048/020"));

        let item = record.get_item("I048/010").unwrap();
        assert_eq!(item.fields.get("SAC").unwrap().as_i64(), Some(1));
    }
}
