//! ASTERIX D-Bus Service Example
//!
//! Runs the ASTERIX parser as a D-Bus service that other applications
//! can call to parse ASTERIX data.
//!
//! # Running
//!
//! ```bash
//! cargo run --features dbus --example dbus_service
//! ```
//!
//! # Testing
//!
//! From another terminal:
//! ```bash
//! # Get version
//! dbus-send --session --print-reply \
//!     --dest=com.asterix.Parser \
//!     /com/asterix/Parser \
//!     com.asterix.Parser.GetVersion
//!
//! # Health check
//! dbus-send --session --print-reply \
//!     --dest=com.asterix.Parser \
//!     /com/asterix/Parser \
//!     com.asterix.Parser.HealthCheck
//! ```

#[cfg(feature = "dbus")]
use asterix::init_default;
#[cfg(feature = "dbus")]
use asterix::transport::dbus::{DbusConfig, DbusService};

#[cfg(feature = "dbus")]
fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize logging
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();

    // Initialize ASTERIX parser with default configuration
    log::info!("Initializing ASTERIX parser...");
    init_default()?;

    // Create D-Bus service with default configuration
    let config = DbusConfig::default();
    log::info!("Creating D-Bus service...");
    log::info!("  Service name: {}", config.service_name);
    log::info!("  Object path: {}", config.object_path);
    log::info!("  Bus type: {:?}", config.bus_type);

    let service = DbusService::new(config)?;

    log::info!("ASTERIX D-Bus service started successfully!");
    log::info!("Press Ctrl+C to stop the service.");
    log::info!("");
    log::info!("Test with:");
    log::info!("  dbus-send --session --print-reply \\");
    log::info!("      --dest=com.asterix.Parser \\");
    log::info!("      /com/asterix/Parser \\");
    log::info!("      com.asterix.Parser.GetVersion");

    // Run service (blocks forever)
    service.run()?;

    Ok(())
}

#[cfg(not(feature = "dbus"))]
fn main() {
    eprintln!("This example requires the 'dbus' feature.");
    eprintln!("Run with: cargo run --features dbus --example dbus_service");
    std::process::exit(1);
}
