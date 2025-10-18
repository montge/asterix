# Test Infrastructure

This directory contains the test infrastructure for DO-278A compliance.

## Directory Structure

```
tests/
├── cpp/                    C++ unit tests (Google Test or Catch2)
├── python/                 Python unit tests (pytest)
├── integration/            Integration test scripts
└── README.md              This file
```

## Existing Tests

The project already has tests in:
- `install/test/test.sh` - Integration tests for C++ executable
- `asterix/test/` - Python unit tests

## New Test Infrastructure

### C++ Unit Tests (`tests/cpp/`)

**Framework:** Google Test (recommended) or Catch2

**Setup:**
```bash
# Install Google Test
sudo apt-get install libgtest-dev
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib
```

**Example test structure:**
```cpp
// tests/cpp/test_dataitem.cpp
#include <gtest/gtest.h>
#include "DataItemFormatFixed.h"

TEST(DataItemFormatFixed, ParseValidData) {
    // Test parsing fixed format data item
    // ...
}

TEST(DataItemFormatFixed, ParseInvalidData) {
    // Test error handling
    // ...
}
```

**Build and run:**
```bash
cd tests/cpp
cmake .
make
./run_tests
```

### Python Unit Tests (`tests/python/`)

**Framework:** pytest with coverage

**Setup:**
```bash
pip install pytest pytest-cov
```

**Example test structure:**
```python
# tests/python/test_parse.py
import pytest
import asterix

def test_parse_cat048():
    """Test parsing Category 048 data"""
    data = bytearray([...])
    result = asterix.parse(data)
    assert result[0]['category'] == 48
    assert 'I048/010' in result[0]
```

**Run:**
```bash
cd tests/python
pytest --cov=asterix --cov-report=html --cov-report=term
```

### Integration Tests (`tests/integration/`)

**Framework:** Bash scripts, Python scripts

**Example:**
```bash
#!/bin/bash
# tests/integration/test_cli_formats.sh
# Test that all output formats work

./install/asterix -P -f sample.pcap -j > output.json
./install/asterix -P -f sample.pcap -x > output.xml
./install/asterix -P -f sample.pcap -l > output.txt

# Validate outputs
python validate_json.py output.json
python validate_xml.py output.xml
```

## Coverage Targets

**DO-278A AL-3 Requirements:**
- Overall: **90-95%**
- Per module: **≥80%**

**Module-specific targets:**
- `src/asterix/*.cpp`: ≥90% (critical)
- `src/engine/*.cpp`: ≥85%
- `src/main/*.cpp`: ≥80%
- `src/python/*.c`: ≥85%
- Python module: ≥90%

## Running All Tests

```bash
# C++ tests
cd tests/cpp && ./run_tests

# Python tests
pytest tests/python/ --cov

# Integration tests
cd install/test && ./test.sh

# All tests via CI
git push  # Triggers GitHub Actions
```

## Test-Driven Development Workflow

1. **Write requirement** - Document in `do-278/requirements/`
2. **Write test** - Create failing test
3. **Implement** - Write code to pass test
4. **Refactor** - Improve code while keeping tests green
5. **Measure coverage** - Ensure targets met
6. **Document** - Update traceability

## Coverage Measurement

### C++ Coverage

```bash
# Build with coverage flags
cd src
make clean
CXXFLAGS="--coverage" make
make install

# Run tests
cd ../install/test
./test.sh

# Generate report
cd ../../
lcov --capture --directory src --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

### Python Coverage

```bash
# Run with coverage
pytest --cov=asterix --cov-report=html --cov-report=xml

# View report
open htmlcov/index.html
```

### Combined Coverage

CI will combine coverage from all sources and upload to Codecov.

## Adding New Tests

### For a new ASTERIX category:

1. Create requirement: `REQ-HLR-XXX: Parse Category XXX`
2. Create test data: `install/sample_data/cat_XXX.pcap`
3. Create expected output: `install/sample_output/cat_XXX.json`
4. Add test case: `tests/python/test_cat_XXX.py`
5. Update RTM: Link requirement → test
6. Run and verify coverage

### For a new feature:

1. Document requirement
2. Write failing test
3. Implement feature
4. Ensure test passes
5. Check coverage increased
6. Update documentation

## CI Integration

Tests run automatically on:
- Every push
- Every pull request
- Nightly (full test suite)

See `.github/workflows/ci-verification.yml`

## Quality Gates

PR merge requires:
- [ ] All tests pass
- [ ] Coverage targets met (or justified)
- [ ] No new memory leaks (Valgrind clean)
- [ ] Static analysis clean (or justified)
- [ ] Code reviewed

## Resources

- Google Test: https://github.com/google/googletest
- pytest: https://pytest.org
- Coverage.py: https://coverage.readthedocs.io
- lcov: http://ltp.sourceforge.net/coverage/lcov.php
