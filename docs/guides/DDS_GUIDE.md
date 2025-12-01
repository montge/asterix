# DDS Integration Guide

This guide covers using DDS (Data Distribution Service) with ASTERIX for real-time pub/sub data distribution in ATM/ATC environments.

## Overview

[DDS (Data Distribution Service)](https://www.omg.org/spec/DDS/) is an OMG standard for real-time data distribution. The asterix-rs implementation uses [RustDDS](https://github.com/Atostek/RustDDS), a native Rust implementation of the RTPS (Real-Time Publish-Subscribe) protocol.

Key features:
- **RTPS Protocol** - Interoperable with Fast DDS, OpenDDS, Cyclone DDS, and other DDS implementations
- **QoS Policies** - Fine-grained control over reliability, durability, history, and deadlines
- **Automatic Discovery** - SPDP/SEDP protocols for automatic participant discovery
- **UDP Multicast/Unicast** - Flexible network transport options
- **Domain Isolation** - Separate communication domains for network segmentation

DDS is particularly suited for:
- Real-time surveillance data distribution
- Multi-sensor fusion systems
- Safety-critical ATM applications requiring QoS guarantees
- Interoperability with existing DDS-based systems (ROS 2, military systems)

## Building with DDS Support (Rust)

The DDS integration is implemented in the Rust `asterix-rs` crate using native Rust for optimal performance and safety.

### Prerequisites

- Rust 1.88+ with Cargo
- C++17 compatible compiler (for ASTERIX core)
- libexpat-devel (for XML parsing)

### Build Configuration

```bash
cd asterix-rs

# Build with DDS feature enabled
cargo build --features dds

# Run tests
cargo test --features dds

# Build release version
cargo build --release --features dds

# Run benchmarks
cargo bench --features dds -- dds
```

### Cargo.toml Configuration

Add the DDS feature to your `Cargo.toml`:

```toml
[dependencies]
asterix = { version = "0.1", features = ["dds"] }
```

## API Overview

### Key Components

| Component | Description |
|-----------|-------------|
| `DdsPublisher` | Publishes ASTERIX records to DDS topics |
| `DdsSubscriber` | Subscribes to ASTERIX data from DDS topics |
| `DdsConfig` | Configuration for DDS domain participants |
| `AsterixSample` | Received ASTERIX sample with metadata |
| `AsterixMessage` | Internal message format for DDS transport |

### Topic Naming Convention

ASTERIX data is published using the following topic naming format:

```
asterix_cat{category}
```

Or with SAC/SIC routing:

```
asterix_cat{category}_sac{sac}_sic{sic}
```

| Component | Description | Example |
|-----------|-------------|---------|
| `category` | ASTERIX category number | 48, 62, 65 |
| `sac` | System Area Code | 1, 10, 255 |
| `sic` | System Identification Code | 2, 20, 128 |

**Examples:**
- `asterix_cat48` - All CAT048 data
- `asterix_cat62_sac1_sic2` - CAT062 from SAC=1, SIC=2
- `asterix_cat48_sac10_sic20` - CAT048 from SAC=10, SIC=20

## Usage Examples

### Publishing ASTERIX Data

```rust
use asterix::{parse, ParseOptions, init_default};
use asterix::transport::dds::{DdsPublisher, DdsConfig};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize ASTERIX parser
    init_default()?;

    // Read ASTERIX data
    let data = std::fs::read("sample.asterix")?;
    let records = parse(&data, ParseOptions::default())?;

    // Create publisher with default config (domain 0, reliable)
    let publisher = DdsPublisher::new(DdsConfig::default())?;

    // Publish each record
    for record in records {
        publisher.publish(&record)?;
        println!("Published CAT{}", record.category);
    }

    Ok(())
}
```

### Publishing Raw ASTERIX Data

```rust
use asterix::transport::dds::{DdsPublisher, DdsConfig};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let publisher = DdsPublisher::new(DdsConfig::default())?;

    // Raw ASTERIX bytes
    let data = vec![0x30, 0x00, 0x10, /* ... */];

    // Publish with category only
    publisher.publish_raw(48, &data)?;

    // Publish with full routing info
    publisher.publish_raw_with_routing(48, 1, 2, &data)?;

    Ok(())
}
```

### Subscribing to ASTERIX Data

```rust
use asterix::transport::dds::{DdsSubscriber, DdsConfig};
use std::time::Duration;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Subscribe to CAT048 data
    let mut subscriber = DdsSubscriber::new(
        DdsConfig::default(),
        "asterix_cat48"
    )?;

    // Receive loop
    loop {
        // Non-blocking receive
        if let Some(sample) = subscriber.try_recv() {
            println!("Received CAT{}: {} bytes",
                sample.category,
                sample.data.len()
            );
        }

        // Or with timeout
        if let Some(sample) = subscriber.recv_timeout(Duration::from_secs(1)) {
            println!("Got data: {:?}", sample);
        }

        std::thread::sleep(Duration::from_millis(10));
    }
}
```

## Configuration Options

### DdsConfig

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `domain_id` | `u16` | `0` | DDS domain ID for isolation |
| `topic_prefix` | `String` | `"asterix"` | Prefix for topic names |
| `reliability` | `Reliability` | `Reliable` | QoS reliability mode |
| `durability` | `Durability` | `Volatile` | QoS durability mode |
| `history` | `History` | `KeepLast(10)` | QoS history mode |
| `deadline_ms` | `u64` | `0` | Deadline period (0 = none) |

### Configuration Presets

```rust
use asterix::transport::dds::DdsConfig;

// Default: Reliable, Volatile, KeepLast(10)
let default_config = DdsConfig::default();

// Best effort: Lower latency, may lose samples
let fast_config = DdsConfig::best_effort();

// Reliable with durability: Late joiners receive recent data
let reliable_config = DdsConfig::reliable();

// Specific domain
let isolated_config = DdsConfig::with_domain(42);

// Custom configuration
let custom_config = DdsConfig {
    domain_id: 5,
    topic_prefix: "radar".to_string(),
    reliability: Reliability::BestEffort,
    durability: Durability::TransientLocal,
    history: History::KeepAll,
    deadline_ms: 100,
};
```

## QoS Policies

### Reliability

| Mode | Description | Use Case |
|------|-------------|----------|
| `BestEffort` | No retransmission, lowest latency | Real-time display |
| `Reliable` | Guaranteed delivery with retransmission | Data recording |

### Durability

| Mode | Description | Use Case |
|------|-------------|----------|
| `Volatile` | Data not persisted | Live streaming |
| `TransientLocal` | Recent data available to late joiners | System startup |

### History

| Mode | Description | Use Case |
|------|-------------|----------|
| `KeepLast(N)` | Keep only last N samples | Normal operation |
| `KeepAll` | Keep all samples | Replay/analysis |

## Domain Isolation

DDS uses domain IDs to isolate communication:

```rust
use asterix::transport::dds::{DdsPublisher, DdsConfig};

// Production system on domain 0
let prod_publisher = DdsPublisher::new(DdsConfig::with_domain(0))?;

// Test system on domain 1 (completely isolated)
let test_publisher = DdsPublisher::new(DdsConfig::with_domain(1))?;

// Development on domain 99
let dev_publisher = DdsPublisher::new(DdsConfig::with_domain(99))?;
```

## Interoperability

The RustDDS implementation uses the RTPS protocol, making it interoperable with:

- **Fast DDS (eProsima)** - ROS 2 default middleware
- **OpenDDS** - Open-source DDS implementation
- **Cyclone DDS** - Eclipse project DDS
- **RTI Connext DDS** - Commercial DDS implementation
- **CoreDX DDS** - Embedded DDS

### Topic Type Compatibility

To receive ASTERIX data from this implementation in another DDS system, register the following type:

```idl
// AsterixMessage.idl
module asterix {
    struct AsterixMessage {
        string key;           // Topic key
        octet category;       // ASTERIX category (0-255)
        octet sac;            // System Area Code (optional)
        octet sic;            // System Identification Code (optional)
        sequence<octet> data; // Raw ASTERIX bytes
        unsigned long long timestamp; // Microseconds since epoch
    };
    #pragma keylist AsterixMessage key
};
```

## Performance Tuning

### Best Effort for Low Latency

```rust
let config = DdsConfig {
    reliability: Reliability::BestEffort,
    history: History::KeepLast(1),
    ..Default::default()
};
```

### Reliable for Data Integrity

```rust
let config = DdsConfig {
    reliability: Reliability::Reliable,
    durability: Durability::TransientLocal,
    history: History::KeepLast(100),
    ..Default::default()
};
```

### High-Throughput Configuration

```rust
let config = DdsConfig {
    reliability: Reliability::BestEffort,
    history: History::KeepLast(1),
    deadline_ms: 0, // No deadline checking
    ..Default::default()
};
```

## Benchmarks

Run performance benchmarks:

```bash
cargo bench --features dds -- dds
```

Benchmark results (example on typical hardware):

| Benchmark | Result |
|-----------|--------|
| Publisher create | ~5ms |
| Publish 100B (best effort) | ~50µs |
| Publish 1KB (best effort) | ~60µs |
| Publish 64KB (best effort) | ~200µs |
| Pub/sub roundtrip 1KB | ~2ms |

## Troubleshooting

### No Data Received

1. **Check domain IDs match** - Publisher and subscriber must use same domain
2. **Allow discovery time** - DDS needs 100-500ms for participant discovery
3. **Check topic names match** - Topic names are case-sensitive
4. **Check QoS compatibility** - Reliable publisher requires reliable subscriber

### Performance Issues

1. Use `BestEffort` reliability for lowest latency
2. Reduce history depth with `KeepLast(1)`
3. Use appropriate payload sizes (batch small messages)
4. Consider using separate domains for different data flows

### Firewall Configuration

DDS uses the following ports by default:
- **7400** - RTPS discovery (multicast)
- **7401+** - User traffic (unicast/multicast)

Ensure these ports are open for UDP traffic.

## See Also

- [RustDDS Documentation](https://docs.rs/rustdds)
- [DDS Specification](https://www.omg.org/spec/DDS/)
- [RTPS Specification](https://www.omg.org/spec/DDSI-RTPS/)
- [Zenoh Guide](ZENOH_GUIDE.md) - Alternative pub/sub transport

