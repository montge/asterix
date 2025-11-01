# Cross-Platform Implementation Checklist

Quick-start guide for implementing cross-platform build support. Follow these steps in order.

## Phase 1: Enable CI/CD (30 minutes)

### Step 1.1: Deploy Workflow File
```bash
# Verify workflow file exists
ls -la .github/workflows/cross-platform-builds.yml

# Commit and push to trigger CI
git add .github/workflows/cross-platform-builds.yml
git commit -m "feat: Add cross-platform build workflow (Windows/macOS/Linux)"
git push origin master  # or your feature branch
```

**Expected Result**: GitHub Actions starts running builds for all platforms.

**Note**: Initial builds will **FAIL** on Windows due to missing code changes. This is expected.

---

## Phase 2: Fix Windows Build (2-3 hours)

### Step 2.1: Update CMakeLists.txt - Compiler Flags

**File**: `CMakeLists.txt` (lines 68-78)

**Find this code:**
```cmake
# SECURITY: Compiler and linker hardening flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong")

# SECURITY: Buffer overflow detection (only for Release builds with -O2)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")

# SECURITY: Read-only relocations and immediate binding
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,relro,-z,now")
```

**Replace with:**
```cmake
# SECURITY: Platform-appropriate compiler and linker hardening flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC and Clang: Stack protection
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong")

    # Buffer overflow detection (Release builds only, requires -O2)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
elseif(MSVC)
    # MSVC: Security Development Lifecycle checks
    add_compile_options(
        /GS        # Buffer security check
        /sdl       # Additional security checks
        /guard:cf  # Control Flow Guard
    )
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS  # Disable warnings for standard C functions
    )
endif()

# SECURITY: Linker hardening (Linux-specific)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Read-only relocations and immediate binding
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,relro,-z,now")
elseif(APPLE)
    # macOS: No equivalent flags needed (linker differences)
    # Stack protection is enabled by default on macOS
elseif(MSVC)
    # Windows: Enable ASLR and DEP
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DYNAMICBASE /NXCOMPAT")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DYNAMICBASE /NXCOMPAT")
endif()
```

**Verification**:
```bash
# Linux/macOS
git diff CMakeLists.txt

# Commit
git add CMakeLists.txt
git commit -m "fix: Platform-specific compiler hardening flags for Windows/macOS/Linux"
```

---

### Step 2.2: Update CMakeLists.txt - Disable Network Devices on Windows

**File**: `CMakeLists.txt` (lines 114-124)

**Find this section:**
```cmake
# Engine
src/engine/channelfactory.cxx
src/engine/converterengine.cxx
src/engine/descriptor.cxx
src/engine/devicefactory.cxx
src/engine/diskdevice.cxx
src/engine/serialdevice.cxx
src/engine/stddevice.cxx
src/engine/tcpdevice.cxx      # ← Problem: Unix sockets only
src/engine/udpdevice.cxx      # ← Problem: Unix sockets only
```

**Replace with:**
```cmake
# Engine - Core components (all platforms)
list(APPEND ASTERIX_LIB_SOURCES
    src/engine/channelfactory.cxx
    src/engine/converterengine.cxx
    src/engine/descriptor.cxx
    src/engine/devicefactory.cxx
    src/engine/diskdevice.cxx
    src/engine/stddevice.cxx
)

# Platform-specific network devices
if(UNIX)
    # Unix platforms (Linux, macOS): Use BSD sockets
    list(APPEND ASTERIX_LIB_SOURCES
        src/engine/tcpdevice.cxx
        src/engine/udpdevice.cxx
    )
    message(STATUS "Network device support: ENABLED (Unix sockets)")
elseif(WIN32)
    # Windows: Network devices require Winsock2 refactoring
    # TODO: Implement Windows socket support
    message(WARNING "Network device support: DISABLED (Windows Winsock2 support not yet implemented)")
endif()

# Serial device (platform-specific, disabled for now)
# TODO: Implement platform-specific serial support
if(FALSE)
    list(APPEND ASTERIX_LIB_SOURCES src/engine/serialdevice.cxx)
endif()
```

**Verification**:
```bash
git diff CMakeLists.txt
git add CMakeLists.txt
git commit -m "fix: Conditionally compile network devices (Unix only, Windows TODO)"
```

---

### Step 2.3: Update setup.py - Platform-Specific Python Flags

**File**: `setup.py` (lines 84-87)

**Find this code:**
```python
asterix_module = Extension('_asterix',
                           sources=[...],
                           include_dirs=['./asterix/python', './src/asterix', './src/engine'],
                           # SECURITY: Hardening flags for buffer overflow and stack protection
                           extra_compile_args=['-DPYTHON_WRAPPER', '-std=c++17',
                                             '-fstack-protector-strong', '-D_FORTIFY_SOURCE=2'],
                           # SECURITY: Read-only relocations for hardening
                           extra_link_args=['-lexpat', '-Wl,-z,relro,-z,now'])
```

**Replace with:**
```python
import platform
import sys

# ... (keep existing code above)

# Determine platform-specific compilation flags
extra_compile_args = ['-DPYTHON_WRAPPER']
extra_link_args = []

if sys.platform == 'win32':
    # Windows (MSVC)
    extra_compile_args.extend([
        '/std:c++17',      # C++17 standard
        '/GS',             # Buffer security check
        '/sdl',            # Security Development Lifecycle checks
        '/EHsc',           # Exception handling
    ])
    extra_link_args.extend([
        'expat.lib',       # Link expat library
    ])

elif sys.platform == 'darwin':
    # macOS (Clang)
    extra_compile_args.extend([
        '-std=c++17',
        '-fstack-protector-strong',
        '-stdlib=libc++',  # Use libc++ on macOS
    ])
    extra_link_args.extend([
        '-lexpat',
    ])

else:
    # Linux (GCC/Clang)
    extra_compile_args.extend([
        '-std=c++17',
        '-fstack-protector-strong',
        '-D_FORTIFY_SOURCE=2',
    ])
    extra_link_args.extend([
        '-lexpat',
        '-Wl,-z,relro,-z,now',  # Linker hardening (Linux only)
    ])

asterix_module = Extension('_asterix',
                           sources=['./src/python/asterix.c',
                                    './src/python/python_wrapper.c',
                                    './src/python/python_parser.cpp',
                                    # ... (keep all existing sources)
                                   ],
                           include_dirs=['./asterix/python', './src/asterix', './src/engine'],
                           extra_compile_args=extra_compile_args,
                           extra_link_args=extra_link_args)
```

**Verification**:
```bash
git diff setup.py
git add setup.py
git commit -m "fix: Platform-specific Python extension build flags (Windows/macOS/Linux)"
```

---

### Step 2.4: Push Changes and Verify CI

```bash
# Push all changes
git push origin master  # or your feature branch

# Monitor GitHub Actions
# Navigate to: https://github.com/{user}/{repo}/actions
# Watch "Cross-Platform Builds" workflow

# Expected: Windows builds should now SUCCEED ✅
```

---

## Phase 3: Local Testing (1 hour per platform)

### Windows Testing

```powershell
# Prerequisites
# 1. Install Visual Studio 2022
# 2. Install vcpkg: git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
# 3. Install expat: C:\vcpkg\vcpkg install expat:x64-windows

# Clone repository
git clone https://github.com/{user}/asterix.git
cd asterix

# Configure
cmake -B build `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Install
cmake --install build --config Release

# Test
.\install\bin\asterix.exe --version
.\install\bin\asterix.exe --help

# Run test suite
cd install\test
.\test.ps1
```

**Expected Output**: All tests PASS except network device tests (disabled).

---

### macOS Testing

```bash
# Prerequisites
# 1. Install Xcode: xcode-select --install
# 2. Install Homebrew: /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
# 3. Install dependencies: brew install cmake expat

# Clone repository
git clone https://github.com/{user}/asterix.git
cd asterix

# Configure
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64  # or x86_64 for Intel

# Build
cmake --build build --config Release

# Install
cmake --install build --config Release

# Test
./install/bin/asterix --version
./install/bin/asterix --help

# Run test suite
cd install/test
./test.sh
```

**Expected Output**: All tests PASS (11/11).

---

### Linux Testing

```bash
# Prerequisites (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y build-essential cmake libexpat1-dev gcc-13 g++-13

# Clone repository
git clone https://github.com/{user}/asterix.git
cd asterix

# Configure
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=gcc-13 \
  -DCMAKE_CXX_COMPILER=g++-13

# Build
cmake --build build --config Release

# Install
cmake --install build --config Release

# Test
./install/bin/asterix --version
./install/bin/asterix --help

# Run test suite
cd install/test
./test.sh
```

**Expected Output**: All tests PASS (11/11).

---

## Phase 4: Python Module Testing (30 minutes per platform)

### All Platforms

```bash
# Install build dependencies
pip install --upgrade pip setuptools wheel pytest pytest-cov

# Build Python module
python setup.py build

# Install
python setup.py install

# Run tests
pytest asterix/test/ -v --cov=asterix

# Expected: All tests PASS, coverage > 90%
```

**Platform-Specific Notes**:

**Windows**:
```powershell
# Set vcpkg toolchain before building
$env:CMAKE_TOOLCHAIN_FILE = "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
python setup.py build
```

**macOS**:
```bash
# Ensure correct architecture for Apple Silicon
export ARCHFLAGS="-arch arm64"
python setup.py build
```

**Linux**:
```bash
# Standard build
python setup.py build
```

---

## Phase 5: Verify CI/CD Success (10 minutes)

### Check GitHub Actions

1. Navigate to: `https://github.com/{user}/asterix/actions`
2. Select latest "Cross-Platform Builds" workflow run
3. Verify all jobs are green ✅:
   - Windows builds (2019, 2022) × (Release, Debug) = 4 jobs
   - macOS builds (13, 14) × (Release, Debug) = 4 jobs
   - Linux builds (22.04, 24.04) × (Release, Debug) = 4 jobs
   - Python builds (3.9-3.13) × (Windows, macOS, Linux) = ~15 jobs

### Download and Test Artifacts

```bash
# From GitHub Actions page:
# 1. Click on successful workflow run
# 2. Scroll to "Artifacts" section
# 3. Download platform-specific builds
# 4. Extract and test locally
```

**Example**:
```bash
# Linux
unzip asterix-linux-ubuntu-24.04-Release.zip
cd asterix-linux-ubuntu-24.04-Release
./bin/asterix --version

# Windows
# Extract asterix-windows-windows-2022-Release.zip
# Run: asterix.exe --version

# macOS
unzip asterix-macos-macos-14-Release.zip
cd asterix-macos-macos-14-Release
./bin/asterix --version
```

---

## Phase 6: Documentation Updates (30 minutes)

### Update README.md

Add cross-platform build badges:

```markdown
## Build Status

![Windows](https://github.com/{user}/asterix/workflows/Cross-Platform%20Builds/badge.svg?branch=master&event=push)
![macOS](https://github.com/{user}/asterix/workflows/Cross-Platform%20Builds/badge.svg?branch=master&event=push)
![Linux](https://github.com/{user}/asterix/workflows/Cross-Platform%20Builds/badge.svg?branch=master&event=push)

## Supported Platforms

- **Windows**: 10/11, Server 2019/2022 (MSVC 2022, C++23)
- **macOS**: 11.0+ Big Sur (Intel/ARM, Xcode 15+, C++23)
- **Linux**: Ubuntu 22.04+, RHEL 8+, Debian 11+ (GCC 13+, C++23)
- **Python**: 3.9, 3.10, 3.11, 3.12, 3.13 (all platforms)
```

Add build instructions:

```markdown
## Building from Source

### Windows (Visual Studio 2022)
See [CROSS_PLATFORM_BUILD_GUIDE.md](CROSS_PLATFORM_BUILD_GUIDE.md#windows-build)

### macOS (Xcode 15+)
See [CROSS_PLATFORM_BUILD_GUIDE.md](CROSS_PLATFORM_BUILD_GUIDE.md#macos-build)

### Linux (GCC 13+)
See [CROSS_PLATFORM_BUILD_GUIDE.md](CROSS_PLATFORM_BUILD_GUIDE.md#linux-build)
```

---

## Completion Checklist

### Phase 1: CI/CD
- [ ] `.github/workflows/cross-platform-builds.yml` committed and pushed
- [ ] GitHub Actions workflow triggered
- [ ] Initial build failures observed (expected)

### Phase 2: Code Changes
- [ ] CMakeLists.txt updated (compiler flags)
- [ ] CMakeLists.txt updated (network device conditional)
- [ ] setup.py updated (platform-specific flags)
- [ ] Changes committed and pushed
- [ ] GitHub Actions shows Windows builds passing ✅

### Phase 3: Local Testing
- [ ] Windows build tested locally (if available)
- [ ] macOS build tested locally (if available)
- [ ] Linux build tested locally
- [ ] Test scripts run successfully on each platform

### Phase 4: Python Module
- [ ] Python module builds on Windows
- [ ] Python module builds on macOS
- [ ] Python module builds on Linux
- [ ] pytest passes on all platforms (> 90% coverage)

### Phase 5: CI/CD Verification
- [ ] All GitHub Actions jobs passing
- [ ] Artifacts downloadable and functional
- [ ] No build warnings or errors

### Phase 6: Documentation
- [ ] README.md updated with build badges
- [ ] README.md updated with platform support
- [ ] Build instructions reference CROSS_PLATFORM_BUILD_GUIDE.md
- [ ] CHANGELOG.md updated (if applicable)

---

## Success Criteria

✅ **Windows**: Builds, installs, and runs (file parsing works, network disabled)
✅ **macOS**: Builds, installs, and runs (all features work)
✅ **Linux**: Builds, installs, and runs (all features work)
✅ **Python**: Module builds and tests pass on all platforms
✅ **CI/CD**: All automated builds pass (40+ build/test combinations)
✅ **Documentation**: Build instructions clear and complete

---

## Troubleshooting

### Windows Build Fails with "cannot find expat.h"
```powershell
# Verify vcpkg installation
C:\vcpkg\vcpkg list expat

# Reinstall if needed
C:\vcpkg\vcpkg install expat:x64-windows --recurse

# Verify CMake toolchain
cmake -B build -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

### macOS Build Fails with "library not found"
```bash
# Check Homebrew expat
brew list expat
brew info expat

# Reinstall if needed
brew reinstall expat

# Verify library path
ls -la /opt/homebrew/lib/libexpat*  # ARM
ls -la /usr/local/lib/libexpat*      # Intel
```

### Linux Build Fails with "undefined reference to `XML_*`"
```bash
# Install expat development package
sudo apt-get install libexpat1-dev  # Debian/Ubuntu
sudo dnf install expat-devel        # RHEL/Fedora

# Verify installation
pkg-config --cflags --libs expat
```

### Python Module Import Error
```bash
# Windows: Missing Visual C++ Redistributable
# Download from: https://aka.ms/vs/17/release/vc_redist.x64.exe

# macOS: Incorrect architecture
arch -arm64 python setup.py build  # Force ARM64

# Linux: Missing libexpat.so
sudo ldconfig  # Refresh library cache
```

---

## Time Estimates

| Phase | Task                          | Time      | Dependencies        |
|-------|-------------------------------|-----------|---------------------|
| 1     | Deploy CI/CD workflow         | 30 min    | GitHub access       |
| 2     | Fix Windows build             | 2-3 hours | Text editor         |
| 3     | Local Windows testing         | 1 hour    | Windows machine     |
| 3     | Local macOS testing           | 1 hour    | macOS machine       |
| 3     | Local Linux testing           | 1 hour    | Linux machine       |
| 4     | Python module testing (all)   | 1.5 hours | Python 3.9+         |
| 5     | Verify CI/CD                  | 10 min    | GitHub Actions      |
| 6     | Update documentation          | 30 min    | Markdown editor     |
|       | **Total**                     | **8-10 hours** |                |

**Note**: Time estimates assume familiarity with CMake, Python, and basic C++ development.

---

## Next Steps After Completion

1. **Network Device Support (Windows)**:
   - Implement Winsock2 abstraction layer
   - Port UDP/TCP devices to Windows
   - Estimated: 2-3 weeks

2. **Installer Creation**:
   - Windows: WiX Toolset MSI
   - macOS: create-dmg DMG
   - Linux: DEB/RPM (already working)
   - Estimated: 1-2 weeks

3. **Serial Device Support**:
   - Windows: COM port API
   - Unix: existing termios (keep)
   - Estimated: 1-2 weeks

4. **Performance Optimization**:
   - Platform-specific profiling
   - Benchmark comparisons
   - Estimated: 1 week

5. **Distribution**:
   - PyPI: Publish wheels
   - GitHub Releases: Attach installers
   - Package managers: vcpkg, Homebrew, apt
   - Estimated: 1 week

---

**Document Version**: 1.0
**Last Updated**: 2025-11-01
**Estimated Total Implementation Time**: 8-10 hours (Phase 1-6)
