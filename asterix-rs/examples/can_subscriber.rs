//! CAN Bus Subscriber Example
//!
//! This example demonstrates receiving ASTERIX data from CAN Bus via SocketCAN.
//!
//! # Prerequisites
//!
//! 1. Set up virtual CAN interface (vcan0):
//!    ```bash
//!    sudo modprobe vcan
//!    sudo ip link add dev vcan0 type vcan
//!    sudo ip link set up vcan0
//!    ```
//!
//! 2. Run the publisher in another terminal:
//!    ```bash
//!    cargo run --example can_publisher --features can
//!    ```
//!
//! # Running
//!
//! ```bash
//! cargo run --example can_subscriber --features can
//! ```

#[cfg(feature = "can")]
use asterix::transport::can::{CanConfig, CanSubscriber};

#[cfg(feature = "can")]
use asterix::{init_default, parse, ParseOptions};

#[cfg(feature = "can")]
fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    println!("=== ASTERIX CAN Bus Subscriber Example ===\n");

    // Initialize ASTERIX parser
    println!("Initializing ASTERIX parser...");
    init_default()?;

    // Create CAN subscriber on vcan0
    println!("Opening CAN interface vcan0...");
    let config = CanConfig::new("vcan0")?;
    let mut subscriber = CanSubscriber::new(config)?;
    println!("CAN subscriber ready on vcan0\n");

    // Subscribe to specific categories
    println!("Subscribing to categories:");
    subscriber.subscribe(48)?;
    println!("  - CAT048 (Monoradar Target Reports)");
    subscriber.subscribe(62)?;
    println!("  - CAT062 (System Track Data)");
    subscriber.subscribe(65)?;
    println!("  - CAT065 (SDPS Service Status Messages)\n");

    println!("Waiting for ASTERIX data... (Press Ctrl+C to stop)\n");

    let mut message_count = 0;

    // Receive loop
    loop {
        match subscriber.receive_timeout(std::time::Duration::from_secs(5)) {
            Ok(Some(sample)) => {
                message_count += 1;
                println!("[Message #{message_count}]");
                println!("  Category:  {}", sample.category);
                println!("  Data size: {} bytes", sample.data.len());
                println!("  Timestamp: {} μs", sample.timestamp);
                println!("  Interface: {}", sample.interface);

                // Try to parse the ASTERIX data
                match parse(&sample.data, ParseOptions::default()) {
                    Ok(records) => {
                        println!("  Records:   {} parsed successfully", records.len());
                        for (idx, record) in records.iter().enumerate() {
                            println!(
                                "    Record {}: CAT{:03} with {} items",
                                idx + 1,
                                record.category,
                                record.items.len()
                            );
                        }
                    }
                    Err(e) => {
                        println!("  Parse error: {e}");
                        println!(
                            "  Raw data: {:02X?}",
                            &sample.data[..sample.data.len().min(32)]
                        );
                    }
                }
                println!();
            }
            Ok(None) => {
                // Timeout - no data received
                if message_count == 0 {
                    println!("No data received yet. Make sure:");
                    println!("  1. vcan0 is up: ip link show vcan0");
                    println!("  2. Publisher is running in another terminal");
                    println!("  3. Or send test data with: cansend vcan0 123#0102030405060708");
                } else {
                    println!("Waiting for more data... ({message_count} messages received so far)");
                }
            }
            Err(e) => {
                eprintln!("Receive error: {e}");
                std::thread::sleep(std::time::Duration::from_millis(100));
            }
        }
    }
}

#[cfg(not(feature = "can"))]
fn main() {
    eprintln!("This example requires the 'can' feature to be enabled.");
    eprintln!("Run with: cargo run --example can_subscriber --features can");
    std::process::exit(1);
}
