# asterix-rs

[![Rust CI/CD](https://github.com/CroatiaControlLtd/asterix/actions/workflows/rust-ci.yml/badge.svg)](https://github.com/CroatiaControlLtd/asterix/actions/workflows/rust-ci.yml)
[![codecov](https://codecov.io/gh/CroatiaControlLtd/asterix/branch/master/graph/badge.svg?flag=rust)](https://codecov.io/gh/CroatiaControlLtd/asterix)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Rust Version](https://img.shields.io/badge/rust-1.70%2B-blue.svg)](https://www.rust-lang.org)

Rust bindings for the ASTERIX ATM surveillance protocol decoder.

## Overview

`asterix` is a Rust crate providing safe, idiomatic bindings to the C++ ASTERIX decoder library. ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is a binary protocol used for Air Traffic Management (ATM) surveillance data exchange.

This crate enables Rust developers to parse and process ASTERIX data with type safety, memory safety, and zero-copy performance where possible.

## Features

- Type-safe parsing of ASTERIX categories (48, 62, 65, etc.)
- Multiple input formats: raw binary, PCAP, HDLC, FINAL, GPS
- Memory-safe FFI using the `cxx` crate for C++/Rust interop
- Comprehensive error handling with detailed error types
- Optional serde support for JSON serialization
- Incremental parsing for large files and streaming data
- Metadata queries for category/item/field descriptions
- **Transport layers**: Optional Zenoh pub/sub support for distributed systems

## Feature Flags

| Feature | Description | Default |
|---------|-------------|---------|
| `serde` | JSON serialization/deserialization | Enabled |
| `zenoh` | Zenoh pub/sub transport for distributed ASTERIX data | Disabled |

### Zenoh Transport

Enable Zenoh for pub/sub communication across distributed systems:

```toml
[dependencies]
asterix = { version = "0.1", features = ["zenoh"] }
```

```rust
use asterix::transport::zenoh::{ZenohPublisher, ZenohSubscriber, ZenohConfig};

// Publish ASTERIX data
let publisher = ZenohPublisher::new(ZenohConfig::default()).await?;
publisher.publish(&record).await?;

// Subscribe to ASTERIX data
let mut subscriber = ZenohSubscriber::new(ZenohConfig::default(), "asterix/**").await?;
while let Some(sample) = subscriber.recv().await {
    println!("Received CAT{}: {} bytes", sample.category, sample.data.len());
}
```

See [ZENOH_GUIDE.md](../docs/guides/ZENOH_GUIDE.md) for detailed documentation.

## Installation

Add to your `Cargo.toml`:

```toml
[dependencies]
asterix = "0.1.0"
```

### Build Requirements

- Rust: 1.70+ (2021 edition)
- C++ Compiler: GCC 7+, Clang 5+, or MSVC 2017+
- C++17 support required
- libexpat-devel: XML parsing library

## Quick Start

```rust
use asterix::{init_default, parse, ParseOptions};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize ASTERIX with default config
    init_default()?;

    // Read ASTERIX data
    let data = std::fs::read("sample.asterix")?;

    // Parse with default options
    let records = parse(&data, ParseOptions::default())?;

    // Process parsed records
    for record in records {
        println!("Category {}: {} items", record.category, record.items.len());
    }

    Ok(())
}
```

## Quality Standards

This project maintains high quality standards matching the C++ and Python implementations:

### Code Coverage
- **Minimum threshold**: 80% per module
- **Target coverage**: 90% overall
- **Tools**: Both `cargo-tarpaulin` and `cargo-llvm-cov` for comprehensive coverage analysis
- **CI enforcement**: Builds fail if coverage drops below 80%

### Code Quality
- **Formatting**: Enforced via `cargo fmt` (rustfmt)
- **Linting**: Zero warnings policy with `cargo clippy -D warnings`
- **Documentation**: All public APIs documented, warnings treated as errors
- **Security**: Regular audits with `cargo-audit`
- **Dependencies**: Monitored for outdated packages with `cargo-outdated`

### Testing
- Unit tests for all core functionality
- Integration tests with real ASTERIX data
- Property-based testing where applicable
- Memory safety verified with Valgrind
- Cross-platform testing (Linux, macOS, Windows)

### Continuous Integration
All PRs must pass:
- ✅ Test suite (stable and nightly Rust)
- ✅ Coverage checks (80% minimum)
- ✅ Clippy lints (zero warnings)
- ✅ Format verification
- ✅ Documentation build
- ✅ Security audit
- ✅ Memory safety checks

## Documentation

See [RUST_BINDINGS_IMPLEMENTATION_PLAN.md](../RUST_BINDINGS_IMPLEMENTATION_PLAN.md) for complete implementation details.

## License

GNU General Public License v3.0 or later.
