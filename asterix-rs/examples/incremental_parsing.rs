//! Example: Incremental parsing of large ASTERIX files
//!
//! This example demonstrates how to use parse_with_offset for processing
//! large ASTERIX data files or streams incrementally.
//!
//! Usage:
//!   cargo run --example incremental_parsing -- <file.asterix> [blocks_per_chunk]

use asterix::{init_default, parse_with_offset, ParseOptions};
use std::env;
use std::fs;
use std::process;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    // Parse command line
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: {} <asterix_file> [blocks_per_chunk]", args[0]);
        eprintln!("\nExample:");
        eprintln!("  {} large_file.pcap 100", args[0]);
        process::exit(1);
    }

    let filename = &args[1];
    let blocks_per_chunk = if args.len() >= 3 {
        args[2].parse().unwrap_or(100)
    } else {
        100
    };

    // Initialize
    println!("Initializing ASTERIX decoder...");
    init_default()?;

    // Read file
    println!("Reading file: {}", filename);
    let data = fs::read(filename)?;
    println!("File size: {} bytes", data.len());

    // Parse incrementally
    let options = ParseOptions {
        verbose: false,
        filter_category: None,
        max_records: None,
    };

    let mut offset = 0;
    let mut total_records = 0;
    let mut chunk_num = 0;

    println!("\nParsing in chunks of {} blocks...\n", blocks_per_chunk);

    loop {
        chunk_num += 1;
        print!("Chunk {}: ", chunk_num);

        let result = match parse_with_offset(&data, offset, blocks_per_chunk, options.clone()) {
            Ok(r) => r,
            Err(e) => {
                eprintln!("\nParse error at offset {}: {}", offset, e);
                break;
            }
        };

        let chunk_records = result.records.len();
        total_records += chunk_records;

        println!(
            "parsed {} records, consumed {} bytes, {} blocks remaining",
            chunk_records,
            result.bytes_consumed - offset,
            result.remaining_blocks
        );

        offset = result.bytes_consumed;

        // Check if we're done
        if result.remaining_blocks == 0 || chunk_records == 0 {
            println!("\nReached end of data");
            break;
        }

        // Safety check to prevent infinite loop
        if offset >= data.len() {
            println!("\nReached end of file");
            break;
        }
    }

    println!("\nParsing complete!");
    println!("Total records parsed: {}", total_records);
    println!("Total bytes processed: {}/{}", offset, data.len());

    Ok(())
}
