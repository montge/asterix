//! Example: Query ASTERIX metadata
//!
//! This example demonstrates how to use the describe() function to query
//! metadata about ASTERIX categories, items, fields, and values.
//!
//! Usage:
//!   cargo run --example describe_category -- <category> [item] [field] [value]

use asterix::{describe, init_default};
use std::env;
use std::process;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    // Parse command line
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: {} <category> [item] [field] [value]", args[0]);
        eprintln!("\nExamples:");
        eprintln!("  {} 62                    # Describe category 62", args[0]);
        eprintln!(
            "  {} 62 010                # Describe item I062/010",
            args[0]
        );
        eprintln!("  {} 62 010 SAC            # Describe SAC field", args[0]);
        eprintln!("  {} 62 010 SAC 1          # Describe SAC=1 value", args[0]);
        process::exit(1);
    }

    let category: u8 = match args[1].parse() {
        Ok(c) => c,
        Err(_) => {
            eprintln!("Invalid category number: {}", args[1]);
            process::exit(1);
        }
    };

    let item = args.get(2).map(|s| s.as_str());
    let field = args.get(3).map(|s| s.as_str());
    let value = args.get(4).map(|s| s.as_str());

    // Initialize
    println!("Initializing ASTERIX decoder...");
    init_default()?;

    // Query description
    println!("\nQuerying description...");
    print!("  Category: {}", category);
    if let Some(i) = item {
        print!(", Item: {}", i);
    }
    if let Some(f) = field {
        print!(", Field: {}", f);
    }
    if let Some(v) = value {
        print!(", Value: {}", v);
    }
    println!("\n");

    match describe(category, item, field, value) {
        Ok(description) => {
            println!("Description:");
            println!("{}", description);
        }
        Err(e) => {
            eprintln!("Error getting description: {}", e);
            eprintln!("\nPossible reasons:");
            eprintln!("  - Category {} is not defined", category);
            eprintln!("  - Configuration files not found");
            eprintln!("  - Item/field does not exist");
            process::exit(1);
        }
    }

    Ok(())
}
