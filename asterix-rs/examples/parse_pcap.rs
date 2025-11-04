//! Example: Parse ASTERIX data from a PCAP file
//!
//! This example demonstrates parsing ASTERIX data encapsulated in PCAP format,
//! which is commonly used for network captures. It shows incremental parsing
//! to handle large files efficiently.
//!
//! Usage:
//!     cargo run --example parse_pcap -- <pcap_file> [--max-records N]
//!
//! Example:
//!     cargo run --example parse_pcap -- ../install/test/sample_cat062_065.pcap
//!     cargo run --example parse_pcap -- capture.pcap --max-records 100

use asterix::{init_default, parse_with_offset, ParseOptions, ParseResult};
use std::collections::HashMap;
use std::env;
use std::fs;
use std::process;
use std::time::Instant;

fn main() {
    if let Err(e) = run() {
        eprintln!("Error: {e}");
        process::exit(1);
    }
}

fn run() -> Result<(), Box<dyn std::error::Error>> {
    // Parse command line arguments
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        print_usage(&args[0]);
        process::exit(1);
    }

    let filename = &args[1];
    let max_records = parse_max_records(&args)?;

    println!("ASTERIX PCAP Parser Example");
    println!("===========================\n");

    // Initialize parser
    println!("Initializing ASTERIX parser...");
    init_default()?;
    println!("✓ Parser initialized\n");

    // Read PCAP file
    println!("Reading PCAP file: {filename}");
    let data = fs::read(filename)?;
    println!(
        "✓ Read {} bytes ({:.2} MB)\n",
        data.len(),
        data.len() as f64 / 1_048_576.0
    );

    // Parse incrementally
    println!("Parsing ASTERIX data from PCAP...");
    let start_time = Instant::now();

    let options = ParseOptions {
        verbose: false, // Disable verbose for performance
        filter_category: None,
        max_records,
    };

    let mut all_records = Vec::new();
    let mut offset = 0;
    let mut blocks_parsed = 0;
    let chunk_size = 100; // Parse 100 blocks at a time

    loop {
        let result: ParseResult = parse_with_offset(&data, offset, chunk_size, options.clone())?;

        let count = result.records.len();
        blocks_parsed += count;
        all_records.extend(result.records);

        offset = result.bytes_consumed;

        // Print progress
        print!("\r  Parsed {blocks_parsed} blocks, {offset} bytes consumed...");
        std::io::Write::flush(&mut std::io::stdout())?;

        // Check if we've reached max_records or end of data
        if let Some(max) = max_records {
            if all_records.len() >= max {
                all_records.truncate(max);
                break;
            }
        }

        if result.remaining_blocks == 0 || offset >= data.len() {
            break;
        }
    }

    let elapsed = start_time.elapsed();
    println!(
        "\n✓ Parsed {} records in {:.3} seconds",
        all_records.len(),
        elapsed.as_secs_f64()
    );
    println!(
        "  Throughput: {:.2} records/sec\n",
        all_records.len() as f64 / elapsed.as_secs_f64()
    );

    // Analyze results
    println!("Analysis");
    println!("========\n");

    // Category distribution
    let mut category_stats: HashMap<u8, CategoryStats> = HashMap::new();
    for record in &all_records {
        let stats = category_stats.entry(record.category).or_default();
        stats.count += 1;
        stats.total_bytes += record.length as usize;
        stats.total_items += record.item_count();
    }

    println!("Category Distribution:");
    let mut cats: Vec<_> = category_stats.iter().collect();
    cats.sort_by_key(|&(cat, _)| cat);

    for (category, stats) in cats {
        println!(
            "  Category {:3}: {:6} records, {:8} bytes, {:6} items (avg {:.1} items/record)",
            category,
            stats.count,
            stats.total_bytes,
            stats.total_items,
            stats.total_items as f64 / stats.count as f64
        );
    }

    // Time range analysis
    if !all_records.is_empty() {
        println!("\nTime Range:");
        let min_ts = all_records.iter().map(|r| r.timestamp_ms).min().unwrap();
        let max_ts = all_records.iter().map(|r| r.timestamp_ms).max().unwrap();
        let duration_ms = max_ts - min_ts;

        println!("  First record: {min_ts} ms");
        println!("  Last record:  {max_ts} ms");
        println!("  Duration:     {:.3} seconds", duration_ms as f64 / 1000.0);

        if duration_ms > 0 {
            println!(
                "  Message rate: {:.2} messages/sec",
                all_records.len() as f64 / (duration_ms as f64 / 1000.0)
            );
        }
    }

    // Size statistics
    println!("\nSize Statistics:");
    let total_bytes: usize = all_records.iter().map(|r| r.length as usize).sum();
    let avg_size = total_bytes as f64 / all_records.len().max(1) as f64;
    let min_size = all_records.iter().map(|r| r.length).min().unwrap_or(0);
    let max_size = all_records.iter().map(|r| r.length).max().unwrap_or(0);

    println!("  Total:   {total_bytes} bytes");
    println!("  Average: {avg_size:.1} bytes/record");
    println!("  Min:     {min_size} bytes");
    println!("  Max:     {max_size} bytes");

    // Sample records
    if !all_records.is_empty() {
        println!("\nSample Records (first 3):");
        for (i, record) in all_records.iter().take(3).enumerate() {
            println!("\n  Record #{}:", i + 1);
            println!("    Category: {}", record.category);
            println!("    Length:   {} bytes", record.length);
            println!("    Items:    {}", record.item_count());

            // Show first few item IDs
            let item_ids: Vec<_> = record.items.keys().take(5).collect();
            if !item_ids.is_empty() {
                println!(
                    "    Item IDs: {}",
                    item_ids
                        .iter()
                        .map(|s| s.as_str())
                        .collect::<Vec<_>>()
                        .join(", ")
                );
            }
        }
    }

    println!("\n✓ Analysis complete");

    Ok(())
}

#[derive(Default)]
struct CategoryStats {
    count: usize,
    total_bytes: usize,
    total_items: usize,
}

fn print_usage(program: &str) {
    eprintln!("Usage: {program} <pcap_file> [OPTIONS]");
    eprintln!("\nOptions:");
    eprintln!("  --max-records N    Limit parsing to N records");
    eprintln!("\nExample:");
    eprintln!("  {program} ../install/test/sample_cat062_065.pcap");
    eprintln!("  {program} capture.pcap --max-records 1000");
}

fn parse_max_records(args: &[String]) -> Result<Option<usize>, Box<dyn std::error::Error>> {
    for i in 0..args.len() {
        if args[i] == "--max-records" && i + 1 < args.len() {
            let value = args[i + 1].parse::<usize>()?;
            return Ok(Some(value));
        }
    }
    Ok(None)
}
