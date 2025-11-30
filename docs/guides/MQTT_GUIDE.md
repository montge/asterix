# MQTT Transport Guide

This guide covers using MQTT (Message Queuing Telemetry Transport) for distributing and receiving ASTERIX data through IoT and cloud messaging infrastructure.

## Overview

MQTT is a lightweight publish-subscribe messaging protocol ideal for:
- **IoT Integration** - Connect ASTERIX data to IoT platforms
- **Cloud Messaging** - Distribute data via cloud MQTT brokers (AWS IoT, Azure IoT Hub, HiveMQ)
- **Low-Bandwidth Networks** - Efficient protocol for constrained networks
- **Topic-Based Distribution** - Organize data by category, sensor, or region

## Building with MQTT Support

MQTT support requires the Eclipse Mosquitto library (`libmosquitto`).

### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt-get install libmosquitto-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install mosquitto-devel
```

**macOS:**
```bash
brew install mosquitto
```

**Windows (vcpkg):**
```bash
vcpkg install mosquitto
```

### Build Configuration

```bash
cmake -B build -DENABLE_MQTT=ON -DCMAKE_INSTALL_PREFIX=install
cmake --build build --parallel
cmake --install build
```

## Command Line Usage

### Basic Syntax

```bash
asterix -m MODE:HOST:PORT:TOPIC[:QOS[:CLIENTID[:USER:PASS]]] [options]
```

### Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| MODE | SUB (subscribe) or PUB (publish) | SUB |
| HOST | MQTT broker hostname | localhost |
| PORT | Broker port (1883 standard, 8883 TLS) | 1883 |
| TOPIC | MQTT topic for pub/sub | asterix/raw |
| QOS | Quality of Service level (0, 1, 2) | 1 |
| CLIENTID | Client identifier | asterix-decoder |
| USER:PASS | Authentication credentials | admin:secret |

### Examples

**Subscribe to all ASTERIX topics:**
```bash
asterix -m SUB:localhost:1883:asterix/# -j
```

**Subscribe with QoS 1:**
```bash
asterix -m SUB:broker.example.com:1883:asterix/cat048:1 -j
```

**Subscribe with custom client ID:**
```bash
asterix -m SUB:localhost:1883:asterix/radar/primary:1:asterix-consumer -j
```

**Subscribe with authentication:**
```bash
asterix -m SUB:broker.example.com:1883:asterix/#:1:my-client:admin:secret123 -j
```

## MQTT Topics

### Recommended Topic Structure

```
asterix/
├── raw/                    # Raw ASTERIX data
│   ├── cat048/            # Category 048 (Surveillance)
│   ├── cat062/            # Category 062 (SDPS Track)
│   └── cat021/            # Category 021 (ADS-B)
├── decoded/               # Pre-decoded JSON data
│   ├── cat048/
│   └── cat062/
└── alerts/                # Alert messages
    └── track-updates/
```

### Topic Wildcards

- **`#`** - Multi-level wildcard: `asterix/#` matches all topics
- **`+`** - Single-level wildcard: `asterix/+/cat048` matches any middle segment

## Quality of Service (QoS) Levels

| QoS | Name | Delivery Guarantee | Use Case |
|-----|------|-------------------|----------|
| 0 | At most once | Fire and forget | High-frequency updates |
| 1 | At least once | Acknowledged delivery | Standard surveillance |
| 2 | Exactly once | Four-step handshake | Critical data (alerts) |

**Recommendation:** Use QoS 1 for most ASTERIX data to balance reliability and performance.

## Architecture Patterns

### Pattern 1: Direct Subscription

```
MQTT Broker → asterix decoder → stdout/file
```

```bash
# Decode ASTERIX from MQTT to JSON
asterix -m SUB:localhost:1883:asterix/raw:1 -j > decoded.json
```

### Pattern 2: Bridge from Multicast

```
UDP Multicast → asterix encoder → MQTT Broker → subscribers
```

```bash
# Future: Publishing support
asterix -i 232.1.1.10:10.0.0.1:21000 -m PUB:localhost:1883:asterix/cat048:1
```

### Pattern 3: Cloud Integration

```
Remote Sensor → MQTT (TLS) → Cloud Broker → asterix decoder
```

```bash
# Connect to cloud broker
asterix -m SUB:mqtt.example.cloud:8883:asterix/sensor-1:2:my-app -j
```

## Configuration Examples

### Local Development

```bash
# Start local Mosquitto broker
mosquitto -v

# Subscribe to ASTERIX data
asterix -m SUB:localhost:1883:asterix/#:0 -j
```

### AWS IoT Core

```bash
# Connect with AWS IoT endpoint (requires certificates configured in broker)
asterix -m SUB:YOUR-IOT-ENDPOINT.iot.us-east-1.amazonaws.com:8883:asterix/data:1:asterix-client -j
```

### HiveMQ Cloud

```bash
# Connect to HiveMQ cloud cluster
asterix -m SUB:CLUSTER-ID.hivemq.cloud:8883:asterix/#:1:my-client:username:password -j
```

## Message Format

ASTERIX data is published as raw binary payloads. The decoder expects:

- **Input (SUB mode):** Raw ASTERIX binary data
- **Output:** Decoded format based on output options (-j, -x, -l)

### Publishing Custom Data

When publishing ASTERIX data to MQTT (PUB mode), ensure:
1. Data is raw ASTERIX binary format
2. Each message contains complete data blocks
3. Topic matches the expected subscription pattern

## Troubleshooting

### Connection Issues

**Problem:** "Failed to connect to MQTT broker"
```bash
# Verify broker is running
mosquitto_sub -h localhost -p 1883 -t test

# Check firewall rules
sudo ufw allow 1883/tcp
```

**Problem:** "Timeout waiting for MQTT connection"
```bash
# Test connectivity
nc -zv broker.example.com 1883

# Check DNS resolution
nslookup broker.example.com
```

### Authentication Failures

**Problem:** "MQTT connection failed: Connection refused"
```bash
# Verify credentials
mosquitto_pub -h localhost -u username -P password -t test -m "hello"

# Check broker ACL configuration
cat /etc/mosquitto/acl
```

### Message Not Received

**Problem:** No data appears after subscribing
```bash
# Test topic manually
mosquitto_sub -h localhost -t 'asterix/#' -v

# Check topic pattern matches publisher
mosquitto_pub -h localhost -t 'asterix/raw' -m "test"
```

## Performance Considerations

### Throughput

| Configuration | Messages/sec | Latency |
|--------------|--------------|---------|
| QoS 0, local | ~50,000 | <1ms |
| QoS 1, local | ~20,000 | ~5ms |
| QoS 2, local | ~5,000 | ~20ms |
| QoS 1, cloud | ~1,000 | 50-200ms |

### Best Practices

1. **Use QoS 0** for high-frequency, loss-tolerant data
2. **Use QoS 1** for standard surveillance data
3. **Reserve QoS 2** for critical alerts only
4. **Batch messages** when possible to reduce overhead
5. **Use persistent sessions** for reconnection resilience

## Security

### TLS/SSL

For production deployments, always use TLS (port 8883):

```bash
# Connect with TLS
asterix -m SUB:secure-broker.example.com:8883:asterix/#:1 -j
```

### Authentication

Always configure username/password or certificate-based authentication:

```bash
# Username/password authentication
asterix -m SUB:broker:8883:asterix/#:1:client-id:user:password -j
```

### Access Control

Configure broker ACLs to restrict topic access:

```
# mosquitto.acl example
user asterix-decoder
topic read asterix/#

user asterix-publisher
topic write asterix/#
```

## Integration Examples

### Python MQTT Publisher

```python
import paho.mqtt.client as mqtt

# Read ASTERIX data and publish to MQTT
client = mqtt.Client("asterix-publisher")
client.connect("localhost", 1883)

with open("asterix_data.bin", "rb") as f:
    data = f.read()
    client.publish("asterix/raw", data, qos=1)

client.disconnect()
```

### Node.js Subscriber

```javascript
const mqtt = require('mqtt');
const { spawn } = require('child_process');

const client = mqtt.connect('mqtt://localhost:1883');

client.on('connect', () => {
    client.subscribe('asterix/#');
});

client.on('message', (topic, message) => {
    // Decode ASTERIX data
    const decoder = spawn('asterix', ['-j']);
    decoder.stdin.write(message);
    decoder.stdin.end();

    decoder.stdout.on('data', (data) => {
        console.log('Decoded:', JSON.parse(data));
    });
});
```

## See Also

- [ZeroMQ Transport Guide](ZEROMQ_GUIDE.md) - Low-latency messaging
- [gRPC Transport Guide](GRPC_GUIDE.md) - High-performance RPC
- [Network Setup Guide](NETWORK_SETUP.md) - UDP multicast configuration
