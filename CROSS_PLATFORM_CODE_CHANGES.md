# Cross-Platform Code Changes Required

This document details the specific code modifications needed to achieve full Windows/macOS/Linux compatibility for the ASTERIX decoder project.

## Overview

The ASTERIX codebase is currently Linux-centric. To support Windows and macOS, we need to:

1. Abstract platform-specific networking APIs
2. Conditionally compile platform-specific code
3. Update build configurations for different compilers
4. Handle path separators and filesystem differences

---

## Priority 1: Critical Changes (Required for Windows Build)

### 1.1 CMakeLists.txt - Platform-Specific Compiler Flags

**File**: `/CMakeLists.txt`
**Lines**: 68-78
**Issue**: GCC-specific hardening flags cause MSVC compilation errors

**Current Code:**
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

**Proposed Fix:**
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

**Testing**: Build on all three platforms and verify no compilation errors.

---

### 1.2 CMakeLists.txt - Conditional Network Device Compilation

**File**: `/CMakeLists.txt`
**Lines**: 119-124
**Issue**: Network device code uses Unix-only socket APIs

**Current Code:**
```cmake
# Engine
src/engine/channelfactory.cxx
src/engine/converterengine.cxx
src/engine/descriptor.cxx
src/engine/devicefactory.cxx
src/engine/diskdevice.cxx
src/engine/serialdevice.cxx
src/engine/stddevice.cxx
src/engine/tcpdevice.cxx      # ← Unix sockets only
src/engine/udpdevice.cxx      # ← Unix sockets only
```

**Proposed Fix (Short-term - Disable on Windows):**
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

# Serial device (platform-specific, disable on all for now)
if(FALSE)  # Enable when platform-specific serial support is implemented
    list(APPEND ASTERIX_LIB_SOURCES src/engine/serialdevice.cxx)
endif()
```

**Proposed Fix (Long-term - Platform Abstraction):**
See sections 2.1 and 2.2 for full socket abstraction layer.

**Testing**:
- Windows: Build succeeds without network devices
- Linux/macOS: Build succeeds with network devices enabled

---

### 1.3 setup.py - Platform-Specific Python Build Flags

**File**: `/setup.py`
**Lines**: 84-87
**Issue**: GCC-specific flags break MSVC Python extension build

**Current Code:**
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

**Proposed Fix:**
```python
import platform
import sys

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
                           sources=[...],  # existing sources
                           include_dirs=['./asterix/python', './src/asterix', './src/engine'],
                           extra_compile_args=extra_compile_args,
                           extra_link_args=extra_link_args)
```

**Testing**:
- `python setup.py build` on Windows, macOS, Linux
- `pip install -e .` on all platforms
- Run Python tests: `python -m pytest asterix/test/`

---

## Priority 2: Networking Code Abstraction (Medium Priority)

### 2.1 Platform-Specific Socket Headers

**Files**:
- `src/engine/udpdevice.hxx` (lines 27-31)
- `src/engine/tcpdevice.hxx` (lines 26-28)

**Issue**: Unix socket headers not available on Windows

**Current Code (`udpdevice.hxx`):**
```cpp
#include <sys/socket.h>
#include <sys/select.h> // fd_set
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
```

**Proposed Fix:**
```cpp
// Platform-specific socket headers
#ifdef _WIN32
  // Windows Sockets 2
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")

  // Type compatibility aliases
  typedef int socklen_t;

  // Function name compatibility
  #define close closesocket
  #define ioctl ioctlsocket

#else
  // Unix (Linux, macOS, BSD)
  #include <sys/socket.h>
  #include <sys/select.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <fcntl.h>
#endif

#include <vector>
```

**Apply same fix to `tcpdevice.hxx`**

---

### 2.2 Socket Initialization (Windows WSAStartup Required)

**Files**: `src/engine/udpdevice.cxx`, `src/engine/tcpdevice.cxx`

**Issue**: Windows requires `WSAStartup()` before using sockets

**Proposed Solution**: Add platform-specific initialization helper

**New File**: `src/engine/socket_platform.h`
```cpp
#ifndef SOCKET_PLATFORM_H
#define SOCKET_PLATFORM_H

#ifdef _WIN32
  #include <winsock2.h>

  // RAII wrapper for WSA initialization
  class WSAInitializer {
  public:
    WSAInitializer() {
      WSADATA wsaData;
      int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
      if (result != 0) {
        // Handle error
      }
    }

    ~WSAInitializer() {
      WSACleanup();
    }

    // Singleton access
    static WSAInitializer& instance() {
      static WSAInitializer inst;
      return inst;
    }
  };

  // Call this before any socket operations
  inline void initializeSockets() {
    WSAInitializer::instance();
  }

#else
  // Unix: No initialization needed
  inline void initializeSockets() { }
#endif

#endif // SOCKET_PLATFORM_H
```

**Usage in `udpdevice.cxx` constructor:**
```cpp
#include "socket_platform.h"

CUdpDevice::CUdpDevice(CDescriptor &descriptor) : CBaseDevice(descriptor) {
    // Initialize sockets (no-op on Unix, WSAStartup on Windows)
    initializeSockets();

    // ... existing code
}
```

---

### 2.3 Socket Error Handling (errno vs WSAGetLastError)

**Files**: `src/engine/udpdevice.cxx`, `src/engine/tcpdevice.cxx`

**Issue**: Windows uses `WSAGetLastError()`, Unix uses `errno`

**Proposed Solution**: Platform-agnostic error function

**Add to `socket_platform.h`:**
```cpp
#ifdef _WIN32
  inline int getLastSocketError() {
    return WSAGetLastError();
  }

  inline void setLastSocketError(int error) {
    WSASetLastError(error);
  }

  // Windows error codes
  #define SOCKET_EWOULDBLOCK WSAEWOULDBLOCK
  #define SOCKET_EINPROGRESS WSAEINPROGRESS
  #define SOCKET_EINTR       WSAEINTR

#else
  #include <errno.h>

  inline int getLastSocketError() {
    return errno;
  }

  inline void setLastSocketError(int error) {
    errno = error;
  }

  // Unix error codes
  #define SOCKET_EWOULDBLOCK EWOULDBLOCK
  #define SOCKET_EINPROGRESS EINPROGRESS
  #define SOCKET_EINTR       EINTR
#endif
```

**Usage Example:**
```cpp
// Instead of:
if (errno == EWOULDBLOCK) { ... }

// Use:
if (getLastSocketError() == SOCKET_EWOULDBLOCK) { ... }
```

---

## Priority 3: Optional Platform Enhancements

### 3.1 Serial Device Platform Support

**File**: `src/engine/serialdevice.cxx`

**Issue**: Uses Unix termios API, not available on Windows

**Proposed Solution**: Implement Windows COM port support

**Simplified Structure:**
```cpp
#ifdef _WIN32
  // Windows COM port implementation using CreateFile, ReadFile, WriteFile
  #include <windows.h>

  class CSerialDeviceImpl {
    HANDLE m_hComm;
    // ... Windows-specific members
  };

#else
  // Unix termios implementation (existing code)
  #include <termios.h>
  #include <fcntl.h>

  class CSerialDeviceImpl {
    int m_fd;
    struct termios m_oldtio, m_newtio;
    // ... Unix-specific members
  };
#endif
```

---

### 3.2 File Path Handling

**Issue**: Hardcoded Unix path separators (`/`)

**Proposed Solution**: Use `std::filesystem` (C++17)

**Example:**
```cpp
#include <filesystem>
namespace fs = std::filesystem;

// Instead of:
std::string configPath = "../config/asterix.ini";

// Use:
fs::path configPath = fs::path("..") / "config" / "asterix.ini";
std::string configPathStr = configPath.string();
```

---

## Testing Strategy

### Phase 1: Build Verification

1. **Linux**:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```

2. **macOS**:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
   cmake --build build
   ```

3. **Windows**:
   ```powershell
   cmake -B build -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release
   ```

### Phase 2: Functionality Testing

1. **Basic Execution**: `./asterix --help` on all platforms
2. **Python Module**: `python -m asterix` on all platforms
3. **Integration Tests**: Run `test.sh` (Unix) or `test.ps1` (Windows)

### Phase 3: Cross-Platform CI

- Use GitHub Actions matrix strategy
- Test on windows-2022, macos-14, ubuntu-24.04
- Automated builds on every commit

---

## Implementation Roadmap

### Milestone 1: Windows Build Compiles (Week 1)
- [ ] Update CMakeLists.txt with platform detection (1.1)
- [ ] Conditionally exclude network devices on Windows (1.2)
- [ ] Update setup.py for MSVC (1.3)
- [ ] Test: `cmake --build build` succeeds on Windows

### Milestone 2: Basic Windows Functionality (Week 2)
- [ ] File input/output works on Windows
- [ ] PCAP parsing works on Windows
- [ ] Python module builds and imports on Windows
- [ ] Test: Parse sample PCAP file on Windows

### Milestone 3: Network Support (Week 3-4)
- [ ] Implement socket abstraction layer (2.1, 2.2, 2.3)
- [ ] Port UDP device to Windows
- [ ] Port TCP device to Windows
- [ ] Test: Network multicast on Windows

### Milestone 4: Full Cross-Platform Parity (Week 5-6)
- [ ] Serial device support on Windows (3.1)
- [ ] Automated installers (MSI, DMG, DEB)
- [ ] CI/CD pipeline fully operational
- [ ] Test: All integration tests pass on all platforms

---

## Dependencies

### Windows-Specific Dependencies

1. **Visual Studio 2022** with:
   - MSVC v143 toolset
   - Windows 10/11 SDK
   - C++ CMake tools

2. **vcpkg** for libraries:
   ```powershell
   vcpkg install expat:x64-windows
   ```

3. **Optional**: WiX Toolset for MSI installers

### macOS-Specific Dependencies

1. **Xcode 15.0+** with Command Line Tools
2. **Homebrew** packages:
   ```bash
   brew install cmake expat
   ```

### Linux-Specific Dependencies

1. **Build tools**:
   ```bash
   sudo apt-get install build-essential cmake libexpat1-dev
   ```

---

## Validation Checklist

Before merging cross-platform changes:

- [ ] Code compiles without errors on Windows, macOS, Linux
- [ ] No new compiler warnings on any platform
- [ ] All existing tests pass on all platforms
- [ ] Python module installs via `pip install` on all platforms
- [ ] Executable runs and produces expected output on all platforms
- [ ] Memory leaks checked (valgrind on Linux, no regressions)
- [ ] Documentation updated (README, build guides)
- [ ] CI/CD pipeline passes all checks

---

## References

- [CMake Platform Detection](https://cmake.org/cmake/help/latest/variable/CMAKE_SYSTEM_NAME.html)
- [Windows Sockets 2 (Winsock2)](https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-start-page-2)
- [MSVC Compiler Options](https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options)
- [Python Extension Building](https://docs.python.org/3/extending/building.html)
- [std::filesystem Documentation](https://en.cppreference.com/w/cpp/filesystem)
