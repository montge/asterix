# ASTERIX Decoder

[![CI Status](https://github.com/montge/asterix/actions/workflows/ci-verification.yml/badge.svg)](https://github.com/montge/asterix/actions/workflows/ci-verification.yml)
[![CodeQL](https://github.com/montge/asterix/workflows/CodeQL/badge.svg)](https://github.com/montge/asterix/security/code-scanning)
[![Coverage](https://img.shields.io/badge/coverage-92.2%25-brightgreen)](https://github.com/montge/asterix)
[![PyPI](https://img.shields.io/pypi/v/asterix_decoder.svg)](https://pypi.org/project/asterix_decoder/)
[![Python Versions](https://img.shields.io/pypi/pyversions/asterix_decoder.svg)](https://pypi.org/project/asterix_decoder/)
[![License](https://img.shields.io/badge/license-GPL--2.0-blue.svg)](LICENSE)

> Professional ASTERIX protocol decoder for Air Traffic Management surveillance data

**ASTERIX** (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is the ATM surveillance data binary messaging format used worldwide. This decoder parses ASTERIX data from files, stdin, or network multicast streams.

More about ASTERIX protocol: http://www.eurocontrol.int/services/asterix

## Features

- **Multi-format support**: PCAP, raw binary, FINAL, HDLC, GPS
- **Python module** with simple, intuitive API
- **Fast C++ executable** for command-line use
- **Multiple output formats**: JSON, XML, human-readable text
- **Network streaming** via UDP multicast
- **92.2% test coverage** | 560 passing tests
- **DO-278A compliant** development process for aviation software safety

## Quick Start

### Python Module (Recommended)

```bash
pip install asterix_decoder
```

```python
import asterix

# Parse ASTERIX data
with open('capture.pcap', 'rb') as f:
    data = f.read()

parsed = asterix.parse(data)

# Display results
for record in parsed:
    print(asterix.describe(record))
```

### Pre-built Packages

Download pre-built packages from [GitHub Releases](https://github.com/montge/asterix/releases):

| Platform | Package Type | Installation |
|----------|--------------|--------------|
| **Ubuntu 20.04/22.04/24.04** | `.deb` | `sudo dpkg -i asterix_*.deb` |
| **Debian 11/12** | `.deb` | `sudo dpkg -i asterix_*.deb` |
| **RHEL/Rocky/Fedora** | `.rpm` | `sudo rpm -i asterix-*.rpm` |
| **Windows** | Installer | See [BUILD_WINDOWS.md](BUILD_WINDOWS.md) |

### Build from Source

**C++ executable:**
```bash
cd src
make install
cd ../install
./asterix --help
```

**Python module:**
```bash
python setup.py install
```

**Dependencies:**
- Linux/macOS: `libexpat-devel` (XML parsing)
- Windows: See [BUILD_WINDOWS.md](BUILD_WINDOWS.md)

## Documentation

- **Installation Guide**: [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md)
- **Contributing Guide**: [CONTRIBUTING.md](CONTRIBUTING.md) (Coming soon)
- **Architecture Overview**: [CLAUDE.md](CLAUDE.md) - Technical architecture
- **Packaging Status**: [PACKAGING_AND_CI_STATUS.md](PACKAGING_AND_CI_STATUS.md)
- **ASTERIX Specifications**: [asterix-specs-converter/](asterix-specs-converter/README.md)
- **API Documentation**: [ReadTheDocs](https://asterix.readthedocs.io/) (placeholder - setup pending)

## Supported ASTERIX Categories

**Fully Supported:**
- CAT 001, 002, 004, 008, 010, 011, 019, 020, 021, 023, 025, 030, 031, 032, 034
- CAT 048, 062, 063, 065
- CAT 205, 240, 247, 252

See [asterix/config/](asterix/config/) for specific version details and [asterix-specs-converter/](asterix-specs-converter/) for specification updates.

## Usage Examples

### Command Line

```bash
# Parse PCAP file to human-readable text
asterix -P -f capture.pcap

# Parse with JSON output
asterix -P -j -f capture.pcap > output.json

# Parse with extensive JSON (includes descriptions)
asterix -P -je -f capture.pcap > detailed.json

# Receive multicast stream (format: mcast_addr:interface_ip:port)
asterix -i 232.1.1.11:192.168.1.100:21111

# Parse raw ASTERIX file
asterix -f data.ast

# Parse FINAL format
asterix -F -f data.final

# Parse HDLC format
asterix -H -f data.hdlc
```

**Output Format Options:**
- Default: Human-readable text
- `-l, --line`: One line per item (parsable)
- `-j, --json`: Compact JSON (one object per line)
- `-jh, --jsonh`: Human-readable JSON (formatted)
- `-je, --json-extensive`: Extensive JSON with descriptions
- `-x, --xml`: Compact XML
- `-xh, --xmlh`: Human-readable XML

### Python API

**Basic parsing:**
```python
import asterix

# Parse ASTERIX data
data = b'\x30\x00\x30\xfd...'  # Raw ASTERIX bytes
parsed = asterix.parse(data)
print(parsed)
```

**Incremental parsing for large streams:**
```python
offset = 0
blocks_to_read = 10

while True:
    parsed, offset = asterix.parse_with_offset(data, offset, blocks_to_read)
    if not parsed:
        break

    for record in parsed:
        process(record)
```

**Get human-readable descriptions:**
```python
# Get description for a specific value
description = asterix.describe(category=48, item='010', field='SAC', value=7)
print(description)
```

**Load custom category definitions:**
```python
# Load additional or custom ASTERIX category XML
asterix.init('/path/to/custom_asterix_cat.xml')
```

### More Examples

See [asterix/examples/](asterix/examples/) directory for complete examples:
- [`read_raw_bytes.py`](asterix/examples/read_raw_bytes.py) - Parse from byte array
- [`read_raw_file.py`](asterix/examples/read_raw_file.py) - Parse from file
- [`read_pcap_file.py`](asterix/examples/read_pcap_file.py) - Parse PCAP capture
- [`read_final_file.py`](asterix/examples/read_final_file.py) - Parse FINAL format
- [`multicast_receive.py`](asterix/examples/multicast_receive.py) - Receive multicast stream
- [`multicast_send_receive.py`](asterix/examples/multicast_send_receive.py) - Full multicast demo
- [`xml_parser.py`](asterix/examples/xml_parser.py) - Work with XML definitions

## Development

### Setup Development Environment

```bash
# Clone repository
git clone https://github.com/montge/asterix.git
cd asterix

# Install Python development dependencies
pip install -e .
```

### Run Tests

```bash
# Python tests
python -m unittest

# C++ integration tests
cd install/test
./test.sh

# Memory leak tests (requires valgrind)
cd install/test
./valgrind_test.sh
```

### Build System

**Make (Primary):**
```bash
cd src
make              # Production build
make debug        # Debug build with symbols
make install      # Install to install/ directory
make clean        # Clean build artifacts
make test         # Run test suite
```

**CMake (Alternative):**
```bash
cmake .
make
```

### CI/CD Pipeline

All commits are automatically tested across:
- **Python versions**: 3.8, 3.9, 3.10, 3.11, 3.12
- **Build systems**: Make, CMake
- **Quality checks**: Coverage analysis, memory checks (Valgrind), static analysis
- **Security**: CodeQL scanning for C++ and Python
- **Platforms**: Ubuntu 20.04, 22.04, 24.04, Debian 11/12, RHEL/Rocky 8/9, Fedora 38/39/40

See [PACKAGING_AND_CI_STATUS.md](PACKAGING_AND_CI_STATUS.md) for detailed CI/CD status.

## Project Status

- **Test Coverage**: 92.2% (560 tests, 100% passing)
- **CI/CD**: All workflows active and passing
- **Security**: CodeQL enabled, 0 known vulnerabilities
- **Dependencies**: Dependabot monitoring active
- **Compliance**: DO-278A AL-3 process in progress
- **Packaging**: DEB, RPM, and Windows builds available

## ASTERIX Specification Updates

Category definitions are sourced from [asterix-specs](https://github.com/zoranbosnjak/asterix-specs) and converted to XML format.

To update specifications:
```bash
cd asterix-specs-converter

# Update all specifications
python3 update-specs.py

# Convert single category
curl https://zoranbosnjak.github.io/asterix-specs/specs/cat062/cats/cat1.18/definition.json | \
  python3 asterixjson2xml.py > specs/asterix_cat062_1_18.xml
```

See [asterix-specs-converter/README.md](asterix-specs-converter/README.md) for details.

## Contributing

Contributions are welcome! Please see our contributing guidelines (coming soon).

**Before submitting PRs:**
1. Ensure all tests pass: `python -m unittest`
2. Run integration tests: `cd install/test && ./test.sh`
3. Check code coverage remains above 90%
4. Follow existing code style (C++ and Python)
5. Add tests for new functionality
6. Update documentation as needed

## License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.

This program comes with ABSOLUTELY NO WARRANTY. See [LICENSE](LICENSE) for details.

## Credits

**Originally developed by**: [Croatia Control Ltd.](https://github.com/CroatiaControlLtd)
**Current maintainer**: [@montge](https://github.com/montge)

For questions about the ASTERIX protocol or this tool, feel free to contact the original author:
[Damir Salantic](https://hr.linkedin.com/in/damirsalantic)

## Support

- **Report Issues**: [GitHub Issues](https://github.com/montge/asterix/issues)
- **Discussions**: [GitHub Discussions](https://github.com/montge/asterix/discussions)
- **PyPI Package**: [asterix_decoder](https://pypi.org/project/asterix_decoder/)

---

**Star this repository if you find it useful!**
