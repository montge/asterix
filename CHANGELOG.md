# Changelog

All notable changes to the ASTERIX decoder project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- **Wireshark 4.x Plugin** - Modern ASTERIX dissector for Wireshark 4.0+
  - Phase 1 MVP: Basic data block dissection, FSPEC parsing, heuristic detection
  - Phase 2: C/C++ FFI wrapper for full data item parsing (in progress)
  - Conditional compilation support (works with or without ASTERIX core)
  - CMake build system with cross-platform support
  - Comprehensive design document (784 lines)
  - README with installation and usage instructions (362 lines)

- **Node.js Bindings - Complete Feature Set**
  - 6 comprehensive examples (1,169 lines total):
    - `parse_pcap.js` - Parse PCAP files with statistics
    - `incremental_parsing.js` - Memory-efficient chunked parsing
    - `stream_processing.js` - Node.js Transform/Writable streams
    - `express_api.js` - Full REST API with 8 endpoints
    - `file_watcher.js` - Real-time directory monitoring with chokidar
    - `basic.js` - Quick start example
  - Integration tests with real ASTERIX data (458 lines)
    - Tests with sample files (cat048.raw, cat062cat065.raw, etc.)
    - Incremental parsing validation
    - Category filtering tests
    - Performance and error handling tests
  - Performance benchmarks (388 lines)
    - 7 benchmark suites covering all operations
    - Throughput, latency, and memory usage measurements
    - Expected: 5-50 MB/s parse throughput
  - Comprehensive README (677 lines)
    - Complete API reference for all 6 functions
    - 5 code examples with explanations
    - Performance tips and thread safety guide
    - Troubleshooting section
  - Enhanced package.json with proper test/benchmark scripts

- **Documentation Overhaul (Issue #47)**
  - Python module README (709 lines) - Complete API reference, 5 examples, thread safety guide
  - C++ executable user guide (895 lines) - CLI reference, 7 formats, 10 examples, enterprise integrations
  - Rust crate user guide (1,397 lines) - Complete API docs, 8 examples, performance tips, async support
  - TROUBLESHOOTING.md (1,063 lines) - 9 categories covering build, runtime, memory, network issues
  - FAQ.md (862 lines) - 40+ Q&A pairs across 9 major sections
  - DOCUMENTATION.md - Reorganized with links to all user guides
  - Total: 6,000+ lines of comprehensive documentation

- **C++ API Documentation (Issue #72)**
  - Doxygen documentation for 24 core C++ headers (~1,200 lines)
  - Professional API documentation with `@class`, `@param`, `@return`, `@throws`
  - Documented classes: DataBlock, DataRecord, DataItem, Category, UAP, XMLParser, etc.
  - Format parsers: Fixed, Variable, Compound, Repetitive, Explicit, BDS
  - Input formatters: PCAP, HDLC, FINAL, GPS
  - 100% coverage of public C++ API

- **Documentation Standards (CLAUDE.md)**
  - Language-appropriate self-documenting standards (Doxygen, Docstrings, Doc Comments)
  - Examples and best practices for C++, Python, Rust
  - Guidelines for AI-assisted development

- **GitHub CLI Configuration**
  - Instructions for setting default fork repository
  - Prevents accidental issue creation in upstream

### Changed
- **README.md** - Reorganized Documentation section with links to comprehensive guides
- **Fixed broken Rust documentation link** - Corrected path to asterix-rs/README.md
- **valgrind_test.sh** - Updated for CMake build system (was using old Makefile paths)

### Removed
- **Legacy Wireshark/Ethereal Plugins (BREAKING, Issue #22 Phase 1)**
  - Removed Wireshark 1.8.4 plugin (plugins/asterix-wireshark-1-8-4/)
  - Removed Wireshark 1.10.6 plugin (plugins/asterix-wireshark-1-10-6/)
  - Removed Ethereal 0.99.0 plugin (plugins/asterix-ethereal-0-99-0/)
  - Created BREAKING_CHANGES.md with migration path to Wireshark 4.x
  - Rationale: Plugins unmaintained since 2014, incompatible with modern Wireshark

### Fixed
- **Rust Miri undefined behavior** (Issue #60)
  - Skipped concurrent parsing test under Miri due to C FFI limitations
  - Fixed undefined behavior detection in concurrent test
- **GPL-3.0 license compliance check**
  - Excluded project's own GPL-3.0 license from third-party compliance scan
  - Prevents false positive license violations
- **Node.js Windows build issues**
  - Multiple fixes for EXPAT library linking on Windows (x64-windows triplet)
  - Fixed missing DataRecord.cpp and globals.cpp compilation
  - Auto-detect EXPAT library filename
  - Use dynamic EXPAT library with XML_POOR_ENTROPY to avoid rand_s dependency
- **MSRV check in Rust CI**
  - Added missing toolchain version to MSRV check
  - Reverted to use dtolnay/rust-toolchain@1.87

---

## [2.8.10] - 2025-01-15

### Added
- **C++23 Standard Support** (Linux/macOS)
  - Upgraded from C++17 to C++23 on Linux/macOS
  - C++20 on Windows/MSVC (full C++23 not yet supported by MSVC)
  - Feature detection macros in `src/asterix/cxx23_features.h`
  - Graceful fallback to C++17/20 on older compilers
  - Deduced this (explicit object parameters) for better polymorphism
  - Ranges algorithms for cleaner container operations
  - `std::format` support when available

- **C23 Standard Support** (Linux/macOS)
  - C23 on Linux/macOS (set in CMakeLists.txt)
  - C17 on Windows/MSVC (MSVC doesn't fully support C23 yet)

- **Performance Optimizations** - 55-61% cumulative speedup
  - Quick Win #1: String operator+ elimination (15% speedup)
  - Quick Win #2: String reserve() optimization (15% speedup)
  - Quick Win #3: Hex string loop reserve() (8% speedup)
  - Quick Win #5: PCAP buffer reuse (15-20% speedup)
  - Quick Win #6: UDP multicast fd_set caching (2-3% speedup)
  - All verified with 11/11 integration tests passing, 0 memory leaks

- **Rust Bindings Enhancements**
  - 8 comprehensive examples (incremental parsing, JSON export, stream processing, etc.)
  - Complete error handling with `AsterixError` enum
  - Zero-copy parsing where possible
  - Comprehensive README with API docs, examples, troubleshooting

- **Python Module Enhancements**
  - 7 comprehensive examples (basic parsing, PCAP, streams, incremental, etc.)
  - Complete README with API reference, thread safety guide
  - Platform-specific build notes (C++23 on Linux, C++17 on macOS)

- **CI/CD Improvements**
  - Dependabot for GitHub Actions updates
  - Multi-platform testing (Linux, Windows, macOS)
  - Node.js LTS version support (20, 22, 24)
  - Rust MSRV 1.70+
  - Python 3.10-3.14 support

### Changed
- **CMake is now the only supported build system**
  - GNU Make build files removed in v2.8.10
  - CMake 3.20+ required
  - Cross-platform (Linux, Windows, macOS)
  - Out-of-source builds
  - Better dependency management

- **C++ Standard per Platform**
  - **Linux:** C++23 (GCC 13+, Clang 16+)
  - **macOS:** C++23 (AppleClang 15+)
  - **Windows:** C++20 (MSVC 2019+ - C++23 not fully supported yet)
  - Python module: Matches CMake standard per platform
  - Rust bindings: Use C++17 for better compatibility

- **Install directory structure**
  - Executable: `install/bin/asterix`
  - Libraries: `install/lib/libasterix.so`
  - Headers: `install/include/asterix/*.h`
  - Config: `install/share/asterix/config/*.xml`
  - Samples: `install/share/asterix/samples/*.{pcap,raw}`

### Deprecated
- **GNU Make build system** - Removed in v2.8.10, use CMake instead

### Fixed
- **Memory leaks** - All valgrind tests passing with 0 leaks
- **ASAN tests** - 19/19 tests passing on all platforms
- **Cross-platform builds** - Windows, macOS, Linux all building successfully
- **Compiler warnings** - Resolved C4100 warnings on MSVC (unused parameter names)

---

## [2.8.9] - 2024-12-10

### Added
- **ASTERIX Specs Converter** - Tools for converting JSON specs to XML
  - `asterixjson2xml.py` - Single category converter
  - `update-specs.py` - Bulk update tool
  - Integration with [asterix-specs](https://github.com/zoranbosnjak/asterix-specs) upstream

- **Additional ASTERIX Categories**
  - CAT205, CAT240, CAT247, CAT252
  - Total: 24 categories supported

- **Sample Files**
  - `cat_062_065.pcap` - Multi-category PCAP sample
  - `cat_034_048.pcap` - CAT034/CAT048 samples
  - `asterix.pcap` - General ASTERIX PCAP
  - `parsegps.gps` - GPS format sample

### Changed
- **Configuration files** - Updated to latest EUROCONTROL specifications
  - CAT001 v1.4, CAT002 v1.1, CAT004 v1.12
  - CAT062 v1.19 (updated from v1.18)
  - CAT034 v1.29 (updated from v1.28)

### Fixed
- **XML parsing** - Improved error messages for invalid category definitions
- **BDS format parsing** - Fixed edge cases in BDS register decoding

---

## [2.8.8] - 2024-10-15

### Added
- **Rust Bindings (asterix-rs)**
  - Type-safe, memory-safe bindings using CXX crate
  - Zero-copy parsing with owned Rust types
  - Published to crates.io as `asterix-decoder`
  - Comprehensive examples and benchmarks
  - MSRV: Rust 1.70+

- **Node.js Bindings (asterix-node)**
  - N-API bindings for Node.js 18+, 20+, 22+
  - TypeScript definitions included
  - Cross-platform: Linux, macOS, Windows
  - Memory-safe with automatic garbage collection
  - Published to npm as `asterix-decoder`

### Changed
- **Python Bindings** - Platform-specific C++ standards
  - Linux: C++23
  - Windows: C++20 (MSVC limitation)
  - macOS: C++17 (better compiler compatibility)

### Fixed
- **Windows build issues** - Resolved vcpkg triplet and library linking
- **Thread safety** - Fixed race conditions in singleton patterns

---

## [2.8.7] - 2024-08-20

### Added
- **JSON Output Format** - Extensive JSON with descriptions (`-je, --json-extensive`)
- **Category Filtering** - Filter by specific ASTERIX category during parsing
- **HDLC Format Support** - Parse HDLC-framed ASTERIX data
- **GPS Format Support** - Parse GPS packet format

### Changed
- **Output Formatters** - Modular design with pluggable formatters
- **Error Handling** - More descriptive error messages

### Fixed
- **PCAP parsing** - Fixed endianness issues on big-endian systems
- **Memory alignment** - Fixed unaligned access on ARM platforms

---

## [2.8.6] - 2024-06-10

### Added
- **Python Module** - First release of Python bindings
  - `pip install asterix-decoder`
  - Support for Python 3.8-3.12
  - Comprehensive API with `parse()`, `describe()`, `init()`

### Changed
- **CMake Build System** - Unified build for C++ executable and Python module
- **XML Configuration** - Improved category loading performance

### Fixed
- **Multicast Reception** - Fixed UDP multicast socket options on Windows
- **FINAL Format** - Fixed parsing of FINAL packet headers

---

## [2.8.5] - 2024-04-15

### Added
- **PCAP Support** - Parse ASTERIX data encapsulated in PCAP files
- **XML Output Format** - Human-readable XML (`-xh, --xmlh`)
- **Line Output Format** - One line per item (`-l, --line`)

### Changed
- **Default Output** - Human-readable text format improved
- **Category Definitions** - Updated to latest EUROCONTROL specs

### Fixed
- **Compound Items** - Fixed parsing of nested compound data items
- **Repetitive Items** - Fixed variable-length repetitive item parsing

---

## [2.8.0] - 2024-01-20

### Added
- **24 ASTERIX Categories** - Complete support for:
  - CAT001, CAT002, CAT004, CAT008, CAT010, CAT011, CAT015
  - CAT019, CAT020, CAT021, CAT023, CAT025, CAT030, CAT031
  - CAT032, CAT034, CAT048, CAT062, CAT063, CAT065
- **Multiple Input Formats** - stdin, file, TCP, UDP, multicast, serial
- **Multiple Output Formats** - Text, JSON, XML (compact and human-readable)
- **Multicast Support** - Receive ASTERIX data via UDP multicast
- **Category Descriptions** - Get human-readable descriptions for categories, items, fields

### Changed
- **Architecture** - Three-layer design (Engine/ASTERIX/Application)
- **Build System** - CMake 3.20+ required
- **License** - GPL-3.0-or-later

### Fixed
- **Memory Management** - Fixed memory leaks in DataBlock parsing
- **Endianness** - Portable byte order handling

---

## [2.7.0] - 2023-10-01

### Added
- **Initial Open Source Release** - Based on Croatia Control Ltd. proprietary decoder
- **Core Parsing Engine** - Generic data processing framework
- **ASTERIX Protocol Layer** - Category-based parsing with XML configuration
- **CLI Application** - Command-line interface for parsing ASTERIX data

---

## Legend

- **Added** - New features
- **Changed** - Changes in existing functionality
- **Deprecated** - Soon-to-be removed features
- **Removed** - Removed features
- **Fixed** - Bug fixes
- **Security** - Security vulnerability fixes

---

## Links

- [Repository](https://github.com/montge/asterix)
- [Issue Tracker](https://github.com/montge/asterix/issues)
- [Documentation](https://montge.github.io/asterix/)
- [npm Package](https://www.npmjs.com/package/asterix-decoder)
- [PyPI Package](https://pypi.org/project/asterix-decoder/)
- [crates.io Package](https://crates.io/crates/asterix-decoder)

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute to this project.

---

## Acknowledgments

Based on the ASTERIX decoder by Croatia Control Ltd.

ASTERIX specifications maintained by EUROCONTROL.
