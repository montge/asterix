# Ubuntu 24.04 Release Build - Comprehensive Instructions

## Overview

This document provides step-by-step instructions to simulate and reproduce the Ubuntu 24.04 Release build from the GitHub Actions workflow (`.github/workflows/cross-platform-builds.yml`).

**Key Workflow Details:**
- **OS:** Ubuntu 24.04 (ubuntu-24.04)
- **Build Type:** Release
- **Compiler:** GCC 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04)
- **C++ Standard:** C++23 (full support in GCC 13+)
- **Build Generator:** Ninja
- **CMake Version Required:** 3.20+

---

## Part 1: Exact Workflow Commands

### Workflow Steps Extracted from CI Configuration

Below are the **exact commands** that GitHub Actions runs for the Ubuntu 24.04 Release build:

#### Step 1: Checkout Repository
```bash
# (Already done - you're in the repository)
cd /path/to/asterix
```

#### Step 2: Install GCC 13
```bash
sudo apt-get update
sudo apt-get install -y gcc-13 g++-13
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
gcc --version  # Verify: should show gcc 13.x.x
g++ --version  # Verify: should show g++ 13.x.x
```

**Note:** Your local environment already has GCC 13.3.0 installed, so this step may be skipped unless you need to explicitly set it as default.

#### Step 3: Install Dependencies
```bash
sudo apt-get install -y \
  libexpat1-dev \
  cmake \
  ninja-build \
  lcov \
  valgrind
```

**Status in your environment:**
- libexpat1-dev: ✓ INSTALLED (2.6.1-2ubuntu0.3)
- cmake: ✓ INSTALLED (3.28.3-1build7) - exceeds 3.20 requirement
- ninja-build: ✗ NOT INSTALLED (needs installation)
- lcov: ? (check with: `which lcov`)
- valgrind: ? (check with: `which valgrind`)

#### Step 4: Configure CMake (C++23)
```bash
mkdir -p /tmp/asterix-build-ubuntu24 && cd /tmp/asterix-build-ubuntu24

cmake -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++ \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_EXECUTABLE=ON \
  -DBUILD_TESTING=OFF \
  -DCMAKE_INSTALL_PREFIX=/tmp/asterix-build-ubuntu24/install \
  /path/to/asterix
```

**CMake Configuration Details:**
- **Generator:** Ninja (parallel build system, faster than Make)
- **Build Type:** Release (optimized, no debug symbols)
- **C++ Standard:** C++23 (with C++23 features enabled via CMakeLists.txt)
- **C Standard:** C23 (set automatically in CMakeLists.txt for GCC/Clang)
- **Compiler:** GCC 13 (gcc/g++)
- **Libraries:** Both shared (.so) and static (.a)
- **Executable:** Yes (asterix CLI tool)
- **Testing:** OFF (as per CI config)
- **Install Prefix:** Customizable (using `/tmp/asterix-build-ubuntu24/install` for local testing)

#### Step 5: Build
```bash
cd /tmp/asterix-build-ubuntu24

cmake --build build --config Release --parallel $(nproc)
```

**Build Details:**
- Uses Ninja generator for parallel compilation
- `$(nproc)` = automatic CPU core detection
- Expected output files in `build/lib/`:
  - `libasterix.so.2.8.10` (shared library - version 2.8.10)
  - `libasterix.so.2` (shared library symlink)
  - `libasterix.so` (shared library symlink)
  - `libasterix.a` (static library)

#### Step 6: Install
```bash
cd /tmp/asterix-build-ubuntu24

cmake --install build --config Release
```

**Install Details:**
- Creates `/tmp/asterix-build-ubuntu24/install/` directory structure
- **Installed Artifacts:**
  - Binary: `install/bin/asterix`
  - Shared library: `install/lib/libasterix.so*`
  - Static library: `install/lib/libasterix.a`
  - Headers: `install/include/asterix/*.h`
  - Configuration: `install/share/asterix/config/` (ASTERIX XML definitions)
  - Documentation: `install/share/doc/asterix/`

#### Step 7: Test Executable
```bash
/tmp/asterix-build-ubuntu24/install/bin/asterix --help
/tmp/asterix-build-ubuntu24/install/bin/asterix --version
```

**Expected Output:**
- Help message listing all command-line options
- Version output: Shows ASTERIX version (currently 2.8.10)

#### Step 8: Run Integration Tests (Debug Build Only)
**Note:** In CI, integration tests only run for Debug builds. For Release build verification, basic tests are:

```bash
# Skip this for Release (only runs in Debug per workflow)
# For completeness, these would be:
# cd /tmp/asterix-build-ubuntu24/install/test
# ./test.sh
```

#### Step 9: Verify Library Linkage
```bash
echo "=== Checking executable dependencies ==="
ldd /tmp/asterix-build-ubuntu24/install/bin/asterix

echo -e "\n=== Checking library exports ==="
nm -D /tmp/asterix-build-ubuntu24/install/lib/libasterix.so | grep " T " | head -20
```

**Expected Output:**
- `ldd` shows dependencies on libexpat, libc, etc.
- `nm` shows exported symbols (public functions)

#### Step 10: Create DEB Package (Ubuntu 24.04 Release Only)
```bash
cd /tmp/asterix-build-ubuntu24/build

# Create DEB package
cpack -G DEB

# Create TGZ and ZIP packages
cpack -G TGZ
cpack -G ZIP

# List created packages
ls -lh *.deb *.tar.gz *.zip
```

**Expected Output:**
- `asterix-2.8.10-Linux.deb` (Debian package for installation)
- `asterix-2.8.10-Linux.tar.gz` (Source tarball)
- `asterix-2.8.10-Linux.zip` (ZIP archive)

---

## Part 2: Prerequisites Check & Installation

### Prerequisites Already Available

```bash
# Verify your current environment
uname -a
cat /etc/os-release | grep VERSION
gcc --version
g++ --version
cmake --version
python3 --version
```

**Your System:**
- OS: Ubuntu 24.04 (via WSL2)
- GCC: 13.3.0 ✓
- CMake: 3.28.3 ✓
- Python: 3.12.3 ✓
- Python venv: ~/.venv ✓

### Missing Prerequisites to Install

```bash
# Install ninja-build (REQUIRED)
sudo apt-get install -y ninja-build

# Verify installation
ninja --version  # Should show version 1.x.x

# Optional: Install code coverage tools
sudo apt-get install -y lcov

# Optional: Install memory testing tool
sudo apt-get install -y valgrind
```

### Environment Variables Setup

For your local build, you may set these environment variables:

```bash
# Recommended: Use all CPU cores for faster builds
export MAKEFLAGS="-j$(nproc)"

# Optional: Enable verbose CMake output for debugging
export CMAKE_VERBOSE_MAKEFILE=ON

# Python venv activation (if running Python tests)
source ~/.venv/bin/activate
```

---

## Part 3: Complete Local Build Sequence

### Quick Start (Copy-Paste Ready)

```bash
#!/bin/bash
set -e  # Exit on first error

# 1. Navigate to project root
cd /path/to/asterix

# 2. Install missing dependencies
echo "Installing missing dependencies..."
sudo apt-get update
sudo apt-get install -y ninja-build lcov valgrind

# 3. Create isolated build directory
BUILD_DIR="/tmp/asterix-build-ubuntu24"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 4. Configure CMake
echo "Configuring CMake..."
cmake -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++ \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_EXECUTABLE=ON \
  -DBUILD_TESTING=OFF \
  -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/install" \
  /path/to/asterix

# 5. Build
echo "Building (parallel on $(nproc) cores)..."
cmake --build build --config Release --parallel $(nproc)

# 6. Install
echo "Installing..."
cmake --install build --config Release

# 7. Verify
echo "Verifying installation..."
"$BUILD_DIR/install/bin/asterix" --version
"$BUILD_DIR/install/bin/asterix" --help | head -20

echo "Success! Build available at: $BUILD_DIR/install"
```

### Step-by-Step Manual Build

#### 1. Prepare Environment
```bash
# Navigate to your development directory
cd /path/to/asterix

# Verify you're in the correct repo
git status  # Should show "On branch master"

# Check git for any uncommitted changes
git log -1 --oneline  # Shows latest commit
```

#### 2. Install Ninja Build Tool (One-Time)
```bash
# Check if ninja is already installed
which ninja || (echo "Installing ninja..." && sudo apt-get install -y ninja-build)

# Verify
ninja --version
```

#### 3. Create Isolated Build Environment
```bash
# Create a clean build directory (doesn't interfere with repo)
BUILD_DIR="/tmp/asterix-build-ubuntu24-release"
rm -rf "$BUILD_DIR"  # Clean any previous builds
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Build directory: $BUILD_DIR"
pwd  # Verify current location
```

#### 4. Run CMake Configuration
```bash
# Run from the build directory (out-of-tree build)
cmake -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++ \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_EXECUTABLE=ON \
  -DBUILD_TESTING=OFF \
  -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/install" \
  /path/to/asterix

# Expected output shows:
# - ASTERIX 2.8.10 Build Configuration
# - Build shared library: ON
# - Build static library: ON
# - Build executable: ON
# - C++ Standard: C++23
# - C Standard: C23
# - Compiler: GNU x.x.x
```

**Troubleshooting CMake Configuration:**
```bash
# If CMake fails, check these:
cmake --version  # Must be >= 3.20
gcc --version    # Must be >= 13.0
which expat-config || pkg-config --modversion expat  # Check expat

# Verify source tree
ls -la /path/to/asterix/CMakeLists.txt
ls -la /path/to/asterix/src/
```

#### 5. Build the Project
```bash
# Navigate to build directory
cd "$BUILD_DIR"

# Perform parallel build using all CPU cores
cmake --build build --config Release --parallel $(nproc)

# Progress indicators:
# - Lines showing [X/Y] compilation progress
# - Linking messages
# - Final summary with build time
```

**Troubleshooting Build Failures:**
```bash
# If build fails, check compiler
gcc --version  # Should be 13.x
g++ --version  # Should be 13.x

# Check verbose output
cmake --build build --config Release --verbose 2>&1 | tail -50

# Check for C++23 feature issues
cat build/CMakeFiles/asterix_shared.dir/link.txt | head -1
```

#### 6. Install Build Artifacts
```bash
cd "$BUILD_DIR"

# Install to prefix directory
cmake --install build --config Release

# Verify installed files
echo "=== Installed Binaries ==="
ls -lh install/bin/

echo "=== Installed Libraries ==="
ls -lh install/lib/

echo "=== Installed Headers ==="
ls -lh install/include/asterix/

echo "=== Installed Configuration ==="
ls -lh install/share/asterix/config/
```

---

## Part 4: Verification & Testing

### 4.1 Basic Verification

```bash
BUILD_DIR="/tmp/asterix-build-ubuntu24-release"

# Test 1: Version info
echo "=== Test 1: Version ==="
$BUILD_DIR/install/bin/asterix --version

# Test 2: Help information
echo -e "\n=== Test 2: Help ==="
$BUILD_DIR/install/bin/asterix --help | head -30

# Test 3: List available filters
echo -e "\n=== Test 3: List Filters ==="
$BUILD_DIR/install/bin/asterix \
  -d $BUILD_DIR/install/share/asterix/config/asterix.ini \
  -L 2>&1 | head -20
```

**Expected Output:**
- Version: `ASTERIX v2.8.10` (or similar)
- Help: Command-line options and usage information
- Filters: List of available ASTERIX filters

### 4.2 Library Verification

```bash
BUILD_DIR="/tmp/asterix-build-ubuntu24-release"

echo "=== Checking Shared Library Dependencies ==="
ldd $BUILD_DIR/install/lib/libasterix.so.2.8.10

echo -e "\n=== Checking Executable Dependencies ==="
ldd $BUILD_DIR/install/bin/asterix

echo -e "\n=== Exported Symbols (First 20) ==="
nm -D $BUILD_DIR/install/lib/libasterix.so | grep " T " | head -20

echo -e "\n=== Library File Information ==="
file $BUILD_DIR/install/lib/libasterix.so*
file $BUILD_DIR/install/lib/libasterix.a
```

**Expected Output:**
- Dependencies include: libexpat, libc, libm, libstdc++
- Exported symbols: Public C++ functions (mangled names)
- File types: ELF 64-bit shared objects and static archives

### 4.3 Integration Tests (Optional - Debug Build Required)

**Note:** The workflow only runs full integration tests for Debug builds. For a Release build, you can verify the executable works with sample data:

```bash
BUILD_DIR="/tmp/asterix-build-ubuntu24-release"

# If test samples are available in the install directory
if [ -f "$BUILD_DIR/install/share/asterix/samples/cat_062_065.pcap" ]; then
  echo "=== Processing Sample PCAP Data ==="
  $BUILD_DIR/install/bin/asterix \
    -d $BUILD_DIR/install/share/asterix/config/asterix.ini \
    -P \
    -j \
    -f "$BUILD_DIR/install/share/asterix/samples/cat_062_065.pcap" | head -50
fi
```

### 4.4 Package Creation

```bash
BUILD_DIR="/tmp/asterix-build-ubuntu24-release"

cd "$BUILD_DIR/build"

# Create DEB package (Debian/Ubuntu native format)
echo "=== Creating DEB Package ==="
cpack -G DEB
ls -lh asterix*.deb

# Create TGZ archive
echo -e "\n=== Creating TGZ Package ==="
cpack -G TGZ
ls -lh asterix*.tar.gz

# Create ZIP archive
echo -e "\n=== Creating ZIP Package ==="
cpack -G ZIP
ls -lh asterix*.zip

# Verify DEB package contents (requires dpkg)
echo -e "\n=== DEB Package Contents ==="
dpkg -c asterix*.deb | head -20
```

**Expected Output:**
- `asterix-2.8.10-Linux.deb` (Debian installer)
- `asterix-2.8.10-Linux.tar.gz` (Tarball)
- `asterix-2.8.10-Linux.zip` (ZIP archive)

---

## Part 5: Differences Between CI and Local Environment

### Comparison Table

| Aspect | CI Environment | Local Environment | Impact |
|--------|---|---|---|
| **OS** | Ubuntu 24.04 (GitHub runner) | Ubuntu 24.04 (WSL2) | Identical, no issues |
| **GCC** | 13.x (auto-installed) | 13.3.0 (pre-installed) | Identical, C++23 fully supported |
| **CMake** | 3.x (auto-installed) | 3.28.3 | Identical (3.28 > 3.20 required) |
| **Ninja** | Auto-installed in CI | NOT installed locally | Must install: `sudo apt-get install ninja-build` |
| **libexpat1-dev** | Auto-installed in CI | Installed (2.6.1) | Identical |
| **lcov** | Auto-installed in CI | Unclear (optional) | Not required for Release build |
| **valgrind** | Auto-installed in CI | Unclear (optional) | Only needed for memory tests |
| **Build Type** | Release | Can be Release or Debug | Matching for exact reproduction |
| **Install Prefix** | GitHub workspace path | Your custom path | Different paths, same structure |
| **Python venv** | Not used in C++ CI | Available at ~/.venv | Irrelevant for C++ build |

### Key Differences Explained

#### 1. Ninja Build System (Critical)
- **CI:** Uses Ninja (`-G Ninja`) for faster parallel builds on GitHub runners
- **Local:** Must install Ninja manually
- **Impact:** Without Ninja, CMake will default to Unix Makefiles (slower but functional)
- **Solution:** Install with `sudo apt-get install -y ninja-build`

#### 2. Build Directory Location
- **CI:** Builds in `${{ github.workspace }}/build`
- **Local:** Recommended: `/tmp/asterix-build-ubuntu24-release/build`
- **Impact:** Minimal - outputs are the same structure
- **Note:** Out-of-tree builds are cleaner and don't clutter the repo

#### 3. Installation Prefix
- **CI:** `${{ github.workspace }}/install`
- **Local:** Custom prefix (e.g., `/tmp/asterix-build-ubuntu24-release/install`)
- **Impact:** None - just affects where files are installed
- **Flexibility:** Can use system prefix (`/usr/local`) with `sudo cmake --install`

#### 4. Testing Framework
- **CI:** Doesn't run integration tests for Release builds (only Debug)
- **Local:** Can optionally run integration tests if needed
- **Impact:** CI doesn't verify tests on Release, but Release builds are tested elsewhere

#### 5. Package Creation
- **CI:** Creates DEB, TGZ, ZIP packages (DEB only for Ubuntu 24.04)
- **Local:** Same capabilities, but requires cpack
- **Impact:** None - cpack is included with CMake

#### 6. Security & Hardening Flags
Both CI and local use identical CMake security flags:
```cmake
-fstack-protector-strong
-D_FORTIFY_SOURCE=2 (Release only)
-Wl,-z,relro,-z,now (Linux linking)
```

---

## Part 6: Environment Variables Reference

### Build Environment Variables

```bash
# C++ Compiler selection (already set by CMake)
export CC=gcc
export CXX=g++

# Optimization flags (automatically set via Release build type)
# Release: -O3 -DNDEBUG (optimized, no debugging)
# Debug: -g -O0 (debugging info, no optimization)

# Parallel build jobs
export MAKEFLAGS="-j$(nproc)"  # For Make generator
export NINJA_BUILD_PARALLELISM=$(nproc)  # For Ninja (auto-detected)

# CMake verbosity
export CMAKE_VERBOSE_MAKEFILE=ON  # Show full command lines
export CMAKE_MESSAGE_LOG_LEVEL=VERBOSE  # Verbose logging

# C++ standard (set by CMake)
# For GCC 13: CMAKE_CXX_STANDARD=23, CMAKE_C_STANDARD=23
```

### Optional Environment Variables for Advanced Usage

```bash
# Force specific compiler version
export CC=/usr/bin/gcc-13
export CXX=/usr/bin/g++-13

# Custom install location
export CMAKE_INSTALL_PREFIX=/opt/asterix

# Enable code coverage (adds instrumentation)
export ENABLE_COVERAGE=ON  # Use in CMake: -DENABLE_COVERAGE=ON

# Memory leak detection
export VALGRIND_FLAGS="--leak-check=full --show-leak-kinds=all"

# Python venv (for Python module builds)
source ~/.venv/bin/activate
```

---

## Part 7: Troubleshooting Guide

### Issue: Ninja Not Found

**Error:**
```
cmake: error: "-G Ninja" generator not found
```

**Solution:**
```bash
sudo apt-get install -y ninja-build
ninja --version  # Verify
```

### Issue: C++23 Features Not Available

**Error:**
```
error: 'std::ranges::...' is not a member of 'std'
```

**Solution:**
```bash
# Verify GCC version
gcc --version  # Must be 13.0+

# Reconfigure with explicit C++23 flag
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  ...
```

### Issue: libexpat Not Found

**Error:**
```
CMake Error at CMakeLists.txt:164 (find_package):
  Could not find a configuration file for package "EXPAT"
```

**Solution:**
```bash
# Verify libexpat is installed
dpkg -l | grep libexpat
# Should show: libexpat1-dev

# If missing, install:
sudo apt-get install -y libexpat1-dev

# Reconfigure CMake
rm -rf build && cmake -B build ...
```

### Issue: Build Fails with "Undefined Reference"

**Error:**
```
undefined reference to `expat_...'
```

**Solution:**
```bash
# Rebuild completely
rm -rf build
cmake -B build -G Ninja \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++ \
  ...

cmake --build build --verbose  # Show full linker commands
```

### Issue: Slow Build Performance

**Solution:**
```bash
# Increase parallel jobs
cmake --build build --config Release --parallel $(nproc)

# Or set environment variable
export MAKEFLAGS="-j$(nproc)"
cmake --build build --config Release

# Check number of cores
nproc  # Shows available CPU cores
```

### Issue: Installation Permission Denied

**Error:**
```
error: Permission denied while trying to install
```

**Solution:**
```bash
# Option 1: Use sudo
sudo cmake --install build --config Release

# Option 2: Use custom prefix in home directory
cmake -B build \
  -DCMAKE_INSTALL_PREFIX="$HOME/asterix-install" \
  ...
cmake --install build --config Release

# Option 3: Use /usr/local (requires sudo once)
cmake -B build \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  ...
sudo cmake --install build --config Release
```

---

## Part 8: Quick Reference Commands

### All-in-One Build Script

Save as `build-ubuntu24.sh`:

```bash
#!/bin/bash
set -e

# Configuration
BUILD_DIR="${1:-.}/asterix-build"
SOURCE_DIR="/path/to/asterix"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'  # No Color

echo -e "${YELLOW}=== Ubuntu 24.04 ASTERIX Release Build ===${NC}"
echo "Source: $SOURCE_DIR"
echo "Build Dir: $BUILD_DIR"

# 1. Install dependencies
echo -e "\n${YELLOW}[1/6] Installing dependencies...${NC}"
sudo apt-get update -qq
sudo apt-get install -y -qq ninja-build lcov valgrind

# 2. Prepare build directory
echo -e "\n${YELLOW}[2/6] Preparing build directory...${NC}"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 3. Configure CMake
echo -e "\n${YELLOW}[3/6] Configuring CMake...${NC}"
cmake -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_CXX_STANDARD_REQUIRED=ON \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++ \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_EXECUTABLE=ON \
  -DBUILD_TESTING=OFF \
  -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/install" \
  "$SOURCE_DIR"

# 4. Build
echo -e "\n${YELLOW}[4/6] Building (parallel on $(nproc) cores)...${NC}"
cmake --build build --config Release --parallel $(nproc)

# 5. Install
echo -e "\n${YELLOW}[5/6] Installing...${NC}"
cmake --install build --config Release

# 6. Verify
echo -e "\n${YELLOW}[6/6] Verifying installation...${NC}"
"$BUILD_DIR/install/bin/asterix" --version
"$BUILD_DIR/install/bin/asterix" --help | head -15

echo -e "\n${GREEN}=== Build Complete ===${NC}"
echo "Install directory: $BUILD_DIR/install"
echo "Binary: $BUILD_DIR/install/bin/asterix"
echo "Libraries: $BUILD_DIR/install/lib/"
echo ""
echo "To create packages:"
echo "  cd $BUILD_DIR/build && cpack -G DEB && cpack -G TGZ && cpack -G ZIP"
```

### Usage

```bash
chmod +x build-ubuntu24.sh
./build-ubuntu24.sh                    # Use default ./asterix-build
./build-ubuntu24.sh /tmp/asterix-build # Use custom location
```

### Essential Commands Summary

```bash
# 1. Install dependencies (one-time)
sudo apt-get install -y ninja-build lcov valgrind

# 2. Configure
cmake -B /tmp/asterix-build/build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=23 \
  -DCMAKE_INSTALL_PREFIX=/tmp/asterix-build/install \
  /path/to/asterix

# 3. Build
cmake --build /tmp/asterix-build/build --parallel $(nproc)

# 4. Install
cmake --install /tmp/asterix-build/build

# 5. Verify
/tmp/asterix-build/install/bin/asterix --version

# 6. Create packages
cd /tmp/asterix-build/build
cpack -G DEB  # Creates .deb
cpack -G TGZ  # Creates .tar.gz
cpack -G ZIP  # Creates .zip
```

---

## Summary

### What Gets Built

1. **libasterix.so.2.8.10** - Shared library (main library)
2. **libasterix.so.2** - Shared library symlink
3. **libasterix.so** - Shared library symlink
4. **libasterix.a** - Static library
5. **asterix** - Command-line executable
6. **asterix.deb** - Debian package
7. **asterix.tar.gz** - Gzipped archive
8. **asterix.zip** - ZIP archive

### Key Files

- **CMakeLists.txt** - Build configuration
- **VERSION** - Project version (2.8.10)
- **src/** - C++ source code
- **asterix/config/** - ASTERIX category definitions (XML)

### Expected Build Time

- Clean build: 30-60 seconds (depending on CPU)
- Incremental: 5-10 seconds
- With `$(nproc)` parallelization on multi-core system: Very fast

### Next Steps After Build

1. Verify with `asterix --version`
2. Create packages: `cpack -G DEB`
3. Run tests: `cd install/test && ./test.sh` (if test data available)
4. Deploy: `sudo dpkg -i asterix-2.8.10-Linux.deb`

---

## Support

For issues or questions:
1. Check **Troubleshooting Guide** (Part 7) above
2. Review GitHub Actions workflow: `.github/workflows/cross-platform-builds.yml`
3. Check CMakeLists.txt for build configuration
4. Review CLAUDE.md for project context
