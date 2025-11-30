# Cyclone DDS Integration Guide

This guide covers using Eclipse Cyclone DDS with ASTERIX for real-time, deterministic data distribution in safety-critical ATM/ATC environments.

## Overview

Eclipse Cyclone DDS is an open-source implementation of the OMG Data Distribution Service (DDS) standard. It provides:

- **Microsecond latency** - Optimized for real-time systems
- **Deterministic delivery** - Predictable message timing
- **QoS policies** - Configurable reliability, durability, and ordering
- **Interoperability** - Standard DDS wire protocol

Cyclone DDS is particularly suited for:
- Safety-critical aviation systems (DO-178C/DO-278A)
- High-frequency radar data distribution
- Multi-sensor fusion systems
- Distributed ATM applications

## Building with Cyclone DDS Support

### Prerequisites

Install Cyclone DDS C++ bindings:

```bash
# Ubuntu/Debian
sudo apt-get install libcyclonedds-cxx-dev

# macOS (Homebrew)
brew install cyclonedds cyclonedds-cxx

# From source
git clone https://github.com/eclipse-cyclonedds/cyclonedds-cxx.git
cd cyclonedds-cxx
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make && sudo make install
```

### Build Configuration

```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_CYCLONEDDS=ON

cmake --build build --parallel
cmake --install build
```

## Command Line Usage

### Subscriber Mode

Receive ASTERIX data from DDS:

```bash
# Basic subscriber on domain 0
./asterix --dds SUB:0:AsterixData -j

# With reliable QoS
./asterix --dds SUB:0:AsterixCat048:reliable -j

# With transient durability (receive late-joiner data)
./asterix --dds SUB:0:AsterixCat062:transient -jh
```

### Publisher Mode

Publish ASTERIX data to DDS:

```bash
# Basic publisher on domain 0
./asterix -f recording.pcap --dds PUB:0:AsterixData

# With reliable QoS
./asterix -f cat048.raw --dds PUB:0:AsterixCat048:reliable
```

## Descriptor Format

```
mode:domain:topic[:qos]
```

| Parameter | Description | Values |
|-----------|-------------|--------|
| mode | DDS operation mode | `PUB` (publisher), `SUB` (subscriber) |
| domain | DDS domain ID | 0-232 (default: 0) |
| topic | Topic name | Any valid DDS topic name |
| qos | QoS profile (optional) | `besteffort`, `reliable`, `transient` |

## QoS Profiles

### Best Effort (default)
- Lowest latency
- No delivery guarantees
- Suitable for high-frequency radar data where latest is most important

```bash
./asterix --dds SUB:0:AsterixData:besteffort
```

### Reliable
- Guaranteed delivery with acknowledgments
- Higher latency than best-effort
- Suitable for control messages and critical updates

```bash
./asterix --dds SUB:0:AsterixData:reliable
```

### Transient
- Reliable delivery + late joiner support
- New subscribers receive historical data
- Suitable for configuration and status data

```bash
./asterix --dds SUB:0:AsterixStatus:transient
```

## Architecture

### Publisher Flow
```
ASTERIX Source → Parser → DDS DataWriter → DDS Network
```

### Subscriber Flow
```
DDS Network → DDS DataReader → Message Queue → ASTERIX Parser → Output
```

### Message Format

ASTERIX data is transmitted as raw bytes using DDS's built-in `BytesTopicType`:
- Preserves original ASTERIX binary format
- No serialization overhead
- Compatible with any DDS implementation

## Multi-Domain Configuration

Different ASTERIX categories can be isolated on separate DDS domains:

```bash
# Terminal 1: CAT048 on domain 1
./asterix --dds SUB:1:AsterixCat048 -j

# Terminal 2: CAT062 on domain 2
./asterix --dds SUB:2:AsterixCat062 -j

# Terminal 3: CAT065 on domain 3
./asterix --dds SUB:3:AsterixCat065 -j
```

## Performance Considerations

### Latency Optimization

1. **Use best-effort QoS** for high-frequency data
2. **Configure Cyclone DDS** for low latency:
   ```xml
   <!-- cyclonedds.xml -->
   <CycloneDDS>
     <Domain>
       <General>
         <NetworkInterfaceAddress>auto</NetworkInterfaceAddress>
       </General>
       <Internal>
         <MaxMessageSize>65500</MaxMessageSize>
       </Internal>
     </Domain>
   </CycloneDDS>
   ```

3. **Set environment variable**:
   ```bash
   export CYCLONEDDS_URI=file://cyclonedds.xml
   ```

### Throughput Optimization

1. **Batch messages** when possible
2. **Use multicast** for multiple subscribers
3. **Configure buffer sizes** appropriately

## Troubleshooting

### Common Issues

**"Cyclone DDS not found"**
- Install cyclonedds-cxx development package
- Ensure CMake can find the package config

**"Domain ID out of range"**
- Valid domain IDs are 0-232
- Domain 233+ is reserved

**"Topic name mismatch"**
- Topic names are case-sensitive
- Ensure publishers and subscribers use exact same topic name

### Debug Logging

Enable verbose logging:
```bash
export CYCLONEDDS_TRACE=all
./asterix --dds SUB:0:AsterixData -v
```

## Integration Examples

### Python with Cyclone DDS

```python
import asterix
from cyclonedds.pub import DataWriter
from cyclonedds.domain import DomainParticipant

# Parse ASTERIX and publish via DDS
dp = DomainParticipant(0)
writer = DataWriter(dp, "AsterixData", Bytes)

with open("recording.raw", "rb") as f:
    data = f.read()
    records = asterix.parse(data)
    for record in records:
        writer.write(record.raw_bytes)
```

### Rust with Cyclone DDS

```rust
use asterix_decoder::{parse, ParseOptions};
use cyclonedds_rs::*;

fn main() {
    let dp = DomainParticipant::new(0)?;
    let topic = dp.create_topic::<Vec<u8>>("AsterixData")?;
    let writer = dp.create_datawriter(&topic)?;

    let data = std::fs::read("recording.raw")?;
    let records = parse(&data, ParseOptions::default())?;

    for record in records {
        writer.write(&record.raw_bytes())?;
    }
}
```

## Security Considerations

### DDS Security

For production deployments, enable DDS Security:

1. Configure authentication (PKI-based)
2. Enable access control
3. Encrypt data on the wire

See [OMG DDS Security Specification](https://www.omg.org/spec/DDS-SECURITY/) for details.

### Network Isolation

- Use separate VLANs for ASTERIX DDS traffic
- Configure firewall rules for DDS ports (7400-7499 by default)
- Consider domain isolation for different trust levels

## Related Resources

- [Eclipse Cyclone DDS Documentation](https://cyclonedds.io/docs/)
- [OMG DDS Specification](https://www.omg.org/spec/DDS/)
- [ASTERIX Protocol Documentation](../README.md)
- [gRPC Integration Guide](GRPC_GUIDE.md)
- [MQTT Integration Guide](MQTT_GUIDE.md)
