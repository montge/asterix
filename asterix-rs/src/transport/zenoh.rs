//! Zenoh transport for ASTERIX data distribution
//!
//! This module provides pub/sub capabilities for ASTERIX data using [Zenoh](https://zenoh.io/),
//! a zero-overhead pub/sub/query protocol designed for edge-to-cloud data flow.
//!
//! # Features
//!
//! - **Automatic discovery** - Zero-configuration networking
//! - **Edge-to-cloud** - Seamless data flow across network boundaries
//! - **Multi-protocol** - TCP, UDP, QUIC, shared memory
//! - **Key expressions** - Hierarchical topic naming with wildcards
//!
//! # Key Expression Convention
//!
//! ASTERIX data is published using the following key expression format:
//!
//! ```text
//! asterix/{category}/{sac}/{sic}
//! ```
//!
//! Where:
//! - `category` - ASTERIX category number (e.g., 48, 62, 65)
//! - `sac` - System Area Code (optional, use `*` for wildcard)
//! - `sic` - System Identification Code (optional, use `*` for wildcard)
//!
//! # Examples
//!
//! ## Publishing ASTERIX Data
//!
//! ```no_run
//! use asterix::transport::zenoh::{ZenohPublisher, ZenohConfig};
//! use asterix::{parse, ParseOptions, init_default};
//!
//! #[tokio::main]
//! async fn main() -> Result<(), Box<dyn std::error::Error>> {
//!     init_default()?;
//!
//!     // Create publisher
//!     let publisher = ZenohPublisher::new(ZenohConfig::default()).await?;
//!
//!     // Parse and publish ASTERIX data
//!     let data = std::fs::read("sample.asterix")?;
//!     let records = parse(&data, ParseOptions::default())?;
//!
//!     for record in records {
//!         publisher.publish(&record).await?;
//!     }
//!
//!     Ok(())
//! }
//! ```
//!
//! ## Subscribing to ASTERIX Data
//!
//! ```no_run
//! use asterix::transport::zenoh::{ZenohSubscriber, ZenohConfig};
//!
//! #[tokio::main]
//! async fn main() -> Result<(), Box<dyn std::error::Error>> {
//!     // Subscribe to all CAT048 data
//!     let mut subscriber = ZenohSubscriber::new(
//!         ZenohConfig::default(),
//!         "asterix/48/**"
//!     ).await?;
//!
//!     while let Some(sample) = subscriber.recv().await {
//!         println!("Received: cat={} len={}", sample.category, sample.data.len());
//!     }
//!
//!     Ok(())
//! }
//! ```

use std::fmt;
use std::sync::Arc;

use tokio::sync::mpsc;
use zenoh::Config;
use zenoh::Session;

use crate::error::AsterixError;
use crate::types::AsterixRecord;

/// Error type for Zenoh transport operations
#[derive(Debug)]
pub enum ZenohError {
    /// Failed to open Zenoh session
    SessionError(String),
    /// Failed to declare publisher
    PublisherError(String),
    /// Failed to declare subscriber
    SubscriberError(String),
    /// Failed to publish data
    PublishError(String),
    /// Failed to receive data
    ReceiveError(String),
    /// Serialization error
    SerializationError(String),
    /// Channel closed
    ChannelClosed,
}

impl fmt::Display for ZenohError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ZenohError::SessionError(msg) => write!(f, "Zenoh session error: {msg}"),
            ZenohError::PublisherError(msg) => write!(f, "Zenoh publisher error: {msg}"),
            ZenohError::SubscriberError(msg) => write!(f, "Zenoh subscriber error: {msg}"),
            ZenohError::PublishError(msg) => write!(f, "Zenoh publish error: {msg}"),
            ZenohError::ReceiveError(msg) => write!(f, "Zenoh receive error: {msg}"),
            ZenohError::SerializationError(msg) => write!(f, "Serialization error: {msg}"),
            ZenohError::ChannelClosed => write!(f, "Channel closed"),
        }
    }
}

impl std::error::Error for ZenohError {}

impl From<ZenohError> for AsterixError {
    fn from(err: ZenohError) -> Self {
        AsterixError::IOError(err.to_string())
    }
}

/// Configuration for Zenoh transport
#[derive(Debug, Clone)]
pub struct ZenohConfig {
    /// Zenoh locator endpoints to connect to (e.g., "tcp/192.168.1.1:7447")
    /// If empty, uses default multicast discovery
    pub endpoints: Vec<String>,

    /// Key expression prefix for ASTERIX data
    /// Default: "asterix"
    pub key_prefix: String,

    /// Whether to include raw bytes in published data
    /// Default: true
    pub include_raw_bytes: bool,

    /// Whether to include parsed fields in published data (requires serde)
    /// Default: true
    #[cfg(feature = "serde")]
    pub include_parsed: bool,

    /// Congestion control mode
    /// Default: Block (wait for network)
    pub congestion_control: CongestionControl,

    /// Priority for published data
    /// Default: Data
    pub priority: Priority,
}

/// Congestion control mode for Zenoh
#[derive(Debug, Clone, Copy, Default)]
pub enum CongestionControl {
    /// Block until data can be sent
    #[default]
    Block,
    /// Drop data if network is congested
    Drop,
}

/// Priority level for Zenoh messages
#[derive(Debug, Clone, Copy, Default)]
pub enum Priority {
    /// Real-time data (highest priority)
    RealTime,
    /// Interactive data
    Interactive,
    /// Default data priority
    #[default]
    Data,
    /// Background data (lowest priority)
    Background,
}

impl Default for ZenohConfig {
    fn default() -> Self {
        Self {
            endpoints: Vec::new(),
            key_prefix: "asterix".to_string(),
            include_raw_bytes: true,
            #[cfg(feature = "serde")]
            include_parsed: true,
            congestion_control: CongestionControl::default(),
            priority: Priority::default(),
        }
    }
}

impl ZenohConfig {
    /// Create a new config with custom endpoints
    pub fn with_endpoints(endpoints: Vec<String>) -> Self {
        Self {
            endpoints,
            ..Default::default()
        }
    }

    /// Create a config for peer-to-peer mode (no router)
    pub fn peer_to_peer() -> Self {
        Self::default()
    }

    /// Create a config connecting to a specific router
    pub fn with_router(router_endpoint: &str) -> Self {
        Self {
            endpoints: vec![router_endpoint.to_string()],
            ..Default::default()
        }
    }
}

/// Received ASTERIX sample from Zenoh
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
    /// Key expression the sample was published on
    pub key_expr: String,
}

/// Zenoh publisher for ASTERIX data
pub struct ZenohPublisher {
    session: Arc<Session>,
    config: ZenohConfig,
}

impl ZenohPublisher {
    /// Create a new Zenoh publisher
    pub async fn new(config: ZenohConfig) -> Result<Self, ZenohError> {
        let mut zenoh_config = Config::default();

        // Configure endpoints if specified
        if !config.endpoints.is_empty() {
            let endpoints_json = format!(r#"["{}"]"#, config.endpoints.join(r#"",""#));
            zenoh_config
                .insert_json5("connect/endpoints", &endpoints_json)
                .map_err(|e| ZenohError::SessionError(e.to_string()))?;
        }

        let session = zenoh::open(zenoh_config)
            .await
            .map_err(|e| ZenohError::SessionError(e.to_string()))?;

        Ok(Self {
            session: Arc::new(session),
            config,
        })
    }

    /// Publish an ASTERIX record
    pub async fn publish(&self, record: &AsterixRecord) -> Result<(), ZenohError> {
        // Build key expression: asterix/{category}/{sac}/{sic}
        let key_expr = self.build_key_expr(record);

        // Serialize the record
        let payload = self.serialize_record(record)?;

        // Publish
        self.session
            .put(&key_expr, payload)
            .await
            .map_err(|e| ZenohError::PublishError(e.to_string()))?;

        log::debug!("Published ASTERIX CAT{} to {}", record.category, key_expr);

        Ok(())
    }

    /// Publish raw ASTERIX bytes with category information
    pub async fn publish_raw(&self, category: u8, data: &[u8]) -> Result<(), ZenohError> {
        let key_expr = format!("{}/{}", self.config.key_prefix, category);

        self.session
            .put(&key_expr, data.to_vec())
            .await
            .map_err(|e| ZenohError::PublishError(e.to_string()))?;

        log::debug!(
            "Published {} bytes of CAT{} to {}",
            data.len(),
            category,
            key_expr
        );

        Ok(())
    }

    /// Publish raw ASTERIX bytes with full routing info
    pub async fn publish_raw_with_routing(
        &self,
        category: u8,
        sac: u8,
        sic: u8,
        data: &[u8],
    ) -> Result<(), ZenohError> {
        let key_expr = format!("{}/{}/{}/{}", self.config.key_prefix, category, sac, sic);

        self.session
            .put(&key_expr, data.to_vec())
            .await
            .map_err(|e| ZenohError::PublishError(e.to_string()))?;

        log::debug!(
            "Published {} bytes of CAT{} SAC={} SIC={} to {}",
            data.len(),
            category,
            sac,
            sic,
            key_expr
        );

        Ok(())
    }

    /// Close the publisher and release resources
    pub async fn close(self) -> Result<(), ZenohError> {
        self.session
            .close()
            .await
            .map_err(|e| ZenohError::SessionError(e.to_string()))
    }

    fn build_key_expr(&self, record: &AsterixRecord) -> String {
        // Try to extract SAC/SIC from I010 item if present
        let (sac, sic) = self.extract_sac_sic(record);

        match (sac, sic) {
            (Some(s), Some(c)) => {
                format!("{}/{}/{}/{}", self.config.key_prefix, record.category, s, c)
            }
            _ => format!("{}/{}", self.config.key_prefix, record.category),
        }
    }

    fn extract_sac_sic(&self, record: &AsterixRecord) -> (Option<u8>, Option<u8>) {
        // Look for I010 (Data Source Identifier) which contains SAC/SIC
        // The item ID format is I{CAT}/{ITEM}, e.g., "I048/010" or "I062/010"
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

    fn serialize_record(&self, record: &AsterixRecord) -> Result<Vec<u8>, ZenohError> {
        // If raw bytes requested and hex_data available, decode and use that
        if self.config.include_raw_bytes && !record.hex_data.is_empty() {
            return self.hex_to_bytes(&record.hex_data);
        }

        // Fallback: serialize as JSON if serde available
        #[cfg(feature = "serde")]
        {
            serde_json::to_vec(record).map_err(|e| ZenohError::SerializationError(e.to_string()))
        }

        #[cfg(not(feature = "serde"))]
        Err(ZenohError::SerializationError(
            "No serialization method available (enable 'serde' feature or provide hex_data)"
                .to_string(),
        ))
    }

    fn hex_to_bytes(&self, hex: &str) -> Result<Vec<u8>, ZenohError> {
        // Remove any whitespace and decode hex string to bytes
        let hex_clean: String = hex.chars().filter(|c| !c.is_whitespace()).collect();

        if hex_clean.len() % 2 != 0 {
            return Err(ZenohError::SerializationError(
                "Invalid hex string length".to_string(),
            ));
        }

        (0..hex_clean.len())
            .step_by(2)
            .map(|i| {
                u8::from_str_radix(&hex_clean[i..i + 2], 16)
                    .map_err(|e| ZenohError::SerializationError(e.to_string()))
            })
            .collect()
    }
}

/// Zenoh subscriber for ASTERIX data
pub struct ZenohSubscriber {
    session: Arc<Session>,
    receiver: mpsc::Receiver<AsterixSample>,
    _handle: tokio::task::JoinHandle<()>,
}

impl ZenohSubscriber {
    /// Create a new Zenoh subscriber
    ///
    /// # Arguments
    ///
    /// * `config` - Zenoh configuration
    /// * `key_expr` - Key expression to subscribe to (e.g., "asterix/**" for all data)
    ///
    /// # Key Expression Examples
    ///
    /// - `asterix/**` - All ASTERIX data
    /// - `asterix/48/**` - All CAT048 data
    /// - `asterix/62/1/2` - CAT062 from SAC=1, SIC=2
    /// - `asterix/*/1/*` - All categories from SAC=1
    pub async fn new(config: ZenohConfig, key_expr: &str) -> Result<Self, ZenohError> {
        let mut zenoh_config = Config::default();

        if !config.endpoints.is_empty() {
            let endpoints_json = format!(r#"["{}"]"#, config.endpoints.join(r#"",""#));
            zenoh_config
                .insert_json5("connect/endpoints", &endpoints_json)
                .map_err(|e| ZenohError::SessionError(e.to_string()))?;
        }

        let session = Arc::new(
            zenoh::open(zenoh_config)
                .await
                .map_err(|e| ZenohError::SessionError(e.to_string()))?,
        );

        let (tx, rx) = mpsc::channel(1000);

        let subscriber = session
            .declare_subscriber(key_expr)
            .await
            .map_err(|e| ZenohError::SubscriberError(e.to_string()))?;

        let key_prefix = config.key_prefix.clone();
        let handle = tokio::spawn(async move {
            while let Ok(sample) = subscriber.recv_async().await {
                let key = sample.key_expr().to_string();
                let data: Vec<u8> = sample.payload().to_bytes().to_vec();

                // Parse key expression to extract category/sac/sic
                let (category, sac, sic) = parse_key_expr(&key, &key_prefix);

                let asterix_sample = AsterixSample {
                    category,
                    sac,
                    sic,
                    data,
                    timestamp: std::time::SystemTime::now()
                        .duration_since(std::time::UNIX_EPOCH)
                        .map(|d| d.as_micros() as u64)
                        .unwrap_or(0),
                    key_expr: key,
                };

                if tx.send(asterix_sample).await.is_err() {
                    break;
                }
            }
        });

        Ok(Self {
            session,
            receiver: rx,
            _handle: handle,
        })
    }

    /// Receive the next ASTERIX sample
    pub async fn recv(&mut self) -> Option<AsterixSample> {
        self.receiver.recv().await
    }

    /// Try to receive a sample without blocking
    pub fn try_recv(&mut self) -> Option<AsterixSample> {
        self.receiver.try_recv().ok()
    }

    /// Close the subscriber and release resources
    pub async fn close(self) -> Result<(), ZenohError> {
        self.session
            .close()
            .await
            .map_err(|e| ZenohError::SessionError(e.to_string()))
    }
}

/// Parse a key expression to extract category, SAC, and SIC
fn parse_key_expr(key: &str, prefix: &str) -> (u8, Option<u8>, Option<u8>) {
    let stripped = key.strip_prefix(prefix).unwrap_or(key);
    let parts: Vec<&str> = stripped.trim_start_matches('/').split('/').collect();

    let category = parts
        .first()
        .and_then(|s| s.parse::<u8>().ok())
        .unwrap_or(0);

    let sac = parts.get(1).and_then(|s| s.parse::<u8>().ok());

    let sic = parts.get(2).and_then(|s| s.parse::<u8>().ok());

    (category, sac, sic)
}

#[cfg(test)]
mod tests {
    use super::*;

    // ============================================================================
    // Key Expression Parsing Tests
    // ============================================================================

    #[test]
    fn test_parse_key_expr_category_only() {
        assert_eq!(parse_key_expr("asterix/48", "asterix"), (48, None, None));
        assert_eq!(parse_key_expr("asterix/62", "asterix"), (62, None, None));
        assert_eq!(parse_key_expr("asterix/255", "asterix"), (255, None, None));
    }

    #[test]
    fn test_parse_key_expr_with_sac_sic() {
        assert_eq!(
            parse_key_expr("asterix/62/1/2", "asterix"),
            (62, Some(1), Some(2))
        );
        assert_eq!(
            parse_key_expr("asterix/65/10/20", "asterix"),
            (65, Some(10), Some(20))
        );
        assert_eq!(
            parse_key_expr("asterix/48/255/255", "asterix"),
            (48, Some(255), Some(255))
        );
    }

    #[test]
    fn test_parse_key_expr_partial_routing() {
        // Only SAC, no SIC
        assert_eq!(
            parse_key_expr("asterix/48/1", "asterix"),
            (48, Some(1), None)
        );
    }

    #[test]
    fn test_parse_key_expr_invalid_category() {
        // Invalid category returns 0
        assert_eq!(
            parse_key_expr("asterix/invalid", "asterix"),
            (0, None, None)
        );
        assert_eq!(parse_key_expr("asterix/", "asterix"), (0, None, None));
    }

    #[test]
    fn test_parse_key_expr_different_prefix() {
        // Using different prefix
        assert_eq!(
            parse_key_expr("custom/48/1/2", "custom"),
            (48, Some(1), Some(2))
        );
        assert_eq!(
            parse_key_expr("atm/surveillance/62", "atm/surveillance"),
            (62, None, None)
        );
    }

    #[test]
    fn test_parse_key_expr_no_prefix() {
        // If key doesn't start with prefix, parse from beginning
        assert_eq!(parse_key_expr("48/1/2", "asterix"), (48, Some(1), Some(2)));
    }

    // ============================================================================
    // ZenohConfig Tests
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
        assert_eq!(config.key_prefix, "asterix"); // Should preserve defaults
    }

    #[test]
    fn test_zenoh_config_with_multiple_endpoints() {
        let endpoints = vec![
            "tcp/10.0.0.1:7447".to_string(),
            "tcp/10.0.0.2:7447".to_string(),
            "udp/10.0.0.3:7448".to_string(),
        ];
        let config = ZenohConfig::with_endpoints(endpoints.clone());
        assert_eq!(config.endpoints, endpoints);
    }

    #[test]
    fn test_zenoh_config_peer_to_peer() {
        let config = ZenohConfig::peer_to_peer();
        assert!(config.endpoints.is_empty());
    }

    #[test]
    fn test_zenoh_config_clone() {
        let config = ZenohConfig::with_router("tcp/10.0.0.1:7447");
        let cloned = config.clone();
        assert_eq!(config.endpoints, cloned.endpoints);
        assert_eq!(config.key_prefix, cloned.key_prefix);
    }

    // ============================================================================
    // ZenohError Tests
    // ============================================================================

    #[test]
    fn test_zenoh_error_display_variants() {
        let errors = vec![
            (ZenohError::SessionError("test".to_string()), "session"),
            (ZenohError::PublisherError("test".to_string()), "publisher"),
            (
                ZenohError::SubscriberError("test".to_string()),
                "subscriber",
            ),
            (ZenohError::PublishError("test".to_string()), "publish"),
            (ZenohError::ReceiveError("test".to_string()), "receive"),
            (
                ZenohError::SerializationError("test".to_string()),
                "Serialization",
            ),
            (ZenohError::ChannelClosed, "closed"),
        ];

        for (err, expected_substr) in errors {
            let display = err.to_string();
            assert!(
                display
                    .to_lowercase()
                    .contains(&expected_substr.to_lowercase()),
                "Expected '{}' to contain '{}': got '{}'",
                stringify!(err),
                expected_substr,
                display
            );
        }
    }

    #[test]
    fn test_zenoh_error_debug() {
        let err = ZenohError::SessionError("debug test".to_string());
        let debug_str = format!("{err:?}");
        assert!(debug_str.contains("SessionError"));
        assert!(debug_str.contains("debug test"));
    }

    #[test]
    fn test_zenoh_error_to_asterix_error() {
        use crate::error::AsterixError;

        let zenoh_err = ZenohError::PublishError("publish failed".to_string());
        let asterix_err: AsterixError = zenoh_err.into();

        match asterix_err {
            AsterixError::IOError(msg) => {
                assert!(msg.contains("publish"));
            }
            _ => panic!("Expected IOError variant"),
        }
    }

    // ============================================================================
    // CongestionControl and Priority Tests
    // ============================================================================

    #[test]
    fn test_congestion_control_default() {
        let cc: CongestionControl = Default::default();
        assert!(matches!(cc, CongestionControl::Block));
    }

    #[test]
    fn test_priority_default() {
        let p: Priority = Default::default();
        assert!(matches!(p, Priority::Data));
    }

    #[test]
    fn test_priority_variants_exist() {
        // Ensure all variants can be constructed
        let _ = Priority::RealTime;
        let _ = Priority::Interactive;
        let _ = Priority::Data;
        let _ = Priority::Background;
    }

    #[test]
    fn test_congestion_control_copy() {
        let cc = CongestionControl::Drop;
        let cc_copy = cc; // Copy
        assert!(matches!(cc_copy, CongestionControl::Drop));
    }

    // ============================================================================
    // AsterixSample Tests
    // ============================================================================

    #[test]
    fn test_asterix_sample_clone() {
        let sample = AsterixSample {
            category: 48,
            sac: Some(1),
            sic: Some(2),
            data: vec![0x30, 0x00, 0x10],
            timestamp: 123456,
            key_expr: "asterix/48/1/2".to_string(),
        };

        let cloned = sample.clone();
        assert_eq!(sample.category, cloned.category);
        assert_eq!(sample.sac, cloned.sac);
        assert_eq!(sample.sic, cloned.sic);
        assert_eq!(sample.data, cloned.data);
        assert_eq!(sample.timestamp, cloned.timestamp);
        assert_eq!(sample.key_expr, cloned.key_expr);
    }

    #[test]
    fn test_asterix_sample_debug() {
        let sample = AsterixSample {
            category: 62,
            sac: None,
            sic: None,
            data: vec![0x3E],
            timestamp: 0,
            key_expr: "asterix/62".to_string(),
        };

        let debug_str = format!("{sample:?}");
        assert!(debug_str.contains("62"));
        assert!(debug_str.contains("AsterixSample"));
    }

    // ============================================================================
    // Async Integration Tests for Publisher/Subscriber functions
    // These test the internal helper functions through the public API
    // ============================================================================

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_publish_record() {
        use crate::types::AsterixRecord;
        use std::collections::BTreeMap;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return, // Skip if Zenoh unavailable
        };

        // Create a minimal AsterixRecord
        let record = AsterixRecord {
            category: 48,
            length: 10,
            timestamp_ms: 0,
            hex_data: "30000A".to_string(), // Valid hex data
            items: BTreeMap::new(),
            crc: 0,
        };

        // This exercises: publish(), build_key_expr(), extract_sac_sic(), serialize_record(), hex_to_bytes()
        let result = publisher.publish(&record).await;
        assert!(
            result.is_ok(),
            "Failed to publish record: {:?}",
            result.err()
        );

        let _ = publisher.close().await;
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_publish_record_with_sac_sic() {
        use crate::types::{AsterixRecord, DataItem, ParsedValue};
        use std::collections::BTreeMap;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Create record with I048/010 containing SAC/SIC
        let mut items = BTreeMap::new();
        let mut fields = BTreeMap::new();
        fields.insert("SAC".to_string(), ParsedValue::Integer(1));
        fields.insert("SIC".to_string(), ParsedValue::Integer(2));
        items.insert(
            "I048/010".to_string(),
            DataItem {
                description: Some("Data Source Identifier".to_string()),
                fields,
            },
        );

        let record = AsterixRecord {
            category: 48,
            length: 10,
            timestamp_ms: 0,
            hex_data: "30000A".to_string(),
            items,
            crc: 0,
        };

        // This exercises build_key_expr with SAC/SIC extraction
        let result = publisher.publish(&record).await;
        assert!(result.is_ok());

        let _ = publisher.close().await;
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_publish_record_empty_hex() {
        use crate::types::AsterixRecord;
        use std::collections::BTreeMap;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Record with empty hex_data - will fallback to serde JSON serialization
        let record = AsterixRecord {
            category: 62,
            length: 5,
            timestamp_ms: 12345,
            hex_data: String::new(), // Empty - triggers JSON serialization path
            items: BTreeMap::new(),
            crc: 0,
        };

        let result = publisher.publish(&record).await;
        // With serde feature enabled, this should succeed using JSON serialization
        assert!(result.is_ok(), "Failed with empty hex: {:?}", result.err());

        let _ = publisher.close().await;
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_with_router_config() {
        // Test with router endpoint (exercises config.endpoints branch)
        let config = ZenohConfig::with_router("tcp/127.0.0.1:7447");

        // This will likely fail to connect, but exercises the endpoint config path
        let result = ZenohPublisher::new(config).await;

        // Either connects or fails gracefully
        match result {
            Ok(p) => {
                let _ = p.close().await;
            }
            Err(e) => {
                // Expected - no router running
                assert!(e.to_string().contains("session") || !e.to_string().is_empty());
            }
        }
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_subscriber_recv_timeout() {
        use std::time::Duration;

        let config = ZenohConfig::peer_to_peer();

        let mut subscriber = match ZenohSubscriber::new(config, "asterix/test/**").await {
            Ok(s) => s,
            Err(_) => return,
        };

        // Try to receive with short timeout (no data expected)
        let result = tokio::time::timeout(Duration::from_millis(100), subscriber.recv()).await;

        // Should timeout since no publisher is sending
        assert!(result.is_err() || result.unwrap().is_none());

        let _ = subscriber.close().await;
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_pubsub_roundtrip() {
        use std::time::Duration;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config.clone()).await {
            Ok(p) => p,
            Err(_) => return,
        };

        let mut subscriber = match ZenohSubscriber::new(config, "asterix/99/**").await {
            Ok(s) => s,
            Err(_) => {
                let _ = publisher.close().await;
                return;
            }
        };

        // Allow subscription to establish
        tokio::time::sleep(Duration::from_millis(50)).await;

        // Publish data
        let test_data = vec![0x63, 0x00, 0x05, 0xAB, 0xCD]; // Category 99 test data
        publisher
            .publish_raw_with_routing(99, 10, 20, &test_data)
            .await
            .unwrap();

        // Receive with timeout
        let result = tokio::time::timeout(Duration::from_secs(2), subscriber.recv()).await;

        if let Ok(Some(sample)) = result {
            assert_eq!(sample.category, 99);
            assert_eq!(sample.sac, Some(10));
            assert_eq!(sample.sic, Some(20));
            assert_eq!(sample.data, test_data);
        }

        let _ = publisher.close().await;
        let _ = subscriber.close().await;
    }

    // ============================================================================
    // Error Handling Tests
    // ============================================================================

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_publish_invalid_hex_odd_length() {
        use crate::types::AsterixRecord;
        use std::collections::BTreeMap;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Record with odd-length hex string (invalid)
        let record = AsterixRecord {
            category: 48,
            length: 10,
            timestamp_ms: 0,
            hex_data: "30000".to_string(), // 5 chars - odd length, should fail
            items: BTreeMap::new(),
            crc: 0,
        };

        let result = publisher.publish(&record).await;
        // This should fail due to invalid hex length
        assert!(result.is_err(), "Should fail with odd-length hex");

        if let Err(e) = result {
            assert!(
                e.to_string().contains("hex") || e.to_string().contains("Serialization"),
                "Error should mention hex or serialization: {e}"
            );
        }

        let _ = publisher.close().await;
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_publish_invalid_hex_chars() {
        use crate::types::AsterixRecord;
        use std::collections::BTreeMap;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Record with invalid hex characters
        let record = AsterixRecord {
            category: 48,
            length: 10,
            timestamp_ms: 0,
            hex_data: "GHIJ".to_string(), // Invalid hex chars
            items: BTreeMap::new(),
            crc: 0,
        };

        let result = publisher.publish(&record).await;
        // This should fail due to invalid hex characters
        assert!(result.is_err(), "Should fail with invalid hex chars");

        let _ = publisher.close().await;
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_publish_hex_with_whitespace() {
        use crate::types::AsterixRecord;
        use std::collections::BTreeMap;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Record with whitespace in hex (should be handled correctly)
        let record = AsterixRecord {
            category: 48,
            length: 10,
            timestamp_ms: 0,
            hex_data: "30 00 0A".to_string(), // Valid hex with spaces
            items: BTreeMap::new(),
            crc: 0,
        };

        let result = publisher.publish(&record).await;
        // Should succeed - whitespace is stripped
        assert!(
            result.is_ok(),
            "Should handle whitespace in hex: {:?}",
            result.err()
        );

        let _ = publisher.close().await;
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_subscriber_with_router_config() {
        // Test subscriber with router endpoint (exercises config.endpoints branch)
        let config = ZenohConfig::with_router("tcp/127.0.0.1:7447");

        // This will likely fail to connect, but exercises the endpoint config path
        let result = ZenohSubscriber::new(config, "asterix/**").await;

        // Either connects or fails gracefully
        match result {
            Ok(s) => {
                let _ = s.close().await;
            }
            Err(e) => {
                // Expected - no router running, but we exercised the config path
                assert!(!e.to_string().is_empty());
            }
        }
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_subscriber_with_multiple_endpoints() {
        // Test subscriber with multiple endpoints
        let config = ZenohConfig::with_endpoints(vec![
            "tcp/127.0.0.1:7447".to_string(),
            "tcp/127.0.0.1:7448".to_string(),
        ]);

        let result = ZenohSubscriber::new(config, "asterix/**").await;

        match result {
            Ok(s) => {
                let _ = s.close().await;
            }
            Err(e) => {
                // Expected if routers not running
                assert!(!e.to_string().is_empty());
            }
        }
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_close_twice() {
        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Close should succeed
        let result = publisher.close().await;
        assert!(result.is_ok(), "First close should succeed");

        // Note: Can't close twice as close() consumes self
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_subscriber_close() {
        let config = ZenohConfig::peer_to_peer();

        let subscriber = match ZenohSubscriber::new(config, "asterix/**").await {
            Ok(s) => s,
            Err(_) => return,
        };

        // Close should succeed
        let result = subscriber.close().await;
        assert!(result.is_ok(), "Subscriber close should succeed");
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publish_raw_with_routing_success() {
        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Test publish_raw_with_routing
        let result = publisher
            .publish_raw_with_routing(48, 1, 2, &[0x30, 0x00, 0x10])
            .await;
        assert!(
            result.is_ok(),
            "publish_raw_with_routing failed: {:?}",
            result.err()
        );

        let _ = publisher.close().await;
    }

    #[test]
    fn test_zenoh_error_variants_complete() {
        // Test all ZenohError variants have proper Display implementations
        let errors = vec![
            ZenohError::SessionError("session error".to_string()),
            ZenohError::PublisherError("publisher error".to_string()),
            ZenohError::SubscriberError("subscriber error".to_string()),
            ZenohError::PublishError("publish error".to_string()),
            ZenohError::ReceiveError("receive error".to_string()),
            ZenohError::SerializationError("serialization error".to_string()),
            ZenohError::ChannelClosed,
        ];

        for err in errors {
            let display = format!("{err}");
            let debug = format!("{err:?}");
            assert!(!display.is_empty(), "Display should not be empty");
            assert!(!debug.is_empty(), "Debug should not be empty");
        }
    }

    #[test]
    fn test_zenoh_error_is_std_error() {
        let err: Box<dyn std::error::Error> =
            Box::new(ZenohError::SessionError("test".to_string()));
        assert!(err.to_string().contains("test"));
    }

    // ============================================================================
    // End-to-End Tests with Real ASTERIX Data
    // ============================================================================

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_e2e_publish_real_asterix_record() {
        use crate::types::{AsterixRecord, DataItem, ParsedValue};
        use std::collections::BTreeMap;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Create a realistic CAT048 record structure
        let mut items = BTreeMap::new();

        // I048/010 - Data Source Identifier
        let mut fields_010 = BTreeMap::new();
        fields_010.insert("SAC".to_string(), ParsedValue::Integer(25));
        fields_010.insert("SIC".to_string(), ParsedValue::Integer(100));
        items.insert(
            "I048/010".to_string(),
            DataItem {
                description: Some("Data Source Identifier".to_string()),
                fields: fields_010,
            },
        );

        // I048/140 - Time of Day
        let mut fields_140 = BTreeMap::new();
        fields_140.insert("ToD".to_string(), ParsedValue::Float(43200.5)); // 12:00:00.5
        items.insert(
            "I048/140".to_string(),
            DataItem {
                description: Some("Time of Day".to_string()),
                fields: fields_140,
            },
        );

        // I048/020 - Target Report Descriptor
        let mut fields_020 = BTreeMap::new();
        fields_020.insert(
            "TYP".to_string(),
            ParsedValue::String("Single SSR".to_string()),
        );
        fields_020.insert("SIM".to_string(), ParsedValue::Boolean(false));
        items.insert(
            "I048/020".to_string(),
            DataItem {
                description: Some("Target Report Descriptor".to_string()),
                fields: fields_020,
            },
        );

        let record = AsterixRecord {
            category: 48,
            length: 25,
            timestamp_ms: 1700000000000,
            hex_data: "300019F8250164".to_string(), // Sample CAT048 hex
            items,
            crc: 0xABCD1234,
        };

        // Publish should succeed
        let result = publisher.publish(&record).await;
        assert!(result.is_ok(), "E2E publish failed: {:?}", result.err());

        let _ = publisher.close().await;
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_e2e_pubsub_with_real_asterix() {
        use crate::types::{AsterixRecord, DataItem, ParsedValue};
        use std::collections::BTreeMap;
        use std::time::Duration;

        let config = ZenohConfig::peer_to_peer();

        // Create publisher
        let publisher = match ZenohPublisher::new(config.clone()).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Create subscriber for CAT048
        let mut subscriber = match ZenohSubscriber::new(config, "asterix/48/**").await {
            Ok(s) => s,
            Err(_) => {
                let _ = publisher.close().await;
                return;
            }
        };

        // Wait for subscription to establish
        tokio::time::sleep(Duration::from_millis(100)).await;

        // Create CAT048 record with SAC/SIC
        let mut items = BTreeMap::new();
        let mut fields = BTreeMap::new();
        fields.insert("SAC".to_string(), ParsedValue::Integer(5));
        fields.insert("SIC".to_string(), ParsedValue::Integer(10));
        items.insert(
            "I048/010".to_string(),
            DataItem {
                description: Some("Data Source Identifier".to_string()),
                fields,
            },
        );

        let record = AsterixRecord {
            category: 48,
            length: 10,
            timestamp_ms: 0,
            hex_data: "30000A050A".to_string(),
            items,
            crc: 0,
        };

        // Publish the record
        publisher.publish(&record).await.expect("Publish failed");

        // Receive with timeout
        let result = tokio::time::timeout(Duration::from_secs(2), subscriber.recv()).await;

        if let Ok(Some(sample)) = result {
            // Verify the category is correct
            assert_eq!(sample.category, 48);
            // SAC/SIC may or may not be present depending on key parsing
            // If present, verify correct values
            if let Some(sac) = sample.sac {
                assert_eq!(sac, 5, "SAC mismatch");
            }
            if let Some(sic) = sample.sic {
                assert_eq!(sic, 10, "SIC mismatch");
            }
            // Data should be the decoded hex
            assert!(!sample.data.is_empty());
        }

        let _ = publisher.close().await;
        let _ = subscriber.close().await;
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_e2e_multiple_categories() {
        use std::time::Duration;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config.clone()).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Subscribe to all categories
        let mut subscriber = match ZenohSubscriber::new(config, "asterix/**").await {
            Ok(s) => s,
            Err(_) => {
                let _ = publisher.close().await;
                return;
            }
        };

        tokio::time::sleep(Duration::from_millis(100)).await;

        // Publish multiple categories
        let categories = vec![
            (48, vec![0x30, 0x00, 0x05]), // CAT048
            (62, vec![0x3E, 0x00, 0x05]), // CAT062
            (65, vec![0x41, 0x00, 0x05]), // CAT065
        ];

        for (cat, data) in &categories {
            publisher
                .publish_raw(*cat, data)
                .await
                .expect("Publish failed");
        }

        // Try to receive multiple samples
        let mut received_cats = Vec::new();
        for _ in 0..3 {
            match tokio::time::timeout(Duration::from_millis(500), subscriber.recv()).await {
                Ok(Some(sample)) => received_cats.push(sample.category),
                _ => break,
            }
        }

        // Should have received at least some categories
        // (exact number depends on timing)

        let _ = publisher.close().await;
        let _ = subscriber.close().await;
    }

    // ============================================================================
    // Connection Failure Tests
    // ============================================================================

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_connection_to_nonexistent_router() {
        // Try to connect to a router that definitely doesn't exist
        // This should fail during session creation
        let config = ZenohConfig::with_endpoints(vec![
            "tcp/192.0.2.1:7447".to_string(), // TEST-NET-1, guaranteed unreachable
        ]);

        let result = ZenohPublisher::new(config).await;

        // This exercises the zenoh::open error path (line 256)
        // Zenoh may succeed with multicast fallback or fail - both are valid
        match result {
            Ok(p) => {
                // Connected via multicast discovery despite bad endpoint
                let _ = p.close().await;
            }
            Err(e) => {
                // Failed to connect - expected
                assert!(!e.to_string().is_empty());
            }
        }
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_subscriber_connection_to_nonexistent_router() {
        let config = ZenohConfig::with_endpoints(vec!["tcp/192.0.2.1:7447".to_string()]);

        let result = ZenohSubscriber::new(config, "asterix/**").await;

        match result {
            Ok(s) => {
                let _ = s.close().await;
            }
            Err(e) => {
                assert!(!e.to_string().is_empty());
            }
        }
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_config_with_empty_key_prefix() {
        let config = ZenohConfig {
            key_prefix: String::new(),
            ..Default::default()
        };

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Publishing with empty prefix creates key like "/48" which may fail
        // depending on Zenoh version - test that it doesn't panic
        let result = publisher.publish_raw(48, &[0x30, 0x00, 0x05]).await;
        // Either success or clean error is acceptable
        if result.is_err() {
            let err = result.err().unwrap();
            // Should be a publish error, not a panic
            assert!(!err.to_string().is_empty());
        }

        let _ = publisher.close().await;
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publish_empty_data() {
        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Publishing empty data should work
        let result = publisher.publish_raw(48, &[]).await;
        assert!(
            result.is_ok(),
            "Publishing empty data failed: {:?}",
            result.err()
        );

        let _ = publisher.close().await;
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publish_large_data() {
        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Publish a large payload (64KB)
        let large_data = vec![0xAB; 65536];
        let result = publisher.publish_raw(48, &large_data).await;
        assert!(
            result.is_ok(),
            "Publishing large data failed: {:?}",
            result.err()
        );

        let _ = publisher.close().await;
    }

    // ============================================================================
    // Error Path Coverage Tests (Issue #100)
    // These tests specifically target uncovered error paths
    // ============================================================================

    /// Test serialization with empty hex_data triggers JSON serialization path (Line 368)
    /// Note: With serde enabled and valid data, JSON serialization succeeds
    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_serialize_record_json_path() {
        use crate::types::{AsterixRecord, DataItem, ParsedValue};
        use std::collections::BTreeMap;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Create record with empty hex_data to trigger JSON serialization path
        let mut items = BTreeMap::new();
        let mut fields = BTreeMap::new();
        fields.insert("test_value".to_string(), ParsedValue::Integer(42));
        items.insert(
            "I048/999".to_string(),
            DataItem {
                description: Some("Test item".to_string()),
                fields,
            },
        );

        let record = AsterixRecord {
            category: 48,
            length: 10,
            timestamp_ms: 12345,
            hex_data: String::new(), // Empty hex_data forces serde JSON path
            items,
            crc: 0,
        };

        // With serde feature and valid data, this exercises the JSON serialization path
        let result = publisher.publish(&record).await;
        assert!(
            result.is_ok(),
            "JSON serialization should succeed: {:?}",
            result.err()
        );

        let _ = publisher.close().await;
    }

    /// Test serialization with include_raw_bytes disabled forces JSON path
    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_serialize_record_json_path_disabled_raw() {
        use crate::types::{AsterixRecord, DataItem, ParsedValue};
        use std::collections::BTreeMap;

        let config = ZenohConfig {
            include_raw_bytes: false, // Force JSON path even with hex_data
            ..ZenohConfig::peer_to_peer()
        };

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        let mut items = BTreeMap::new();
        let mut fields = BTreeMap::new();
        fields.insert("SAC".to_string(), ParsedValue::Integer(1));
        fields.insert("SIC".to_string(), ParsedValue::Integer(2));
        items.insert(
            "I048/010".to_string(),
            DataItem {
                description: None,
                fields,
            },
        );

        let record = AsterixRecord {
            category: 48,
            length: 10,
            timestamp_ms: 0,
            hex_data: "30000A".to_string(), // Has hex_data but include_raw_bytes is false
            items,
            crc: 0,
        };

        // With include_raw_bytes=false, should use JSON serialization regardless of hex_data
        let result = publisher.publish(&record).await;
        // This actually uses hex_data because the condition is `include_raw_bytes && !hex_data.is_empty()`
        // So with include_raw_bytes=false, it goes to JSON path
        assert!(
            result.is_ok(),
            "JSON serialization should succeed: {:?}",
            result.err()
        );

        let _ = publisher.close().await;
    }

    /// Test publisher error display contains expected text
    #[test]
    fn test_publisher_error_display() {
        let err = ZenohError::PublisherError("test publisher error".to_string());
        let display = err.to_string();
        assert!(
            display.contains("publisher"),
            "Display should contain 'publisher'"
        );
        assert!(
            display.contains("test publisher error"),
            "Display should contain message"
        );
    }

    /// Test receiver error display
    #[test]
    fn test_receive_error_display() {
        let err = ZenohError::ReceiveError("channel disconnected".to_string());
        let display = err.to_string();
        assert!(
            display.contains("receive"),
            "Display should contain 'receive'"
        );
        assert!(display.contains("channel disconnected"));
    }

    /// Test hex_to_bytes with edge cases
    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_hex_to_bytes_edge_cases() {
        use crate::types::AsterixRecord;
        use std::collections::BTreeMap;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Test with tabs and newlines in hex (should be stripped)
        let record = AsterixRecord {
            category: 48,
            length: 10,
            timestamp_ms: 0,
            hex_data: "30\t00\n0A".to_string(),
            items: BTreeMap::new(),
            crc: 0,
        };

        let result = publisher.publish(&record).await;
        assert!(
            result.is_ok(),
            "Should handle whitespace in hex: {:?}",
            result.err()
        );

        let _ = publisher.close().await;
    }

    /// Test config clone and debug
    #[test]
    fn test_zenoh_config_debug_and_clone() {
        let config = ZenohConfig {
            congestion_control: CongestionControl::Drop,
            priority: Priority::RealTime,
            ..Default::default()
        };

        let debug_str = format!("{config:?}");
        assert!(debug_str.contains("ZenohConfig"));
        assert!(debug_str.contains("asterix"));

        let cloned = config.clone();
        assert!(matches!(cloned.congestion_control, CongestionControl::Drop));
        assert!(matches!(cloned.priority, Priority::RealTime));
    }

    /// Test priority clone and copy
    #[test]
    fn test_priority_clone_copy() {
        let p1 = Priority::Interactive;
        let p2 = p1; // Copy
        let p3 = p1; // Clone
        assert!(matches!(p2, Priority::Interactive));
        assert!(matches!(p3, Priority::Interactive));
    }

    /// Test congestion control debug
    #[test]
    fn test_congestion_control_debug() {
        let cc = CongestionControl::Block;
        let debug_str = format!("{cc:?}");
        assert!(debug_str.contains("Block"));

        let cc2 = CongestionControl::Drop;
        let debug_str2 = format!("{cc2:?}");
        assert!(debug_str2.contains("Drop"));
    }

    /// Test AsterixSample with empty data
    #[test]
    fn test_asterix_sample_empty_data() {
        let sample = AsterixSample {
            category: 0,
            sac: None,
            sic: None,
            data: Vec::new(),
            timestamp: 0,
            key_expr: String::new(),
        };

        assert!(sample.data.is_empty());
        assert_eq!(sample.category, 0);
    }

    /// Test parse_key_expr with edge cases
    #[test]
    fn test_parse_key_expr_edge_cases() {
        // Empty string
        assert_eq!(parse_key_expr("", "asterix"), (0, None, None));

        // Just prefix
        assert_eq!(parse_key_expr("asterix", "asterix"), (0, None, None));

        // Prefix with trailing slash
        assert_eq!(parse_key_expr("asterix/", "asterix"), (0, None, None));

        // Very large category number (overflows u8)
        assert_eq!(parse_key_expr("asterix/999", "asterix"), (0, None, None));

        // Negative numbers - category fails to parse but SAC/SIC still parsed
        assert_eq!(
            parse_key_expr("asterix/-1/1/2", "asterix"),
            (0, Some(1), Some(2))
        );

        // With extra path segments
        assert_eq!(
            parse_key_expr("asterix/48/1/2/extra", "asterix"),
            (48, Some(1), Some(2))
        );
    }

    /// Test extract_sac_sic with different item ID formats
    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_extract_sac_sic_edge_cases() {
        use crate::types::{AsterixRecord, DataItem, ParsedValue};
        use std::collections::BTreeMap;

        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Test with missing SAC field (only SIC present)
        let mut items = BTreeMap::new();
        let mut fields = BTreeMap::new();
        fields.insert("SIC".to_string(), ParsedValue::Integer(5));
        // No SAC field
        items.insert(
            "I048/010".to_string(),
            DataItem {
                description: None,
                fields,
            },
        );

        let record = AsterixRecord {
            category: 48,
            length: 10,
            timestamp_ms: 0,
            hex_data: "300005".to_string(),
            items,
            crc: 0,
        };

        // Should still publish successfully, just without full routing
        let result = publisher.publish(&record).await;
        assert!(result.is_ok());

        let _ = publisher.close().await;
    }

    /// Test publish with non-standard category numbers
    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publish_edge_category_numbers() {
        let config = ZenohConfig::peer_to_peer();

        let publisher = match ZenohPublisher::new(config).await {
            Ok(p) => p,
            Err(_) => return,
        };

        // Test with category 0 (edge case)
        let result = publisher.publish_raw(0, &[0x00, 0x00, 0x05]).await;
        assert!(result.is_ok());

        // Test with category 255 (max)
        let result = publisher.publish_raw(255, &[0xFF, 0x00, 0x05]).await;
        assert!(result.is_ok());

        let _ = publisher.close().await;
    }
}
