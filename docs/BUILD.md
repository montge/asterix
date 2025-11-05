# ASTERIX Build & Installation Guide

**Complete guide for building and installing the ASTERIX decoder across all platforms**

**Supported Platforms:** Linux (x86_64, ARM64), Windows (x64), macOS (Intel & ARM M1+)
**Language Bindings:** C++ executable, Python module, Rust crate
**Last Updated:** 2025-11-04

---

## Table of Contents

1. [Quick Start](#1-quick-start)
2. [Prerequisites](#2-prerequisites)
3. [Platform-Specific Build Instructions](#3-platform-specific-build-instructions)
4. [Language Bindings](#4-language-bindings)
5. [C++23 Features (Advanced)](#5-c23-features-advanced)
6. [Testing & Quality](#6-testing--quality)
7. [Packaging](#7-packaging)
8. [Appendices](#8-appendices)

---

## 1. Quick Start

### One-Command Installation

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get update && sudo apt-get install -y build-essential cmake libexpat1-dev gcc-13 g++-13 && \
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel && cmake --install build
```

**macOS:**
```bash
brew install cmake expat && \
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel && cmake --install build
```

**Windows (PowerShell with vcpkg):**
```powershell
vcpkg install expat:x64-windows && cmake -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" && `
  cmake --build build --config Release
```

### Quick Verification

```bash
# Linux/macOS
./install/bin/asterix --version
./install/bin/asterix --help

# Windows
.\install\bin\asterix.exe --version
```

---

## 2. Prerequisites

### 2.1 Compiler Requirements

The ASTERIX decoder supports C++23 with automatic fallback to C++17 for older compilers.

#### Recommended (Full C++23 Support)

| Platform | Compiler | Version | C++ Standard |
|----------|----------|---------|--------------|
| **Linux** | GCC | 13.0+ | C++23 |
| **Linux** | Clang | 16.0+ | C++23 |
| **Windows** | MSVC | 2022 v17.4+ | C++20* |
| **macOS** | AppleClang | 15.0+ (Xcode 15+) | C++23 |

*MSVC doesn't fully support C++23 yet; uses C++20 with C++23 features where available.

#### Minimum (C++17 Fallback Mode)

| Platform | Compiler | Version | C++ Standard |
|----------|----------|---------|--------------|
| **Linux** | GCC | 7.0+ | C++17 |
| **Linux** | Clang | 5.0+ | C++17 |
| **Windows** | MSVC | 2017 15.3+ | C++17 |
| **macOS** | AppleClang | 9.1+ | C++17 |

**Feature Detection:**
The codebase includes `src/asterix/cxx23_features.h` which automatically detects available C++23 features and provides graceful fallback to C++17 when needed. See [Section 5](#5-c23-features-advanced) for details.

### 2.2 Required Libraries

**All Platforms:**
- **CMake**: 3.20+ (required for C++23 builds; 3.12+ minimum)
- **libexpat**: XML parsing library (version 2.2.0+)

**Platform-Specific Installation:**

**Linux (Debian/Ubuntu):**
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libexpat1-dev git
```

**Linux (RHEL/Fedora/Rocky):**
```bash
sudo dnf install -y cmake ninja-build expat-devel gcc gcc-c++
```

**macOS:**
```bash
# Install Xcode Command Line Tools first
xcode-select --install

# Install dependencies via Homebrew
brew update
brew install cmake expat
```

**Windows:**
```powershell
# Using vcpkg (recommended)
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install expat:x64-windows
.\vcpkg integrate install
```

### 2.3 Optional Tools

**For Development & Testing:**

```bash
# Linux (Ubuntu/Debian)
sudo apt-get install -y \
  lcov \                # C++ code coverage
  valgrind \            # Memory leak detection
  ninja-build           # Faster builds

# macOS
brew install lcov ninja
```

**For Python Development:**

```bash
# All platforms (via pip)
pip install --upgrade pip setuptools wheel pytest pytest-cov black pylint flake8
```

**For Rust Development:**

```bash
# All platforms
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
rustc --version  # Should be 1.70.0+
```

### 2.4 Compiler Upgrade Guide

**Upgrade to GCC 13 for C++23 (Ubuntu 22.04+):**

```bash
sudo apt-get install -y gcc-13 g++-13
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 130
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 130

# Verify
g++ --version  # Should show 13.x
```

**Install Visual Studio 2022 (Windows):**

1. Download [Visual Studio 2022 Community](https://visualstudio.microsoft.com/downloads/)
2. Run installer and select:
   - Workload: "Desktop development with C++"
   - Components: MSVC v143, Windows SDK, CMake tools

**Install Xcode 15+ (macOS):**

```bash
# Check current version
xcodebuild -version

# Update via App Store or:
# Download from https://developer.apple.com/xcode/
```

---

## 3. Platform-Specific Build Instructions

### 3.1 Linux

#### CMake Build (Recommended)

This is the official build method for the C++ executable.

> **Note**: GNU Make build files were removed in v2.8.10. CMake is now the only supported build system.

**Step 1: Configure**
```bash
cd /path/to/asterix

cmake -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_C_STANDARD=23 \
  -DCMAKE_C_COMPILER=gcc-13 \
  -DCMAKE_CXX_COMPILER=g++-13 \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_EXECUTABLE=ON \
  -DCMAKE_INSTALL_PREFIX=$PWD/install
```

**Step 2: Build**
```bash
cmake --build build --config Release --parallel $(nproc)
```

**Step 3: Install**
```bash
cmake --install build --config Release
```

**Step 4: Verify**
```bash
./install/bin/asterix --version
```

#### Troubleshooting (Linux)

**Issue: "undefined reference to `expat_*`"**

**Solution:**
```bash
sudo apt-get install libexpat1-dev
```

**Issue: C++23 features not available**

**Solution:** Upgrade compiler to GCC 13+ or Clang 16+ (see [Section 2.4](#24-compiler-upgrade-guide))

**Issue: CMake version too old**

**Solution:**
```bash
# Ubuntu 20.04/22.04 - install from Kitware repository
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
  gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt-get update
sudo apt-get install cmake
```

---

### 3.2 Windows

#### Prerequisites (Windows-Specific)

1. **Visual Studio 2022** (Community, Professional, or Enterprise)
   - Workload: "Desktop development with C++"
   - Components: MSVC v143, Windows SDK, CMake tools

2. **vcpkg** (dependency manager)
   ```powershell
   cd C:\
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg install expat:x64-windows
   .\vcpkg integrate install
   ```

3. **Git for Windows** (optional)

#### CMake Build with Visual Studio 2022

**Step 1: Configure**
```powershell
cd path\to\asterix

cmake -B build ^
  -G "Visual Studio 17 2022" ^
  -A x64 ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_CXX_STANDARD=20 ^
  -DCMAKE_CXX_STANDARD_REQUIRED=ON ^
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" ^
  -DBUILD_SHARED_LIBS=ON ^
  -DBUILD_STATIC_LIBS=ON ^
  -DBUILD_EXECUTABLE=ON ^
  -DCMAKE_INSTALL_PREFIX="%CD%\install"
```

**Step 2: Build**
```powershell
# Build Release configuration
cmake --build build --config Release --parallel 4

# Or open in Visual Studio
start build\asterix.sln
```

**Step 3: Install**
```powershell
cmake --install build --config Release
```

**Step 4: Test**
```powershell
.\install\bin\asterix.exe --version
.\install\bin\asterix.exe --help
```

#### Method 2: Using Visual Studio IDE

1. Open Visual Studio 2022
2. Select "Open a local folder" → navigate to ASTERIX repository
3. CMake integration will auto-configure
4. Select build configuration (Release/Debug)
5. Build → Build All
6. Output in `out/build/x64-Release/`

#### Known Issues (Windows)

**⚠️ Warning: Networking Code Incompatibility**

Unix-style socket headers (`sys/socket.h`, `arpa/inet.h`) are not available on Windows.

**Affected Files:**
- `src/engine/UdpDevice.cpp`
- `src/engine/TcpDevice.cpp`

**Status:** TCP/UDP device classes will not compile on Windows

**Workaround 1: Disable Network Devices**

Add to CMakeLists.txt:
```cmake
if(NOT WIN32)
  list(APPEND ASTERIX_LIB_SOURCES
    src/engine/tcpdevice.cxx
    src/engine/udpdevice.cxx
  )
endif()
```

**Workaround 2: Use Winsock2 (Requires Code Refactoring)**

```cpp
#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")

  // Type compatibility
  typedef int socklen_t;
  #define close closesocket
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
#endif
```

**Issue: Hardening Flags (GCC-specific)**

CMakeLists.txt contains GCC-specific flags that need MSVC equivalents.

**Required Changes:**
```cmake
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,relro,-z,now")
elseif(MSVC)
    # MSVC equivalent flags
    add_compile_options(/GS /sdl)
endif()
```

#### Troubleshooting (Windows)

**Issue: "Cannot find expat"**

**Solution:** Ensure vcpkg integration is complete:
```powershell
cd C:\vcpkg
.\vcpkg integrate install
```

**Issue: Build fails with socket errors**

**Solution:** Disable network device compilation (see Known Issues above)

---

### 3.3 macOS

#### Prerequisites (macOS-Specific)

1. **Xcode 15.0 or later** (15.4 recommended for ARM64/M1)
2. **macOS 11.0+** (deployment target)
3. **Homebrew** (recommended)

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew update
brew install cmake expat
```

#### CMake Build

**Step 1: Configure for Apple Silicon (M1/M2/M3)**
```bash
cd /path/to/asterix

cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_EXECUTABLE=ON \
  -DCMAKE_INSTALL_PREFIX=$PWD/install
```

**Step 1 (Alternative): Configure for Intel Macs**
```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_EXECUTABLE=ON \
  -DCMAKE_INSTALL_PREFIX=$PWD/install
```

**Step 1 (Alternative): Configure for Universal Binary (Both Architectures)**
```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_EXECUTABLE=ON \
  -DCMAKE_INSTALL_PREFIX=$PWD/install
```

**Step 2: Build**
```bash
cmake --build build --config Release --parallel $(sysctl -n hw.ncpu)
```

**Step 3: Install**
```bash
cmake --install build --config Release
```

**Step 4: Test**
```bash
./install/bin/asterix --version
./install/bin/asterix --help
```

#### Known Issues (macOS)

**Platform Compatibility:**
- ✅ The code already handles macOS differences (e.g., `MSG_NOSIGNAL` → `SO_NOSIGPIPE`)
- ✅ Socket headers use Unix-style APIs (compatible)

**⚠️ Deployment Target:**
Set minimum macOS version to 11.0+ for C++23 features

**⚠️ Hardening Flags:**
Some Linux-specific flags don't apply:
- `-Wl,-z,relro,-z,now` → Not applicable on macOS linker
- Stack protection works differently but is enabled by default

**Recommended CMake Updates:**
```cmake
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    # Remove Linux-specific linker flags
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
endif()
```

#### Troubleshooting (macOS)

**Issue: "unknown type name 'fd_set'"**

**Solution:** Include `<sys/select.h>` (already present in code)

**Issue: Library not found errors**

**Solution:**
```bash
brew install expat
# If needed, set library paths:
export LDFLAGS="-L/opt/homebrew/lib"
export CPPFLAGS="-I/opt/homebrew/include"
```

---

## 4. Language Bindings

### 4.1 C++ Executable

The C++ executable provides a high-performance command-line tool for parsing ASTERIX data.

#### Build with CMake

> **Note**: GNU Make build files were removed in v2.8.10. CMake is now the only supported build system.

See platform-specific instructions:
- **Linux**: [Section 3.1 Linux](#31-linux)
- **Windows**: [Section 3.2 Windows](#32-windows)
- **macOS**: [Section 3.3 macOS](#33-macos)

**Quick Build (all platforms):**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build
```

**Output Locations:**
- Executable: `install/bin/asterix`
- Configuration: `install/share/asterix/config/`
- Test scripts: `install/test/`

#### C++ Standard Configuration

The project uses different C++ standards based on platform:

| Platform | CMake Build | Python Module Build |
|----------|-------------|---------------------|
| **Linux/macOS** | C++23 | C++23 |
| **Windows/MSVC** | C++20 | C++20 |
| **macOS (Python)** | C++23 | C++17* |

*C++17 for Python module on macOS ensures better compiler compatibility with older systems.

**Automatic Fallback:**
The codebase includes `src/asterix/cxx23_features.h` which automatically detects available features and gracefully falls back to C++17/20 when C++23 is not available.

#### Running the Executable

**Basic Usage:**
```bash
# Help
./install/bin/asterix --help

# Parse file
./install/bin/asterix -f sample_data/cat048.pcap -P

# Parse with JSON output
./install/bin/asterix -f sample_data/cat048.pcap -P -j

# Parse multicast stream
./install/bin/asterix -i 239.0.0.1:eth0:5000
```

**Input Sources:**
- `-f filename`: File input (PCAP, raw, FINAL, HDLC, GPS)
- `-i m:i:p[:s]`: Multicast (format: mcast_addr:interface_ip:port[:source])
- stdin: Default if no source specified

**Input Format Flags:**
- `-P`: PCAP format
- `-R`: ORADIS PCAP format
- `-O`: ORADIS format
- `-F`: FINAL format
- `-H`: HDLC format
- `-G`: GPS format

**Output Format Flags:**
- Default: Human-readable text
- `-l, --line`: One line per item (parsable)
- `-x, --xml`: Compact XML (one object per line)
- `-xh, --xmlh`: Human-readable XML
- `-j, --json`: Compact JSON (one object per line)
- `-jh, --jsonh`: Human-readable JSON
- `-je, --json-extensive`: Extensive JSON with descriptions

---

### 4.2 Python Module

The Python module wraps the C++ parser via a C extension, providing a high-level API.

#### Prerequisites

- Python 3.8+ (3.10+ recommended, supports 3.10-3.14)
- pip, setuptools, wheel
- libexpat development headers
- C++ compiler (same as C++ executable)

#### Setup Virtual Environment (Recommended)

```bash
# Create virtual environment
python3 -m venv .venv

# Activate virtual environment
source .venv/bin/activate  # Linux/macOS
# OR
.venv\Scripts\activate  # Windows

# Upgrade pip
pip install --upgrade pip setuptools wheel
```

#### Install Development Dependencies

```bash
# Install all development tools
pip install -r requirements-dev.txt
```

This installs:
- pytest (testing framework)
- pytest-cov (coverage plugin)
- coverage (coverage reporting)
- black (code formatter)
- pylint (linter)
- flake8 (style checker)
- memory-profiler (memory analysis)
- lxml (XML support)

#### Build and Install

**Standard Installation:**
```bash
# Build the C++ extension
python setup.py build

# Install
python setup.py install
```

**Development Mode (Recommended for Development):**
```bash
# Install in editable mode
pip install -e .
```

**Windows-Specific:**
```powershell
# Set vcpkg toolchain before building
$env:CMAKE_TOOLCHAIN_FILE = "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
python setup.py build
python setup.py install
```

#### Install from PyPI

```bash
pip install asterix_decoder
```

#### Python API Usage

```python
import asterix

# Parse raw ASTERIX bytes
data = open('sample_data/cat048.asterix', 'rb').read()
records = asterix.parse(data)

for record in records:
    print(f"Category: {record['category']}")
    print(f"Items: {record['items']}")

# Incremental parsing (for streaming)
offset = 0
blocks_count = 1
while offset < len(data):
    records, offset = asterix.parse_with_offset(data, offset, blocks_count)
    for record in records:
        process_record(record)

# Get descriptions
desc = asterix.describe(category=48, item=220, field=None, value=None)
print(desc)

# Load custom category definition
asterix.init('path/to/custom/asterix_cat999.xml')
```

#### Running Python Tests

```bash
# Activate virtual environment
source .venv/bin/activate

# Run tests
pytest asterix/test/ tests/python/ -v

# Run tests with coverage
pytest asterix/test/ tests/python/ -v --cov=asterix --cov-report=html

# Or use unittest
python -m unittest discover -s asterix/test -p 'test_*.py'

# Integration tests
cd install/test
./test.sh
```

**Expected Results:**
- 60 tests passing
- 88% coverage

#### Building Python Wheels

```bash
# Install build tools
pip install build wheel

# Build wheel
python -m build --wheel

# Install wheel
pip install dist/asterix_decoder-*.whl
```

#### Platform-Specific Python Notes

**setup.py Hardening Flags:**

The current `setup.py` uses GCC-specific flags. For cross-platform support:

```python
import platform
import sys

extra_compile_args = ['-DPYTHON_WRAPPER', '-std=c++17']
extra_link_args = []

if sys.platform == 'win32':
    extra_compile_args.extend(['/GS', '/sdl'])
    extra_link_args.append('expat.lib')
elif sys.platform == 'darwin':
    extra_compile_args.extend(['-fstack-protector-strong'])
    extra_link_args.append('-lexpat')
else:  # Linux
    extra_compile_args.extend(['-fstack-protector-strong', '-D_FORTIFY_SOURCE=2'])
    extra_link_args.extend(['-lexpat', '-Wl,-z,relro,-z,now'])
```

#### Troubleshooting (Python)

**Issue: "No module named '_asterix'"**

**Solution:** Rebuild and install Python extension
```bash
source .venv/bin/activate
python setup.py build
python setup.py install
```

**Issue: "pytest: command not found"**

**Solution:** Activate virtual environment
```bash
source .venv/bin/activate
```

**Issue: Build fails with compiler errors**

**Solution:** Ensure C++ compiler and expat headers are available
```bash
# Linux
sudo apt-get install build-essential libexpat1-dev

# macOS
brew install expat

# Windows
vcpkg install expat:x64-windows
```

---

### 4.3 Rust Crate

The Rust crate provides type-safe, memory-safe bindings with zero-copy performance.

#### Prerequisites

- Rust 1.70+ (2021 edition)
- C++17 compatible compiler (same as C++ executable)
- libexpat development headers
- Cargo build system

#### Installation

**Install Rust:**
```bash
# Linux/macOS
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# Windows
# Download and run rustup-init.exe from https://rustup.rs/
```

**Verify Installation:**
```bash
rustc --version  # Should be 1.70.0 or higher
cargo --version  # Should be 1.70.0 or higher
```

#### Directory Structure

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

#### Building

```bash
cd asterix-rs

# Debug build
cargo build

# Production build
cargo build --release

# Run tests
cargo test --all-features

# Run benchmarks
cargo bench

# Run examples
cargo run --example parse_file
```

#### FFI Architecture

- Uses **CXX crate** for safe C++/Rust interop
- Build script (`build.rs`) invokes CMake to compile C++ core
- Type-safe FFI bridge eliminates manual unsafe code
- Memory ownership: Data copied to Rust side (owned `Vec<AsterixRecord>`)

#### Rust API Usage

```rust
use asterix_decoder::{Parser, ParseOptions};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize with default categories
    let parser = Parser::new()
        .init_default()?
        .build()?;

    // Parse raw bytes
    let data = std::fs::read("sample_data/cat048.asterix")?;
    let options = ParseOptions::default();
    let records = parser.parse(&data, &options)?;

    for record in records {
        println!("Category: {}", record.category);
        for item in record.items {
            println!("  Item {}: {:?}", item.id, item.value);
        }
    }

    Ok(())
}
```

**Incremental Parsing:**
```rust
let mut offset = 0;
let blocks_count = 1;

while offset < data.len() {
    let (records, new_offset) = parser.parse_with_offset(
        &data,
        offset,
        blocks_count,
        &options
    )?;

    for record in records {
        process_record(record);
    }

    offset = new_offset;
}
```

**Custom Categories:**
```rust
let parser = Parser::new()
    .add_category("path/to/asterix_cat999.xml")?
    .build()?;
```

#### Running Rust Tests

```bash
cd asterix-rs

# Unit + integration tests
cargo test

# Documentation tests
cargo test --doc

# All features
cargo test --all-features

# Performance benchmarks
cargo bench

# Code quality
cargo clippy
cargo fmt --check
```

#### Publishing to crates.io

```bash
cd asterix-rs
cargo publish
```

#### Using in Your Rust Project

**Add to Cargo.toml:**
```toml
[dependencies]
asterix-decoder = "0.1.0"
```

**Or via command line:**
```bash
cargo add asterix-decoder
```

---

## 5. C++23 Features (Advanced)

The ASTERIX codebase has been upgraded to C++23 (from C++17) with automatic fallback for older compilers. This section documents C++23 features, migration patterns, and backward compatibility.

### 5.1 Feature Detection Header

**File:** `src/asterix/cxx23_features.h`

This header provides compile-time feature detection and graceful fallback to C++17/20.

```cpp
/*
 * C++23 Feature Detection and Compatibility Layer
 * Allows graceful fallback to C++20/17 if needed
 */

#ifndef CXX23_FEATURES_H_
#define CXX23_FEATURES_H_

#include <version>  // C++20 <version> header

// C++23 Feature Availability
#if __cplusplus >= 202302L
    #define CXX23_ENABLED 1
    #define HAS_DEDUCED_THIS 1
    #define HAS_RANGES_ALGORITHMS 1
    #define HAS_FORMAT 1
    #define HAS_STRING_VIEW 1
    #define HAS_EXPLICIT_THIS 1
#else
    #define CXX23_ENABLED 0
    #define HAS_DEDUCED_THIS 0
    #define HAS_RANGES_ALGORITHMS 0
    #define HAS_FORMAT 0
    #define HAS_STRING_VIEW 1  // C++17 has string_view
    #define HAS_EXPLICIT_THIS 0
    #warning "C++23 not detected; falling back to C++20/17 compatibility mode"
#endif

// Utility macros for conditional code
#if HAS_DEDUCED_THIS
    #define DEDUCED_THIS(cls) cls
#else
    #define DEDUCED_THIS(cls)  // Empty for older standards
#endif

#if HAS_RANGES_ALGORITHMS
    #include <ranges>
    #include <algorithm>
    namespace asterix {
        namespace ranges = std::ranges;
    }
#else
    // Fallback using iterator-based approach
    namespace asterix {
        // Define compatibility layer if needed
    }
#endif

#if HAS_FORMAT
    #include <format>
    namespace asterix {
        using std::format;
        using std::format_to;
    }
#else
    // Fallback to sprintf for format
    #include <cstdio>
    #include <string>
    namespace asterix {
        inline std::string format(const char* fmt, ...) {
            // Fallback implementation using sprintf
            char buffer[1024];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buffer, sizeof(buffer), fmt, args);
            va_end(args);
            return std::string(buffer);
        }
    }
#endif

// Compiler-specific attributes
#if defined(__GNUC__) || defined(__clang__)
    #define ASTERIX_NODISCARD [[nodiscard]]
    #define ASTERIX_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(_MSC_VER)
    #define ASTERIX_NODISCARD [[nodiscard]]
    #define ASTERIX_DEPRECATED(msg) [[deprecated(msg)]]
#else
    #define ASTERIX_NODISCARD
    #define ASTERIX_DEPRECATED(msg)
#endif

#endif  // CXX23_FEATURES_H_
```

### 5.2 Deduced This (Explicit Object Parameters)

**Purpose:** Better polymorphic performance and cleaner virtual function syntax

**File:** `src/asterix/DataItemFormat.h`

**Before (C++17):**
```cpp
class DataItemFormat {
public:
    // Traditional virtual function
    virtual DataItemFormat *clone() const = 0;

    // Type check methods
    virtual bool isFixed() { return false; }
    virtual bool isVariable() { return false; }
    virtual bool isCompound() { return false; }
};
```

**After (C++23):**
```cpp
#include "cxx23_features.h"

class DataItemFormat {
public:
    #if HAS_DEDUCED_THIS
    // C++23: Deduced this for better polymorphic support
    virtual DataItemFormat *clone(this const auto& self) const = 0;

    // Template members with deduced this
    template<typename Self>
    bool checkFormat(this Self&& self, int typeMask) const {
        return (self.getTypeMask() & typeMask) != 0;
    }
    #else
    // C++20/17 fallback: Traditional approach
    virtual DataItemFormat *clone() const = 0;
    bool checkFormat(int typeMask) const {
        return (getTypeMask() & typeMask) != 0;
    }
    #endif

    // Type mask helpers
    enum TypeMask : int {
        FIXED = 1,
        VARIABLE = 2,
        COMPOUND = 4,
        REPETITIVE = 8,
        EXPLICIT = 16,
        BDS = 32,
        BITS = 64
    };

    virtual int getTypeMask() const { return 0; }
};
```

**Derived Class Implementation:**
```cpp
#include "cxx23_features.h"

class DataItemFormatFixed : public DataItemFormat {
public:
    #if HAS_DEDUCED_THIS
    DataItemFormat *clone(this const auto& self) const override {
        return new DataItemFormatFixed(self);
    }
    #else
    DataItemFormat *clone() const override {
        return new DataItemFormatFixed(*this);
    }
    #endif

    // Type mask for this format
    int getTypeMask() const override { return FIXED; }
};
```

### 5.3 Ranges Algorithms

**Purpose:** More expressive, composable operations on containers

**File:** `src/asterix/DataItemFormatVariable.cpp`

**Before (C++17):**
```cpp
DataItemFormatVariable::DataItemFormatVariable(const DataItemFormatVariable &obj)
        : DataItemFormat(obj.m_nID) {
    // Manual iterator-based copy
    std::list<DataItemFormat *>::iterator it =
        ((DataItemFormat &) obj).m_lSubItems.begin();

    while (it != obj.m_lSubItems.end()) {
        DataItemFormat *di = (DataItemFormat *) (*it);
        m_lSubItems.push_back(di->clone());
        it++;
    }

    m_pParentFormat = obj.m_pParentFormat;
}
```

**After (C++23):**
```cpp
#include "cxx23_features.h"

DataItemFormatVariable::DataItemFormatVariable(const DataItemFormatVariable &obj)
        : DataItemFormat(obj.m_nID), m_pParentFormat(obj.m_pParentFormat) {

    #if HAS_RANGES_ALGORITHMS
    // C++23: Ranges-based transformation
    std::ranges::transform(
        obj.m_lSubItems,
        std::back_inserter(m_lSubItems),
        [](const DataItemFormat* item) { return item->clone(); }
    );
    #else
    // C++17: Iterator-based approach (fallback)
    for (auto item : obj.m_lSubItems) {
        m_lSubItems.push_back(item->clone());
    }
    #endif
}
```

**Filtered Iteration Example:**

**Before (C++17):**
```cpp
// Count variable-format items
int varCount = 0;
std::list<DataItem *>::iterator it;
for (it = m_lDataItems.begin(); it != m_lDataItems.end(); it++) {
    DataItem *di = (DataItem *)(*it);
    if (di != NULL && di->m_pDescription != NULL) {
        DataItemFormat *fmt = di->m_pDescription->m_pFormat;
        if (fmt != NULL && fmt->isVariable()) {
            varCount++;
        }
    }
}
```

**After (C++23):**
```cpp
#include "cxx23_features.h"

// Count variable-format items
#if HAS_RANGES_ALGORITHMS
auto variableItems = m_lDataItems
    | std::views::filter([](const DataItem* di) {
        return di != nullptr && di->m_pDescription != nullptr;
    })
    | std::views::filter([](const DataItem* di) {
        auto fmt = di->m_pDescription->m_pFormat;
        return fmt != nullptr && fmt->isVariable();
    });

int varCount = std::ranges::distance(variableItems);
#else
// C++17: Traditional approach
int varCount = 0;
for (auto di : m_lDataItems) {
    if (di && di->m_pDescription && di->m_pDescription->m_pFormat &&
        di->m_pDescription->m_pFormat->isVariable()) {
        varCount++;
    }
}
#endif
```

### 5.4 std::format Integration

**Purpose:** Type-safe, efficient string formatting

**File:** `src/asterix/Utils.cpp`

**Before (C++17):**
```cpp
#include <cstdio>
#include <cstdarg>
#include <string>

std::string format(const char *fmt, ...) {
    // Use fixed-size buffer (dangerous!)
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return std::string(buffer);
}

// Usage
std::string msg = format("Category %03d, %d bytes", cat, len);
```

**After (C++23):**
```cpp
#include "cxx23_features.h"
#include <string>

// C++23: Using std::format
#if HAS_FORMAT
// For dynamic formatting
template<typename... Args>
std::string format(std::format_string<Args...> fmt, Args&&... args) {
    return std::format(fmt, std::forward<Args>(args)...);
}

// Usage with type checking
std::string msg = format("Category {:03d}, {} bytes", cat, len);
#else
// C++17: Fallback to sprintf
std::string format(const char *fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return std::string(buffer);
}
#endif
```

**Output Formatting Example:**

**Before (C++17):**
```cpp
// Building output strings (inefficient)
std::string result = "";
result += "ASTERIX CAT ";
result += std::to_string(category);
result += " Records: ";
result += std::to_string(recordCount);
result += " (";
result += std::to_string(totalBytes);
result += " bytes)";
```

**After (C++23):**
```cpp
#include "cxx23_features.h"

#if HAS_FORMAT
// C++23: Type-safe formatting
std::string result = std::format(
    "ASTERIX CAT {:03d} Records: {:5d} ({:8d} bytes)",
    category,
    recordCount,
    totalBytes
);
#else
// C++17: Traditional concatenation (reserve for efficiency)
std::string result;
result.reserve(50);
result += "ASTERIX CAT ";
result += std::to_string(category);
result += " Records: ";
result += std::to_string(recordCount);
result += " (";
result += std::to_string(totalBytes);
result += " bytes)";
#endif
```

### 5.5 String View (Zero-Copy Operations)

**Purpose:** Avoid unnecessary string copies

**Available in C++17+**

**Before (C++17):**
```cpp
// Creates substring copy
std::string getId(const std::string& fullId) {
    size_t pos = fullId.find('/');
    if (pos != std::string::npos) {
        return fullId.substr(pos + 1);  // Creates copy!
    }
    return fullId;
}
```

**After (C++23):**
```cpp
#include "cxx23_features.h"

// Use string_view for zero-copy substring (C++17+)
std::string_view getId(std::string_view fullId) {
    size_t pos = fullId.find('/');
    if (pos != std::string_view::npos) {
        return fullId.substr(pos + 1);  // No copy!
    }
    return fullId;
}

// Usage
std::string_view id = getId("ASTERIX/062");  // Points into original string
```

### 5.6 Build Configuration for C++23

**CMakeLists.txt:**

```cmake
cmake_minimum_required(VERSION 3.20)
project(asterix
    VERSION 2.9.0
    DESCRIPTION "ASTERIX protocol decoder library and tools"
    LANGUAGES C CXX)

# C++ Standard Configuration
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# C Standard Configuration (Optional - minimal upgrade)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler Version Checking for C++23
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
        message(WARNING
            "GCC 13.0+ is recommended for C++23 support. "
            "You are using GCC ${CMAKE_CXX_COMPILER_VERSION}. "
            "Some C++23 features may not work correctly.")
    endif()
    # Enable specific C++23 flags if needed
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0")
        message(WARNING
            "Clang 16.0+ is recommended for C++23 support. "
            "You are using Clang ${CMAKE_CXX_COMPILER_VERSION}.")
    endif()
    # Clang needs libc++ for full C++23 support
    if(NOT APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
    endif()

elseif(MSVC)
    if(MSVC_VERSION LESS 1930)  # VS 2022 v17.0
        message(WARNING
            "MSVC 2022 v17.4+ is recommended for C++23 support. "
            "You are using MSVC ${MSVC_VERSION}.")
    endif()
    # Use /std:c++latest for latest C++ standard
    string(REGEX REPLACE "/std:c\\+\\+[^ ]*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif()

# Add feature detection header to all targets
add_compile_definitions("$<$<COMPILE_LANGUAGE:CXX>:HAS_CXX23=1>")
```

**Makefile (makefile.include):**

```makefile
# C++23 Standard (from C++17)
CXXSTD := -std=c++23

# C23 Standard (optional upgrade from C17)
CSTD := -std=c23

# Add C++23 specific flags for compilers that need them
ifeq ($(CXX),g++)
    CXXSTD := -std=c++23 -fconcepts
endif

ifeq ($(CXX),clang++)
    CXXSTD := -std=c++23 -stdlib=libc++
endif
```

### 5.7 Compiler-Specific Warnings

**GCC 13+ Configuration:**
```bash
CXXFLAGS = -std=c++23 -fconcepts -Wall -Wextra -Wpedantic \
           -Wno-unused-parameter \
           -Wno-deprecated-declarations
```

**Clang 16+ Configuration:**
```bash
CXXFLAGS = -std=c++23 -stdlib=libc++ -Wall -Wextra -Wpedantic \
           -Wno-c++20-extensions \
           -Wno-reserved-identifier
```

**MSVC 2022 Configuration:**
```bash
CXXFLAGS = /std:c++latest /W4 /permissive- /Zc:inline
```

---

## 6. Testing & Quality

### 6.1 Running Tests

#### C++ Integration Tests

```bash
# From install/test directory
cd install/test
./test.sh

# Expected output: 11/11 tests passing
```

**Test Categories:**
- PCAP parsing (cat048.pcap)
- Raw ASTERIX parsing
- JSON/XML output formatting
- Error handling
- Memory leak testing

#### Python Tests

```bash
# Activate virtual environment
source .venv/bin/activate

# Run tests with pytest
pytest asterix/test/ tests/python/ -v

# Run tests with coverage
pytest asterix/test/ tests/python/ -v --cov=asterix --cov-report=html

# Or use unittest
python -m unittest discover -s asterix/test -p 'test_*.py'

# Expected output: 60 tests passing, 88% coverage
```

#### Rust Tests

```bash
cd asterix-rs

# Unit + integration tests
cargo test

# All features
cargo test --all-features

# Documentation tests
cargo test --doc

# Performance benchmarks
cargo bench
```

### 6.2 Code Coverage

#### C++ Coverage with lcov

**Prerequisites:**
```bash
sudo apt-get install -y lcov
```

**Generate Coverage Report:**

```bash
# Step 1: Configure with coverage enabled
cmake -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_COVERAGE=ON \
  -DBUILD_TESTING=ON

# Step 2: Build
cmake --build build --parallel $(nproc)

# Step 3: Run tests
cd build
ctest --output-on-failure -V

# Step 4: Capture coverage data
lcov --capture --directory . --output-file coverage.info

# Step 5: Remove system/external library coverage
lcov --remove coverage.info '/usr/*' '*/googletest/*' '*/test/*' \
  --output-file coverage_filtered.info

# Step 6: Generate HTML report
genhtml coverage_filtered.info --output-directory coverage_html

# Step 7: View report
# Open build/coverage_html/index.html in browser
```

**Coverage Thresholds:**
- **Target:** 85% line coverage
- **Current:** ~75% (as of 2025-10-20)

#### Python Coverage

```bash
# Activate virtual environment
source .venv/bin/activate

# Run tests with coverage
pytest asterix/test/ tests/python/ -v --cov=asterix --cov-report=html

# Coverage report saved to htmlcov/index.html

# Or generate terminal report
pytest --cov=asterix --cov-report=term-missing
```

**Expected Coverage:** 88%

#### Rust Coverage

```bash
cd asterix-rs

# Install tarpaulin (coverage tool for Rust)
cargo install cargo-tarpaulin

# Generate coverage report
cargo tarpaulin --all-features --out Html --output-dir target/coverage

# View report at target/coverage/index.html
```

### 6.3 Memory Testing

#### Valgrind (Linux/macOS Only)

**Prerequisites:**
```bash
sudo apt-get install -y valgrind
```

**Run Memory Leak Tests:**

```bash
# From install/test directory
cd install/test
./valgrind_test.sh

# Or manually:
valgrind --leak-check=full --error-exitcode=1 \
  ./install/bin/asterix -P -f sample_data/cat048.pcap

# Expected output: 0 bytes lost
```

**Memory Testing for Python Module:**

```bash
# Install memory-profiler
pip install memory-profiler

# Run with profiling
python -m memory_profiler asterix/test/test_parsing.py
```

#### Windows Alternative (Dr. Memory)

```powershell
# Download Dr. Memory from https://drmemory.org/

# Run test
drmemory.exe -- .\install\bin\asterix.exe -P -f sample_data\cat048.pcap
```

### 6.4 Code Quality Tools

#### Python Code Quality

**Black (Code Formatter):**
```bash
source .venv/bin/activate

# Format code
black asterix/ tests/python/ --line-length 100

# Check formatting without changes
black asterix/ tests/python/ --check --diff
```

**Pylint (Linter):**
```bash
source .venv/bin/activate

# Run pylint
pylint asterix/*.py
```

**Flake8 (Style Checker):**
```bash
source .venv/bin/activate

# Run flake8
flake8 asterix/ tests/python/ --max-line-length=100
```

#### Rust Code Quality

```bash
cd asterix-rs

# Clippy (Linter)
cargo clippy

# Format check
cargo fmt --check

# Format code
cargo fmt
```

#### C++ Code Quality

**Clang-Tidy (Static Analysis):**
```bash
# Generate compile_commands.json
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Run clang-tidy
clang-tidy src/asterix/*.cpp -p build/compile_commands.json
```

**Cppcheck (Static Analysis):**
```bash
sudo apt-get install cppcheck

# Run cppcheck
cppcheck --enable=all --suppress=missingIncludeSystem src/asterix/
```

### 6.5 Continuous Integration

See `.github/workflows/cross-platform-builds.yml` for automated builds across:
- Windows 2019/2022 (MSVC)
- macOS 13 (Intel) / 14 (ARM)
- Ubuntu 22.04 / 24.04

**CI Pipeline Stages:**
1. Build (all platforms)
2. Unit tests
3. Integration tests
4. Code coverage
5. Memory leak detection (Linux only)
6. Package creation

---

## 7. Packaging

### 7.1 Linux Packages

#### DEB Package (Debian/Ubuntu)

```bash
cd build

# Build DEB package
cpack -G DEB

# Install package
sudo dpkg -i asterix-2.8.9-Linux.deb

# Verify installation
asterix --version
```

**Package Contents:**
- `/usr/local/bin/asterix`
- `/usr/local/share/asterix/config/`
- `/usr/local/share/doc/asterix/`

#### RPM Package (RHEL/Fedora/Rocky)

```bash
cd build

# Build RPM package
cpack -G RPM

# Install package
sudo rpm -i asterix-2.8.9-Linux.rpm

# Or use dnf/yum
sudo dnf install asterix-2.8.9-Linux.rpm

# Verify installation
asterix --version
```

#### Generic Tarballs

```bash
cd build

# TGZ archive
cpack -G TGZ

# ZIP archive
cpack -G ZIP
```

### 7.2 macOS Packages

#### DMG Image (Recommended for Distribution)

**Prerequisites:**
```bash
brew install create-dmg
```

**Create DMG:**
```bash
create-dmg \
  --volname "ASTERIX Decoder" \
  --volicon "resources/icon.icns" \
  --window-pos 200 120 \
  --window-size 600 400 \
  --icon-size 100 \
  --icon "asterix" 175 120 \
  --hide-extension "asterix" \
  --app-drop-link 425 120 \
  "asterix-2.8.9.dmg" \
  "install/"
```

#### PKG Installer

```bash
productbuild \
  --component install/bin/asterix /usr/local/bin \
  --component install/share/asterix /usr/local/share \
  asterix-2.8.9.pkg
```

#### Code Signing (Production)

```bash
# Sign the executable
codesign --force --options runtime \
  --sign "Developer ID Application: Your Name" \
  install/bin/asterix

# Sign the DMG
codesign --force \
  --sign "Developer ID Application: Your Name" \
  asterix-2.8.9.dmg

# Notarize with Apple (requires Apple Developer account)
xcrun notarytool submit asterix-2.8.9.dmg \
  --keychain-profile "AC_PASSWORD"

xcrun stapler staple asterix-2.8.9.dmg
```

#### Generic Archives

```bash
cd build

# TGZ archive
cpack -G TGZ

# ZIP archive
cpack -G ZIP
```

### 7.3 Windows Packages

#### ZIP Archive

```bash
cd build

# ZIP archive
cpack -G ZIP -C Release
```

#### NSIS Installer

**Prerequisites:** Install [NSIS](https://nsis.sourceforge.io/)

```bash
cd build

# Build NSIS installer
cpack -G NSIS -C Release
```

#### WiX MSI Installer

**Prerequisites:** Install [WiX Toolset v3.11+](https://wixtoolset.org/)

```bash
cd build

# Build MSI installer
cpack -G WIX -C Release
```

**Manual WiX Build:**
```powershell
# Create WiX configuration (asterix.wxs)
candle asterix.wxs

# Build MSI
light -out asterix.msi asterix.wixobj
```

### 7.4 Python Package Distribution

#### Build Python Wheel

```bash
# Install build tools
pip install build wheel twine

# Build wheel
python -m build --wheel

# Build source distribution
python -m build --sdist

# Verify wheel
twine check dist/*
```

#### Upload to PyPI

```bash
# Test PyPI (for testing)
twine upload --repository testpypi dist/*

# Production PyPI
twine upload dist/*
```

### 7.5 Rust Crate Publishing

#### Prepare for Publishing

```bash
cd asterix-rs

# Update version in Cargo.toml
# Update CHANGELOG.md
# Update README.md

# Run tests
cargo test --all-features

# Check package
cargo package --allow-dirty

# Verify package contents
cargo package --list
```

#### Publish to crates.io

```bash
# Login (first time only)
cargo login

# Publish
cargo publish
```

---

## 8. Appendices

### Appendix A: Troubleshooting

#### A.1 Common Build Errors

**Error: "Cannot find expat"**

**Platforms:** All

**Solutions:**
```bash
# Linux (Debian/Ubuntu)
sudo apt-get install libexpat1-dev

# Linux (RHEL/Fedora)
sudo dnf install expat-devel

# macOS
brew install expat

# Windows
vcpkg install expat:x64-windows
```

**Error: "C++ compiler does not support C++23"**

**Platforms:** All

**Solutions:**
- Upgrade compiler (see [Section 2.4](#24-compiler-upgrade-guide))
- Build will automatically fall back to C++17 via `cxx23_features.h`

**Error: "CMake version too old"**

**Platforms:** Linux

**Solutions:**
```bash
# Ubuntu 20.04/22.04 - install from Kitware repository
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
  gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt-get update
sudo apt-get install cmake
```

**Error: "No module named '_asterix'" (Python)**

**Platforms:** All

**Solutions:**
```bash
source .venv/bin/activate
python setup.py build
python setup.py install
```

**Error: "pytest: command not found"**

**Platforms:** All

**Solutions:**
```bash
source .venv/bin/activate
pip install pytest pytest-cov
```

**Error: Network device compilation fails (Windows)**

**Platforms:** Windows

**Solutions:** See [Section 3.2 Known Issues](#known-issues-windows)

#### A.2 Performance Issues

**Issue: Slow build times**

**Solutions:**
- Use Ninja instead of Make: `cmake -B build -G Ninja`
- Increase parallel jobs: `make -j$(nproc)` or `cmake --build build --parallel`
- Use ccache: `sudo apt-get install ccache && export CXX="ccache g++"`

**Issue: Slow parsing performance**

**Solutions:**
- Build with Release configuration: `cmake -DCMAKE_BUILD_TYPE=Release`
- Enable compiler optimizations: `-O3 -march=native`
- Use C++23 ranges (see [Section 5.3](#53-ranges-algorithms))

**Issue: High memory usage**

**Solutions:**
- Use incremental parsing: `parse_with_offset()`
- Reduce buffer sizes in configuration
- Profile with valgrind/Dr. Memory

#### A.3 Testing Issues

**Issue: Tests fail with segmentation fault**

**Solutions:**
- Build with debug symbols: `make debug`
- Run with valgrind: `valgrind --leak-check=full ./install/bin/asterix`
- Check for null pointer dereferences
- Verify ASTERIX configuration files are present

**Issue: Coverage shows 0%**

**Solutions:**
```bash
cd build
cmake .. -DENABLE_COVERAGE=ON -DBUILD_TESTING=ON
make clean
make -j$(nproc)
ctest
lcov --capture --directory . -o coverage.info
```

---

### Appendix B: CMake Reference

#### B.1 Common CMake Options

| Option | Values | Description |
|--------|--------|-------------|
| `CMAKE_BUILD_TYPE` | Debug, Release, RelWithDebInfo, MinSizeRel | Build configuration |
| `CMAKE_CXX_STANDARD` | 17, 20, 23 | C++ standard version |
| `CMAKE_CXX_COMPILER` | g++, clang++, cl.exe | C++ compiler |
| `CMAKE_C_COMPILER` | gcc, clang, cl.exe | C compiler |
| `CMAKE_INSTALL_PREFIX` | Path | Installation directory |
| `BUILD_SHARED_LIBS` | ON/OFF | Build shared libraries |
| `BUILD_STATIC_LIBS` | ON/OFF | Build static libraries |
| `BUILD_EXECUTABLE` | ON/OFF | Build executable |
| `BUILD_TESTING` | ON/OFF | Build tests |
| `ENABLE_COVERAGE` | ON/OFF | Enable code coverage |
| `CMAKE_TOOLCHAIN_FILE` | Path | Toolchain file (e.g., vcpkg) |

#### B.2 Example CMake Configurations

**Debug Build with Tests:**
```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_STANDARD=23 \
  -DBUILD_TESTING=ON \
  -DENABLE_COVERAGE=ON
```

**Release Build for Distribution:**
```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON
```

**Windows Build with vcpkg:**
```bash
cmake -B build \
  -G "Visual Studio 17 2022" \
  -A x64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

---

### Appendix C: Known Platform Issues

#### C.1 Networking Code (Windows Incompatibility)

**Problem:** Unix socket headers not available on Windows

**Affected Files:**
- `src/engine/UdpDevice.cpp`
- `src/engine/UdpDevice.hxx`
- `src/engine/TcpDevice.cpp`
- `src/engine/TcpDevice.hxx`

**Current Code:**
```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
```

**Required Changes for Windows:**
```cpp
#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")

  // Type compatibility
  typedef int socklen_t;
  #define close closesocket
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
#endif
```

**Workaround:** Disable network device compilation on Windows (see [Section 3.2](#known-issues-windows))

#### C.2 Compiler Hardening Flags

**Problem:** GCC-specific security flags in CMakeLists.txt

**Affected Lines:** CMakeLists.txt:69-78

**Required Changes:**
```cmake
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,relro,-z,now")
elseif(MSVC)
    # MSVC equivalent flags
    add_compile_options(/GS /sdl)
endif()
```

#### C.3 Python Setup.py Hardening Flags

**Problem:** GCC-specific flags in `setup.py:84-87`

**Current Code:**
```python
extra_compile_args=['-DPYTHON_WRAPPER', '-std=c++17',
                    '-fstack-protector-strong', '-D_FORTIFY_SOURCE=2'],
extra_link_args=['-lexpat', '-Wl,-z,relro,-z,now']
```

**Required Changes:**
```python
import platform
import sys

extra_compile_args = ['-DPYTHON_WRAPPER', '-std=c++17']
extra_link_args = []

if sys.platform == 'win32':
    extra_compile_args.extend(['/GS', '/sdl'])
    extra_link_args.append('expat.lib')
elif sys.platform == 'darwin':
    extra_compile_args.extend(['-fstack-protector-strong'])
    extra_link_args.append('-lexpat')
else:  # Linux
    extra_compile_args.extend(['-fstack-protector-strong', '-D_FORTIFY_SOURCE=2'])
    extra_link_args.extend(['-lexpat', '-Wl,-z,relro,-z,now'])
```

#### C.4 Serial Device Support (Platform-Specific)

**Problem:** Serial port APIs differ across platforms

**Current:** Unix termios API only
**Needs:** Windows COM port support via `CreateFile()` API

**Status:** Not yet implemented for Windows

---

### Appendix D: Setup Scripts

#### D.1 Linux Development Environment Setup

**File:** `setup_dev_env.sh`

```bash
#!/bin/bash
set -e

echo "=== Installing system packages (requires sudo) ==="
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  git \
  libexpat1-dev \
  gcc-13 \
  g++-13 \
  lcov \
  valgrind \
  ninja-build

# Set GCC 13 as default
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 130
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 130

echo ""
echo "=== Setting up Python virtual environment ==="
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip setuptools wheel
pip install -r requirements-dev.txt

echo ""
echo "=== Building ASTERIX decoder ==="
python setup.py build
python setup.py install

echo ""
echo "=== Building C++ with tests and coverage ==="
mkdir -p build
cd build
cmake .. \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DBUILD_TESTING=ON \
  -DENABLE_COVERAGE=ON \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON
cmake --build . --parallel $(nproc)

echo ""
echo "=== Running tests ==="
ctest --output-on-failure
cd ..
source .venv/bin/activate
pytest asterix/test/ tests/python/ -v --cov=asterix

echo ""
echo "=== Generating C++ coverage report ==="
cd build
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/googletest/*' '*/test/*' \
  --output-file coverage_filtered.info
genhtml coverage_filtered.info --output-directory coverage_html

echo ""
echo "✅ Setup complete!"
echo ""
echo "Coverage reports:"
echo "  - Python: htmlcov/index.html"
echo "  - C++: build/coverage_html/index.html"
echo ""
echo "To activate venv in future sessions:"
echo "  source .venv/bin/activate"
```

**Make it executable:**
```bash
chmod +x setup_dev_env.sh
./setup_dev_env.sh
```

#### D.2 macOS Development Environment Setup

**File:** `setup_dev_env_macos.sh`

```bash
#!/bin/bash
set -e

echo "=== Installing Xcode Command Line Tools ==="
xcode-select --install || true

echo "=== Installing Homebrew dependencies ==="
brew update
brew install cmake expat ninja lcov

echo ""
echo "=== Setting up Python virtual environment ==="
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip setuptools wheel
pip install -r requirements-dev.txt

echo ""
echo "=== Building ASTERIX decoder ==="
python setup.py build
python setup.py install

echo ""
echo "=== Building C++ with tests ==="
mkdir -p build
cd build
cmake .. \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DBUILD_TESTING=ON \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON
cmake --build . --parallel $(sysctl -n hw.ncpu)

echo ""
echo "=== Running tests ==="
ctest --output-on-failure
cd ..
source .venv/bin/activate
pytest asterix/test/ tests/python/ -v --cov=asterix

echo ""
echo "✅ Setup complete!"
echo ""
echo "To activate venv in future sessions:"
echo "  source .venv/bin/activate"
```

**Make it executable:**
```bash
chmod +x setup_dev_env_macos.sh
./setup_dev_env_macos.sh
```

#### D.3 Windows Development Environment Setup

**File:** `setup_dev_env.ps1`

```powershell
# PowerShell script for Windows development environment setup

Write-Host "=== Installing vcpkg dependencies ===" -ForegroundColor Green

# Check if vcpkg is installed
$vcpkgPath = "C:\vcpkg"
if (-Not (Test-Path $vcpkgPath)) {
    Write-Host "Installing vcpkg..."
    cd C:\
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    .\bootstrap-vcpkg.bat
}

cd $vcpkgPath
.\vcpkg install expat:x64-windows
.\vcpkg integrate install

Write-Host ""
Write-Host "=== Setting up Python virtual environment ===" -ForegroundColor Green
cd $PSScriptRoot
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip setuptools wheel
pip install -r requirements-dev.txt

Write-Host ""
Write-Host "=== Building ASTERIX decoder ===" -ForegroundColor Green
$env:CMAKE_TOOLCHAIN_FILE = "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
python setup.py build
python setup.py install

Write-Host ""
Write-Host "=== Building C++ with CMake ===" -ForegroundColor Green
cmake -B build `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_CXX_STANDARD=20 `
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DBUILD_SHARED_LIBS=ON `
  -DBUILD_STATIC_LIBS=ON `
  -DBUILD_EXECUTABLE=ON

cmake --build build --config Release --parallel 4

Write-Host ""
Write-Host "=== Running Python tests ===" -ForegroundColor Green
.\.venv\Scripts\Activate.ps1
pytest asterix/test/ -v

Write-Host ""
Write-Host "✅ Setup complete!" -ForegroundColor Green
Write-Host ""
Write-Host "To activate venv in future sessions:"
Write-Host "  .\.venv\Scripts\Activate.ps1"
```

**Run in PowerShell:**
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
.\setup_dev_env.ps1
```

---

## Summary of Required Code Changes for Full Cross-Platform Support

### Priority 1: Critical for Windows Support

1. **Networking Abstraction** (`src/engine/UdpDevice.*`, `src/engine/TcpDevice.*`)
   - Add Winsock2 support with `#ifdef _WIN32`
   - Abstract socket API differences

2. **CMakeLists.txt Hardening Flags** (lines 69-78)
   - Add compiler detection
   - Use platform-appropriate flags

3. **setup.py Compilation Flags** (lines 84-87)
   - Add platform detection
   - Set appropriate flags per OS

### Priority 2: Enhanced Cross-Platform Support

4. **Serial Device** (`src/engine/SerialDevice.cpp`)
   - Add Windows COM port support

5. **Integration Tests for Windows**
   - Create `test.ps1` PowerShell script
   - Port Unix shell tests to PowerShell

### Priority 3: Optional Enhancements

6. **Installer Creation**
   - WiX configuration for MSI
   - DMG bundle for macOS
   - Desktop shortcuts/menu entries

7. **Code Signing**
   - Windows Authenticode
   - macOS notarization
   - Linux package signing

---

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

---

## References

**Related Documents:**
- `CLAUDE.md` - Project overview and coding guidelines
- `PERFORMANCE_OPTIMIZATIONS_COMPLETED.md` - Performance optimization history
- `CPP23_C23_UPGRADE_PLAN.md` - C++23 upgrade plan

**External Resources:**
- [CMake Documentation](https://cmake.org/documentation/)
- [vcpkg Documentation](https://vcpkg.io/)
- [Homebrew Documentation](https://docs.brew.sh/)
- [CPack Documentation](https://cmake.org/cmake/help/latest/module/CPack.html)
- [GCC C++23 Support](https://gcc.gnu.org/projects/cxx-status.html)
- [Clang C++23 Support](https://clang.llvm.org/cxx_status.html)
- [MSVC C++23 Support](https://learn.microsoft.com/en-us/cpp/build/standards-conformance)
- [ASTERIX Upstream Repository](https://github.com/CroatiaControlLtd/asterix)

---

**Document Version:** 1.0
**Last Updated:** 2025-11-04
**Maintained By:** ASTERIX Development Team
