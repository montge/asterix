//! CAN Bus Bidirectional Example
//!
//! This example demonstrates both publishing and subscribing to ASTERIX data
//! over CAN Bus, useful for testing loopback scenarios.
//!
//! # Prerequisites
//!
//! Set up virtual CAN interface (vcan0):
//! ```bash
//! sudo modprobe vcan
//! sudo ip link add dev vcan0 type vcan
//! sudo ip link set up vcan0
//! ```
//!
//! # Running
//!
//! ```bash
//! cargo run --example can_bidirectional --features can
//! ```

#[cfg(feature = "can")]
use asterix::transport::can::{CanConfig, CanPublisher, CanSubscriber};

#[cfg(feature = "can")]
use asterix::init_default;

#[cfg(feature = "can")]
use std::thread;
#[cfg(feature = "can")]
use std::time::Duration;

#[cfg(feature = "can")]
fn main() -> Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    println!("=== ASTERIX CAN Bus Bidirectional Example ===\n");

    // Initialize ASTERIX parser
    init_default()?;

    // Create publisher and subscriber
    let config = CanConfig::new("vcan0")?;
    let publisher = CanPublisher::new(config.clone())?;
    let mut subscriber = CanSubscriber::new(config)?;

    println!("CAN interface ready on vcan0\n");

    // Subscribe to all test categories
    subscriber.subscribe(48)?;
    subscriber.subscribe(62)?;
    subscriber.subscribe(65)?;

    // Spawn publisher thread
    let publisher_handle = thread::spawn(move || {
        println!("[Publisher] Starting...\n");

        let test_messages = vec![
            (48, vec![0x30, 0x00, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05]),
            (
                62,
                vec![
                    0x3E, 0x00, 0x10, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44,
                    0x55, 0x66,
                ],
            ),
            (65, vec![0x41; 50]), // Large message (fragmentation test)
        ];

        for i in 0..10 {
            for (category, data) in &test_messages {
                match publisher.publish_raw(*category, data) {
                    Ok(_) => {
                        println!("[Publisher] Sent CAT{:03} ({} bytes)", category, data.len());
                    }
                    Err(e) => {
                        eprintln!("[Publisher] Error sending CAT{category:03}: {e}");
                    }
                }
                thread::sleep(Duration::from_millis(100));
            }
            println!("[Publisher] Round {}/10 completed\n", i + 1);
            thread::sleep(Duration::from_secs(1));
        }

        println!("[Publisher] Finished");
    });

    // Subscriber loop
    println!("[Subscriber] Listening for 12 seconds...\n");

    let start = std::time::Instant::now();
    let mut received_count = 0;

    while start.elapsed() < Duration::from_secs(12) {
        match subscriber.receive_timeout(Duration::from_millis(500)) {
            Ok(Some(sample)) => {
                received_count += 1;
                println!(
                    "[Subscriber] Received CAT{:03}: {} bytes at {} μs",
                    sample.category,
                    sample.data.len(),
                    sample.timestamp
                );
            }
            Ok(None) => {
                // Timeout
            }
            Err(e) => {
                eprintln!("[Subscriber] Error: {e}");
            }
        }
    }

    // Wait for publisher to finish
    publisher_handle.join().unwrap();

    println!("\n=== Summary ===");
    println!("Total messages received: {received_count}");
    println!("Expected: ~30 messages (3 categories × 10 rounds)");

    if received_count > 0 {
        println!("\n✓ Bidirectional communication successful!");
    } else {
        println!("\n✗ No messages received - check vcan0 setup");
    }

    Ok(())
}

#[cfg(not(feature = "can"))]
fn main() {
    eprintln!("This example requires the 'can' feature to be enabled.");
    eprintln!("Run with: cargo run --example can_bidirectional --features can");
    std::process::exit(1);
}
