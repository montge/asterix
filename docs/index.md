---
layout: default
title: ASTERIX Decoder Documentation
---

# ASTERIX Decoder Documentation

[![C++ Docs](https://img.shields.io/badge/docs-C%2B%2B%20API-blue)](cpp/html/index.html)
[![Python Docs](https://img.shields.io/badge/docs-Python-blue)](https://asterix.readthedocs.io/)
[![Rust Docs](https://img.shields.io/badge/docs-Rust-blue)](https://docs.rs/asterix-decoder)
[![GitHub](https://img.shields.io/badge/source-GitHub-black)](https://github.com/montge/asterix)

ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is a decoder/parser for the EUROCONTROL ASTERIX protocol - an ATM (Air Traffic Management) Surveillance Data Binary Messaging Format.

---

## Choose Your Language

### 🔧 C++
High-performance command-line tool for processing ASTERIX data.

- **[C++ API Reference](cpp/html/index.html)** - Complete API documentation (Doxygen)
- **[Build Guide](BUILD.html)** - Building from source
- **[GitHub Repository](https://github.com/montge/asterix)** - Source code

**Quick Start:**
```bash
# Ubuntu/Debian
sudo apt-get install asterix
asterix -f sample.pcap -j
```

---

### 🐍 Python
Simple, intuitive API for Python developers (3.10-3.14).

- **[Python Documentation](https://asterix.readthedocs.io/)** - User guide and API reference (ReadTheDocs)
- **[PyPI Package](https://pypi.org/project/asterix-decoder/)** - Install with pip
- **[Build Guide](BUILD.html#python-module)** - Building Python module

**Quick Start:**
```python
pip install asterix-decoder

import asterix
records = asterix.parse(raw_bytes)
```

---

### 🦀 Rust
Type-safe, memory-safe bindings with zero-copy performance (Rust 1.70+).

- **[Rust Documentation](https://docs.rs/asterix-decoder)** - API reference (docs.rs)
- **[crates.io](https://crates.io/crates/asterix-decoder)** - Install with cargo
- **[Build Guide](BUILD.html#rust-crate)** - Building Rust crate

**Quick Start:**
```bash
cargo add asterix-decoder
```

```rust
use asterix::Parser;

let parser = Parser::new().build();
let records = parser.parse(&data)?;
```

---

## Documentation

### Getting Started
- **[Build & Installation Guide](BUILD.html)** - Multi-platform build instructions
- **[Quick Start Examples](https://github.com/montge/asterix#quick-start)** - Get running in 5 minutes
- **[Language Comparison](LANGUAGE_COMPARISON.html)** - Feature/performance comparison

### User Guides
- **[ASTERIX Categories](CATEGORIES.html)** - Supported categories and versions
- **[Configuration](CONFIGURATION.html)** - XML category definitions
- **[Performance Guide](PERFORMANCE.html)** - Optimization and benchmarking

### Technical Documentation
- **[Architecture](ARCHITECTURE.html)** - System design and components
- **[Security](../SECURITY.md)** - Security policy and audit
- **[Performance Optimizations](../PERFORMANCE_OPTIMIZATIONS.md)** - 55-61% speedup details

### Development
- **[Contributing Guide](../CONTRIBUTING.md)** - How to contribute
- **[Testing Guide](TESTING.html)** - Running tests and coverage
- **[DO-278 Compliance](../do-278/README.md)** - Aviation safety certification

---

## Features

### Multi-Language Support
- **C++ Executable** - Command-line tool (C++23/C23)
- **Python Module** - Python 3.10-3.14 bindings
- **Rust Crate** - Safe, zero-copy bindings

### Multi-Platform
- Linux (Ubuntu, Debian, RHEL, Fedora)
- Windows (10/11, Server 2019/2022)
- macOS (Intel & Apple Silicon)

### Input Formats
- Raw ASTERIX binary
- PCAP captures (Wireshark)
- FINAL packet format
- HDLC framed data
- GPS packet format
- UDP multicast streams

### Output Formats
- Human-readable text
- JSON (compact & pretty)
- XML (compact & pretty)
- Parsable line format

### Performance
- **55-61% faster** than baseline (optimized C++ core)
- Zero-copy operations where possible
- Efficient memory management
- Streaming support for large files

---

## Quick Links

- [GitHub Repository](https://github.com/montge/asterix)
- [Issue Tracker](https://github.com/montge/asterix/issues)
- [Release Notes](https://github.com/montge/asterix/releases)
- [License (GPL-3.0)](https://github.com/montge/asterix/blob/master/LICENSE)

---

## Support

- **Documentation Issues:** [File a bug](https://github.com/montge/asterix/issues)
- **Questions:** [GitHub Discussions](https://github.com/montge/asterix/discussions)
- **Security:** See [SECURITY.md](../SECURITY.md)

---

<small>
Last updated: 2025-11-04 | Version 2.8.10 | [Edit this page](https://github.com/montge/asterix/edit/master/docs/index.md)
</small>
