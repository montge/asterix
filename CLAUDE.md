# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is a decoder/parser for EUROCONTROL ASTERIX protocol - an ATM (Air Traffic Management) Surveillance Data Binary Messaging Format. The project provides three language bindings:
1. **C++ standalone executable** - High-performance command-line tool (C++23/C23)
2. **Python module** - Simple, intuitive API for Python developers (3.10-3.14)
3. **Rust crate** - Type-safe, memory-safe bindings with zero-copy performance (1.70+)

All three bindings share the same C++ core for parsing ASTERIX data from various sources (files, stdin, network multicast streams).

## Build System & Commands

### C++ Executable

**Requirements:**
- **C++23 compatible compiler** (upgraded from C++17):
  - GCC 13.0+ (recommended for full C++23 support)
  - Clang 16.0+ (recommended for full C++23 support)
  - MSVC 2022 v17.4+ (Visual Studio 2022 version 17.4 or later)
  - AppleClang 15.0+ (Xcode 15 or later)
- libexpat-devel (for XML parsing)
- CMake 3.20+ or GNU Make

**Using Make (Primary method):**
```bash
# Build from src/ directory
cd src
make              # Production build
make install      # Install to install/ directory
make debug        # Debug build
make debug install
make clean
make test         # Run test suite (from src/)
```

**Using CMake (Alternative):**
```bash
cmake .
make
```

The executable will be created at `install/asterix` (not asterix.exe despite documentation).

**C++ Standard:** The project uses C++23 (set in CMakeLists.txt via `CMAKE_CXX_STANDARD 23`)
**C Standard:** The project uses C23 (set in CMakeLists.txt via `CMAKE_C_STANDARD 23`)

### Python Module

```bash
# Build
python setup.py build

# Install locally
python setup.py install

# Install from PyPI
pip install asterix_decoder

# Run tests
python -m unittest
cd install/test && ./test.sh   # Integration tests
```

### Rust Bindings

**Requirements:**
- Rust 1.70+ (2021 edition)
- C++17 compatible compiler (same as C++ executable)
- libexpat-devel (for XML parsing)
- Cargo build system

**Using Cargo:**
```bash
# Build from asterix-rs/ directory
cd asterix-rs
cargo build              # Debug build
cargo build --release    # Production build
cargo test               # Run tests
cargo test --all-features # Run all tests
cargo bench              # Run benchmarks

# Install from crates.io
cargo add asterix-decoder
```

**Directory Structure:**
```
asterix-rs/
├── Cargo.toml           # Rust package manifest
├── build.rs             # Build script (compiles C++ via CMake)
├── README.md            # Rust-specific README
├── src/
│   ├── lib.rs          # Public API
│   ├── ffi.rs          # CXX bridge (unsafe FFI)
│   ├── parser.rs       # Safe Rust API
│   ├── data_types.rs   # AsterixRecord, DataItem types
│   ├── error.rs        # AsterixError enum
│   └── config.rs       # Configuration management
├── tests/
│   └── integration_tests.rs  # Integration tests
├── examples/
│   ├── parse_raw.rs    # Parse raw bytes
│   ├── parse_file.rs   # Parse from file
│   ├── parse_pcap.rs   # Parse PCAP with incremental parsing
│   ├── incremental_parsing.rs
│   ├── json_export.rs
│   ├── describe_category.rs
│   └── stream_processing.rs
└── benches/
    └── benchmarks.rs   # Performance benchmarks
```

**FFI Architecture:**
- Uses CXX crate for safe C++/Rust interop
- Build script (build.rs) invokes CMake to compile C++ core
- Type-safe FFI bridge eliminates manual unsafe code
- Memory ownership: Data copied to Rust side (owned Vec<AsterixRecord>)

**Testing:**
```bash
cd asterix-rs
cargo test              # Unit + integration tests
cargo test --doc        # Documentation tests
cargo bench             # Performance benchmarks
cargo clippy            # Linter
cargo fmt --check       # Format check
```

### Running Tests

```bash
# C++ integration tests
cd install/test
./test.sh

# Python tests
python -m unittest

# Rust tests
cd asterix-rs
cargo test --all-features
cargo bench

# Memory leak tests (requires valgrind)
cd install/test
./valgrind_test.sh
```

## Architecture

### Multi-Layer Design

The codebase has three distinct layers that work together:

1. **Engine Layer** (`src/engine/`) - Generic data processing framework
   - Device abstraction (stdin, file, TCP, UDP, multicast, serial)
   - Format/codec framework
   - Channel management and routing
   - Factory patterns for devices and formats

2. **ASTERIX Layer** (`src/asterix/`) - Protocol-specific implementation
   - XML-based configuration parser for ASTERIX categories
   - Data parsing for various ASTERIX item formats (Fixed, Variable, Compound, Repetitive, Explicit, BDS)
   - Format submodules for encapsulation protocols (PCAP, HDLC, FINAL, GPS)
   - Output formatters (text, JSON, XML)

3. **Application Layer** (`src/main/`) - CLI and language bindings
   - Command-line interface (`asterix.cpp`)
   - Python C extension wrapper (`src/python/`)
   - Rust bindings via CXX FFI (`asterix-rs/`)

### Key Components

**ASTERIX Parsing Pipeline:**
```
Input Source → Format Parser → Data Block → Data Record → Data Items → Output Formatter
```

- `AsterixDefinition`: Manages XML category definitions
- `Category`: Represents ASTERIX category with UAP (User Application Profile)
- `DataBlock`: ASTERIX data block container
- `DataRecord`: Single ASTERIX record
- `DataItem`: Individual data field with format-specific parsing
- `DataItemFormat*`: Polymorphic format handlers (Fixed/Variable/Compound/etc.)
- `XMLParser`: Loads ASTERIX category definitions from XML

**Input Format Handlers:**
- `asterixrawsubformat`: Raw ASTERIX binary
- `asterixpcapsubformat`: PCAP encapsulated data
- `asterixhdlcsubformat`: HDLC framed data
- `asterixfinalsubformat`: FINAL packet format
- `asterixgpssubformat`: GPS packet format

### Configuration System

ASTERIX categories are defined in XML files following the DTD specification at `asterix/config/asterix.dtd`. Configuration files are listed in `asterix.ini`.

- Category definitions: `asterix/config/asterix_cat*_*.xml` (e.g., `asterix_cat062_1_18.xml`)
- BDS definitions: `asterix/config/asterix_bds.xml`
- Each category file defines data items, formats, UAPs, and value meanings

### Python Module Design

The Python module (`asterix/`) wraps the C++ parser via a C extension (`_asterix`):

- C extension built from `src/python/*.c` and core ASTERIX C++ files
- Python wrapper in `asterix/__init__.py` provides high-level API
- Auto-initializes all XML configurations on import
- Returns parsed data as nested dictionaries/lists

**Key Python APIs:**
- `parse(data)` - Parse raw bytes
- `parse_with_offset(data, offset, blocks_count)` - Incremental parsing
- `describe(category, item, field, value)` - Get descriptions
- `init(filename)` - Load custom category definition

### Rust Module Design

The Rust module (`asterix-rs/`) wraps the C++ parser via the CXX crate:

- FFI bridge built using CXX crate for type-safe C++/Rust interop
- Build script (build.rs) compiles C++ core via CMake
- Rust wrapper in `src/lib.rs` provides safe, idiomatic API
- Auto-initializes XML configurations via `init_default()`
- Returns parsed data as owned Rust types (Vec<AsterixRecord>)

**Key Rust APIs:**
- `parse(data, options)` - Parse raw bytes with options
- `parse_with_offset(data, offset, count, options)` - Incremental parsing
- `describe(category, item, field, value)` - Get descriptions
- `init_default()` - Initialize with default categories
- `Parser::new().add_category(path).build()` - Custom categories

## Common Development Tasks

### Adding/Updating ASTERIX Categories

Categories should be updated via the upstream [asterix-specs](https://github.com/zoranbosnjak/asterix-specs) project, then converted:

```bash
cd asterix-specs-converter

# Convert single category from JSON to XML
curl https://zoranbosnjak.github.io/asterix-specs/specs/cat062/cats/cat1.18/definition.json | \
  python3 asterixjson2xml.py > specs/asterix_cat062_1_18.xml

# Update all specs automatically
python3 update-specs.py
```

Then copy to `asterix/config/` (the build system automatically copies to `install/share/asterix/config/`).

### C++ Code Organization

**Language Standard:** C++23 (upgraded from C++17)

The codebase uses modern C++23 features including:
- Deduced this (explicit object parameters) - for better polymorphic performance
- Ranges algorithms - for cleaner container operations
- `std::format` - for type-safe formatting (when available)
- `std::string_view` - for efficient string handling (C++17 feature, retained)
- `std::optional` - for optional values (C++17 feature, retained)
- Structured bindings (C++17 feature, retained)
- `if constexpr` - for compile-time conditionals (C++17 feature, retained)
- Feature detection via `src/asterix/cxx23_features.h` - graceful fallback to C++17/20

**Backward Compatibility:**
The codebase includes feature detection macros in `src/asterix/cxx23_features.h` that allow building with older C++17/20 compilers when C++23 features are not available. Performance optimizations from C++23 features (deduced this, ranges) will be automatically disabled when using older compilers.

**Code Structure:**
- Header/implementation pairs in `src/asterix/`
- Pure virtual base classes for extensibility (`DataItemFormat`, `BaseDevice`, `BaseFormat`)
- Factory patterns in engine layer (`DeviceFactory`, `ChannelFactory`)
- Singleton pattern for global state (`Tracer`, `AsterixDefinition`)

### Output Formats

Controlled via CLI flags:
- Default: Human-readable text
- `-l, --line`: One line per item (parsable)
- `-x, --xml`: Compact XML (one object per line)
- `-xh, --xmlh`: Human-readable XML
- `-j, --json`: Compact JSON (one object per line)
- `-jh, --jsonh`: Human-readable JSON
- `-je, --json-extensive`: Extensive JSON with descriptions

### Input Sources

- `-f filename`: File input (PCAP, raw, FINAL, HDLC, GPS)
- `-i m:i:p[:s]`: Multicast (format: mcast_addr:interface_ip:port[:source])
- stdin: Default if no source specified

Input format flags: `-P` (PCAP), `-R` (ORADIS PCAP), `-O` (ORADIS), `-F` (FINAL), `-H` (HDLC), `-G` (GPS)

## Repository Structure Notes

- `asterix/` - Python module source
- `asterix-rs/` - Rust crate (bindings via CXX)
- `src/` - C++ source code (engine, asterix, main, python bindings)
- `install/` - Build output directory and test suite
- `asterix-specs-converter/` - Tools for converting JSON specs to XML
- Configuration files are in `asterix/config/` (source), copied to `install/share/asterix/config/` during build
- Rust uses config files from `asterix/config/` via FFI during build

## Upstream Repository

Original repository: https://github.com/CroatiaControlLtd/asterix

When updating from upstream:
```bash
git remote add upstream https://github.com/CroatiaControlLtd/asterix.git
git fetch upstream
git merge upstream/master --ff-only
git push origin master
```

## Performance Optimizations Completed

The codebase has been optimized through 6 Quick Wins achieving **55-61% cumulative speedup** (as of 2025-10-20):

1. **Quick Win #1** (fed87cd): String operator+ elimination - 15% speedup
2. **Quick Win #2** (fed87cd): String reserve() optimization - 15% speedup
3. **Quick Win #3** (7feea81): Hex string loop reserve() - 8% speedup
4. **Quick Win #5** (af6ef19): PCAP buffer reuse - 15-20% speedup
5. **Quick Win #6** (cc856f3): UDP multicast fd_set caching - 2-3% speedup

**Key Files Modified:**
- `src/asterix/DataItemFormatVariable.cpp` - String concatenation optimization
- `src/asterix/Utils.cpp` - Hex string reserve()
- `src/asterix/asterixpcapsubformat.cpp` - Buffer reuse
- `src/engine/UdpDevice.cpp` - fd_set template caching

**Testing:** All optimizations verified with 11/11 integration tests passing and 0 memory leaks (valgrind).

**⚠️ WARNING:** Do NOT optimize FSPEC parsing (`DataRecord::parse()` FSPEC loop) - causes memory corruption and segfaults.

See `PERFORMANCE_OPTIMIZATIONS_COMPLETED.md` for detailed analysis and measurements.
