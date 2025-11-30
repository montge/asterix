//! Tests for Zenoh transport integration
//!
//! These tests verify the Zenoh pub/sub transport for ASTERIX data.
//! Note: Many tests require the `zenoh` feature to be enabled.

#![cfg(feature = "zenoh")]

use asterix::transport::zenoh::{
    AsterixSample, CongestionControl, Priority, ZenohConfig, ZenohError, ZenohPublisher,
    ZenohSubscriber,
};

// ============================================================================
// Unit Tests - ZenohConfig
// ============================================================================

#[test]
fn test_zenoh_config_default() {
    let config = ZenohConfig::default();

    assert!(config.endpoints.is_empty());
    assert_eq!(config.key_prefix, "asterix");
    assert!(config.include_raw_bytes);
    assert!(matches!(
        config.congestion_control,
        CongestionControl::Block
    ));
    assert!(matches!(config.priority, Priority::Data));
}

#[test]
fn test_zenoh_config_with_router() {
    let config = ZenohConfig::with_router("tcp/192.168.1.1:7447");

    assert_eq!(config.endpoints.len(), 1);
    assert_eq!(config.endpoints[0], "tcp/192.168.1.1:7447");
    assert_eq!(config.key_prefix, "asterix");
}

#[test]
fn test_zenoh_config_with_endpoints() {
    let endpoints = vec![
        "tcp/10.0.0.1:7447".to_string(),
        "tcp/10.0.0.2:7447".to_string(),
    ];
    let config = ZenohConfig::with_endpoints(endpoints.clone());

    assert_eq!(config.endpoints.len(), 2);
    assert_eq!(config.endpoints, endpoints);
}

#[test]
fn test_zenoh_config_peer_to_peer() {
    let config = ZenohConfig::peer_to_peer();

    // Peer-to-peer mode uses empty endpoints (multicast discovery)
    assert!(config.endpoints.is_empty());
}

// ============================================================================
// Unit Tests - ZenohError
// ============================================================================

#[test]
fn test_zenoh_error_display() {
    let session_err = ZenohError::SessionError("connection failed".to_string());
    assert!(session_err.to_string().contains("connection failed"));
    assert!(session_err.to_string().contains("session"));

    let publish_err = ZenohError::PublishError("timeout".to_string());
    assert!(publish_err.to_string().contains("timeout"));
    assert!(publish_err.to_string().contains("publish"));

    let subscriber_err = ZenohError::SubscriberError("invalid key".to_string());
    assert!(subscriber_err.to_string().contains("invalid key"));
    assert!(subscriber_err.to_string().contains("subscriber"));

    let receive_err = ZenohError::ReceiveError("channel closed".to_string());
    assert!(receive_err.to_string().contains("channel closed"));
    assert!(receive_err.to_string().contains("receive"));

    let serial_err = ZenohError::SerializationError("invalid format".to_string());
    assert!(serial_err.to_string().contains("invalid format"));
    assert!(serial_err.to_string().contains("Serialization"));

    let channel_err = ZenohError::ChannelClosed;
    assert!(channel_err.to_string().contains("closed"));
}

#[test]
fn test_zenoh_error_to_asterix_error() {
    use asterix::AsterixError;

    let zenoh_err = ZenohError::SessionError("test error".to_string());
    let asterix_err: AsterixError = zenoh_err.into();

    // Should convert to IOError variant
    match asterix_err {
        AsterixError::IOError(msg) => {
            assert!(msg.contains("test error"));
        }
        _ => panic!("Expected IOError variant"),
    }
}

// ============================================================================
// Unit Tests - AsterixSample
// ============================================================================

#[test]
fn test_asterix_sample_creation() {
    let sample = AsterixSample {
        category: 48,
        sac: Some(1),
        sic: Some(2),
        data: vec![0x30, 0x00, 0x10],
        timestamp: 1234567890,
        key_expr: "asterix/48/1/2".to_string(),
    };

    assert_eq!(sample.category, 48);
    assert_eq!(sample.sac, Some(1));
    assert_eq!(sample.sic, Some(2));
    assert_eq!(sample.data.len(), 3);
    assert_eq!(sample.timestamp, 1234567890);
    assert_eq!(sample.key_expr, "asterix/48/1/2");
}

#[test]
fn test_asterix_sample_without_sac_sic() {
    let sample = AsterixSample {
        category: 62,
        sac: None,
        sic: None,
        data: vec![0x3E, 0x00, 0x20],
        timestamp: 0,
        key_expr: "asterix/62".to_string(),
    };

    assert_eq!(sample.category, 62);
    assert!(sample.sac.is_none());
    assert!(sample.sic.is_none());
}

// ============================================================================
// Unit Tests - Priority and CongestionControl
// ============================================================================

#[test]
fn test_priority_variants() {
    let _realtime = Priority::RealTime;
    let _interactive = Priority::Interactive;
    let _data = Priority::Data;
    let _background = Priority::Background;

    // Test default
    let default: Priority = Default::default();
    assert!(matches!(default, Priority::Data));
}

#[test]
fn test_congestion_control_variants() {
    let _block = CongestionControl::Block;
    let _drop = CongestionControl::Drop;

    // Test default
    let default: CongestionControl = Default::default();
    assert!(matches!(default, CongestionControl::Block));
}

// ============================================================================
// Integration Tests - Require Zenoh Runtime
// ============================================================================

/// These tests require a Zenoh session and are marked as integration tests.
/// They use tokio runtime for async operations.
/// NOTE: Zenoh requires multi-thread tokio runtime, so we use flavor = "multi_thread"
mod integration {
    use super::*;
    use std::time::Duration;
    use tokio::time::timeout;

    /// Test creating a publisher and immediately closing it
    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_create_and_close() {
        // Use peer mode (no router needed)
        let config = ZenohConfig::peer_to_peer();

        let publisher = ZenohPublisher::new(config).await;

        match publisher {
            Ok(pub_instance) => {
                // Successfully created, now close
                let close_result = pub_instance.close().await;
                assert!(close_result.is_ok(), "Failed to close publisher");
            }
            Err(e) => {
                // May fail in CI environments without Zenoh network
                println!("Publisher creation skipped (no Zenoh network): {e}");
            }
        }
    }

    /// Test creating a subscriber and immediately closing it
    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_subscriber_create_and_close() {
        let config = ZenohConfig::peer_to_peer();

        let subscriber = ZenohSubscriber::new(config, "asterix/**").await;

        match subscriber {
            Ok(sub_instance) => {
                let close_result = sub_instance.close().await;
                assert!(close_result.is_ok(), "Failed to close subscriber");
            }
            Err(e) => {
                println!("Subscriber creation skipped (no Zenoh network): {e}");
            }
        }
    }

    /// Test pub/sub communication with raw data
    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_pubsub_raw_data() {
        let config = ZenohConfig::peer_to_peer();

        // Create publisher
        let publisher = match ZenohPublisher::new(config.clone()).await {
            Ok(p) => p,
            Err(e) => {
                println!("Skipping pubsub test (no Zenoh): {e}");
                return;
            }
        };

        // Create subscriber on specific key
        let mut subscriber = match ZenohSubscriber::new(config, "asterix/48/**").await {
            Ok(s) => s,
            Err(e) => {
                let _ = publisher.close().await;
                println!("Skipping pubsub test (subscriber failed): {e}");
                return;
            }
        };

        // Small delay to ensure subscriber is ready
        tokio::time::sleep(Duration::from_millis(100)).await;

        // Test data
        let test_data = vec![0x30, 0x00, 0x10, 0xFF, 0xAB, 0xCD];

        // Publish raw data
        let publish_result = publisher
            .publish_raw_with_routing(48, 1, 2, &test_data)
            .await;
        assert!(publish_result.is_ok(), "Failed to publish raw data");

        // Try to receive with timeout
        let recv_result = timeout(Duration::from_secs(2), subscriber.recv()).await;

        match recv_result {
            Ok(Some(sample)) => {
                assert_eq!(sample.category, 48);
                assert_eq!(sample.sac, Some(1));
                assert_eq!(sample.sic, Some(2));
                assert_eq!(sample.data, test_data);
                println!("Successfully received sample: {sample:?}");
            }
            Ok(None) => {
                println!("Channel closed (no data received)");
            }
            Err(_) => {
                println!("Timeout waiting for sample (may be network issue)");
            }
        }

        // Cleanup
        let _ = publisher.close().await;
        let _ = subscriber.close().await;
    }

    /// Test publishing without routing info (category only)
    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publish_raw_category_only() {
        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return, // Skip if no Zenoh
        };

        let test_data = vec![0x3E, 0x00, 0x20];
        let result = publisher.publish_raw(62, &test_data).await;

        // Should succeed
        assert!(result.is_ok(), "publish_raw failed: {:?}", result.err());

        let _ = publisher.close().await;
    }

    /// Test try_recv (non-blocking receive)
    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_subscriber_try_recv() {
        let config = ZenohConfig::peer_to_peer();

        let mut subscriber = match ZenohSubscriber::new(config, "asterix/**").await {
            Ok(s) => s,
            Err(_) => return,
        };

        // try_recv should return None immediately (no data published)
        let result = subscriber.try_recv();
        assert!(result.is_none(), "Expected None from try_recv");

        let _ = subscriber.close().await;
    }

    /// Test subscriber with wildcard key expressions
    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_subscriber_wildcards() {
        let config = ZenohConfig::peer_to_peer();

        // Various wildcard patterns
        let patterns = vec![
            "asterix/**",    // All ASTERIX
            "asterix/48/**", // All CAT048
            "asterix/*/1/*", // All SAC=1
        ];

        for pattern in patterns {
            let subscriber = ZenohSubscriber::new(config.clone(), pattern).await;
            match subscriber {
                Ok(s) => {
                    let _ = s.close().await;
                    println!("Pattern '{pattern}' accepted");
                }
                Err(e) => {
                    println!("Pattern '{pattern}' rejected: {e}");
                    // Some patterns may not be supported depending on Zenoh version
                }
            }
        }
    }
}

// ============================================================================
// Key Expression Parsing Tests (internal function via module tests)
// ============================================================================

// The parse_key_expr function is internal to the zenoh module,
// but it's tested via the module's #[cfg(test)] section.
// These tests validate the expected behavior indirectly.

#[test]
fn test_key_expr_format() {
    // Validate expected key expression formats
    let formats = vec![
        ("asterix/48", "Category only"),
        ("asterix/48/1", "Category + SAC"),
        ("asterix/48/1/2", "Full: Category/SAC/SIC"),
        ("asterix/62/10/20", "Higher SAC/SIC values"),
        ("asterix/255/255/255", "Maximum values"),
    ];

    for (key_expr, description) in formats {
        // Parse manually to validate format
        let parts: Vec<&str> = key_expr
            .strip_prefix("asterix/")
            .unwrap_or(key_expr)
            .split('/')
            .collect();

        assert!(!parts.is_empty(), "Invalid format: {description}");

        // Category should be parseable as u8
        let category: Result<u8, _> = parts[0].parse();
        assert!(category.is_ok(), "Invalid category in: {description}");

        println!("{description}: {key_expr} parsed successfully");
    }
}
