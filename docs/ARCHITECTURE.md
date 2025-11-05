# ASTERIX Decoder Architecture

**Version:** 2.8.10
**Last Updated:** 2025-11-05
**Status:** Current system documentation + safety-critical design goals

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [Current Architecture](#current-architecture)
3. [Ideal Safety-Critical Architecture](#ideal-safety-critical-architecture)
4. [Data Flow](#data-flow)
5. [FFI Boundaries and Safety Patterns](#ffi-boundaries-and-safety-patterns)
6. [Language Bindings](#language-bindings)
7. [Configuration System](#configuration-system)
8. [Input/Output Format Handling](#inputoutput-format-handling)
9. [Performance Characteristics](#performance-characteristics)
10. [Migration Path](#migration-path)
11. [References](#references)

---

## Executive Summary

The ASTERIX decoder is a multi-layer, cross-platform system for parsing EUROCONTROL ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) protocol data. The system provides three language bindings (C++, Python, Rust) sharing a common C++ parsing core.

**Current State:**
- **Core Parser**: C++23 (Linux/macOS), C++20 (Windows/MSVC)
- **Language Bindings**: Python 3.10-3.14, Rust 1.87+
- **Categories**: 67 ASTERIX categories supported (100% working)
- **Safety Level**: Production-ready, but not formally safety-certified

**Strategic Direction:**
The project is transitioning toward a **safety-critical architecture** with:
- Validated FFI boundary layer for all language bindings
- Type-safe, memory-safe core (migrating to Rust for safety-critical core)
- DO-278A alignment for CNS/ATM software integrity
- Wrapper model: Safe core + validated bindings

---

## Current Architecture

### Multi-Layer Design

The codebase follows a **three-layer architecture**:

```
┌─────────────────────────────────────────────────────────────┐
│                   APPLICATION LAYER                         │
│  - CLI tools (asterix executable)                           │
│  - Language bindings (Python, Rust)                         │
│  - User-facing APIs                                         │
│                                                             │
│  Languages: C++23, Python 3.10-3.14, Rust 1.87+            │
│  Location: src/main/, asterix/, asterix-rs/                │
└─────────────────────────┬───────────────────────────────────┘
                          │
┌─────────────────────────┴───────────────────────────────────┐
│                   ASTERIX LAYER                             │
│  - Protocol-specific implementation                         │
│  - XML category parser                                      │
│  - Data item format handlers                                │
│  - Output formatters (JSON, XML, text)                      │
│                                                             │
│  Languages: C++23 (core), C++20 (MSVC)                      │
│  Location: src/asterix/                                     │
└─────────────────────────┬───────────────────────────────────┘
                          │
┌─────────────────────────┴───────────────────────────────────┐
│                   ENGINE LAYER                              │
│  - Generic data processing framework                        │
│  - Device abstraction (stdin, file, network, serial)        │
│  - Format/codec framework                                   │
│  - Channel management                                       │
│                                                             │
│  Languages: C++23                                           │
│  Location: src/engine/                                      │
└─────────────────────────────────────────────────────────────┘
```

### Layer Responsibilities

#### 1. Engine Layer (`src/engine/`)

**Generic, reusable data processing framework:**

- **Device Abstraction**: Input sources (stdin, file, TCP, UDP, multicast, serial)
  - `BaseDevice` - Pure virtual base class
  - `StdioDevice` - Standard input/output
  - `FileDevice` - File I/O
  - `UdpDevice` - UDP unicast/multicast
  - `TcpDevice` - TCP client/server
  - `SerialDevice` - Serial port (RS-232/422/485)
  - `DeviceFactory` - Factory pattern for device creation

- **Format/Codec Framework**: Protocol encapsulation
  - `BaseFormat` - Pure virtual base class
  - Format handlers for various encapsulations
  - Codec pipeline for layered protocols

- **Channel Management**: Data routing and processing
  - `Channel` - Processing pipeline abstraction
  - `ChannelFactory` - Factory pattern for channel creation

**Design Pattern:** Factory pattern + Strategy pattern

#### 2. ASTERIX Layer (`src/asterix/`)

**Protocol-specific implementation:**

- **Configuration Management**:
  - `AsterixDefinition` - Singleton managing all category definitions
  - `XMLParser` - Loads category definitions from XML (DTD-validated)
  - `Category` - Represents ASTERIX category with UAP (User Application Profile)

- **Parsing Pipeline**:
  ```
  Input → Format Parser → DataBlock → DataRecord → DataItem → Output Formatter
  ```

- **Core Data Structures**:
  - `DataBlock` - ASTERIX data block container (Category + Length + Records)
  - `DataRecord` - Single ASTERIX record with FSPEC (Field Specification)
  - `DataItem` - Individual data field
  - `DataItemFormat*` - Polymorphic format handlers

- **Format Handlers** (Polymorphic Hierarchy):
  - `DataItemFormatFixed` - Fixed-length data items
  - `DataItemFormatVariable` - Variable-length data items
  - `DataItemFormatCompound` - Compound data items (multiple sub-items)
  - `DataItemFormatRepetitive` - Repetitive data items (REP field)
  - `DataItemFormatExplicit` - Explicit length encoding
  - `DataItemFormatBDS` - Mode S BDS registers

- **Input Format Handlers** (Encapsulation Protocols):
  - `asterixrawsubformat` - Raw ASTERIX binary
  - `asterixpcapsubformat` - PCAP encapsulated (optimized buffer reuse)
  - `asterixhdlcsubformat` - HDLC framed
  - `asterixfinalsubformat` - FINAL packet format
  - `asterixgpssubformat` - GPS packet format

- **Output Formatters**:
  - Text (human-readable, line mode)
  - JSON (compact, human-readable, extensive with descriptions)
  - XML (compact, human-readable)

**Key Optimizations**:
- String reserve() for concatenation (15% speedup)
- Hex string loop reserve() (8% speedup)
- PCAP buffer reuse (15-20% speedup)
- UDP multicast fd_set caching (2-3% speedup)
- **Total cumulative speedup: 55-61%** (as of 2025-10-20)

#### 3. Application Layer (`src/main/`, `asterix/`, `asterix-rs/`)

**User-facing interfaces:**

- **C++ Executable** (`src/main/asterix.cpp`):
  - Command-line interface with multiple input/output options
  - Built with CMake (unified cross-platform build)
  - Output: `install/bin/asterix`

- **Python Module** (`asterix/`):
  - C extension wrapper (`src/python/`)
  - Pythonic API (`asterix/__init__.py`)
  - Auto-initializes XML configurations on import
  - Returns data as nested dicts/lists
  - Published to PyPI: `pip install asterix_decoder`

- **Rust Crate** (`asterix-rs/`):
  - CXX-based FFI bridge (`src/ffi.rs`)
  - Safe, idiomatic Rust API (`src/lib.rs`)
  - Build script compiles C++ via CMake (`build.rs`)
  - Memory ownership: Data copied to Rust side (owned `Vec<AsterixRecord>`)
  - Published to crates.io: `cargo add asterix-decoder`

---

## Ideal Safety-Critical Architecture

### Overview

For safety-critical Air Traffic Management (ATM) systems, the ASTERIX decoder is evolving toward a **validated wrapper model** with strict separation between safety-critical core and application layer.

**Alignment:** DO-278A (Software Integrity Assurance for CNS/ATM Systems)

### Three-Layer Safety Model

```
┌──────────────────────────────────────────────────────────────┐
│                  APPLICATION LAYER                           │
│  - CLI tools, language bindings                              │
│  - NOT safety-critical                                       │
│  - Can throw exceptions, allocate freely                     │
│  - User-facing convenience APIs                              │
│                                                              │
│  Example: Python/Rust wrapper APIs                           │
└───────────────────────┬──────────────────────────────────────┘
                        │
          ┌─────────────┴─────────────┐
          │   FFI BOUNDARY LAYER      │ ← VALIDATION LAYER
          │                           │
          │  - Input validation       │   - Buffer bounds checking
          │  - Type safety            │   - Offset validation
          │  - Error code mapping     │   - Integer overflow prevention
          │  - Sanitization           │   - Null pointer checks
          │                           │
          │  Compliance: >80% coverage│
          └─────────────┬─────────────┘
                        │
┌───────────────────────┴──────────────────────────────────────┐
│              SAFETY-CRITICAL CORE                            │
│  - ASTERIX parser (target: Rust for memory safety)           │
│  - Deterministic execution (bounded loops, no recursion)     │
│  - No dynamic allocation in parse path (preallocated)        │
│  - No undefined behavior (comprehensive bounds checking)     │
│  - Comprehensive error codes (no silent failures)            │
│  - >90% test coverage (unit + integration + fuzz)            │
│  - Static analysis clean (CodeQL, Clippy, ASAN, MSAN)        │
│                                                              │
│  Compliance: DO-278A Level C/D                               │
└──────────────────────────────────────────────────────────────┘
```

### Safety-Critical Core Requirements

#### Deterministic Behavior

**Goal:** Predictable, bounded execution

- All loops have maximum iteration count (no `while (true)`)
- No recursion (or bounded with max depth)
- No dynamic memory allocation in hot path
- Fixed-size data structures where possible
- All code paths have bounded execution time

**Example:**
```cpp
// BAD: Unbounded loop
while (true) {
    if (parseItem()) break;  // Could loop forever
}

// GOOD: Bounded iteration
for (int i = 0; i < MAX_ITEMS; i++) {
    if (!parseItem()) break;
}
```

#### Memory Safety

**Goal:** Zero crashes, zero undefined behavior

- All buffer accesses bounds-checked
- No raw pointer arithmetic (use safe wrappers)
- RAII for resource management (automatic cleanup)
- No manual memory management (use smart pointers)
- Static analysis: ASAN, MSAN, UBSAN, Valgrind

**Current Status:**
- ✅ All 110 Rust tests passing
- ✅ 0 memory leaks (Valgrind clean)
- ✅ Integration tests passing (12/12)

**Target:** Migrate core to Rust for compile-time memory safety guarantees

#### Error Handling

**Goal:** Comprehensive detection and reporting

- No exceptions in core parser (use error codes)
- All error paths tested
- Graceful degradation (partial parse on error)
- Detailed error reporting (category, offset, reason)
- No silent failures

**Example Error Hierarchy:**
```cpp
enum AsterixError {
    OK = 0,
    ERROR_EMPTY_INPUT,
    ERROR_INVALID_CATEGORY,
    ERROR_TRUNCATED_MESSAGE,
    ERROR_BUFFER_OVERFLOW,
    ERROR_INVALID_FSPEC,
    ERROR_MALFORMED_DATA,
    // ... comprehensive error codes
};
```

---

## Data Flow

### High-Level Parsing Pipeline

```
┌─────────────┐
│ Input Source│  (file, stdin, network multicast)
└──────┬──────┘
       │
       ▼
┌─────────────┐
│Format Parser│  (PCAP, HDLC, FINAL, GPS, raw)
└──────┬──────┘
       │ Extracts ASTERIX payload
       ▼
┌─────────────┐
│  Data Block │  (Category + Length + Records)
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ Data Record │  (FSPEC parsing → identifies present items)
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Data Items │  (Polymorphic parsing: Fixed/Variable/Compound/etc.)
└──────┬──────┘
       │
       ▼
┌─────────────┐
│Output Format│  (Text, JSON, XML)
└─────────────┘
```

### Detailed: DataRecord FSPEC Parsing

**FSPEC (Field Specification):** Variable-length bitfield indicating which data items are present in record.

```
┌─────────────────────────────────────────────────────────┐
│ FSPEC Byte 1   FSPEC Byte 2   ...   Data Items          │
│ [7 6 5 4 3 2 1 0] [7 6 5 4 3 2 1 0]                      │
│  │ │ │ │ │ │ │ │                                        │
│  │ │ │ │ │ │ │ └─ Item 1 present?                      │
│  │ │ │ │ │ │ └─── Item 2 present?                      │
│  │ │ │ │ │ └───── ...                                   │
│  │ │ │ │ └─────── Item 7 present?                      │
│  │ │ │ └───────── FX bit (1 = more FSPEC bytes follow) │
└─────────────────────────────────────────────────────────┘
```

**Parsing Algorithm:**
1. Read FSPEC bytes until FX bit = 0
2. For each bit set in FSPEC:
   - Lookup data item definition from XML
   - Dispatch to appropriate format handler (Fixed/Variable/Compound/etc.)
   - Parse data item from byte stream
3. Return parsed record with all data items

**Safety Note:** FSPEC parsing is **performance-critical** and has been extensively tested. Modifications to FSPEC loop can cause memory corruption. See `PERFORMANCE_OPTIMIZATIONS.md`.

### Detailed: Data Item Format Parsing

Each data item type has a specialized parser:

**Fixed Length:**
```
┌────────────────────┐
│  N bytes (fixed)   │  → Parse directly (memcpy, decode)
└────────────────────┘
```

**Variable Length:**
```
┌─────┬──────────────┐
│ LEN │  Data (LEN)  │  → Read length byte, then parse data
└─────┴──────────────┘
```

**Compound:**
```
┌─────┬─────┬──────┬──────┬──────┐
│ FSPEC│ SubI│ SubI │ SubI │ ... │  → Recursive FSPEC parsing
└─────┴──────┴──────┴──────┴─────┘
```

**Repetitive:**
```
┌─────┬──────┬──────┬──────┬──────┐
│ REP │ Item │ Item │ Item │ ... │  → Repeat count, then parse REP items
└─────┴──────┴──────┴──────┴─────┘
```

---

## FFI Boundaries and Safety Patterns

### Current FFI Implementations

#### Python C Extension

**File:** `src/python/asterix_wrapper.c`

**Current Status:** Basic validation, needs comprehensive audit (Issue #29)

**Validation Checklist:**
- ✅ Buffer is bytes object
- ⚠️ Buffer length > 0 (needs enhancement)
- ⚠️ Offset/count parameters (needs validation)
- ❌ Integer overflow prevention (needs addition)
- ✅ Proper error handling (returns None on error)
- ✅ Memory safety (proper refcounting)

**Example (current):**
```c
static PyObject* asterix_parse(PyObject* self, PyObject* args) {
    const char* buffer;
    Py_ssize_t length;

    // Basic validation
    if (!PyArg_ParseTuple(args, "s#", &buffer, &length)) {
        return NULL;  // PyArg_ParseTuple sets exception
    }

    // TODO: Add bounds checking, overflow prevention
    // Call C++ parser
    return parse_internal(buffer, length);
}
```

#### Rust CXX Bridge

**File:** `asterix-rs/src/ffi.rs`

**Current Status:** Type-safe via CXX, needs comprehensive audit (Issue #29)

**Validation Checklist:**
- ✅ Data slice not empty (enforced by Rust)
- ⚠️ Bounds checking (needs explicit validation)
- ✅ Integer overflow prevention (Rust checked arithmetic)
- ✅ No panics (uses Result)
- ✅ Clippy passes
- ⚠️ Miri testing (needs integration)
- ❌ Fuzz testing (needs addition)

**Example (current):**
```rust
pub fn parse(data: &[u8], options: &ParseOptions) -> Result<Vec<AsterixRecord>, AsterixError> {
    if data.is_empty() {
        return Err(AsterixError::EmptyInput);
    }

    // TODO: Add comprehensive validation
    // Call C++ via CXX bridge
    unsafe_ffi::parse_asterix(data, options)
        .map_err(|e| AsterixError::ParseError(e))
}
```

### Ideal FFI Boundary Pattern

**Design Goal:** Validated, safe FFI boundary for ALL language bindings

```cpp
// Safe FFI boundary function (future implementation)
extern "C" int asterix_parse_safe(
    const uint8_t* data,
    size_t data_len,
    size_t offset,
    size_t count,
    AsterixRecord** out_records,
    size_t* out_count,
    char* error_buffer,
    size_t error_buffer_size)
{
    // ========== INPUT VALIDATION ==========

    // 1. Null pointer checks
    if (!data || !out_records || !out_count) {
        snprintf(error_buffer, error_buffer_size, "Null pointer argument");
        return ERROR_INVALID_ARGUMENT;
    }

    // 2. Empty input check
    if (data_len == 0) {
        snprintf(error_buffer, error_buffer_size, "Empty input data");
        return ERROR_EMPTY_INPUT;
    }

    // 3. Integer overflow prevention
    if (offset > SIZE_MAX - count) {
        snprintf(error_buffer, error_buffer_size, "Integer overflow in offset+count");
        return ERROR_INTEGER_OVERFLOW;
    }

    // 4. Bounds checking
    if (offset + count > data_len) {
        snprintf(error_buffer, error_buffer_size,
            "Offset/count out of range: offset=%zu, count=%zu, data_len=%zu",
            offset, count, data_len);
        return ERROR_OUT_OF_RANGE;
    }

    // ========== CALL SAFE CORE PARSER ==========
    return asterix_parse_internal(data + offset, count, out_records, out_count);
}
```

**Key Principles:**
1. **Validate ALL inputs** before calling core parser
2. **No assumptions** about caller behavior
3. **Comprehensive error reporting** with detailed messages
4. **No silent failures** - always return error codes
5. **Test ALL error paths** with fuzzing and error injection

---

## Language Bindings

### C++ Executable

**Build System:** CMake 3.20+

**Standard:** C++23 (Linux/macOS), C++20 (Windows/MSVC)

**Build Commands:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build

./install/bin/asterix --help
```

**Key Files:**
- `src/main/asterix.cpp` - Main entry point
- `CMakeLists.txt` - Root CMake configuration
- `src/asterix/CMakeLists.txt` - Core library configuration
- `src/engine/CMakeLists.txt` - Engine library configuration

**Features:**
- Multiple input sources (file, stdin, network multicast)
- Multiple output formats (text, JSON, XML)
- Encapsulation support (PCAP, HDLC, FINAL, GPS)
- Performance optimizations (55-61% cumulative speedup)

### Python Module

**Package:** `asterix_decoder` (PyPI)

**Supported Versions:** Python 3.10 - 3.14

**Architecture:**
```
┌──────────────────────────────────┐
│  asterix/__init__.py             │  ← User-facing Python API
│  - parse(data)                   │
│  - parse_with_offset(...)        │
│  - describe(...)                 │
└────────────┬─────────────────────┘
             │ (import)
┌────────────┴─────────────────────┐
│  _asterix.so (C extension)       │  ← FFI boundary (needs audit)
│  - src/python/asterix_wrapper.c  │
│  - Converts Python bytes → C++   │
│  - Returns Python dicts/lists    │
└────────────┬─────────────────────┘
             │ (calls)
┌────────────┴─────────────────────┐
│  libasterix.so (C++ core)        │  ← Core parser
│  - src/asterix/*.cpp             │
└──────────────────────────────────┘
```

**Installation:**
```bash
pip install asterix_decoder
```

**Example:**
```python
import asterix

# Initialize with default categories (auto-loaded)
data = open('sample.pcap', 'rb').read()

# Parse ASTERIX data
records = asterix.parse(data)

for record in records:
    print(f"Category: {record['category']}")
    for item in record['items']:
        print(f"  {item['name']}: {item['value']}")
```

**Safety Status:**
- ⚠️ Needs FFI boundary audit (Issue #29)
- ✅ Test coverage >70%
- ✅ Integration tests passing

### Rust Crate

**Package:** `asterix-decoder` (crates.io)

**MSRV:** Rust 1.87

**Architecture:**
```
┌──────────────────────────────────┐
│  asterix-rs/src/lib.rs           │  ← Safe, idiomatic Rust API
│  - parse(data, options)          │
│  - Parser::new().build()         │
└────────────┬─────────────────────┘
             │ (calls)
┌────────────┴─────────────────────┐
│  asterix-rs/src/ffi.rs           │  ← CXX bridge (type-safe FFI)
│  - #[cxx::bridge]                │
│  - Safe Rust ↔ C++ interop       │
└────────────┬─────────────────────┘
             │ (CXX codegen)
┌────────────┴─────────────────────┐
│  libasterix.a (C++ core)         │  ← Core parser (built by build.rs)
│  - Compiled via CMake            │
└──────────────────────────────────┘
```

**Build Script:** `asterix-rs/build.rs`
- Invokes CMake to compile C++ core
- Generates CXX bridge code
- Links C++ library with Rust binary

**Installation:**
```bash
cargo add asterix-decoder
```

**Example:**
```rust
use asterix::{Parser, ParseOptions};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize with default categories
    let parser = Parser::new()
        .init_default()?
        .build()?;

    // Parse ASTERIX data
    let data = std::fs::read("sample.pcap")?;
    let options = ParseOptions::default();
    let records = parser.parse(&data, &options)?;

    for record in records {
        println!("Category: {}", record.category);
        for item in record.items {
            println!("  {}: {:?}", item.name, item.value);
        }
    }

    Ok(())
}
```

**Safety Status:**
- ⚠️ Needs FFI boundary audit (Issue #29)
- ✅ All 110 tests passing
- ✅ Clippy clean
- ❌ Miri testing (needs integration)
- ❌ Fuzz testing (needs addition)

**Memory Ownership:**
- Data **copied** from C++ to Rust (owned `Vec<AsterixRecord>`)
- No shared ownership across FFI boundary
- C++ allocates, Rust takes ownership and frees

---

## Configuration System

### XML-Based Category Definitions

ASTERIX categories are defined in XML files following the DTD specification.

**DTD:** `asterix/config/asterix.dtd`

**Category Files:** `asterix/config/asterix_cat{CATEGORY}_{VERSION}.xml`

**Examples:**
- `asterix_cat048_1_32.xml` - CAT 048 (Monoradar Target Reports) v1.32
- `asterix_cat062_1_19.xml` - CAT 062 (System Track Data) v1.19
- `asterix_cat021_2_6.xml` - CAT 021 (ADS-B Target Reports) v2.6

**BDS Registers:** `asterix/config/asterix_bds.xml`

### Configuration Loading

**C++ Executable:**
```cpp
// Singleton pattern
AsterixDefinition* def = AsterixDefinition::instance();

// Load category from XML
def->load_category("asterix/config/asterix_cat048_1_32.xml");

// Auto-loads all categories from asterix.ini
```

**Python Module:**
```python
# Auto-initializes on import
import asterix  # Loads all default categories

# Load custom category
asterix.init("custom_category.xml")
```

**Rust Crate:**
```rust
// Default initialization
let parser = Parser::new()
    .init_default()?  // Loads all default categories
    .build()?;

// Custom categories
let parser = Parser::new()
    .add_category("custom_category.xml")?
    .build()?;
```

### XML Structure

**Example:**
```xml
<Category id="48" ver="1.32">
  <DataItem id="010">
    <DataItemName>Data Source Identifier</DataItemName>
    <DataItemDefinition>Identification of the radar station</DataItemDefinition>
    <DataItemFormat desc="Two-octet fixed length Data Item">
      <Fixed length="2">
        <Bits from="16" to="9"><BitsShortName>SAC</BitsShortName></Bits>
        <Bits from="8" to="1"><BitsShortName>SIC</BitsShortName></Bits>
      </Fixed>
    </DataItemFormat>
  </DataItem>
  <!-- ... more items ... -->
  <UAP>
    <UAPItem bit="0" frn="1" len="2">010</UAPItem>
    <!-- ... UAP mapping ... -->
  </UAP>
</Category>
```

**UAP (User Application Profile):**
- Maps FSPEC bit positions to data item IDs
- Allows parser to decode FSPEC → identify present items
- Stored in memory for fast lookup during parsing

---

## Input/Output Format Handling

### Input Formats

The decoder supports multiple encapsulation protocols:

| Format | CLI Flag | Handler | Description |
|--------|----------|---------|-------------|
| Raw ASTERIX | (default) | `asterixrawsubformat` | Pure ASTERIX binary |
| PCAP | `-P` | `asterixpcapsubformat` | PCAP file (Wireshark format) |
| ORADIS PCAP | `-R` | Special PCAP | ORADIS-specific PCAP |
| ORADIS | `-O` | Special format | ORADIS binary |
| FINAL | `-F` | `asterixfinalsubformat` | FINAL packet format |
| HDLC | `-H` | `asterixhdlcsubformat` | HDLC framed |
| GPS | `-G` | `asterixgpssubformat` | GPS packet format |

**PCAP Optimization:**
The PCAP handler includes buffer reuse optimization (15-20% speedup). See `src/asterix/asterixpcapsubformat.cpp:146`.

### Input Sources

| Source | CLI Flag | Device | Description |
|--------|----------|--------|-------------|
| File | `-f <file>` | `FileDevice` | Read from file |
| Stdin | (default) | `StdioDevice` | Read from standard input |
| Multicast | `-i m:i:p[:s]` | `UdpDevice` | UDP multicast (mcast:iface:port[:source]) |
| TCP | (future) | `TcpDevice` | TCP client/server |
| Serial | (future) | `SerialDevice` | Serial port |

### Output Formats

| Format | CLI Flag | Description |
|--------|----------|-------------|
| Text | (default) | Human-readable text |
| Line | `-l, --line` | One line per item (parsable) |
| JSON Compact | `-j, --json` | One object per line |
| JSON Human | `-jh, --jsonh` | Human-readable JSON |
| JSON Extensive | `-je, --json-extensive` | JSON with descriptions |
| XML Compact | `-x, --xml` | One object per line |
| XML Human | `-xh, --xmlh` | Human-readable XML |

**Example (JSON Extensive):**
```json
{
  "category": 48,
  "len": 42,
  "items": [
    {
      "id": "010",
      "name": "Data Source Identifier",
      "value": {
        "SAC": {"value": 7, "desc": "System Area Code"},
        "SIC": {"value": 1, "desc": "System Identification Code"}
      }
    }
  ]
}
```

---

## Performance Characteristics

### Parsing Throughput

**Baseline (before optimizations):**
- CAT 048: ~150 MB/s
- CAT 062: ~120 MB/s
- Memory usage: ~50 MB peak

**After Quick Wins (55-61% cumulative speedup):**
- CAT 048: ~230 MB/s (+53%)
- CAT 062: ~185 MB/s (+54%)
- Memory usage: ~45 MB peak (-10%)

### Optimization Summary

| Quick Win | Commit | File | Speedup |
|-----------|--------|------|---------|
| #1 | fed87cd | `DataItemFormatVariable.cpp` | 15% |
| #2 | fed87cd | `DataItemFormatVariable.cpp` | 15% |
| #3 | 7feea81 | `Utils.cpp` | 8% |
| #5 | af6ef19 | `asterixpcapsubformat.cpp` | 15-20% |
| #6 | cc856f3 | `UdpDevice.cpp` | 2-3% |

**Total:** 55-61% cumulative speedup

**See:** `PERFORMANCE_OPTIMIZATIONS.md` for detailed analysis

### Memory Safety

**Current Status:**
- ✅ 0 memory leaks (Valgrind clean)
- ✅ All integration tests passing (12/12)
- ✅ All Rust tests passing (110/110)
- ✅ ASAN clean (Address Sanitizer)

**Test Coverage:**
- Core parser: ~85% (target: >90%)
- Python bindings: ~70% (target: >80%)
- Rust bindings: ~75% (target: >80%)

---

## Migration Path

### Phase 1: Documentation & Validation (Current)

**Status:** IN PROGRESS

**Goals:**
- ✅ Document current architecture (this file)
- 🔄 Create documentation review template for all issues
- 🔄 Audit existing FFI boundaries (Issue #29)

**Deliverables:**
- `docs/ARCHITECTURE.md` ← **THIS FILE**
- FFI boundary audit report
- Documentation standards

### Phase 2: Safety-Critical Core (Future)

**Status:** PLANNED (Issue #26)

**Goals:**
- Migrate core parser to Rust (memory safety)
- Implement validated FFI boundary layer
- Achieve >90% test coverage
- Integrate fuzz testing (AFL/libFuzzer)
- Integrate static analysis (CodeQL, Clippy, ASAN, MSAN)

**Deliverables:**
- `src/asterix-core/` (Rust implementation)
- Validated FFI boundary functions
- Comprehensive test suite
- Static analysis CI integration

### Phase 3: DO-278A Alignment (Future)

**Status:** PLANNED (Issue #26)

**Goals:**
- Establish traceability matrix (Requirements → Code → Tests)
- Document safety-critical design patterns
- Implement deterministic behavior (bounded loops, no recursion)
- Performance baselines and regression detection

**Deliverables:**
- `docs/SAFETY_CRITICAL.md`
- `do-278/requirements/Safety_Requirements.md`
- Traceability matrix
- Performance baseline documentation

### Phase 4: Formal Verification (Long-term)

**Status:** RESEARCH

**Goals:**
- Explore formal verification tools (TLA+, Coq, Isabelle)
- Prove safety properties (memory safety, determinism)
- Consider MISRA C++ compliance
- Investigate DO-278A certification path

---

## References

### Standards

- **DO-278A:** Guidelines for Communication, Navigation, Surveillance and Air Traffic Management (CNS/ATM) Systems Software Integrity Assurance
- **MISRA C++:** Guidelines for the use of the C++ language in critical systems
- **CERT C++ Secure Coding:** https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=88046682
- **EUROCONTROL ASTERIX:** https://www.eurocontrol.int/asterix

### Documentation

- **BUILD.md:** Comprehensive build guide (all platforms)
- **CONTRIBUTING.md:** Developer contribution guidelines
- **CLAUDE.md:** Technical architecture for Claude Code
- **PERFORMANCE_OPTIMIZATIONS.md:** Performance optimization history
- **LANGUAGE_BINDINGS_COMPARISON.md:** Python vs. Rust bindings comparison

### Issues

- **#26:** Safety-critical design patterns and guidelines
- **#29:** Audit Python/Rust bindings for safety boundary compliance
- **#47:** Documentation overhaul
- **#28:** Standardize CI/CD across bindings

### External

- **Upstream ASTERIX specs:** https://github.com/zoranbosnjak/asterix-specs
- **GitHub Repository:** https://github.com/montge/asterix
- **GitHub Pages:** https://montge.github.io/asterix/

---

**Document Version:** 1.0
**Last Updated:** 2025-11-05
**Maintainers:** ASTERIX Contributors
**License:** GPL-3.0-or-later
