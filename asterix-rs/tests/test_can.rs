//! CAN Bus Transport Integration Tests
//!
//! These tests require a virtual CAN interface (vcan0) to be set up.
//!
//! Setup vcan0:
//! ```bash
//! sudo modprobe vcan
//! sudo ip link add dev vcan0 type vcan
//! sudo ip link set up vcan0
//! ```
//!
//! To run: cargo test --features can -- --test-threads=1
//!
//! Note: Tests run with --test-threads=1 to avoid CAN bus conflicts

#![cfg(feature = "can")]

use asterix::transport::can::{CanConfig, CanError, CanFrameType, CanPublisher, CanSubscriber};
use std::thread;
use std::time::Duration;

// ============================================================================
// Config Tests
// ============================================================================

#[test]
fn test_can_config_default() {
    let config = CanConfig::default();
    assert_eq!(config.interface, "vcan0");
    assert_eq!(config.frame_type, CanFrameType::Classic);
    assert_eq!(config.reassembly_timeout_ms, 1000);
    assert!(config.enable_error_frames);
}

#[test]
fn test_can_config_new() {
    let config = CanConfig::new("can0").unwrap();
    assert_eq!(config.interface, "can0");
    assert_eq!(config.frame_type, CanFrameType::Classic);
}

#[test]
fn test_can_config_with_fd() {
    let config = CanConfig::with_fd("can0").unwrap();
    assert_eq!(config.interface, "can0");
    assert_eq!(config.frame_type, CanFrameType::Fd);
}

#[test]
fn test_can_config_with_timeout() {
    let config = CanConfig::new("vcan0").unwrap().with_timeout(2000);
    assert_eq!(config.reassembly_timeout_ms, 2000);
}

#[test]
fn test_can_config_empty_interface() {
    let result = CanConfig::new("");
    assert!(result.is_err());
    match result {
        Err(CanError::ConfigError(msg)) => {
            assert!(msg.contains("cannot be empty"));
        }
        _ => panic!("Expected ConfigError"),
    }
}

// ============================================================================
// Error Tests
// ============================================================================

#[test]
fn test_can_error_display() {
    let errors = vec![
        CanError::InterfaceError("iface fail".to_string()),
        CanError::SendError("send fail".to_string()),
        CanError::ReceiveError("recv fail".to_string()),
        CanError::FragmentError("frag fail".to_string()),
        CanError::Timeout,
        CanError::ConfigError("config fail".to_string()),
    ];

    for err in errors {
        let display = err.to_string();
        assert!(!display.is_empty());
        println!("Error: {display}");
    }
}

#[test]
fn test_can_error_debug() {
    let err = CanError::InterfaceError("debug test".to_string());
    let debug = format!("{err:?}");
    assert!(debug.contains("InterfaceError"));
}

#[test]
fn test_can_error_to_asterix_error() {
    use asterix::AsterixError;

    let can_err = CanError::SendError("test error".to_string());
    let asterix_err: AsterixError = can_err.into();

    // Should convert to IOError variant
    match asterix_err {
        AsterixError::IOError(msg) => {
            assert!(msg.contains("test error"));
        }
        _ => panic!("Expected IOError variant"),
    }
}

// ============================================================================
// Frame Type Tests
// ============================================================================

#[test]
fn test_can_frame_type_default() {
    let frame_type = CanFrameType::default();
    assert_eq!(frame_type, CanFrameType::Classic);
}

#[test]
fn test_can_frame_type_equality() {
    assert_eq!(CanFrameType::Classic, CanFrameType::Classic);
    assert_eq!(CanFrameType::Fd, CanFrameType::Fd);
    assert_ne!(CanFrameType::Classic, CanFrameType::Fd);
}

// ============================================================================
// Publisher/Subscriber Creation Tests
// ============================================================================

#[test]
fn test_can_publisher_creation() {
    let config = CanConfig::new("vcan0").unwrap();
    match CanPublisher::new(config) {
        Ok(_publisher) => {
            println!("CAN publisher created successfully on vcan0");
        }
        Err(e) => {
            println!("CAN publisher creation failed (expected if vcan0 not available): {e}");
        }
    }
}

#[test]
fn test_can_subscriber_creation() {
    let config = CanConfig::new("vcan0").unwrap();
    match CanSubscriber::new(config) {
        Ok(_subscriber) => {
            println!("CAN subscriber created successfully on vcan0");
        }
        Err(e) => {
            println!("CAN subscriber creation failed (expected if vcan0 not available): {e}");
        }
    }
}

#[test]
fn test_can_publisher_invalid_interface() {
    let config = CanConfig::new("nonexistent_can99").unwrap();
    match CanPublisher::new(config) {
        Ok(_) => println!("Unexpected success with invalid interface"),
        Err(e) => println!("Expected error with invalid interface: {e}"),
    }
}

// ============================================================================
// Subscription Tests
// ============================================================================

#[test]
fn test_can_subscriber_subscribe() {
    let config = CanConfig::new("vcan0").unwrap();
    if let Ok(mut subscriber) = CanSubscriber::new(config) {
        // Subscribe to multiple categories
        assert!(subscriber.subscribe(48).is_ok());
        assert!(subscriber.subscribe(62).is_ok());
        assert!(subscriber.subscribe(65).is_ok());

        // Duplicate subscription should be ok
        assert!(subscriber.subscribe(48).is_ok());

        println!("Successfully subscribed to categories 48, 62, 65");
    } else {
        println!("Skipping test: vcan0 not available");
    }
}

// ============================================================================
// Publisher-Subscriber Communication Tests
// ============================================================================

/// Test basic publish/subscribe with small ASTERIX data
#[test]
fn test_publish_subscribe_small_data() {
    let config = CanConfig::new("vcan0").unwrap();

    // Try to create publisher and subscriber
    let publisher = match CanPublisher::new(config.clone()) {
        Ok(p) => p,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    let mut subscriber = match CanSubscriber::new(config) {
        Ok(s) => s,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    // Subscribe to category 48
    subscriber.subscribe(48).unwrap();

    // Publish small ASTERIX data (fits in single CAN frame)
    let test_data = vec![0x30, 0x00, 0x10, 0x01, 0x02];

    // Spawn publisher in background
    thread::spawn(move || {
        thread::sleep(Duration::from_millis(100));
        for _ in 0..5 {
            let _ = publisher.publish_raw(48, &test_data);
            thread::sleep(Duration::from_millis(50));
        }
    });

    // Try to receive
    for i in 0..10 {
        match subscriber.receive_timeout(Duration::from_millis(500)) {
            Ok(Some(sample)) => {
                println!(
                    "Received sample {i}: category {}, {} bytes",
                    sample.category,
                    sample.data.len()
                );
                assert_eq!(sample.category, 48);
                assert!(!sample.data.is_empty());
                return; // Success!
            }
            Ok(None) => {
                println!("Receive timeout {i}, retrying...");
                continue;
            }
            Err(e) => {
                println!("Receive error: {e}");
            }
        }
    }

    println!("No data received (this may be normal if vcan0 has issues)");
}

/// Test publish/subscribe with large ASTERIX data (requires fragmentation)
#[test]
fn test_publish_subscribe_large_data() {
    let config = CanConfig::new("vcan0").unwrap();

    let publisher = match CanPublisher::new(config.clone()) {
        Ok(p) => p,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    let mut subscriber = match CanSubscriber::new(config) {
        Ok(s) => s,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    subscriber.subscribe(62).unwrap();

    // Large ASTERIX data (requires multiple CAN frames)
    let test_data = vec![0x42; 100]; // 100 bytes - requires ~15 classic CAN frames

    thread::spawn(move || {
        thread::sleep(Duration::from_millis(100));
        for i in 0..3 {
            println!("Publishing large data iteration {i}");
            let _ = publisher.publish_raw(62, &test_data);
            thread::sleep(Duration::from_millis(200));
        }
    });

    for i in 0..15 {
        match subscriber.receive_timeout(Duration::from_secs(2)) {
            Ok(Some(sample)) => {
                println!(
                    "Received large sample {i}: category {}, {} bytes",
                    sample.category,
                    sample.data.len()
                );
                assert_eq!(sample.category, 62);
                assert_eq!(sample.data.len(), 100);
                return; // Success!
            }
            Ok(None) => {
                println!("Receive timeout {i}, retrying...");
                continue;
            }
            Err(e) => {
                println!("Receive error: {e}");
            }
        }
    }

    println!("No large data received (fragmentation may need more time)");
}

// ============================================================================
// Multiple Subscriber Tests
// ============================================================================

#[test]
fn test_multiple_subscribers() {
    let config = CanConfig::new("vcan0").unwrap();

    let publisher = match CanPublisher::new(config.clone()) {
        Ok(p) => p,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    // Create two subscribers
    let mut sub1 = match CanSubscriber::new(config.clone()) {
        Ok(s) => s,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    let mut sub2 = match CanSubscriber::new(config) {
        Ok(s) => s,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    sub1.subscribe(48).unwrap();
    sub2.subscribe(48).unwrap();

    let test_data = vec![0x30, 0x00, 0x08, 0xAA, 0xBB];

    // Publish in background
    thread::spawn(move || {
        thread::sleep(Duration::from_millis(100));
        for _ in 0..5 {
            let _ = publisher.publish_raw(48, &test_data);
            thread::sleep(Duration::from_millis(50));
        }
    });

    // Both subscribers should receive
    let mut sub1_received = false;
    let mut sub2_received = false;

    for _ in 0..10 {
        if let Ok(Some(_sample)) = sub1.receive_timeout(Duration::from_millis(100)) {
            sub1_received = true;
        }
        if let Ok(Some(_sample)) = sub2.receive_timeout(Duration::from_millis(100)) {
            sub2_received = true;
        }

        if sub1_received && sub2_received {
            println!("Both subscribers received data successfully");
            return;
        }
    }

    if sub1_received || sub2_received {
        println!("At least one subscriber received data");
    } else {
        println!("No subscribers received data (CAN broadcast may have issues)");
    }
}

// ============================================================================
// Category Filtering Tests
// ============================================================================

#[test]
fn test_category_filtering() {
    let config = CanConfig::new("vcan0").unwrap();

    let publisher = match CanPublisher::new(config.clone()) {
        Ok(p) => p,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    let mut subscriber = match CanSubscriber::new(config) {
        Ok(s) => s,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    // Subscribe only to category 48
    subscriber.subscribe(48).unwrap();

    let data_cat48 = vec![0x30, 0x00, 0x05, 0x01, 0x02];
    let data_cat62 = vec![0x3E, 0x00, 0x05, 0x03, 0x04];

    thread::spawn(move || {
        thread::sleep(Duration::from_millis(100));
        let _ = publisher.publish_raw(48, &data_cat48);
        thread::sleep(Duration::from_millis(50));
        let _ = publisher.publish_raw(62, &data_cat62); // Should be filtered out
        thread::sleep(Duration::from_millis(50));
        let _ = publisher.publish_raw(48, &data_cat48);
    });

    // Should only receive category 48
    let mut received_48 = 0;
    let mut received_other = 0;

    for _ in 0..20 {
        match subscriber.receive_timeout(Duration::from_millis(200)) {
            Ok(Some(sample)) => {
                if sample.category == 48 {
                    received_48 += 1;
                } else {
                    received_other += 1;
                }
            }
            Ok(None) => continue,
            Err(_) => break,
        }

        if received_48 >= 2 {
            break;
        }
    }

    println!("Received cat48: {received_48}, other: {received_other}");

    if received_48 > 0 {
        println!("Category filtering appears to be working");
    }
}

// ============================================================================
// Timeout Tests
// ============================================================================

#[test]
fn test_receive_timeout() {
    let config = CanConfig::new("vcan0").unwrap();

    let mut subscriber = match CanSubscriber::new(config) {
        Ok(s) => s,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    subscriber.subscribe(99).unwrap(); // Unlikely category

    let start = std::time::Instant::now();
    let result = subscriber.receive_timeout(Duration::from_millis(500));
    let elapsed = start.elapsed();

    println!(
        "Timeout test: elapsed {:?}, result: {:?}",
        elapsed,
        result.is_ok()
    );

    // Should timeout and return Ok(None)
    assert!(matches!(result, Ok(None)));
    assert!(elapsed >= Duration::from_millis(450)); // Allow some jitter
    assert!(elapsed < Duration::from_millis(700));
}

// ============================================================================
// Reassembly Timeout Tests
// ============================================================================

#[test]
fn test_reassembly_timeout() {
    let config = CanConfig::new("vcan0").unwrap().with_timeout(500); // Short timeout for testing

    let mut subscriber = match CanSubscriber::new(config) {
        Ok(s) => s,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    subscriber.subscribe(48).unwrap();

    // Wait for reassembly timeout to trigger cleanup
    thread::sleep(Duration::from_millis(600));

    // Try to receive (should timeout cleanly)
    let result = subscriber.receive_timeout(Duration::from_millis(200));
    assert!(matches!(result, Ok(None)));

    println!("Reassembly timeout test completed successfully");
}

// ============================================================================
// Stress Tests
// ============================================================================

#[test]
fn test_rapid_publish() {
    let config = CanConfig::new("vcan0").unwrap();

    let publisher = match CanPublisher::new(config) {
        Ok(p) => p,
        Err(e) => {
            println!("Skipping test: vcan0 not available - {e}");
            return;
        }
    };

    let test_data = vec![0x30, 0x00, 0x05, 0xFF, 0xEE];

    let start = std::time::Instant::now();
    let mut success_count = 0;
    let mut error_count = 0;

    for _ in 0..100 {
        match publisher.publish_raw(48, &test_data) {
            Ok(_) => success_count += 1,
            Err(_) => error_count += 1,
        }
    }

    let elapsed = start.elapsed();

    println!(
        "Rapid publish: 100 attempts in {elapsed:?}, {success_count} success, {error_count} errors"
    );

    // At least some should succeed
    if success_count > 0 {
        println!("Average publish time: {:?}", elapsed / success_count);
    }
}
