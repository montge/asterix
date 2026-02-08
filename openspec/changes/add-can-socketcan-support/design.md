# Design: CAN Bus SocketCAN Support

## Architecture Overview

CAN Bus support follows the existing Engine Layer device abstraction pattern, adding a new device type alongside TCP, UDP, serial, ZeroMQ, etc.

```
┌─────────────────────────────────────────────────────────┐
│                   Application Layer                      │
│                    (asterix.cpp)                        │
│                         │                               │
│              ┌──────────┴──────────┐                    │
│              │    -c vcan0         │                    │
│              └──────────┬──────────┘                    │
├─────────────────────────┼───────────────────────────────┤
│                   Engine Layer                           │
│                         │                               │
│              ┌──────────┴──────────┐                    │
│              │   CDeviceFactory    │                    │
│              └──────────┬──────────┘                    │
│                         │                               │
│    ┌────────┬───────┬───┴───┬────────┬────────┐        │
│    │        │       │       │        │        │        │
│  CTcp    CUdp   CSerial   CDisk   CZeromq  CCanDevice  │
│  Device  Device  Device   Device   Device   (NEW)      │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

## CCanDevice Class Design

### Inheritance
```cpp
class CCanDevice : public CBaseDevice
```

### Key Members
```cpp
private:
    int _socketDesc;                  // SocketCAN file descriptor
    std::string _interface;           // e.g., "vcan0", "can0"
    CanFrameType _frameType;          // Classic or FD
    unsigned int _reassemblyTimeoutMs; // Default 1000ms

    // Reassembly state per category
    std::map<uint8_t, ReassemblyState> _reassemblyStates;

    // Output buffer for reassembled messages
    std::vector<uint8_t> _outputBuffer;
```

### ReassemblyState Structure
```cpp
struct ReassemblyState {
    uint8_t category;
    std::map<uint8_t, std::vector<uint8_t>> fragments; // index -> data
    std::chrono::steady_clock::time_point lastUpdate;

    bool IsExpired(unsigned int timeoutMs) const;
    bool IsComplete() const;
    std::vector<uint8_t> Assemble() const;
};
```

## Fragmentation Protocol

### CAN ID Encoding (11-bit Standard ID)
```
Bits [10:8]: Category high 3 bits = (category >> 5) & 0x07
Bits [7:0]:  Fragment sequence number (0-255)

Example: Category 48 (0x30), fragment 5
  catHigh = (0x30 >> 5) & 0x07 = 1
  CAN ID = (1 << 8) | 5 = 0x105
```

**Known Limitation (inherited from Rust reference):** The CAN ID only encodes the HIGH 3 bits of the ASTERIX category. On reassembly, `parse_can_id()` recovers `cat_high << 5`, meaning categories sharing the same high 3 bits (e.g., CAT32-63 all map to cat_high=1) could collide in the reassembly key. The Rust implementation acknowledges this at `can.rs:624-626`: "For proper implementation, category should be in first fragment." The C++ implementation MUST replicate this same behavior for interoperability. Future enhancement: extract full category from the first fragment's ASTERIX data block header.

### Fragment Header Byte
```
Bit 7:      Is last fragment (0x80)
Bits [6:0]: Fragment index (0-127)

Examples:
  0x00 = Fragment 0, not last
  0x05 = Fragment 5, not last
  0x85 = Fragment 5, IS last
```

### Payload Layout
```
Classic CAN (8 bytes):
  [0]:     Fragment header
  [1..7]:  Payload (7 bytes)

CAN FD (64 bytes):
  [0]:     Fragment header
  [1..63]: Payload (63 bytes)
```

### Fragmentation Example
```
100-byte ASTERIX message in Classic CAN:
  Frame 0: ID=0x100, [0x00, data[0..6]]      (7 bytes)
  Frame 1: ID=0x101, [0x01, data[7..13]]     (7 bytes)
  ...
  Frame 14: ID=0x10E, [0x8E, data[98..99]]   (2 bytes, is_last)
```

## SocketCAN API Usage

### Socket Creation
```cpp
int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);

struct sockaddr_can addr;
addr.can_family = AF_CAN;
addr.can_ifindex = if_nametoindex(interface);

bind(sock, (struct sockaddr*)&addr, sizeof(addr));
```

### Frame Reception
```cpp
struct can_frame frame;
ssize_t nbytes = read(sock, &frame, sizeof(frame));

// Parse CAN ID
uint32_t canId = frame.can_id & CAN_SFF_MASK;  // 11-bit mask

// Parse fragment header
uint8_t header = frame.data[0];
uint8_t fragIndex = header & 0x7F;
bool isLast = (header & 0x80) != 0;

// Payload
uint8_t* payload = &frame.data[1];
size_t payloadLen = frame.can_dlc - 1;
```

### Frame Transmission
```cpp
struct can_frame frame;
frame.can_id = BuildCanId(category, fragIndex);
frame.can_dlc = payloadLen + 1;  // +1 for header
frame.data[0] = BuildFragmentHeader(fragIndex, isLast);
memcpy(&frame.data[1], payload, payloadLen);

write(sock, &frame, sizeof(frame));
```

## Configuration Parsing

Descriptor format: `interface[:fd[:timeout]]`

```cpp
CCanDevice::CCanDevice(CDescriptor &descriptor) {
    const char* interface = descriptor.GetFirst();
    const char* mode = descriptor.GetNext();      // Optional: "fd" or "classic"
    const char* timeout = descriptor.GetNext();   // Optional: milliseconds

    _interface = interface ? interface : "can0";
    _frameType = (mode && strcasecmp(mode, "fd") == 0)
                 ? CAN_TYPE_FD : CAN_TYPE_CLASSIC;
    _reassemblyTimeoutMs = timeout ? atoi(timeout) : 1000;

    Init(_interface.c_str(), _frameType == CAN_TYPE_FD);
}
```

## Write() Method - No-Op Stub

CAN output/publishing is out of scope (see proposal). However, the `CBaseDevice` interface requires a `Write()` implementation. The `Write()` method SHALL be a no-op stub returning false:

```cpp
bool CCanDevice::Write(const void *data, size_t len) {
    // CAN output not implemented - receive-only device
    // See proposal: CAN Bus output marked as out-of-scope
    return false;
}
```

This matches the pattern used by other receive-only scenarios in the codebase.

## CMake Header Detection

Unlike external library transports (ZeroMQ uses `find_package()`, MQTT uses `pkg_check_modules()`), CAN is a Linux kernel API with no external library. Use CMake `check_include_file()`:

```cmake
if(ENABLE_SOCKETCAN)
    if(NOT UNIX OR APPLE)
        message(WARNING "SocketCAN is Linux-only. Disabling ENABLE_SOCKETCAN.")
        set(ENABLE_SOCKETCAN OFF)
    else()
        include(CheckIncludeFile)
        check_include_file("linux/can.h" HAVE_CAN_H)
        if(HAVE_CAN_H)
            add_compile_definitions(HAVE_SOCKETCAN)
            message(STATUS "SocketCAN support: ON")
        else()
            message(WARNING "linux/can.h not found. Disabling SocketCAN.")
            set(ENABLE_SOCKETCAN OFF)
        endif()
    endif()
endif()
```

## Error Handling

Follow existing patterns:
```cpp
bool CCanDevice::Read(void *data, size_t *len) {
    if (_socketDesc < 0) {
        CountReadError();
        return false;
    }

    // ... read and reassemble ...

    if (success) {
        ResetReadErrors(true);
        return true;
    } else {
        CountReadError();
        return false;
    }
}
```

## Testing Approach

### Virtual CAN (Linux Native)
```bash
# Setup
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0

# Test
./asterix -c vcan0 -j &
cansend vcan0 100#0001020304050607  # Send test frame
```

### GitHub Actions CI
- Use Ubuntu runners (full kernel)
- Setup vcan in workflow step
- Run integration tests

### WSL2 Workaround
- Skip vcan tests on WSL2 (detect via kernel version string)
- Use GitHub Actions for full testing
- Document limitation in README

## Reference Implementation

The authoritative protocol specification is in:
- `asterix-rs/src/transport/can.rs` (669 lines)
- Key functions: `build_can_id()`, `parse_can_id()`, `build_fragment_header()`, `parse_fragment_header()`

All C++ implementation MUST match Rust behavior exactly for interoperability.
