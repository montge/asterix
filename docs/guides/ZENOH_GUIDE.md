# Zenoh Integration Guide

This guide covers using Zenoh with ASTERIX for edge-to-cloud pub/sub data distribution in ATM/ATC environments.

## Overview

[Zenoh](https://zenoh.io/) is a zero-overhead pub/sub/query protocol designed for edge-to-cloud data flows. It provides:

- **Zero-copy data paths** - Minimal overhead for high-throughput scenarios
- **Automatic discovery** - Zero-configuration networking via multicast
- **Edge-to-cloud** - Seamless data flow across network boundaries (edge, fog, cloud)
- **Multi-protocol** - TCP, UDP, QUIC, WebSocket, and shared memory
- **Key expressions** - Hierarchical topic naming with powerful wildcards

Zenoh is particularly suited for:
- Distributed surveillance systems spanning multiple sites
- Edge processing of radar data with cloud aggregation
- Multi-sensor data fusion across network boundaries
- ATM applications requiring flexible network topologies

## Building with Zenoh Support (Rust)

The Zenoh integration is implemented in the Rust `asterix-rs` crate, leveraging Zenoh's native Rust implementation for optimal performance.

### Prerequisites

- Rust 1.70+ with Cargo
- C++17 compatible compiler (for ASTERIX core)
- libexpat-devel (for XML parsing)

### Build Configuration

```bash
cd asterix-rs

# Build with Zenoh feature enabled
cargo build --features zenoh

# Run tests
cargo test --features zenoh

# Build release version
cargo build --release --features zenoh
```

### Cargo.toml Configuration

Add the Zenoh feature to your `Cargo.toml`:

```toml
[dependencies]
asterix = { version = "0.1", features = ["zenoh"] }
```

## API Overview

### Key Components

| Component | Description |
|-----------|-------------|
| `ZenohPublisher` | Publishes ASTERIX records to Zenoh network |
| `ZenohSubscriber` | Subscribes to ASTERIX data from Zenoh network |
| `ZenohConfig` | Configuration for Zenoh sessions |
| `AsterixSample` | Received ASTERIX sample with metadata |

### Key Expression Convention

ASTERIX data is published using hierarchical key expressions:

```
asterix/{category}/{sac}/{sic}
```

| Component | Description | Example |
|-----------|-------------|---------|
| `category` | ASTERIX category number | 48, 62, 65 |
| `sac` | System Area Code | 1, 10, 255 |
| `sic` | System Identification Code | 2, 20, 128 |

**Examples:**
- `asterix/48` - All CAT048 data
- `asterix/62/1/2` - CAT062 from SAC=1, SIC=2
- `asterix/**` - All ASTERIX data (wildcard)
- `asterix/48/**` - All CAT048 from any source
- `asterix/*/1/*` - All categories from SAC=1

## Usage Examples

### Publishing ASTERIX Data

```rust
use asterix::{parse, ParseOptions, init_default};
use asterix::transport::zenoh::{ZenohPublisher, ZenohConfig};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize ASTERIX parser
    init_default()?;

    // Read ASTERIX data
    let data = std::fs::read("recording.asterix")?;
    let records = parse(&data, ParseOptions::default())?;

    // Create Zenoh publisher (uses multicast discovery by default)
    let publisher = ZenohPublisher::new(ZenohConfig::default()).await?;

    // Publish each record
    for record in &records {
        publisher.publish(record).await?;
        println!("Published CAT{} record", record.category);
    }

    // Clean shutdown
    publisher.close().await?;

    Ok(())
}
```

### Subscribing to ASTERIX Data

```rust
use asterix::transport::zenoh::{ZenohSubscriber, ZenohConfig};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Subscribe to all CAT048 data
    let mut subscriber = ZenohSubscriber::new(
        ZenohConfig::default(),
        "asterix/48/**"
    ).await?;

    println!("Waiting for ASTERIX data...");

    // Receive samples
    while let Some(sample) = subscriber.recv().await {
        println!(
            "Received: CAT{} from SAC={:?} SIC={:?} ({} bytes)",
            sample.category,
            sample.sac,
            sample.sic,
            sample.data.len()
        );
    }

    subscriber.close().await?;

    Ok(())
}
```

### Publishing Raw Bytes

For scenarios where you have raw ASTERIX data without parsing:

```rust
use asterix::transport::zenoh::{ZenohPublisher, ZenohConfig};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let publisher = ZenohPublisher::new(ZenohConfig::default()).await?;

    // Raw ASTERIX bytes
    let raw_data = vec![0x30, 0x00, 0x10, /* ... */];

    // Publish with category only
    publisher.publish_raw(48, &raw_data).await?;

    // Or publish with full routing info
    publisher.publish_raw_with_routing(48, 1, 2, &raw_data).await?;

    publisher.close().await?;

    Ok(())
}
```

### Connecting to a Zenoh Router

For larger deployments with a central router:

```rust
use asterix::transport::zenoh::{ZenohPublisher, ZenohConfig};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Connect to specific router
    let config = ZenohConfig::with_router("tcp/192.168.1.100:7447");

    let publisher = ZenohPublisher::new(config).await?;
    // ... publish data

    Ok(())
}

```

### Multiple Endpoints

Connect to multiple routers for redundancy:

```rust
use asterix::transport::zenoh::ZenohConfig;

let config = ZenohConfig::with_endpoints(vec![
    "tcp/router1.atc.local:7447".to_string(),
    "tcp/router2.atc.local:7447".to_string(),
]);
```

## Configuration Options

### ZenohConfig

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `endpoints` | `Vec<String>` | `[]` | Router endpoints to connect to |
| `key_prefix` | `String` | `"asterix"` | Prefix for key expressions |
| `include_raw_bytes` | `bool` | `true` | Include raw bytes in published data |
| `congestion_control` | `CongestionControl` | `Block` | How to handle network congestion |
| `priority` | `Priority` | `Data` | Message priority level |

### Congestion Control

| Mode | Description | Use Case |
|------|-------------|----------|
| `Block` | Wait until data can be sent | Critical data, guaranteed delivery |
| `Drop` | Drop data if network congested | High-frequency updates, latest matters most |

### Priority Levels

| Priority | Description |
|----------|-------------|
| `RealTime` | Highest priority (safety-critical) |
| `Interactive` | User-facing data |
| `Data` | Default for bulk data |
| `Background` | Lowest priority |

## Network Topologies

### Peer-to-Peer (Default)

Uses multicast discovery for automatic peer detection:

```rust
let config = ZenohConfig::peer_to_peer();
// or simply
let config = ZenohConfig::default();
```

**Suitable for:**
- Local network deployments
- Development and testing
- Small-scale systems

### Client-Router

Connect to central router(s):

```rust
let config = ZenohConfig::with_router("tcp/router.atc.local:7447");
```

**Suitable for:**
- Large-scale deployments
- Cross-site connectivity
- Cloud integration

### Mesh with Routers

Multiple interconnected routers:

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Site A    │     │   Site B    │     │   Cloud     │
│  Publisher  │────▶│   Router    │────▶│  Subscriber │
│  (CAT048)   │     │  (Bridge)   │     │  (Archive)  │
└─────────────┘     └─────────────┘     └─────────────┘
```

## Running the Examples

The crate includes ready-to-run examples:

### Publisher Example

```bash
# Parse a file and publish all records
cargo run --example zenoh_publisher --features zenoh -- sample.asterix

# Connect to specific router
cargo run --example zenoh_publisher --features zenoh -- \
    --router tcp/192.168.1.1:7447 sample.asterix
```

### Subscriber Example

```bash
# Subscribe to all ASTERIX data (multicast discovery)
cargo run --example zenoh_subscriber --features zenoh

# Subscribe to specific category
cargo run --example zenoh_subscriber --features zenoh -- \
    --filter "asterix/48/**"

# Receive limited samples
cargo run --example zenoh_subscriber --features zenoh -- \
    --max 100
```

## Performance Considerations

### Latency Optimization

1. **Use peer-to-peer mode** for lowest latency in local networks
2. **Set appropriate priority** for time-critical data:
   ```rust
   let mut config = ZenohConfig::default();
   config.priority = Priority::RealTime;
   ```

3. **Use congestion control wisely**:
   - `Block` for critical data
   - `Drop` for high-frequency updates

### Throughput Optimization

1. **Batch publications** when possible
2. **Use raw byte publishing** to avoid serialization overhead:
   ```rust
   publisher.publish_raw(category, &data).await?;
   ```

3. **Configure Zenoh for high throughput** (advanced):
   - Adjust buffer sizes
   - Enable batching
   - Use shared memory for local subscribers

### Memory Usage

- The subscriber uses a bounded channel (1000 samples) to prevent unbounded memory growth
- Use `try_recv()` for non-blocking receive to avoid blocking threads

## Troubleshooting

### Common Issues

**"Failed to open Zenoh session"**
- Check network connectivity
- Verify router is running (if configured)
- Check firewall rules for UDP port 7447

**"No data received"**
- Verify key expressions match between publisher and subscriber
- Check that publisher and subscriber are on same Zenoh network
- Try using broader wildcard subscription: `asterix/**`

**"High latency"**
- Check network path between peers
- Consider using routers for cross-network communication
- Monitor system resources (CPU, memory)

### Debug Logging

Enable Zenoh logging for diagnostics:

```bash
RUST_LOG=zenoh=debug cargo run --example zenoh_subscriber --features zenoh
```

## Integration with Other Transports

### Bridging to MQTT

Zenoh can bridge to MQTT brokers:

```bash
# Run Zenoh router with MQTT bridge
zenohd --rest-http-port 8000 --plugin mqtt
```

### Bridging to DDS

Zenoh has built-in DDS compatibility:

```bash
# Run Zenoh router with DDS bridge
zenohd --plugin dds
```

## Security Considerations

### Transport Security

Zenoh supports TLS for encrypted communications:

```rust
// Connect via TLS
let config = ZenohConfig::with_router("tls/secure-router.atc.local:7447");
```

### Access Control

For production deployments:

1. **Use dedicated network segments** for ASTERIX traffic
2. **Configure router access control** to limit who can publish/subscribe
3. **Enable authentication** on Zenoh routers
4. **Encrypt data in transit** using TLS

### Network Isolation

Consider network segmentation:

```
┌─────────────────────────────────────────────────────────┐
│                  Operational Network                     │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐          │
│  │ Radar 1  │    │ Radar 2  │    │ Radar 3  │          │
│  │ (CAT048) │    │ (CAT048) │    │ (CAT062) │          │
│  └────┬─────┘    └────┬─────┘    └────┬─────┘          │
│       │               │               │                  │
│       └───────────────┼───────────────┘                  │
│                       │                                   │
│              ┌────────▼────────┐                         │
│              │  Zenoh Router   │                         │
│              │  (DMZ/Bridge)   │                         │
│              └────────┬────────┘                         │
└───────────────────────┼──────────────────────────────────┘
                        │
                        │ (Filtered/Encrypted)
                        ▼
            ┌───────────────────────┐
            │   External Network    │
            │  (Cloud/Analytics)    │
            └───────────────────────┘
```

## Related Resources

- [Zenoh Documentation](https://zenoh.io/docs/)
- [Zenoh GitHub Repository](https://github.com/eclipse-zenoh/zenoh)
- [ASTERIX Protocol Documentation](../README.md)
- [Cyclone DDS Integration Guide](CYCLONEDDS_GUIDE.md)
- [gRPC Integration Guide](GRPC_GUIDE.md)
- [MQTT Integration Guide](MQTT_GUIDE.md)
