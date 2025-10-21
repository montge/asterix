# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is a decoder/parser for EUROCONTROL ASTERIX protocol - an ATM (Air Traffic Management) Surveillance Data Binary Messaging Format. The project provides both a C++ standalone executable and a Python module for parsing ASTERIX data from various sources (files, stdin, network multicast streams).

## Build System & Commands

### C++ Executable

**Requirements:**
- **C++17 compatible compiler**:
  - GCC 7.0+ (recommended: GCC 9+ for better C++17 support)
  - Clang 5.0+ (recommended: Clang 9+ for better C++17 support)
  - MSVC 2017 15.3+ (Visual Studio 2017 version 15.3 or later)
  - AppleClang 9.1+ (Xcode 9.3 or later)
- libexpat-devel (for XML parsing)
- CMake 3.12+ or GNU Make

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

**C++ Standard:** The project uses C++17 (set in CMakeLists.txt via `CMAKE_CXX_STANDARD 17`)

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

### Running Tests

```bash
# C++ integration tests
cd install/test
./test.sh

# Python tests
python -m unittest

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

3. **Application Layer** (`src/main/`) - CLI and Python bindings
   - Command-line interface (`asterix.cpp`)
   - Python C extension wrapper (`src/python/`)

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

Then copy to `asterix/config/` and `install/config/`.

### C++ Code Organization

**Language Standard:** C++17

The codebase uses modern C++17 features including:
- Structured bindings
- `std::optional` for optional values
- `std::string_view` for efficient string handling
- `if constexpr` for compile-time conditionals
- Inline variables
- Nested namespaces

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
- `src/` - C++ source code (engine, asterix, main, python bindings)
- `install/` - Build output directory and test suite
- `asterix-specs-converter/` - Tools for converting JSON specs to XML
- Configuration files are duplicated in `asterix/config/` (Python) and `install/config/` (C++)

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
