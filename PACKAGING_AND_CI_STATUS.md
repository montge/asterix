# Packaging and CI Status

## Overview

This document tracks packaging and CI/CD status for all three language bindings: C++, Python, and Rust.

## ✅ Packaging Complete

### Debian/Ubuntu Packages (.deb)
**Status**: Fully configured and ready to build

**Supported Platforms**:
- Ubuntu 20.04 LTS (Focal Fossa)
- Ubuntu 22.04 LTS (Jammy Jellyfish)  
- Ubuntu 24.04 LTS (Noble Numbat)
- Debian 11 (Bullseye)
- Debian 12 (Bookworm)

**Packages**:
- `libasterix2` - Shared library
- `libasterix-dev` - Development headers and static library
- `asterix-tools` - Command-line executable

**Files**:
- `debian/control` - Package definitions
- `debian/rules` - Build rules (executable)
- `debian/changelog` - Version history
- `debian/copyright` - License information
- `debian/compat` - Debhelper v13
- `debian/*.install` - Installation manifests

### RedHat/CentOS Packages (.rpm)
**Status**: Fully configured and ready to build

**Supported Platforms**:
- RHEL 8, 9
- CentOS Stream 8, 9
- Rocky Linux 8, 9
- Fedora 38, 39, 40

**Packages**:
- `asterix-libs` - Shared library
- `asterix-devel` - Development files
- `asterix-tools` - Command-line tools

**Files**:
- `asterix.spec` - Complete RPM specification

### Windows Build Support
**Status**: Documented and tested approach

**Build Methods**:
- MSYS2/MinGW-w64 (recommended)
- Visual Studio + vcpkg
- CMake GUI

**Installers**:
- NSIS (.exe installer)
- WiX Toolset (.msi installer)
- Portable ZIP archive

**Files**:
- `BUILD_WINDOWS.md` - Comprehensive build guide

### Rust Crate Packaging
**Status**: Ready for crates.io publication

**Supported Platforms**:
- Linux x86_64, ARM64
- Windows (MSVC 2019+, MinGW)
- macOS Intel & ARM (M1/M2)

**Package**:
- `asterix-decoder` - Rust crate with C++ FFI bindings

**Files**:
- `asterix-rs/Cargo.toml` - Package manifest
- `asterix-rs/build.rs` - Build script (compiles C++ core)
- `asterix-rs/README.md` - Crate documentation
- `asterix-rs/PUBLISHING_CHECKLIST.md` - Publishing guide

**Installation**:
```bash
cargo add asterix-decoder
```

**Build from source**:
```bash
cd asterix-rs
cargo build --release
cargo test --all-features
```

## ✅ CI/CD Workflows

### Package Building (`build-packages.yml`)
**Status**: Configured and ready

**Triggers**:
- Release creation
- Version tags (v*)
- Manual workflow dispatch

**Actions**:
1. Build DEB packages for all Ubuntu/Debian versions
2. Build RPM packages for all RHEL/Fedora versions
3. Build source tarballs (TGZ, ZIP)
4. Test package installation
5. Upload to GitHub Releases

### Code Quality (`ci-verification.yml`)
**Status**: Active ✅

**Triggers**: Push to master, PR to master, nightly schedule

**Jobs**:
- Build C++ (Make)
- Build C++ (CMake)
- Test Python (3.10, 3.11, 3.12, 3.13, 3.14)
- Build Rust (Cargo)
- Test Rust (cargo test --all-features)
- Rust Clippy (linter)
- Rust Benchmarks
- C++ Coverage Analysis
- Memory Check (Valgrind)
- Static Analysis (cppcheck, clang-tidy)
- Integration Tests

### Security Scanning (`codeql-analysis.yml`)
**Status**: Active ✅

**Triggers**: Push to master, PR to master, weekly schedule

**Languages**: C++, Python

**Configuration**: Uses @v3 actions (latest)

## ✅ Dependency Management

### Dependabot (`dependabot.yml`)
**Status**: Configured ✅

**Monitors**:
- GitHub Actions (weekly, Mondays)
- Python dependencies (weekly, Tuesdays)
- Rust/Cargo dependencies (weekly, Wednesdays)

**Features**:
- Automatic PR creation
- Labeled PRs (dependencies, github-actions, python, rust)
- Up to 10 open PRs per ecosystem

## 🎯 Next Steps to Create Release

### 1. Create a Release Tag
```bash
git tag -a v2.8.9 -m "Release version 2.8.9 - 92.2% test coverage"
git push origin v2.8.9
```

### 2. Create GitHub Release
- Go to: https://github.com/montge/asterix/releases/new
- Tag: v2.8.9
- Title: "ASTERIX v2.8.9 - 92.2% Test Coverage"
- Description: Release notes

### 3. Automated Package Build
The `build-packages.yml` workflow will automatically:
- Build packages for all platforms
- Upload artifacts
- Attach packages to the release

### 4. Download Packages
Users can download from: https://github.com/montge/asterix/releases

**Available formats**:
- `asterix_2.8.9-1_amd64.deb` (Debian/Ubuntu - C++ & Python)
- `asterix-libs-2.8.9-1.x86_64.rpm` (RHEL/Fedora - C++ & Python)
- `asterix-2.8.9-Source.tar.gz` (Source - All languages)
- `asterix-2.8.9-Linux.tar.gz` (Binary - C++ & Python)
- `asterix-decoder-0.1.0.crate` (Rust crate - crates.io)

**Language-specific packages**:
- **Python**: `pip install asterix_decoder` (PyPI)
- **Rust**: `cargo add asterix-decoder` (crates.io)
- **C++**: Use DEB/RPM packages above

## 📊 Current Status Summary

✅ **Coverage**: 92.2% (exceeded 90% goal!)
✅ **Tests**: 560 tests, 100% passing (C++ & Python)
✅ **Packaging**:
   - C++: Debian, RPM, Windows (ZIP/MSI/EXE)
   - Python: PyPI (3.10-3.14)
   - Rust: crates.io (ready to publish)
✅ **CI/CD**: All workflows active for C++, Python, Rust
✅ **Security**: CodeQL scanning enabled (C++ & Python)
✅ **Dependencies**: Dependabot monitoring active (GitHub Actions, Python, Rust)
✅ **Documentation**: Complete for all platforms and all three languages
✅ **Cross-platform**: Linux (x86_64, ARM64), Windows, macOS (Intel & M1)

## ❓ CodeQL Note

If you see a "disabled" CodeQL job in GitHub's Security tab, it's likely the automatic code scanning that GitHub offers. Since we have an explicit CodeQL workflow (`.github/workflows/codeql-analysis.yml`), you can safely:

1. **Use our workflow** (recommended): It's configured with security-extended queries and proper build steps
2. **Or enable default**: GitHub's default scanning, but our custom one is more comprehensive

Both won't conflict - GitHub will use whichever is more recent/comprehensive.
