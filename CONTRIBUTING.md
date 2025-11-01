# Contributing to ASTERIX Decoder

Thank you for your interest in contributing to the ASTERIX Decoder project! This document provides guidelines and instructions for contributing to this project. We welcome contributions from the community, whether you're fixing bugs, adding features, improving documentation, or helping with testing.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How to Contribute](#how-to-contribute)
- [Development Setup](#development-setup)
- [Git Hooks](#git-hooks)
- [Code Style Guidelines](#code-style-guidelines)
- [Testing Requirements](#testing-requirements)
- [Documentation Requirements](#documentation-requirements)
- [Pull Request Process](#pull-request-process)
- [Commit Message Guidelines](#commit-message-guidelines)
- [Getting Help](#getting-help)

## Code of Conduct

### Our Commitment

We are committed to providing a welcoming and inclusive environment for all contributors, regardless of experience level, gender identity, sexual orientation, disability, personal appearance, body size, race, ethnicity, age, religion, or nationality.

### Expected Behavior

- Be respectful and considerate in all communications
- Welcome newcomers and help them get started
- Provide constructive feedback
- Focus on what is best for the project and the community
- Show empathy towards other contributors

### Unacceptable Behavior

- Harassment, trolling, or discriminatory comments
- Personal attacks or insults
- Publishing others' private information without permission
- Any conduct that would be inappropriate in a professional setting

## How to Contribute

We welcome many types of contributions:

### Bug Reports

Found a bug? Please open an issue with:
- A clear, descriptive title
- Steps to reproduce the problem
- Expected vs actual behavior
- Your environment (OS, Python version, etc.)
- Sample ASTERIX data if applicable (anonymized if necessary)

### Feature Requests

Have an idea for a new feature? Open an issue describing:
- The problem your feature would solve
- Your proposed solution
- Any alternatives you've considered
- Whether you're willing to implement it

### Code Contributions

- Bug fixes
- New features
- Performance improvements
- Code refactoring
- Test coverage improvements

### Documentation Improvements

- README updates
- Code comments
- API documentation
- Examples and tutorials
- Translation of documentation

### Other Contributions

- Reviewing pull requests
- Answering questions in issues
- Adding or improving tests
- Updating ASTERIX category definitions

## Development Setup

### Prerequisites

**System Requirements:**
- Linux, macOS, or Windows (with Cygwin)
- Python 3.8 or higher
- **C++17 compatible compiler**:
  - GCC 7.0+ (recommended: GCC 9+)
  - Clang 5.0+ (recommended: Clang 9+)
  - MSVC 2017 15.3+ (Visual Studio 2017+)
  - AppleClang 9.1+ (Xcode 9.3+)
- CMake 3.12 or higher
- Git

**Required Libraries:**
- libexpat-devel (for XML parsing)

### Quick Setup

For detailed setup instructions, see [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md).

#### 1. Clone the Repository

```bash
git clone https://github.com/CroatiaControlLtd/asterix.git
cd asterix
```

#### 2. Install System Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git libexpat-dev
sudo apt-get install -y lcov valgrind  # For testing and coverage
```

**Fedora/RHEL:**
```bash
sudo dnf install -y gcc gcc-c++ make cmake git expat-devel
sudo dnf install -y lcov valgrind
```

**macOS:**
```bash
brew install cmake expat
brew install lcov
```

#### 3. Set Up Python Virtual Environment

```bash
python3 -m venv .venv
source .venv/bin/activate  # On Windows: .venv\Scripts\activate
pip install --upgrade pip setuptools wheel
pip install -r requirements-dev.txt
```

#### 4. Build the Project

**Python Module:**
```bash
python setup.py build
python setup.py install
```

**C++ Executable:**
```bash
cd src
make install
cd ..
```

**Or using CMake:**
```bash
mkdir -p build
cd build
cmake .. -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON
make -j$(nproc)
cd ..
```

#### 5. Verify Installation

```bash
# Run Python tests
pytest asterix/test/ tests/python/ -v

# Run C++ tests
cd build
ctest --output-on-failure
```

#### 6. Set Up Git Hooks (Recommended)

The repository includes automated git hooks to catch issues before they're committed or pushed:

```bash
# Make hooks executable (usually automatic)
chmod +x .git/hooks/pre-commit .git/hooks/pre-push

# Optional: Install pre-commit framework for additional checks
pip install pre-commit
pre-commit install
```

See the [Git Hooks](#git-hooks) section below for detailed information.

## Code Style Guidelines

### Python Code Style

We follow [PEP 8](https://www.python.org/dev/peps/pep-0008/) style guidelines with some modifications:

**Formatting:**
- Use [Black](https://github.com/psf/black) code formatter with 100 character line length
- Run before committing: `black asterix/ tests/ --line-length 100`

**Code Standards:**
- Use meaningful variable and function names
- Type hints are encouraged for function signatures
- Use docstrings for all public functions and classes (Google style)

**Example:**

```python
from typing import List, Dict, Optional


def parse_asterix_record(data: bytes, category: int, verbose: bool = True) -> Dict:
    """Parse an ASTERIX record from raw binary data.

    Args:
        data: Raw ASTERIX binary data as bytes.
        category: ASTERIX category number (e.g., 48, 62).
        verbose: If True, include descriptions in output.

    Returns:
        Dictionary containing parsed ASTERIX fields and values.

    Raises:
        ValueError: If data is invalid or category is unsupported.
        RuntimeError: If internal parsing error occurs.

    Example:
        >>> data = b'\x30\x00\x30...'
        >>> result = parse_asterix_record(data, 48)
        >>> print(result['I010'])
    """
    if not data:
        raise ValueError("Data cannot be empty")

    # Implementation here
    return {}
```

**Linting:**
```bash
# Check code style
flake8 asterix/ tests/ --max-line-length=100

# Run static analysis
pylint asterix/*.py
```

### C++ Code Style

**Language Standard:** C++17

The project requires C++17 and developers can use modern C++17 features including:
- **Structured bindings**: `auto [key, value] = map.insert(...)`
- **std::optional**: For optional return values and nullable types
- **std::string_view**: For efficient string parameter passing
- **if constexpr**: For compile-time conditionals
- **Inline variables**: For header-only constants
- **Nested namespaces**: `namespace A::B::C { }`
- **std::variant**: For type-safe unions
- **std::any**: For type-erased values
- **Fold expressions**: For variadic template expansion

**General Guidelines:**
- Follow the existing code style in the repository
- Use consistent indentation (4 spaces, no tabs)
- Class names use PascalCase: `DataItem`, `AsterixDefinition`
- Function names use camelCase: `parseDataBlock`, `getText`
- Member variables use m_ prefix: `m_pData`, `m_nLength`
- Pointer variables use p prefix: `pDesc`, `m_pDescription`
- Prefer C++17 features over older alternatives when appropriate

**Header Files:**
- Include guards for all header files
- Forward declarations when possible
- Group includes: system headers, then project headers

**Documentation:**
- Use Doxygen-style comments for public APIs
- Document all parameters, return values, and exceptions

**Example:**

```cpp
/**
 * @brief Parses ASTERIX data item from binary stream
 *
 * This function extracts a single data item from the provided binary
 * buffer according to the ASTERIX category specification.
 *
 * @param pData Pointer to binary data buffer
 * @param nLength Length of data buffer in bytes
 * @param category ASTERIX category number
 * @return Pointer to parsed DataItem or nullptr on failure
 *
 * @throws std::invalid_argument if pData is null or nLength is 0
 * @throws std::runtime_error if parsing fails
 *
 * @note The caller is responsible for deleting the returned DataItem
 *
 * Example:
 * @code
 *   unsigned char buffer[] = {0x30, 0x00, 0x30};
 *   DataItem* item = parseDataItem(buffer, sizeof(buffer), 48);
 *   if (item) {
 *       // Use item
 *       delete item;
 *   }
 * @endcode
 */
DataItem* parseDataItem(const unsigned char* pData, size_t nLength, int category);
```

**Memory Management:**
- Use RAII principles
- Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers
- Use `std::optional` instead of nullable pointers when appropriate
- Avoid manual memory management when possible
- Delete allocated memory in destructors (for legacy code)
- Check for null pointers before use

**C++17 Best Practices:**
- Use `std::string_view` for read-only string parameters instead of `const std::string&`
- Use structured bindings for clearer tuple/pair unpacking
- Prefer `if constexpr` over SFINAE for template metaprogramming
- Use inline variables for constants in headers
- Leverage `[[nodiscard]]` attribute for important return values

## Testing Requirements

All contributions must maintain or improve our test coverage, which is currently at 92.2%.

### Python Testing

**Framework:** pytest with coverage plugin

**Running Tests:**
```bash
# Run all tests with coverage
pytest asterix/test/ tests/python/ -v --cov=asterix --cov-report=term-missing --cov-report=html

# Run specific test file
pytest asterix/test/test_parser.py -v

# Run with verbose output
pytest -vv
```

**Writing Tests:**
- Place tests in `asterix/test/` or `tests/python/`
- Use descriptive test function names: `test_parse_cat048_valid_packet()`
- Test both success and failure cases
- Use fixtures for common setup

**Example:**

```python
import pytest
import asterix


def test_parse_valid_asterix_packet():
    """Test parsing of valid CAT048 packet."""
    packet = bytearray([0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19])
    result = asterix.parse(packet)

    assert result is not None
    assert len(result) > 0
    assert result[0]['category'] == 48


def test_parse_empty_packet_raises_error():
    """Test that parsing empty packet raises ValueError."""
    with pytest.raises(ValueError):
        asterix.parse(bytearray())


def test_parse_with_custom_category(tmp_path):
    """Test parsing with custom category definition."""
    config_file = tmp_path / "custom_cat.xml"
    config_file.write_text("<?xml version='1.0'?>...")

    asterix.init(str(config_file))
    packet = bytearray([0x30, 0x00, 0x10])
    result = asterix.parse(packet)

    assert result is not None
```

### C++ Testing

**Framework:** Google Test

**Running Tests:**
```bash
cd build
ctest --output-on-failure -V

# Run specific test
./tests/test_dataitem
```

**Writing Tests:**
- Place tests in appropriate test file or create new one
- Use descriptive test names
- Test edge cases and error conditions

**Example:**

```cpp
#include <gtest/gtest.h>
#include "DataItem.h"

TEST(DataItemTest, ParseValidFixedLengthItem) {
    unsigned char data[] = {0x01, 0x02, 0x03, 0x04};
    DataItemDescription desc("010", "SAC/SIC", 4);

    DataItem item(&desc);
    bool result = item.parse(data, sizeof(data));

    EXPECT_TRUE(result);
    EXPECT_EQ(item.getLength(), 4);
}

TEST(DataItemTest, ParseNullDataReturnsError) {
    DataItemDescription desc("010", "SAC/SIC", 4);
    DataItem item(&desc);

    bool result = item.parse(nullptr, 0);

    EXPECT_FALSE(result);
}

TEST(DataItemTest, ParseVariableLengthItem) {
    unsigned char data[] = {0x80, 0x01, 0x02};  // FX bit set
    DataItemDescription desc("020", "Variable Item", 0);

    DataItem item(&desc);
    bool result = item.parse(data, sizeof(data));

    EXPECT_TRUE(result);
    EXPECT_GT(item.getLength(), 0);
}
```

### Coverage Requirements

- All new code must include tests
- Overall coverage should not decrease (currently 92.2%)
- Aim for 90%+ coverage on new files

**Generate Coverage Reports:**

```bash
# Python coverage
pytest --cov=asterix --cov-report=html
# View: htmlcov/index.html

# C++ coverage
cd build
lcov --capture --directory . -o coverage.info
lcov --remove coverage.info '/usr/*' '*/googletest/*' -o coverage_filtered.info
genhtml coverage_filtered.info -o coverage_html
# View: build/coverage_html/index.html
```

### Memory Leak Testing

Run valgrind on C++ code before submitting:

```bash
cd build
valgrind --leak-check=full --error-exitcode=1 ./bin/asterix -P -d ../config/asterix.ini -f ../sample_data/cat048.pcap
```

## Documentation Requirements

### Python Documentation

**Required:**
- Module-level docstrings
- Class docstrings
- Function/method docstrings for all public APIs
- Use Google-style docstrings

**Example:**

```python
"""Module for parsing ASTERIX data blocks.

This module provides functionality for parsing ASTERIX data blocks
from various input sources including files, stdin, and network streams.
"""


class DataBlock:
    """Represents a single ASTERIX data block.

    A data block is the primary container for ASTERIX records and
    contains category information, timestamp, and one or more records.

    Attributes:
        category: ASTERIX category number (e.g., 48, 62).
        timestamp: Block timestamp as datetime object.
        records: List of DataRecord objects.
    """

    def __init__(self, category: int):
        """Initialize DataBlock with category.

        Args:
            category: ASTERIX category number.
        """
        self.category = category
        self.records = []
```

### C++ Documentation

**Required:**
- Doxygen comments for all public classes, functions, and methods
- Document parameters, return values, exceptions
- Include usage examples for complex APIs

**Example:**

```cpp
/**
 * @class DataItem
 * @brief Represents a single ASTERIX data item
 *
 * DataItem encapsulates a single data field within an ASTERIX record,
 * including its description, binary data, and parsed values.
 */
class DataItem {
public:
    /**
     * @brief Construct DataItem from description
     * @param pDesc Pointer to DataItemDescription (must not be null)
     */
    DataItem(DataItemDescription* pDesc);

    /**
     * @brief Destructor - frees allocated data buffer
     */
    ~DataItem();

    /**
     * @brief Parse binary data according to item format
     * @param pData Pointer to binary data buffer
     * @param nLength Length of data buffer in bytes
     * @return true if parsing successful, false otherwise
     */
    bool parse(const unsigned char* pData, size_t nLength);
};
```

### Update Documentation When:

- Adding new features: Update README.md with examples
- Changing APIs: Update docstrings and comments
- Adding new ASTERIX categories: Update configuration documentation
- Fixing bugs: Add comments explaining the fix

## Pull Request Process

### 1. Fork and Branch

```bash
# Fork repository on GitHub, then clone your fork
git clone https://github.com/YOUR_USERNAME/asterix.git
cd asterix

# Add upstream remote
git remote add upstream https://github.com/CroatiaControlLtd/asterix.git

# Create feature branch
git checkout -b feature/your-feature-name
```

**Branch Naming Convention:**
- `feature/description` - New features
- `fix/description` - Bug fixes
- `docs/description` - Documentation updates
- `test/description` - Test additions/improvements
- `refactor/description` - Code refactoring

### 2. Make Changes

- Write clean, well-documented code
- Follow code style guidelines
- Add tests for new functionality
- Update documentation as needed

### 3. Test Your Changes

```bash
# Format Python code
black asterix/ tests/ --line-length 100

# Run linters
flake8 asterix/ tests/ --max-line-length=100
pylint asterix/*.py

# Run all tests
pytest asterix/test/ tests/python/ -v --cov=asterix

# Run C++ tests
cd build
ctest --output-on-failure

# Check for memory leaks
valgrind --leak-check=full ./bin/asterix -f ../sample_data/cat048.pcap
```

### 4. Commit Changes

Follow [commit message guidelines](#commit-message-guidelines):

```bash
git add .
git commit -m "feat: add support for ASTERIX CAT065"
```

### 5. Push to Your Fork

```bash
git push origin feature/your-feature-name
```

### 6. Submit Pull Request

Open a pull request on GitHub with:

**Title:** Clear, descriptive title (e.g., "Add support for ASTERIX CAT065")

**Description:**

```markdown
## Summary
Brief description of what this PR does.

## Changes
- Added new feature X
- Fixed bug Y
- Updated documentation Z

## Testing
- [ ] All existing tests pass
- [ ] Added new tests for new functionality
- [ ] Manual testing performed: (describe)
- [ ] Coverage maintained/improved

## Related Issues
Closes #123

## Checklist
- [ ] Code follows style guidelines
- [ ] Documentation updated
- [ ] Tests added/updated
- [ ] Coverage not decreased
- [ ] No memory leaks (C++ changes)
```

### 7. Code Review Process

- Maintainers will review your PR
- Address feedback by pushing new commits
- Once approved, maintainers will merge

**Review Timeline:**
- Initial response: Within 1 week
- Follow-up reviews: Within 3-5 days

## Git Hooks

The repository includes automated git hooks that catch issues before code is committed or pushed. These hooks are implemented as shell scripts in `.git/hooks/` and can be optionally enhanced with the pre-commit framework.

### Hook Types

#### Pre-Commit Hook (`.git/hooks/pre-commit`)

Runs automatically before each `git commit`. Designed to be fast (<5 seconds) to not disrupt workflow. Checks include:

**Code Formatting:**
- Python: [black](https://github.com/psf/black) formatter check
- C/C++: [clang-format](https://clang.llvm.org/docs/ClangFormat/) formatter check

**Linting:**
- Python: [flake8](https://flake8.pycqa.org/) linting
- C/C++: [cpplint](https://github.com/google/styleguide/tree/gh-pages/cpplint) linting (warnings only)

**Security Checks:**
- Detects hardcoded passwords, API keys, tokens, and private keys
- Identifies common security anti-patterns
- Prevents accidental secret commits

**File Checks:**
- No files larger than 1 MB
- No trailing whitespace
- No merge conflict markers
- No debug code (console.log, debugger, print_debug, etc.)

**Installation:**

```bash
# Hooks are automatically set up, but ensure they're executable:
chmod +x .git/hooks/pre-commit .git/hooks/pre-push
```

**Usage:**

```bash
# Run normally - hook executes automatically
git commit -m "fix: update parser logic"

# Bypass checks if absolutely necessary (not recommended)
git commit --no-verify -m "WIP: experimental changes"
```

#### Pre-Push Hook (`.git/hooks/pre-push`)

Runs automatically before `git push`. More comprehensive than pre-commit, can take up to 60 seconds. Ensures code quality before pushing to the repository.

**Build Verification:**
- Full C++ production build: `make clean && make`
- Full C++ debug build: `make debug`
- Ensures code compiles without errors

**Unit Tests:**
- C++ Google Test suite
- Python pytest suite
- All unit tests must pass

**Integration Tests:**
- Run `install/test/test.sh` (11 integration tests)
- Validates end-to-end functionality

**Static Analysis:**
- [cppcheck](http://cppcheck.sourceforge.net/) for C/C++ (informational)
- [mypy](http://mypy-lang.org/) for Python type checking (if type hints exist)
- Reports issues without blocking push

**Optional Checks:**
- Memory leak detection with [valgrind](https://valgrind.org/) (requires valgrind installed)
- Coverage verification (warns if below 80%)

**Usage:**

```bash
# Run normally - hook executes automatically
git push origin feature/my-feature

# Bypass checks if absolutely necessary (not recommended)
git push --no-verify origin feature/my-feature
```

### Optional: Pre-Commit Framework

In addition to the shell-based hooks, the repository includes configuration for the [pre-commit.com](https://pre-commit.com/) framework, which provides additional automated checks.

**Installation:**

```bash
# Install pre-commit
pip install pre-commit

# Set up framework hooks
pre-commit install

# Install all hook dependencies
pre-commit install --install-hooks
```

**Manual Run:**

```bash
# Run all checks on all files
pre-commit run --all-files

# Run specific hook
pre-commit run black --all-files

# Update hook versions
pre-commit autoupdate
```

**Configuration:**

See `.pre-commit-config.yaml` for the list of enabled hooks and their versions. Hooks include:
- Python formatting (black)
- Python import sorting (isort)
- Python linting (flake8)
- Python type checking (mypy)
- File checks (whitespace, conflicts, large files, etc.)
- C/C++ formatting (clang-format)
- Bash linting (shellcheck)
- YAML/Markdown linting

### Hook Requirements

**For Pre-Commit Hook to work fully, install:**

```bash
# Ubuntu/Debian
sudo apt-get install clang-format clang-tools

# macOS
brew install clang-format

# Python tools
pip install black flake8 cpplint
```

**For Pre-Push Hook to work fully, install:**

```bash
# Ubuntu/Debian
sudo apt-get install valgrind cppcheck

# macOS
brew install valgrind cppcheck

# C++ testing (usually already installed)
# Google Test, CMake

# Python tools
pip install pytest pytest-cov coverage mypy
```

### Troubleshooting

**Hook not executing:**

```bash
# Check hooks are executable
ls -la .git/hooks/pre-commit .git/hooks/pre-push

# Make executable if needed
chmod +x .git/hooks/pre-*
```

**Formatting issues:**

```bash
# Auto-format Python code
black asterix/ tests/ --line-length 100

# Auto-format C++ code
find . -name '*.cpp' -o -name '*.h' | xargs clang-format -i
```

**Test failures:**

```bash
# Run failing test locally
cd src
make clean && make test

# Or for Python
pytest asterix/test/ -v
```

**Skipping hooks temporarily:**

```bash
# Skip pre-commit hook only
git commit --no-verify -m "message"

# Skip pre-push hook only
git push --no-verify origin branch

# Skip both hooks (not recommended)
git commit --no-verify -m "message"
git push --no-verify origin branch
```

**Note:** Skipping hooks is strongly discouraged. They exist to maintain code quality and prevent issues from reaching the repository.

## Commit Message Guidelines

We use [Conventional Commits](https://www.conventionalcommits.org/) format:

### Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `test`: Adding or updating tests
- `refactor`: Code refactoring (no functional changes)
- `perf`: Performance improvements
- `style`: Code style changes (formatting, no code changes)
- `build`: Build system or dependency changes
- `ci`: CI/CD configuration changes
- `chore`: Other changes (maintenance tasks)

### Examples

**Good:**

```
feat(parser): add support for ASTERIX CAT065

Implemented parsing for ASTERIX Category 065 (SDPS Service Status Messages).
Added XML configuration file and updated parser to handle new data formats.

Closes #145
```

```
fix(dataitem): prevent buffer overflow in variable length parsing

Fixed potential buffer overflow when parsing variable length data items
with malformed FX bits. Added bounds checking and error handling.

Fixes #234
```

```
docs(readme): add installation instructions for macOS

Added detailed installation steps for macOS including Homebrew
dependencies and troubleshooting common issues.
```

```
test(python): increase coverage for parse_with_offset function

Added edge case tests for offset parsing including zero offset,
large offsets, and boundary conditions.
```

**Bad:**

```
fixed stuff          # Too vague, missing type
update code          # Not descriptive
WIP                  # Work in progress, don't commit
bug fix              # Missing conventional commit format
```

### Commit Best Practices

- Keep commits focused on single logical change
- Write clear, descriptive commit messages
- Reference issue numbers when applicable
- Don't commit commented-out code
- Don't commit temporary/debug files

## Getting Help

We're here to help you contribute successfully!

### Resources

- **Documentation:** [README.md](README.md), [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md)
- **ASTERIX Protocol:** [EUROCONTROL ASTERIX](http://www.eurocontrol.int/services/asterix)
- **Issue Tracker:** [GitHub Issues](https://github.com/CroatiaControlLtd/asterix/issues)
- **Pull Requests:** [GitHub PRs](https://github.com/CroatiaControlLtd/asterix/pulls)

### Contact

- **Project Maintainer:** Damir Salantic
  - LinkedIn: [damirsalantic](https://hr.linkedin.com/in/damirsalantic)

- **GitHub Discussions:** For general questions and discussions

- **Issues:** For bug reports and feature requests

### Common Questions

**Q: I'm new to ASTERIX. Where should I start?**
A: Start by reading the README and trying out the examples. For contributing, look for issues labeled "good first issue" or "help wanted".

**Q: How long does PR review take?**
A: We aim to provide initial feedback within one week. Complex changes may take longer.

**Q: Can I work on an issue that's already assigned?**
A: Please comment on the issue first to coordinate. We want to avoid duplicate work.

**Q: My tests pass locally but fail in CI. What should I do?**
A: Check the CI logs for details. Common issues include path differences, missing dependencies, or platform-specific code.

**Q: How do I update my PR after feedback?**
A: Simply push new commits to your branch. The PR will update automatically.

## License

By contributing to ASTERIX Decoder, you agree that your contributions will be licensed under the GNU General Public License v3.0. See the [COPYING](COPYING) file for details.

---

**Thank you for contributing to ASTERIX Decoder!**

Your contributions help improve air traffic management systems worldwide. We appreciate your time and effort in making this project better.
