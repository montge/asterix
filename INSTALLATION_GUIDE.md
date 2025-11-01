# Development Tools Installation Guide
## ASTERIX Decoder - Multi-Platform Setup

**Date:** 2025-11-01
**Platforms:** Linux (x86_64, ARM64), Windows, macOS (Intel & ARM M1)
**Purpose:** Install tools for C++, Python, and Rust development, testing, and coverage analysis

---

## System-Level Tools (Require sudo)

### 1. Install C++23 Compatible Compiler

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential
```

**Verify C++23 Support:**
```bash
g++ --version
# Minimum: GCC 7.0 for C++17 (fallback mode)
# Recommended: GCC 13.0+ for full C++23 support
```

**Compiler Version Requirements:**
- **C++23 (Full Support - Recommended):**
  - GCC 13.0+
  - Clang 16.0+
  - MSVC 2022 v17.4+ (Visual Studio 2022 version 17.4+)
  - AppleClang 15.0+ (Xcode 15+)
- **C++17 (Fallback Mode):**
  - GCC 7.0+
  - Clang 5.0+
  - MSVC 2017 15.3+
  - AppleClang 9.1+

**Upgrade to GCC 13 for C++23 (Ubuntu 22.04+):**
```bash
sudo apt-get install -y gcc-13 g++-13
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 130
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 130
```

### 2. Install C++ Coverage Tools

```bash
sudo apt-get update
sudo apt-get install -y lcov
```

**What it does:**
- `lcov`: Graphical front-end for gcov coverage data
- `genhtml`: Generates HTML coverage reports (included with lcov)
- Used for: Creating detailed C++ code coverage reports

**Verification:**
```bash
lcov --version
# Expected: lcov: LCOV version 2.x
```

### 3. Install Memory Analysis Tools

```bash
sudo apt-get install -y valgrind
```

**What it does:**
- `valgrind`: Memory leak and profiling tool
- Used for: Detecting memory leaks, buffer overflows, use-after-free errors

**Verification:**
```bash
valgrind --version
# Expected: valgrind-3.x
```

### 4. Install Build Tools (if missing)

```bash
sudo apt-get install -y cmake git libexpat-dev
```

**What it does:**
- Ensures CMake, Git, and libexpat are available
- CMake 3.20+ is required for building C++23 projects
- libexpat is required for XML parsing

**Verification:**
```bash
cmake --version
# Expected: cmake version 3.20 or higher
```

### 5. Install Rust (for Rust bindings)

**Linux/macOS:**
```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

**Windows:**
Download and run rustup-init.exe from https://rustup.rs/

**Verification:**
```bash
rustc --version
# Expected: rustc 1.70.0 or higher

cargo --version
# Expected: cargo 1.70.0 or higher
```

**What it does:**
- Installs Rust compiler, Cargo package manager, and standard library
- Required for building and using Rust bindings
- Minimum version: 1.70+ (Rust 2021 edition)

---

## Python Virtual Environment Setup

### 1. Create Virtual Environment

```bash
# From the /home/e/Development/asterix directory
python3 -m venv .venv
```

### 2. Activate Virtual Environment

```bash
source .venv/bin/activate
```

You should see `(.venv)` prefix in your shell prompt.

### 3. Upgrade pip

```bash
pip install --upgrade pip setuptools wheel
```

### 4. Install Development Dependencies

```bash
# Install all development tools
pip install -r requirements-dev.txt
```

This installs:
- pytest (testing framework)
- pytest-cov (coverage plugin)
- coverage (coverage reporting)
- black (code formatter)
- pylint (linter)
- flake8 (style checker)
- memory-profiler (memory analysis)
- lxml (XML support)

### 5. Build and Install ASTERIX Decoder

```bash
# Build the C++ extension
python setup.py build

# Install in development mode
python setup.py install
```

### 6. Verify Installation

```bash
# Run Python tests
pytest asterix/test/ tests/python/ -v --cov=asterix

# Should see: 60 tests passing, 88% coverage
```

---

## Rust Build and Testing Setup

### 1. Build Rust Bindings

```bash
cd asterix-rs
cargo build --release
```

### 2. Run Rust Tests

```bash
cargo test --all-features
# Should see: All tests passing
```

### 3. Run Rust Benchmarks

```bash
cargo bench
# Generates benchmark reports in target/criterion/
```

### 4. Install Rust Crate Locally

```bash
# From asterix-rs/ directory
cargo install --path .
```

### 5. Add to Your Rust Project

```bash
# Add to Cargo.toml dependencies
cargo add asterix-decoder

# Or manually:
[dependencies]
asterix-decoder = "0.1.0"
```

---

## C++ Build and Testing Setup

### 1. Create Build Directory

```bash
mkdir -p build
cd build
```

### 2. Configure with CMake (Coverage Enabled)

```bash
cmake .. \
  -DCMAKE_CXX_STANDARD=17 \
  -DBUILD_TESTING=ON \
  -DENABLE_COVERAGE=ON \
  -DBUILD_SHARED_LIBS=ON \
  -DBUILD_STATIC_LIBS=ON
```

**Note:** C++17 is set by default in CMakeLists.txt, but you can explicitly specify it with `-DCMAKE_CXX_STANDARD=17`.

### 3. Build the Project

```bash
make -j$(nproc)
```

### 4. Run C++ Tests

```bash
ctest --output-on-failure -V
# Should see: 33 tests passing (100%)
```

### 5. Generate C++ Coverage Report

```bash
# Capture coverage data
lcov --capture --directory . --output-file coverage.info

# Remove system/external library coverage
lcov --remove coverage.info '/usr/*' '*/googletest/*' '*/test/*' --output-file coverage_filtered.info

# Generate HTML report
genhtml coverage_filtered.info --output-directory coverage_html

# View report
# Open coverage_html/index.html in browser
```

---

## Quick Start Script

Save this as `setup_dev_env.sh` and run `bash setup_dev_env.sh`:

```bash
#!/bin/bash
set -e

echo "=== Installing system packages (requires sudo) ==="
sudo apt-get update
sudo apt-get install -y lcov valgrind build-essential cmake git

echo ""
echo "=== Setting up Python virtual environment ==="
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip setuptools wheel
pip install -r requirements-dev.txt

echo ""
echo "=== Building ASTERIX decoder ==="
python setup.py build
python setup.py install

echo ""
echo "=== Building C++ with tests and coverage ==="
mkdir -p build
cd build
cmake .. -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=ON
make -j$(nproc)

echo ""
echo "=== Running tests ==="
ctest --output-on-failure
cd ..
source .venv/bin/activate
pytest asterix/test/ tests/python/ -v --cov=asterix

echo ""
echo "=== Generating C++ coverage report ==="
cd build
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/googletest/*' '*/test/*' --output-file coverage_filtered.info
genhtml coverage_filtered.info --output-directory coverage_html

echo ""
echo "✅ Setup complete!"
echo ""
echo "Coverage reports:"
echo "  - Python: htmlcov/index.html"
echo "  - C++: build/coverage_html/index.html"
echo ""
echo "To activate venv in future sessions:"
echo "  source .venv/bin/activate"
```

Make it executable:
```bash
chmod +x setup_dev_env.sh
```

---

## Tool Usage Reference

### Running Tests

```bash
# Python tests with coverage
source .venv/bin/activate
pytest asterix/test/ tests/python/ -v --cov=asterix --cov-report=term-missing --cov-report=html

# C++ tests
cd build
ctest -V

# All tests
pytest asterix/test/ tests/python/ -v && cd build && ctest -V
```

### Code Formatting

```bash
# Format Python code with Black
source .venv/bin/activate
black asterix/ tests/python/ --line-length 100

# Check formatting without changes
black asterix/ tests/python/ --check --diff
```

### Code Quality Checks

```bash
# Run pylint
source .venv/bin/activate
pylint asterix/*.py

# Run flake8
flake8 asterix/ tests/python/ --max-line-length=100
```

### Memory Leak Testing

```bash
# Test C++ executable with valgrind
cd build
valgrind --leak-check=full --error-exitcode=1 ./bin/asterix -P -d ../config/asterix.ini -f ../sample_data/cat048.pcap
```

### Coverage Analysis

```bash
# Python coverage report (terminal)
pytest --cov=asterix --cov-report=term-missing

# Python coverage report (HTML)
pytest --cov=asterix --cov-report=html
# Open htmlcov/index.html

# C++ coverage report
cd build
lcov --capture --directory . -o coverage.info
lcov --remove coverage.info '/usr/*' '*/googletest/*' -o coverage_filtered.info
genhtml coverage_filtered.info -o coverage_html
# Open build/coverage_html/index.html
```

---

## Troubleshooting

### Issue: "pytest: command not found"

**Solution:** Activate virtual environment
```bash
source .venv/bin/activate
```

### Issue: "lcov: command not found"

**Solution:** Install lcov system-wide
```bash
sudo apt-get install -y lcov
```

### Issue: "No module named '_asterix'"

**Solution:** Rebuild and install Python extension
```bash
source .venv/bin/activate
python setup.py build
python setup.py install
```

### Issue: "CTest not found" or tests don't run

**Solution:** Rebuild with testing enabled
```bash
cd build
cmake .. -DBUILD_TESTING=ON
make -j$(nproc)
ctest -V
```

### Issue: Coverage shows 0%

**Solution:** Rebuild with coverage enabled and run tests
```bash
cd build
cmake .. -DENABLE_COVERAGE=ON -DBUILD_TESTING=ON
make clean
make -j$(nproc)
ctest
lcov --capture --directory . -o coverage.info
```

---

## Summary of Tools

| Tool | Purpose | Installation | Required For |
|------|---------|--------------|--------------|
| **GCC 13+** | C++23 compiler | `sudo apt-get install gcc-13 g++-13` | C++ builds (C++23) |
| **Rust 1.70+** | Rust compiler & Cargo | `curl https://sh.rustup.rs \| sh` | Rust bindings |
| **lcov** | C++ coverage HTML reports | `sudo apt-get install lcov` | Coverage visualization |
| **valgrind** | Memory leak detection | `sudo apt-get install valgrind` | Memory testing |
| **gcov** | C++ coverage data | Included with GCC | Coverage measurement |
| **pytest** | Python testing | `pip install pytest` | Python tests |
| **pytest-cov** | Python coverage | `pip install pytest-cov` | Python coverage |
| **black** | Python formatter | `pip install black` | Code formatting |
| **pylint** | Python linter | `pip install pylint` | Code quality |
| **flake8** | Python style checker | `pip install flake8` | Code style |
| **cmake** | Build system | `sudo apt-get install cmake` | Building (all) |
| **Google Test** | C++ testing | Auto-downloaded by CMake | C++ tests |
| **cargo-clippy** | Rust linter | Included with Rust | Rust code quality |
| **cargo-bench** | Rust benchmarking | Included with Cargo | Rust performance |

---

## Next Steps After Installation

1. ✅ Verify all tools are installed
2. ✅ Run full test suite (Python + C++)
3. ✅ Generate coverage reports (Python + C++)
4. ✅ Review coverage gaps
5. ⏳ Add tests to reach 90% coverage
6. ⏳ Continue with Task #8 (RPM packages)

---

*Installation Guide Version: 1.0*
*Last Updated: 2025-10-18*
*For: ASTERIX Decoder DO-278A AL-3 Compliance*
