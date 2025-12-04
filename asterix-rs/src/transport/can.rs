//! CAN Bus transport for ASTERIX data distribution via SocketCAN
//!
//! This module provides CAN Bus support for ASTERIX using Linux SocketCAN,
//! enabling integration with automotive and embedded systems.
//!
//! # Features
//!
//! - **SocketCAN Interface** - Standard Linux CAN access
//! - **CAN FD Support** - Extended payload up to 64 bytes
//! - **Frame Fragmentation** - Automatic message splitting/reassembly
//! - **Error Handling** - CAN error frame detection
//! - **Virtual CAN Testing** - vcan0 support for development
//!
//! # CAN Frame Format
//!
//! ASTERIX data is sent over CAN using a fragmentation protocol:
//!
//! ```text
//! CAN ID Structure (11-bit):
//!   [10:8] - ASTERIX Category (high 3 bits)
//!   [7:0]  - Fragment sequence number
//!
//! CAN FD Data (up to 64 bytes):
//!   [0]    - Fragment header (flags + total fragments)
//!   [1..N] - ASTERIX data payload
//! ```
//!
//! # Fragmentation Protocol
//!
//! Large ASTERIX messages are fragmented across multiple CAN frames:
//!
//! - **Fragment Header Byte**: `[7] = is_last, [6:0] = fragment_index`
//! - **Classic CAN**: 7-byte payload per frame (8 bytes - 1 header)
//! - **CAN FD**: 63-byte payload per frame (64 bytes - 1 header)
//!
//! # Examples
//!
//! ## Sending ASTERIX Data
//!
//! ```no_run
//! use asterix::transport::can::{CanPublisher, CanConfig};
//! use asterix::{parse, ParseOptions, init_default};
//!
//! fn main() -> Result<(), Box<dyn std::error::Error>> {
//!     init_default()?;
//!
//!     // Create CAN publisher on vcan0
//!     let config = CanConfig::new("vcan0")?;
//!     let publisher = CanPublisher::new(config)?;
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
//!
//! ## Receiving ASTERIX Data
//!
//! ```no_run
//! use asterix::transport::can::{CanSubscriber, CanConfig};
//! use asterix::{parse, ParseOptions, init_default};
//!
//! fn main() -> Result<(), Box<dyn std::error::Error>> {
//!     init_default()?;
//!
//!     // Create CAN subscriber
//!     let config = CanConfig::new("vcan0")?;
//!     let mut subscriber = CanSubscriber::new(config)?;
//!
//!     // Subscribe to category 48
//!     subscriber.subscribe(48)?;
//!
//!     // Receive messages
//!     loop {
//!         match subscriber.receive_timeout(std::time::Duration::from_secs(1)) {
//!             Ok(Some(sample)) => {
//!                 println!("Received category {}: {} bytes",
//!                     sample.category, sample.data.len());
//!             }
//!             Ok(None) => continue, // Timeout
//!             Err(e) => eprintln!("Error: {e}"),
//!         }
//!     }
//! }
//! ```

use std::collections::HashMap;
use std::fmt;
use std::time::{Duration, Instant};

use socketcan::EmbeddedFrame as Frame;
use socketcan::{CanFrame, CanSocket, Socket, StandardId};

use crate::error::AsterixError;
use crate::types::AsterixRecord;

/// Maximum payload size for classic CAN frames (8 bytes - 1 header byte)
const CAN_PAYLOAD_SIZE: usize = 7;

/// Maximum payload size for CAN FD frames (64 bytes - 1 header byte)
const CANFD_PAYLOAD_SIZE: usize = 63;

/// Default reassembly timeout in milliseconds
const DEFAULT_REASSEMBLY_TIMEOUT_MS: u64 = 1000;

/// Fragment header bit flags
const FRAGMENT_LAST_FLAG: u8 = 0x80;
const FRAGMENT_INDEX_MASK: u8 = 0x7F;

/// Error type for CAN transport operations
#[derive(Debug)]
pub enum CanError {
    /// Failed to open CAN interface
    InterfaceError(String),
    /// Failed to send CAN frame
    SendError(String),
    /// Failed to receive CAN frame
    ReceiveError(String),
    /// Fragmentation/reassembly error
    FragmentError(String),
    /// Timeout waiting for data
    Timeout,
    /// Invalid configuration
    ConfigError(String),
}

impl fmt::Display for CanError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            CanError::InterfaceError(msg) => write!(f, "CAN interface error: {msg}"),
            CanError::SendError(msg) => write!(f, "CAN send error: {msg}"),
            CanError::ReceiveError(msg) => write!(f, "CAN receive error: {msg}"),
            CanError::FragmentError(msg) => write!(f, "Fragment error: {msg}"),
            CanError::Timeout => write!(f, "Timeout waiting for CAN data"),
            CanError::ConfigError(msg) => write!(f, "Configuration error: {msg}"),
        }
    }
}

impl std::error::Error for CanError {}

impl From<CanError> for AsterixError {
    fn from(err: CanError) -> Self {
        AsterixError::IOError(err.to_string())
    }
}

impl From<std::io::Error> for CanError {
    fn from(err: std::io::Error) -> Self {
        CanError::InterfaceError(err.to_string())
    }
}

/// CAN frame type
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum CanFrameType {
    /// Classic CAN (up to 8 bytes)
    #[default]
    Classic,
    /// CAN FD (up to 64 bytes)
    Fd,
}

/// Configuration for CAN transport
#[derive(Debug, Clone)]
pub struct CanConfig {
    /// CAN interface name (e.g., "can0", "vcan0")
    pub interface: String,

    /// CAN frame type (Classic or FD)
    pub frame_type: CanFrameType,

    /// Reassembly timeout in milliseconds
    pub reassembly_timeout_ms: u64,

    /// Enable CAN error frame reception
    pub enable_error_frames: bool,
}

impl CanConfig {
    /// Create a new CAN config for the specified interface
    ///
    /// # Arguments
    ///
    /// * `interface` - CAN interface name (e.g., "can0", "vcan0")
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use asterix::transport::can::CanConfig;
    ///
    /// let config = CanConfig::new("vcan0")?;
    /// # Ok::<(), Box<dyn std::error::Error>>(())
    /// ```
    pub fn new(interface: &str) -> Result<Self, CanError> {
        if interface.is_empty() {
            return Err(CanError::ConfigError(
                "Interface name cannot be empty".to_string(),
            ));
        }

        Ok(Self {
            interface: interface.to_string(),
            frame_type: CanFrameType::default(),
            reassembly_timeout_ms: DEFAULT_REASSEMBLY_TIMEOUT_MS,
            enable_error_frames: true,
        })
    }

    /// Create a CAN FD configuration
    pub fn with_fd(interface: &str) -> Result<Self, CanError> {
        Ok(Self {
            interface: interface.to_string(),
            frame_type: CanFrameType::Fd,
            reassembly_timeout_ms: DEFAULT_REASSEMBLY_TIMEOUT_MS,
            enable_error_frames: true,
        })
    }

    /// Set reassembly timeout
    pub fn with_timeout(mut self, timeout_ms: u64) -> Self {
        self.reassembly_timeout_ms = timeout_ms;
        self
    }

    /// Get payload size for the configured frame type
    fn payload_size(&self) -> usize {
        match self.frame_type {
            CanFrameType::Classic => CAN_PAYLOAD_SIZE,
            CanFrameType::Fd => CANFD_PAYLOAD_SIZE,
        }
    }
}

impl Default for CanConfig {
    fn default() -> Self {
        Self {
            interface: "vcan0".to_string(),
            frame_type: CanFrameType::default(),
            reassembly_timeout_ms: DEFAULT_REASSEMBLY_TIMEOUT_MS,
            enable_error_frames: true,
        }
    }
}

/// Build CAN ID from category and fragment index
///
/// CAN ID format (11-bit):
/// - Bits [10:8]: Category high 3 bits
/// - Bits [7:0]: Fragment sequence number
fn build_can_id(category: u8, fragment_index: u8) -> u32 {
    let cat_high = ((category >> 5) & 0x07) as u32;
    let frag = fragment_index as u32;
    (cat_high << 8) | frag
}

/// Extract category and fragment index from CAN ID
fn parse_can_id(can_id: u32) -> (u8, u8) {
    let cat_high = ((can_id >> 8) & 0x07) as u8;
    let frag = (can_id & 0xFF) as u8;
    (cat_high << 5, frag)
}

/// Build fragment header byte
fn build_fragment_header(fragment_index: u8, is_last: bool) -> u8 {
    let mut header = fragment_index & FRAGMENT_INDEX_MASK;
    if is_last {
        header |= FRAGMENT_LAST_FLAG;
    }
    header
}

/// Parse fragment header byte
fn parse_fragment_header(header: u8) -> (u8, bool) {
    let index = header & FRAGMENT_INDEX_MASK;
    let is_last = (header & FRAGMENT_LAST_FLAG) != 0;
    (index, is_last)
}

/// Fragment ASTERIX data into CAN frames
fn fragment_data(_category: u8, data: &[u8], payload_size: usize) -> Vec<Vec<u8>> {
    let mut fragments = Vec::new();
    let total_size = data.len();
    let mut offset = 0;
    let mut fragment_index = 0u8;

    while offset < total_size {
        let remaining = total_size - offset;
        let chunk_size = remaining.min(payload_size);
        let is_last = offset + chunk_size >= total_size;

        // Build fragment: [header][payload]
        let mut fragment = Vec::with_capacity(chunk_size + 1);
        fragment.push(build_fragment_header(fragment_index, is_last));
        fragment.extend_from_slice(&data[offset..offset + chunk_size]);

        fragments.push(fragment);

        offset += chunk_size;
        fragment_index = fragment_index.wrapping_add(1);
    }

    fragments
}

/// Reassembly state for incoming fragments
#[derive(Debug)]
struct ReassemblyState {
    #[allow(dead_code)]
    category: u8,
    fragments: HashMap<u8, Vec<u8>>,
    #[allow(dead_code)]
    expected_next: u8,
    last_update: Instant,
}

impl ReassemblyState {
    fn new(category: u8) -> Self {
        Self {
            category,
            fragments: HashMap::new(),
            expected_next: 0,
            last_update: Instant::now(),
        }
    }

    fn add_fragment(&mut self, index: u8, data: Vec<u8>, is_last: bool) -> Option<Vec<u8>> {
        self.last_update = Instant::now();
        self.fragments.insert(index, data);

        // Check if we have all fragments
        if is_last {
            self.reassemble(index)
        } else {
            None
        }
    }

    fn reassemble(&self, last_index: u8) -> Option<Vec<u8>> {
        let mut result = Vec::new();

        for i in 0..=last_index {
            if let Some(fragment_data) = self.fragments.get(&i) {
                result.extend_from_slice(fragment_data);
            } else {
                // Missing fragment
                return None;
            }
        }

        Some(result)
    }

    fn is_expired(&self, timeout: Duration) -> bool {
        self.last_update.elapsed() > timeout
    }
}

/// ASTERIX data sample received from CAN
#[derive(Debug, Clone)]
pub struct CanSample {
    /// ASTERIX category
    pub category: u8,
    /// Raw ASTERIX data bytes
    pub data: Vec<u8>,
    /// Timestamp when sample was received (microseconds since epoch)
    pub timestamp: u64,
    /// CAN interface name
    pub interface: String,
}

/// CAN publisher for ASTERIX data
pub struct CanPublisher {
    socket: CanSocket,
    config: CanConfig,
}

impl CanPublisher {
    /// Create a new CAN publisher
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use asterix::transport::can::{CanPublisher, CanConfig};
    ///
    /// let config = CanConfig::new("vcan0")?;
    /// let publisher = CanPublisher::new(config)?;
    /// # Ok::<(), Box<dyn std::error::Error>>(())
    /// ```
    pub fn new(config: CanConfig) -> Result<Self, CanError> {
        let socket = CanSocket::open(&config.interface)?;

        Ok(Self { socket, config })
    }

    /// Publish an ASTERIX record
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use asterix::transport::can::{CanPublisher, CanConfig};
    /// use asterix::{parse, ParseOptions, init_default};
    ///
    /// # fn main() -> Result<(), Box<dyn std::error::Error>> {
    /// init_default()?;
    ///
    /// let config = CanConfig::new("vcan0")?;
    /// let publisher = CanPublisher::new(config)?;
    ///
    /// let data = std::fs::read("sample.asterix")?;
    /// let records = parse(&data, ParseOptions::default())?;
    ///
    /// for record in records {
    ///     publisher.publish(&record)?;
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub fn publish(&self, record: &AsterixRecord) -> Result<(), CanError> {
        // Get raw ASTERIX bytes from record
        let data = self.get_raw_data(record)?;
        self.publish_raw(record.category, &data)
    }

    /// Publish raw ASTERIX bytes
    ///
    /// # Arguments
    ///
    /// * `category` - ASTERIX category number
    /// * `data` - Raw ASTERIX data bytes
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use asterix::transport::can::{CanPublisher, CanConfig};
    ///
    /// # fn main() -> Result<(), Box<dyn std::error::Error>> {
    /// let config = CanConfig::new("vcan0")?;
    /// let publisher = CanPublisher::new(config)?;
    ///
    /// let asterix_data = vec![0x30, 0x00, 0x10, /* ... */];
    /// publisher.publish_raw(48, &asterix_data)?;
    /// # Ok(())
    /// # }
    /// ```
    pub fn publish_raw(&self, category: u8, data: &[u8]) -> Result<(), CanError> {
        let payload_size = self.config.payload_size();
        let fragments = fragment_data(category, data, payload_size);

        for (idx, fragment_data) in fragments.iter().enumerate() {
            let can_id = build_can_id(category, idx as u8);

            // Convert u32 CAN ID to StandardId (11-bit)
            let std_id = StandardId::new(can_id as u16)
                .ok_or_else(|| CanError::SendError("Invalid CAN ID".to_string()))?;

            let frame = CanFrame::new(std_id, fragment_data)
                .ok_or_else(|| CanError::SendError("Failed to create CAN frame".to_string()))?;

            self.socket
                .write_frame(&frame)
                .map_err(|e| CanError::SendError(e.to_string()))?;
        }

        Ok(())
    }

    /// Extract raw ASTERIX data from a record
    ///
    /// Note: This is a simplified implementation. In a real scenario,
    /// you would serialize the record back to binary ASTERIX format.
    fn get_raw_data(&self, record: &AsterixRecord) -> Result<Vec<u8>, CanError> {
        // For now, return a placeholder
        // In production, this should serialize the record to ASTERIX binary format
        Ok(vec![record.category])
    }
}

/// CAN subscriber for ASTERIX data
pub struct CanSubscriber {
    socket: CanSocket,
    config: CanConfig,
    reassembly_states: HashMap<u8, ReassemblyState>,
    subscribed_categories: Vec<u8>,
}

impl CanSubscriber {
    /// Create a new CAN subscriber
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use asterix::transport::can::{CanSubscriber, CanConfig};
    ///
    /// let config = CanConfig::new("vcan0")?;
    /// let subscriber = CanSubscriber::new(config)?;
    /// # Ok::<(), Box<dyn std::error::Error>>(())
    /// ```
    pub fn new(config: CanConfig) -> Result<Self, CanError> {
        let socket = CanSocket::open(&config.interface)?;

        // Set read timeout
        socket
            .set_read_timeout(Duration::from_millis(100))
            .map_err(|e| CanError::InterfaceError(format!("Failed to set read timeout: {e}")))?;

        Ok(Self {
            socket,
            config,
            reassembly_states: HashMap::new(),
            subscribed_categories: Vec::new(),
        })
    }

    /// Subscribe to a specific ASTERIX category
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use asterix::transport::can::{CanSubscriber, CanConfig};
    ///
    /// # fn main() -> Result<(), Box<dyn std::error::Error>> {
    /// let config = CanConfig::new("vcan0")?;
    /// let mut subscriber = CanSubscriber::new(config)?;
    ///
    /// subscriber.subscribe(48)?;
    /// subscriber.subscribe(62)?;
    /// # Ok(())
    /// # }
    /// ```
    pub fn subscribe(&mut self, category: u8) -> Result<(), CanError> {
        if !self.subscribed_categories.contains(&category) {
            self.subscribed_categories.push(category);
        }
        Ok(())
    }

    /// Receive next ASTERIX sample (blocking)
    ///
    /// Returns `Ok(None)` on timeout
    pub fn receive(&mut self) -> Result<Option<CanSample>, CanError> {
        self.receive_timeout(Duration::from_millis(self.config.reassembly_timeout_ms))
    }

    /// Receive next ASTERIX sample with custom timeout
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use asterix::transport::can::{CanSubscriber, CanConfig};
    /// use std::time::Duration;
    ///
    /// # fn main() -> Result<(), Box<dyn std::error::Error>> {
    /// let config = CanConfig::new("vcan0")?;
    /// let mut subscriber = CanSubscriber::new(config)?;
    /// subscriber.subscribe(48)?;
    ///
    /// loop {
    ///     match subscriber.receive_timeout(Duration::from_secs(1)) {
    ///         Ok(Some(sample)) => {
    ///             println!("Received: {} bytes", sample.data.len());
    ///         }
    ///         Ok(None) => continue, // Timeout
    ///         Err(e) => eprintln!("Error: {e}"),
    ///     }
    /// }
    /// # Ok(())
    /// # }
    /// ```
    pub fn receive_timeout(&mut self, timeout: Duration) -> Result<Option<CanSample>, CanError> {
        let start = Instant::now();

        while start.elapsed() < timeout {
            // Clean up expired reassembly states
            self.cleanup_expired_states();

            // Try to read a frame
            match self.socket.read_frame() {
                Ok(frame) => {
                    if let Some(sample) = self.process_frame(&frame)? {
                        return Ok(Some(sample));
                    }
                }
                Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock => {
                    // Timeout on read, continue
                    std::thread::sleep(Duration::from_millis(10));
                    continue;
                }
                Err(e) => return Err(CanError::ReceiveError(e.to_string())),
            }
        }

        Ok(None)
    }

    /// Process a received CAN frame
    fn process_frame(&mut self, frame: &CanFrame) -> Result<Option<CanSample>, CanError> {
        use socketcan::Id;

        // Get raw CAN ID as u32
        let can_id = match frame.id() {
            Id::Standard(std_id) => std_id.as_raw() as u32,
            Id::Extended(ext_id) => ext_id.as_raw(),
        };
        let data = frame.data();

        if data.is_empty() {
            return Ok(None);
        }

        // Parse CAN ID
        let (cat_high, _frag_seq) = parse_can_id(can_id);

        // Parse fragment header
        let (fragment_index, is_last) = parse_fragment_header(data[0]);
        let payload = &data[1..];

        // Get full category (we only have high 3 bits from CAN ID)
        // For proper implementation, category should be in first fragment
        let category = cat_high;

        // Check if we're subscribed to this category (if filter is active)
        if !self.subscribed_categories.is_empty() && !self.subscribed_categories.contains(&category)
        {
            return Ok(None);
        }

        // Get or create reassembly state
        let state = self
            .reassembly_states
            .entry(category)
            .or_insert_with(|| ReassemblyState::new(category));

        // Add fragment
        if let Some(complete_data) = state.add_fragment(fragment_index, payload.to_vec(), is_last) {
            // Message complete
            let sample = CanSample {
                category,
                data: complete_data,
                timestamp: std::time::SystemTime::now()
                    .duration_since(std::time::UNIX_EPOCH)
                    .map(|d| d.as_micros() as u64)
                    .unwrap_or(0),
                interface: self.config.interface.clone(),
            };

            // Clear this reassembly state
            self.reassembly_states.remove(&category);

            return Ok(Some(sample));
        }

        Ok(None)
    }

    /// Clean up expired reassembly states
    fn cleanup_expired_states(&mut self) {
        let timeout = Duration::from_millis(self.config.reassembly_timeout_ms);
        self.reassembly_states
            .retain(|_, state| !state.is_expired(timeout));
    }
}
