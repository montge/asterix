# Project Context

## Purpose

ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is a decoder/parser for EUROCONTROL ASTERIX protocol - an ATM (Air Traffic Management) Surveillance Data Binary Messaging Format used globally by air traffic control systems.

## Tech Stack

- **C++23/C17**: Core parsing engine (high-performance)
- **Python 3.10-3.14**: Python bindings module (`asterix_decoder` on PyPI)
- **Rust 1.70+**: Rust bindings crate (`asterix-decoder` on crates.io) - includes CAN, CCSDS, DDS, D-Bus, Zenoh transport modules
- **Node.js 20+**: Node.js bindings via N-API (`asterix-node/`)
- **CMake 3.20+**: Build system
- **libexpat**: XML parsing for ASTERIX category definitions

## Project Conventions

### Code Style

- **C++**: C++23 standard (C++20 on MSVC), feature detection via `cxx23_features.h`
- **Python**: Google-style docstrings, type hints for public APIs
- **Rust**: rustfmt defaults, comprehensive doc comments with examples
- No trailing whitespace, UTF-8 encoding

### Architecture Patterns

Three-layer design:
1. **Engine Layer** (`src/engine/`): Device abstraction (file, TCP, UDP, multicast, serial)
2. **ASTERIX Layer** (`src/asterix/`): Protocol-specific parsing, XML configuration
3. **Application Layer** (`src/main/`, `asterix/`, `asterix-rs/`): CLI and language bindings

### Testing Strategy

- **C++ Integration Tests**: `install/test/test.sh` (11 test cases), plus Google Test unit tests in `tests/cpp/`
- **Python Tests**: `python -m unittest` (927+ tests)
- **Rust Tests**: `cargo test --all-features` (coverage ~76.2% via tarpaulin)
- **Node.js Tests**: `npm test` from `asterix-node/`
- **Memory Safety**: Valgrind leak testing required for FFI changes
- **Coverage Target**: >80% for new code, 90% overall target (current overall: 21.4%)

### Known CI Issues (2026-02-07)

- **Rust stable CI broken on master**: `test_ccsds.rs` out of sync with CCSDS API (19 compilation errors). Blocks all Dependabot PR merges.
- **DO-278 CI**: lcov `exclude` pattern `*/test/*` unused error (strict mode)
- **14 Dependabot PRs pending**: 2 security (tar, rsa), 2 security-adjacent (rustix, rustls-pki-types), 10 routine cargo bumps

### Git Workflow

- Issue-first workflow: Create GitHub issue before significant work
- Commit messages: "Brief description\n\nRelates to #<issue>\n\n- Details"
- PRs stay open until ALL CI passes across all platforms
- Dependabot PRs reviewed and merged within 2 weeks

## Domain Context

**ASTERIX Protocol**:
- Binary messaging format for ATM surveillance data
- Categories define data structures (e.g., CAT048=radar, CAT062=multi-sensor fusion)
- XML configuration files define field layouts and meanings
- Safety-critical: Used in real air traffic control systems

**Key Terminology**:
- **UAP**: User Application Profile (field ordering)
- **FSPEC**: Field Specification (bitmask indicating present fields)
- **Data Block**: Container for multiple records of same category
- **Data Item**: Individual field with format-specific parsing

## Important Constraints

- **GPL v3 License**: All contributions must be GPL v3 compatible
- **Safety-Critical**: Code quality and testing standards are strict
- **Cross-Platform**: Must build on Linux, Windows (MSVC), macOS
- **Backward Compatibility**: Python/Rust API changes need deprecation path

## External Dependencies

- **SonarCloud**: Static code analysis and quality gates
- **Codecov**: Code coverage reporting
- **GitHub Actions**: CI/CD for all platforms
- **PyPI**: Python package distribution
- **crates.io**: Rust crate distribution
- **EUROCONTROL asterix-specs**: Upstream category definitions
