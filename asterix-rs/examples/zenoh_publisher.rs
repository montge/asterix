//! Zenoh Publisher Example
//!
//! This example demonstrates publishing ASTERIX data over Zenoh pub/sub.
//!
//! # Usage
//!
//! ```bash
//! # Run with default config (multicast discovery)
//! cargo run --example zenoh_publisher --features zenoh -- sample.asterix
//!
//! # Connect to a specific router
//! cargo run --example zenoh_publisher --features zenoh -- --router tcp/192.168.1.1:7447 sample.asterix
//! ```

use std::env;
use std::process;

use asterix::{init_default, parse, ParseOptions};

#[cfg(feature = "zenoh")]
use asterix::transport::zenoh::{ZenohConfig, ZenohPublisher};

#[cfg(feature = "zenoh")]
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize logging
    env_logger::init();

    // Parse command line arguments
    let args: Vec<String> = env::args().collect();

    let (router, file_path) = parse_args(&args);

    // Read ASTERIX data
    let data = std::fs::read(&file_path).map_err(|e| {
        eprintln!("Failed to read file '{}': {}", file_path, e);
        e
    })?;

    println!("Read {} bytes from {}", data.len(), file_path);

    // Initialize ASTERIX parser
    init_default()?;

    // Parse ASTERIX data
    let records = parse(&data, ParseOptions::default())?;
    println!("Parsed {} ASTERIX records", records.len());

    // Create Zenoh publisher
    let config = match router {
        Some(endpoint) => {
            println!("Connecting to router: {}", endpoint);
            ZenohConfig::with_router(&endpoint)
        }
        None => {
            println!("Using multicast discovery");
            ZenohConfig::default()
        }
    };

    let publisher = ZenohPublisher::new(config).await?;
    println!("Zenoh publisher created");

    // Publish each record
    for (i, record) in records.iter().enumerate() {
        publisher.publish(record).await?;
        println!(
            "Published record {}/{}: CAT{} ({} items)",
            i + 1,
            records.len(),
            record.category,
            record.items.len()
        );
    }

    println!("\nAll records published successfully!");

    // Close publisher
    publisher.close().await?;

    Ok(())
}

#[cfg(not(feature = "zenoh"))]
fn main() {
    eprintln!("This example requires the 'zenoh' feature.");
    eprintln!("Run with: cargo run --example zenoh_publisher --features zenoh -- <file>");
    process::exit(1);
}

fn parse_args(args: &[String]) -> (Option<String>, String) {
    let mut router = None;
    let mut file_path = None;
    let mut i = 1;

    while i < args.len() {
        match args[i].as_str() {
            "--router" | "-r" => {
                if i + 1 < args.len() {
                    router = Some(args[i + 1].clone());
                    i += 2;
                } else {
                    eprintln!("--router requires an argument");
                    print_usage();
                    process::exit(1);
                }
            }
            "--help" | "-h" => {
                print_usage();
                process::exit(0);
            }
            arg if !arg.starts_with('-') => {
                file_path = Some(arg.to_string());
                i += 1;
            }
            _ => {
                eprintln!("Unknown argument: {}", args[i]);
                print_usage();
                process::exit(1);
            }
        }
    }

    match file_path {
        Some(path) => (router, path),
        None => {
            eprintln!("Error: No input file specified");
            print_usage();
            process::exit(1);
        }
    }
}

fn print_usage() {
    eprintln!("\nUsage: zenoh_publisher [OPTIONS] <FILE>");
    eprintln!("\nOptions:");
    eprintln!("  -r, --router <ENDPOINT>  Connect to Zenoh router (e.g., tcp/192.168.1.1:7447)");
    eprintln!("  -h, --help               Show this help message");
    eprintln!("\nExamples:");
    eprintln!("  zenoh_publisher sample.asterix");
    eprintln!("  zenoh_publisher --router tcp/10.0.0.1:7447 sample.asterix");
}
