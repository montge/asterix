//! Example demonstrating CCSDS Space Packet Protocol publisher for ASTERIX data
//!
//! This example shows how to publish ASTERIX surveillance data encapsulated in
//! CCSDS Space Packets, enabling space mission integration and satellite tracking
//! data correlation.
//!
//! # Use Case
//!
//! Ground stations can use this to:
//! - Send ASTERIX radar tracking data to spacecraft via CCSDS uplink
//! - Downlink ASTERIX data from airborne sensors via CCSDS telemetry
//! - Correlate satellite positions (ASTERIX CAT 062) with spacecraft telemetry
//!
//! # Running
//!
//! ```bash
//! # Build with CCSDS feature
//! cargo build --example ccsds_publisher --features ccsds
//!
//! # Run the example
//! cargo run --example ccsds_publisher --features ccsds
//! ```
//!
//! # CCSDS Packet Structure
//!
//! ```text
//! ┌─────────────────────────────────────────────┐
//! │ CCSDS Primary Header (6 bytes)              │
//! ├─────────────────────────────────────────────┤
//! │ - Version: 0                                │
//! │ - Type: Telemetry (0) or Telecommand (1)    │
//! │ - APID: 0x300 + ASTERIX category            │
//! │ - Sequence Count: Auto-incremented          │
//! │ - Data Length: ASTERIX block size - 1       │
//! ├─────────────────────────────────────────────┤
//! │ ASTERIX Data Block (variable)               │
//! │ - CAT 048, 062, 065, etc.                   │
//! └─────────────────────────────────────────────┘
//! ```

use asterix::transport::ccsds::{CcsdsConfig, CcsdsMode, CcsdsPublisher};
use asterix::{init_default, parse, ParseOptions};
use std::error::Error;

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    // Initialize logging
    env_logger::init();

    println!("=== ASTERIX CCSDS Space Packet Publisher ===\n");

    // Initialize ASTERIX parser with default categories
    init_default().map_err(|e| format!("Failed to initialize ASTERIX: {e}"))?;
    println!("✓ ASTERIX parser initialized\n");

    // Create CCSDS publisher configuration
    let config = CcsdsConfig {
        mode: CcsdsMode::Telemetry,
        base_apid: 0x300, // APID range: 0x300-0x3FF for ASTERIX
        use_secondary_header: false,
        max_packet_length: 65536,
        enable_crc: false,
        udp_port: Some(7447),
        multicast_addr: None,
    };

    println!("CCSDS Configuration:");
    println!("  Mode: {:?}", config.mode);
    println!("  Base APID: 0x{:03X}", config.base_apid);
    println!("  Max Packet Length: {} bytes", config.max_packet_length);
    println!(
        "  CRC: {}\n",
        if config.enable_crc {
            "Enabled"
        } else {
            "Disabled"
        }
    );

    // Create CCSDS publisher
    let publisher = CcsdsPublisher::new(config)
        .await
        .map_err(|e| format!("Failed to create CCSDS publisher: {e}"))?;
    println!("✓ CCSDS publisher created\n");

    // Example 1: Publish raw ASTERIX bytes
    println!("Example 1: Publishing Raw ASTERIX Data");
    println!("---------------------------------------");

    let raw_cat048 = vec![
        0x30, 0x00, 0x1E, // CAT 048, length 30 bytes
        0x48, 0x11, 0x06, 0x01, 0x06, 0x01, 0x60,
        // ... (simplified ASTERIX data)
    ];

    publisher
        .publish_raw(48, &raw_cat048)
        .await
        .map_err(|e| format!("Failed to publish raw data: {e}"))?;

    println!("  Published CAT 048 data ({} bytes)", raw_cat048.len());
    println!("  CCSDS APID: 0x{:03X} (0x300 + 48)", 0x300 + 48);
    println!("  Sequence Count: Auto-incremented");
    println!();

    // Example 2: Parse and publish ASTERIX records
    println!("Example 2: Publishing Parsed ASTERIX Records");
    println!("--------------------------------------------");

    // Sample ASTERIX CAT 062 data (System Track Data)
    let cat062_data = vec![
        0x3E, 0x00, 0x32, 0x00, 0x00, 0xC0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    ];

    // Parse ASTERIX data
    let records = parse(&cat062_data, ParseOptions::default())
        .map_err(|e| format!("Failed to parse ASTERIX: {e}"))?;

    println!("  Parsed {} ASTERIX record(s)", records.len());

    for (i, record) in records.iter().enumerate() {
        println!("\n  Record {}:", i + 1);
        println!("    Category: {}", record.category);
        println!("    Length: {} bytes", record.length);
        println!("    Items: {}", record.items.len());

        // Publish the record
        publisher
            .publish(record)
            .await
            .map_err(|e| format!("Failed to publish record: {e}"))?;

        let apid = 0x300 + record.category as u16;
        println!("    Published to APID: 0x{apid:03X}");
    }
    println!();

    // Example 3: Satellite tracking correlation use case
    println!("Example 3: Satellite Tracking Correlation");
    println!("-----------------------------------------");
    println!("Use case: Correlating ASTERIX CAT 062 (System Track Data)");
    println!("          with spacecraft telemetry for space situational awareness.");
    println!();

    // Simulate satellite tracking data
    let satellite_track = create_sample_cat062_track();

    println!("  Simulated satellite track:");
    println!("    Category: CAT 062 (System Track Data)");
    println!("    Track Number: 12345");
    println!("    Latitude: 45.5231° N");
    println!("    Longitude: -122.6765° W");
    println!("    Altitude: 408 km (ISS orbit)");
    println!();

    // Publish satellite tracking data
    publisher
        .publish_raw(62, &satellite_track)
        .await
        .map_err(|e| format!("Failed to publish satellite track: {e}"))?;

    println!("  ✓ Published to CCSDS APID 0x33E (CAT 062)");
    println!("  This data can now be correlated with:");
    println!("    - Spacecraft telemetry (CCSDS TM frames)");
    println!("    - Ground station uplink commands (CCSDS TC frames)");
    println!("    - Space weather data");
    println!();

    // Summary
    println!("=== Summary ===");
    println!("Successfully demonstrated ASTERIX-over-CCSDS integration:");
    println!("  ✓ Published raw ASTERIX bytes");
    println!("  ✓ Published parsed ASTERIX records");
    println!("  ✓ Demonstrated satellite tracking correlation");
    println!();
    println!("APID Mapping Convention:");
    println!("  CAT 048 → APID 0x330 (0x300 + 48)");
    println!("  CAT 062 → APID 0x33E (0x300 + 62)");
    println!("  CAT 065 → APID 0x341 (0x300 + 65)");
    println!();
    println!("Next Steps:");
    println!("  1. Set up UDP/multicast transport for ground station");
    println!("  2. Integrate with spacecraft telemetry processing");
    println!("  3. Implement CRC validation for data integrity");
    println!("  4. Add packet fragmentation for large ASTERIX blocks");

    Ok(())
}

/// Create a sample CAT 062 track for satellite tracking demonstration
fn create_sample_cat062_track() -> Vec<u8> {
    // Simplified CAT 062 data block
    // In a real implementation, this would contain:
    // - I062/010: Data Source Identifier (SAC/SIC)
    // - I062/015: Service Identification
    // - I062/040: Track Number
    // - I062/105: Calculated Position in WGS-84 Coordinates
    // - I062/185: Calculated Track Velocity
    vec![
        0x3E, 0x00, 0x40, // CAT 062, length 64 bytes
        0xF8, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        // ... (simplified tracking data)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00,
    ]
}
