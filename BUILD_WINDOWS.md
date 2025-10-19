# Building ASTERIX on Windows

This document describes how to build ASTERIX on Windows without major code modifications.

## Prerequisites

**C++ Compiler Requirements:**

This project requires a C++17 compatible compiler. Supported compilers:

- **Visual Studio**: 2017 version 15.3 or later (recommended: VS 2019 or VS 2022)
  - C++17 support enabled by default in VS 2017 15.3+
- **MinGW-w64 GCC**: 7.0 or later (recommended: GCC 9+)
  - Included with MSYS2 installation
- **Clang for Windows**: 5.0 or later (optional)

### Option 1: MSYS2/MinGW-w64 (Recommended)

1. Install MSYS2 from https://www.msys2.org/
2. Open MSYS2 MinGW 64-bit terminal
3. Install dependencies:
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-expat make
```

**Note:** This installs GCC 13+ which has full C++17 support.

### Option 2: Visual Studio (Native Windows)

1. Install Visual Studio 2017 15.3+ or later with C++ support
   - **Recommended:** Visual Studio 2019 or Visual Studio 2022
   - Ensure "Desktop development with C++" workload is installed
   - C++17 features are enabled by default
2. Install CMake: https://cmake.org/download/
3. Install vcpkg for dependency management:
```powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg integrate install
./vcpkg install expat:x64-windows
```

## Building with MSYS2/MinGW

```bash
# Clone repository
git clone https://github.com/montge/asterix.git
cd asterix

# Create build directory
mkdir build && cd build

# Configure (MinGW Makefiles)
cmake .. -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_STATIC_LIBS=ON \
    -DBUILD_EXECUTABLE=ON

# Build
mingw32-make -j4

# Install (optional)
mingw32-make install
```

## Building with Visual Studio

```powershell
# Clone repository
git clone https://github.com/montge/asterix.git
cd asterix

# Create build directory
mkdir build
cd build

# Configure with vcpkg toolchain
cmake .. -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" `
    -DCMAKE_BUILD_TYPE=Release `
    -DBUILD_SHARED_LIBS=ON `
    -DBUILD_STATIC_LIBS=ON `
    -DBUILD_EXECUTABLE=ON

# Build
cmake --build . --config Release

# Install (optional)
cmake --install . --config Release
```

## Building with CMake GUI

1. Open CMake GUI
2. Set source directory to asterix repository
3. Set build directory to asterix/build
4. Click "Configure"
5. Select generator (Visual Studio or MinGW Makefiles)
6. Set options:
   - BUILD_SHARED_LIBS: ON
   - BUILD_STATIC_LIBS: ON
   - BUILD_EXECUTABLE: ON
7. Click "Generate"
8. Click "Open Project" or build with make/nmake

## Compiler Version Verification

Verify your compiler supports C++17:

**Visual Studio:**
```powershell
cl /? | findstr /C:"Version"
# Should show version 19.11 or higher (VS 2017 15.3+)
```

**MinGW-w64 GCC:**
```bash
g++ --version
# Should show GCC 7.0 or higher
```

## Known Limitations

1. **C++17 Requirement**: Older compilers (GCC < 7, MSVC < 2017 15.3) are not supported
2. **Network Features**: UDP multicast may require Windows-specific socket configuration
3. **Serial Device**: Serial port access requires Windows COM port handling
4. **Path Separators**: Use forward slashes (/) or escaped backslashes (\\\\) in configuration paths

## Creating Windows Installer

For creating a Windows installer (.msi), we recommend using WiX Toolset or NSIS:

### Using CPack with NSIS

```bash
# In build directory
cpack -G NSIS
```

This requires NSIS installed from https://nsis.sourceforge.io/

### Using CPack with WiX

```bash
# In build directory
cpack -G WIX
```

This requires WiX Toolset installed from https://wixtoolset.org/

## Binary Distribution

Pre-built Windows binaries may be available in GitHub Releases:
https://github.com/montge/asterix/releases

Look for files like:
- `asterix-2.8.9-win64.zip` - Portable ZIP archive
- `asterix-2.8.9-win64.exe` - NSIS installer
- `asterix-2.8.9-win64.msi` - WiX MSI installer

## Testing

```bash
# Run tests (if built with -DBUILD_TESTING=ON)
ctest --output-on-failure

# Test the executable
./bin/asterix --version
./bin/asterix --help
```

## Troubleshooting

### expat Not Found

If CMake cannot find expat:
- MSYS2: Ensure mingw-w64-x86_64-expat is installed
- vcpkg: Ensure expat:x64-windows is installed and CMAKE_TOOLCHAIN_FILE is set

### Build Errors

Common issues:
- Use forward slashes in paths, not backslashes
- Ensure all dependencies are for the same architecture (x64 vs x86)
- For MSYS2, use the correct terminal (MinGW 64-bit, not MSYS2 MSYS)

## Contributing

Windows-specific improvements are welcome! Please test on Windows 10/11 and submit pull requests.
