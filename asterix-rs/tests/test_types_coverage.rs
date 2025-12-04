//! Type coverage tests for improved branch coverage
//!
//! These tests target uncovered paths in types.rs, focusing on all methods
//! and edge cases of data structures.

use asterix::{AsterixRecord, DataItem, ParseOptions, ParseResult, ParsedValue};
use std::collections::BTreeMap;

#[test]
fn test_asterix_record_get_item() {
    let mut record = AsterixRecord::default();

    // Test get_item when item doesn't exist
    assert!(record.get_item("I048/010").is_none());

    // Add an item
    let item = DataItem::new(Some("Test".to_string()));
    record.items.insert("I048/010".to_string(), item);

    // Test get_item when item exists
    assert!(record.get_item("I048/010").is_some());
}

#[test]
fn test_asterix_record_has_item() {
    let mut record = AsterixRecord::default();

    // Test has_item when item doesn't exist
    assert!(!record.has_item("I048/010"));

    // Add an item
    let item = DataItem::new(None);
    record.items.insert("I048/010".to_string(), item);

    // Test has_item when item exists
    assert!(record.has_item("I048/010"));
}

#[test]
fn test_asterix_record_item_count() {
    let mut record = AsterixRecord::default();

    // Test item_count when empty
    assert_eq!(record.item_count(), 0);

    // Add items
    record
        .items
        .insert("I048/010".to_string(), DataItem::new(None));
    assert_eq!(record.item_count(), 1);

    record
        .items
        .insert("I048/020".to_string(), DataItem::new(None));
    assert_eq!(record.item_count(), 2);
}

#[test]
fn test_data_item_get_field() {
    let mut item = DataItem::new(None);

    // Test get_field when field doesn't exist
    assert!(item.get_field("SAC").is_none());

    // Add a field
    item.fields
        .insert("SAC".to_string(), ParsedValue::Integer(42));

    // Test get_field when field exists
    assert!(item.get_field("SAC").is_some());
    assert_eq!(item.get_field("SAC").unwrap().as_i64(), Some(42));
}

#[test]
fn test_data_item_insert_field() {
    let mut item = DataItem::new(Some("Test item".to_string()));

    // Test insert_field
    item.insert_field("SAC".to_string(), ParsedValue::Integer(1));
    item.insert_field("SIC".to_string(), ParsedValue::Integer(2));

    assert_eq!(item.fields.len(), 2);
    assert_eq!(item.get_field("SAC").unwrap().as_i64(), Some(1));
    assert_eq!(item.get_field("SIC").unwrap().as_i64(), Some(2));
}

#[test]
fn test_parsed_value_as_i64() {
    let int_val = ParsedValue::Integer(42);
    assert_eq!(int_val.as_i64(), Some(42));

    let float_val = ParsedValue::Float(2.5);
    assert_eq!(float_val.as_i64(), None);

    let str_val = ParsedValue::String("test".to_string());
    assert_eq!(str_val.as_i64(), None);
}

#[test]
fn test_parsed_value_as_f64() {
    let float_val = ParsedValue::Float(2.5);
    assert_eq!(float_val.as_f64(), Some(2.5));

    let int_val = ParsedValue::Integer(42);
    assert_eq!(int_val.as_f64(), Some(42.0));

    let str_val = ParsedValue::String("test".to_string());
    assert_eq!(str_val.as_f64(), None);
}

#[test]
fn test_parsed_value_as_str() {
    let str_val = ParsedValue::String("test".to_string());
    assert_eq!(str_val.as_str(), Some("test"));

    let int_val = ParsedValue::Integer(42);
    assert_eq!(int_val.as_str(), None);
}

#[test]
fn test_parsed_value_as_bool() {
    let bool_val = ParsedValue::Boolean(true);
    assert_eq!(bool_val.as_bool(), Some(true));

    let bool_val2 = ParsedValue::Boolean(false);
    assert_eq!(bool_val2.as_bool(), Some(false));

    let int_val = ParsedValue::Integer(1);
    assert_eq!(int_val.as_bool(), None);
}

#[test]
fn test_parsed_value_as_bytes() {
    let bytes_val = ParsedValue::Bytes(vec![0x01, 0x02, 0x03]);
    assert_eq!(bytes_val.as_bytes(), Some(&[0x01, 0x02, 0x03][..]));

    let int_val = ParsedValue::Integer(42);
    assert_eq!(int_val.as_bytes(), None);
}

#[test]
fn test_parsed_value_is_nested() {
    let mut nested = BTreeMap::new();
    nested.insert("field1".to_string(), Box::new(ParsedValue::Integer(10)));
    let nested_val = ParsedValue::Nested(nested);

    assert!(nested_val.is_nested());
    assert!(!nested_val.is_array());

    let int_val = ParsedValue::Integer(42);
    assert!(!int_val.is_nested());
}

#[test]
fn test_parsed_value_is_array() {
    let array_val = ParsedValue::Array(vec![ParsedValue::Integer(1), ParsedValue::Integer(2)]);

    assert!(array_val.is_array());
    assert!(!array_val.is_nested());

    let int_val = ParsedValue::Integer(42);
    assert!(!int_val.is_array());
}

#[test]
fn test_parse_options_default() {
    let opts = ParseOptions::default();
    assert!(!opts.verbose);
    assert_eq!(opts.filter_category, None);
    assert_eq!(opts.max_records, None);
}

#[test]
fn test_parse_options_custom() {
    let opts = ParseOptions {
        verbose: true,
        filter_category: Some(62),
        max_records: Some(100),
    };

    assert!(opts.verbose);
    assert_eq!(opts.filter_category, Some(62));
    assert_eq!(opts.max_records, Some(100));
}

#[test]
fn test_parse_result_new() {
    let records = vec![AsterixRecord::default()];
    let result = ParseResult::new(records.clone(), 1024, 5);

    assert_eq!(result.records.len(), 1);
    assert_eq!(result.bytes_consumed, 1024);
    assert_eq!(result.remaining_blocks, 5);
}

#[test]
fn test_asterix_record_default() {
    let record = AsterixRecord::default();
    assert_eq!(record.category, 0);
    assert_eq!(record.length, 0);
    assert_eq!(record.timestamp_ms, 0);
    assert_eq!(record.crc, 0);
    assert!(record.hex_data.is_empty());
    assert!(record.items.is_empty());
}

#[test]
fn test_data_item_new_with_description() {
    let item = DataItem::new(Some("Data Source Identifier".to_string()));
    assert_eq!(item.description, Some("Data Source Identifier".to_string()));
    assert!(item.fields.is_empty());
}

#[test]
fn test_data_item_new_without_description() {
    let item = DataItem::new(None);
    assert_eq!(item.description, None);
    assert!(item.fields.is_empty());
}

#[test]
#[cfg(feature = "serde")]
fn test_asterix_record_serialization() {
    use serde_json;

    let record = AsterixRecord {
        category: 48,
        length: 256,
        timestamp_ms: 1234567890000,
        crc: 0xABCD1234,
        hex_data: "30003048".to_string(),
        items: BTreeMap::new(),
    };

    let json = serde_json::to_string(&record).expect("Failed to serialize");
    assert!(json.contains("48"));
    assert!(json.contains("256"));

    let deserialized: AsterixRecord = serde_json::from_str(&json).expect("Failed to deserialize");
    assert_eq!(deserialized.category, 48);
    assert_eq!(deserialized.length, 256);
}

#[test]
#[cfg(feature = "serde")]
fn test_data_item_serialization() {
    use serde_json;

    let mut item = DataItem::new(Some("Test".to_string()));
    item.fields
        .insert("SAC".to_string(), ParsedValue::Integer(1));
    item.fields
        .insert("SIC".to_string(), ParsedValue::Integer(2));

    let json = serde_json::to_string(&item).expect("Failed to serialize");
    assert!(json.contains("SAC"));
    assert!(json.contains("SIC"));

    let deserialized: DataItem = serde_json::from_str(&json).expect("Failed to deserialize");
    assert_eq!(deserialized.description, Some("Test".to_string()));
    assert_eq!(deserialized.fields.len(), 2);
}

#[test]
#[cfg(feature = "serde")]
fn test_parsed_value_serialization_all_types() {
    use serde_json;

    // Integer
    let val = ParsedValue::Integer(42);
    let json = serde_json::to_string(&val).unwrap();
    let _: ParsedValue = serde_json::from_str(&json).unwrap();

    // Float
    let val = ParsedValue::Float(2.5);
    let json = serde_json::to_string(&val).unwrap();
    let _: ParsedValue = serde_json::from_str(&json).unwrap();

    // String
    let val = ParsedValue::String("test".to_string());
    let json = serde_json::to_string(&val).unwrap();
    let _: ParsedValue = serde_json::from_str(&json).unwrap();

    // Boolean
    let val = ParsedValue::Boolean(true);
    let json = serde_json::to_string(&val).unwrap();
    let _: ParsedValue = serde_json::from_str(&json).unwrap();

    // Bytes
    let val = ParsedValue::Bytes(vec![1, 2, 3]);
    let json = serde_json::to_string(&val).unwrap();
    let _: ParsedValue = serde_json::from_str(&json).unwrap();

    // Array
    let val = ParsedValue::Array(vec![ParsedValue::Integer(1), ParsedValue::Integer(2)]);
    let json = serde_json::to_string(&val).unwrap();
    let _: ParsedValue = serde_json::from_str(&json).unwrap();

    // Nested
    let mut nested = BTreeMap::new();
    nested.insert("key".to_string(), Box::new(ParsedValue::Integer(42)));
    let val = ParsedValue::Nested(nested);
    let json = serde_json::to_string(&val).unwrap();
    let _: ParsedValue = serde_json::from_str(&json).unwrap();
}

#[test]
fn test_parsed_value_debug() {
    // Test Debug trait implementation
    let val = ParsedValue::Integer(42);
    let debug_str = format!("{val:?}");
    assert!(debug_str.contains("Integer") || debug_str.contains("42"));
}

#[test]
fn test_asterix_record_clone() {
    let mut record = AsterixRecord {
        category: 48,
        length: 100,
        timestamp_ms: 1234567890,
        crc: 0x12345678,
        hex_data: "test".to_string(),
        items: BTreeMap::new(),
    };

    record
        .items
        .insert("I048/010".to_string(), DataItem::new(None));

    let cloned = record.clone();
    assert_eq!(cloned.category, 48);
    assert_eq!(cloned.length, 100);
    assert_eq!(cloned.items.len(), 1);
}

#[test]
fn test_data_item_clone() {
    let mut item = DataItem::new(Some("Test".to_string()));
    item.fields
        .insert("SAC".to_string(), ParsedValue::Integer(1));

    let cloned = item.clone();
    assert_eq!(cloned.description, Some("Test".to_string()));
    assert_eq!(cloned.fields.len(), 1);
}

#[test]
fn test_parse_options_clone() {
    let opts = ParseOptions {
        verbose: true,
        filter_category: Some(62),
        max_records: Some(100),
    };

    let cloned = opts.clone();
    assert!(cloned.verbose);
    assert_eq!(cloned.filter_category, Some(62));
    assert_eq!(cloned.max_records, Some(100));
}
