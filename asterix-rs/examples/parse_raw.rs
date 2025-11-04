//! Example: Parse raw ASTERIX data from a file
//!
//! This example demonstrates basic ASTERIX parsing from a raw binary file.
//! It shows how to initialize the parser, load data, and iterate through
//! parsed records.
//!
//! Usage:
//!     cargo run --example parse_raw -- <asterix_file>
//!
//! Example:
//!     cargo run --example parse_raw -- ../install/test/sample_cat062_065.raw

use asterix::{init_default, parse, AsterixError, ParseOptions};
use std::env;
use std::fs;
use std::process;

fn main() {
    if let Err(e) = run() {
        eprintln!("Error: {}", e);
        process::exit(1);
    }
}

fn run() -> Result<(), Box<dyn std::error::Error>> {
    // Get filename from command line arguments
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: {} <asterix_file>", args[0]);
        eprintln!("\nExample:");
        eprintln!("    {} ../install/test/sample_cat062_065.raw", args[0]);
        process::exit(1);
    }

    let filename = &args[1];

    println!("ASTERIX Raw Parser Example");
    println!("==========================\n");

    // Step 1: Initialize ASTERIX parser with default configuration
    println!("Initializing ASTERIX parser...");
    init_default()?;
    println!("✓ Parser initialized with default categories\n");

    // Step 2: Read raw ASTERIX data from file
    println!("Reading file: {}", filename);
    let data = fs::read(filename)?;
    println!("✓ Read {} bytes\n", data.len());

    // Step 3: Parse with verbose mode enabled to get descriptions
    println!("Parsing ASTERIX data...");
    let options = ParseOptions {
        verbose: true,         // Include descriptions
        filter_category: None, // Parse all categories
        max_records: Some(10), // Limit to first 10 records for demo
    };

    let records = parse(&data, options)?;
    println!("✓ Parsed {} records\n", records.len());

    // Step 4: Display parsed records
    println!("Parsed Records");
    println!("==============\n");

    for (i, record) in records.iter().enumerate() {
        println!("Record #{}", i + 1);
        println!("  Category:    {}", record.category);
        println!("  Length:      {} bytes", record.length);
        println!("  Timestamp:   {} ms", record.timestamp_ms);
        println!("  CRC:         0x{:08X}", record.crc);
        println!("  Data items:  {}", record.item_count());

        // Show hex data (truncated)
        let hex_preview = if record.hex_data.len() > 32 {
            format!("{}...", &record.hex_data[..32])
        } else {
            record.hex_data.clone()
        };
        println!("  Hex data:    {}", hex_preview);

        // Display data items
        if !record.items.is_empty() {
            println!("\n  Data Items:");
            for (item_id, item) in &record.items {
                println!(
                    "    {} ({})",
                    item_id,
                    item.description
                        .as_ref()
                        .unwrap_or(&"No description".to_string())
                );

                // Show first few fields
                for (field_name, field_value) in item.fields.iter().take(5) {
                    match field_value {
                        asterix::ParsedValue::Integer(v) => {
                            println!("      {}: {}", field_name, v);
                        }
                        asterix::ParsedValue::Float(v) => {
                            println!("      {}: {:.4}", field_name, v);
                        }
                        asterix::ParsedValue::String(v) => {
                            println!("      {}: \"{}\"", field_name, v);
                        }
                        asterix::ParsedValue::Boolean(v) => {
                            println!("      {}: {}", field_name, v);
                        }
                        asterix::ParsedValue::Nested(_) => {
                            println!("      {}: <nested>", field_name);
                        }
                        asterix::ParsedValue::Array(_) => {
                            println!("      {}: <array>", field_name);
                        }
                        _ => {}
                    }
                }

                if item.fields.len() > 5 {
                    println!("      ... and {} more fields", item.fields.len() - 5);
                }
            }
        }

        println!();
    }

    // Summary statistics
    println!("Summary");
    println!("=======");

    let mut category_counts = std::collections::HashMap::new();
    for record in &records {
        *category_counts.entry(record.category).or_insert(0) += 1;
    }

    println!("Records by category:");
    let mut cats: Vec<_> = category_counts.iter().collect();
    cats.sort_by_key(|&(cat, _)| cat);
    for (cat, count) in cats {
        println!("  Category {}: {} records", cat, count);
    }

    let total_items: usize = records.iter().map(|r| r.item_count()).sum();
    println!("\nTotal data items: {}", total_items);
    println!(
        "Average items per record: {:.1}",
        total_items as f64 / records.len().max(1) as f64
    );

    Ok(())
}
