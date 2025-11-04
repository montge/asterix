//! Example: Parse an ASTERIX file
//!
//! This example demonstrates basic usage of the asterix crate to parse
//! ASTERIX data from a file.
//!
//! Usage:
//!   cargo run --example parse_file -- <file.asterix>

use asterix::{init_default, parse, AsterixError, ParseOptions};
use std::env;
use std::fs;
use std::process;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    // Get filename from command line
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: {} <asterix_file>", args[0]);
        eprintln!("\nExample:");
        eprintln!("  {} sample.asterix", args[0]);
        process::exit(1);
    }

    let filename = &args[1];

    // Initialize ASTERIX with default config
    println!("Initializing ASTERIX decoder...");
    match init_default() {
        Ok(_) => println!("Initialization successful"),
        Err(e) => {
            eprintln!("Failed to initialize ASTERIX: {e}");
            eprintln!("\nNote: Make sure ASTERIX configuration files are installed.");
            eprintln!("Default locations:");
            eprintln!("  Linux:   ~/.config/asterix/config or /etc/asterix/config");
            eprintln!("  macOS:   ~/Library/Application Support/asterix/config");
            eprintln!("  Windows: C:\\Program Files\\asterix\\config");
            process::exit(1);
        }
    }

    // Read file
    println!("\nReading file: {filename}");
    let data = match fs::read(filename) {
        Ok(d) => d,
        Err(e) => {
            eprintln!("Error reading file: {e}");
            process::exit(1);
        }
    };

    println!("File size: {} bytes", data.len());

    // Parse with verbose output
    let options = ParseOptions {
        verbose: true,
        filter_category: None,
        max_records: None,
    };

    println!("\nParsing ASTERIX data...");
    let records = match parse(&data, options) {
        Ok(r) => r,
        Err(e) => {
            eprintln!("Parse error: {e}");
            match e {
                AsterixError::ParseError { offset, message } => {
                    eprintln!("  Offset: {offset} (0x{offset:X})");
                    eprintln!("  Message: {message}");
                }
                AsterixError::InvalidCategory { category, reason } => {
                    eprintln!("  Category: {category}");
                    eprintln!("  Reason: {reason}");
                }
                _ => {}
            }
            process::exit(1);
        }
    };

    println!("\nParsing successful!");
    println!("Total records: {}", records.len());

    // Display summary
    let mut category_counts = std::collections::HashMap::new();
    for record in &records {
        *category_counts.entry(record.category).or_insert(0) += 1;
    }

    println!("\nRecords by category:");
    for (cat, count) in &category_counts {
        println!("  Category {cat:03}: {count} records");
    }

    // Display first record details
    if let Some(first) = records.first() {
        println!("\nFirst record details:");
        println!("  Category: {}", first.category);
        println!("  Length: {} bytes", first.length);
        println!("  Timestamp: {} ms", first.timestamp_ms);
        println!("  CRC: 0x{:08X}", first.crc);
        println!("  Items: {}", first.items.len());

        if !first.items.is_empty() {
            println!("\n  Data items:");
            for (item_id, item) in &first.items {
                println!("    {item_id}");
                if let Some(desc) = &item.description {
                    println!("      Description: {desc}");
                }
                println!("      Fields: {}", item.fields.len());
            }
        }
    }

    Ok(())
}
