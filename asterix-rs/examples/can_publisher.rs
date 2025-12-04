//! CAN Bus Publisher Example
//!
//! This example demonstrates publishing ASTERIX data over CAN Bus via SocketCAN.
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
//! 2. Monitor CAN traffic (optional):
//!    ```bash
//!    candump vcan0
//!    ```
//!
//! # Running
//!
//! ```bash
//! cargo run --example can_publisher --features can
//! ```

#[cfg(all(feature = "can", target_os = "linux"))]
use asterix::transport::can::{CanConfig, CanPublisher};

#[cfg(all(feature = "can", target_os = "linux"))]
use asterix::init_default;

#[cfg(all(feature = "can", target_os = "linux"))]
fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    println!("=== ASTERIX CAN Bus Publisher Example ===\n");

    // Initialize ASTERIX parser with default categories
    println!("Initializing ASTERIX parser...");
    init_default()?;

    // Create CAN publisher on vcan0
    println!("Opening CAN interface vcan0...");
    let config = CanConfig::new("vcan0")?;
    let publisher = CanPublisher::new(config)?;
    println!("CAN publisher ready on vcan0\n");

    // Example 1: Publish raw ASTERIX data
    println!("Example 1: Publishing raw ASTERIX data");
    let raw_data = vec![
        0x30, 0x00, 0x10, // CAT048, 16 bytes
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,
    ];
    publisher.publish_raw(48, &raw_data)?;
    println!("Published CAT048: {} bytes\n", raw_data.len());

    // Example 2: Publish large data (requires fragmentation)
    println!("Example 2: Publishing large data (fragmentation)");
    let large_data = vec![0x42; 100]; // 100 bytes - requires multiple CAN frames
    publisher.publish_raw(62, &large_data)?;
    println!(
        "Published CAT062: {} bytes (fragmented across ~15 CAN frames)\n",
        large_data.len()
    );

    // Example 3: Continuous publishing
    println!("Example 3: Publishing every second for 10 seconds...");
    for i in 0..10 {
        let data = vec![0x30, 0x00, 0x08, 0xFF, i as u8, 0x01, 0x02, 0x03];
        publisher.publish_raw(48, &data)?;
        println!("  Published message {}/10", i + 1);
        std::thread::sleep(std::time::Duration::from_secs(1));
    }

    println!("\nPublisher example completed!");
    println!("Monitor with: candump vcan0");

    Ok(())
}

#[cfg(not(all(feature = "can", target_os = "linux")))]
fn main() {
    #[cfg(not(target_os = "linux"))]
    eprintln!("This example is only available on Linux (requires SocketCAN).");
    #[cfg(all(target_os = "linux", not(feature = "can")))]
    eprintln!("This example requires the 'can' feature to be enabled.");
    #[cfg(all(target_os = "linux", not(feature = "can")))]
    eprintln!("Run with: cargo run --example can_publisher --features can");
    std::process::exit(1);
}
