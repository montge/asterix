//! Zenoh Subscriber Example
//!
//! This example demonstrates subscribing to ASTERIX data over Zenoh pub/sub.
//!
//! # Usage
//!
//! ```bash
//! # Subscribe to all ASTERIX data (multicast discovery)
//! cargo run --example zenoh_subscriber --features zenoh
//!
//! # Subscribe to specific category
//! cargo run --example zenoh_subscriber --features zenoh -- --filter "asterix/48/**"
//!
//! # Connect to a specific router
//! cargo run --example zenoh_subscriber --features zenoh -- --router tcp/192.168.1.1:7447
//! ```

use std::env;
use std::process;

#[cfg(feature = "zenoh")]
use asterix::transport::zenoh::{ZenohConfig, ZenohSubscriber};

#[cfg(feature = "zenoh")]
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize logging
    env_logger::init();

    // Parse command line arguments
    let args: Vec<String> = env::args().collect();
    let (router, key_expr, max_samples) = parse_args(&args);

    // Create Zenoh config
    let config = match router {
        Some(endpoint) => {
            println!("Connecting to router: {endpoint}");
            ZenohConfig::with_router(&endpoint)
        }
        None => {
            println!("Using multicast discovery");
            ZenohConfig::default()
        }
    };

    println!("Subscribing to: {key_expr}");
    if let Some(max) = max_samples {
        println!("Will receive {max} samples then exit");
    } else {
        println!("Press Ctrl+C to exit");
    }

    // Create subscriber
    let mut subscriber = ZenohSubscriber::new(config, &key_expr).await?;
    println!("Zenoh subscriber created\n");

    let mut count = 0;

    // Receive samples
    while let Some(sample) = subscriber.recv().await {
        count += 1;

        println!("Sample #{count}");
        println!("  Key:       {}", sample.key_expr);
        println!("  Category:  {}", sample.category);
        if let Some(sac) = sample.sac {
            println!("  SAC:       {sac}");
        }
        if let Some(sic) = sample.sic {
            println!("  SIC:       {sic}");
        }
        println!("  Data len:  {} bytes", sample.data.len());
        println!("  Timestamp: {}", sample.timestamp);

        // Show first few bytes of data
        if !sample.data.is_empty() {
            let preview: String = sample
                .data
                .iter()
                .take(16)
                .map(|b| format!("{b:02X}"))
                .collect::<Vec<_>>()
                .join(" ");
            println!("  Data:      {preview} ...");
        }
        println!();

        // Check if we've received enough samples
        if let Some(max) = max_samples {
            if count >= max {
                println!("Received {count} samples, exiting");
                break;
            }
        }
    }

    // Close subscriber
    subscriber.close().await?;

    println!("Total samples received: {count}");

    Ok(())
}

#[cfg(not(feature = "zenoh"))]
fn main() {
    eprintln!("This example requires the 'zenoh' feature.");
    eprintln!("Run with: cargo run --example zenoh_subscriber --features zenoh");
    process::exit(1);
}

fn parse_args(args: &[String]) -> (Option<String>, String, Option<usize>) {
    let mut router = None;
    let mut key_expr = "asterix/**".to_string();
    let mut max_samples = None;
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
            "--filter" | "-f" => {
                if i + 1 < args.len() {
                    key_expr = args[i + 1].clone();
                    i += 2;
                } else {
                    eprintln!("--filter requires an argument");
                    print_usage();
                    process::exit(1);
                }
            }
            "--max" | "-n" => {
                if i + 1 < args.len() {
                    max_samples = args[i + 1].parse().ok();
                    i += 2;
                } else {
                    eprintln!("--max requires a number");
                    print_usage();
                    process::exit(1);
                }
            }
            "--help" | "-h" => {
                print_usage();
                process::exit(0);
            }
            _ => {
                eprintln!("Unknown argument: {}", args[i]);
                print_usage();
                process::exit(1);
            }
        }
    }

    (router, key_expr, max_samples)
}

fn print_usage() {
    eprintln!("\nUsage: zenoh_subscriber [OPTIONS]");
    eprintln!("\nOptions:");
    eprintln!("  -r, --router <ENDPOINT>  Connect to Zenoh router (e.g., tcp/192.168.1.1:7447)");
    eprintln!("  -f, --filter <KEY_EXPR>  Key expression to subscribe to (default: asterix/**)");
    eprintln!("  -n, --max <N>            Exit after receiving N samples");
    eprintln!("  -h, --help               Show this help message");
    eprintln!("\nKey Expression Examples:");
    eprintln!("  asterix/**         All ASTERIX data");
    eprintln!("  asterix/48/**      All CAT048 data");
    eprintln!("  asterix/62/1/2     CAT062 from SAC=1, SIC=2");
    eprintln!("\nExamples:");
    eprintln!("  zenoh_subscriber");
    eprintln!("  zenoh_subscriber --filter \"asterix/48/**\"");
    eprintln!("  zenoh_subscriber --router tcp/10.0.0.1:7447 --max 100");
}
