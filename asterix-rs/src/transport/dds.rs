//! DDS transport for ASTERIX data distribution
//!
//! This module provides pub/sub capabilities for ASTERIX data using the
//! [DDS (Data Distribution Service)](https://www.omg.org/spec/DDS/) standard,
//! implemented via the native Rust [RustDDS](https://github.com/Atostek/RustDDS) crate.
//!
//! # Features
//!
//! - **RTPS Protocol** - Interoperable with other DDS implementations (Fast DDS, OpenDDS, Cyclone DDS)
//! - **QoS Policies** - Reliability, durability, history, and deadline settings
//! - **Automatic Discovery** - SPDP/SEDP for peer discovery
//! - **UDP Multicast/Unicast** - Flexible network transport options
//!
//! # Topic Naming Convention
//!
//! ASTERIX data is published using the following topic naming format:
//!
//! ```text
//! asterix_cat{category}
//! ```
//!
//! Or with SAC/SIC routing:
//!
//! ```text
//! asterix_cat{category}_sac{sac}_sic{sic}
//! ```
//!
//! # Examples
//!
//! ## Publishing ASTERIX Data
//!
//! ```no_run
//! use asterix::transport::dds::{DdsPublisher, DdsConfig};
//! use asterix::{parse, ParseOptions, init_default};
//!
//! fn main() -> Result<(), Box<dyn std::error::Error>> {
//!     init_default()?;
//!
//!     // Create publisher
//!     let publisher = DdsPublisher::new(DdsConfig::default())?;
//!
//!     // Parse and publish ASTERIX data
//!     let data = std::fs::read("sample.asterix")?;
//!     let records = parse(&data, ParseOptions::default())?;
//!
//!     for record in records {
//!         publisher.publish(&record)?;
//!     }
//!
//!     Ok(())
//! }
//! ```

use std::fmt;
use std::sync::Arc;
use std::time::Duration;

use rustdds::dds::DomainParticipant;
use rustdds::serialization::{CDRDeserializerAdapter, CDRSerializerAdapter};
use rustdds::with_key::{DataReader, DataWriter};
use rustdds::{
    policy::{Durability as DdsDurability, History as DdsHistory, Reliability as DdsReliability},
    DomainParticipantBuilder, Keyed, QosPolicyBuilder, TopicKind,
};
use serde::{Deserialize, Serialize};

use crate::error::AsterixError;
use crate::types::AsterixRecord;

/// Error type for DDS transport operations
#[derive(Debug)]
pub enum DdsError {
    /// Failed to create DDS domain participant
    ParticipantError(String),
    /// Failed to create publisher
    PublisherError(String),
    /// Failed to create subscriber
    SubscriberError(String),
    /// Failed to create topic
    TopicError(String),
    /// Failed to write data
    WriteError(String),
    /// Failed to read data
    ReadError(String),
    /// Serialization error
    SerializationError(String),
}

impl fmt::Display for DdsError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            DdsError::ParticipantError(msg) => write!(f, "DDS participant error: {msg}"),
            DdsError::PublisherError(msg) => write!(f, "DDS publisher error: {msg}"),
            DdsError::SubscriberError(msg) => write!(f, "DDS subscriber error: {msg}"),
            DdsError::TopicError(msg) => write!(f, "DDS topic error: {msg}"),
            DdsError::WriteError(msg) => write!(f, "DDS write error: {msg}"),
            DdsError::ReadError(msg) => write!(f, "DDS read error: {msg}"),
            DdsError::SerializationError(msg) => write!(f, "Serialization error: {msg}"),
        }
    }
}

impl std::error::Error for DdsError {}

impl From<DdsError> for AsterixError {
    fn from(err: DdsError) -> Self {
        AsterixError::IOError(err.to_string())
    }
}

/// QoS reliability mode for DDS
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum Reliability {
    /// Best effort delivery - may lose samples
    BestEffort,
    /// Reliable delivery - retransmits lost samples
    #[default]
    Reliable,
}

/// QoS durability mode for DDS
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum Durability {
    /// Data only available to currently matched readers
    #[default]
    Volatile,
    /// Data persisted for late-joining readers (within writer lifetime)
    TransientLocal,
}

/// QoS history mode for DDS
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum History {
    /// Keep only the last N samples
    KeepLast(i32),
    /// Keep all samples (limited by resource limits)
    KeepAll,
}

impl Default for History {
    fn default() -> Self {
        History::KeepLast(10)
    }
}

/// Configuration for DDS transport
#[derive(Debug, Clone)]
pub struct DdsConfig {
    /// DDS Domain ID (default: 0)
    /// Participants in different domains cannot communicate
    pub domain_id: u16,

    /// Topic name prefix for ASTERIX data
    /// Default: "asterix"
    pub topic_prefix: String,

    /// Reliability QoS policy
    pub reliability: Reliability,

    /// Durability QoS policy
    pub durability: Durability,

    /// History QoS policy
    pub history: History,

    /// Deadline period in milliseconds (0 = no deadline)
    pub deadline_ms: u64,
}

impl Default for DdsConfig {
    fn default() -> Self {
        Self {
            domain_id: 0,
            topic_prefix: "asterix".to_string(),
            reliability: Reliability::default(),
            durability: Durability::default(),
            history: History::default(),
            deadline_ms: 0,
        }
    }
}

impl DdsConfig {
    /// Create a config with best-effort reliability (lower latency)
    pub fn best_effort() -> Self {
        Self {
            reliability: Reliability::BestEffort,
            ..Default::default()
        }
    }

    /// Create a config with reliable delivery
    pub fn reliable() -> Self {
        Self {
            reliability: Reliability::Reliable,
            durability: Durability::TransientLocal,
            ..Default::default()
        }
    }

    /// Create a config for a specific domain
    pub fn with_domain(domain_id: u16) -> Self {
        Self {
            domain_id,
            ..Default::default()
        }
    }

    /// Build QoS policies from config
    fn build_qos(&self) -> rustdds::QosPolicies {
        let mut builder = QosPolicyBuilder::new();

        // Set reliability
        match self.reliability {
            Reliability::BestEffort => {
                builder = builder.reliability(DdsReliability::BestEffort);
            }
            Reliability::Reliable => {
                builder = builder.reliability(DdsReliability::Reliable {
                    max_blocking_time: rustdds::Duration::from_millis(100),
                });
            }
        }

        // Set durability
        match self.durability {
            Durability::Volatile => {
                builder = builder.durability(DdsDurability::Volatile);
            }
            Durability::TransientLocal => {
                builder = builder.durability(DdsDurability::TransientLocal);
            }
        }

        // Set history
        match self.history {
            History::KeepLast(depth) => {
                builder = builder.history(DdsHistory::KeepLast { depth });
            }
            History::KeepAll => {
                builder = builder.history(DdsHistory::KeepAll);
            }
        }

        // Set deadline if specified
        if self.deadline_ms > 0 {
            use rustdds::policy::Deadline;
            builder = builder.deadline(Deadline(rustdds::Duration::from_millis(
                self.deadline_ms as i64,
            )));
        }

        builder.build()
    }
}

/// ASTERIX data message for DDS transport
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AsterixMessage {
    /// Unique key for the message (category + optional SAC/SIC)
    pub key: String,
    /// ASTERIX category
    pub category: u8,
    /// System Area Code (if available)
    pub sac: Option<u8>,
    /// System Identification Code (if available)
    pub sic: Option<u8>,
    /// Raw ASTERIX data bytes
    pub data: Vec<u8>,
    /// Timestamp in microseconds since epoch
    pub timestamp: u64,
}

impl Keyed for AsterixMessage {
    type K = String;

    fn key(&self) -> Self::K {
        self.key.clone()
    }
}

/// Received ASTERIX sample from DDS
#[derive(Debug, Clone)]
pub struct AsterixSample {
    /// ASTERIX category
    pub category: u8,
    /// System Area Code (if available)
    pub sac: Option<u8>,
    /// System Identification Code (if available)
    pub sic: Option<u8>,
    /// Raw ASTERIX data bytes
    pub data: Vec<u8>,
    /// Timestamp when sample was received (microseconds since epoch)
    pub timestamp: u64,
    /// Topic name the sample was published on
    pub topic_name: String,
}

/// DDS publisher for ASTERIX data
pub struct DdsPublisher {
    participant: Arc<DomainParticipant>,
    config: DdsConfig,
    writers: std::sync::Mutex<
        std::collections::HashMap<
            String,
            DataWriter<AsterixMessage, CDRSerializerAdapter<AsterixMessage>>,
        >,
    >,
}

impl DdsPublisher {
    /// Create a new DDS publisher
    pub fn new(config: DdsConfig) -> Result<Self, DdsError> {
        let participant = DomainParticipantBuilder::new(config.domain_id)
            .build()
            .map_err(|e| DdsError::ParticipantError(format!("{e:?}")))?;

        Ok(Self {
            participant: Arc::new(participant),
            config,
            writers: std::sync::Mutex::new(std::collections::HashMap::new()),
        })
    }

    /// Publish an ASTERIX record
    pub fn publish(&self, record: &AsterixRecord) -> Result<(), DdsError> {
        let (sac, sic) = self.extract_sac_sic(record);
        let topic_name = self.build_topic_name(record.category, sac, sic);

        let message = AsterixMessage {
            key: topic_name.clone(),
            category: record.category,
            sac,
            sic,
            data: self.get_raw_data(record)?,
            timestamp: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .map(|d| d.as_micros() as u64)
                .unwrap_or(0),
        };

        self.write_to_topic(&topic_name, message)
    }

    /// Publish raw ASTERIX bytes with category information
    pub fn publish_raw(&self, category: u8, data: &[u8]) -> Result<(), DdsError> {
        let topic_name = format!("{}_cat{}", self.config.topic_prefix, category);

        let message = AsterixMessage {
            key: topic_name.clone(),
            category,
            sac: None,
            sic: None,
            data: data.to_vec(),
            timestamp: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .map(|d| d.as_micros() as u64)
                .unwrap_or(0),
        };

        self.write_to_topic(&topic_name, message)
    }

    /// Publish raw ASTERIX bytes with full routing info
    pub fn publish_raw_with_routing(
        &self,
        category: u8,
        sac: u8,
        sic: u8,
        data: &[u8],
    ) -> Result<(), DdsError> {
        let topic_name = format!(
            "{}_cat{}_sac{}_sic{}",
            self.config.topic_prefix, category, sac, sic
        );

        let message = AsterixMessage {
            key: topic_name.clone(),
            category,
            sac: Some(sac),
            sic: Some(sic),
            data: data.to_vec(),
            timestamp: std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .map(|d| d.as_micros() as u64)
                .unwrap_or(0),
        };

        self.write_to_topic(&topic_name, message)
    }

    fn write_to_topic(&self, topic_name: &str, message: AsterixMessage) -> Result<(), DdsError> {
        let mut writers = self.writers.lock().unwrap();

        // Get or create writer for this topic
        if !writers.contains_key(topic_name) {
            let qos = self.config.build_qos();

            let topic = self
                .participant
                .create_topic(
                    topic_name.to_string(),
                    "AsterixMessage".to_string(),
                    &qos,
                    TopicKind::WithKey,
                )
                .map_err(|e| DdsError::TopicError(format!("{e:?}")))?;

            let publisher = self
                .participant
                .create_publisher(&qos)
                .map_err(|e| DdsError::PublisherError(format!("{e:?}")))?;

            let writer = publisher
                .create_datawriter_cdr::<AsterixMessage>(&topic, None)
                .map_err(|e| DdsError::PublisherError(format!("{e:?}")))?;

            writers.insert(topic_name.to_string(), writer);
        }

        let writer = writers.get(topic_name).unwrap();
        writer
            .write(message, None)
            .map_err(|e| DdsError::WriteError(format!("{e:?}")))?;

        log::debug!("Published ASTERIX to DDS topic {topic_name}");
        Ok(())
    }

    fn build_topic_name(&self, category: u8, sac: Option<u8>, sic: Option<u8>) -> String {
        match (sac, sic) {
            (Some(s), Some(c)) => {
                format!(
                    "{}_cat{}_sac{}_sic{}",
                    self.config.topic_prefix, category, s, c
                )
            }
            _ => format!("{}_cat{}", self.config.topic_prefix, category),
        }
    }

    fn extract_sac_sic(&self, record: &AsterixRecord) -> (Option<u8>, Option<u8>) {
        let item_id = format!("I{:03}/010", record.category);

        if let Some(item) = record.get_item(&item_id) {
            let sac = item
                .fields
                .get("SAC")
                .and_then(|v| v.as_i64())
                .map(|v| v as u8);

            let sic = item
                .fields
                .get("SIC")
                .and_then(|v| v.as_i64())
                .map(|v| v as u8);

            return (sac, sic);
        }

        (None, None)
    }

    fn get_raw_data(&self, record: &AsterixRecord) -> Result<Vec<u8>, DdsError> {
        if !record.hex_data.is_empty() {
            return self.hex_to_bytes(&record.hex_data);
        }

        #[cfg(feature = "serde")]
        {
            serde_json::to_vec(record).map_err(|e| DdsError::SerializationError(e.to_string()))
        }

        #[cfg(not(feature = "serde"))]
        Err(DdsError::SerializationError(
            "No serialization method available (provide hex_data)".to_string(),
        ))
    }

    fn hex_to_bytes(&self, hex: &str) -> Result<Vec<u8>, DdsError> {
        let hex_clean: String = hex.chars().filter(|c| !c.is_whitespace()).collect();

        if hex_clean.len() % 2 != 0 {
            return Err(DdsError::SerializationError(
                "Invalid hex string length".to_string(),
            ));
        }

        (0..hex_clean.len())
            .step_by(2)
            .map(|i| {
                u8::from_str_radix(&hex_clean[i..i + 2], 16)
                    .map_err(|e| DdsError::SerializationError(e.to_string()))
            })
            .collect()
    }
}

/// DDS subscriber for ASTERIX data
pub struct DdsSubscriber {
    #[allow(dead_code)]
    participant: Arc<DomainParticipant>,
    reader: DataReader<AsterixMessage, CDRDeserializerAdapter<AsterixMessage>>,
    topic_name: String,
}

impl DdsSubscriber {
    /// Create a new DDS subscriber
    ///
    /// # Arguments
    ///
    /// * `config` - DDS configuration
    /// * `topic_name` - Topic name to subscribe to (e.g., "asterix_cat48")
    pub fn new(config: DdsConfig, topic_name: &str) -> Result<Self, DdsError> {
        let participant = DomainParticipantBuilder::new(config.domain_id)
            .build()
            .map_err(|e| DdsError::ParticipantError(format!("{e:?}")))?;

        let qos = config.build_qos();

        let topic = participant
            .create_topic(
                topic_name.to_string(),
                "AsterixMessage".to_string(),
                &qos,
                TopicKind::WithKey,
            )
            .map_err(|e| DdsError::TopicError(format!("{e:?}")))?;

        let subscriber = participant
            .create_subscriber(&qos)
            .map_err(|e| DdsError::SubscriberError(format!("{e:?}")))?;

        let reader = subscriber
            .create_datareader_cdr::<AsterixMessage>(&topic, None)
            .map_err(|e| DdsError::SubscriberError(format!("{e:?}")))?;

        Ok(Self {
            participant: Arc::new(participant),
            reader,
            topic_name: topic_name.to_string(),
        })
    }

    /// Try to receive a sample without blocking
    pub fn try_recv(&mut self) -> Option<AsterixSample> {
        use rustdds::with_key::Sample;

        match self.reader.take_next_sample() {
            Ok(Some(sample)) => {
                match sample.into_value() {
                    Sample::Value(msg) => Some(AsterixSample {
                        category: msg.category,
                        sac: msg.sac,
                        sic: msg.sic,
                        data: msg.data,
                        timestamp: msg.timestamp,
                        topic_name: self.topic_name.clone(),
                    }),
                    Sample::Dispose(_key) => {
                        // Instance was disposed, no data to return
                        None
                    }
                }
            }
            Ok(None) => None,
            Err(e) => {
                log::warn!("DDS read error: {e:?}");
                None
            }
        }
    }

    /// Receive the next sample with timeout
    pub fn recv_timeout(&mut self, timeout: Duration) -> Option<AsterixSample> {
        let start = std::time::Instant::now();
        while start.elapsed() < timeout {
            if let Some(sample) = self.try_recv() {
                return Some(sample);
            }
            std::thread::sleep(Duration::from_millis(1));
        }
        None
    }
}

/// Parse a topic name to extract category, SAC, and SIC
pub fn parse_topic_name(topic: &str, prefix: &str) -> (u8, Option<u8>, Option<u8>) {
    let stripped = topic.strip_prefix(prefix).unwrap_or(topic);
    let stripped = stripped.trim_start_matches('_');

    // Parse patterns like "cat48" or "cat48_sac1_sic2"
    let mut category = 0u8;
    let mut sac = None;
    let mut sic = None;

    for part in stripped.split('_') {
        if let Some(cat_str) = part.strip_prefix("cat") {
            category = cat_str.parse().unwrap_or(0);
        } else if let Some(sac_str) = part.strip_prefix("sac") {
            sac = sac_str.parse().ok();
        } else if let Some(sic_str) = part.strip_prefix("sic") {
            sic = sic_str.parse().ok();
        }
    }

    (category, sac, sic)
}

#[cfg(test)]
mod tests {
    use super::*;

    // ============================================================================
    // Error Type Tests
    // ============================================================================

    #[test]
    fn test_dds_error_display_variants() {
        let errors = vec![
            (
                DdsError::ParticipantError("test".to_string()),
                "participant",
            ),
            (DdsError::PublisherError("test".to_string()), "publisher"),
            (DdsError::SubscriberError("test".to_string()), "subscriber"),
            (DdsError::TopicError("test".to_string()), "topic"),
            (DdsError::WriteError("test".to_string()), "write"),
            (DdsError::ReadError("test".to_string()), "read"),
            (
                DdsError::SerializationError("test".to_string()),
                "Serialization",
            ),
        ];

        for (err, expected_substring) in errors {
            let display = err.to_string();
            assert!(
                display.contains(expected_substring),
                "Expected '{expected_substring}' in '{display}'"
            );
        }
    }

    #[test]
    fn test_dds_error_debug() {
        let err = DdsError::ParticipantError("test error".to_string());
        let debug = format!("{err:?}");
        assert!(debug.contains("ParticipantError"));
        assert!(debug.contains("test error"));
    }

    #[test]
    fn test_dds_error_to_asterix_error() {
        let err = DdsError::WriteError("write failed".to_string());
        let asterix_err: AsterixError = err.into();
        let msg = asterix_err.to_string();
        assert!(msg.contains("write"));
    }

    #[test]
    fn test_dds_error_is_std_error() {
        let err: Box<dyn std::error::Error> = Box::new(DdsError::TopicError("test".to_string()));
        assert!(err.to_string().contains("topic"));
    }

    // ============================================================================
    // Config Tests
    // ============================================================================

    #[test]
    fn test_dds_config_default() {
        let config = DdsConfig::default();
        assert_eq!(config.domain_id, 0);
        assert_eq!(config.topic_prefix, "asterix");
        assert_eq!(config.reliability, Reliability::Reliable);
        assert_eq!(config.durability, Durability::Volatile);
    }

    #[test]
    fn test_dds_config_best_effort() {
        let config = DdsConfig::best_effort();
        assert_eq!(config.reliability, Reliability::BestEffort);
    }

    #[test]
    fn test_dds_config_reliable() {
        let config = DdsConfig::reliable();
        assert_eq!(config.reliability, Reliability::Reliable);
        assert_eq!(config.durability, Durability::TransientLocal);
    }

    #[test]
    fn test_dds_config_with_domain() {
        let config = DdsConfig::with_domain(42);
        assert_eq!(config.domain_id, 42);
    }

    #[test]
    fn test_dds_config_clone() {
        let config = DdsConfig {
            domain_id: 5,
            topic_prefix: "custom".to_string(),
            reliability: Reliability::BestEffort,
            durability: Durability::TransientLocal,
            history: History::KeepAll,
            deadline_ms: 100,
        };
        let cloned = config.clone();
        assert_eq!(cloned.domain_id, 5);
        assert_eq!(cloned.topic_prefix, "custom");
        assert_eq!(cloned.reliability, Reliability::BestEffort);
    }

    #[test]
    fn test_reliability_default() {
        assert_eq!(Reliability::default(), Reliability::Reliable);
    }

    #[test]
    fn test_durability_default() {
        assert_eq!(Durability::default(), Durability::Volatile);
    }

    #[test]
    fn test_history_default() {
        match History::default() {
            History::KeepLast(depth) => assert_eq!(depth, 10),
            _ => panic!("Expected KeepLast"),
        }
    }

    // ============================================================================
    // Topic Name Parsing Tests
    // ============================================================================

    #[test]
    fn test_parse_topic_name_category_only() {
        assert_eq!(
            parse_topic_name("asterix_cat48", "asterix"),
            (48, None, None)
        );
        assert_eq!(
            parse_topic_name("asterix_cat62", "asterix"),
            (62, None, None)
        );
    }

    #[test]
    fn test_parse_topic_name_with_sac_sic() {
        assert_eq!(
            parse_topic_name("asterix_cat48_sac1_sic2", "asterix"),
            (48, Some(1), Some(2))
        );
        assert_eq!(
            parse_topic_name("asterix_cat62_sac10_sic20", "asterix"),
            (62, Some(10), Some(20))
        );
    }

    #[test]
    fn test_parse_topic_name_different_prefix() {
        assert_eq!(parse_topic_name("radar_cat48", "radar"), (48, None, None));
    }

    #[test]
    fn test_parse_topic_name_no_prefix() {
        assert_eq!(parse_topic_name("cat48", ""), (48, None, None));
    }

    // ============================================================================
    // AsterixMessage Tests
    // ============================================================================

    #[test]
    fn test_asterix_message_keyed() {
        let msg = AsterixMessage {
            key: "asterix_cat48".to_string(),
            category: 48,
            sac: Some(1),
            sic: Some(2),
            data: vec![1, 2, 3, 4],
            timestamp: 12345,
        };
        assert_eq!(msg.key(), "asterix_cat48");
    }

    #[test]
    fn test_asterix_message_clone() {
        let msg = AsterixMessage {
            key: "test".to_string(),
            category: 62,
            sac: None,
            sic: None,
            data: vec![0xFF],
            timestamp: 0,
        };
        let cloned = msg.clone();
        assert_eq!(cloned.category, 62);
        assert_eq!(cloned.data, vec![0xFF]);
    }

    #[test]
    fn test_asterix_sample_debug() {
        let sample = AsterixSample {
            category: 48,
            sac: Some(1),
            sic: Some(2),
            data: vec![1, 2, 3],
            timestamp: 100,
            topic_name: "test".to_string(),
        };
        let debug = format!("{sample:?}");
        assert!(debug.contains("48"));
        assert!(debug.contains("Some(1)"));
    }

    // ============================================================================
    // Hex Conversion Tests
    // ============================================================================

    #[test]
    fn test_hex_to_bytes_valid() {
        let hex = "30001E3048110601060160";
        let hex_clean: String = hex.chars().filter(|c| !c.is_whitespace()).collect();
        let result: Result<Vec<u8>, _> = (0..hex_clean.len())
            .step_by(2)
            .map(|i| u8::from_str_radix(&hex_clean[i..i + 2], 16))
            .collect();
        assert!(result.is_ok());
        let bytes = result.unwrap();
        assert_eq!(bytes[0], 0x30);
        assert_eq!(bytes[1], 0x00);
    }

    #[test]
    fn test_hex_to_bytes_with_whitespace() {
        let hex = "30 00 1E 30 48";
        let hex_clean: String = hex.chars().filter(|c| !c.is_whitespace()).collect();
        let result: Result<Vec<u8>, _> = (0..hex_clean.len())
            .step_by(2)
            .map(|i| u8::from_str_radix(&hex_clean[i..i + 2], 16))
            .collect();
        assert!(result.is_ok());
    }

    // ============================================================================
    // QoS Policy Tests
    // ============================================================================

    #[test]
    fn test_config_builds_qos() {
        let config = DdsConfig::default();
        let _qos = config.build_qos();
        // If we get here without panic, QoS building works
    }

    #[test]
    fn test_config_builds_qos_best_effort() {
        let config = DdsConfig::best_effort();
        let _qos = config.build_qos();
    }

    #[test]
    fn test_config_builds_qos_with_deadline() {
        let config = DdsConfig {
            deadline_ms: 100,
            ..Default::default()
        };
        let _qos = config.build_qos();
    }

    #[test]
    fn test_config_builds_qos_keep_all() {
        let config = DdsConfig {
            history: History::KeepAll,
            ..Default::default()
        };
        let _qos = config.build_qos();
    }
}
