# Proposal: Add CAN Bus SocketCAN Support to C++ CLI

## Change ID
`add-can-socketcan-support`

## Related Issues
- GitHub Issue #39: Add CAN Bus support via SocketCAN for automotive/embedded systems

## Summary

Add CAN Bus transport support to the ASTERIX C++ CLI tool via Linux SocketCAN API, enabling ASTERIX data reception/transmission over Controller Area Network interfaces commonly used in automotive and embedded systems.

## Motivation

CAN Bus enables:
- **Automotive integration** - ASTERIX over CAN networks in vehicles
- **Embedded systems** - Real-time data on resource-constrained devices
- **Industrial control** - Process automation with ASTERIX surveillance data
- **SocketCAN** - Standard Linux kernel CAN interface

The Rust crate (`asterix-rs/src/transport/can.rs`) already has complete CAN support with 669 lines of production code and 559 lines of tests. This proposal brings parity to the C++ CLI.

## Scope

### In Scope
- CCanDevice class implementing CBaseDevice interface
- SocketCAN socket creation and frame handling
- Frame fragmentation for large ASTERIX messages (matching Rust protocol)
- Frame reassembly with timeout handling
- CLI option `-c`/`--can` for CAN input
- CMake option `ENABLE_SOCKETCAN`
- Linux-only implementation (guarded by `#ifndef _WIN32`)

### Out of Scope
- Windows CAN support (no native SocketCAN on Windows)
- macOS CAN support (SocketCAN is Linux-only)
- CAN Bus output/publishing (focus on receiving ASTERIX data)
- CAN FD extended mode (can be added later)

## Technical Approach

Follow existing device patterns:
- Pattern: CSerialDevice (Linux-only), CZeromqDevice (feature-gated)
- Factory registration in devicefactory.cxx
- Descriptor format: `interface[:fd[:timeout]]`

### Fragmentation Protocol (must match Rust exactly)
```
CAN ID (11-bit standard):
  Bits [10:8]: Category high 3 bits (category >> 5) & 0x07
  Bits [7:0]:  Fragment sequence number (0-255)

Fragment Header Byte (first byte of CAN payload):
  Bit 7:      Is last fragment flag (0x80)
  Bits [6:0]: Fragment index (0-127)

Payload sizes:
  Classic CAN: 7 bytes (8 total - 1 header)
  CAN FD:     63 bytes (64 total - 1 header)
```

## Testing Notes

### WSL2 Limitation
Virtual CAN (vcan) is NOT available in standard WSL2 kernel:
```
modprobe: FATAL: Module vcan not found in directory /lib/modules/<kernel-version>-microsoft-standard-WSL2
```

### Testing Alternatives
1. **Native Linux VM** - Use VirtualBox/VMware with full Linux kernel
2. **Docker with --privileged** - May work with custom kernel modules
3. **Real CAN hardware** - USB-to-CAN adapters (e.g., Peak PCAN-USB)
4. **GitHub Actions** - CI runners have full Linux kernel with vcan support
5. **Custom WSL2 kernel** - Rebuild WSL2 kernel with CAN support enabled

### Virtual CAN Setup (Native Linux)
```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

## Risks

| Risk | Mitigation |
|------|------------|
| WSL2 testing limitation | Use GitHub Actions CI for vcan tests |
| Protocol mismatch with Rust | Use Rust can.rs as authoritative reference |
| Memory leaks in reassembly | Timeout cleanup + valgrind testing |

## Success Criteria

- [ ] CCanDevice compiles with `-DENABLE_SOCKETCAN=ON`
- [ ] CLI accepts `-c vcan0` input option
- [ ] Fragment reassembly matches Rust implementation
- [ ] CI tests pass with virtual CAN (GitHub Actions)
- [ ] Valgrind shows 0 memory leaks
- [ ] Documentation updated with CAN usage examples
