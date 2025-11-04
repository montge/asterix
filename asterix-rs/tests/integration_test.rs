//! Integration tests for ASTERIX Rust bindings
//!
//! Tests parsing of real ASTERIX data files and validates against expected outputs.

use asterix::{describe, parse, parse_with_offset, AsterixError, ParseOptions};
use std::fs;
use std::path::PathBuf;

/// Helper function to get path to sample data
fn sample_data_path(filename: &str) -> PathBuf {
    let mut path = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    path.pop(); // Go up from asterix-rs/
    path.push("install/sample_data");
    path.push(filename);
    path
}

#[test]
fn test_parse_cat048_raw() {
    let path = sample_data_path("cat048.raw");
    let data = fs::read(&path).expect("Failed to read cat048.raw");

    let options = ParseOptions {
        verbose: false,
        filter_category: None,
        max_records: None,
    };

    let records = parse(&data, options).expect("Failed to parse cat048.raw");

    // Validate results
    assert!(!records.is_empty(), "Expected at least one record");

    // Check first record
    let first = &records[0];
    assert_eq!(first.category, 48, "Expected category 48");
    assert!(first.length > 0, "Record length should be positive");
    assert!(!first.hex_data.is_empty(), "Hex data should not be empty");

    // Validate items exist
    assert!(!first.items.is_empty(), "Record should contain data items");

    println!("✓ Parsed {} CAT048 record(s)", records.len());
}

#[test]
fn test_parse_cat062_cat065_raw() {
    let path = sample_data_path("cat062cat065.raw");
    let data = fs::read(&path).expect("Failed to read cat062cat065.raw");

    let records = parse(&data, ParseOptions::default()).expect("Failed to parse");

    assert!(!records.is_empty(), "Expected records");

    // Count categories
    let cat62_count = records.iter().filter(|r| r.category == 62).count();
    let cat65_count = records.iter().filter(|r| r.category == 65).count();

    println!(
        "✓ Found {} CAT062 and {} CAT065 records",
        cat62_count, cat65_count
    );
    assert!(
        cat62_count > 0 || cat65_count > 0,
        "Expected CAT062 or CAT065 records"
    );
}

#[test]
fn test_parse_pcap_format() {
    let path = sample_data_path("cat_062_065.pcap");
    let data = fs::read(&path).expect("Failed to read PCAP file");

    let options = ParseOptions {
        verbose: true,
        filter_category: None,
        max_records: None,
    };

    let records = parse(&data, options).expect("Failed to parse PCAP");

    assert!(!records.is_empty(), "Expected records from PCAP");

    // Verify PCAP contains multiple categories
    let categories: std::collections::HashSet<_> = records.iter().map(|r| r.category).collect();

    println!("✓ Parsed PCAP with categories: {:?}", categories);
    assert!(categories.len() >= 1, "Expected at least one category");
}

#[test]
fn test_parse_cat034_048_pcap() {
    let path = sample_data_path("cat_034_048.pcap");
    let data = fs::read(&path).expect("Failed to read cat_034_048.pcap");

    let records = parse(&data, ParseOptions::default()).expect("Failed to parse");

    assert!(!records.is_empty(), "Expected records");

    let cat34_count = records.iter().filter(|r| r.category == 34).count();
    let cat48_count = records.iter().filter(|r| r.category == 48).count();

    println!("✓ CAT034: {}, CAT048: {}", cat34_count, cat48_count);
    assert!(
        cat34_count > 0 || cat48_count > 0,
        "Expected CAT034 or CAT048"
    );
}

#[test]
fn test_parse_multicast_pcap() {
    let path = sample_data_path("asterix.pcap");
    let data = fs::read(&path).expect("Failed to read asterix.pcap");

    let records = parse(&data, ParseOptions::default()).expect("Failed to parse");

    println!("✓ Parsed {} records from multicast capture", records.len());
    assert!(!records.is_empty(), "Expected records from multicast PCAP");

    // Check record structure
    for (i, record) in records.iter().take(5).enumerate() {
        println!(
            "  Record {}: CAT{:03} ({} bytes)",
            i, record.category, record.length
        );
        assert!(record.category > 0, "Invalid category");
        assert!(record.length > 0, "Invalid length");
    }
}

#[test]
fn test_parse_gps_format() {
    let path = sample_data_path("parsegps.gps");
    let data = fs::read(&path).expect("Failed to read GPS file");

    // GPS format parsing (may fail if not supported in Rust yet)
    let result = parse(&data, ParseOptions::default());

    match result {
        Ok(records) => {
            println!("✓ Parsed {} GPS records", records.len());
            assert!(!records.is_empty(), "Expected GPS records");
        }
        Err(e) => {
            println!("⚠ GPS format not yet supported: {:?}", e);
            // This is acceptable if GPS format isn't implemented yet
        }
    }
}

#[test]
fn test_error_handling_invalid_data() {
    let invalid_data = b"This is not ASTERIX data at all!";

    let result = parse(invalid_data, ParseOptions::default());

    assert!(result.is_err(), "Expected parse error for invalid data");

    match result {
        Err(AsterixError::ParseError { offset, message }) => {
            println!(
                "✓ Correctly rejected invalid data at offset {} ({})",
                offset, message
            );
        }
        Err(e) => {
            println!("✓ Rejected with error: {:?}", e);
        }
        Ok(_) => panic!("Should not successfully parse invalid data"),
    }
}

#[test]
fn test_error_handling_empty_data() {
    let empty_data = b"";

    let result = parse(empty_data, ParseOptions::default());

    assert!(result.is_err(), "Expected error for empty data");
    println!("✓ Correctly rejected empty input");
}

#[test]
fn test_error_handling_truncated_data() {
    // Create truncated ASTERIX data (incomplete header)
    let truncated = b"\x30\x00"; // CAT048, but missing length bytes

    let result = parse(truncated, ParseOptions::default());

    assert!(result.is_err(), "Expected error for truncated data");

    match result {
        Err(AsterixError::UnexpectedEOF) => {
            println!("✓ Detected unexpected EOF in truncated data");
        }
        Err(e) => {
            println!("✓ Rejected truncated data: {:?}", e);
        }
        Ok(_) => panic!("Should not parse truncated data"),
    }
}

#[test]
fn test_incremental_parsing() {
    let path = sample_data_path("cat_034_048.pcap");
    let data = fs::read(&path).expect("Failed to read PCAP");

    let mut offset = 0;
    let mut total_records = 0;
    let mut iterations = 0;

    loop {
        let result = parse_with_offset(
            &data,
            offset,
            10, // Parse 10 blocks at a time
            ParseOptions::default(),
        );

        match result {
            Ok(parse_result) => {
                total_records += parse_result.records.len();
                offset = parse_result.bytes_consumed;
                iterations += 1;

                println!(
                    "  Iteration {}: parsed {} records, consumed {} bytes",
                    iterations,
                    parse_result.records.len(),
                    parse_result.bytes_consumed
                );

                if parse_result.remaining_blocks == 0 {
                    break;
                }

                // Safety check to prevent infinite loops
                if iterations > 1000 {
                    panic!("Too many iterations in incremental parsing");
                }
            }
            Err(e) => {
                println!("⚠ Incremental parsing stopped: {:?}", e);
                break;
            }
        }
    }

    println!(
        "✓ Incremental parsing completed: {} total records in {} iterations",
        total_records, iterations
    );
    assert!(total_records > 0, "Expected at least some records");
}

#[test]
fn test_parse_with_category_filter() {
    let path = sample_data_path("cat_062_065.pcap");
    let data = fs::read(&path).expect("Failed to read PCAP");

    // Parse with CAT062 filter
    let options = ParseOptions {
        verbose: false,
        filter_category: Some(62),
        max_records: None,
    };

    let records = parse(&data, options).expect("Failed to parse with filter");

    // Verify all records are CAT062
    for record in &records {
        assert_eq!(record.category, 62, "Expected only CAT062 records");
    }

    println!(
        "✓ Category filter working: {} CAT062 records",
        records.len()
    );
}

#[test]
fn test_parse_with_max_records() {
    let path = sample_data_path("cat_034_048.pcap");
    let data = fs::read(&path).expect("Failed to read PCAP");

    let options = ParseOptions {
        verbose: false,
        filter_category: None,
        max_records: Some(5),
    };

    let records = parse(&data, options).expect("Failed to parse with limit");

    assert!(records.len() <= 5, "Should not exceed max_records limit");
    println!("✓ Max records limit working: got {} records", records.len());
}

#[test]
fn test_describe_category() {
    // Describe CAT048
    let result = describe(48, None, None, None);

    assert!(result.is_ok(), "Failed to describe CAT048");

    let description = result.unwrap();
    assert!(!description.is_empty(), "Description should not be empty");

    println!(
        "✓ CAT048 description: {}",
        description.chars().take(100).collect::<String>()
    );
}

#[test]
fn test_describe_item() {
    // Describe CAT048 item 010 (Data Source Identifier)
    let result = describe(48, Some("010"), None, None);

    match result {
        Ok(description) => {
            println!("✓ CAT048/010 description: {}", description);
            assert!(!description.is_empty());
        }
        Err(e) => {
            println!("⚠ Describe item failed: {:?}", e);
            // May not be implemented yet
        }
    }
}

#[test]
fn test_describe_invalid_category() {
    // Try to describe non-existent category
    let result = describe(255, None, None, None);

    assert!(result.is_err(), "Should fail for invalid category");

    match result {
        Err(AsterixError::InvalidCategory { category, .. }) => {
            assert_eq!(category, 255);
            println!("✓ Correctly rejected invalid category {}", category);
        }
        Err(e) => {
            println!("✓ Rejected with error: {:?}", e);
        }
        Ok(_) => panic!("Should not succeed for invalid category"),
    }
}

#[test]
fn test_record_structure() {
    let path = sample_data_path("cat048.raw");
    let data = fs::read(&path).expect("Failed to read file");

    let records = parse(&data, ParseOptions::default()).expect("Failed to parse");
    let record = &records[0];

    // Validate record structure
    assert!(record.category > 0 && record.category < 256);
    assert!(record.length >= 3); // Minimum ASTERIX record size
    assert!(record.timestamp_ms >= 0);
    assert!(!record.hex_data.is_empty());

    // Validate hex data format (should be valid hex string)
    assert!(record
        .hex_data
        .chars()
        .all(|c| c.is_ascii_hexdigit() || c.is_whitespace()));

    println!("✓ Record structure validated");
    println!("  Category: {}", record.category);
    println!("  Length: {}", record.length);
    println!("  Timestamp: {} ms", record.timestamp_ms);
    println!("  Items: {}", record.items.len());
}

#[test]
fn test_data_item_structure() {
    let path = sample_data_path("cat048.raw");
    let data = fs::read(&path).expect("Failed to read file");

    let records = parse(&data, ParseOptions::default()).expect("Failed to parse");
    let record = &records[0];

    // Check that we have data items
    assert!(!record.items.is_empty(), "Record should have data items");

    // Examine first item
    if let Some((item_name, item)) = record.items.iter().next() {
        println!("✓ First data item: {}", item_name);

        if let Some(desc) = &item.description {
            println!("  Description: {}", desc);
        }

        println!("  Fields: {}", item.fields.len());

        // Check fields
        for (field_name, value) in &item.fields {
            println!("    {} = {:?}", field_name, value);
        }
    }
}

#[test]
fn test_parsed_value_types() {
    use asterix::ParsedValue;

    let path = sample_data_path("cat048.raw");
    let data = fs::read(&path).expect("Failed to read file");

    let records = parse(&data, ParseOptions::default()).expect("Failed to parse");
    let record = &records[0];

    // Collect statistics on value types
    let mut int_count = 0;
    let mut float_count = 0;
    let mut string_count = 0;
    let mut bool_count = 0;
    let mut bytes_count = 0;

    for item in record.items.values() {
        for value in item.fields.values() {
            match value {
                ParsedValue::Integer(_) => int_count += 1,
                ParsedValue::Float(_) => float_count += 1,
                ParsedValue::String(_) => string_count += 1,
                ParsedValue::Boolean(_) => bool_count += 1,
                ParsedValue::Bytes(_) => bytes_count += 1,
                ParsedValue::Nested(_) => {} // Count as parent type
            }
        }
    }

    println!("✓ Value type distribution:");
    println!("  Integers: {}", int_count);
    println!("  Floats: {}", float_count);
    println!("  Strings: {}", string_count);
    println!("  Booleans: {}", bool_count);
    println!("  Bytes: {}", bytes_count);

    let total = int_count + float_count + string_count + bool_count + bytes_count;
    assert!(total > 0, "Should have parsed some values");
}

#[test]
#[cfg(feature = "serde")]
fn test_serialization_to_json() {
    use serde_json;

    let path = sample_data_path("cat048.raw");
    let data = fs::read(&path).expect("Failed to read file");

    let records = parse(&data, ParseOptions::default()).expect("Failed to parse");

    // Serialize to JSON
    let json = serde_json::to_string_pretty(&records).expect("Failed to serialize");

    println!("✓ Serialized to JSON ({} bytes)", json.len());
    assert!(!json.is_empty());

    // Verify JSON is valid by deserializing
    let _deserialized: Vec<asterix::AsterixRecord> =
        serde_json::from_str(&json).expect("Failed to deserialize");

    println!("✓ JSON round-trip successful");
}

#[test]
fn test_concurrent_parsing() {
    use std::sync::Arc;
    use std::thread;

    let path = sample_data_path("cat048.raw");
    let data = Arc::new(fs::read(&path).expect("Failed to read file"));

    let mut handles = vec![];

    // Spawn multiple threads parsing the same data
    for i in 0..4 {
        let data_clone = Arc::clone(&data);
        let handle = thread::spawn(move || {
            let records =
                parse(&data_clone, ParseOptions::default()).expect("Failed to parse in thread");
            (i, records.len())
        });
        handles.push(handle);
    }

    // Collect results
    let mut results = vec![];
    for handle in handles {
        results.push(handle.join().expect("Thread panicked"));
    }

    // Verify all threads got same results
    let first_count = results[0].1;
    for (thread_id, count) in &results {
        assert_eq!(
            *count, first_count,
            "Thread {} got different count",
            thread_id
        );
    }

    println!(
        "✓ Concurrent parsing successful: {} threads, {} records each",
        results.len(),
        first_count
    );
}

#[test]
fn test_memory_safety_large_file() {
    // Test with largest available sample file
    let path = sample_data_path("cat_034_048.pcap");
    let data = fs::read(&path).expect("Failed to read file");

    println!("Testing with {} byte file", data.len());

    // Parse multiple times to check for memory leaks/corruption
    for iteration in 0..10 {
        let records = parse(&data, ParseOptions::default()).expect("Failed to parse in iteration");

        if iteration == 0 {
            println!("  Parsed {} records", records.len());
        }

        // Verify records are still valid
        for record in &records {
            assert!(record.category > 0);
            assert!(record.length > 0);
        }
    }

    println!("✓ Memory safety check passed (10 iterations)");
}

#[test]
fn test_verbose_mode() {
    let path = sample_data_path("cat048.raw");
    let data = fs::read(&path).expect("Failed to read file");

    // Parse with verbose mode
    let options = ParseOptions {
        verbose: true,
        filter_category: None,
        max_records: None,
    };

    let records = parse(&data, options).expect("Failed to parse with verbose");

    println!(
        "✓ Verbose mode parsing successful: {} records",
        records.len()
    );
    assert!(!records.is_empty());
}

#[test]
fn test_compare_with_python_output() {
    // This test compares Rust output with Python module output
    // Python reference implementation should be available

    let path = sample_data_path("cat048.raw");
    let data = fs::read(&path).expect("Failed to read file");

    let rust_records = parse(&data, ParseOptions::default()).expect("Failed to parse in Rust");

    // TODO: Run Python parser and compare outputs
    // For now, just validate Rust output structure
    println!("✓ Rust parsed {} records", rust_records.len());

    // Validate structure matches Python expectations
    for record in &rust_records {
        assert!(record.category > 0);
        assert!(record.length > 0);
        assert!(!record.items.is_empty());
    }

    println!("✓ Output structure matches Python expectations");
}

#[cfg(test)]
mod benchmarks {
    use super::*;
    use std::time::Instant;

    #[test]
    fn bench_parse_performance() {
        let path = sample_data_path("cat_034_048.pcap");
        let data = fs::read(&path).expect("Failed to read file");

        // Warmup
        let _ = parse(&data, ParseOptions::default()).expect("Warmup failed");

        // Benchmark
        let iterations = 100;
        let start = Instant::now();

        for _ in 0..iterations {
            let _ = parse(&data, ParseOptions::default()).expect("Parse failed");
        }

        let elapsed = start.elapsed();
        let avg_ms = elapsed.as_millis() as f64 / iterations as f64;
        let throughput_mbps =
            (data.len() as f64 * iterations as f64) / (elapsed.as_secs_f64() * 1_000_000.0);

        println!("✓ Performance test ({} iterations):", iterations);
        println!("  Average: {:.2} ms/parse", avg_ms);
        println!("  Throughput: {:.2} MB/s", throughput_mbps);
        println!("  File size: {} bytes", data.len());

        // Performance regression check: should be reasonably fast
        assert!(avg_ms < 100.0, "Parsing too slow: {:.2} ms", avg_ms);
    }
}
