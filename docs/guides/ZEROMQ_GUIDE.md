# ZeroMQ Transport Guide

Complete guide to using the ASTERIX decoder with ZeroMQ for low-latency messaging.

## Overview

ZeroMQ integration provides high-performance messaging transport for ASTERIX data distribution:

- **Low latency** - Microsecond message passing
- **Flexible patterns** - PUB/SUB, PUSH/PULL
- **No broker** - Direct peer-to-peer messaging
- **Cross-platform** - Works on Linux, Windows, macOS

## Building with ZeroMQ Support

ZeroMQ support is optional and disabled by default. Enable it at build time:

```bash
# Install ZeroMQ development libraries
# Ubuntu/Debian
sudo apt-get install libzmq3-dev

# Fedora/RHEL
sudo dnf install zeromq-devel

# macOS
brew install zeromq

# Build with ZeroMQ enabled
cmake -B build -DENABLE_ZEROMQ=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build
```

Verify ZeroMQ support is enabled:

```bash
asterix --help | grep -i zmq
```

## Command-Line Usage

### Basic Subscriber

Connect to a ZeroMQ publisher and receive ASTERIX data:

```bash
# Subscribe to remote publisher
asterix -z SUB:tcp://192.168.1.10:5555 -j

# Subscribe to local IPC socket
asterix -z SUB:ipc:///tmp/asterix.sock -j

# Subscribe to inproc socket (same process)
asterix -z SUB:inproc://asterix -j
```

### Bind Mode (Server)

Bind a PULL socket to receive connections:

```bash
# Bind PULL socket on all interfaces
asterix -z PULL:tcp://*:5556:bind -j

# Bind on specific interface
asterix -z PULL:tcp://10.0.0.1:5556:bind -j
```

### Input/Output Format Combinations

```bash
# Receive from ZeroMQ, output JSON
asterix -z SUB:tcp://192.168.1.10:5555 -j

# Receive from ZeroMQ, output line format
asterix -z SUB:tcp://192.168.1.10:5555 -l

# Receive from ZeroMQ, output XML
asterix -z SUB:tcp://192.168.1.10:5555 -x

# Receive from ZeroMQ, output extensive JSON
asterix -z SUB:tcp://192.168.1.10:5555 -je
```

## Socket Types

### SUB (Subscriber)

Subscribe to messages from a PUB socket:

```bash
# Connect to publisher
asterix -z SUB:tcp://publisher.example.com:5555 -j
```

- **Default mode:** Connect to remote endpoint
- **Use case:** Multiple subscribers receiving from one publisher
- **Pattern:** PUB/SUB (one-to-many)

### PULL

Pull messages from a PUSH socket:

```bash
# Bind to receive PUSH messages
asterix -z PULL:tcp://*:5556:bind -j

# Connect to PUSH server
asterix -z PULL:tcp://sender.example.com:5556 -j
```

- **Use case:** Load balancing, work distribution
- **Pattern:** PUSH/PULL (pipeline)

## Endpoint Formats

### TCP

```bash
# Connect to remote host
tcp://hostname:port
tcp://192.168.1.10:5555

# Bind to all interfaces
tcp://*:5555

# Bind to specific interface
tcp://10.0.0.1:5555
```

### IPC (Inter-Process Communication)

```bash
# Unix socket (Linux/macOS)
ipc:///tmp/asterix.sock
ipc:///var/run/asterix.ipc
```

### Inproc (In-Process)

```bash
# Same-process communication
inproc://asterix
inproc://channel1
```

## Architecture Examples

### Simple PUB/SUB

```
+-------------+          +-------------+
|   Radar     |   TCP    |   ASTERIX   |
|   Encoder   | -------> |   Decoder   |
| (ZMQ PUB)   |          | (ZMQ SUB)   |
+-------------+          +-------------+
```

```bash
# Decoder command
asterix -z SUB:tcp://radar-encoder:5555 -j
```

### Multi-Subscriber

```
                          +-------------+
                     /--> |  Decoder 1  |
+-------------+     /     +-------------+
|   Radar     |    /
|   Encoder   | ---+      +-------------+
| (ZMQ PUB)   |    \----> |  Decoder 2  |
+-------------+     \     +-------------+
                     \
                      \   +-------------+
                       `->|  Decoder 3  |
                          +-------------+
```

All subscribers receive all messages (one-to-many).

### Pipeline (Load Balancing)

```
                          +-------------+
                     /--> |  Worker 1   |
+-------------+     /     +-------------+
|   Source    |    /
| (ZMQ PUSH)  | ---+      +-------------+
+-------------+    \----> |  Worker 2   |
                    \     +-------------+
                     \
                      \   +-------------+
                       `->|  Worker 3   |
                          +-------------+
```

```bash
# Each worker pulls work items
asterix -z PULL:tcp://source:5557 -j
```

Messages are distributed round-robin (load balanced).

## Integration with Other Systems

### Python Publisher Example

```python
import zmq

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://*:5555")

# Send ASTERIX data
asterix_data = b'\x30\x00\x2a...'  # CAT 48 record
socket.send(asterix_data)
```

### Rust Publisher Example

```rust
use zmq::Context;

fn main() {
    let ctx = Context::new();
    let socket = ctx.socket(zmq::PUB).unwrap();
    socket.bind("tcp://*:5555").unwrap();

    // Send ASTERIX data
    let asterix_data = vec![0x30, 0x00, 0x2a, /* ... */];
    socket.send(&asterix_data, 0).unwrap();
}
```

### C++ Publisher Example

```cpp
#include <zmq.h>
#include <string.h>

int main() {
    void *ctx = zmq_ctx_new();
    void *socket = zmq_socket(ctx, ZMQ_PUB);
    zmq_bind(socket, "tcp://*:5555");

    // Send ASTERIX data
    unsigned char data[] = {0x30, 0x00, 0x2a, /* ... */};
    zmq_send(socket, data, sizeof(data), 0);

    zmq_close(socket);
    zmq_ctx_destroy(ctx);
    return 0;
}
```

## Performance Considerations

### High-Throughput Settings

For high-volume ASTERIX data streams:

1. **Increase receive buffer:**
   ```bash
   # Set system-wide limits
   sudo sysctl -w net.core.rmem_max=26214400
   sudo sysctl -w net.core.rmem_default=26214400
   ```

2. **Use appropriate output format:**
   - `-j` (compact JSON) - fastest for parsing
   - `-l` (line) - fast, easy to parse
   - Avoid `-jh` or `-xh` for high throughput

3. **Use TCP for reliability, IPC for lowest latency**

### Throughput Expectations

| Configuration | Expected Throughput |
|--------------|---------------------|
| TCP local | 500,000+ msg/sec |
| TCP network | 100,000+ msg/sec |
| IPC | 1,000,000+ msg/sec |
| inproc | 2,000,000+ msg/sec |

## Troubleshooting

### "ZeroMQ support not compiled in"

Rebuild with ZeroMQ enabled:

```bash
cmake -B build -DENABLE_ZEROMQ=ON
cmake --build build --parallel
```

### Connection refused

Check that the publisher is running and the endpoint is correct:

```bash
# Verify with netstat/ss
ss -tlnp | grep 5555

# Test with zmq tools
# Install: pip install pyzmq
python -c "import zmq; c=zmq.Context(); s=c.socket(zmq.SUB); s.connect('tcp://localhost:5555'); s.subscribe(''); print(s.recv())"
```

### No data received

1. **Check endpoint format** - Must include transport prefix (`tcp://`, `ipc://`)
2. **Check socket type match** - SUB connects to PUB, PULL connects to PUSH
3. **Check bind vs connect** - One side binds, other connects
4. **Check firewall** - Ensure port is open

### High latency

1. **Use TCP_NODELAY** - ZeroMQ enables by default
2. **Check network** - Use `ping` and `traceroute`
3. **Use IPC** - If processes are on same machine, IPC is faster

## Related Documentation

- [C++ User Guide](CPP_GUIDE.md) - General command-line usage
- [Architecture Overview](../ARCHITECTURE.md) - System design
- [Troubleshooting Guide](../TROUBLESHOOTING.md) - General troubleshooting
- [ZeroMQ Documentation](https://zeromq.org/get-started/) - Official ZeroMQ docs

## Support

- **Issues**: https://github.com/montge/asterix/issues
- **Source**: `src/engine/zeromqdevice.cxx`
