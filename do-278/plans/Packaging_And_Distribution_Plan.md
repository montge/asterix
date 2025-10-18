# Packaging and Distribution Plan
## ASTERIX Decoder - DO-278A AL-3

**Document ID:** PDP-ASTERIX-001
**Revision:** 1.0
**Date:** 2025-10-17

---

## 1. Packaging Goals

### 1.1 Target Platforms

**Primary Platforms:**
- Linux (Debian/Ubuntu - .deb packages)
- Linux (RedHat/CentOS/Fedora - .rpm packages)
- POSIX OS (BSD variants, RTOS)
- Windows (MSVC, MinGW/Cygwin)

**Secondary Platforms:**
- macOS (already supported)
- Embedded Linux (cross-compilation)

### 1.2 Library Formats

**C/C++ Libraries:**
- **Shared Libraries (.so, .dll, .dylib)** - Dynamic linking
- **Static Libraries (.a, .lib)** - Static linking
- **Header-only option** - For template-heavy code (if applicable)

**Python Packages:**
- **Source Distribution (sdist)** - Platform-independent source
- **Binary Wheels (.whl)** - Platform-specific pre-built binaries
- **Conda packages** - For scientific/aerospace Python users

### 1.3 Package Contents

Each package shall include:
- Compiled libraries (shared and/or static)
- Header files (C/C++ development)
- Configuration files (ASTERIX XML definitions)
- Documentation
- Examples
- License information

---

## 2. Build System Strategy

### 2.1 Current State

**Existing Build Systems:**
- GNU Make (Linux/POSIX) - `src/Makefile`
- CMake (cross-platform) - `CMakeLists.txt`
- Python setuptools - `setup.py`

**Recommendation:** Standardize on **CMake as primary build system**

**Rationale:**
- Cross-platform support (Windows, Linux, BSD, macOS)
- Supports both shared and static library builds
- Integrates with CPack for package generation
- Industry standard for C/C++ projects
- Supports RTOS/embedded cross-compilation

### 2.2 Build Targets

```cmake
# Static library
add_library(asterix_static STATIC ${SOURCES})

# Shared library
add_library(asterix_shared SHARED ${SOURCES})

# Python extension (current)
add_library(_asterix MODULE ${PYTHON_SOURCES})

# Executable
add_executable(asterix ${CLI_SOURCES})
```

### 2.3 Installation Layout

```
Standard installation tree:
/usr/local/
├── bin/
│   └── asterix                    # CLI executable
├── lib/
│   ├── libasterix.a              # Static library
│   ├── libasterix.so.1.0         # Shared library
│   └── libasterix.so -> libasterix.so.1.0
├── include/
│   └── asterix/
│       ├── asterix.h             # Main C API header
│       ├── AsterixDefinition.h   # C++ headers
│       └── ...
└── share/
    └── asterix/
        ├── config/               # ASTERIX XML configs
        │   └── asterix_cat*.xml
        ├── examples/             # Usage examples
        └── doc/                  # Documentation
            └── do-278/
```

---

## 3. Platform-Specific Packaging

### 3.1 Linux - Debian/Ubuntu (.deb)

**Tools:** dpkg-deb, fpm, or CPack

**Package Structure:**
```
asterix/
├── DEBIAN/
│   ├── control              # Package metadata
│   ├── postinst            # Post-install script
│   └── prerm               # Pre-remove script
├── usr/
│   ├── bin/
│   ├── lib/
│   ├── include/
│   └── share/
└── etc/
    └── asterix/
        └── asterix.conf
```

**Package Names:**
- `libasterix1` - Shared library runtime
- `libasterix-dev` - Development headers/static lib
- `asterix-tools` - CLI tools
- `python3-asterix` - Python bindings

**Build Command:**
```bash
cmake . -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
make package
# Creates: asterix_1.0.0_amd64.deb
```

**Dependencies:**
- `libexpat1` (runtime)
- `libexpat1-dev` (development)
- `python3` (for Python bindings)

### 3.2 Linux - RedHat/CentOS (.rpm)

**Tools:** rpmbuild, fpm, or CPack

**Spec File Structure:**
```spec
Name:           libasterix
Version:        1.0.0
Release:        1%{?dist}
Summary:        ASTERIX protocol decoder library
License:        GPL-3.0
URL:            https://github.com/montge/asterix

BuildRequires:  gcc-c++, cmake, expat-devel
Requires:       expat

%description
ASTERIX (All Purpose STructured EUROCONTROL SuRveillance
Information EXchange) protocol decoder library.

%package devel
Summary:        Development files for libasterix
Requires:       %{name} = %{version}-%{release}

%package tools
Summary:        ASTERIX command-line tools
Requires:       %{name} = %{version}-%{release}
```

**Build Command:**
```bash
rpmbuild -ba asterix.spec
# Creates: libasterix-1.0.0-1.x86_64.rpm
```

### 3.3 POSIX OS (BSD, RTOS)

**Strategy:** Source distribution with autotools or CMake

**BSD Ports/Packages:**
- FreeBSD: Create port in `/usr/ports/devel/asterix`
- NetBSD: Create package in `pkgsrc/devel/asterix`
- OpenBSD: Create port in `ports/devel/asterix`

**RTOS Considerations:**
- **VxWorks:** Static linking preferred, cross-compile with Wind River
- **QNX:** Use qcc compiler, create .qpk package
- **Embedded Linux:** Cross-compile with toolchain
- **Minimal dependencies:** Only libexpat required

**CMake Cross-Compilation:**
```cmake
# Toolchain file for cross-compilation
cmake -DCMAKE_TOOLCHAIN_FILE=arm-linux.cmake .
```

### 3.4 Windows

**Build Options:**
1. **MSVC (Visual Studio)** - Native Windows
2. **MinGW** - GNU toolchain for Windows
3. **Cygwin** - POSIX layer (already supported)

**Package Formats:**
- **Installer:** NSIS, WiX, or Inno Setup
- **NuGet package:** For C/C++ developers
- **Python wheel:** For Python users
- **Vcpkg port:** Modern C++ package manager

**MSVC Build:**
```batch
cmake . -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
cmake --install . --prefix C:\asterix
```

**DLL Considerations:**
- Export symbols properly (`__declspec(dllexport)`)
- Provide `.lib` import library
- Bundle dependencies (expat.dll)

### 3.5 Python Packages

**PyPI (Python Package Index):**
```bash
# Source distribution
python setup.py sdist

# Binary wheel (platform-specific)
python setup.py bdist_wheel

# Upload to PyPI
twine upload dist/*
```

**Platform-Specific Wheels:**
- `asterix-1.0.0-cp38-cp38-manylinux2014_x86_64.whl` (Linux)
- `asterix-1.0.0-cp38-cp38-win_amd64.whl` (Windows)
- `asterix-1.0.0-cp38-cp38-macosx_10_9_x86_64.whl` (macOS)

**GitHub Actions can build wheels for all platforms automatically**

---

## 4. CMake Build System Design

### 4.1 Top-Level CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.12)
project(asterix VERSION 1.0.0 LANGUAGES C CXX)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(BUILD_PYTHON "Build Python bindings" ON)
option(BUILD_CLI "Build command-line tool" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Find dependencies
find_package(EXPAT REQUIRED)
if(BUILD_PYTHON)
    find_package(Python3 COMPONENTS Interpreter Development)
endif()

# Add subdirectories
add_subdirectory(src/engine)
add_subdirectory(src/asterix)
add_subdirectory(src/main)
if(BUILD_PYTHON)
    add_subdirectory(src/python)
endif()
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS asterix_shared asterix_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY asterix/config/
        DESTINATION share/asterix/config)

# Packaging with CPack
include(CPack)
set(CPACK_GENERATOR "DEB;RPM;TGZ;ZIP")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libexpat1")
set(CPACK_RPM_PACKAGE_REQUIRES "expat")
```

### 4.2 Benefits

- **Single build system** for all platforms
- **Automated packaging** with CPack
- **Easy cross-compilation**
- **IDE integration** (Visual Studio, CLion, etc.)
- **Modern C++ standards** support

---

## 5. Distribution Strategy

### 5.1 Package Repositories

**Linux:**
- **Debian:** Publish to PPA (Personal Package Archive)
- **RedHat:** Publish to Copr (Community Projects)
- **Arch:** AUR (Arch User Repository)

**Python:**
- **PyPI:** Main Python package index
- **Conda-forge:** For scientific users

**Windows:**
- **Vcpkg:** Microsoft C++ package manager
- **Conan:** Cross-platform C++ package manager

**Universal:**
- **GitHub Releases:** Source tarballs, pre-built binaries
- **Docker images:** Containerized runtime

### 5.2 Versioning Strategy

Follow **Semantic Versioning (SemVer):**
- `MAJOR.MINOR.PATCH` (e.g., 1.0.0)
- **MAJOR:** Incompatible API changes
- **MINOR:** Backward-compatible features
- **PATCH:** Backward-compatible bug fixes

**Git Tags:**
```bash
git tag -a v1.0.0 -m "Release 1.0.0 - DO-278A AL-3 certified"
git push origin v1.0.0
```

**Python version** in `asterix/version.py`
**C++ version** in `CMakeLists.txt` and `version.h`

---

## 6. DO-278A Compliance for Packaging

### 6.1 Configuration Management

**Baseline Identification:**
- Each release = configuration baseline
- Git tag = version identifier
- Release notes document changes
- Software Configuration Index (SCI) lists all items

**Package Contents Tracking:**
- Manifest file in each package
- MD5/SHA256 checksums
- Digital signatures for release packages

### 6.2 Build Reproducibility

**Requirement:** Identical inputs → identical outputs

**Implementation:**
- Dockerfile for reproducible build environment
- Lock dependency versions
- Record compiler version
- Deterministic build flags

**Verification:**
```bash
# Build twice, compare
sha256sum package1.deb package2.deb
# Should match
```

### 6.3 Traceability

**Each package shall include:**
- Version number
- Git commit hash
- Build timestamp
- Compiler version
- DO-278A compliance statement
- Test results summary

**Package Metadata:**
```
Package: libasterix1
Version: 1.0.0+git06bba98
DO-278A: AL-3 Compliant
Tested: 24/24 Python, 12/12 C++ passed
Coverage: 95%
Build-Date: 2025-10-17T12:00:00Z
```

---

## 7. CI/CD Integration

### 7.1 GitHub Actions Workflow

```yaml
name: Build and Package

on:
  release:
    types: [published]

jobs:
  build-linux-deb:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build DEB package
        run: |
          cmake . -DCPACK_GENERATOR=DEB
          make package
      - name: Upload DEB
        uses: actions/upload-artifact@v3
        with:
          name: asterix-deb
          path: '*.deb'

  build-linux-rpm:
    runs-on: ubuntu-latest
    container: fedora:latest
    steps:
      - name: Build RPM package
        run: |
          cmake . -DCPACK_GENERATOR=RPM
          make package

  build-windows:
    runs-on: windows-latest
    steps:
      - name: Build Windows installer
        run: |
          cmake . -G "Visual Studio 17 2022"
          cmake --build . --config Release
          cpack -G NSIS

  build-python-wheels:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
        python: ['3.8', '3.9', '3.10', '3.11', '3.12']
    steps:
      - name: Build wheel
        run: |
          python setup.py bdist_wheel
      - name: Upload to PyPI
        if: github.event_name == 'release'
        run: |
          twine upload dist/*
```

### 7.2 Automated Testing

**Before packaging:**
- Run full test suite
- Verify coverage targets
- Check for memory leaks
- Validate DO-278A compliance

**Package validation:**
- Install package
- Run smoke tests
- Verify all files present
- Check dependencies

---

## 8. Documentation for Packages

### 8.1 README for Developers

Each package shall include:
- Installation instructions
- Quick start guide
- API reference
- Example code
- Link to full documentation

### 8.2 C/C++ API Documentation

**Doxygen:** Generate API docs from code comments

```bash
doxygen Doxyfile
# Generates: html/ and latex/
```

**Install with package:**
- `/usr/share/doc/asterix/html/index.html`

### 8.3 Python API Documentation

**Sphinx:** Generate docs from docstrings

```bash
cd asterix/docs
make html
# Generates: _build/html/
```

**Publish to:** ReadTheDocs.org

---

## 9. Implementation Plan

### Phase 1: CMake Migration (Weeks 1-2)
- ✅ CMakeLists.txt exists (basic)
- [ ] Enhance CMakeLists.txt for static/shared libs
- [ ] Add install targets
- [ ] Test on Linux, Windows, macOS
- [ ] Integrate with CI

### Phase 2: Linux Packaging (Weeks 3-4)
- [ ] Create .deb packaging
- [ ] Create .rpm packaging
- [ ] Test on Ubuntu, Debian, Fedora, CentOS
- [ ] Publish to test repositories

### Phase 3: Windows Packaging (Week 5)
- [ ] MSVC build support
- [ ] Create installer (NSIS/WiX)
- [ ] NuGet package
- [ ] Test on Windows 10/11

### Phase 4: Python Packaging (Week 6)
- [ ] Build wheels for all platforms
- [ ] Publish to Test PyPI
- [ ] Publish to PyPI
- [ ] Create conda package

### Phase 5: BSD/RTOS (Weeks 7-8)
- [ ] Test on FreeBSD
- [ ] Cross-compile for embedded targets
- [ ] Document platform-specific build

### Phase 6: Distribution (Weeks 9-10)
- [ ] Publish to package repositories
- [ ] Create Docker images
- [ ] Set up automated releases

---

## 10. Quality Metrics

**Package Quality Checklist:**
- [ ] Passes all DO-278A verification tests
- [ ] ≥90% code coverage
- [ ] No memory leaks (Valgrind clean)
- [ ] Static analysis clean
- [ ] Documentation complete
- [ ] Examples work
- [ ] Cross-platform tested
- [ ] Backwards compatible (after 1.0)

---

## Document Approval

| Role | Name | Date | Signature |
|------|------|------|-----------|
| Project Lead | TBD | 2025-10-17 | |
| Release Manager | TBD | | |

---

**Next Steps:**
1. Enhance CMakeLists.txt for multi-platform builds
2. Add shared/static library targets
3. Test packaging on Ubuntu (DEB)
4. Integrate with GitHub Actions CI
