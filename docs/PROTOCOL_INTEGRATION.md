# Protocol Integration Guide

**Version:** 1.0
**Last Updated:** 2025-11-06
**Status:** Active Development Guidelines

---

## Table of Contents

1. [Overview](#overview)
2. [Architecture: Where to Add Code](#architecture-where-to-add-code)
3. [Adding Input Format Handlers](#adding-input-format-handlers)
4. [Adding Output Formatters](#adding-output-formatters)
5. [Testing Requirements](#testing-requirements)
6. [Performance Considerations](#performance-considerations)
7. [Security Considerations](#security-considerations)
8. [Reference Implementations](#reference-implementations)

---

## Overview

This document provides guidelines for integrating new protocol adapters (input formats and output formatters) into the ASTERIX decoder.

**Current Input Formats:**
- Raw ASTERIX binary (`asterixrawsubformat`)
- PCAP (`asterixpcapsubformat`) - With buffer optimization
- HDLC (`asterixhdlcsubformat`) - HDLC framing
- FINAL (`asterixfinalsubformat`) - FINAL packet format
- GPS (`asterixgpssubformat`) - GPS packet format

**Current Output Formats:**
- Text (human-readable, line mode)
- JSON (compact, human-readable, extensive)
- XML (compact, human-readable)

**Planned Protocols:**
- RTP (Real-time Transport Protocol)
- WebSocket (real-time streaming)
- gRPC (protocol buffers)
- MQTT (message queuing)
- ZeroMQ (distributed messaging)

---

## Architecture: Where to Add Code

### Layer Separation

The ASTERIX decoder follows a **three-layer architecture**. Protocol adapters belong in different layers:

```
┌─────────────────────────────────────────────────────────────┐
│                   APPLICATION LAYER                         │
│  - CLI flags and argument parsing                           │
│  - User-facing output formatters                            │
│  - Language binding APIs                                    │
│                                                             │
│  Location: src/main/asterix.cpp                             │
│  Add: Output format selection logic                         │
└─────────────────────────┬───────────────────────────────────┘
                          │
┌─────────────────────────┴───────────────────────────────────┐
│                   ASTERIX LAYER                             │
│  - Protocol-specific format handlers                        │
│  - Input format parsers (PCAP, HDLC, FINAL, GPS)            │
│  - Output formatters (Text, JSON, XML)                      │
│  - ASTERIX data structures                                  │
│                                                             │
│  Location: src/asterix/*subformat*.{cxx,hxx}                │
│  Add: New protocol adapters HERE                            │
└─────────────────────────┬───────────────────────────────────┘
                          │
┌─────────────────────────┴───────────────────────────────────┐
│                   ENGINE LAYER                              │
│  - Generic device abstraction (file, network, serial)       │
│  - Transport-layer protocols (TCP, UDP, multicast)          │
│  - Base format/codec framework                              │
│                                                             │
│  Location: src/engine/                                      │
│  Add: New transport mechanisms (if needed)                  │
└─────────────────────────────────────────────────────────────┘
```

### Decision Matrix: Where to Add Code?

| Protocol Type | Layer | Example Files |
|---------------|-------|---------------|
| **Encapsulation format** (wraps ASTERIX) | ASTERIX Layer | `asterixpcapsubformat.{cxx,hxx}` |
| **Output format** (serialize parsed data) | ASTERIX Layer | `DataBlock::getText()`, `toJSON()` |
| **Transport mechanism** (network/file I/O) | Engine Layer | `UdpDevice.cpp`, `FileDevice.cpp` |
| **CLI flags** (user interface) | Application Layer | `src/main/asterix.cpp` |

**Example Decision Tree:**

```
Is it an ASTERIX encapsulation format (PCAP, HDLC, RTP)?
├─ YES → ASTERIX Layer (src/asterix/*subformat*)
└─ NO  → Is it a transport mechanism (TCP, WebSocket)?
         ├─ YES → Engine Layer (src/engine/*Device*)
         └─ NO  → Is it an output format (JSON, Protobuf)?
                  ├─ YES → ASTERIX Layer (DataBlock::to*)
                  └─ NO  → Application Layer (CLI)
```

---

## Adding Input Format Handlers

Input format handlers extract ASTERIX payload from encapsulation protocols.

### Step 1: Create Handler Files

**Naming Convention:** `asterix<protocol>subformat.{cxx,hxx}`

**Example:** Adding RTP encapsulation

```bash
cd src/asterix
touch asterixrtpsubformat.hxx
touch asterixrtpsubformat.cxx
```

### Step 2: Define Header Interface

**File:** `src/asterix/asterixrtpsubformat.hxx`

```cpp
#ifndef ASTERIXRTPSUBFORMAT_HXX__
#define ASTERIXRTPSUBFORMAT_HXX__

class CBaseDevice;
class CBaseFormatDescriptor;

/**
 * @class CAsterixRtpSubformat
 * @brief ASTERIX RTP (Real-time Transport Protocol) sub-format
 *
 * Extracts ASTERIX payload from RTP packets (RFC 3550).
 * RTP header format:
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|X|  CC   |M|     PT      |       sequence number         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                           timestamp                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           synchronization source (SSRC) identifier            |
 * +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * |            contributing source (CSRC) identifiers             |
 * |                             ....                              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    ASTERIX payload (variable)                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
class CAsterixRtpSubformat {
public:
    /**
     * @brief Read and parse RTP packet, extract ASTERIX payload
     * @param formatDescriptor Format descriptor context
     * @param device Input device
     * @param discard Output flag - set to true if packet should be discarded
     * @return true on success, false on error
     */
    static bool ReadPacket(
        CBaseFormatDescriptor& formatDescriptor,
        CBaseDevice& device,
        bool& discard
    );

    /**
     * @brief Write ASTERIX data wrapped in RTP packet
     * @param formatDescriptor Format descriptor context
     * @param device Output device
     * @param discard Output flag - set to true if packet should be discarded
     * @return true on success, false on error
     */
    static bool WritePacket(
        CBaseFormatDescriptor& formatDescriptor,
        CBaseDevice& device,
        bool& discard
    );

    /**
     * @brief Process RTP packet (parse ASTERIX payload)
     * @param formatDescriptor Format descriptor context
     * @param device Device context
     * @param discard Output flag - set to true if packet should be discarded
     * @return true on success, false on error
     */
    static bool ProcessPacket(
        CBaseFormatDescriptor& formatDescriptor,
        CBaseDevice& device,
        bool& discard
    );

    /**
     * @brief Send heartbeat/keepalive packet
     * @param formatDescriptor Format descriptor context
     * @param device Device context
     * @return true on success, false on error
     */
    static bool Heartbeat(
        CBaseFormatDescriptor& formatDescriptor,
        CBaseDevice& device
    );

private:
    // RTP header structure (RFC 3550)
    typedef struct rtp_hdr_s {
        unsigned char  vpxcc;      // V(2) + P(1) + X(1) + CC(4)
        unsigned char  mpt;        // M(1) + PT(7)
        unsigned short seq_num;    // Sequence number
        unsigned int   timestamp;  // Timestamp
        unsigned int   ssrc;       // SSRC identifier
        // CSRC list follows (if CC > 0)
    } rtp_hdr_t;

    // Minimum RTP header size (no CSRC)
    static constexpr size_t RTP_MIN_HEADER_SIZE = 12;

    // Maximum CSRC count
    static constexpr size_t RTP_MAX_CSRC = 15;

    /**
     * @brief Get RTP header size (accounts for CSRC list)
     * @param header RTP header pointer
     * @return Total header size in bytes
     */
    static size_t GetHeaderSize(const rtp_hdr_t* header);

    /**
     * @brief Validate RTP header
     * @param header RTP header pointer
     * @param buffer_size Available buffer size
     * @return true if valid, false otherwise
     */
    static bool ValidateHeader(const rtp_hdr_t* header, size_t buffer_size);
};

#endif // ASTERIXRTPSUBFORMAT_HXX__
```

### Step 3: Implement Handler Logic

**File:** `src/asterix/asterixrtpsubformat.cxx`

```cpp
#include "asterixrtpsubformat.hxx"
#include "asterix.h"
#include "InputParser.h"
#include "AsterixData.h"
#include <cstring>

// Get RTP header size (12 bytes + 4*CC bytes for CSRC list)
size_t CAsterixRtpSubformat::GetHeaderSize(const rtp_hdr_t* header) {
    unsigned char cc = header->vpxcc & 0x0F;  // Extract CC field
    return RTP_MIN_HEADER_SIZE + (cc * 4);
}

// Validate RTP header
bool CAsterixRtpSubformat::ValidateHeader(const rtp_hdr_t* header, size_t buffer_size) {
    // Check buffer is large enough for minimum header
    if (buffer_size < RTP_MIN_HEADER_SIZE) {
        LOGERROR(1, "RTP header truncated: buffer size %zu < %zu",
                 buffer_size, RTP_MIN_HEADER_SIZE);
        return false;
    }

    // Check RTP version (must be 2)
    unsigned char version = (header->vpxcc >> 6) & 0x03;
    if (version != 2) {
        LOGERROR(1, "Invalid RTP version: %u (expected 2)", version);
        return false;
    }

    // Check CSRC count
    unsigned char cc = header->vpxcc & 0x0F;
    if (cc > RTP_MAX_CSRC) {
        LOGERROR(1, "Invalid CSRC count: %u (max %zu)", cc, RTP_MAX_CSRC);
        return false;
    }

    // Check buffer is large enough for full header
    size_t header_size = GetHeaderSize(header);
    if (buffer_size < header_size) {
        LOGERROR(1, "RTP header truncated: buffer size %zu < %zu",
                 buffer_size, header_size);
        return false;
    }

    return true;
}

bool CAsterixRtpSubformat::ReadPacket(
    CBaseFormatDescriptor& formatDescriptor,
    CBaseDevice& device,
    bool& discard)
{
    discard = false;

    // Read packet from device
    unsigned char buffer[65536];  // Max UDP packet size
    size_t bytes_read = 0;

    if (!device.Read(buffer, sizeof(buffer), bytes_read)) {
        LOGERROR(1, "Failed to read RTP packet from device");
        return false;
    }

    if (bytes_read == 0) {
        discard = true;
        return true;  // No data available
    }

    // Validate RTP header
    const rtp_hdr_t* rtp_header = reinterpret_cast<const rtp_hdr_t*>(buffer);

    if (!ValidateHeader(rtp_header, bytes_read)) {
        LOGERROR(1, "Invalid RTP header, discarding packet");
        discard = true;
        return true;
    }

    // Extract ASTERIX payload
    size_t header_size = GetHeaderSize(rtp_header);
    size_t payload_size = bytes_read - header_size;

    if (payload_size == 0) {
        LOGDEBUG(1, "RTP packet has no payload, discarding");
        discard = true;
        return true;
    }

    const unsigned char* asterix_data = buffer + header_size;

    // Store payload in format descriptor for processing
    // (Implementation depends on your descriptor structure)
    // formatDescriptor.SetData(asterix_data, payload_size);

    LOGDEBUG(1, "RTP packet: seq=%u, timestamp=%u, payload=%zu bytes",
             ntohs(rtp_header->seq_num),
             ntohl(rtp_header->timestamp),
             payload_size);

    return true;
}

bool CAsterixRtpSubformat::ProcessPacket(
    CBaseFormatDescriptor& formatDescriptor,
    CBaseDevice& device,
    bool& discard)
{
    // Process ASTERIX payload using InputParser
    // (This is typically handled by the framework after ReadPacket)
    return true;
}

bool CAsterixRtpSubformat::WritePacket(
    CBaseFormatDescriptor& formatDescriptor,
    CBaseDevice& device,
    bool& discard)
{
    // TODO: Implement RTP packet writing
    // 1. Build RTP header
    // 2. Append ASTERIX payload
    // 3. Write to device
    return false;
}

bool CAsterixRtpSubformat::Heartbeat(
    CBaseFormatDescriptor& formatDescriptor,
    CBaseDevice& device)
{
    // RTP doesn't require heartbeat (uses RTCP for that)
    return true;
}
```

### Step 4: Integrate with Build System

**File:** `src/asterix/CMakeLists.txt`

Add the new source files:

```cmake
set(ASTERIX_SOURCES
    # ... existing sources ...
    asterixrtpsubformat.cxx
    asterixrtpsubformat.hxx
)
```

### Step 5: Add CLI Flag

**File:** `src/main/asterix.cpp`

Add command-line option:

```cpp
// Add to argument parsing section
else if (!strcmp(argv[nArgPos], "-T") || !strcmp(argv[nArgPos], "--rtp"))
{
    // Enable RTP format
    use_rtp = true;
}

// Add to usage text
printf("  -T, --rtp           Use RTP encapsulation\n");

// Apply format in initialization
if (use_rtp) {
    // Set RTP format handler
    // (Implementation depends on your framework)
}
```

### Step 6: Update Documentation

**Files to Update:**
- `CLAUDE.md` - Add to Input Formats table
- `docs/ARCHITECTURE.md` - Add to format handlers list
- `README.md` - Add CLI flag documentation

---

## Adding Output Formatters

Output formatters serialize parsed ASTERIX data to various formats.

### Step 1: Add Formatter Method

Output formatters are methods on the `DataBlock` class:

**File:** `src/asterix/DataBlock.h`

```cpp
class DataBlock {
public:
    // ... existing methods ...

    /**
     * @brief Export data block to Protocol Buffers format
     * @param formatType Output format type flags
     * @return Protocol Buffers serialized string
     */
    std::string toProtobuf(unsigned int formatType) const;

    /**
     * @brief Export data block to MessagePack format
     * @param formatType Output format type flags
     * @return MessagePack binary data
     */
    std::string toMessagePack(unsigned int formatType) const;
};
```

### Step 2: Implement Formatter

**File:** `src/asterix/DataBlock.cpp`

```cpp
#include "DataBlock.h"
#include "DataRecord.h"
#include <google/protobuf/message.h>  // Example for Protobuf
#include <msgpack.hpp>  // Example for MessagePack

std::string DataBlock::toProtobuf(unsigned int formatType) const {
    // Create Protobuf message (requires .proto definition)
    // This is a simplified example - you'll need to define the schema

    AsterixBlockProto proto_msg;

    // Set metadata
    proto_msg.set_category(m_nCategory);
    proto_msg.set_length(m_nLength);
    proto_msg.set_timestamp_ms(m_nTimestamp);

    // Add data records
    for (const auto& record : m_lDataRecords) {
        auto* proto_record = proto_msg.add_records();

        // Serialize each data item
        for (const auto& item : record->m_lDataItems) {
            auto* proto_item = proto_record->add_items();
            proto_item->set_id(item->m_strID);
            proto_item->set_value(item->toString());
        }
    }

    // Serialize to string
    std::string serialized;
    proto_msg.SerializeToString(&serialized);

    return serialized;
}

std::string DataBlock::toMessagePack(unsigned int formatType) const {
    msgpack::sbuffer buffer;
    msgpack::packer<msgpack::sbuffer> packer(&buffer);

    // Pack as map
    packer.pack_map(4);

    // Category
    packer.pack("category");
    packer.pack(m_nCategory);

    // Length
    packer.pack("length");
    packer.pack(m_nLength);

    // Timestamp
    packer.pack("timestamp_ms");
    packer.pack(m_nTimestamp);

    // Records
    packer.pack("records");
    packer.pack_array(m_lDataRecords.size());

    for (const auto& record : m_lDataRecords) {
        packer.pack_map(1);
        packer.pack("items");
        packer.pack_array(record->m_lDataItems.size());

        for (const auto& item : record->m_lDataItems) {
            packer.pack_map(2);
            packer.pack("id");
            packer.pack(item->m_strID);
            packer.pack("value");
            packer.pack(item->toString());
        }
    }

    return std::string(buffer.data(), buffer.size());
}
```

### Step 3: Add CLI Flag

**File:** `src/main/asterix.cpp`

```cpp
// Add format flag
else if (!strcmp(argv[nArgPos], "-pb") || !strcmp(argv[nArgPos], "--protobuf"))
{
    output_format = FORMAT_PROTOBUF;
}
else if (!strcmp(argv[nArgPos], "-mp") || !strcmp(argv[nArgPos], "--msgpack"))
{
    output_format = FORMAT_MSGPACK;
}

// Use format in output
switch (output_format) {
    case FORMAT_PROTOBUF:
        output = dataBlock->toProtobuf(formatType);
        break;
    case FORMAT_MSGPACK:
        output = dataBlock->toMessagePack(formatType);
        break;
    // ... other formats
}
```

### Step 4: Update Language Bindings

Add output format support to Python/Rust bindings:

**Python Example:**
```python
def to_protobuf(record: Dict) -> bytes:
    """Convert record to Protocol Buffers format"""
    # Call C++ formatter via FFI
    return _asterix.record_to_protobuf(record)
```

**Rust Example:**
```rust
impl AsterixRecord {
    pub fn to_protobuf(&self) -> Result<Vec<u8>> {
        // Call C++ formatter via CXX bridge
        unsafe { ffi::asterix_record_to_protobuf(self) }
    }
}
```

---

## Testing Requirements

### 1. Unit Tests

Test format handler logic in isolation:

```cpp
// tests/test_rtp_subformat.cpp
#include <gtest/gtest.h>
#include "asterixrtpsubformat.hxx"

TEST(RtpSubformat, ValidHeader) {
    // Valid RTP header (version 2, no CSRC)
    unsigned char packet[] = {
        0x80, 0x60,  // V=2, P=0, X=0, CC=0, M=0, PT=96
        0x12, 0x34,  // Sequence number
        0x00, 0x00, 0x00, 0x01,  // Timestamp
        0xAB, 0xCD, 0xEF, 0x12,  // SSRC
        // ASTERIX payload follows
    };

    const auto* header = reinterpret_cast<const CAsterixRtpSubformat::rtp_hdr_t*>(packet);

    EXPECT_TRUE(CAsterixRtpSubformat::ValidateHeader(header, sizeof(packet)));
    EXPECT_EQ(CAsterixRtpSubformat::GetHeaderSize(header), 12);
}

TEST(RtpSubformat, InvalidVersion) {
    // Invalid RTP version (V=1)
    unsigned char packet[] = {
        0x40, 0x60,  // V=1 (INVALID), P=0, X=0, CC=0, M=0, PT=96
        0x12, 0x34,
        0x00, 0x00, 0x00, 0x01,
        0xAB, 0xCD, 0xEF, 0x12,
    };

    const auto* header = reinterpret_cast<const CAsterixRtpSubformat::rtp_hdr_t*>(packet);

    EXPECT_FALSE(CAsterixRtpSubformat::ValidateHeader(header, sizeof(packet)));
}

TEST(RtpSubformat, TruncatedHeader) {
    unsigned char packet[] = {
        0x80, 0x60,  // Only 2 bytes (truncated)
    };

    const auto* header = reinterpret_cast<const CAsterixRtpSubformat::rtp_hdr_t*>(packet);

    EXPECT_FALSE(CAsterixRtpSubformat::ValidateHeader(header, sizeof(packet)));
}
```

### 2. Integration Tests

Test with real protocol data:

```bash
# tests/test_rtp_integration.sh
#!/bin/bash

# Test RTP encapsulated ASTERIX data
./install/asterix -T -f tests/data/asterix_rtp.pcap -j > output.json

# Validate output
if [ -s output.json ]; then
    echo "PASS: RTP parser produced output"
else
    echo "FAIL: RTP parser produced no output"
    exit 1
fi

# Check parsed data is valid JSON
if jq empty output.json 2>/dev/null; then
    echo "PASS: Output is valid JSON"
else
    echo "FAIL: Output is not valid JSON"
    exit 1
fi
```

### 3. Performance Tests

Measure parsing throughput:

```cpp
// benches/bench_rtp.cpp
#include <benchmark/benchmark.h>
#include "asterixrtpsubformat.hxx"

static void BM_RtpParsing(benchmark::State& state) {
    // Load test data
    std::vector<unsigned char> rtp_data = load_test_data("rtp_sample.bin");

    for (auto _ : state) {
        // Parse RTP packet
        CAsterixRtpSubformat::ReadPacket(/* ... */);
    }

    state.SetBytesProcessed(state.iterations() * rtp_data.size());
}

BENCHMARK(BM_RtpParsing);
BENCHMARK_MAIN();
```

**Run benchmarks:**
```bash
cd build
cmake --build . --target bench_rtp
./bench_rtp --benchmark_format=json > rtp_bench.json
```

### 4. Fuzz Testing

Test with malformed protocol data:

```cpp
// fuzz/fuzz_rtp.cpp
#include <stdint.h>
#include <stddef.h>
#include "asterixrtpsubformat.hxx"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Fuzz RTP parser with random input
    if (size < 12) return 0;  // Too small for RTP header

    const auto* header = reinterpret_cast<const CAsterixRtpSubformat::rtp_hdr_t*>(data);

    // Should not crash or leak memory
    CAsterixRtpSubformat::ValidateHeader(header, size);

    return 0;
}
```

**Run fuzzer:**
```bash
clang++ -g -fsanitize=fuzzer,address fuzz_rtp.cpp -o fuzz_rtp
./fuzz_rtp -max_total_time=3600
```

---

## Performance Considerations

### 1. Buffer Reuse

Reuse buffers to avoid allocations (example from PCAP handler):

```cpp
// Performance Quick Win #5: Buffer reuse (15-20% speedup)
class CAsterixPcapSubformat {
private:
    static std::vector<unsigned char> s_packetBuffer;  // Reusable buffer

public:
    static bool ReadPacket(/* ... */) {
        // Reuse buffer instead of allocating each time
        if (s_packetBuffer.size() < required_size) {
            s_packetBuffer.resize(required_size);
        }

        // Read into reused buffer
        device.Read(s_packetBuffer.data(), required_size, bytes_read);
    }
};

// Initialize static buffer
std::vector<unsigned char> CAsterixPcapSubformat::s_packetBuffer(65536);
```

### 2. String Reserve

Reserve string capacity before concatenation:

```cpp
// Performance Quick Win #1/#2: String reserve (15% speedup each)
std::string buildOutput() {
    std::string result;
    result.reserve(1024);  // Pre-allocate capacity

    result += "Header: ";
    result += data1;
    result += ", Payload: ";
    result += data2;

    return result;
}
```

### 3. Avoid Unnecessary Copies

Use `std::string_view` for read-only string operations:

```cpp
// BAD: Copies string
void processField(std::string field) {
    if (field == "010") { /* ... */ }
}

// GOOD: Uses view (no copy)
void processField(std::string_view field) {
    if (field == "010") { /* ... */ }
}
```

### 4. Minimize Memory Allocations

Preallocate data structures:

```cpp
class ProtocolHandler {
private:
    std::vector<DataRecord> m_records;

public:
    void parsePackets(size_t expected_count) {
        // Reserve space upfront
        m_records.clear();
        m_records.reserve(expected_count);

        // Parse without reallocation
        for (size_t i = 0; i < expected_count; i++) {
            m_records.emplace_back(parseRecord());
        }
    }
};
```

### 5. Benchmark Before Optimizing

Always measure before optimizing:

```bash
# Baseline
./install/asterix -f large_file.pcap -j > /dev/null
time: 2.34s

# After optimization
./install/asterix -f large_file.pcap -j > /dev/null
time: 1.87s

# Speedup: 25%
```

---

## Security Considerations

### 1. Input Validation

Validate ALL protocol fields:

```cpp
bool ValidateRtpHeader(const rtp_hdr_t* header, size_t buffer_size) {
    // Check buffer size
    if (buffer_size < RTP_MIN_HEADER_SIZE) {
        return false;
    }

    // Check RTP version
    unsigned char version = (header->vpxcc >> 6) & 0x03;
    if (version != 2) {
        return false;
    }

    // Check CSRC count (prevent integer overflow)
    unsigned char cc = header->vpxcc & 0x0F;
    if (cc > RTP_MAX_CSRC) {
        return false;
    }

    // Check total header size (prevent buffer overflow)
    size_t header_size = RTP_MIN_HEADER_SIZE + (cc * 4);
    if (header_size > buffer_size) {
        return false;
    }

    // Validate payload size (prevent underflow)
    size_t payload_size = buffer_size - header_size;
    if (payload_size > buffer_size) {  // Underflow check
        return false;
    }

    return true;
}
```

### 2. Bounds Checking

Always check array/buffer bounds:

```cpp
// BAD: No bounds checking
unsigned char* payload = buffer + header_size;
unsigned char first_byte = payload[0];  // May be out of bounds!

// GOOD: Explicit bounds check
if (buffer_size > header_size) {
    unsigned char* payload = buffer + header_size;
    size_t payload_size = buffer_size - header_size;

    if (payload_size > 0) {
        unsigned char first_byte = payload[0];  // Safe
    }
}
```

### 3. Integer Overflow Prevention

Use checked arithmetic:

```cpp
// BAD: Integer overflow possible
size_t total_size = header_size + payload_size;  // May overflow!
buffer.resize(total_size);

// GOOD: Overflow check
if (header_size > SIZE_MAX - payload_size) {
    LOGERROR(1, "Integer overflow in size calculation");
    return false;
}
size_t total_size = header_size + payload_size;  // Safe
```

### 4. Endianness Handling

Handle byte order correctly:

```cpp
// Network byte order (big-endian) to host byte order
uint16_t seq_num = ntohs(rtp_header->seq_num);
uint32_t timestamp = ntohl(rtp_header->timestamp);

// Host byte order to network byte order
rtp_header->seq_num = htons(seq_num);
rtp_header->timestamp = htonl(timestamp);
```

### 5. Sanitize String Output

Prevent injection attacks in formatted output:

```cpp
// BAD: Unsanitized user input
printf("Processing packet from: %s\n", user_input);  // Injection risk!

// GOOD: Sanitize or use safe formatting
std::string sanitized = sanitize_string(user_input);
std::cout << "Processing packet from: " << sanitized << std::endl;
```

---

## Reference Implementations

### PCAP Format Handler

**Files:** `src/asterix/asterixpcapsubformat.{cxx,hxx}`

**Key Features:**
- PCAP global header parsing
- Per-packet header parsing
- Buffer reuse optimization (Quick Win #5)
- Ethernet/IP/UDP layer stripping
- Timestamp extraction

**Usage:**
```bash
./install/asterix -P -f sample.pcap -j
```

**Implementation Highlights:**
```cpp
// Buffer reuse pattern (15-20% speedup)
static unsigned char s_PacketBuffer[65536];

bool CAsterixPcapSubformat::ReadPacket(/* ... */) {
    // Read packet header
    pcaprec_hdr_t packet_header;
    device.Read(&packet_header, sizeof(packet_header), bytes_read);

    // Reuse static buffer (avoid allocation)
    size_t packet_size = packet_header.incl_len;
    if (packet_size > sizeof(s_PacketBuffer)) {
        LOGERROR(1, "Packet too large: %zu bytes", packet_size);
        return false;
    }

    device.Read(s_PacketBuffer, packet_size, bytes_read);

    // Strip Ethernet/IP/UDP headers, extract ASTERIX payload
    size_t header_size = 14 + 20 + 8;  // Ethernet + IP + UDP
    const unsigned char* asterix_data = s_PacketBuffer + header_size;
    size_t asterix_size = packet_size - header_size;

    // Process ASTERIX data
    return ProcessAsterixData(asterix_data, asterix_size);
}
```

### HDLC Format Handler

**Files:** `src/asterix/asterixhdlcsubformat.{cxx,hxx}`

**Key Features:**
- HDLC frame parsing (flag bytes, bit stuffing)
- CRC validation
- Frame synchronization

**HDLC Frame Structure:**
```
┌────┬─────────┬──────────────┬─────┬────┐
│Flag│ Address │   Payload    │ FCS │Flag│
│0x7E│ (1-2 B) │  (variable)  │(2 B)│0x7E│
└────┴─────────┴──────────────┴─────┴────┘
```

### FINAL Format Handler

**Files:** `src/asterix/asterixfinalsubformat.{cxx,hxx}`

**Key Features:**
- FINAL packet header parsing
- Multi-block support
- Timestamp handling

### GPS Format Handler

**Files:** `src/asterix/asterixgpssubformat.{cxx,hxx}`

**Key Features:**
- GPS packet header parsing
- GPS timestamp extraction
- Coordinate conversion

---

## Checklist for New Protocol Adapters

Use this checklist when adding a new protocol handler:

- [ ] **Design**
  - [ ] Determine layer (Engine/ASTERIX/Application)
  - [ ] Define header/payload structure
  - [ ] Document protocol specification (RFC/standard)
  - [ ] Design validation strategy

- [ ] **Implementation**
  - [ ] Create `asterix<protocol>subformat.{cxx,hxx}`
  - [ ] Implement `ReadPacket()` / `WritePacket()`
  - [ ] Implement `ValidateHeader()`
  - [ ] Add bounds checking (ALL buffer accesses)
  - [ ] Add integer overflow prevention
  - [ ] Handle endianness correctly

- [ ] **Build Integration**
  - [ ] Add to `src/asterix/CMakeLists.txt`
  - [ ] Add CLI flag to `src/main/asterix.cpp`
  - [ ] Update usage text

- [ ] **Testing**
  - [ ] Unit tests (header validation, edge cases)
  - [ ] Integration tests (real protocol data)
  - [ ] Performance benchmarks
  - [ ] Fuzz testing
  - [ ] Memory leak tests (Valgrind)

- [ ] **Security**
  - [ ] Static analysis (CodeQL)
  - [ ] ASAN/MSAN testing
  - [ ] Input validation audit
  - [ ] Sanitize all output

- [ ] **Documentation**
  - [ ] Update `CLAUDE.md` (Input Formats table)
  - [ ] Update `ARCHITECTURE.md` (protocol handlers list)
  - [ ] Update `README.md` (CLI flags)
  - [ ] Add protocol-specific README (if complex)

- [ ] **Performance**
  - [ ] Benchmark baseline
  - [ ] Optimize hot paths
  - [ ] Buffer reuse where applicable
  - [ ] Re-benchmark and document speedup

---

## Examples

### Example 1: Adding WebSocket Transport

**Layer:** Engine (transport mechanism)

**Files to Create:**
- `src/engine/WebSocketDevice.cpp`
- `src/engine/WebSocketDevice.h`

**Integration:**
```cpp
// src/engine/DeviceFactory.cpp
BaseDevice* DeviceFactory::CreateDevice(const std::string& type) {
    if (type == "websocket") {
        return new WebSocketDevice();
    }
    // ... other devices
}
```

### Example 2: Adding YAML Output

**Layer:** ASTERIX (output formatter)

**Files to Modify:**
- `src/asterix/DataBlock.h` (add `toYAML()`)
- `src/asterix/DataBlock.cpp` (implement `toYAML()`)
- `src/main/asterix.cpp` (add `-y, --yaml` flag)

**Implementation:**
```cpp
std::string DataBlock::toYAML(unsigned int formatType) const {
    std::ostringstream yaml;

    yaml << "category: " << (int)m_nCategory << "\n";
    yaml << "length: " << m_nLength << "\n";
    yaml << "timestamp_ms: " << m_nTimestamp << "\n";
    yaml << "records:\n";

    for (const auto& record : m_lDataRecords) {
        yaml << "  - items:\n";
        for (const auto& item : record->m_lDataItems) {
            yaml << "      " << item->m_strID << ": " << item->toString() << "\n";
        }
    }

    return yaml.str();
}
```

---

## References

- **ARCHITECTURE.md:** Overall system architecture
- **BINDING_GUIDELINES.md:** Creating language bindings
- **PERFORMANCE_OPTIMIZATIONS.md:** Performance best practices
- **CLAUDE.md:** Development guidelines

---

**Document Version:** 1.0
**Last Updated:** 2025-11-06
**Maintainers:** ASTERIX Contributors
**License:** GPL-3.0-or-later
