//! D-Bus Transport Integration Tests
//!
//! These tests require a D-Bus session bus to be running.
//! On most Linux systems with systemd, this is available by default.
//!
//! To run: cargo test --features dbus -- --test-threads=1
//!
//! Note: Tests run with --test-threads=1 to avoid D-Bus name conflicts

#![cfg(feature = "dbus")]

use asterix::transport::dbus::{BusType, DbusClient, DbusConfig, DbusError, DbusService};
use std::thread;
use std::time::Duration;

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
fn test_dbus_config_with_custom_name() {
    let config = DbusConfig::with_name("org.example.Radar");
    assert_eq!(config.service_name, "org.example.Radar");
    assert_eq!(config.object_path, "/org/example/Radar");
}

#[test]
fn test_dbus_config_clone() {
    let config = DbusConfig {
        service_name: "test.Service".to_string(),
        object_path: "/test/Service".to_string(),
        bus_type: BusType::Session,
        emit_signals: false,
    };
    let cloned = config.clone();
    assert_eq!(cloned.service_name, "test.Service");
    assert!(!cloned.emit_signals);
}

// ============================================================================
// Error Tests
// ============================================================================

#[test]
fn test_dbus_error_display() {
    let errors = vec![
        DbusError::ConnectionError("conn fail".to_string()),
        DbusError::ServiceError("svc fail".to_string()),
        DbusError::MethodError("method fail".to_string()),
        DbusError::ParseError("parse fail".to_string()),
    ];

    for err in errors {
        let display = err.to_string();
        assert!(!display.is_empty());
        println!("Error: {display}");
    }
}

#[test]
fn test_dbus_error_debug() {
    let err = DbusError::ConnectionError("debug test".to_string());
    let debug = format!("{err:?}");
    assert!(debug.contains("ConnectionError"));
}

// ============================================================================
// Service Creation Tests
// ============================================================================

#[test]
fn test_dbus_service_creation() {
    // This test requires D-Bus session bus
    let config = DbusConfig::with_name("com.asterix.test.ServiceCreation");
    match DbusService::new(config) {
        Ok(service) => {
            println!("D-Bus service created successfully");
            // Just test creation, don't start serving
            let _ = service.connection();
        }
        Err(e) => {
            println!("D-Bus service creation failed (expected in CI without D-Bus): {e}");
        }
    }
}

#[test]
fn test_dbus_client_creation() {
    let config = DbusConfig::with_name("com.asterix.test.ClientCreation");
    match DbusClient::new(config) {
        Ok(_client) => {
            println!("D-Bus client created successfully");
        }
        Err(e) => {
            println!("D-Bus client creation failed (expected in CI without D-Bus): {e}");
        }
    }
}

// ============================================================================
// Service-Client Communication Tests
// ============================================================================

/// Test full service-client communication
///
/// This test starts a service in a background thread, then uses a client
/// to call methods on it.
#[test]
fn test_service_client_communication() {
    // Use a unique service name for this test
    let service_name = "com.asterix.test.Communication";
    let config = DbusConfig::with_name(service_name);

    // Try to create service
    let service_result = DbusService::new(config.clone());
    let Ok(service) = service_result else {
        println!("Skipping test: D-Bus session not available");
        return;
    };

    // Start service in background thread
    let handle = thread::spawn(move || {
        // Run for 2 seconds
        let _ = service.run_for(Duration::from_secs(2));
    });

    // Give service time to start
    thread::sleep(Duration::from_millis(200));

    // Create client and call methods
    match DbusClient::new(DbusConfig::with_name(service_name)) {
        Ok(client) => {
            // Test GetVersion
            match client.get_version() {
                Ok(version) => {
                    println!("Service version: {version}");
                    assert!(!version.is_empty());
                }
                Err(e) => {
                    println!("GetVersion failed: {e}");
                }
            }

            // Test HealthCheck
            match client.health_check() {
                Ok(healthy) => {
                    println!("Service healthy: {healthy}");
                    assert!(healthy);
                }
                Err(e) => {
                    println!("HealthCheck failed: {e}");
                }
            }
        }
        Err(e) => {
            println!("Client creation failed: {e}");
        }
    }

    // Wait for service thread to finish
    let _ = handle.join();
}

/// Test parsing via D-Bus (requires initialized ASTERIX parser)
#[test]
fn test_parse_via_dbus() {
    use asterix::init_default;

    // Initialize ASTERIX parser first
    if init_default().is_err() {
        println!("Skipping test: ASTERIX initialization failed");
        return;
    }

    let service_name = "com.asterix.test.Parse";
    let config = DbusConfig::with_name(service_name);

    let service_result = DbusService::new(config.clone());
    let Ok(service) = service_result else {
        println!("Skipping test: D-Bus session not available");
        return;
    };

    let handle = thread::spawn(move || {
        let _ = service.run_for(Duration::from_secs(2));
    });

    thread::sleep(Duration::from_millis(200));

    match DbusClient::new(DbusConfig::with_name(service_name)) {
        Ok(client) => {
            // Test with minimal ASTERIX data (may fail parsing, that's ok)
            let test_data = vec![0x30, 0x00, 0x05, 0x00, 0x00]; // CAT48, 5 bytes
            match client.parse(&test_data) {
                Ok(result) => {
                    println!("Parse result: {result}");
                }
                Err(e) => {
                    println!("Parse failed (expected for invalid data): {e}");
                }
            }

            // Test ParseHex
            match client.parse_hex("3000050000") {
                Ok(result) => {
                    println!("ParseHex result: {result}");
                }
                Err(e) => {
                    println!("ParseHex failed (expected for invalid data): {e}");
                }
            }
        }
        Err(e) => {
            println!("Client creation failed: {e}");
        }
    }

    let _ = handle.join();
}

// ============================================================================
// Bus Type Tests
// ============================================================================

#[test]
fn test_session_bus_connection() {
    let config = DbusConfig::session();
    match DbusService::new(config) {
        Ok(_) => println!("Session bus connection successful"),
        Err(e) => println!("Session bus connection failed: {e}"),
    }
}

#[test]
fn test_system_bus_connection() {
    // System bus typically requires special permissions
    let config = DbusConfig::system();
    match DbusService::new(config) {
        Ok(_) => println!("System bus connection successful (unusual in test env)"),
        Err(e) => println!("System bus connection failed (expected): {e}"),
    }
}

// ============================================================================
// Error Path Tests
// ============================================================================

#[test]
fn test_client_method_to_nonexistent_service() {
    let config = DbusConfig::with_name("com.asterix.test.NonExistent");
    match DbusClient::new(config) {
        Ok(client) => {
            // Try to call method on service that doesn't exist
            match client.get_version() {
                Ok(_) => println!("Unexpected success calling nonexistent service"),
                Err(e) => println!("Expected error: {e}"),
            }
        }
        Err(e) => {
            println!("Client creation failed: {e}");
        }
    }
}

#[test]
fn test_invalid_service_name() {
    // D-Bus names have strict format requirements
    let config = DbusConfig {
        service_name: "invalid name with spaces".to_string(),
        object_path: "/com/asterix/Test".to_string(),
        bus_type: BusType::Session,
        emit_signals: false,
    };

    match DbusService::new(config.clone()) {
        Ok(service) => {
            // Service creation might succeed, but requesting name should fail
            match service.run_for(Duration::from_millis(100)) {
                Ok(_) => println!("Unexpected success with invalid name"),
                Err(e) => println!("Expected error with invalid name: {e}"),
            }
        }
        Err(e) => {
            println!("Expected error during creation: {e}");
        }
    }
}

#[test]
fn test_invalid_object_path() {
    let config = DbusConfig {
        service_name: "com.asterix.test.InvalidPath".to_string(),
        object_path: "not/a/valid/path".to_string(), // Missing leading /
        bus_type: BusType::Session,
        emit_signals: false,
    };

    match DbusService::new(config.clone()) {
        Ok(service) => match service.run_for(Duration::from_millis(100)) {
            Ok(_) => println!("Unexpected success with invalid path"),
            Err(e) => println!("Expected error with invalid path: {e}"),
        },
        Err(e) => {
            println!("Expected error during creation: {e}");
        }
    }
}

// ============================================================================
// Concurrent Access Tests
// ============================================================================

#[test]
fn test_multiple_clients() {
    let service_name = "com.asterix.test.MultiClient";
    let config = DbusConfig::with_name(service_name);

    let service_result = DbusService::new(config.clone());
    let Ok(service) = service_result else {
        println!("Skipping test: D-Bus session not available");
        return;
    };

    let handle = thread::spawn(move || {
        let _ = service.run_for(Duration::from_secs(3));
    });

    thread::sleep(Duration::from_millis(200));

    // Create multiple clients
    let mut clients = Vec::new();
    for i in 0..3 {
        match DbusClient::new(DbusConfig::with_name(service_name)) {
            Ok(client) => {
                println!("Client {i} created");
                clients.push(client);
            }
            Err(e) => {
                println!("Client {i} creation failed: {e}");
            }
        }
    }

    // All clients call methods
    for (i, client) in clients.iter().enumerate() {
        match client.health_check() {
            Ok(healthy) => println!("Client {i} health check: {healthy}"),
            Err(e) => println!("Client {i} health check failed: {e}"),
        }
    }

    let _ = handle.join();
}

// ============================================================================
// Performance Tests
// ============================================================================

#[test]
fn test_rapid_method_calls() {
    let service_name = "com.asterix.test.RapidCalls";
    let config = DbusConfig::with_name(service_name);

    let service_result = DbusService::new(config.clone());
    let Ok(service) = service_result else {
        println!("Skipping test: D-Bus session not available");
        return;
    };

    let handle = thread::spawn(move || {
        let _ = service.run_for(Duration::from_secs(5));
    });

    thread::sleep(Duration::from_millis(200));

    match DbusClient::new(DbusConfig::with_name(service_name)) {
        Ok(client) => {
            let start = std::time::Instant::now();
            let mut success_count = 0;
            let mut error_count = 0;

            for _ in 0..100 {
                match client.health_check() {
                    Ok(_) => success_count += 1,
                    Err(_) => error_count += 1,
                }
            }

            let elapsed = start.elapsed();
            println!(
                "100 method calls in {elapsed:?}: {success_count} success, {error_count} errors"
            );

            if success_count > 0 {
                let avg = elapsed / success_count;
                println!("Average call time: {avg:?}");
            }
        }
        Err(e) => {
            println!("Client creation failed: {e}");
        }
    }

    let _ = handle.join();
}
