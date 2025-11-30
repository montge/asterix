//! Transport modules for ASTERIX data distribution
//!
//! This module provides various transport mechanisms for publishing and subscribing
//! to ASTERIX surveillance data across different network topologies.
//!
//! # Available Transports
//!
//! - `zenoh` - Edge-to-cloud pub/sub with automatic discovery (requires `zenoh` feature)
//!
//! # Feature Flags
//!
//! Each transport is behind its own feature flag to minimize dependencies:
//!
//! ```toml
//! [dependencies]
//! asterix = { version = "0.1", features = ["zenoh"] }
//! ```

#[cfg(feature = "zenoh")]
pub mod zenoh;

#[cfg(feature = "zenoh")]
pub use self::zenoh::{ZenohConfig, ZenohError, ZenohPublisher, ZenohSubscriber};
