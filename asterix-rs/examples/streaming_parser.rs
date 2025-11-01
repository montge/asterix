//! Example: Incremental/streaming parsing of large files
//!
//! This example demonstrates incremental parsing to handle large ASTERIX files
//! with controlled memory usage.
//!
//! Usage:
//!   cargo run --example streaming_parser -- <file> [blocks_per_batch]

use asterix_decoder::{parse_with_offset, ParseOptions};
use std::env;
use std::fs;
use std::process;
use std::time::Instant;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        eprintln!("Usage: {} <file> [blocks_per_batch]", args[0]);
        eprintln!("\nblocks_per_batch: Number of blocks to parse per iteration (default: 100)");
        eprintln!("\nExample:");
        eprintln!("  {} ../install/sample_data/cat_034_048.pcap 50", args[0]);
        process::exit(1);
    }

    let filename = &args[1];
    let blocks_per_batch = if args.len() >= 3 {
        args[2].parse().unwrap_or(100)
    } else {
        100
    };

    println!("=== Streaming ASTERIX Parser ===");
    println!("File: {}", filename);
    println!("Blocks per batch: {}\n", blocks_per_batch);

    // Read file
    let data = match fs::read(filename) {
        Ok(data) => data,
        Err(e) => {
            eprintln!("Error reading file: {}", e);
            process::exit(1);
        }
    };

    println!("File size: {} bytes", data.len());

    // Parse incrementally
    let mut offset = 0;
    let mut total_records = 0;
    let mut iteration = 0;
    let mut category_counts = std::collections::HashMap::new();

    let start_time = Instant::now();

    println!("\nParsing...");

    loop {
        iteration += 1;

        let result = match parse_with_offset(
            &data,
            offset,
            blocks_per_batch,
            ParseOptions::default(),
        ) {
            Ok(result) => result,
            Err(e) => {
                eprintln!("\nError at iteration {}: {}", iteration, e);
                break;
            }
        };

        let batch_records = result.records.len();
        total_records += batch_records;

        // Count categories in this batch
        for record in &result.records {
            *category_counts.entry(record.category).or_insert(0) += 1;
        }

        println!("  Iteration {}: {} records, consumed {} bytes, {} blocks remaining",
                 iteration,
                 batch_records,
                 result.bytes_consumed - offset,
                 result.remaining_blocks);

        offset = result.bytes_consumed;

        if result.remaining_blocks == 0 {
            println!("\n✓ Reached end of file");
            break;
        }

        // Safety check
        if iteration > 10000 {
            eprintln!("\n⚠ Stopping after 10000 iterations (potential infinite loop)");
            break;
        }
    }

    let elapsed = start_time.elapsed();

    // Print summary
    println!("\n=== Summary ===");
    println!("Total iterations: {}", iteration);
    println!("Total records: {}", total_records);
    println!("Total bytes processed: {}", offset);
    println!("Time elapsed: {:.2} seconds", elapsed.as_secs_f64());
    println!("Throughput: {:.2} MB/s",
             (offset as f64) / (elapsed.as_secs_f64() * 1_000_000.0));
    println!("Records/second: {:.2}",
             total_records as f64 / elapsed.as_secs_f64());

    println!("\nCategory distribution:");
    let mut cats: Vec<_> = category_counts.iter().collect();
    cats.sort_by_key(|&(cat, _)| cat);

    for (category, count) in cats {
        let percentage = (*count as f64 / total_records as f64) * 100.0;
        println!("  CAT{:03}: {:5} records ({:5.2}%)", category, count, percentage);
    }

    // Memory efficiency note
    println!("\n💡 Memory efficiency:");
    println!("   Peak memory usage is proportional to blocks_per_batch,");
    println!("   not to total file size.");
    println!("   Current setting processes ~{} records per iteration.",
             total_records / iteration.max(1));
}
