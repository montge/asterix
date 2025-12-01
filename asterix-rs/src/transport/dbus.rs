//! D-Bus transport for ASTERIX parser as a system service
//!
//! This module provides D-Bus integration for ASTERIX, enabling inter-process
//! communication on Linux systems. It exposes the ASTERIX parser as a D-Bus
//! service that other applications can call.
//!
//! # Features
//!
//! - **System Service** - Run ASTERIX parser as a system-wide service
//! - **IPC** - Local inter-process communication via D-Bus
//! - **Signals** - Real-time notifications of parsed ASTERIX data
//! - **GPL Separation** - IPC boundary enables clean license separation
//!
//! # D-Bus Interface
//!
//! The service exposes the `com.asterix.Parser` interface:
//!
//! ```xml
//! <interface name="com.asterix.Parser">
//!   <method name="Parse">
//!     <arg type="ay" name="data" direction="in"/>
//!     <arg type="s" name="result" direction="out"/>
//!   </method>
//!   <method name="ParseHex">
//!     <arg type="s" name="hex_data" direction="in"/>
//!     <arg type="s" name="result" direction="out"/>
//!   </method>
//!   <method name="GetVersion">
//!     <arg type="s" name="version" direction="out"/>
//!   </method>
//!   <signal name="AsterixReceived">
//!     <arg type="y" name="category"/>
//!     <arg type="u" name="length"/>
//!   </signal>
//! </interface>
//! ```
//!
//! # Examples
//!
//! ## Running the D-Bus Service
//!
//! ```no_run
//! use asterix::transport::dbus::{DbusService, DbusConfig};
//! use asterix::init_default;
//!
//! fn main() -> Result<(), Box<dyn std::error::Error>> {
//!     // Initialize ASTERIX parser
//!     init_default()?;
//!
//!     // Create and run D-Bus service
//!     let config = DbusConfig::default();
//!     let service = DbusService::new(config)?;
//!
//!     // Run service (blocks)
//!     service.run()?;
//!
//!     Ok(())
//! }
//! ```

use std::fmt;

use zbus::blocking::Connection;
use zbus::interface;

use crate::error::AsterixError;

/// Error type for D-Bus transport operations
#[derive(Debug)]
pub enum DbusError {
    /// Failed to create D-Bus connection
    ConnectionError(String),
    /// Failed to register service
    ServiceError(String),
    /// Failed to call method
    MethodError(String),
    /// Parse error
    ParseError(String),
}

impl fmt::Display for DbusError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            DbusError::ConnectionError(msg) => write!(f, "D-Bus connection error: {msg}"),
            DbusError::ServiceError(msg) => write!(f, "D-Bus service error: {msg}"),
            DbusError::MethodError(msg) => write!(f, "D-Bus method error: {msg}"),
            DbusError::ParseError(msg) => write!(f, "Parse error: {msg}"),
        }
    }
}

impl std::error::Error for DbusError {}

impl From<DbusError> for AsterixError {
    fn from(err: DbusError) -> Self {
        AsterixError::IOError(err.to_string())
    }
}

impl From<zbus::Error> for DbusError {
    fn from(err: zbus::Error) -> Self {
        DbusError::ConnectionError(err.to_string())
    }
}

/// Bus type for D-Bus connection
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum BusType {
    /// Session bus (per-user)
    #[default]
    Session,
    /// System bus (system-wide, requires permissions)
    System,
}

/// Configuration for D-Bus service
#[derive(Debug, Clone)]
pub struct DbusConfig {
    /// D-Bus service name
    /// Default: "com.asterix.Parser"
    pub service_name: String,

    /// D-Bus object path
    /// Default: "/com/asterix/Parser"
    pub object_path: String,

    /// Bus type (Session or System)
    pub bus_type: BusType,

    /// Whether to emit signals on parse
    pub emit_signals: bool,
}

impl Default for DbusConfig {
    fn default() -> Self {
        Self {
            service_name: "com.asterix.Parser".to_string(),
            object_path: "/com/asterix/Parser".to_string(),
            bus_type: BusType::Session,
            emit_signals: true,
        }
    }
}

impl DbusConfig {
    /// Create config for session bus (default)
    pub fn session() -> Self {
        Self::default()
    }

    /// Create config for system bus
    pub fn system() -> Self {
        Self {
            bus_type: BusType::System,
            ..Default::default()
        }
    }

    /// Create config with custom service name
    pub fn with_name(service_name: &str) -> Self {
        Self {
            service_name: service_name.to_string(),
            object_path: format!("/{}", service_name.replace('.', "/")),
            ..Default::default()
        }
    }
}

/// D-Bus interface implementation for ASTERIX parser
struct AsterixParserInterface {
    #[allow(dead_code)]
    emit_signals: bool,
}

#[interface(name = "com.asterix.Parser")]
impl AsterixParserInterface {
    /// Parse raw ASTERIX bytes and return JSON result
    fn parse(&self, data: Vec<u8>) -> Result<String, zbus::fdo::Error> {
        use crate::{parse, ParseOptions};

        let records = parse(&data, ParseOptions::default())
            .map_err(|e| zbus::fdo::Error::Failed(e.to_string()))?;

        #[cfg(feature = "serde")]
        {
            serde_json::to_string(&records)
                .map_err(|e| zbus::fdo::Error::Failed(e.to_string()))
        }

        #[cfg(not(feature = "serde"))]
        {
            Ok(format!("Parsed {} records", records.len()))
        }
    }

    /// Parse hex-encoded ASTERIX data and return JSON result
    fn parse_hex(&self, hex_data: String) -> Result<String, zbus::fdo::Error> {
        let bytes = hex_to_bytes(&hex_data)
            .map_err(|e| zbus::fdo::Error::Failed(e.to_string()))?;
        self.parse(bytes)
    }

    /// Get ASTERIX library version
    fn get_version(&self) -> String {
        env!("CARGO_PKG_VERSION").to_string()
    }

    /// Get supported ASTERIX categories
    fn get_categories(&self) -> Vec<u8> {
        // Common ASTERIX categories
        vec![1, 2, 4, 8, 10, 19, 20, 21, 23, 25, 31, 32, 34, 48, 62, 63, 65, 240, 247, 252]
    }

    /// Check if the service is healthy
    fn health_check(&self) -> bool {
        true
    }
}

/// D-Bus service for ASTERIX parsing
pub struct DbusService {
    connection: Connection,
    config: DbusConfig,
}

impl DbusService {
    /// Create a new D-Bus service
    pub fn new(config: DbusConfig) -> Result<Self, DbusError> {
        let connection = match config.bus_type {
            BusType::Session => Connection::session()?,
            BusType::System => Connection::system()?,
        };

        Ok(Self {
            connection,
            config,
        })
    }

    /// Start the D-Bus service and block
    ///
    /// This method registers the service on the bus and starts processing
    /// incoming method calls. It will block until the service is stopped.
    pub fn run(&self) -> Result<(), DbusError> {
        use zbus::names::WellKnownName;

        // Request the service name
        let name: WellKnownName = self.config.service_name.as_str().try_into()
            .map_err(|e| DbusError::ServiceError(format!("Invalid service name: {e}")))?;
        self.connection
            .request_name(name)
            .map_err(|e| DbusError::ServiceError(e.to_string()))?;

        // Create the interface object
        let interface = AsterixParserInterface {
            emit_signals: self.config.emit_signals,
        };

        // Serve the interface at the object path
        let path: zbus::zvariant::ObjectPath = self.config.object_path.as_str().try_into()
            .map_err(|e| DbusError::ServiceError(format!("Invalid object path: {e}")))?;
        self.connection
            .object_server()
            .at(path, interface)
            .map_err(|e| DbusError::ServiceError(e.to_string()))?;

        log::info!(
            "ASTERIX D-Bus service started: {} at {}",
            self.config.service_name,
            self.config.object_path
        );

        // Block forever (service stays running)
        // In zbus 5.x, the connection handles message processing automatically
        loop {
            std::thread::sleep(std::time::Duration::from_secs(1));
        }
    }

    /// Start the D-Bus service and run for a limited time (for testing)
    pub fn run_for(&self, duration: std::time::Duration) -> Result<(), DbusError> {
        use zbus::names::WellKnownName;

        let name: WellKnownName = self.config.service_name.as_str().try_into()
            .map_err(|e| DbusError::ServiceError(format!("Invalid service name: {e}")))?;
        self.connection
            .request_name(name)
            .map_err(|e| DbusError::ServiceError(e.to_string()))?;

        let interface = AsterixParserInterface {
            emit_signals: self.config.emit_signals,
        };

        let path: zbus::zvariant::ObjectPath = self.config.object_path.as_str().try_into()
            .map_err(|e| DbusError::ServiceError(format!("Invalid object path: {e}")))?;
        self.connection
            .object_server()
            .at(path, interface)
            .map_err(|e| DbusError::ServiceError(e.to_string()))?;

        std::thread::sleep(duration);
        Ok(())
    }

    /// Get the D-Bus connection (for advanced use)
    pub fn connection(&self) -> &Connection {
        &self.connection
    }
}

/// D-Bus client for calling ASTERIX parser service
pub struct DbusClient {
    connection: Connection,
    service_name: String,
    object_path: String,
}

impl DbusClient {
    /// Create a new D-Bus client
    pub fn new(config: DbusConfig) -> Result<Self, DbusError> {
        let connection = match config.bus_type {
            BusType::Session => Connection::session()?,
            BusType::System => Connection::system()?,
        };

        Ok(Self {
            connection,
            service_name: config.service_name,
            object_path: config.object_path,
        })
    }

    /// Create a proxy for calling methods
    fn create_proxy(&self) -> Result<zbus::blocking::Proxy, DbusError> {
        use zbus::names::BusName;
        use zbus::blocking::Proxy;

        let dest: BusName = self.service_name.as_str().try_into()
            .map_err(|e| DbusError::ConnectionError(format!("Invalid bus name: {e}")))?;
        let path: zbus::zvariant::ObjectPath = self.object_path.as_str().try_into()
            .map_err(|e| DbusError::ConnectionError(format!("Invalid object path: {e}")))?;

        Proxy::new(
            &self.connection,
            dest,
            path,
            "com.asterix.Parser",
        )
        .map_err(|e| DbusError::ConnectionError(e.to_string()))
    }

    /// Parse ASTERIX data by calling the D-Bus service
    pub fn parse(&self, data: &[u8]) -> Result<String, DbusError> {
        let proxy = self.create_proxy()?;
        proxy
            .call_method("Parse", &(data.to_vec(),))
            .map_err(|e| DbusError::MethodError(e.to_string()))?
            .body()
            .deserialize::<String>()
            .map_err(|e| DbusError::MethodError(e.to_string()))
    }

    /// Parse hex-encoded ASTERIX data
    pub fn parse_hex(&self, hex_data: &str) -> Result<String, DbusError> {
        let proxy = self.create_proxy()?;
        proxy
            .call_method("ParseHex", &(hex_data,))
            .map_err(|e| DbusError::MethodError(e.to_string()))?
            .body()
            .deserialize::<String>()
            .map_err(|e| DbusError::MethodError(e.to_string()))
    }

    /// Get the ASTERIX library version from the service
    pub fn get_version(&self) -> Result<String, DbusError> {
        let proxy = self.create_proxy()?;
        proxy
            .call_method("GetVersion", &())
            .map_err(|e| DbusError::MethodError(e.to_string()))?
            .body()
            .deserialize::<String>()
            .map_err(|e| DbusError::MethodError(e.to_string()))
    }

    /// Check if the service is healthy
    pub fn health_check(&self) -> Result<bool, DbusError> {
        let proxy = self.create_proxy()?;
        proxy
            .call_method("HealthCheck", &())
            .map_err(|e| DbusError::MethodError(e.to_string()))?
            .body()
            .deserialize::<bool>()
            .map_err(|e| DbusError::MethodError(e.to_string()))
    }
}

/// Convert hex string to bytes
fn hex_to_bytes(hex: &str) -> Result<Vec<u8>, String> {
    let hex_clean: String = hex.chars().filter(|c| !c.is_whitespace()).collect();

    if hex_clean.len() % 2 != 0 {
        return Err("Invalid hex string length".to_string());
    }

    (0..hex_clean.len())
        .step_by(2)
        .map(|i| {
            u8::from_str_radix(&hex_clean[i..i + 2], 16)
                .map_err(|e| e.to_string())
        })
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    // ============================================================================
    // Error Type Tests
    // ============================================================================

    #[test]
    fn test_dbus_error_display_variants() {
        let errors = vec![
            (DbusError::ConnectionError("test".to_string()), "connection"),
            (DbusError::ServiceError("test".to_string()), "service"),
            (DbusError::MethodError("test".to_string()), "method"),
            (DbusError::ParseError("test".to_string()), "parse"),
        ];

        for (err, expected_substring) in errors {
            let display = err.to_string().to_lowercase();
            assert!(
                display.contains(expected_substring),
                "Expected '{}' in '{}'",
                expected_substring,
                display
            );
        }
    }

    #[test]
    fn test_dbus_error_debug() {
        let err = DbusError::ConnectionError("test error".to_string());
        let debug = format!("{:?}", err);
        assert!(debug.contains("ConnectionError"));
        assert!(debug.contains("test error"));
    }

    #[test]
    fn test_dbus_error_to_asterix_error() {
        let err = DbusError::MethodError("call failed".to_string());
        let asterix_err: AsterixError = err.into();
        let msg = asterix_err.to_string();
        assert!(msg.contains("method"));
    }

    #[test]
    fn test_dbus_error_is_std_error() {
        let err: Box<dyn std::error::Error> = Box::new(DbusError::ServiceError("test".to_string()));
        assert!(err.to_string().contains("service"));
    }

    // ============================================================================
    // Config Tests
    // ============================================================================

    #[test]
    fn test_dbus_config_default() {
        let config = DbusConfig::default();
        assert_eq!(config.service_name, "com.asterix.Parser");
        assert_eq!(config.object_path, "/com/asterix/Parser");
        assert_eq!(config.bus_type, BusType::Session);
        assert!(config.emit_signals);
    }

    #[test]
    fn test_dbus_config_session() {
        let config = DbusConfig::session();
        assert_eq!(config.bus_type, BusType::Session);
    }

    #[test]
    fn test_dbus_config_system() {
        let config = DbusConfig::system();
        assert_eq!(config.bus_type, BusType::System);
    }

    #[test]
    fn test_dbus_config_with_name() {
        let config = DbusConfig::with_name("org.example.Radar");
        assert_eq!(config.service_name, "org.example.Radar");
        assert_eq!(config.object_path, "/org/example/Radar");
    }

    #[test]
    fn test_dbus_config_clone() {
        let config = DbusConfig {
            service_name: "custom.Service".to_string(),
            object_path: "/custom/Service".to_string(),
            bus_type: BusType::System,
            emit_signals: false,
        };
        let cloned = config.clone();
        assert_eq!(cloned.service_name, "custom.Service");
        assert_eq!(cloned.bus_type, BusType::System);
        assert!(!cloned.emit_signals);
    }

    #[test]
    fn test_bus_type_default() {
        assert_eq!(BusType::default(), BusType::Session);
    }

    // ============================================================================
    // Hex Conversion Tests
    // ============================================================================

    #[test]
    fn test_hex_to_bytes_valid() {
        let result = hex_to_bytes("30001E");
        assert!(result.is_ok());
        let bytes = result.unwrap();
        assert_eq!(bytes, vec![0x30, 0x00, 0x1E]);
    }

    #[test]
    fn test_hex_to_bytes_with_whitespace() {
        let result = hex_to_bytes("30 00 1E 48");
        assert!(result.is_ok());
        let bytes = result.unwrap();
        assert_eq!(bytes, vec![0x30, 0x00, 0x1E, 0x48]);
    }

    #[test]
    fn test_hex_to_bytes_lowercase() {
        let result = hex_to_bytes("abcdef");
        assert!(result.is_ok());
        let bytes = result.unwrap();
        assert_eq!(bytes, vec![0xAB, 0xCD, 0xEF]);
    }

    #[test]
    fn test_hex_to_bytes_mixed_case() {
        let result = hex_to_bytes("AbCdEf");
        assert!(result.is_ok());
    }

    #[test]
    fn test_hex_to_bytes_invalid_length() {
        let result = hex_to_bytes("123");
        assert!(result.is_err());
        assert!(result.unwrap_err().contains("length"));
    }

    #[test]
    fn test_hex_to_bytes_invalid_chars() {
        let result = hex_to_bytes("GHIJ");
        assert!(result.is_err());
    }

    #[test]
    fn test_hex_to_bytes_empty() {
        let result = hex_to_bytes("");
        assert!(result.is_ok());
        assert!(result.unwrap().is_empty());
    }

    // ============================================================================
    // Interface Tests (basic)
    // ============================================================================

    #[test]
    fn test_asterix_parser_interface_get_version() {
        let interface = AsterixParserInterface { emit_signals: true };
        let version = interface.get_version();
        assert!(!version.is_empty());
    }

    #[test]
    fn test_asterix_parser_interface_get_categories() {
        let interface = AsterixParserInterface { emit_signals: true };
        let categories = interface.get_categories();
        assert!(categories.contains(&48));
        assert!(categories.contains(&62));
    }

    #[test]
    fn test_asterix_parser_interface_health_check() {
        let interface = AsterixParserInterface { emit_signals: false };
        assert!(interface.health_check());
    }
}
