
# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is a decoder/parser for EUROCONTROL ASTERIX protocol - an ATM (Air Traffic Management) Surveillance Data Binary Messaging Format. The project provides three language bindings:
1. **C++ standalone executable** - High-performance command-line tool (C++23/C23)
2. **Python module** - Simple, intuitive API for Python developers (3.10-3.14)
3. **Rust crate** - Type-safe, memory-safe bindings with zero-copy performance (1.70+)

All three bindings share the same C++ core for parsing ASTERIX data from various sources (files, stdin, network multicast streams).

## Development Guidelines

### Issue-First Workflow

**IMPORTANT:** This project uses GitHub Issues for tracking all development work. Before starting any significant changes:

1. **Check existing issues** - Review open issues to avoid duplicates
2. **Create an issue** - Document the problem, scope, and acceptance criteria
3. **Reference in commits** - Use `Relates to #<issue>` in commit messages
4. **Keep issues open** - Issues remain open until ALL CI/CD pipelines pass across all platforms
5. **Close verification** - Only close issues when all acceptance criteria are met and verified in CI

**⚠️ IMPORTANT: Set Your Fork as Default Repository**

If you've forked this repository, configure `gh` CLI to default to YOUR fork, not upstream:

```bash
# Set default to your fork (replace YOUR_USERNAME with your GitHub username)
gh repo set-default YOUR_USERNAME/asterix

# Verify the default is set correctly
gh repo set-default --view

# Example: If you forked to johndoe/asterix
gh repo set-default johndoe/asterix
```

**Why this matters:** Without setting the default, `gh issue create` and other commands will create issues in the upstream repository (`montge/asterix` or `CroatiaControlLtd/asterix`) instead of your fork. This causes confusion and pollutes the upstream issue tracker with your development work.

**When to use upstream:** Only create issues in upstream when reporting bugs or requesting features that affect all users, not for your personal development tasks.

**Issue Creation Guidelines:**
- **Enhancement**: New features, language bindings, protocol integrations
- **Bug**: Defects, incorrect behavior, crashes
- **Documentation**: README updates, API docs, guides
- **Security**: Vulnerabilities, safety-critical concerns

**Using `gh` CLI:**
```bash
# Create an issue
gh issue create --repo montge/asterix --title "Add feature X" --label enhancement

# List issues
gh issue list --repo montge/asterix

# View issue details
gh issue view 21 --repo montge/asterix
```

**Commit Message Format:**
```
Brief description of change

Relates to #<issue-number>

- Detailed point 1
- Detailed point 2
- Testing performed
```

**Example:**
```bash
# 1. Create issue first
gh issue create --title "Add Node.js bindings" --label enhancement

# 2. Implement feature
git checkout -b feature/nodejs-bindings

# 3. Commit with reference
git commit -m "Add N-API wrapper for Node.js bindings

Relates to #24

- Implement core parsing API
- Add TypeScript definitions
- Include unit tests with >80% coverage"

# 4. Issue stays open until CI/CD passes on all platforms
# 5. Close only when all acceptance criteria verified
```

### Local Development Directory

Use the `.local/` directory (hidden) for development files that should never be committed to git:
- **Testing scripts** - PowerShell, Bash, Python scripts for local testing
- **Build experiments** - Custom build configurations and outputs
- **Draft documentation** - Work-in-progress markdown files
- **Personal notes** - Development notes, TODO lists, debugging logs
- **Temporary data** - Test data, output files, logs
- **AI-generated content** - Code analysis reports, security findings, performance metrics
- **Coverage reports** - HTML/JSON coverage outputs, benchmark results
- **Experimental work** - Prototypes, temporary scripts, visualizations

The `.local/` directory is hidden and ignored by git (except for `.local/README.md`).

**Example:**
```bash
# Create your local testing script
cat > .local/my_test.sh <<'EOF'
#!/bin/bash
./install/asterix -f .local/test_data.pcap -j
EOF
chmod +x .local/my_test.sh

# Run it (won't be committed to git)
./.local/my_test.sh

# Store AI-generated analysis in .local/
# Example: Claude Code analysis reports, security findings, etc.
```

**AI-Assisted Development:**
When using Claude Code or other AI tools, store all generated reports and analysis in `.local/`:
- Code quality reports → `.local/analysis/`
- Security findings → `.local/security/`
- Performance benchmarks → `.local/benchmarks/`
- Coverage reports → `.local/coverage/`

The `.claude/` directory is also gitignored for Claude Code-specific configuration.

### Repository Hygiene - No Hardcoded Paths

**CRITICAL:** Never commit hardcoded personal directory paths to git.

**❌ BAD Examples:**
```bash
# In documentation or scripts
cd C:\Users\username\Documents\Development\asterix
cd /home/username/projects/asterix

# In code or config files
ASTERIX_PATH = "/Users/username/asterix"
```

**✅ GOOD Examples:**
```bash
# Use relative paths
cd <path-to-asterix-repo>
cd $(git rev-parse --show-toplevel)

# Use placeholders in documentation
ASTERIX_PATH = "<path-to-asterix-repo>"

# Use environment variables in code
ASTERIX_PATH = os.getenv("ASTERIX_PATH", ".")
```

**Before committing:**
1. Search for your username: `git grep -i "$(whoami)"` or `git grep -i "your-username"`
2. Search for home directory patterns: `git grep -E "(C:\\\\Users|/home/|/Users/)"`
3. Review changes: `git diff` before `git add`
4. Use the `.local/` directory for user-specific files

**Allowed path references:**
- Relative paths (e.g., `./install/asterix`, `../config/`)
- Placeholder paths (e.g., `<path-to-repo>`, `/path/to/asterix`)
- Environment variables (e.g., `$ASTERIX_HOME`, `${PWD}`)
- Example paths that are clearly generic (e.g., `/opt/asterix`, `C:\Program Files\asterix`)
- Public repository URLs (e.g., `github.com/montge/asterix`)

### Code Quality & Testing Standards

**For comprehensive code quality guidelines, see [CONTRIBUTING.md](CONTRIBUTING.md).**

**Quick Reference - Testing Requirements:**
- **C++ Integration Tests**: All changes must pass `install/test/test.sh` (11 test cases)
- **Python Tests**: Run `python -m unittest` from repository root
- **Rust Tests**: Run `cargo test --all-features` from `asterix-rs/`
- **Memory Safety**: Run `install/test/valgrind_test.sh` for memory leak detection (must show 0 leaks)
- **Coverage Target**: Aim for >80% test coverage on new code (current: 92.2%)

**Quality Checks:**
```bash
# C++ - Build and test
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build
cd install/test && ./test.sh

# Python - Run tests
python -m unittest

# Rust - Comprehensive testing
cd asterix-rs
cargo test --all-features  # Unit + integration tests
cargo clippy               # Linter (strict mode)
cargo fmt --check          # Format verification
cargo bench                # Performance benchmarks

# Memory leak testing (requires valgrind)
cd install/test && ./valgrind_test.sh
```

**Pre-Commit Best Practices:**
1. Run relevant test suites before committing
2. Check for memory leaks on FFI boundary changes
3. Verify no hardcoded paths: `git grep -E "(C:\\\\Users|/home/|/Users/)"`
4. Review diff: `git diff` before staging
5. Keep commits atomic and well-described

**Dependency Updates:**
- Dependabot PRs (GitHub Actions, dependencies) should be reviewed and merged regularly
- Check CI/CD status before merging: all platforms must pass
- Use `gh pr list --repo montge/asterix` to view open PRs
- Merge dependency updates to keep security posture strong

## Build System & Commands

### C++ Executable

**Requirements:**
- **C++ compiler** (platform-specific):
  - GCC 13.0+ (recommended for full C++23 support on Linux)
  - Clang 16.0+ (recommended for full C++23 support on Linux)
  - MSVC 2019 v16.0+ or later (Visual Studio 2019/2022, uses C++20 - MSVC doesn't fully support C++23 yet)
  - AppleClang 15.0+ (Xcode 15 or later, uses C++23)
- libexpat-devel (for XML parsing)
- CMake 3.20+

> **Note:** GNU Make build files were removed in v2.8.10. CMake is now the only supported build system.

**Using CMake:**
```bash
# Configure and build (out-of-source build)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Install to install/ directory
cmake --install build

# Debug build
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel

# The executable will be in install/bin/asterix
```

**Why CMake:**
- ✅ Cross-platform (Linux, Windows, macOS)
- ✅ Out-of-source builds (keeps source tree clean)
- ✅ Used by Python module (setup.py) and Rust crate (build.rs)
- ✅ Better dependency management and feature detection
- ✅ Modern IDE integration (VS Code, CLion, Visual Studio)

**C++ Standard (Platform-Specific):**
- **C++ Executable & CMake builds:** 
  - **Linux/macOS:** C++23 (set in CMakeLists.txt via `CMAKE_CXX_STANDARD 23`)
  - **Windows/MSVC:** C++20 (set in CMakeLists.txt via `CMAKE_CXX_STANDARD 20` - MSVC doesn't fully support C++23 yet)
  - Automatic fallback to C++17/20 on older compilers (see `src/asterix/cxx23_features.h`)
- **Python module builds:**
  - **Linux:** C++23 (matches CMake for full feature parity)
  - **Windows:** C++20 (matches CMake for MSVC compatibility)
  - **macOS:** C++17 (better compiler compatibility, set in setup.py)
**C Standard:** The project uses C23 on Linux/macOS (set in CMakeLists.txt via `CMAKE_C_STANDARD 23`) and C17 on Windows/MSVC (MSVC doesn't fully support C23 yet)

### Python Module

```bash
# Build
python setup.py build

# Install locally
python setup.py install

# Install from PyPI
pip install asterix_decoder

# Run tests
python -m unittest
cd install/test && ./test.sh   # Integration tests
```

### Rust Bindings

**Requirements:**
- Rust 1.70+ (2021 edition)
- C++17 compatible compiler (same as C++ executable)
- libexpat-devel (for XML parsing)
- Cargo build system

**Using Cargo:**
```bash
# Build from asterix-rs/ directory
cd asterix-rs
cargo build              # Debug build
cargo build --release    # Production build
cargo test               # Run tests
cargo test --all-features # Run all tests
cargo bench              # Run benchmarks

# Install from crates.io
cargo add asterix-decoder
```

**Directory Structure:**
```
asterix-rs/
├── Cargo.toml           # Rust package manifest
├── build.rs             # Build script (compiles C++ via CMake)
├── README.md            # Rust-specific README
├── src/
│   ├── lib.rs          # Public API
│   ├── ffi.rs          # CXX bridge (unsafe FFI)
│   ├── parser.rs       # Safe Rust API
│   ├── data_types.rs   # AsterixRecord, DataItem types
│   ├── error.rs        # AsterixError enum
│   └── config.rs       # Configuration management
├── tests/
│   └── integration_tests.rs  # Integration tests
├── examples/
│   ├── parse_raw.rs    # Parse raw bytes
│   ├── parse_file.rs   # Parse from file
│   ├── parse_pcap.rs   # Parse PCAP with incremental parsing
│   ├── incremental_parsing.rs
│   ├── json_export.rs
│   ├── describe_category.rs
│   └── stream_processing.rs
└── benches/
    └── benchmarks.rs   # Performance benchmarks
```

**FFI Architecture:**
- Uses CXX crate for safe C++/Rust interop
- Build script (build.rs) invokes CMake to compile C++ core
- Type-safe FFI bridge eliminates manual unsafe code
- Memory ownership: Data copied to Rust side (owned Vec<AsterixRecord>)

**Testing:**
```bash
cd asterix-rs
cargo test              # Unit + integration tests
cargo test --doc        # Documentation tests
cargo bench             # Performance benchmarks
cargo clippy            # Linter
cargo fmt --check       # Format check
```

### Running Tests

```bash
# C++ integration tests
cd install/test
./test.sh

# Python tests
python -m unittest

# Rust tests
cd asterix-rs
cargo test --all-features
cargo bench

# Memory leak tests (requires valgrind)
cd install/test
./valgrind_test.sh
```

## Architecture

### Multi-Layer Design

The codebase has three distinct layers that work together:

1. **Engine Layer** (`src/engine/`) - Generic data processing framework
   - Device abstraction (stdin, file, TCP, UDP, multicast, serial)
   - Format/codec framework
   - Channel management and routing
   - Factory patterns for devices and formats

2. **ASTERIX Layer** (`src/asterix/`) - Protocol-specific implementation
   - XML-based configuration parser for ASTERIX categories
   - Data parsing for various ASTERIX item formats (Fixed, Variable, Compound, Repetitive, Explicit, BDS)
   - Format submodules for encapsulation protocols (PCAP, HDLC, FINAL, GPS)
   - Output formatters (text, JSON, XML)

3. **Application Layer** (`src/main/`) - CLI and language bindings
   - Command-line interface (`asterix.cpp`)
   - Python C extension wrapper (`src/python/`)
   - Rust bindings via CXX FFI (`asterix-rs/`)

### Key Components

**ASTERIX Parsing Pipeline:**
```
Input Source → Format Parser → Data Block → Data Record → Data Items → Output Formatter
```

- `AsterixDefinition`: Manages XML category definitions
- `Category`: Represents ASTERIX category with UAP (User Application Profile)
- `DataBlock`: ASTERIX data block container
- `DataRecord`: Single ASTERIX record
- `DataItem`: Individual data field with format-specific parsing
- `DataItemFormat*`: Polymorphic format handlers (Fixed/Variable/Compound/etc.)
- `XMLParser`: Loads ASTERIX category definitions from XML

**Input Format Handlers:**
- `asterixrawsubformat`: Raw ASTERIX binary
- `asterixpcapsubformat`: PCAP encapsulated data
- `asterixhdlcsubformat`: HDLC framed data
- `asterixfinalsubformat`: FINAL packet format
- `asterixgpssubformat`: GPS packet format

### Configuration System

ASTERIX categories are defined in XML files following the DTD specification at `asterix/config/asterix.dtd`. Configuration files are listed in `asterix.ini`.

- Category definitions: `asterix/config/asterix_cat*_*.xml` (e.g., `asterix_cat062_1_18.xml`)
- BDS definitions: `asterix/config/asterix_bds.xml`
- Each category file defines data items, formats, UAPs, and value meanings

### Python Module Design

The Python module (`asterix/`) wraps the C++ parser via a C extension (`_asterix`):

- C extension built from `src/python/*.c` and core ASTERIX C++ files
- Python wrapper in `asterix/__init__.py` provides high-level API
- Auto-initializes all XML configurations on import
- Returns parsed data as nested dictionaries/lists

**Key Python APIs:**
- `parse(data)` - Parse raw bytes
- `parse_with_offset(data, offset, blocks_count)` - Incremental parsing
- `describe(category, item, field, value)` - Get descriptions
- `init(filename)` - Load custom category definition

### Rust Module Design

The Rust module (`asterix-rs/`) wraps the C++ parser via the CXX crate:

- FFI bridge built using CXX crate for type-safe C++/Rust interop
- Build script (build.rs) compiles C++ core via CMake
- Rust wrapper in `src/lib.rs` provides safe, idiomatic API
- Auto-initializes XML configurations via `init_default()`
- Returns parsed data as owned Rust types (Vec<AsterixRecord>)

**Key Rust APIs:**
- `parse(data, options)` - Parse raw bytes with options
- `parse_with_offset(data, offset, count, options)` - Incremental parsing
- `describe(category, item, field, value)` - Get descriptions
- `init_default()` - Initialize with default categories
- `Parser::new().add_category(path).build()` - Custom categories

## Common Development Tasks

### Adding/Updating ASTERIX Categories

Categories should be updated via the upstream [asterix-specs](https://github.com/zoranbosnjak/asterix-specs) project, then converted:

```bash
cd asterix-specs-converter

# Convert single category from JSON to XML
curl https://zoranbosnjak.github.io/asterix-specs/specs/cat062/cats/cat1.18/definition.json | \
  python3 asterixjson2xml.py > specs/asterix_cat062_1_18.xml

# Update all specs automatically
python3 update-specs.py
```

Then copy to `asterix/config/` (the build system automatically copies to `install/share/asterix/config/`).

### C++ Code Organization

**Language Standard:** C++23 (upgraded from C++17)

The codebase uses modern C++23 features including:
- Deduced this (explicit object parameters) - for better polymorphic performance
- Ranges algorithms - for cleaner container operations
- `std::format` - for type-safe formatting (when available)
- `std::string_view` - for efficient string handling (C++17 feature, retained)
- `std::optional` - for optional values (C++17 feature, retained)
- Structured bindings (C++17 feature, retained)
- `if constexpr` - for compile-time conditionals (C++17 feature, retained)
- Feature detection via `src/asterix/cxx23_features.h` - graceful fallback to C++17/20

**Backward Compatibility:**
The codebase includes feature detection macros in `src/asterix/cxx23_features.h` that allow building with older C++17/20 compilers when C++23 features are not available. Performance optimizations from C++23 features (deduced this, ranges) will be automatically disabled when using older compilers.

**Code Structure:**
- Header/implementation pairs in `src/asterix/`
- Pure virtual base classes for extensibility (`DataItemFormat`, `BaseDevice`, `BaseFormat`)
- Factory patterns in engine layer (`DeviceFactory`, `ChannelFactory`)
- Singleton pattern for global state (`Tracer`, `AsterixDefinition`)

### Output Formats

Controlled via CLI flags:
- Default: Human-readable text
- `-l, --line`: One line per item (parsable)
- `-x, --xml`: Compact XML (one object per line)
- `-xh, --xmlh`: Human-readable XML
- `-j, --json`: Compact JSON (one object per line)
- `-jh, --jsonh`: Human-readable JSON
- `-je, --json-extensive`: Extensive JSON with descriptions

### Input Sources

- `-f filename`: File input (PCAP, raw, FINAL, HDLC, GPS)
- `-i m:i:p[:s]`: Multicast (format: mcast_addr:interface_ip:port[:source])
- stdin: Default if no source specified

Input format flags: `-P` (PCAP), `-R` (ORADIS PCAP), `-O` (ORADIS), `-F` (FINAL), `-H` (HDLC), `-G` (GPS)

## Repository Structure Notes

- `asterix/` - Python module source
- `asterix-rs/` - Rust crate (bindings via CXX)
- `src/` - C++ source code (engine, asterix, main, python bindings)
- `install/` - Build output directory and test suite
- `asterix-specs-converter/` - Tools for converting JSON specs to XML
- Configuration files are in `asterix/config/` (source), copied to `install/share/asterix/config/` during build
- Rust uses config files from `asterix/config/` via FFI during build

## Architecture & Design Guidance

For comprehensive architecture documentation, refer to:

### Core Documentation

- **`docs/ARCHITECTURE.md`** - Complete system architecture
  - Three-layer design (Engine/ASTERIX/Application)
  - Multi-language binding strategy with mermaid diagrams
  - GPL license separation strategies
  - FFI boundary design patterns
  - Safety-critical architecture goals

- **`docs/BINDING_GUIDELINES.md`** - Creating new language bindings
  - Required API surface (parse, init, describe)
  - FFI boundary validation (>80% coverage required)
  - Testing requirements (unit, integration, property-based, fuzzing)
  - Security requirements (CodeQL, ASAN, fuzz testing)
  - CI/CD integration patterns
  - Reference implementations (Python, Rust)

- **`docs/PROTOCOL_INTEGRATION.md`** - Adding protocol adapters
  - Layer decision matrix (where to add code)
  - Input format handlers (PCAP, HDLC, FINAL, GPS examples)
  - Output formatters (JSON, XML, text)
  - Testing requirements (unit, integration, performance, fuzz)
  - Performance best practices (buffer reuse, string reserve)
  - Security considerations (validation, bounds checking, overflow prevention)

### Architecture Quick Reference

**When to use which document:**
- Creating a new language binding (Go, Node.js, Java)? → `BINDING_GUIDELINES.md`
- Adding a new protocol (RTP, WebSocket, gRPC)? → `PROTOCOL_INTEGRATION.md`
- Understanding overall system design? → `ARCHITECTURE.md`
- Commercial/proprietary integration? → `ARCHITECTURE.md` (GPL Separation section)

**Key Architectural Principles:**
1. **Safety First** - Validate all FFI boundaries (no assumptions about callers)
2. **Layer Separation** - Engine (transport) → ASTERIX (protocol) → Application (CLI/bindings)
3. **Idiomatic APIs** - Each binding feels natural to its language
4. **Comprehensive Testing** - >80% coverage, fuzz testing, memory leak testing
5. **Performance** - Buffer reuse, string reserve, zero-copy where possible

## Performance Optimizations Completed

The codebase has been optimized through 6 Quick Wins achieving **55-61% cumulative speedup** (as of 2025-10-20):

1. **Quick Win #1** (fed87cd): String operator+ elimination - 15% speedup
2. **Quick Win #2** (fed87cd): String reserve() optimization - 15% speedup
3. **Quick Win #3** (7feea81): Hex string loop reserve() - 8% speedup
4. **Quick Win #5** (af6ef19): PCAP buffer reuse - 15-20% speedup
5. **Quick Win #6** (cc856f3): UDP multicast fd_set caching - 2-3% speedup

**Key Files Modified:**
- `src/asterix/DataItemFormatVariable.cpp` - String concatenation optimization
- `src/asterix/Utils.cpp` - Hex string reserve()
- `src/asterix/asterixpcapsubformat.cpp` - Buffer reuse
- `src/engine/UdpDevice.cpp` - fd_set template caching

**Testing:** All optimizations verified with 11/11 integration tests passing and 0 memory leaks (valgrind).

**⚠️ WARNING:** Do NOT optimize FSPEC parsing (`DataRecord::parse()` FSPEC loop) - causes memory corruption and segfaults.

See `docs/PERFORMANCE_OPTIMIZATIONS.md` for detailed analysis and measurements.

## Documentation Standards

**IMPORTANT:** All code must be documented using language-appropriate self-documenting formats.

### C++ Documentation (Doxygen)

**Required for all public APIs:**
- Class declarations must have `/** @class */` comments
- Functions must document `@param`, `@return`, `@throws`
- Include usage examples for complex APIs with `@code` blocks
- Use `@brief` for short descriptions

**Current Status:** C++ headers lack comprehensive Doxygen documentation. See [GitHub Issue #72](https://github.com/montge/asterix/issues/72) for improvement tasks.

**Example:**
```cpp
/**
 * @class DataItem
 * @brief Represents a single ASTERIX data item
 *
 * DataItem encapsulates a single data field within an ASTERIX record,
 * including its description, binary data, and parsed values.
 *
 * @note The caller is responsible for memory management
 */
class DataItem {
public:
    /**
     * @brief Parse binary data according to item format
     * @param pData Pointer to binary data buffer
     * @param nLength Length of data buffer in bytes
     * @return true if parsing successful, false otherwise
     * @throws std::invalid_argument if pData is null
     */
    bool parse(const unsigned char* pData, size_t nLength);
};
```

### Python Documentation (Docstrings)

**Required for all public APIs:**
- Module-level docstrings
- Class docstrings with `Attributes:` section
- Function/method docstrings with `Args:`, `Returns:`, `Raises:`, `Example:`
- Use Google-style docstrings (consistent with existing code)

**Current Status:** Partial docstring coverage (~70% estimated). See [GitHub Issue #72](https://github.com/montge/asterix/issues/72) for improvement tasks.

**Example:**
```python
"""Module for parsing ASTERIX data blocks.

This module provides functionality for parsing ASTERIX data blocks
from various input sources including files, stdin, and network streams.
"""

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
    return {}
```

### Rust Documentation (Doc Comments)

**Required for all public APIs:**
- Crate-level documentation with `//!` (module docs)
- Public items documented with `///` (doc comments)
- Include safety warnings, examples, and panic conditions
- Use `# Safety`, `# Examples`, `# Errors`, `# Panics` sections

**Current Status:** Excellent documentation in `asterix-rs/` crate (exemplary standard).

**Example:**
```rust
//! Rust bindings for the ASTERIX ATM surveillance protocol decoder
//!
//! # Examples
//!
//! ```no_run
//! use asterix::{parse, ParseOptions, init_default};
//!
//! init_default()?;
//! let data = std::fs::read("sample.asterix")?;
//! let records = parse(&data, ParseOptions::default())?;
//! ```

/// Parse ASTERIX data from raw bytes
///
/// # Arguments
///
/// * `data` - Raw ASTERIX binary data
/// * `options` - Parsing configuration options
///
/// # Errors
///
/// Returns `AsterixError` if parsing fails due to invalid data
///
/// # Examples
///
/// ```
/// let records = parse(&data, ParseOptions::default())?;
/// ```
pub fn parse(data: &[u8], options: ParseOptions) -> Result<Vec<AsterixRecord>, AsterixError> {
    // Implementation
}
```

### Documentation Best Practices

**When adding new code:**
1. Write documentation BEFORE or alongside the code (TDD for docs)
2. Include examples for non-trivial functions
3. Document edge cases, error conditions, and safety concerns
4. Update docs when changing function signatures or behavior

**When using AI tools:**
- AI-generated documentation must be reviewed for accuracy
- Verify examples compile and run correctly
- Check that safety warnings are appropriate for the code
- Ensure consistency with existing documentation style

**Generating documentation:**
```bash
# C++ - Generate Doxygen documentation
doxygen Doxyfile
# View: docs/cpp/html/index.html

# Python - Generate Sphinx documentation (if configured)
cd docs && make html

# Rust - Generate rustdoc documentation
cd asterix-rs && cargo doc --open
```

**See [CONTRIBUTING.md](CONTRIBUTING.md) for comprehensive documentation requirements and examples.**

## AI-Assisted Development Best Practices

When working with Claude Code or other AI development tools on this codebase:

### 1. Use `.local/` for All AI-Generated Artifacts

**Store analysis outputs in `.local/`:**
- Code analysis reports → `.local/analysis/`
- Security findings → `.local/security/`
- Performance benchmarks → `.local/benchmarks/`
- Coverage reports → `.local/coverage/`
- Experimental prototypes → `.local/experiments/`
- Testing logs → `.local/logs/`

**Never commit AI-generated temporary files:**
- Analysis reports with patterns: `*_REPORT.md`, `*_ANALYSIS.md`, `*_FINDINGS.md`
- Agent outputs: `*AGENT*.md`, `*PHASE*.md`
- Work products: `*_SUMMARY.md`, `*_CHECKLIST.md`

These patterns are already gitignored (see `.gitignore` lines 136-147).

### 2. Maintain Test Coverage

**When AI suggests code changes:**
- Run full test suite before and after changes
- Verify 0 memory leaks with valgrind
- Check FFI boundaries with sanitizers (ASAN/MSAN/UBSAN)
- Maintain >80% coverage on modified components
- Add new tests for new functionality

**Test execution workflow:**
```bash
# 1. Baseline - Run tests before changes
cd install/test && ./test.sh        # C++ integration tests
python -m unittest                   # Python tests
cd asterix-rs && cargo test          # Rust tests

# 2. Make AI-suggested changes

# 3. Verification - Run tests after changes
cd install/test && ./test.sh
cd install/test && ./valgrind_test.sh  # Memory leak check
python -m unittest
cd asterix-rs && cargo test --all-features
```

### 3. Follow Pre-Commit Standards

**Before committing AI-generated code:**
1. **Review the diff carefully** - AI can introduce subtle bugs
2. **Check for hardcoded paths** - Run: `git grep -E "(C:\\\\Users|/home/|/Users/)"`
3. **Verify no secrets** - No API keys, tokens, credentials
4. **Run code formatters** - C++: clang-format, Rust: `cargo fmt`, Python: black/ruff
5. **Test on all platforms** - Linux, Windows (MSVC), macOS (if available)
6. **Document decisions** - Add comments explaining non-obvious changes

### 4. Document AI-Generated Analysis

**When AI produces useful insights:**
- Store full reports in `.local/analysis/` for your reference
- Extract key decisions and add to code comments or commit messages
- Do NOT commit raw AI analysis - synthesize and summarize
- Reference issue numbers in commits: `Relates to #<issue>`

**Example:**
```bash
# AI generates analysis report
mv code_analysis.md .local/analysis/$(date +%Y%m%d)_code_analysis.md

# Extract key decisions for commit message
git commit -m "Optimize PCAP buffer reuse

Relates to #42

- Reuse 65KB buffer across parse iterations
- Reduces allocations by 95% in multicast scenarios
- Verified with valgrind: 0 memory leaks
- Based on analysis in .local/analysis/20251110_code_analysis.md"
```

### 5. Preserve Git History & Attribution

**Commit message quality:**
- Use issue-first workflow (see "Issue-First Workflow" section)
- Write clear, descriptive commit messages
- Include testing verification in commit body
- Maintain clean git history (avoid "fix typo" commits)

**When AI suggests large refactorings:**
- Break into multiple atomic commits
- Each commit should pass all tests
- Document reasoning in commit messages
- Create GitHub issue first for tracking

### 6. Safety-Critical Considerations

**This is a safety-critical aviation project (ATM/ATC data):**
- AI-generated code requires extra scrutiny
- Never skip memory leak testing on FFI changes
- Validate all boundary conditions and edge cases
- Fuzz test parser changes (especially ASTERIX category updates)
- Security scan with CodeQL on significant changes

**Verification checklist for AI changes:**
- [ ] All tests pass (C++, Python, Rust)
- [ ] Valgrind shows 0 memory leaks
- [ ] No new compiler warnings
- [ ] FFI boundaries validated with sanitizers
- [ ] Performance impact measured (if applicable)
- [ ] Security implications reviewed
- [ ] Cross-platform compatibility verified

### 7. Handling Dependency Updates

**Current open PRs (as of 2025-11-10):**
- 7 Dependabot PRs for GitHub Actions updates (see `gh pr list --repo montge/asterix`)

**When AI or Dependabot suggests updates:**
1. Review the changelog/release notes
2. Check for breaking changes
3. Verify CI/CD passes on all platforms
4. Merge promptly to maintain security posture
5. Monitor for CI failures after merge

**Quick review workflow:**
```bash
# List open dependency PRs
gh pr list --repo montge/asterix --label dependencies

# View specific PR details
gh pr view <pr-number> --repo montge/asterix

# Check CI status
gh pr checks <pr-number> --repo montge/asterix

# Merge when CI passes (after review)
gh pr merge <pr-number> --repo montge/asterix --squash
```
