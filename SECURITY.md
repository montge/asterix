# Security Policy

## Reporting a Vulnerability

Please report security issues to damir dot salantic at gmail dot com

## Security Practices

### Multi-Language Security

The ASTERIX project consists of three language components, each with its own security infrastructure:

#### C++ Security
- **Static Analysis**: CodeQL scanning (weekly + on PR)
- **Runtime Analysis**: Valgrind memory leak detection
- **Static Checkers**: cppcheck, clang-tidy
- **Manual Review**: Security-critical code reviewed before merge

#### Python Security
- **Static Analysis**: CodeQL scanning (weekly + on PR)
- **Dependency Monitoring**: Dependabot automatic updates (weekly)
- **Code Quality**: Pre-commit hooks (black, flake8, mypy)
- **Testing**: Unit tests with 97% coverage

#### Rust Security
- **Static Analysis**: CodeQL scanning (weekly + on PR)
- **Vulnerability Scanning**:
  - `cargo-audit` via RustSec Advisory Database
  - `cargo-deny` for advisories, licenses, and supply chain
- **Dependency Monitoring**: Dependabot automatic updates (weekly)
- **Memory Safety**:
  - Compile-time borrow checker
  - Valgrind integration tests
  - Address Sanitizer support
- **Code Quality**:
  - `rustfmt` for consistent formatting
  - `clippy` for linting (deny warnings)
  - Pre-commit hooks for automatic checks
- **Supply Chain Security**:
  - License verification (GPL-3.0 compatible only)
  - Source verification (crates.io only)
  - Duplicate dependency detection

### Automated Security Workflows

1. **CodeQL Analysis** (`.github/workflows/codeql.yml`)
   - Runs weekly on Monday at 3 AM UTC
   - Scans C++, Python, and Rust code
   - Uses `security-and-quality` query suite
   - Results published to GitHub Security tab

2. **Dependabot** (`.github/dependabot.yml`)
   - GitHub Actions: Weekly on Monday
   - Python dependencies: Weekly on Tuesday
   - Rust dependencies: Weekly on Wednesday
   - Automatic PRs for security updates

3. **Rust Security Audit** (in `rust-ci.yml`)
   - `cargo-audit`: Vulnerability scanning
   - `cargo-deny advisories`: Security advisories (fails on issues)
   - `cargo-deny licenses`: License compliance (fails on violations)
   - `cargo-deny bans`: Duplicate/banned dependencies (warns)
   - `cargo-deny sources`: Source verification (warns)

### License Compliance

The project is licensed under **GPL-3.0-or-later**. All dependencies must be compatible:

**Allowed licenses:**
- GPL-3.0, GPL-3.0-or-later
- MIT, Apache-2.0 (with LLVM exception)
- BSD-2-Clause, BSD-3-Clause
- ISC, Zlib, Unicode-DFS-2016
- CC0-1.0, 0BSD

**Rust cargo-deny configuration:** `asterix-rs/cargo-deny.toml`

### Security Update Process

1. **Automated Detection**
   - Dependabot creates PRs for dependency updates
   - CodeQL alerts appear in Security tab
   - cargo-audit/cargo-deny fail CI on critical issues

2. **Review Process**
   - Security PRs prioritized for review
   - Breaking changes tested in isolation
   - Security updates merged within 48 hours when possible

3. **Manual Verification**
   - Critical vulnerabilities verified manually
   - Exploit proof-of-concept tested if available
   - CHANGELOG.md updated with security notes

### Developer Security Guidelines

#### For C++ Contributions
- Run `valgrind` tests before submitting PRs
- Avoid raw pointers; use smart pointers
- Check for buffer overflows in data parsing
- Validate all external input

#### For Python Contributions
- Run pre-commit hooks: `pre-commit run --all-files`
- Keep dependencies minimal and well-maintained
- Use type hints for better static analysis

#### For Rust Contributions
- Run security checks before submitting PRs:
  ```bash
  cd asterix-rs
  cargo fmt --check
  cargo clippy --all-features -- -D warnings
  cargo deny check
  cargo audit
  ```
- Avoid `unsafe` code unless absolutely necessary
- Document all `unsafe` blocks with safety justification
- Use `#![forbid(unsafe_code)]` in new modules when possible
- Prefer owned types over raw pointers in FFI boundaries

### Known Security Considerations

1. **XML Parsing**: Uses libexpat for ASTERIX category definitions
   - Keep libexpat updated to latest version
   - Category XML files should be from trusted sources only

2. **Network Input**: Multicast/UDP data parsing
   - All network data treated as untrusted
   - Bounds checking on all buffer operations
   - Timeout protection on network operations

3. **PCAP Files**: Binary file parsing
   - PCAP files from untrusted sources may trigger parser bugs
   - Use latest version for security fixes
   - Report crashes or hangs as potential security issues

4. **FFI Boundaries** (Rust ↔ C++)
   - CXX bridge provides memory safety guarantees
   - All pointers validated before dereferencing
   - Lifetimes enforced at compile time

### Security Testing

Run the full security test suite:

```bash
# C++ memory leak tests
cd install/test
./valgrind_test.sh

# Python security checks
python -m pip install safety
safety check

# Rust security audit
cd asterix-rs
cargo audit
cargo deny check
cargo test

# CodeQL local scan (requires CodeQL CLI)
codeql database create codeql-db --language=cpp,python,rust
codeql database analyze codeql-db --format=sarif-latest --output=results.sarif
```

## Supported Versions

| Component | Version | Supported |
|-----------|---------|-----------|
| C++ Core  | Latest  | ✅ Yes    |
| Python    | 3.10+   | ✅ Yes    |
| Python    | 3.8-3.9 | ⚠️  Best effort |
| Rust      | Latest  | ✅ Yes    |
| Rust      | MSRV 1.70 | ✅ Yes  |

Security updates are provided for all supported versions.
