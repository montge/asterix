//! Unit tests for data type serialization and conversion
//!
//! Tests ensure proper construction, conversion, and serialization
//! of ASTERIX data structures.

#[cfg(test)]
mod tests {
    use crate::data_types::{AsterixRecord, DataItem, ParsedValue, ParseOptions, ParseResult};
    use std::collections::BTreeMap;

    #[test]
    fn test_asterix_record_creation() {
        let record = AsterixRecord {
            category: 48,
            length: 100,
            timestamp_ms: 1234567890,
            crc: 0xABCDEF12,
            hex_data: "30 00 64".to_string(),
            items: BTreeMap::new(),
        };

        assert_eq!(record.category, 48);
        assert_eq!(record.length, 100);
        assert_eq!(record.timestamp_ms, 1234567890);
    }

    #[test]
    fn test_data_item_creation() {
        let mut fields = BTreeMap::new();
        fields.insert("SAC".to_string(), ParsedValue::Integer(10));
        fields.insert("SIC".to_string(), ParsedValue::Integer(20));

        let item = DataItem {
            description: Some("Data Source Identifier".to_string()),
            fields,
        };

        assert_eq!(item.description, Some("Data Source Identifier".to_string()));
        assert_eq!(item.fields.len(), 2);
    }

    #[test]
    fn test_parsed_value_integer() {
        let value = ParsedValue::Integer(42);

        assert_eq!(value.as_i64(), Some(42));
        assert_eq!(value.as_f64(), None);
    }

    #[test]
    fn test_parsed_value_float() {
        let value = ParsedValue::Float(3.14159);

        assert_eq!(value.as_f64(), Some(3.14159));
        assert_eq!(value.as_i64(), None);
    }

    #[test]
    fn test_parsed_value_string() {
        let value = ParsedValue::String("HELLO".to_string());

        match value {
            ParsedValue::String(s) => assert_eq!(s, "HELLO"),
            _ => panic!("Expected String variant"),
        }
    }

    #[test]
    fn test_parsed_value_boolean() {
        let value_true = ParsedValue::Boolean(true);
        let value_false = ParsedValue::Boolean(false);

        assert!(matches!(value_true, ParsedValue::Boolean(true)));
        assert!(matches!(value_false, ParsedValue::Boolean(false)));
    }

    #[test]
    fn test_parsed_value_bytes() {
        let data = vec![0x01, 0x02, 0x03, 0x04];
        let value = ParsedValue::Bytes(data.clone());

        match value {
            ParsedValue::Bytes(bytes) => assert_eq!(bytes, data),
            _ => panic!("Expected Bytes variant"),
        }
    }

    #[test]
    fn test_parsed_value_nested() {
        let inner = ParsedValue::Integer(42);
        let nested = ParsedValue::Nested(Box::new(inner));

        match nested {
            ParsedValue::Nested(boxed) => {
                assert_eq!(boxed.as_i64(), Some(42));
            }
            _ => panic!("Expected Nested variant"),
        }
    }

    #[test]
    fn test_parse_options_default() {
        let options = ParseOptions::default();

        assert!(!options.verbose);
        assert_eq!(options.filter_category, None);
        assert_eq!(options.max_records, None);
    }

    #[test]
    fn test_parse_options_custom() {
        let options = ParseOptions {
            verbose: true,
            filter_category: Some(62),
            max_records: Some(100),
        };

        assert!(options.verbose);
        assert_eq!(options.filter_category, Some(62));
        assert_eq!(options.max_records, Some(100));
    }

    #[test]
    fn test_parse_options_clone() {
        let options1 = ParseOptions {
            verbose: true,
            filter_category: Some(48),
            max_records: Some(50),
        };

        let options2 = options1.clone();

        assert_eq!(options1.verbose, options2.verbose);
        assert_eq!(options1.filter_category, options2.filter_category);
        assert_eq!(options1.max_records, options2.max_records);
    }

    #[test]
    fn test_parse_result_creation() {
        let result = ParseResult {
            records: vec![],
            bytes_consumed: 1024,
            remaining_blocks: 42,
        };

        assert_eq!(result.bytes_consumed, 1024);
        assert_eq!(result.remaining_blocks, 42);
    }

    #[test]
    fn test_asterix_record_with_items() {
        let mut items = BTreeMap::new();

        let mut sac_sic_fields = BTreeMap::new();
        sac_sic_fields.insert("SAC".to_string(), ParsedValue::Integer(10));
        sac_sic_fields.insert("SIC".to_string(), ParsedValue::Integer(20));

        items.insert(
            "010".to_string(),
            DataItem {
                description: Some("Data Source Identifier".to_string()),
                fields: sac_sic_fields,
            },
        );

        let record = AsterixRecord {
            category: 48,
            length: 100,
            timestamp_ms: 0,
            crc: 0,
            hex_data: String::new(),
            items,
        };

        assert_eq!(record.items.len(), 1);
        assert!(record.items.contains_key("010"));

        let item = &record.items["010"];
        assert_eq!(item.fields.len(), 2);
    }

    #[test]
    fn test_btreemap_ordering() {
        // BTreeMap should maintain sorted order
        let mut items = BTreeMap::new();
        items.insert("040".to_string(), DataItem {
            description: None,
            fields: BTreeMap::new(),
        });
        items.insert("010".to_string(), DataItem {
            description: None,
            fields: BTreeMap::new(),
        });
        items.insert("020".to_string(), DataItem {
            description: None,
            fields: BTreeMap::new(),
        });

        let keys: Vec<_> = items.keys().collect();
        assert_eq!(keys, vec!["010", "020", "040"]);
    }

    #[test]
    fn test_parsed_value_debug() {
        let values = vec![
            ParsedValue::Integer(42),
            ParsedValue::Float(3.14),
            ParsedValue::String("test".to_string()),
            ParsedValue::Boolean(true),
            ParsedValue::Bytes(vec![1, 2, 3]),
        ];

        for value in values {
            let debug = format!("{:?}", value);
            assert!(!debug.is_empty());
        }
    }

    #[test]
    #[cfg(feature = "serde")]
    fn test_serde_serialization() {
        use serde_json;

        let mut fields = BTreeMap::new();
        fields.insert("test".to_string(), ParsedValue::Integer(42));

        let item = DataItem {
            description: Some("Test".to_string()),
            fields,
        };

        let json = serde_json::to_string(&item).expect("Failed to serialize");
        assert!(json.contains("test"));
        assert!(json.contains("42"));

        let _deserialized: DataItem = serde_json::from_str(&json)
            .expect("Failed to deserialize");
    }

    #[test]
    #[cfg(feature = "serde")]
    fn test_serde_record_roundtrip() {
        use serde_json;

        let record = AsterixRecord {
            category: 48,
            length: 50,
            timestamp_ms: 1000,
            crc: 0x12345678,
            hex_data: "30 00 32".to_string(),
            items: BTreeMap::new(),
        };

        let json = serde_json::to_string(&record).expect("Serialization failed");
        let deserialized: AsterixRecord = serde_json::from_str(&json)
            .expect("Deserialization failed");

        assert_eq!(deserialized.category, 48);
        assert_eq!(deserialized.length, 50);
        assert_eq!(deserialized.timestamp_ms, 1000);
    }

    #[test]
    fn test_parsed_value_type_safety() {
        let int_value = ParsedValue::Integer(42);
        let float_value = ParsedValue::Float(3.14);

        // Should not be able to get float from integer
        assert_eq!(int_value.as_f64(), None);

        // Should not be able to get integer from float
        assert_eq!(float_value.as_i64(), None);
    }

    #[test]
    fn test_empty_record() {
        let record = AsterixRecord {
            category: 1,
            length: 3,  // Minimum valid length
            timestamp_ms: 0,
            crc: 0,
            hex_data: String::new(),
            items: BTreeMap::new(),
        };

        assert!(record.items.is_empty());
        assert_eq!(record.category, 1);
    }

    #[test]
    fn test_complex_nested_structure() {
        let mut compound_fields = BTreeMap::new();
        compound_fields.insert("subfield1".to_string(), ParsedValue::Integer(10));
        compound_fields.insert("subfield2".to_string(), ParsedValue::Float(20.5));

        let compound = DataItem {
            description: Some("Compound item".to_string()),
            fields: compound_fields,
        };

        assert_eq!(compound.fields.len(), 2);
    }

    #[test]
    fn test_parse_options_builder_pattern() {
        // Test that options can be built incrementally
        let mut options = ParseOptions::default();
        options.verbose = true;
        options.filter_category = Some(48);

        assert!(options.verbose);
        assert_eq!(options.filter_category, Some(48));
    }

    #[test]
    fn test_large_integer_values() {
        let max_i64 = ParsedValue::Integer(i64::MAX);
        let min_i64 = ParsedValue::Integer(i64::MIN);

        assert_eq!(max_i64.as_i64(), Some(i64::MAX));
        assert_eq!(min_i64.as_i64(), Some(i64::MIN));
    }

    #[test]
    fn test_special_float_values() {
        let infinity = ParsedValue::Float(f64::INFINITY);
        let neg_infinity = ParsedValue::Float(f64::NEG_INFINITY);
        let nan = ParsedValue::Float(f64::NAN);

        assert!(infinity.as_f64().unwrap().is_infinite());
        assert!(neg_infinity.as_f64().unwrap().is_infinite());
        assert!(nan.as_f64().unwrap().is_nan());
    }
}
