//! Example: Export ASTERIX data to JSON
//!
//! This example demonstrates parsing ASTERIX data and exporting to JSON format.
//! Requires the 'serde' feature to be enabled.
//!
//! Usage:
//!   cargo run --example json_export --features serde -- <input_file> [output_file]

#[cfg(not(feature = "serde"))]
fn main() {
    eprintln!("This example requires the 'serde' feature to be enabled.");
    eprintln!("\nRun with:");
    eprintln!("  cargo run --example json_export --features serde -- <input_file>");
    std::process::exit(1);
}

#[cfg(feature = "serde")]
fn main() {
    use asterix::{parse, ParseOptions};
    use std::env;
    use std::fs;
    use std::io::Write;
    use std::process;

    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        eprintln!("Usage: {} <input_file> [output_file]", args[0]);
        eprintln!("\nIf output_file is omitted, JSON is written to stdout.");
        eprintln!("\nExample:");
        eprintln!("  {} input.pcap output.json", args[0]);
        eprintln!("  {} input.raw | jq .", args[0]);
        process::exit(1);
    }

    let input_file = &args[1];
    let output_file = args.get(2).map(|s| s.as_str());

    println!("Reading: {input_file}");

    // Read input file
    let data = match fs::read(input_file) {
        Ok(data) => data,
        Err(e) => {
            eprintln!("Error reading file: {e}");
            process::exit(1);
        }
    };

    println!("File size: {} bytes", data.len());

    // Parse ASTERIX data
    println!("Parsing ASTERIX data...");
    let records = match parse(&data, ParseOptions::default()) {
        Ok(records) => records,
        Err(e) => {
            eprintln!("Parse error: {e}");
            process::exit(1);
        }
    };

    println!("✓ Parsed {} record(s)", records.len());

    // Serialize to JSON
    println!("Serializing to JSON...");
    let json = match serde_json::to_string_pretty(&records) {
        Ok(json) => json,
        Err(e) => {
            eprintln!("JSON serialization error: {e}");
            process::exit(1);
        }
    };

    println!("✓ JSON size: {} bytes", json.len());

    // Write output
    match output_file {
        Some(path) => {
            // Write to file
            match fs::File::create(path) {
                Ok(mut file) => {
                    if let Err(e) = file.write_all(json.as_bytes()) {
                        eprintln!("Error writing file: {e}");
                        process::exit(1);
                    }
                    println!("✓ Written to: {path}");
                }
                Err(e) => {
                    eprintln!("Error creating file: {e}");
                    process::exit(1);
                }
            }
        }
        None => {
            // Write to stdout
            println!("\n{json}");
        }
    }

    println!("\n✓ Export completed successfully");
}
