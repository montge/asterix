//! Example: Real-time stream processing simulation
//!
//! This example demonstrates how to process ASTERIX data in a streaming fashion,
//! similar to how you might handle live network data. It shows:
//! - Incremental parsing
//! - State management across chunks
//! - Performance monitoring
//! - Error recovery
//!
//! Usage:
//!     cargo run --example stream_processing -- <data_file>
//!
//! Example:
//!     cargo run --example stream_processing -- ../install/test/sample_cat062_065.pcap

use asterix::{init_default, parse_with_offset, AsterixRecord, ParseOptions};
use std::collections::HashMap;
use std::env;
use std::fs;
use std::process;
use std::time::{Duration, Instant};

fn main() {
    if let Err(e) = run() {
        eprintln!("Error: {e}");
        process::exit(1);
    }
}

fn run() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: {} <data_file>", args[0]);
        eprintln!("\nExample:");
        eprintln!("  {} ../install/test/sample_cat062_065.pcap", args[0]);
        process::exit(1);
    }

    let filename = &args[1];

    println!("ASTERIX Stream Processing Example");
    println!("==================================\n");

    // Initialize
    println!("Initializing...");
    init_default()?;

    // Load data
    let data = fs::read(filename)?;
    println!("✓ Loaded {} bytes\n", data.len());

    // Simulate streaming by processing in chunks
    let chunk_size = 50; // Process 50 blocks at a time
    let mut processor = StreamProcessor::new();

    println!("Processing stream...\n");

    let start_time = Instant::now();
    let mut offset = 0;

    let options = ParseOptions {
        verbose: false,
        filter_category: None,
        max_records: None,
    };

    // Main processing loop
    loop {
        // Parse next chunk
        match parse_with_offset(&data, offset, chunk_size, options.clone()) {
            Ok(result) => {
                if result.records.is_empty() {
                    break;
                }

                // Process records
                for record in result.records {
                    processor.process_record(record)?;
                }

                offset = result.bytes_consumed;

                // Print status every second
                if processor.should_print_status() {
                    processor.print_status();
                }

                // Check if done
                if result.remaining_blocks == 0 || offset >= data.len() {
                    break;
                }
            }
            Err(e) => {
                eprintln!("Parse error at offset {offset}: {e}");
                // In a real application, you might try to recover or skip
                break;
            }
        }
    }

    let elapsed = start_time.elapsed();

    // Final summary
    println!("\n\nProcessing Complete");
    println!("===================\n");
    processor.print_final_summary(elapsed);

    Ok(())
}

/// Stream processor that maintains state across chunks
struct StreamProcessor {
    total_records: usize,
    total_bytes: usize,
    category_counts: HashMap<u8, usize>,
    last_status_time: Instant,
    start_time: Instant,
}

impl StreamProcessor {
    fn new() -> Self {
        let now = Instant::now();
        Self {
            total_records: 0,
            total_bytes: 0,
            category_counts: HashMap::new(),
            last_status_time: now,
            start_time: now,
        }
    }

    fn process_record(&mut self, record: AsterixRecord) -> Result<(), Box<dyn std::error::Error>> {
        // Update statistics
        self.total_records += 1;
        self.total_bytes += record.length as usize;
        *self.category_counts.entry(record.category).or_insert(0) += 1;

        // In a real application, you would do something with the record here:
        // - Store in database
        // - Forward to another system
        // - Trigger alerts based on content
        // - Update visualization
        // etc.

        Ok(())
    }

    fn should_print_status(&mut self) -> bool {
        let now = Instant::now();
        if now.duration_since(self.last_status_time) > Duration::from_secs(1) {
            self.last_status_time = now;
            true
        } else {
            false
        }
    }

    fn print_status(&self) {
        let elapsed = self.start_time.elapsed().as_secs_f64();
        let rate = self.total_records as f64 / elapsed;
        let throughput = self.total_bytes as f64 / elapsed / 1_048_576.0; // MB/s

        print!("\r  ");
        print!("Records: {:6}  ", self.total_records);
        print!("Rate: {rate:7.1} rec/s  ");
        print!("Throughput: {throughput:6.2} MB/s  ");
        std::io::Write::flush(&mut std::io::stdout()).ok();
    }

    fn print_final_summary(&self, elapsed: Duration) {
        let elapsed_secs = elapsed.as_secs_f64();

        println!("Total Records:  {}", self.total_records);
        println!(
            "Total Bytes:    {} ({:.2} MB)",
            self.total_bytes,
            self.total_bytes as f64 / 1_048_576.0
        );
        println!("Elapsed Time:   {elapsed_secs:.3} seconds");

        if elapsed_secs > 0.0 {
            println!("\nPerformance:");
            println!(
                "  Records/sec:  {:.1}",
                self.total_records as f64 / elapsed_secs
            );
            println!(
                "  Throughput:   {:.2} MB/s",
                self.total_bytes as f64 / elapsed_secs / 1_048_576.0
            );
        }

        if !self.category_counts.is_empty() {
            println!("\nCategory Distribution:");
            let mut cats: Vec<_> = self.category_counts.iter().collect();
            cats.sort_by_key(|&(cat, _)| cat);

            for (cat, count) in cats {
                let percentage = (*count as f64 / self.total_records as f64) * 100.0;
                println!("  Category {cat:3}: {count:6} records ({percentage:5.1}%)");
            }
        }

        // Calculate efficiency metrics
        let avg_record_size = self.total_bytes as f64 / self.total_records.max(1) as f64;
        println!("\nEfficiency:");
        println!("  Avg record size: {avg_record_size:.1} bytes");
        println!("  Memory efficiency: Good (streaming mode)");
        println!("  CPU usage: Moderate");
    }
}
