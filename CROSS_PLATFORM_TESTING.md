# Cross-Platform Testing Guide

This document provides comprehensive testing instructions for the ASTERIX decoder across Windows, macOS, and Linux platforms.

## Table of Contents

- [Test Suite Overview](#test-suite-overview)
- [Windows Testing](#windows-testing)
- [macOS Testing](#macos-testing)
- [Linux Testing](#linux-testing)
- [Python Module Testing](#python-module-testing)
- [Continuous Integration](#continuous-integration)
- [Troubleshooting](#troubleshooting)

---

## Test Suite Overview

### Test Categories

1. **Build Tests**: Verify compilation succeeds on all platforms
2. **Unit Tests**: C++ unit tests (planned - using Google Test)
3. **Integration Tests**: End-to-end parsing tests with sample data
4. **Memory Tests**: Leak detection and memory safety (Linux/macOS only)
5. **Python Tests**: Python module functionality and API tests
6. **Performance Tests**: Benchmark parsing speed and resource usage

### Test Data Locations

```
install/
├── sample_data/              # Input test files
│   ├── cat048.raw            # Raw ASTERIX format
│   ├── cat062cat065.raw
│   ├── cat_001_002.pcap      # PCAP encapsulated
│   ├── cat_034_048.pcap
│   ├── cat_062_065.pcap
│   └── parsegps.gps          # GPS format
│
└── sample_output/            # Expected outputs for comparison
    ├── filter.txt
    ├── cat_001_002_json.txt
    ├── cat_001_002.xml
    ├── cat_001_002.txt
    └── ...
```

### Expected Test Results

All platforms should produce **identical output** for the same input data, except:
- Line endings (Windows: `\r\n`, Unix: `\n`)
- File path separators (Windows: `\`, Unix: `/`)
- Floating-point precision differences (minimal)

---

## Windows Testing

### Prerequisites

- Windows 10/11 or Windows Server 2019/2022
- Visual Studio 2019/2022 with C++ desktop development workload
- PowerShell 5.1+ (pre-installed on Windows 10/11)
- Built executable: `install/bin/asterix.exe`

### Method 1: PowerShell Test Script (Automated)

```powershell
# Navigate to test directory
cd install\test

# Run test suite (Release build)
.\test.ps1

# Run test suite (Debug build)
.\test.ps1 -Configuration Debug

# Run with verbose output
.\test.ps1 -Verbose
```

**Expected Output:**
```
========================================
  ASTERIX Decoder Test Suite (Windows)
========================================

Executable:   C:\path\to\install\bin\asterix.exe
Config:       C:\path\to\install\config\asterix.ini
Configuration: Release

========================================
  Basic Execution Tests
========================================

[OK]      Test version output
[OK]      Test help output
[OK]      Test list filters

========================================
  File Parsing Tests
========================================

[OK]      Test PCAP parsing (cat_062_065)
[OK]      Test raw format parsing (cat048)
[OK]      Test GPS parsing

========================================
  TEST SUMMARY
========================================
Total tests:  6
Passed:       6
Failed:       0

✓ All tests PASSED!
```

### Method 2: Manual Testing (Windows Command Prompt)

```batch
cd install\bin

REM Test 1: Help and version
asterix.exe --help
asterix.exe --version

REM Test 2: List available filters
asterix.exe -d ..\config\asterix.ini -L

REM Test 3: Parse PCAP file to JSON
asterix.exe -P -d ..\config\asterix.ini -j -f ..\sample_data\cat_062_065.pcap

REM Test 4: Parse raw ASTERIX to human-readable text
asterix.exe -d ..\config\asterix.ini -f ..\sample_data\cat048.raw

REM Test 5: Parse GPS format
asterix.exe -G -d ..\config\asterix.ini -f ..\sample_data\parsegps.gps
```

### Method 3: Visual Studio Test Explorer (Future)

When C++ unit tests are implemented:

1. Open solution in Visual Studio 2022
2. Build → Build Solution
3. Test → Run All Tests
4. View results in Test Explorer

### Windows-Specific Test Notes

**Network Device Tests**: Currently disabled on Windows (Unix socket API incompatibility)
- TCP device tests: **SKIPPED**
- UDP multicast tests: **SKIPPED**
- These will be enabled once Winsock2 support is implemented

**Memory Leak Tests**: Not available on Windows (no native valgrind equivalent)
- Alternative: Use Application Verifier (requires manual setup)
- Alternative: Dr. Memory (requires separate installation)
- Memory testing is primarily done on Linux in CI/CD

**Path Separator Handling**:
- Tests automatically normalize paths for Windows (`\` vs `/`)
- Config file paths should use forward slashes or escaped backslashes

---

## macOS Testing

### Prerequisites

- macOS 11.0+ (Big Sur or later)
- Xcode 15.0+ with Command Line Tools
- Built executable: `install/bin/asterix`

### Method 1: Shell Script (Automated)

```bash
# Navigate to test directory
cd install/test

# Run test suite
./test.sh

# Check exit code
echo "Exit code: $?"
```

**Expected Output:**
```bash
OK       Test filter
OK       Test json CAT_062_065
OK       Test json CAT_001_002
OK       Test jsonh CAT_001_002
OK       Test xml CAT_001_002
OK       Test txt CAT_001_002
OK       Test line CAT_001_002
OK       Test filtered txt CAT_034_048
OK       Test unfiltered txt CAT_034_048
OK       Test GPS parsing
OK       Test Memory leak with valgrind (1)
OK       Test Memory leak with valgrind (2)
All tests OK
```

### Method 2: Manual Testing (macOS Terminal)

```bash
cd install/bin

# Test 1: Basic functionality
./asterix --help
./asterix --version

# Test 2: PCAP parsing with JSON output
./asterix -P -d ../config/asterix.ini -j -f ../sample_data/cat_062_065.pcap

# Test 3: Human-readable output
./asterix -P -d ../config/asterix.ini -f ../sample_data/cat_034_048.pcap

# Test 4: XML output
./asterix -P -d ../config/asterix.ini -x -f ../sample_data/cat_001_002.pcap

# Test 5: Verify library linkage
otool -L ./asterix
```

### Method 3: Memory Leak Detection (macOS)

**Using valgrind** (if installed via Homebrew):

```bash
# Install valgrind (may have compatibility issues on Apple Silicon)
brew install valgrind

# Run with leak detection
cd install/test
valgrind --leak-check=full --error-exitcode=1 \
  ../../obj/main/debug/asterix -P -d ../config/asterix.ini \
  -f ../sample_data/cat_062_065.pcap -jh
```

**Using Xcode Instruments** (native macOS tool):

```bash
# 1. Build with debug symbols
cd src
make debug
make debug install

# 2. Run with Instruments
instruments -t Leaks -D leaks.trace install/bin/asterix -- -P -d install/config/asterix.ini -f install/sample_data/cat_062_065.pcap

# 3. Analyze results
open leaks.trace
```

### macOS-Specific Test Notes

**Apple Silicon (M1/M2/M3)**:
- Ensure build architecture matches: `arm64`
- Check with: `file install/bin/asterix`
- Should show: `Mach-O 64-bit executable arm64`

**Intel Macs**:
- Architecture: `x86_64`
- Check with: `file install/bin/asterix`
- Should show: `Mach-O 64-bit executable x86_64`

**Universal Binary** (both architectures):
```bash
# Build universal binary
cmake -B build -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build

# Verify
file install/bin/asterix
# Output: Mach-O universal binary with 2 architectures
```

---

## Linux Testing

### Prerequisites

- Linux distribution (Ubuntu 20.04+, Debian 11+, RHEL 8+, Fedora 38+)
- GCC 9+ or Clang 9+ (GCC 13+ recommended for C++23)
- Build tools: make, cmake, pkg-config
- Testing tools: valgrind, lcov (optional)
- Built executable: `install/asterix`

### Method 1: Shell Script (Automated)

```bash
# Navigate to test directory
cd install/test

# Run full test suite
./test.sh

# Check exit code
echo "Exit code: $?"
```

### Method 2: Manual Testing (Linux Terminal)

```bash
cd install

# Test 1: Basic execution
./asterix --help
./asterix --version

# Test 2: Parse PCAP to JSON
./asterix -P -d config/asterix.ini -j -f sample_data/cat_062_065.pcap > output.json

# Test 3: Parse with filters
./asterix -P -d config/asterix.ini -f sample_data/cat_034_048.pcap -LF sample_output/filter.txt

# Test 4: Compare output with expected
diff <(./asterix -P -j -d config/asterix.ini -f sample_data/cat_062_065.pcap) sample_output/cat_062_065_json.txt
```

### Method 3: Memory Leak Detection (Valgrind)

```bash
# Build debug version
cd src
make clean
make debug
make debug install

# Run valgrind tests
cd ../install/test

# Detailed leak check
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --log-file=valgrind.log \
         ../../obj/main/debug/asterix -P -d ../config/asterix.ini \
         -f ../sample_data/cat_062_065.pcap -jh

# Check results
cat valgrind.log | grep "ERROR SUMMARY"
# Expected: ERROR SUMMARY: 0 errors from 0 contexts
```

### Method 4: Code Coverage (lcov/gcov)

```bash
# Build with coverage flags
cd src
make clean
CXXFLAGS="--coverage -g" make
make install

# Run tests
cd ../install/test
./test.sh

# Generate coverage report
lcov --capture --directory ../../src --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage_filtered.info
genhtml coverage_filtered.info --output-directory coverage_html

# View report
firefox coverage_html/index.html
```

### Linux-Specific Test Notes

**Distribution Differences**:
- **Ubuntu/Debian**: Use `apt-get` for dependencies
- **RHEL/Fedora/Rocky**: Use `dnf` or `yum`
- **Arch Linux**: Use `pacman`

**Kernel/Security Features**:
- SELinux (RHEL/Fedora): May affect file access permissions
- AppArmor (Ubuntu): May restrict multicast socket operations
- If tests fail with permission errors, check `dmesg` for SELinux/AppArmor denials

**GCC Version**:
```bash
# Check GCC version
gcc --version

# Ubuntu 22.04: GCC 11 (C++17 support)
# Ubuntu 24.04: GCC 13 (C++23 support)
```

---

## Python Module Testing

### Prerequisites

- Python 3.8+ (3.9+ recommended, 3.13 for latest features)
- pip, setuptools, wheel
- pytest, pytest-cov (for testing)
- Installed Python module: `pip install -e .` or `python setup.py install`

### Method 1: Pytest (Recommended)

```bash
# Install test dependencies
pip install pytest pytest-cov coverage

# Run all tests
pytest asterix/test/ -v

# Run with coverage report
pytest asterix/test/ -v --cov=asterix --cov-report=html --cov-report=term

# View coverage
open htmlcov/index.html  # macOS
xdg-open htmlcov/index.html  # Linux
start htmlcov/index.html  # Windows
```

**Expected Output:**
```
============= test session starts =============
platform linux -- Python 3.11.0, pytest-7.4.0
collected 47 items

asterix/test/test_asterix_data.py::test_category_parsing PASSED     [ 2%]
asterix/test/test_asterix_data.py::test_data_block_parsing PASSED  [ 4%]
asterix/test/test_asterix_data.py::test_data_record_parsing PASSED [ 6%]
...
asterix/test/test_internal_functions.py::test_utils PASSED        [100%]

---------- coverage: platform linux, python 3.11 -----------
Name                              Stmts   Miss  Cover
-----------------------------------------------------
asterix/__init__.py                  45      2    96%
asterix/examples/...                 ...    ...   ...%
-----------------------------------------------------
TOTAL                               892     71    92%

============= 47 passed in 2.34s =============
```

### Method 2: Unittest (Built-in)

```bash
# Run unittest discover
python -m unittest discover -s asterix/test -p 'test_*.py' -v

# Run specific test file
python -m unittest asterix.test.test_asterix_data -v

# Run specific test class
python -m unittest asterix.test.test_asterix_data.TestAsterixData -v

# Run specific test method
python -m unittest asterix.test.test_asterix_data.TestAsterixData.test_category_parsing -v
```

### Method 3: Manual Testing (Interactive)

```python
# Start Python interpreter
python

# Import module
import asterix

# Test 1: Parse raw bytes
data = open('asterix/sample_data/cat048.raw', 'rb').read()
result = asterix.parse(data)
print(result)

# Test 2: Parse with offset
offset = 0
blocks_count = 1
result = asterix.parse_with_offset(data, offset, blocks_count)
print(result)

# Test 3: Get description
desc = asterix.describe(48, '010', '', 'SAC')
print(desc)

# Test 4: Load custom configuration
asterix.init('asterix/config/asterix_cat062_1_18.xml')
```

### Method 4: Example Scripts

```bash
# Run example scripts
cd asterix/examples

# Read raw bytes example
python read_raw_bytes.py

# Multicast receive (syntax check only, requires network setup)
python -m py_compile multicast_receive.py

# Multicast send/receive (syntax check only)
python -m py_compile multicast_send_receive.py
```

### Platform-Specific Python Testing

**Windows**:
```powershell
# Ensure module is built for Windows
python setup.py build_ext --inplace
python -m pytest asterix\test\ -v
```

**macOS**:
```bash
# Ensure correct Python architecture (ARM vs Intel)
python -c "import platform; print(platform.machine())"

# Run tests
python -m pytest asterix/test/ -v
```

**Linux**:
```bash
# Check Python interpreter
which python3
python3 --version

# Run tests with coverage
python3 -m pytest asterix/test/ -v --cov=asterix --cov-report=term-missing
```

---

## Continuous Integration

### GitHub Actions Workflow

The `.github/workflows/cross-platform-builds.yml` workflow runs automated tests on:

- **Windows**: windows-2019, windows-2022 (MSVC)
- **macOS**: macos-13 (Intel x86_64), macos-14 (ARM64)
- **Linux**: ubuntu-22.04, ubuntu-24.04 (GCC)
- **Python**: 3.9, 3.10, 3.11, 3.12, 3.13 (all platforms)

### Triggering CI Tests

```bash
# Push to trigger CI
git add .
git commit -m "Test cross-platform builds"
git push origin feature/cross-platform

# Or manually trigger via GitHub Actions UI:
# 1. Go to Actions tab
# 2. Select "Cross-Platform Builds"
# 3. Click "Run workflow"
```

### Viewing CI Results

1. Navigate to repository on GitHub
2. Click "Actions" tab
3. Select workflow run
4. View job results for each platform
5. Download artifacts (installers, test logs)

### CI Test Matrix

| Platform       | OS Version    | Compiler      | C++ Std | Python Versions        |
|----------------|---------------|---------------|---------|------------------------|
| Windows        | 2019, 2022    | MSVC v142/143 | 17, 23  | 3.9, 3.10, 3.11, 3.12  |
| macOS (Intel)  | 13 (Ventura)  | AppleClang 15 | 23      | 3.9, 3.10, 3.11, 3.12  |
| macOS (ARM)    | 14 (Sonoma)   | AppleClang 15 | 23      | 3.9, 3.10, 3.11, 3.12  |
| Linux (22.04)  | Ubuntu 22.04  | GCC 11        | 17      | 3.9, 3.10, 3.11, 3.12  |
| Linux (24.04)  | Ubuntu 24.04  | GCC 13        | 23      | 3.9, 3.10, 3.11, 3.12, 3.13 |

---

## Troubleshooting

### Windows Issues

**Problem**: `asterix.exe` not found
```powershell
# Check build output directory
dir build\bin\Release
dir build\bin\Debug

# Verify CMake install step completed
cmake --install build --config Release
```

**Problem**: Missing `expat.dll`
```powershell
# Copy DLL to executable directory
copy C:\vcpkg\installed\x64-windows\bin\expat.dll install\bin\
```

**Problem**: Python module import error
```python
# Error: ImportError: DLL load failed
# Solution: Ensure Visual C++ Redistributable is installed
# Download from: https://aka.ms/vs/17/release/vc_redist.x64.exe
```

### macOS Issues

**Problem**: `dyld: Library not loaded: @rpath/libexpat.dylib`
```bash
# Check library linkage
otool -L install/bin/asterix

# Fix: Install expat via Homebrew
brew install expat

# Or: Copy library to @rpath
cp /usr/local/lib/libexpat.dylib install/lib/
```

**Problem**: Python module fails to build on ARM (M1/M2)
```bash
# Ensure correct architecture
arch -arm64 python setup.py build

# Or set environment variable
export ARCHFLAGS="-arch arm64"
python setup.py build
```

### Linux Issues

**Problem**: `error while loading shared libraries: libexpat.so.1`
```bash
# Install expat
sudo apt-get install libexpat1  # Debian/Ubuntu
sudo dnf install expat           # RHEL/Fedora

# Or set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

**Problem**: Valgrind reports false positives
```bash
# Use suppression file
valgrind --suppressions=valgrind.supp --leak-check=full ./asterix ...

# Generate suppression file
valgrind --gen-suppressions=all --leak-check=full ./asterix ... 2>&1 | tee valgrind.log
```

### Python Issues

**Problem**: `ModuleNotFoundError: No module named '_asterix'`
```bash
# Rebuild C extension
python setup.py build_ext --inplace

# Or reinstall
pip uninstall asterix_decoder
pip install -e .
```

**Problem**: Tests fail with "no such file or directory"
```bash
# Ensure sample data is present
ls -la asterix/sample_data/

# Run tests from repository root
cd /path/to/asterix
pytest asterix/test/
```

---

## Performance Benchmarking

### Benchmark Suite (Future)

```bash
# Run performance benchmarks
cd benchmarks
./run_benchmarks.sh

# Compare across platforms
./compare_platforms.sh
```

### Manual Performance Testing

```bash
# Time PCAP parsing
time ./install/asterix -P -d install/config/asterix.ini -j -f large_file.pcap > /dev/null

# Memory usage (Linux)
/usr/bin/time -v ./install/asterix -P -d install/config/asterix.ini -j -f large_file.pcap

# Memory usage (macOS)
/usr/bin/time -l ./install/asterix -P -d install/config/asterix.ini -j -f large_file.pcap
```

---

## Test Coverage Goals

### Current Status

- **C++ Code**: ~85% coverage (via integration tests)
- **Python Code**: ~92% coverage (via pytest)

### Target Goals

- **C++ Code**: 90%+ statement coverage, 80%+ branch coverage
- **Python Code**: 95%+ statement coverage, 85%+ branch coverage
- **Cross-Platform**: All tests pass on Windows, macOS, Linux

### Coverage Reports

Generated by CI/CD and uploaded to Codecov:
- View at: `https://codecov.io/gh/{user}/asterix`
- Badges available for README.md

---

## References

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Pytest Documentation](https://docs.pytest.org/)
- [Valgrind User Manual](https://valgrind.org/docs/manual/manual.html)
- [Xcode Instruments Guide](https://developer.apple.com/documentation/xcode/improving-your-app-s-performance/)
- [Dr. Memory (Windows)](https://drmemory.org/)
