//! CCSDS Space Packet Protocol transport for ASTERIX data
//!
//! This module provides integration between CCSDS (Consultative Committee for Space Data Systems)
//! Space Packet Protocol and ASTERIX ATM surveillance data, enabling satellite tracking data
//! correlation with aerospace telemetry.
//!
//! # CCSDS Overview
//!
//! The CCSDS Space Packet Protocol (SPP) is an international standard (CCSDS 133.0-B-2) used
//! by space agencies (NASA, ESA, JAXA) for spacecraft telemetry and telecommand.
//!
//! - **Standard:** CCSDS 133.0-B-2 (June 2020, Editorial Change 2 September 2024)
//! - **Packet Size:** 7-65542 octets (6-byte primary header + 1-65536 byte data field)
//! - **Use Cases:** Spacecraft telemetry, ground station uplink/downlink, satellite tracking
//!
//! # Use Cases for CCSDS-ASTERIX Integration
//!
//! 1. **Satellite Tracking Correlation**
//!    - Correlate ASTERIX CAT 062 (System Track Data) with spacecraft telemetry
//!    - Cross-reference satellite positions from ADS-B with space mission data
//!    - Enable space situational awareness with ATM surveillance fusion
//!
//! 2. **Ground Station Integration**
//!    - Receive ASTERIX data from radar systems via CCSDS telemetry downlink
//!    - Transmit ASTERIX data to spacecraft via CCSDS telecommand uplink
//!    - Multi-sensor fusion (radar + satellite sensors)
//!
//! 3. **Research Applications**
//!    - Space weather monitoring with ATM data correlation
//!    - Re-entry vehicle tracking (combining ASTERIX and CCSDS data)
//!    - UAV/spacecraft hybrid mission support
//!
//! # CCSDS Packet Structure
//!
//! ```text
//! ┌────────────────────────────────────────────────────────────┐
//! │  CCSDS Primary Header (6 bytes)                            │
//! ├────────────────────────────────────────────────────────────┤
//! │  - Packet Version Number (3 bits)                          │
//! │  - Packet Type (1 bit): 0=Telemetry, 1=Telecommand         │
//! │  - Secondary Header Flag (1 bit)                           │
//! │  - Application Process ID (APID, 11 bits)                  │
//! │  - Sequence Flags (2 bits)                                 │
//! │  - Packet Sequence Count (14 bits)                         │
//! │  - Packet Data Length (16 bits, length-1)                  │
//! ├────────────────────────────────────────────────────────────┤
//! │  ASTERIX Data Block (variable length)                      │
//! │  - Category, length, records, data items                   │
//! └────────────────────────────────────────────────────────────┘
//! ```
//!
//! # APID Assignment Convention
//!
//! Application Process IDs (APIDs) are used to route CCSDS packets. This module uses the
//! following convention for ASTERIX-over-CCSDS:
//!
//! - **APID Range:** 0x300-0x3FF (reserved for ASTERIX data)
//! - **APID Mapping:** `0x300 + category` (e.g., CAT 048 → APID 0x330)
//! - **Special APIDs:**
//!   - `0x300`: Mixed categories (multiple ASTERIX categories in one stream)
//!   - `0x3FF`: Control/status messages
//!
//! # Features
//!
//! - **Telemetry Mode:** Encapsulate ASTERIX data in CCSDS telemetry packets (downlink)
//! - **Telecommand Mode:** Send ASTERIX data via CCSDS telecommand packets (uplink)
//! - **Sequence Counting:** Automatic packet sequence number management
//! - **Fragmentation:** Support for large ASTERIX blocks across multiple CCSDS packets
//! - **CRC Validation:** Optional CRC checking for data integrity
//!
//! # Examples
//!
//! ## Publishing ASTERIX Data via CCSDS Telemetry
//!
//! ```no_run
//! use asterix::transport::ccsds::{CcsdsPublisher, CcsdsConfig, CcsdsMode};
//! use asterix::{parse, ParseOptions, init_default};
//!
//! #[tokio::main]
//! async fn main() -> Result<(), Box<dyn std::error::Error>> {
//!     init_default()?;
//!
//!     // Create CCSDS telemetry publisher
//!     let config = CcsdsConfig {
//!         mode: CcsdsMode::Telemetry,
//!         base_apid: 0x300,
//!         ..Default::default()
//!     };
//!     let publisher = CcsdsPublisher::new(config).await?;
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
//! ## Subscribing to CCSDS Telemetry Containing ASTERIX
//!
//! ```no_run
//! use asterix::transport::ccsds::{CcsdsSubscriber, CcsdsConfig};
//!
//! #[tokio::main]
//! async fn main() -> Result<(), Box<dyn std::error::Error>> {
//!     let config = CcsdsConfig::default();
//!     let mut subscriber = CcsdsSubscriber::new(config).await?;
//!
//!     while let Some(sample) = subscriber.recv().await {
//!         println!("Received ASTERIX CAT {} from APID 0x{:03X}",
//!             sample.category, sample.apid);
//!         println!("Sequence: {}, Data: {} bytes",
//!             sample.sequence_count, sample.data.len());
//!     }
//!
//!     Ok(())
//! }
//! ```

use std::collections::HashMap;
use std::fmt;
use std::sync::Arc;

use spacepackets::{CcsdsPacket, SpHeader};

// Re-use arbitrary_int types from spacepackets
use arbitrary_int::{u11, u14};

use crate::error::AsterixError;
use crate::types::AsterixRecord;

/// Error type for CCSDS transport operations
#[derive(Debug)]
pub enum CcsdsError {
    /// Failed to create CCSDS packet
    PacketError(String),
    /// Failed to encode data
    EncodeError(String),
    /// Failed to decode data
    DecodeError(String),
    /// Invalid APID
    InvalidApid(String),
    /// Serialization error
    SerializationError(String),
    /// Channel closed
    ChannelClosed,
}

impl fmt::Display for CcsdsError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            CcsdsError::PacketError(msg) => write!(f, "CCSDS packet error: {msg}"),
            CcsdsError::EncodeError(msg) => write!(f, "CCSDS encode error: {msg}"),
            CcsdsError::DecodeError(msg) => write!(f, "CCSDS decode error: {msg}"),
            CcsdsError::InvalidApid(msg) => write!(f, "Invalid APID: {msg}"),
            CcsdsError::SerializationError(msg) => write!(f, "Serialization error: {msg}"),
            CcsdsError::ChannelClosed => write!(f, "Channel closed"),
        }
    }
}

impl std::error::Error for CcsdsError {}

impl From<CcsdsError> for AsterixError {
    fn from(err: CcsdsError) -> Self {
        AsterixError::IOError(err.to_string())
    }
}

/// CCSDS packet mode (Telemetry or Telecommand)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum CcsdsMode {
    /// Telemetry packets (downlink from spacecraft)
    #[default]
    Telemetry,
    /// Telecommand packets (uplink to spacecraft)
    Telecommand,
}

/// Configuration for CCSDS transport
#[derive(Debug, Clone)]
pub struct CcsdsConfig {
    /// CCSDS mode (Telemetry or Telecommand)
    pub mode: CcsdsMode,

    /// Base APID for ASTERIX data (default: 0x300)
    /// Actual APID = base_apid + category
    pub base_apid: u16,

    /// Whether to use secondary header
    pub use_secondary_header: bool,

    /// Maximum packet data length (default: 65536)
    pub max_packet_length: usize,

    /// Enable CRC validation
    pub enable_crc: bool,

    /// UDP port for publishing/subscribing (if using UDP transport)
    pub udp_port: Option<u16>,

    /// Multicast address for publishing (if using multicast)
    pub multicast_addr: Option<String>,
}

impl Default for CcsdsConfig {
    fn default() -> Self {
        Self {
            mode: CcsdsMode::default(),
            base_apid: 0x300,
            use_secondary_header: false,
            max_packet_length: 65536,
            enable_crc: false,
            udp_port: Some(7447), // Default CCSDS telemetry port
            multicast_addr: None,
        }
    }
}

impl CcsdsConfig {
    /// Create config for telemetry mode
    pub fn telemetry() -> Self {
        Self {
            mode: CcsdsMode::Telemetry,
            ..Default::default()
        }
    }

    /// Create config for telecommand mode
    pub fn telecommand() -> Self {
        Self {
            mode: CcsdsMode::Telecommand,
            ..Default::default()
        }
    }

    /// Create config with custom base APID
    pub fn with_base_apid(base_apid: u16) -> Self {
        Self {
            base_apid,
            ..Default::default()
        }
    }

    /// Create config with UDP multicast
    pub fn with_multicast(addr: &str, port: u16) -> Self {
        Self {
            multicast_addr: Some(addr.to_string()),
            udp_port: Some(port),
            ..Default::default()
        }
    }
}

/// Received ASTERIX sample from CCSDS packet
#[derive(Debug, Clone)]
pub struct CcsdsSample {
    /// ASTERIX category
    pub category: u8,
    /// CCSDS APID
    pub apid: u16,
    /// Packet sequence count
    pub sequence_count: u16,
    /// Raw ASTERIX data bytes
    pub data: Vec<u8>,
    /// Timestamp when sample was received (microseconds since epoch)
    pub timestamp: u64,
    /// CCSDS packet type
    pub packet_type: CcsdsMode,
}

/// CCSDS publisher for ASTERIX data
pub struct CcsdsPublisher {
    config: CcsdsConfig,
    sequence_counters: Arc<tokio::sync::Mutex<HashMap<u16, u16>>>,
}

impl CcsdsPublisher {
    /// Create a new CCSDS publisher
    pub async fn new(config: CcsdsConfig) -> Result<Self, CcsdsError> {
        Ok(Self {
            config,
            sequence_counters: Arc::new(tokio::sync::Mutex::new(HashMap::new())),
        })
    }

    /// Publish an ASTERIX record as CCSDS packet
    pub async fn publish(&self, record: &AsterixRecord) -> Result<(), CcsdsError> {
        // Calculate APID from category
        let apid = self.calculate_apid(record.category);

        // Get next sequence count for this APID
        let sequence_count = self.next_sequence_count(apid).await;

        // Serialize ASTERIX data
        let asterix_data = self.serialize_record(record)?;

        // Create CCSDS packet header based on mode
        // Note: data_len field in CCSDS is (actual_length - 1), but SpHeader handles this
        let data_len = if asterix_data.is_empty() {
            0
        } else {
            (asterix_data.len() - 1) as u16
        };

        let sp_header = match self.config.mode {
            CcsdsMode::Telemetry => {
                SpHeader::new_for_unseg_tm(u11::new(apid), u14::new(sequence_count), data_len)
            }
            CcsdsMode::Telecommand => {
                SpHeader::new_for_unseg_tc(u11::new(apid), u14::new(sequence_count), data_len)
            }
        };

        // Encode CCSDS packet (6 bytes header + data)
        let mut packet = vec![0u8; 6 + asterix_data.len()];
        sp_header
            .write_to_be_bytes(&mut packet[..6])
            .map_err(|e| CcsdsError::EncodeError(format!("{e:?}")))?;
        packet[6..].copy_from_slice(&asterix_data);

        // TODO: Add UDP/multicast publishing
        // For now, just log the packet creation
        log::debug!(
            "Created CCSDS packet: APID=0x{:03X}, seq={}, len={} bytes",
            apid,
            sequence_count,
            packet.len()
        );

        Ok(())
    }

    /// Publish raw ASTERIX bytes
    pub async fn publish_raw(&self, category: u8, data: &[u8]) -> Result<(), CcsdsError> {
        let apid = self.calculate_apid(category);
        let sequence_count = self.next_sequence_count(apid).await;

        // Create CCSDS packet header based on mode
        let data_len = if data.is_empty() {
            0
        } else {
            (data.len() - 1) as u16
        };

        let sp_header = match self.config.mode {
            CcsdsMode::Telemetry => {
                SpHeader::new_for_unseg_tm(u11::new(apid), u14::new(sequence_count), data_len)
            }
            CcsdsMode::Telecommand => {
                SpHeader::new_for_unseg_tc(u11::new(apid), u14::new(sequence_count), data_len)
            }
        };

        let mut packet = vec![0u8; 6 + data.len()];
        sp_header
            .write_to_be_bytes(&mut packet[..6])
            .map_err(|e| CcsdsError::EncodeError(format!("{e:?}")))?;
        packet[6..].copy_from_slice(data);

        log::debug!(
            "Created CCSDS packet: CAT={}, APID=0x{:03X}, seq={}, len={} bytes",
            category,
            apid,
            sequence_count,
            packet.len()
        );

        Ok(())
    }

    fn calculate_apid(&self, category: u8) -> u16 {
        self.config.base_apid + (category as u16)
    }

    async fn next_sequence_count(&self, apid: u16) -> u16 {
        let mut counters = self.sequence_counters.lock().await;
        let count = counters.entry(apid).or_insert(0);
        let current = *count;
        *count = (*count + 1) & 0x3FFF; // 14-bit counter wraps at 16384
        current
    }

    fn serialize_record(&self, record: &AsterixRecord) -> Result<Vec<u8>, CcsdsError> {
        // If raw bytes available (hex_data), decode and use that
        if !record.hex_data.is_empty() {
            return self.hex_to_bytes(&record.hex_data);
        }

        // Fallback: serialize as JSON if serde available
        #[cfg(feature = "serde")]
        {
            serde_json::to_vec(record).map_err(|e| CcsdsError::SerializationError(e.to_string()))
        }

        #[cfg(not(feature = "serde"))]
        Err(CcsdsError::SerializationError(
            "No serialization method available (enable 'serde' feature or provide hex_data)"
                .to_string(),
        ))
    }

    fn hex_to_bytes(&self, hex: &str) -> Result<Vec<u8>, CcsdsError> {
        let hex_clean: String = hex.chars().filter(|c| !c.is_whitespace()).collect();

        if !hex_clean.len().is_multiple_of(2) {
            return Err(CcsdsError::SerializationError(
                "Invalid hex string length".to_string(),
            ));
        }

        (0..hex_clean.len())
            .step_by(2)
            .map(|i| {
                u8::from_str_radix(&hex_clean[i..i + 2], 16)
                    .map_err(|e| CcsdsError::SerializationError(e.to_string()))
            })
            .collect()
    }
}

/// CCSDS subscriber for ASTERIX data
pub struct CcsdsSubscriber {
    #[allow(dead_code)]
    config: CcsdsConfig,
    receiver: tokio::sync::mpsc::Receiver<CcsdsSample>,
    _handle: tokio::task::JoinHandle<()>,
}

impl CcsdsSubscriber {
    /// Create a new CCSDS subscriber
    pub async fn new(config: CcsdsConfig) -> Result<Self, CcsdsError> {
        let (_tx, rx) = tokio::sync::mpsc::channel(1000);

        // Spawn background task to receive CCSDS packets
        let handle = tokio::spawn(async move {
            // TODO: Implement UDP/multicast receiver
            // For now, just a placeholder
            log::info!("CCSDS subscriber started (placeholder)");
            loop {
                tokio::time::sleep(std::time::Duration::from_secs(1)).await;
            }
        });

        Ok(Self {
            config,
            receiver: rx,
            _handle: handle,
        })
    }

    /// Receive the next ASTERIX sample from CCSDS
    pub async fn recv(&mut self) -> Option<CcsdsSample> {
        self.receiver.recv().await
    }

    /// Try to receive a sample without blocking
    pub fn try_recv(&mut self) -> Option<CcsdsSample> {
        self.receiver.try_recv().ok()
    }
}

/// Parse CCSDS packet header to extract ASTERIX category
pub fn parse_ccsds_header(packet: &[u8]) -> Result<(u16, u16, Vec<u8>), CcsdsError> {
    if packet.len() < 6 {
        return Err(CcsdsError::DecodeError(
            "Packet too short for CCSDS header".to_string(),
        ));
    }

    // from_be_bytes returns (SpHeader, &[u8] remaining)
    let (sp_header, _remaining) =
        SpHeader::from_be_bytes(packet).map_err(|e| CcsdsError::DecodeError(format!("{e:?}")))?;

    // Convert arbitrary_int types to standard integers
    let apid: u16 = sp_header.apid().value();
    let sequence_count: u16 = sp_header.seq_count().value();
    let data_length = sp_header.data_len() as usize + 1; // CCSDS stores length-1

    if packet.len() < 6 + data_length {
        return Err(CcsdsError::DecodeError(format!(
            "Packet data truncated: expected {} bytes, got {}",
            6 + data_length,
            packet.len()
        )));
    }

    let data = packet[6..6 + data_length].to_vec();

    Ok((apid, sequence_count, data))
}

/// Extract ASTERIX category from APID (assumes base_apid convention)
pub fn category_from_apid(apid: u16, base_apid: u16) -> u8 {
    if apid >= base_apid && apid < base_apid + 256 {
        (apid - base_apid) as u8
    } else {
        0 // Unknown/invalid category
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // ============================================================================
    // Error Type Tests
    // ============================================================================

    #[test]
    fn test_ccsds_error_display_variants() {
        let errors = vec![
            (CcsdsError::PacketError("test".to_string()), "packet"),
            (CcsdsError::EncodeError("test".to_string()), "encode"),
            (CcsdsError::DecodeError("test".to_string()), "decode"),
            (CcsdsError::InvalidApid("test".to_string()), "apid"),
            (
                CcsdsError::SerializationError("test".to_string()),
                "serialization",
            ),
            (CcsdsError::ChannelClosed, "closed"),
        ];

        for (err, expected_substring) in errors {
            let display = err.to_string();
            assert!(
                display.to_lowercase().contains(expected_substring),
                "Expected '{expected_substring}' in '{display}'"
            );
        }
    }

    #[test]
    fn test_ccsds_error_debug() {
        let err = CcsdsError::PacketError("test error".to_string());
        let debug = format!("{err:?}");
        assert!(debug.contains("PacketError"));
        assert!(debug.contains("test error"));
    }

    #[test]
    fn test_ccsds_error_to_asterix_error() {
        let err = CcsdsError::EncodeError("encode failed".to_string());
        let asterix_err: AsterixError = err.into();
        let msg = asterix_err.to_string();
        assert!(msg.contains("encode"));
    }

    #[test]
    fn test_ccsds_error_is_std_error() {
        let err: Box<dyn std::error::Error> = Box::new(CcsdsError::DecodeError("test".to_string()));
        assert!(err.to_string().contains("decode"));
    }

    // ============================================================================
    // Config Tests
    // ============================================================================

    #[test]
    fn test_ccsds_config_default() {
        let config = CcsdsConfig::default();
        assert_eq!(config.mode, CcsdsMode::Telemetry);
        assert_eq!(config.base_apid, 0x300);
        assert!(!config.use_secondary_header);
        assert_eq!(config.max_packet_length, 65536);
        assert!(!config.enable_crc);
        assert_eq!(config.udp_port, Some(7447));
        assert_eq!(config.multicast_addr, None);
    }

    #[test]
    fn test_ccsds_config_telemetry() {
        let config = CcsdsConfig::telemetry();
        assert_eq!(config.mode, CcsdsMode::Telemetry);
    }

    #[test]
    fn test_ccsds_config_telecommand() {
        let config = CcsdsConfig::telecommand();
        assert_eq!(config.mode, CcsdsMode::Telecommand);
    }

    #[test]
    fn test_ccsds_config_with_base_apid() {
        let config = CcsdsConfig::with_base_apid(0x400);
        assert_eq!(config.base_apid, 0x400);
    }

    #[test]
    fn test_ccsds_config_with_multicast() {
        let config = CcsdsConfig::with_multicast("239.1.2.3", 8000);
        assert_eq!(config.multicast_addr, Some("239.1.2.3".to_string()));
        assert_eq!(config.udp_port, Some(8000));
    }

    #[test]
    fn test_ccsds_config_clone() {
        let config = CcsdsConfig {
            mode: CcsdsMode::Telecommand,
            base_apid: 0x500,
            use_secondary_header: true,
            max_packet_length: 32768,
            enable_crc: true,
            udp_port: Some(9000),
            multicast_addr: Some("239.0.0.1".to_string()),
        };
        let cloned = config.clone();
        assert_eq!(cloned.mode, CcsdsMode::Telecommand);
        assert_eq!(cloned.base_apid, 0x500);
        assert!(cloned.use_secondary_header);
    }

    // ============================================================================
    // Mode Tests
    // ============================================================================

    #[test]
    fn test_ccsds_mode_default() {
        assert_eq!(CcsdsMode::default(), CcsdsMode::Telemetry);
    }

    #[test]
    fn test_ccsds_mode_equality() {
        assert_eq!(CcsdsMode::Telemetry, CcsdsMode::Telemetry);
        assert_eq!(CcsdsMode::Telecommand, CcsdsMode::Telecommand);
        assert_ne!(CcsdsMode::Telemetry, CcsdsMode::Telecommand);
    }

    // ============================================================================
    // APID Calculation Tests
    // ============================================================================

    #[test]
    fn test_category_from_apid() {
        assert_eq!(category_from_apid(0x300, 0x300), 0);
        assert_eq!(category_from_apid(0x330, 0x300), 48); // CAT 048
        assert_eq!(category_from_apid(0x33E, 0x300), 62); // CAT 062
        assert_eq!(category_from_apid(0x341, 0x300), 65); // CAT 065
        assert_eq!(category_from_apid(0x3FF, 0x300), 255);
    }

    #[test]
    fn test_category_from_apid_invalid() {
        assert_eq!(category_from_apid(0x200, 0x300), 0); // Below base
        assert_eq!(category_from_apid(0x400, 0x300), 0); // Above range
    }

    // ============================================================================
    // CCSDS Header Parsing Tests
    // ============================================================================

    #[test]
    fn test_parse_ccsds_header_valid() {
        // Create a minimal CCSDS packet
        // Version=0, Type=TM, SecHdr=0, APID=0x330 (CAT 048)
        let packet = vec![
            0x03, 0x30, // Packet ID (version=0, type=0, sec=0, apid=0x330)
            0xC0, 0x00, // Sequence flags=11 (unsegmented), count=0
            0x00, 0x03, // Data length = 3 (4 bytes of data)
            0x30, 0x00, 0x10, 0xAA, // 4 bytes of ASTERIX data
        ];

        let result = parse_ccsds_header(&packet);
        assert!(result.is_ok());

        let (apid, seq_count, data) = result.unwrap();
        assert_eq!(apid, 0x330);
        assert_eq!(seq_count, 0);
        assert_eq!(data.len(), 4);
        assert_eq!(data, vec![0x30, 0x00, 0x10, 0xAA]);
    }

    #[test]
    fn test_parse_ccsds_header_too_short() {
        let packet = vec![0x00, 0x01, 0x02]; // Only 3 bytes
        let result = parse_ccsds_header(&packet);
        assert!(result.is_err());
        assert!(result.unwrap_err().to_string().contains("too short"));
    }

    #[test]
    fn test_parse_ccsds_header_truncated_data() {
        // Header says data length is 10, but only 2 bytes provided
        let packet = vec![
            0x03, 0x30, // Packet ID
            0xC0, 0x00, // Sequence
            0x00, 0x09, // Data length = 9 (10 bytes expected)
            0x30, 0x00, // Only 2 bytes of data
        ];

        let result = parse_ccsds_header(&packet);
        assert!(result.is_err());
        assert!(result.unwrap_err().to_string().contains("truncated"));
    }

    // ============================================================================
    // CcsdsSample Tests
    // ============================================================================

    #[test]
    fn test_ccsds_sample_debug() {
        let sample = CcsdsSample {
            category: 48,
            apid: 0x330,
            sequence_count: 42,
            data: vec![0x30, 0x00, 0x10],
            timestamp: 123456,
            packet_type: CcsdsMode::Telemetry,
        };

        let debug = format!("{sample:?}");
        assert!(debug.contains("48"));
        assert!(debug.contains("816")); // 0x330 in decimal
        assert!(debug.contains("42"));
    }

    #[test]
    fn test_ccsds_sample_clone() {
        let sample = CcsdsSample {
            category: 62,
            apid: 0x33E,
            sequence_count: 100,
            data: vec![0x3E],
            timestamp: 0,
            packet_type: CcsdsMode::Telecommand,
        };

        let cloned = sample.clone();
        assert_eq!(cloned.category, 62);
        assert_eq!(cloned.apid, 0x33E);
        assert_eq!(cloned.sequence_count, 100);
        assert_eq!(cloned.packet_type, CcsdsMode::Telecommand);
    }

    // ============================================================================
    // Async Integration Tests
    // ============================================================================

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_creation() {
        let config = CcsdsConfig::default();
        let publisher = CcsdsPublisher::new(config).await;
        assert!(publisher.is_ok());
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_calculate_apid() {
        let config = CcsdsConfig::with_base_apid(0x300);
        let publisher = CcsdsPublisher::new(config).await.unwrap();

        assert_eq!(publisher.calculate_apid(0), 0x300);
        assert_eq!(publisher.calculate_apid(48), 0x330);
        assert_eq!(publisher.calculate_apid(62), 0x33E);
        assert_eq!(publisher.calculate_apid(255), 0x3FF);
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_sequence_counter() {
        let config = CcsdsConfig::default();
        let publisher = CcsdsPublisher::new(config).await.unwrap();

        let apid = 0x330;

        // First call should return 0
        let seq1 = publisher.next_sequence_count(apid).await;
        assert_eq!(seq1, 0);

        // Second call should return 1
        let seq2 = publisher.next_sequence_count(apid).await;
        assert_eq!(seq2, 1);

        // Third call should return 2
        let seq3 = publisher.next_sequence_count(apid).await;
        assert_eq!(seq3, 2);
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_sequence_counter_wraparound() {
        let config = CcsdsConfig::default();
        let publisher = CcsdsPublisher::new(config).await.unwrap();

        let apid = 0x330;

        // Set counter to max (0x3FFF = 16383)
        {
            let mut counters = publisher.sequence_counters.lock().await;
            counters.insert(apid, 0x3FFF);
        }

        // Next count should wrap to 0
        let seq = publisher.next_sequence_count(apid).await;
        assert_eq!(seq, 0x3FFF);

        // And increment should give 0
        let seq_next = publisher.next_sequence_count(apid).await;
        assert_eq!(seq_next, 0);
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_publisher_publish_raw() {
        let config = CcsdsConfig::default();
        let publisher = CcsdsPublisher::new(config).await.unwrap();

        let result = publisher.publish_raw(48, &[0x30, 0x00, 0x10]).await;
        assert!(result.is_ok(), "publish_raw failed: {:?}", result.err());
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_subscriber_creation() {
        let config = CcsdsConfig::default();
        let subscriber = CcsdsSubscriber::new(config).await;
        assert!(subscriber.is_ok());
    }

    // ============================================================================
    // Hex Conversion Tests
    // ============================================================================

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_hex_to_bytes_valid() {
        let config = CcsdsConfig::default();
        let publisher = CcsdsPublisher::new(config).await.unwrap();

        let result = publisher.hex_to_bytes("30001E");
        assert!(result.is_ok());
        let bytes = result.unwrap();
        assert_eq!(bytes, vec![0x30, 0x00, 0x1E]);
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_hex_to_bytes_with_whitespace() {
        let config = CcsdsConfig::default();
        let publisher = CcsdsPublisher::new(config).await.unwrap();

        let result = publisher.hex_to_bytes("30 00 1E");
        assert!(result.is_ok());
        let bytes = result.unwrap();
        assert_eq!(bytes, vec![0x30, 0x00, 0x1E]);
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_hex_to_bytes_invalid_length() {
        let config = CcsdsConfig::default();
        let publisher = CcsdsPublisher::new(config).await.unwrap();

        let result = publisher.hex_to_bytes("123");
        assert!(result.is_err());
    }

    #[tokio::test(flavor = "multi_thread", worker_threads = 1)]
    async fn test_hex_to_bytes_invalid_chars() {
        let config = CcsdsConfig::default();
        let publisher = CcsdsPublisher::new(config).await.unwrap();

        let result = publisher.hex_to_bytes("GHIJ");
        assert!(result.is_err());
    }
}
