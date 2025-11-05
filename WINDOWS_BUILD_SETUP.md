# Windows Build Setup Guide

This guide helps you set up and run Windows builds locally to match the GitHub Actions workflow.

## Quick Start

1. **Ensure Visual Studio 2022 is installed** (Community edition is free)
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Select "Desktop development with C++" workload during installation
   - This includes MSVC compiler, Windows SDK, and CMake tools

2. **Install CMake 3.20+** (if not already installed)
   - Download from: https://cmake.org/download/
   - Or use: `winget install Kitware.CMake` or `choco install cmake`
   - Add to PATH during installation

3. **Run the build script:**
   ```powershell
   .\build-windows-local.ps1
   ```

The script will automatically:
- Detect Visual Studio 2022
- Set up vcpkg (if not already installed)
- Install dependencies (expat)
- Configure and build the project
- Test the executable

## What Was Fixed

### 1. CMakeLists.txt - Platform-Specific Compiler Flags

**Problem:** GCC-specific compiler flags (`-fstack-protector-strong`, `-D_FORTIFY_SOURCE=2`, `-Wl,-z,relro`) were being applied to all platforms, causing MSVC build failures.

**Solution:** Made flags platform-specific:
- **MSVC:** Uses `/GS` (stack protection), `/sdl` (security checks), `/W4` (warnings)
- **GCC/Clang:** Uses `-fstack-protector-strong` and `-D_FORTIFY_SOURCE=2`
- **Linux only:** Uses `-Wl,-z,relro,-z,now` (read-only relocations)

### 2. Networking Code

**Status:** Already has Windows compatibility! The `tcpdevice.cxx` and `udpdevice.cxx` files include `#ifdef _WIN32` blocks that use Winsock2 API, so they work on Windows.

## Build Script Options

```powershell
# Build Release (default)
.\build-windows-local.ps1

# Build Debug
.\build-windows-local.ps1 -Config Debug

# Build both Release and Debug
.\build-windows-local.ps1 -Config Both

# Use custom vcpkg location
.\build-windows-local.ps1 -VcpkgRoot "D:\vcpkg"
```

## Manual Build Steps (if needed)

If you prefer to build manually or need to troubleshoot:

### 1. Set up vcpkg

```powershell
# Clone vcpkg (if not already done)
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg

# Bootstrap vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat

# Install expat
.\vcpkg.exe install expat:x64-windows

# Integrate with system
.\vcpkg.exe integrate install
```

### 2. Configure CMake

```powershell
# Navigate to your asterix repository directory
cd <path-to-asterix-repo>

cmake -B build `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_CXX_STANDARD=20 `
  -DCMAKE_CXX_STANDARD_REQUIRED=ON `
  -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DBUILD_SHARED_LIBS=ON `
  -DBUILD_STATIC_LIBS=ON `
  -DBUILD_EXECUTABLE=ON `
  -DBUILD_TESTING=OFF `
  -DCMAKE_INSTALL_PREFIX="$PWD\install"
```

### 3. Build

```powershell
cmake --build build --config Release --parallel 4
```

### 4. Install

```powershell
cmake --install build --config Release
```

### 5. Test

```powershell
.\install\bin\asterix.exe --help
.\install\bin\asterix.exe --version
```

## Troubleshooting

### CMake not found
- Ensure CMake is installed and added to PATH
- Restart PowerShell after installing CMake
- Verify: `cmake --version`

### Visual Studio not found
- Install Visual Studio 2022 Community
- Ensure "Desktop development with C++" workload is selected
- The script looks for VS in standard locations:
  - `C:\Program Files\Microsoft Visual Studio\2022\Community`
  - `C:\Program Files\Microsoft Visual Studio\2022\Professional`
  - `C:\Program Files\Microsoft Visual Studio\2022\Enterprise`

### vcpkg errors
- Ensure Git is installed (needed to clone vcpkg)
- Check internet connection (vcpkg downloads dependencies)
- Try running vcpkg commands manually from `C:\vcpkg`

### Build errors
- Check that all dependencies are installed via vcpkg
- Ensure you're using the correct architecture (x64)
- Check that Visual Studio Build Tools are installed

## Matching GitHub Actions

The build script replicates the exact steps from `.github/workflows/cross-platform-builds.yml`:
- Same CMake generator: "Visual Studio 17 2022"
- Same CMake configuration options
- Same build process (configure → build → install → test)
- Same dependency management (vcpkg with expat:x64-windows)

## Next Steps

After a successful local build:
1. Test the executable thoroughly
2. Compare output with GitHub Actions artifacts
3. Fix any remaining issues
4. Push changes to trigger GitHub Actions workflow

