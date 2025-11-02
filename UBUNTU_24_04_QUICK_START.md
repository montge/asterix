# Ubuntu 24.04 Release Build - Quick Start Checklist

## Pre-Flight Checklist

- [ ] Operating System: Ubuntu 24.04 (or WSL2 running Ubuntu 24.04)
- [ ] GCC 13+: `gcc --version` (shows 13.3.0 ✓)
- [ ] CMake 3.20+: `cmake --version` (shows 3.28.3 ✓)
- [ ] libexpat-dev: `dpkg -l | grep libexpat1-dev` (shows 2.6.1 ✓)

## Missing Prerequisites to Install

```bash
sudo apt-get install -y ninja-build lcov valgrind
```

- [ ] ninja-build: `ninja --version`
- [ ] lcov: `which lcov` (optional, for coverage)
- [ ] valgrind: `which valgrind` (optional, for memory checks)

## One-Command Build

Copy and paste this entire command block:

```bash
BUILD_DIR="/tmp/asterix-build-ubuntu24-release" && \
rm -rf "$BUILD_DIR" && \
mkdir -p "$BUILD_DIR" && \
cd "$BUILD_DIR" && \
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
  /path/to/asterix && \
cmake --build build --config Release --parallel $(nproc) && \
cmake --install build --config Release && \
echo "Build complete! Binary at: $BUILD_DIR/install/bin/asterix"
```

## Step-by-Step Build

### Step 1: Install Missing Tools (One-time)
```bash
sudo apt-get update
sudo apt-get install -y ninja-build lcov valgrind
```

**Verification:**
```bash
ninja --version    # Should output: 1.x.x or later
```

- [ ] Ninja installed successfully

### Step 2: Create Build Directory
```bash
BUILD_DIR="/tmp/asterix-build-ubuntu24-release"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
echo "Build directory: $BUILD_DIR"
```

**Verification:**
```bash
pwd  # Should show: /tmp/asterix-build-ubuntu24-release
```

- [ ] Build directory created

### Step 3: Configure CMake
```bash
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
```

**Expected Output:**
```
-- ASTERIX 2.8.10 Build Configuration:
--   Build shared library: ON
--   Build static library: ON
--   Build executable: ON
--   C++ Standard: C++23
--   Compiler: GNU 13.3.0
```

**Verification:**
```bash
ls -la build/CMakeCache.txt  # Should exist
```

- [ ] CMake configuration successful

### Step 4: Build
```bash
cmake --build build --config Release --parallel $(nproc)
```

**Progress:** Shows compilation progress with [X/Y] counter

**Expected Output:** Finishes without errors

**Verification:**
```bash
ls -la build/lib/libasterix.so*
ls -la build/lib/libasterix.a
ls -la build/bin/asterix
```

- [ ] Build completed successfully

### Step 5: Install
```bash
cmake --install build --config Release
```

**Expected Output:**
```
-- Install configuration: "Release"
-- Installing: /tmp/asterix-build-ubuntu24-release/install/bin/asterix
-- Installing: /tmp/asterix-build-ubuntu24-release/install/lib/libasterix.so.2.8.10
...
```

**Verification:**
```bash
ls -lh "$BUILD_DIR/install/bin/asterix"
ls -lh "$BUILD_DIR/install/lib/libasterix.so"
ls -lh "$BUILD_DIR/install/include/asterix/"
```

- [ ] Installation successful

### Step 6: Verify Installation
```bash
"$BUILD_DIR/install/bin/asterix" --version
"$BUILD_DIR/install/bin/asterix" --help
```

**Expected Output:**
- Version: Shows ASTERIX v2.8.10
- Help: Shows command-line options

- [ ] Verification successful

## Build Artifacts

After successful build, you have:

### Binaries
```
$BUILD_DIR/install/bin/asterix          # Command-line executable
```

### Libraries
```
$BUILD_DIR/install/lib/libasterix.so.2.8.10  # Shared library (version)
$BUILD_DIR/install/lib/libasterix.so.2       # Shared library symlink
$BUILD_DIR/install/lib/libasterix.so         # Shared library symlink
$BUILD_DIR/install/lib/libasterix.a          # Static library
```

### Headers
```
$BUILD_DIR/install/include/asterix/*.h       # Public header files
```

### Configuration
```
$BUILD_DIR/install/share/asterix/config/     # XML category definitions
```

### Documentation
```
$BUILD_DIR/install/share/doc/asterix/        # README, CLAUDE.md, etc.
```

## Create Distribution Packages

```bash
cd "$BUILD_DIR/build"

# Create DEB package (for Ubuntu/Debian)
cpack -G DEB

# Create TGZ archive (for all Linux)
cpack -G TGZ

# Create ZIP archive (portable)
cpack -G ZIP

# List created packages
ls -lh asterix-*.{deb,tar.gz,zip}
```

**Expected Output:**
```
asterix-2.8.10-Linux.deb       (~100-200 KB)
asterix-2.8.10-Linux.tar.gz    (~500-800 KB)
asterix-2.8.10-Linux.zip       (~500-800 KB)
```

- [ ] Packages created

## Verify Built Artifacts

### Check Shared Library
```bash
ldd "$BUILD_DIR/install/lib/libasterix.so"
```

**Expected Dependencies:**
- libexpat.so.1
- libc.so.6
- libm.so.6
- libstdc++.so.6

### Check Executable
```bash
ldd "$BUILD_DIR/install/bin/asterix"
```

**Expected Dependencies:**
- libasterix.so (the library you just built)
- libexpat.so.1
- libc.so.6
- libstdc++.so.6

### Check Exported Symbols
```bash
nm -D "$BUILD_DIR/install/lib/libasterix.so" | grep " T " | head -10
```

**Expected:** Shows C++ function names (mangled)

- [ ] Library verification complete

## Environment Summary

| Property | Value | Status |
|----------|-------|--------|
| OS | Ubuntu 24.04 (WSL2) | ✓ |
| GCC | 13.3.0 | ✓ |
| CMake | 3.28.3 | ✓ |
| libexpat-dev | 2.6.1 | ✓ |
| ninja-build | To install | * |
| Python venv | ~/.venv | ✓ (not needed for C++ build) |

**Legend:** ✓ = Already available, * = Needs action

## CMake Configuration Details

### Exact Parameters Used

```bash
-G Ninja                           # Build generator: Ninja (parallel build)
-DCMAKE_BUILD_TYPE=Release         # Optimization: -O3, no debug symbols
-DCMAKE_CXX_STANDARD=23            # C++23 features enabled
-DCMAKE_CXX_STANDARD_REQUIRED=ON   # Strict: fail if C++23 unavailable
-DCMAKE_C_COMPILER=gcc             # C compiler: GCC
-DCMAKE_CXX_COMPILER=g++           # C++ compiler: G++
-DBUILD_SHARED_LIBS=ON             # Build .so library
-DBUILD_STATIC_LIBS=ON             # Build .a library
-DBUILD_EXECUTABLE=ON              # Build asterix binary
-DBUILD_TESTING=OFF                # Skip test targets
-DCMAKE_INSTALL_PREFIX=...         # Installation directory
```

### What Each Parameter Does

| Parameter | Effect | Why? |
|-----------|--------|------|
| `-G Ninja` | Use Ninja build system instead of Make | Faster parallel builds |
| `Release` | Compile with optimizations (-O3) | Production performance |
| `C++23` | Enable C++23 language features | Modern C++ (ranges, etc.) |
| `SHARED_LIBS=ON` | Create .so library | Dynamic linking |
| `STATIC_LIBS=ON` | Create .a library | Static linking option |
| `EXECUTABLE=ON` | Create asterix CLI tool | Command-line interface |
| `TESTING=OFF` | Skip test targets | Faster build (tests in CI) |

## Quick Troubleshooting

### Build Won't Start: "ninja: command not found"
```bash
sudo apt-get install -y ninja-build
ninja --version
```

### CMake Configuration Fails: "EXPAT not found"
```bash
sudo apt-get install -y libexpat1-dev
cmake -B build ...  # Re-run configuration
```

### C++23 Errors: "no member named 'ranges' in namespace 'std'"
```bash
gcc --version  # Must be >= 13.0
# You have 13.3.0, so this shouldn't happen
# If it does: rm -rf build && cmake -B build ...
```

### Build is Slow
```bash
# Use all CPU cores (already done with $(nproc))
cmake --build build --parallel $(nproc)

# Check actual cores
nproc  # Shows number of available cores
```

### Installation Fails: "Permission denied"
```bash
# Option 1: Use sudo
sudo cmake --install build --config Release

# Option 2: Use home directory (no sudo needed)
cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/asterix-install ...
cmake --install build --config Release
```

## Build Summary

| Phase | Time | Status |
|-------|------|--------|
| Install deps | 30s | - |
| CMake config | 5s | - |
| Build | 30-60s | Check [X/Y] |
| Install | 5s | - |
| **Total** | **~2 min** | - |

**Total time:** ~2 minutes for clean build on modern system

## Files to Know

| File | Purpose |
|------|---------|
| `CMakeLists.txt` | Build configuration |
| `VERSION` | Project version (2.8.10) |
| `src/asterix/` | ASTERIX parsing code |
| `src/engine/` | Generic input/output engine |
| `src/main/asterix.cpp` | CLI executable entry point |
| `asterix/config/` | ASTERIX category definitions (XML) |
| `.github/workflows/cross-platform-builds.yml` | CI/CD configuration |
| `CLAUDE.md` | Project documentation |

## Verification Checklist

After build completes, verify with:

```bash
BUILD_DIR="/tmp/asterix-build-ubuntu24-release"

echo "1. Check binary exists:"
test -f "$BUILD_DIR/install/bin/asterix" && echo "✓" || echo "✗"

echo "2. Check shared library:"
test -f "$BUILD_DIR/install/lib/libasterix.so" && echo "✓" || echo "✗"

echo "3. Check static library:"
test -f "$BUILD_DIR/install/lib/libasterix.a" && echo "✓" || echo "✗"

echo "4. Run version command:"
"$BUILD_DIR/install/bin/asterix" --version && echo "✓" || echo "✗"

echo "5. Check library dependencies:"
ldd "$BUILD_DIR/install/bin/asterix" | grep expat && echo "✓" || echo "✗"

echo "All checks complete!"
```

## Success Indicators

- [ ] CMake configuration completes without errors
- [ ] Build completes with `cmake --build` showing [X/Y] progress
- [ ] Install step completes without permission errors
- [ ] Binary exists: `ls -l $BUILD_DIR/install/bin/asterix`
- [ ] Binary runs: `$BUILD_DIR/install/bin/asterix --version`
- [ ] Shared library exists: `ls -l $BUILD_DIR/install/lib/libasterix.so*`
- [ ] Static library exists: `ls -l $BUILD_DIR/install/lib/libasterix.a`
- [ ] Libraries are linked properly: `ldd` shows expected dependencies

## Next Steps

1. **Test the build:**
   ```bash
   $BUILD_DIR/install/bin/asterix --version
   ```

2. **Create packages:**
   ```bash
   cd $BUILD_DIR/build
   cpack -G DEB
   ```

3. **Run full integration tests (Debug only):**
   ```bash
   # Not applicable for Release, but if you want:
   cd $BUILD_DIR/build
   cmake --build . --config Debug
   cd ../install/test
   ./test.sh
   ```

4. **Deploy to system (optional):**
   ```bash
   # Install DEB package
   sudo dpkg -i $BUILD_DIR/build/asterix-2.8.10-Linux.deb

   # Or copy binary
   sudo cp $BUILD_DIR/install/bin/asterix /usr/local/bin/
   ```

## Environment Variables Summary

```bash
# Set before building for customization
BUILD_DIR="/tmp/asterix-build-ubuntu24-release"
CMAKE_BUILD_TYPE="Release"
CMAKE_CXX_STANDARD="23"
INSTALL_PREFIX="$BUILD_DIR/install"
BUILD_GENERATOR="Ninja"

# Automatic (no change needed)
CC=gcc
CXX=g++
CFLAGS="-fstack-protector-strong -D_FORTIFY_SOURCE=2"
CXXFLAGS="-fstack-protector-strong -D_FORTIFY_SOURCE=2"
```

## Questions?

See the full documentation in `UBUNTU_24_04_BUILD_INSTRUCTIONS.md` for:
- Detailed explanation of each build step
- Troubleshooting guide for common issues
- Environment differences (CI vs local)
- Integration testing procedures
- Package deployment instructions
