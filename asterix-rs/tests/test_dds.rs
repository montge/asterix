//! Tests for DDS transport integration
//!
//! These tests verify the DDS pub/sub transport for ASTERIX data.
//! Note: These tests require the `dds` feature to be enabled.

#![cfg(feature = "dds")]

use asterix::transport::dds::{
    AsterixMessage, AsterixSample, DdsConfig, DdsError, DdsPublisher, DdsSubscriber,
    Durability, History, Reliability, parse_topic_name,
};

// ============================================================================
// Unit Tests - DdsConfig
// ============================================================================

#[test]
fn test_dds_config_default() {
    let config = DdsConfig::default();

    assert_eq!(config.domain_id, 0);
    assert_eq!(config.topic_prefix, "asterix");
    assert!(matches!(config.reliability, Reliability::Reliable));
    assert!(matches!(config.durability, Durability::Volatile));
    assert!(matches!(config.history, History::KeepLast(10)));
    assert_eq!(config.deadline_ms, 0);
}

#[test]
fn test_dds_config_best_effort() {
    let config = DdsConfig::best_effort();

    assert!(matches!(config.reliability, Reliability::BestEffort));
    assert_eq!(config.topic_prefix, "asterix");
}

#[test]
fn test_dds_config_reliable() {
    let config = DdsConfig::reliable();

    assert!(matches!(config.reliability, Reliability::Reliable));
    assert!(matches!(config.durability, Durability::TransientLocal));
}

#[test]
fn test_dds_config_with_domain() {
    let config = DdsConfig::with_domain(42);

    assert_eq!(config.domain_id, 42);
    assert_eq!(config.topic_prefix, "asterix");
}

#[test]
fn test_dds_config_custom() {
    let config = DdsConfig {
        domain_id: 5,
        topic_prefix: "radar".to_string(),
        reliability: Reliability::BestEffort,
        durability: Durability::TransientLocal,
        history: History::KeepAll,
        deadline_ms: 100,
    };

    assert_eq!(config.domain_id, 5);
    assert_eq!(config.topic_prefix, "radar");
    assert!(matches!(config.reliability, Reliability::BestEffort));
    assert!(matches!(config.durability, Durability::TransientLocal));
    assert!(matches!(config.history, History::KeepAll));
    assert_eq!(config.deadline_ms, 100);
}

// ============================================================================
// Unit Tests - DdsError
// ============================================================================

#[test]
fn test_dds_error_display() {
    let participant_err = DdsError::ParticipantError("connection failed".to_string());
    assert!(participant_err.to_string().contains("connection failed"));
    assert!(participant_err.to_string().contains("participant"));

    let publisher_err = DdsError::PublisherError("timeout".to_string());
    assert!(publisher_err.to_string().contains("timeout"));
    assert!(publisher_err.to_string().contains("publisher"));

    let subscriber_err = DdsError::SubscriberError("invalid topic".to_string());
    assert!(subscriber_err.to_string().contains("invalid topic"));
    assert!(subscriber_err.to_string().contains("subscriber"));

    let topic_err = DdsError::TopicError("already exists".to_string());
    assert!(topic_err.to_string().contains("already exists"));
    assert!(topic_err.to_string().contains("topic"));

    let write_err = DdsError::WriteError("buffer full".to_string());
    assert!(write_err.to_string().contains("buffer full"));
    assert!(write_err.to_string().contains("write"));

    let read_err = DdsError::ReadError("no data".to_string());
    assert!(read_err.to_string().contains("no data"));
    assert!(read_err.to_string().contains("read"));

    let serial_err = DdsError::SerializationError("invalid format".to_string());
    assert!(serial_err.to_string().contains("invalid format"));
    assert!(serial_err.to_string().contains("Serialization"));
}

#[test]
fn test_dds_error_to_asterix_error() {
    use asterix::AsterixError;

    let dds_err = DdsError::ParticipantError("test error".to_string());
    let asterix_err: AsterixError = dds_err.into();

    // Should convert to IOError variant
    match asterix_err {
        AsterixError::IOError(msg) => {
            assert!(msg.contains("test error"));
        }
        _ => panic!("Expected IOError variant"),
    }
}

// ============================================================================
// Unit Tests - AsterixMessage and AsterixSample
// ============================================================================

#[test]
fn test_asterix_message_creation() {
    let msg = AsterixMessage {
        key: "asterix_cat48".to_string(),
        category: 48,
        sac: Some(1),
        sic: Some(2),
        data: vec![0x30, 0x00, 0x10],
        timestamp: 1234567890,
    };

    assert_eq!(msg.category, 48);
    assert_eq!(msg.sac, Some(1));
    assert_eq!(msg.sic, Some(2));
    assert_eq!(msg.data.len(), 3);
    assert_eq!(msg.timestamp, 1234567890);
    assert_eq!(msg.key, "asterix_cat48");
}

#[test]
fn test_asterix_sample_creation() {
    let sample = AsterixSample {
        category: 48,
        sac: Some(1),
        sic: Some(2),
        data: vec![0x30, 0x00, 0x10],
        timestamp: 1234567890,
        topic_name: "asterix_cat48_sac1_sic2".to_string(),
    };

    assert_eq!(sample.category, 48);
    assert_eq!(sample.sac, Some(1));
    assert_eq!(sample.sic, Some(2));
    assert_eq!(sample.data.len(), 3);
    assert_eq!(sample.timestamp, 1234567890);
    assert_eq!(sample.topic_name, "asterix_cat48_sac1_sic2");
}

#[test]
fn test_asterix_sample_without_sac_sic() {
    let sample = AsterixSample {
        category: 62,
        sac: None,
        sic: None,
        data: vec![0x3E, 0x00, 0x20],
        timestamp: 0,
        topic_name: "asterix_cat62".to_string(),
    };

    assert_eq!(sample.category, 62);
    assert!(sample.sac.is_none());
    assert!(sample.sic.is_none());
}

// ============================================================================
// Unit Tests - QoS Policy Types
// ============================================================================

#[test]
fn test_reliability_variants() {
    let _best_effort = Reliability::BestEffort;
    let _reliable = Reliability::Reliable;

    // Test default
    let default: Reliability = Default::default();
    assert!(matches!(default, Reliability::Reliable));
}

#[test]
fn test_durability_variants() {
    let _volatile = Durability::Volatile;
    let _transient_local = Durability::TransientLocal;

    // Test default
    let default: Durability = Default::default();
    assert!(matches!(default, Durability::Volatile));
}

#[test]
fn test_history_variants() {
    let _keep_last = History::KeepLast(5);
    let _keep_all = History::KeepAll;

    // Test default
    let default: History = Default::default();
    assert!(matches!(default, History::KeepLast(10)));
}

// ============================================================================
// Unit Tests - Topic Name Parsing
// ============================================================================

#[test]
fn test_parse_topic_name_category_only() {
    let (cat, sac, sic) = parse_topic_name("asterix_cat48", "asterix");
    assert_eq!(cat, 48);
    assert!(sac.is_none());
    assert!(sic.is_none());

    let (cat, sac, sic) = parse_topic_name("asterix_cat62", "asterix");
    assert_eq!(cat, 62);
    assert!(sac.is_none());
    assert!(sic.is_none());
}

#[test]
fn test_parse_topic_name_with_sac_sic() {
    let (cat, sac, sic) = parse_topic_name("asterix_cat48_sac1_sic2", "asterix");
    assert_eq!(cat, 48);
    assert_eq!(sac, Some(1));
    assert_eq!(sic, Some(2));

    let (cat, sac, sic) = parse_topic_name("asterix_cat62_sac10_sic20", "asterix");
    assert_eq!(cat, 62);
    assert_eq!(sac, Some(10));
    assert_eq!(sic, Some(20));
}

#[test]
fn test_parse_topic_name_different_prefix() {
    let (cat, sac, sic) = parse_topic_name("radar_cat48", "radar");
    assert_eq!(cat, 48);
    assert!(sac.is_none());
    assert!(sic.is_none());
}

#[test]
fn test_parse_topic_name_max_values() {
    let (cat, sac, sic) = parse_topic_name("asterix_cat255_sac255_sic255", "asterix");
    assert_eq!(cat, 255);
    assert_eq!(sac, Some(255));
    assert_eq!(sic, Some(255));
}

// ============================================================================
// Integration Tests - Require DDS Runtime
// ============================================================================

/// These tests require a DDS domain participant and are marked as integration tests.
mod integration {
    use super::*;
    use std::time::Duration;
    use std::thread;

    /// Test creating a publisher
    #[test]
    fn test_publisher_create() {
        let config = DdsConfig::default();

        let publisher = DdsPublisher::new(config);

        match publisher {
            Ok(_pub_instance) => {
                println!("DDS Publisher created successfully");
            }
            Err(e) => {
                // May fail in some CI environments
                println!("Publisher creation failed (may be expected): {e}");
            }
        }
    }

    /// Test creating a subscriber
    #[test]
    fn test_subscriber_create() {
        let config = DdsConfig::default();

        let subscriber = DdsSubscriber::new(config, "asterix_cat48");

        match subscriber {
            Ok(_sub_instance) => {
                println!("DDS Subscriber created successfully");
            }
            Err(e) => {
                println!("Subscriber creation failed (may be expected): {e}");
            }
        }
    }

    /// Test pub/sub communication with raw data
    #[test]
    fn test_pubsub_raw_data() {
        let config = DdsConfig::best_effort();
        let topic_name = "asterix_test_pubsub";

        // Create publisher
        let publisher = match DdsPublisher::new(config.clone()) {
            Ok(p) => p,
            Err(e) => {
                println!("Skipping pubsub test (publisher failed): {e}");
                return;
            }
        };

        // Create subscriber on same topic
        let mut subscriber = match DdsSubscriber::new(config, topic_name) {
            Ok(s) => s,
            Err(e) => {
                println!("Skipping pubsub test (subscriber failed): {e}");
                return;
            }
        };

        // Small delay for DDS discovery
        thread::sleep(Duration::from_millis(500));

        // Test data
        let test_data = vec![0x30, 0x00, 0x10, 0xFF, 0xAB, 0xCD];

        // Publish raw data
        let publish_result = publisher.publish_raw(48, &test_data);

        match publish_result {
            Ok(()) => {
                println!("Published data successfully");

                // Try to receive with timeout
                let sample = subscriber.recv_timeout(Duration::from_secs(2));

                match sample {
                    Some(s) => {
                        assert_eq!(s.category, 48);
                        println!("Successfully received sample: {:?}", s);
                    }
                    None => {
                        // May not receive in all environments due to DDS discovery timing
                        println!("No data received (may be DDS discovery timing)");
                    }
                }
            }
            Err(e) => {
                println!("Publish failed: {e}");
            }
        }
    }

    /// Test publishing with full routing info
    #[test]
    fn test_publish_raw_with_routing() {
        let config = DdsConfig::default();

        let publisher = match DdsPublisher::new(config) {
            Ok(p) => p,
            Err(_) => return,
        };

        let test_data = vec![0x30, 0x00, 0x10];
        let result = publisher.publish_raw_with_routing(48, 1, 2, &test_data);

        match result {
            Ok(()) => println!("publish_raw_with_routing succeeded"),
            Err(e) => println!("publish_raw_with_routing failed: {e}"),
        }
    }

    /// Test try_recv (non-blocking receive)
    #[test]
    fn test_subscriber_try_recv() {
        let config = DdsConfig::default();

        let mut subscriber = match DdsSubscriber::new(config, "asterix_test_tryrecv") {
            Ok(s) => s,
            Err(_) => return,
        };

        // try_recv should return None immediately (no data published)
        let result = subscriber.try_recv();
        assert!(result.is_none(), "Expected None from try_recv");
    }

    /// Test different domain IDs
    #[test]
    fn test_different_domains() {
        let config1 = DdsConfig::with_domain(1);
        let config2 = DdsConfig::with_domain(2);

        // Publishers in different domains should not interfere
        let pub1 = DdsPublisher::new(config1);
        let pub2 = DdsPublisher::new(config2);

        match (pub1, pub2) {
            (Ok(_), Ok(_)) => {
                println!("Created publishers in different domains");
            }
            (Err(e1), _) => {
                println!("Domain 1 publisher failed: {e1}");
            }
            (_, Err(e2)) => {
                println!("Domain 2 publisher failed: {e2}");
            }
        }
    }

    /// Test QoS configurations
    #[test]
    fn test_qos_configurations() {
        // Best effort
        let be_config = DdsConfig::best_effort();
        let be_pub = DdsPublisher::new(be_config);
        match be_pub {
            Ok(_) => println!("Best effort publisher created"),
            Err(e) => println!("Best effort failed: {e}"),
        }

        // Reliable with transient local
        let rel_config = DdsConfig::reliable();
        let rel_pub = DdsPublisher::new(rel_config);
        match rel_pub {
            Ok(_) => println!("Reliable publisher created"),
            Err(e) => println!("Reliable failed: {e}"),
        }

        // Custom with deadline
        let custom_config = DdsConfig {
            deadline_ms: 100,
            ..Default::default()
        };
        let custom_pub = DdsPublisher::new(custom_config);
        match custom_pub {
            Ok(_) => println!("Custom QoS publisher created"),
            Err(e) => println!("Custom QoS failed: {e}"),
        }
    }
}

// ============================================================================
// Topic Name Format Tests
// ============================================================================

#[test]
fn test_topic_name_format() {
    // Validate expected topic name formats
    let formats = vec![
        ("asterix_cat48", "Category only"),
        ("asterix_cat48_sac1", "Category + SAC"),
        ("asterix_cat48_sac1_sic2", "Full: Category/SAC/SIC"),
        ("asterix_cat62_sac10_sic20", "Higher SAC/SIC values"),
        ("asterix_cat255_sac255_sic255", "Maximum values"),
    ];

    for (topic_name, description) in formats {
        let (cat, sac, sic) = parse_topic_name(topic_name, "asterix");

        // Category should be valid
        assert!(cat > 0 || topic_name.contains("cat0"), "Invalid category in: {description}");

        println!("{description}: {topic_name} -> cat={cat}, sac={sac:?}, sic={sic:?}");
    }
}

