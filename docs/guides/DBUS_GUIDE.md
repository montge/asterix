# D-Bus Integration Guide

This guide covers using D-Bus with ASTERIX for Linux inter-process communication (IPC) and system service integration.

## Overview

[D-Bus](https://www.freedesktop.org/wiki/Software/dbus/) is the standard IPC mechanism on Linux, used by systemd, desktop environments, and countless system services. The ASTERIX D-Bus integration exposes the parser as a system service that other applications can call.

### Key Benefits

- **System Integration** - Run ASTERIX as a systemd service
- **IPC** - Local inter-process communication without network overhead
- **GPL Separation** - Process boundary enables clean license separation
- **Auto-activation** - D-Bus can start the service on demand
- **Language Agnostic** - Any language with D-Bus bindings can use the service

## Building with D-Bus Support

### Prerequisites

- Linux system with D-Bus (standard on most distributions)
- Rust 1.77+ with Cargo
- C++17 compatible compiler
- libexpat-dev

### Build Configuration

```bash
cd asterix-rs

# Build with D-Bus feature enabled
cargo build --features dbus

# Run tests (requires D-Bus session)
cargo test --features dbus -- --test-threads=1

# Build release version
cargo build --release --features dbus
```

### Cargo.toml Configuration

```toml
[dependencies]
asterix = { version = "0.1", features = ["dbus"] }
```

## D-Bus Interface

The ASTERIX service exposes the `com.asterix.Parser` interface:

```xml
<interface name="com.asterix.Parser">
  <!-- Parse raw ASTERIX bytes, returns JSON -->
  <method name="Parse">
    <arg type="ay" name="data" direction="in"/>
    <arg type="s" name="result" direction="out"/>
  </method>
  
  <!-- Parse hex-encoded ASTERIX data, returns JSON -->
  <method name="ParseHex">
    <arg type="s" name="hex_data" direction="in"/>
    <arg type="s" name="result" direction="out"/>
  </method>
  
  <!-- Get library version -->
  <method name="GetVersion">
    <arg type="s" name="version" direction="out"/>
  </method>
  
  <!-- Get supported ASTERIX categories -->
  <method name="GetCategories">
    <arg type="ay" name="categories" direction="out"/>
  </method>
  
  <!-- Health check -->
  <method name="HealthCheck">
    <arg type="b" name="healthy" direction="out"/>
  </method>
</interface>
```

### Default Service Name and Path

| Property | Value |
|----------|-------|
| Service Name | `com.asterix.Parser` |
| Object Path | `/com/asterix/Parser` |
| Interface | `com.asterix.Parser` |

## Usage Examples

### Running the D-Bus Service (Rust)

```rust
use asterix::transport::dbus::{DbusService, DbusConfig};
use asterix::init_default;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize ASTERIX parser
    init_default()?;
    
    // Create D-Bus service with default config
    let config = DbusConfig::default();
    let service = DbusService::new(config)?;
    
    println!("ASTERIX D-Bus service starting...");
    println!("Service: com.asterix.Parser");
    println!("Path: /com/asterix/Parser");
    
    // Run service (blocks forever)
    service.run()?;
    
    Ok(())
}
```

### Using the D-Bus Client (Rust)

```rust
use asterix::transport::dbus::{DbusClient, DbusConfig};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Create D-Bus client
    let client = DbusClient::new(DbusConfig::default())?;
    
    // Get version
    let version = client.get_version()?;
    println!("ASTERIX version: {version}");
    
    // Parse ASTERIX data
    let asterix_data = vec![0x30, 0x00, 0x1E, /* ... */];
    let result = client.parse(&asterix_data)?;
    println!("Parse result: {result}");
    
    // Parse hex-encoded data
    let hex_result = client.parse_hex("30001E...")?;
    println!("Hex parse result: {hex_result}");
    
    Ok(())
}
```

### Calling from Python

```python
#!/usr/bin/env python3
"""Example: Call ASTERIX D-Bus service from Python"""

import dbus
import json

# Connect to session bus
bus = dbus.SessionBus()

# Get ASTERIX service
asterix = bus.get_object('com.asterix.Parser', '/com/asterix/Parser')
parser = dbus.Interface(asterix, 'com.asterix.Parser')

# Get version
version = parser.GetVersion()
print(f"ASTERIX version: {version}")

# Health check
healthy = parser.HealthCheck()
print(f"Service healthy: {healthy}")

# Parse ASTERIX data
asterix_bytes = bytes([0x30, 0x00, 0x1E])  # Example CAT48 data
result_json = parser.Parse(dbus.ByteArray(asterix_bytes))
result = json.loads(result_json)
print(f"Parsed {len(result)} records")

# Parse hex-encoded data
hex_result = parser.ParseHex("30001E...")
print(f"Hex parse result: {hex_result}")
```

### Calling from C

```c
/* Example: Call ASTERIX D-Bus service from C using GDBus */

#include <gio/gio.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    GError *error = NULL;
    GDBusConnection *connection;
    GVariant *result;
    
    // Connect to session bus
    connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if (error) {
        fprintf(stderr, "Failed to connect: %s\n", error->message);
        return 1;
    }
    
    // Call GetVersion
    result = g_dbus_connection_call_sync(
        connection,
        "com.asterix.Parser",           // service
        "/com/asterix/Parser",           // object path
        "com.asterix.Parser",            // interface
        "GetVersion",                    // method
        NULL,                            // parameters
        G_VARIANT_TYPE("(s)"),           // return type
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error
    );
    
    if (error) {
        fprintf(stderr, "Method call failed: %s\n", error->message);
        return 1;
    }
    
    const gchar *version;
    g_variant_get(result, "(&s)", &version);
    printf("ASTERIX version: %s\n", version);
    
    g_variant_unref(result);
    g_object_unref(connection);
    return 0;
}
```

### Using dbus-send (Command Line)

```bash
# Get version
dbus-send --session --print-reply \
    --dest=com.asterix.Parser \
    /com/asterix/Parser \
    com.asterix.Parser.GetVersion

# Health check
dbus-send --session --print-reply \
    --dest=com.asterix.Parser \
    /com/asterix/Parser \
    com.asterix.Parser.HealthCheck

# Get supported categories
dbus-send --session --print-reply \
    --dest=com.asterix.Parser \
    /com/asterix/Parser \
    com.asterix.Parser.GetCategories
```

## Configuration Options

### DbusConfig Fields

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `service_name` | String | `com.asterix.Parser` | D-Bus well-known name |
| `object_path` | String | `/com/asterix/Parser` | D-Bus object path |
| `bus_type` | BusType | `Session` | Session or System bus |
| `emit_signals` | bool | `true` | Whether to emit D-Bus signals |

### Bus Types

- **Session Bus**: Per-user bus, no special permissions required
- **System Bus**: System-wide bus, requires D-Bus policy configuration

### Using System Bus

To use the system bus, you need to create a D-Bus policy file:

```xml
<!-- /etc/dbus-1/system.d/com.asterix.Parser.conf -->
<!DOCTYPE busconfig PUBLIC
 "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <!-- Allow asterix user to own the service -->
  <policy user="asterix">
    <allow own="com.asterix.Parser"/>
  </policy>
  
  <!-- Allow anyone to call methods -->
  <policy context="default">
    <allow send_destination="com.asterix.Parser"/>
    <allow receive_sender="com.asterix.Parser"/>
  </policy>
</busconfig>
```

## systemd Integration

### Service Unit File

Create `/etc/systemd/system/asterix-parser.service`:

```ini
[Unit]
Description=ASTERIX Parser D-Bus Service
Documentation=https://github.com/montge/asterix
After=dbus.service
Requires=dbus.service

[Service]
Type=dbus
BusName=com.asterix.Parser
ExecStart=/usr/local/bin/asterix-dbus-service
Restart=on-failure
RestartSec=5

# Security hardening
User=asterix
Group=asterix
NoNewPrivileges=true
ProtectSystem=strict
ProtectHome=true
PrivateTmp=true

[Install]
WantedBy=multi-user.target
```

### D-Bus Service File for Auto-Activation

Create `/usr/share/dbus-1/services/com.asterix.Parser.service`:

```ini
[D-BUS Service]
Name=com.asterix.Parser
Exec=/usr/local/bin/asterix-dbus-service
User=asterix
```

### Managing the Service

```bash
# Enable and start the service
sudo systemctl enable asterix-parser
sudo systemctl start asterix-parser

# Check status
sudo systemctl status asterix-parser

# View logs
sudo journalctl -u asterix-parser -f
```

## Error Handling

### Common Errors

| Error | Cause | Solution |
|-------|-------|----------|
| `ConnectionError` | D-Bus not available | Ensure D-Bus daemon is running |
| `ServiceError` | Name/path registration failed | Check D-Bus policy permissions |
| `MethodError` | Method call failed | Check method arguments and types |
| `ParseError` | ASTERIX parsing failed | Verify input data is valid ASTERIX |

### Error Handling Example

```rust
use asterix::transport::dbus::{DbusClient, DbusConfig, DbusError};

fn call_service() -> Result<(), DbusError> {
    let client = DbusClient::new(DbusConfig::default())?;
    
    match client.get_version() {
        Ok(version) => println!("Version: {version}"),
        Err(DbusError::ConnectionError(e)) => {
            eprintln!("D-Bus not available: {e}");
            eprintln!("Is the D-Bus daemon running?");
        }
        Err(DbusError::MethodError(e)) => {
            eprintln!("Service unavailable: {e}");
            eprintln!("Is asterix-parser service running?");
        }
        Err(e) => eprintln!("Error: {e}"),
    }
    
    Ok(())
}
```

## Performance Considerations

### D-Bus vs Direct Library Calls

| Aspect | D-Bus Service | Direct Library |
|--------|---------------|----------------|
| Latency | ~0.1-1ms per call | ~0.001-0.01ms |
| Throughput | ~1000-10000 calls/sec | ~100000+ calls/sec |
| Memory | Separate process | Same process |
| Isolation | Full process isolation | None |

### When to Use D-Bus

✅ **Use D-Bus when:**
- Multiple applications need to parse ASTERIX
- License separation is required (GPL boundary)
- System service integration is needed
- Different programming languages need access

❌ **Don't use D-Bus when:**
- Single application with high throughput needs
- Minimal latency is critical
- Simple embedded deployment

## Troubleshooting

### Check D-Bus Daemon

```bash
# Check if D-Bus is running
systemctl status dbus

# Check session bus address
echo $DBUS_SESSION_BUS_ADDRESS
```

### Debug D-Bus Communication

```bash
# Monitor D-Bus traffic
dbus-monitor --session "interface='com.asterix.Parser'"

# List services
dbus-send --session --dest=org.freedesktop.DBus \
    --type=method_call --print-reply \
    /org/freedesktop/DBus org.freedesktop.DBus.ListNames
```

### Common Issues

1. **"Connection refused"**: D-Bus daemon not running
   ```bash
   sudo systemctl start dbus
   ```

2. **"Name not owned"**: ASTERIX service not running
   ```bash
   systemctl start asterix-parser
   ```

3. **"Permission denied"** (system bus): Missing policy file
   - Create `/etc/dbus-1/system.d/com.asterix.Parser.conf`

## See Also

- [D-Bus Specification](https://dbus.freedesktop.org/doc/dbus-specification.html)
- [systemd Service Units](https://www.freedesktop.org/software/systemd/man/systemd.service.html)
- [zbus Documentation](https://docs.rs/zbus)

