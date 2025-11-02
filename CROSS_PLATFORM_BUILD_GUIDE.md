# Cross-Platform Build Guide

This guide provides comprehensive instructions for building the ASTERIX decoder on Windows, macOS, and Linux platforms.

## Table of Contents

- [Platform Requirements](#platform-requirements)
- [Windows Build](#windows-build)
- [macOS Build](#macos-build)
- [Linux Build](#linux-build)
- [Python Module Build](#python-module-build)
- [Known Platform Issues](#known-platform-issues)
- [Testing](#testing)
- [Packaging](#packaging)

## Platform Requirements

### All Platforms

- **CMake**: 3.12 or later (3.20+ recommended)
- **libexpat**: XML parsing library
- **C++23 compiler** (C++17 minimum):
  - Windows: MSVC 2022 (v143) or later
  - macOS: AppleClang 15+ (Xcode 15.0+)
  - Linux: GCC 13+ or Clang 16+

### Windows-Specific

- **Visual Studio 2022** (Community, Professional, or Enterprise)
  - Workload: "Desktop development with C++"
  - Components: MSVC v143, Windows SDK, CMake tools
- **vcpkg** (recommended for dependency management)
- **Git for Windows** (optional, for version control)

### macOS-Specific

- **Xcode 15.0 or later** (15.4 recommended for ARM64/M1)
- **Homebrew** (recommended for dependency management)
- **Command Line Tools**: `xcode-select --install`
- **macOS 11.0+** (deployment target)

### Linux-Specific

- **Build essentials**: gcc, g++, make
- **Development packages**: libexpat1-dev (Debian/Ubuntu) or expat-devel (RHEL/Fedora)
- **Optional**: ninja-build, valgrind, lcov (for testing/coverage)

---

## Windows Build

### Method 1: Using CMake with Visual Studio 2022

#### Step 1: Install Dependencies

Using **vcpkg** (recommended):

```powershell
# Clone vcpkg if not already installed
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install expat
.\vcpkg install expat:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

#### Step 2: Configure CMake

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

#### Step 3: Build

```powershell
# Build Release configuration
cmake --build build --config Release --parallel 4

# Or open in Visual Studio
start build\asterix.sln
```

#### Step 4: Install

```powershell
cmake --install build --config Release
```

#### Step 5: Test

```powershell
.\install\bin\asterix.exe --version
.\install\bin\asterix.exe --help
```

### Method 2: Using Visual Studio IDE

1. Open Visual Studio 2022
2. Select "Open a local folder" → navigate to ASTERIX repository
3. CMake integration will auto-configure
4. Select build configuration (Release/Debug)
5. Build → Build All
6. Output in `out/build/x64-Release/`

### Creating Windows Installer (MSI)

#### Using CPack (included with CMake)

```powershell
cd build
cpack -G ZIP -C Release
```

#### Using WiX Toolset (Advanced)

1. Install WiX Toolset v3.11+
2. Create WiX configuration file (`.wxs`)
3. Build MSI:

```powershell
candle asterix.wxs
light -out asterix.msi asterix.wixobj
```

### Windows-Specific Notes

**⚠️ Known Issues:**

1. **Networking Code**: Unix-style socket headers (`sys/socket.h`, `arpa/inet.h`) are not available on Windows
   - **Status**: TCP/UDP device classes will not compile on Windows
   - **Workaround**: Use Windows Socket API (Winsock2) - requires code refactoring
   - **Files affected**: `src/engine/UdpDevice.cpp`, `src/engine/TcpDevice.cpp`

2. **Hardening Flags**: GCC-specific flags need MSVC equivalents
   - `-fstack-protector-strong` → `/GS` (enabled by default)
   - `-D_FORTIFY_SOURCE=2` → Not applicable on MSVC
   - `-Wl,-z,relro,-z,now` → Not applicable on Windows

3. **File Paths**: Use `\` or `/` consistently in paths

**Recommended CMake Updates for Windows:**

```cmake
# In CMakeLists.txt, add platform detection:
if(MSVC)
    # MSVC-specific flags
    add_compile_options(/W4 /WX- /GS /sdl)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX)
    # GCC/Clang flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
    # ... existing Unix flags
endif()
```

---

## macOS Build

### Method 1: Using CMake (Recommended)

#### Step 1: Install Xcode Command Line Tools

```bash
xcode-select --install
```

#### Step 2: Install Dependencies via Homebrew

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew update
brew install cmake expat
```

#### Step 3: Configure CMake

```bash
cd /path/to/asterix

cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_EXECUTABLE=ON \
  -DCMAKE_INSTALL_PREFIX=$PWD/install
```

For **Apple Silicon (M1/M2/M3)**:

```bash
cmake -B build \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  # ... other flags
```

For **Intel Macs**:

```bash
cmake -B build \
  -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  # ... other flags
```

For **Universal Binary** (both architectures):

```bash
cmake -B build \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  # ... other flags
```

#### Step 4: Build

```bash
cmake --build build --config Release --parallel $(sysctl -n hw.ncpu)
```

#### Step 5: Install

```bash
cmake --install build --config Release
```

#### Step 6: Test

```bash
./install/bin/asterix --version
./install/bin/asterix --help
```

### Method 2: Using Make (Traditional)

```bash
cd src
make clean
make
make install
```

### Creating macOS Installer (DMG/PKG)

#### Using CPack

```bash
cd build
cpack -G TGZ      # Tarball
cpack -G ZIP      # ZIP archive
```

#### Using create-dmg (Recommended for distribution)

1. Install create-dmg:

```bash
brew install create-dmg
```

2. Create DMG:

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

#### Code Signing (Production)

```bash
# Sign the executable
codesign --force --options runtime --sign "Developer ID Application: Your Name" install/bin/asterix

# Sign the DMG
codesign --force --sign "Developer ID Application: Your Name" asterix-2.8.9.dmg

# Notarize with Apple (requires Apple Developer account)
xcrun notarytool submit asterix-2.8.9.dmg --keychain-profile "AC_PASSWORD"
xcrun stapler staple asterix-2.8.9.dmg
```

### macOS-Specific Notes

**✅ Platform Compatibility:**

- The code already handles macOS differences (e.g., `MSG_NOSIGNAL` → `SO_NOSIGPIPE`)
- Socket headers use Unix-style APIs (compatible)

**⚠️ Known Issues:**

1. **Deployment Target**: Set minimum macOS version to 11.0+ for C++23 features
2. **Hardening Flags**: Some Linux-specific flags don't apply:
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

---

## Linux Build

### Method 1: Using CMake with Ninja (Fastest)

#### Step 1: Install Dependencies

**Debian/Ubuntu:**

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  ninja-build \
  libexpat1-dev \
  gcc-13 \
  g++-13
```

**RHEL/Fedora/Rocky:**

```bash
sudo dnf install -y \
  cmake \
  ninja-build \
  expat-devel \
  gcc \
  gcc-c++
```

#### Step 2: Configure CMake

```bash
cd /path/to/asterix

cmake -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_C_COMPILER=gcc-13 \
  -DCMAKE_CXX_COMPILER=g++-13 \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_EXECUTABLE=ON \
  -DCMAKE_INSTALL_PREFIX=$PWD/install
```

#### Step 3: Build

```bash
cmake --build build --config Release --parallel $(nproc)
```

#### Step 4: Install

```bash
cmake --install build --config Release
```

### Method 2: Using Make (Traditional)

```bash
cd src
make clean
make
make install

# Debug build
make debug
make debug install
```

### Method 3: Building Packages

#### DEB Package (Debian/Ubuntu)

```bash
cd build
cpack -G DEB
sudo dpkg -i asterix-2.8.9-Linux.deb
```

#### RPM Package (RHEL/Fedora)

```bash
cd build
cpack -G RPM
sudo rpm -i asterix-2.8.9-Linux.rpm
```

### Running Tests

```bash
# Integration tests
cd install/test
./test.sh

# Memory leak detection
cd install/test
./valgrind_test.sh

# Or manually:
valgrind --leak-check=full ./install/bin/asterix --help
```

---

## Python Module Build

### Prerequisites

- Python 3.8+ (3.9+ recommended)
- pip, setuptools, wheel
- libexpat development headers
- C++17 compiler

### Build and Install

**All Platforms:**

```bash
# Install build dependencies
pip install --upgrade pip setuptools wheel pytest pytest-cov

# Build module
python setup.py build

# Install locally
python setup.py install

# Or install in development mode
pip install -e .
```

**Windows-specific:**

```powershell
# Set vcpkg toolchain before building
$env:CMAKE_TOOLCHAIN_FILE = "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
python setup.py build
```

### Running Python Tests

```bash
# Run tests
python -m pytest asterix/test/ -v

# With coverage
python -m pytest asterix/test/ -v --cov=asterix --cov-report=html

# Or use unittest
python -m unittest discover -s asterix/test -p 'test_*.py'
```

### Building Python Wheels

```bash
# Install build tools
pip install build wheel

# Build wheel
python -m build --wheel

# Install wheel
pip install dist/asterix_decoder-*.whl
```

---

## Known Platform Issues

### 1. Networking Code (Windows Incompatibility)

**Problem**: Unix socket headers not available on Windows

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

**Workaround**: Disable network device compilation on Windows:
```cmake
if(NOT WIN32)
  list(APPEND ASTERIX_LIB_SOURCES
    src/engine/tcpdevice.cxx
    src/engine/udpdevice.cxx
  )
endif()
```

### 2. Compiler Hardening Flags

**Problem**: GCC-specific security flags in CMakeLists.txt

**Affected Lines (CMakeLists.txt:69-78)**:
```cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now")
```

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

### 3. Python Setup.py Hardening Flags

**Problem**: GCC-specific flags in `setup.py:84-87`

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

### 4. Serial Device Support (Platform-Specific)

**Problem**: Serial port APIs differ across platforms

**Current**: Unix termios API only
**Needs**: Windows COM port support via `CreateFile()` API

---

## Testing

### Unit Tests (C++)

**Planned** - C++ unit tests using Google Test:

```bash
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

### Integration Tests

**Linux/macOS:**
```bash
cd install/test
./test.sh
```

**Windows:** Create equivalent `test.ps1`:
```powershell
# See CROSS_PLATFORM_BUILD_GUIDE.md for PowerShell test script
```

### Memory Tests

**Linux/macOS only** (valgrind not available on Windows):
```bash
cd install/test
./valgrind_test.sh
```

**Windows Alternative**: Use Application Verifier or Dr. Memory

---

## Packaging

### Linux Packages

```bash
cd build

# DEB (Debian/Ubuntu)
cpack -G DEB

# RPM (RHEL/Fedora)
cpack -G RPM

# Generic tarballs
cpack -G TGZ
cpack -G ZIP
```

### macOS Packages

```bash
cd build

# DMG (requires create-dmg)
create-dmg --volname "ASTERIX" asterix.dmg install/

# PKG installer
productbuild --component install/bin/asterix /usr/local/bin asterix.pkg

# Generic archives
cpack -G TGZ
cpack -G ZIP
```

### Windows Packages

```bash
cd build

# ZIP archive
cpack -G ZIP -C Release

# NSIS installer (requires NSIS installed)
cpack -G NSIS -C Release

# WiX MSI (requires WiX Toolset)
cpack -G WIX -C Release
```

---

## Summary of Required Code Changes

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

## Continuous Integration

See `.github/workflows/cross-platform-builds.yml` for automated builds across:
- Windows 2019/2022 (MSVC)
- macOS 13 (Intel) / 14 (ARM)
- Ubuntu 22.04 / 24.04

---

## Support and Troubleshooting

### Windows Build Fails with "Cannot find expat"

**Solution**: Ensure vcpkg integration is complete:
```powershell
cd C:\vcpkg
.\vcpkg integrate install
```

### macOS Build Fails with "unknown type name 'fd_set'"

**Solution**: Include `<sys/select.h>` (already present in code)

### Linux: "undefined reference to `expat_*`"

**Solution**: Install development package:
```bash
sudo apt-get install libexpat1-dev
```

### Python Module Build Fails

**Solution**: Ensure C++ compiler and expat headers are available
```bash
# Linux
sudo apt-get install build-essential libexpat1-dev

# macOS
brew install expat

# Windows
vcpkg install expat:x64-windows
```

---

## References

- [CMake Documentation](https://cmake.org/documentation/)
- [vcpkg Documentation](https://vcpkg.io/)
- [Homebrew Documentation](https://docs.brew.sh/)
- [CPack Documentation](https://cmake.org/cmake/help/latest/module/CPack.html)
- [ASTERIX Repository](https://github.com/montge/asterix)
