# gRPC Transport Guide

This guide covers using gRPC for high-performance, bidirectional streaming of ASTERIX data between services.

## Overview

gRPC is a high-performance RPC framework ideal for:
- **Microservices Architecture** - Connect ASTERIX decoders to processing services
- **Cross-Language Integration** - Auto-generated clients for 10+ languages
- **Bidirectional Streaming** - Real-time data flow in both directions
- **Cloud-Native Deployment** - Native Kubernetes and service mesh support

## Building with gRPC Support

gRPC support requires the gRPC C++ library (`grpc++`).

### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt-get install libgrpc++-dev protobuf-compiler-grpc
```

**Fedora/RHEL:**
```bash
sudo dnf install grpc-devel grpc-plugins
```

**macOS:**
```bash
brew install grpc
```

**Windows (vcpkg):**
```bash
vcpkg install grpc
```

### Build Configuration

```bash
cmake -B build -DENABLE_GRPC=ON -DCMAKE_INSTALL_PREFIX=install
cmake --build build --parallel
cmake --install build
```

## Command Line Usage

### Basic Syntax

```bash
asterix -g MODE:ENDPOINT[:TLS] [options]
```

### Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| MODE | CLIENT or SERVER | CLIENT |
| ENDPOINT | host:port address | localhost:50051 |
| TLS | Enable TLS/SSL (optional) | tls |

### Examples

**Client connecting to gRPC server:**
```bash
asterix -g CLIENT:localhost:50051 -j
```

**Client with TLS:**
```bash
asterix -g CLIENT:radar.example.com:50051:tls -j
```

**Server accepting connections:**
```bash
asterix -g SERVER:0.0.0.0:50051 -j
```

**Server on specific interface:**
```bash
asterix -g SERVER:192.168.1.100:50051 -j
```

## Architecture Modes

### Client Mode

In client mode, ASTERIX connects to an existing gRPC server to receive streaming data:

```
gRPC Server (data source) → asterix client → decoded output
```

Use cases:
- Connecting to a centralized radar data distribution server
- Receiving data from a gRPC-based data aggregation service
- Integration with cloud-based surveillance platforms

### Server Mode

In server mode, ASTERIX accepts incoming gRPC connections:

```
Data source → asterix server → gRPC clients (consumers)
```

Use cases:
- Broadcasting decoded ASTERIX data to multiple consumers
- Providing a gRPC interface for legacy UDP multicast data
- Building a surveillance data microservice

## Architecture Patterns

### Pattern 1: Direct Client

```
gRPC Data Server → asterix (client) → stdout/file
```

```bash
# Receive and decode ASTERIX from gRPC server
asterix -g CLIENT:data-server.local:50051 -j > decoded.json
```

### Pattern 2: Gateway Server

```
UDP Multicast → asterix (server) → gRPC Clients
```

```bash
# Bridge multicast to gRPC (future feature)
asterix -i 232.1.1.10:10.0.0.1:21000 -g SERVER:0.0.0.0:50051
```

### Pattern 3: Microservices Pipeline

```
asterix (server) → Processing Service → Database
                → Analytics Service → Dashboard
                → Alert Service → Notifications
```

### Pattern 4: Load-Balanced Deployment

```
                    ┌→ asterix instance 1
Load Balancer → ────┼→ asterix instance 2
                    └→ asterix instance 3
```

## Service Integration

### Protocol Buffer Definition

The gRPC service uses a simple streaming protocol for raw ASTERIX data:

```protobuf
syntax = "proto3";

package asterix;

// ASTERIX data streaming service
service AsterixStream {
    // Server streams ASTERIX data blocks to client
    rpc StreamData(StreamRequest) returns (stream AsterixData);

    // Bidirectional streaming for full-duplex communication
    rpc BiDirectionalStream(stream AsterixData) returns (stream AsterixData);
}

message StreamRequest {
    repeated int32 categories = 1;  // Filter by ASTERIX category
    bool include_raw = 2;           // Include raw bytes
}

message AsterixData {
    bytes raw_data = 1;             // Raw ASTERIX binary
    int64 timestamp = 2;            // Reception timestamp
    string source_id = 3;           // Data source identifier
}
```

### Python Client Example

```python
import grpc
import asterix_pb2
import asterix_pb2_grpc

def receive_asterix():
    channel = grpc.insecure_channel('localhost:50051')
    stub = asterix_pb2_grpc.AsterixStreamStub(channel)

    request = asterix_pb2.StreamRequest(
        categories=[48, 62],
        include_raw=True
    )

    for data in stub.StreamData(request):
        print(f"Received {len(data.raw_data)} bytes from {data.source_id}")
        # Process raw ASTERIX data
        process_asterix(data.raw_data)

if __name__ == '__main__':
    receive_asterix()
```

### Go Client Example

```go
package main

import (
    "context"
    "log"

    "google.golang.org/grpc"
    pb "github.com/example/asterix/proto"
)

func main() {
    conn, err := grpc.Dial("localhost:50051", grpc.WithInsecure())
    if err != nil {
        log.Fatalf("Failed to connect: %v", err)
    }
    defer conn.Close()

    client := pb.NewAsterixStreamClient(conn)
    stream, err := client.StreamData(context.Background(), &pb.StreamRequest{
        Categories: []int32{48, 62},
    })
    if err != nil {
        log.Fatalf("Failed to stream: %v", err)
    }

    for {
        data, err := stream.Recv()
        if err != nil {
            break
        }
        log.Printf("Received %d bytes", len(data.RawData))
    }
}
```

### Rust Client Example

```rust
use tonic::transport::Channel;

pub mod asterix {
    tonic::include_proto!("asterix");
}

use asterix::asterix_stream_client::AsterixStreamClient;
use asterix::StreamRequest;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let channel = Channel::from_static("http://localhost:50051")
        .connect()
        .await?;

    let mut client = AsterixStreamClient::new(channel);

    let request = StreamRequest {
        categories: vec![48, 62],
        include_raw: true,
    };

    let mut stream = client.stream_data(request).await?.into_inner();

    while let Some(data) = stream.message().await? {
        println!("Received {} bytes", data.raw_data.len());
    }

    Ok(())
}
```

## TLS/SSL Configuration

### Server with TLS

For production deployments, enable TLS:

```bash
# Server with TLS (certificates auto-configured)
asterix -g SERVER:0.0.0.0:50051:tls -j
```

### Client with TLS

```bash
# Client connecting with TLS
asterix -g CLIENT:secure-server.example.com:50051:tls -j
```

### Certificate Configuration

For custom certificates, set environment variables:

```bash
export GRPC_SSL_CERT_FILE=/path/to/cert.pem
export GRPC_SSL_KEY_FILE=/path/to/key.pem
export GRPC_SSL_CA_FILE=/path/to/ca.pem

asterix -g SERVER:0.0.0.0:50051:tls -j
```

## Performance Tuning

### Channel Configuration

| Setting | Default | High-Throughput |
|---------|---------|-----------------|
| Max message size | 4MB | 64MB |
| Keepalive time | 7200s | 30s |
| Keepalive timeout | 20s | 10s |
| Max concurrent streams | 100 | 1000 |

### Throughput Benchmarks

| Configuration | Messages/sec | Latency (p99) |
|--------------|--------------|---------------|
| Local, unary | ~50,000 | <1ms |
| Local, streaming | ~100,000 | <0.5ms |
| LAN, streaming | ~80,000 | ~2ms |
| WAN, streaming | ~10,000 | 20-100ms |

### Best Practices

1. **Use streaming** for continuous data flow
2. **Enable keepalive** for long-lived connections
3. **Batch small messages** to reduce RPC overhead
4. **Use compression** for WAN connections
5. **Implement backpressure** for slow consumers

## Kubernetes Deployment

### Service Definition

```yaml
apiVersion: v1
kind: Service
metadata:
  name: asterix-grpc
spec:
  selector:
    app: asterix
  ports:
    - port: 50051
      targetPort: 50051
      protocol: TCP
  type: ClusterIP
---
apiVersion: apps/v1
kind: Deployment
metadata:
  name: asterix
spec:
  replicas: 3
  selector:
    matchLabels:
      app: asterix
  template:
    metadata:
      labels:
        app: asterix
    spec:
      containers:
      - name: asterix
        image: asterix:latest
        args: ["-g", "SERVER:0.0.0.0:50051", "-j"]
        ports:
        - containerPort: 50051
```

### Istio Service Mesh

```yaml
apiVersion: networking.istio.io/v1alpha3
kind: DestinationRule
metadata:
  name: asterix-grpc
spec:
  host: asterix-grpc
  trafficPolicy:
    connectionPool:
      http:
        h2UpgradePolicy: UPGRADE
```

## Troubleshooting

### Connection Issues

**Problem:** "Failed to create gRPC channel"
```bash
# Test port connectivity
nc -zv localhost 50051

# Check server is running
grpcurl -plaintext localhost:50051 list
```

**Problem:** "Channel not connected"
```bash
# Enable gRPC debug logging
export GRPC_VERBOSITY=DEBUG
export GRPC_TRACE=all
asterix -g CLIENT:localhost:50051 -j
```

### TLS Issues

**Problem:** "SSL handshake failed"
```bash
# Verify certificate
openssl s_client -connect server:50051

# Check certificate chain
openssl verify -CAfile ca.pem cert.pem
```

### Performance Issues

**Problem:** High latency
```bash
# Enable compression
export GRPC_DEFAULT_COMPRESSION_ALGORITHM=gzip

# Increase buffer sizes
export GRPC_ARG_HTTP2_BDP_PROBE=1
```

## Health Checking

### gRPC Health Protocol

The server implements the standard gRPC health checking protocol:

```bash
# Check health status
grpcurl -plaintext localhost:50051 grpc.health.v1.Health/Check
```

### Prometheus Metrics

Enable metrics endpoint for monitoring:

```bash
# Metrics available at :9090/metrics
asterix -g SERVER:0.0.0.0:50051 --metrics-port 9090 -j
```

## See Also

- [ZeroMQ Transport Guide](ZEROMQ_GUIDE.md) - Low-latency messaging
- [MQTT Transport Guide](MQTT_GUIDE.md) - IoT messaging
- [Network Setup Guide](NETWORK_SETUP.md) - UDP multicast configuration
