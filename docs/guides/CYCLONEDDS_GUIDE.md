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

#### Step 1: Install Cyclone DDS C Library (Ubuntu/Debian)

The C library is available in Ubuntu repositories:

```bash
sudo apt-get update
sudo apt-get install -y \
    cyclonedds-dev \
    cyclonedds-tools \
    libddsc0t64 \
    libcycloneddsidl0t64
```

Verify the installation:
```bash
pkg-config --modversion CycloneDDS
# Expected output: 0.10.4 (or similar)
```

#### Step 2: Build and Install Cyclone DDS C++ Bindings

**Note:** The `cyclonedds-cxx` C++ bindings are **not available** as Ubuntu packages and must be built from source.

```bash
# Clone the repository (or use your own fork)
git clone https://github.com/eclipse-cyclonedds/cyclonedds-cxx.git
cd cyclonedds-cxx

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release

# Build (uses all available CPU cores)
make -j$(nproc)

# Generate Debian packages
cpack -G DEB

# Install the packages
sudo apt install ./eclipse-cyclone-dds-iso-iec-c++-psm_*.deb
sudo apt install ./eclipse-cyclone-dds-iso-iec-c++-psm-dev_*.deb
sudo apt install ./CycloneDDS-CXX-*-Linux-idlcxx.deb
```

**Generated Packages:**

| Package | Description |
|---------|-------------|
| `eclipse-cyclone-dds-iso-iec-c++-psm_*.deb` | C++ runtime library (`libddscxx.so`) |
| `eclipse-cyclone-dds-iso-iec-c++-psm-dev_*.deb` | Development headers |
| `CycloneDDS-CXX-*-Linux-idlcxx.deb` | IDL compiler C++ plugin |

**Verify the installation:**
```bash
# Check that the library is installed
ldconfig -p | grep ddscxx
# Expected: libddscxx.so.0 (libc6,x86-64) => /usr/lib/x86_64-linux-gnu/libddscxx.so.0

# Check CMake can find it
pkg-config --exists CycloneDDS-CXX && echo "CycloneDDS-CXX found"
```

#### macOS (Homebrew)

```bash
brew install cyclonedds cyclonedds-cxx
```

#### From Source (Alternative)

If you prefer to install directly without packages:

```bash
git clone https://github.com/eclipse-cyclonedds/cyclonedds-cxx.git
cd cyclonedds-cxx
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j$(nproc)
sudo make install
sudo ldconfig
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

## Building Debian Packages for Cyclone DDS C++

Since the C++ bindings are not available in Ubuntu repositories, you may want to build and distribute your own Debian packages. This section provides detailed instructions.

### Prerequisites

Ensure you have the required build tools:

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    dpkg-dev \
    cyclonedds-dev \
    cyclonedds-tools \
    libddsc0t64 \
    libcycloneddsidl0t64
```

### Clone and Configure

```bash
# Clone the official repository
git clone https://github.com/eclipse-cyclonedds/cyclonedds-cxx.git
cd cyclonedds-cxx

# Create and enter build directory
mkdir build && cd build

# Configure CMake
# Note: Use -DCMAKE_INSTALL_PREFIX=/usr for proper Debian package paths
cmake .. \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release
```

**CMake Configuration Output:**
You should see output similar to:
```
-- Found iceoryx_binding_c: (found version "2.0.5")
-- Compiling with type discovery support
-- Configuring done
-- Generating done
```

### Build the Project

```bash
# Build using all available CPU cores
make -j$(nproc)
```

The build typically takes 30-60 seconds and produces:
- `lib/libddscxx.so.0.10.4` - Shared library
- `lib/libcycloneddsidlcxx.so` - IDL compiler plugin

### Generate Debian Packages

The project includes CPack configuration for Debian package generation:

```bash
# Generate all three .deb packages
cpack -G DEB
```

**Generated Packages:**

| File | Size | Contents |
|------|------|----------|
| `eclipse-cyclone-dds-iso-iec-c++-psm_0.10.4_amd64.deb` | ~325 KB | Runtime library (`libddscxx.so`) |
| `eclipse-cyclone-dds-iso-iec-c++-psm-dev_0.10.4_amd64.deb` | ~254 KB | Development headers and CMake config |
| `CycloneDDS-CXX-0.10.4-Linux-idlcxx.deb` | ~35 KB | IDL compiler C++ plugin (`libcycloneddsidlcxx.so`) |

### Inspect Package Contents

Before installing, you can inspect what each package contains:

```bash
# View package metadata
dpkg -I eclipse-cyclone-dds-iso-iec-c++-psm_0.10.4_amd64.deb

# List package contents
dpkg -c eclipse-cyclone-dds-iso-iec-c++-psm_0.10.4_amd64.deb
```

**Runtime library package contents:**
```
/usr/lib/x86_64-linux-gnu/libddscxx.so -> libddscxx.so.0
/usr/lib/x86_64-linux-gnu/libddscxx.so.0 -> libddscxx.so.0.10.4
/usr/lib/x86_64-linux-gnu/libddscxx.so.0.10.4
```

**Development package contents:**
```
/usr/include/dds/
/usr/include/org/
/usr/lib/cmake/CycloneDDS-CXX/
```

### Install the Packages

```bash
# Install all three packages
sudo apt install \
    ./eclipse-cyclone-dds-iso-iec-c++-psm_0.10.4_amd64.deb \
    ./eclipse-cyclone-dds-iso-iec-c++-psm-dev_0.10.4_amd64.deb \
    ./CycloneDDS-CXX-0.10.4-Linux-idlcxx.deb

# Update library cache
sudo ldconfig
```

**Note:** You may see a warning like:
```
N: Download is performed unsandboxed as root as file '...' couldn't be accessed by user '_apt'.
```
This is harmless - it's just apt being cautious about local files.

### Verify Installation

```bash
# Check library is in linker cache
ldconfig -p | grep ddscxx
# Expected: libddscxx.so.0 (libc6,x86-64) => /usr/lib/x86_64-linux-gnu/libddscxx.so.0

# Check CMake can find the package
cmake --find-package -DNAME=CycloneDDS-CXX -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST
# Expected: CycloneDDS-CXX found.

# Check installed packages
dpkg -l | grep -i cyclone
```

### Uninstalling

To remove the packages:

```bash
sudo apt remove \
    eclipse-cyclone-dds-iso-iec-c++-psm \
    eclipse-cyclone-dds-iso-iec-c++-psm-dev \
    cyclonedds-cxx-idlcxx
```

### Building for Distribution

If you're building packages for distribution to other machines:

1. **Ensure consistent build environment** - Use a clean Ubuntu installation or Docker container
2. **Version matching** - The C++ bindings version should match the installed C library version
3. **Architecture** - Packages are architecture-specific (amd64, arm64, etc.)

**Docker-based build example:**
```bash
docker run --rm -v $(pwd):/src -w /src ubuntu:24.04 bash -c "
    apt-get update && \
    apt-get install -y build-essential cmake git cyclonedds-dev && \
    cd cyclonedds-cxx && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release && \
    make -j\$(nproc) && \
    cpack -G DEB
"
```

### Troubleshooting Package Generation

**Issue: "CycloneDDS not found" during cmake**

Solution: Install the C library first:
```bash
sudo apt-get install cyclonedds-dev
```

**Issue: cpack fails with permission errors**

Solution: Ensure you're running from a writable directory:
```bash
cd /tmp
git clone https://github.com/eclipse-cyclonedds/cyclonedds-cxx.git
cd cyclonedds-cxx/build
cpack -G DEB
```

**Issue: Package installs but library not found at runtime**

Solution: Update the linker cache:
```bash
sudo ldconfig
```

## Related Resources

- [Eclipse Cyclone DDS Documentation](https://cyclonedds.io/docs/)
- [OMG DDS Specification](https://www.omg.org/spec/DDS/)
- [ASTERIX Protocol Documentation](../README.md)
- [gRPC Integration Guide](GRPC_GUIDE.md)
- [MQTT Integration Guide](MQTT_GUIDE.md)
