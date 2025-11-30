---
layout: default
title: ASTERIX Decoder Documentation
---

# ASTERIX Decoder Documentation

[![C++ Docs](https://img.shields.io/badge/docs-C%2B%2B%20API-blue)](cpp/html/)
[![Python Docs](https://img.shields.io/badge/docs-Python-blue)](https://asterix.readthedocs.io/)
[![Rust Docs](https://img.shields.io/badge/docs-Rust-blue)](https://docs.rs/asterix-decoder)
[![GitHub](https://img.shields.io/badge/source-GitHub-black)](https://github.com/montge/asterix)

ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is a decoder/parser for the EUROCONTROL ASTERIX protocol - an ATM (Air Traffic Management) Surveillance Data Binary Messaging Format.

---

## Choose Your Language

### 🔧 C++
High-performance command-line tool for processing ASTERIX data.

- **[C++ API Reference](cpp/html/)** - Complete API documentation (Doxygen)
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
- **[C++ API Reference](cpp/html/)** - Complete C++ API documentation

### User Guides
- **[C++ User Guide](guides/CPP_GUIDE.html)** - Command-line tool and library guide
- **[Python User Guide](guides/PYTHON_GUIDE.html)** - Complete Python API guide
- **[Rust User Guide](guides/RUST_GUIDE.html)** - Complete Rust API guide
- **[Wireshark Plugin Guide](guides/WIRESHARK_GUIDE.html)** - Wireshark ASTERIX_EXT plugin
- **[Troubleshooting Guide](TROUBLESHOOTING.html)** - Solutions to common issues

### Technical Resources
- **[Architecture Overview](ARCHITECTURE.html)** - System design and layer separation
- **[Binding Guidelines](BINDING_GUIDELINES.html)** - Creating new language bindings
- **[Protocol Integration](PROTOCOL_INTEGRATION.html)** - Adding protocol adapters
- **[GitHub Repository](https://github.com/montge/asterix)** - Source code and examples
- **[Performance Optimizations](https://github.com/montge/asterix/blob/master/PERFORMANCE_OPTIMIZATIONS.md)** - 55-61% speedup details
- **[Security Policy](https://github.com/montge/asterix/blob/master/SECURITY.md)** - Security audit and policy

### Development
- **[Contributing Guide](https://github.com/montge/asterix/blob/master/CONTRIBUTING.md)** - How to contribute
- **[Safety-Critical Guide](SAFETY_CRITICAL.html)** - DO-278 compliance patterns
- **[DO-278 Compliance](https://github.com/montge/asterix/tree/master/do-278)** - Aviation safety certification
- **[Issue Tracker](https://github.com/montge/asterix/issues)** - Report bugs or request features

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
- **Security:** See [SECURITY.md](https://github.com/montge/asterix/blob/master/SECURITY.md)

---

<small>
Last updated: 2025-11-30 | Version 2.9.0 | [Edit this page](https://github.com/montge/asterix/edit/master/docs/index.md)
</small>
