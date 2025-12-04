//! Transport modules for ASTERIX data distribution
//!
//! This module provides various transport mechanisms for publishing and subscribing
//! to ASTERIX surveillance data across different network topologies.
//!
//! # Available Transports
//!
//! - `zenoh` - Edge-to-cloud pub/sub with automatic discovery (requires `zenoh` feature)
//! - `dds` - DDS/RTPS pub/sub for real-time systems (requires `dds` feature)
//! - `dbus` - Linux D-Bus IPC for system services (requires `dbus` feature)
//! - `can` - CAN Bus via SocketCAN for automotive/embedded (requires `can` feature)
//! - `ccsds` - CCSDS Space Packet Protocol for space mission data (requires `ccsds` feature)
//!
//! # Feature Flags
//!
//! Each transport is behind its own feature flag to minimize dependencies:
//!
//! ```toml
//! [dependencies]
//! asterix = { version = "0.1", features = ["zenoh"] }
//! # or
//! asterix = { version = "0.1", features = ["dds"] }
//! # or
//! asterix = { version = "0.1", features = ["dbus"] }
//! # or
//! asterix = { version = "0.1", features = ["can"] }
//! # or
//! asterix = { version = "0.1", features = ["ccsds"] }
//! ```

#[cfg(feature = "zenoh")]
pub mod zenoh;

#[cfg(feature = "zenoh")]
pub use self::zenoh::{ZenohConfig, ZenohError, ZenohPublisher, ZenohSubscriber};

#[cfg(feature = "dds")]
pub mod dds;

#[cfg(feature = "dds")]
pub use self::dds::{DdsConfig, DdsError, DdsPublisher, DdsSubscriber};

#[cfg(feature = "dbus")]
pub mod dbus;

#[cfg(feature = "dbus")]
pub use self::dbus::{BusType, DbusClient, DbusConfig, DbusError, DbusService};

#[cfg(all(feature = "can", target_os = "linux"))]
pub mod can;

#[cfg(all(feature = "can", target_os = "linux"))]
pub use self::can::{CanConfig, CanError, CanFrameType, CanPublisher, CanSample, CanSubscriber};

#[cfg(feature = "ccsds")]
pub mod ccsds;

#[cfg(feature = "ccsds")]
pub use self::ccsds::{
    category_from_apid, parse_ccsds_header, CcsdsConfig, CcsdsError, CcsdsMode, CcsdsPublisher,
    CcsdsSample, CcsdsSubscriber,
};
