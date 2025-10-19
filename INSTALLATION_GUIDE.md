# Development Tools Installation Guide
## ASTERIX Decoder - WSL/Ubuntu Setup

**Date:** 2025-10-18
**Platform:** WSL2 Ubuntu on Windows
**Purpose:** Install tools for DO-278A testing and coverage analysis

---

## System-Level Tools (Require sudo)

### 1. Install C++17 Compatible Compiler

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential
```

**Verify C++17 Support:**
```bash
g++ --version
# Minimum: GCC 7.0 (Ubuntu 18.04+)
# Recommended: GCC 9.0+ (Ubuntu 20.04+)
```

**Compiler Version Requirements:**
- **GCC**: 7.0+ (recommended: 9.0+)
- **Clang**: 5.0+ (recommended: 9.0+)
- **MSVC**: 2017 15.3+ (Visual Studio 2017+)
- **AppleClang**: 9.1+ (Xcode 9.3+)

**If you have an older compiler on Ubuntu 18.04:**
```bash
# Install GCC 9 for better C++17 support
sudo apt-get install -y gcc-9 g++-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90
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
- CMake 3.12+ is required for building
- libexpat is required for XML parsing

**Verification:**
```bash
cmake --version
# Expected: cmake version 3.12 or higher
```

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
| **lcov** | C++ coverage HTML reports | `sudo apt-get install lcov` | Coverage visualization |
| **valgrind** | Memory leak detection | `sudo apt-get install valgrind` | Memory testing |
| **gcov** | C++ coverage data | Included with GCC | Coverage measurement |
| **pytest** | Python testing | `pip install pytest` | Python tests |
| **pytest-cov** | Python coverage | `pip install pytest-cov` | Python coverage |
| **black** | Python formatter | `pip install black` | Code formatting |
| **pylint** | Python linter | `pip install pylint` | Code quality |
| **flake8** | Python style checker | `pip install flake8` | Code style |
| **cmake** | Build system | `sudo apt-get install cmake` | Building |
| **Google Test** | C++ testing | Auto-downloaded by CMake | C++ tests |

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
