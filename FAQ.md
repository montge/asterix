# Frequently Asked Questions (FAQ)

Quick answers to common questions about the ASTERIX decoder project.

---

## Table of Contents

- [General Questions](#general-questions)
- [Installation & Setup](#installation--setup)
- [Usage & Features](#usage--features)
- [Performance](#performance)
- [Language Bindings](#language-bindings)
- [Security](#security)
- [Contributing](#contributing)
- [Licensing](#licensing)
- [Troubleshooting](#troubleshooting)

---

## General Questions

### What is ASTERIX?

**ASTERIX** (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is a binary messaging format used in Air Traffic Management (ATM) for exchanging surveillance data. It's maintained by EUROCONTROL and used worldwide for radar, ADS-B, multilateration, and other surveillance systems.

**Official specification**: http://www.eurocontrol.int/services/asterix

---

### What does this decoder do?

This project provides a high-performance decoder that:
- **Parses** ASTERIX binary data into readable formats (JSON, XML, text)
- **Supports** 24 ASTERIX categories (CAT 001-252)
- **Handles** multiple input formats (PCAP, raw, FINAL, HDLC, GPS)
- **Provides** three language bindings (C++, Python, Rust)
- **Enables** real-time processing of multicast streams
- **Achieves** 10.95x faster JSON generation than baseline

---

### Who uses this decoder?

**Primary users**:
- Air Navigation Service Providers (ANSPs)
- Air Traffic Control systems integrators
- Aviation software developers
- Surveillance data analysts
- Research institutions
- Training simulators

**Use cases**:
- Real-time surveillance data processing
- Offline PCAP file analysis
- System integration testing
- Data quality monitoring
- Research and development

---

### Is this production-ready?

**Yes**. The decoder is:
- ✅ **Battle-tested**: Used in operational ATM systems
- ✅ **Safety-critical**: Follows DO-278A development process
- ✅ **Well-tested**: 92.2% code coverage, 560 passing tests
- ✅ **Memory-safe**: Zero memory leaks (verified with Valgrind)
- ✅ **High-performance**: 5.85x faster than requirements
- ✅ **Secure**: Regular security audits, ASAN/MSAN clean
- ✅ **Documented**: Comprehensive documentation across all bindings

---

### What ASTERIX categories are supported?

**24 categories** currently supported:

- **CAT 001**: Monosensor Surface Movement Data
- **CAT 002**: Monosensor Surface Movement Data
- **CAT 004**: Safety Net Messages
- **CAT 008**: Monosensor Surface Movement Data
- **CAT 010**: Monosensor Radar Data
- **CAT 011**: Monosensor Track Data
- **CAT 015**: INCS Target Reports (new in v2.8.10)
- **CAT 019**: Multilateration System Status
- **CAT 020**: Multilateration Target Reports
- **CAT 021**: ADS-B Target Reports
- **CAT 023**: CNS/ATM Ground Station Status
- **CAT 025**: CNS/ATM System Status
- **CAT 030**: ARTAS System Status
- **CAT 031**: ARTAS Track State
- **CAT 032**: Miniplan Messages
- **CAT 034**: Monosensor Service Messages
- **CAT 048**: Monosensor Target Reports
- **CAT 062**: SDPS Track Messages
- **CAT 063**: Sensor Status Messages
- **CAT 065**: SDPS Service Messages
- **CAT 205**: Ground Station Operational Messages
- **CAT 240**: Radar Video
- **CAT 247**: Reserved
- **CAT 252**: BDS Registers

See [ASTERIX_CATEGORY_SUPPORT_MATRIX.md](ASTERIX_CATEGORY_SUPPORT_MATRIX.md) for detailed field coverage.

---

### Can I add custom ASTERIX categories?

**Yes**. You can define custom categories using XML configuration files:

**C++:**
```bash
./asterix -d custom_config/asterix.ini -P -f capture.pcap
```

**Python:**
```python
import asterix
asterix.init('config/asterix_cat062_custom.xml')
```

**Rust:**
```rust
let parser = Parser::new()
    .add_category("config/asterix_cat062_custom.xml")
    .build()?;
```

XML configuration files follow the DTD specification in `asterix/config/asterix.dtd`.

---

## Installation & Setup

### How do I install the decoder?

**Choose your platform and method:**

**1. Pre-built packages** (recommended for production):
```bash
# Ubuntu/Debian
sudo dpkg -i asterix_*.deb

# RHEL/Fedora
sudo rpm -i asterix-*.rpm

# Windows
# Extract ZIP or run MSI installer

# macOS
# Extract TAR.GZ or install PKG
```

**2. Python module** (from PyPI):
```bash
pip install asterix_decoder
```

**3. Rust crate** (from crates.io):
```bash
cargo add asterix-decoder
```

**4. Build from source**:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build
```

See [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md) for detailed instructions.

---

### What are the system requirements?

**Minimum requirements**:

**C++ executable:**
- C++23 compiler (GCC 13+, Clang 16+, MSVC 2022+, AppleClang 15+)
- CMake 3.20+
- libexpat-devel
- 512 MB RAM
- 100 MB disk space

**Python module:**
- Python 3.10, 3.11, 3.12, 3.13, or 3.14
- C++17 compatible compiler
- libexpat-devel

**Rust crate:**
- Rust 1.70+ (2021 edition)
- C++17 compatible compiler
- libexpat-devel

**Supported platforms:**
- Linux x86_64 and ARM64
- Windows 10/11 (x64)
- macOS 13+ (Intel and Apple Silicon)

---

### Do I need to build the C++ library first?

**It depends on the binding**:

**Python module:**
- **No** - `python setup.py build` compiles C++ sources automatically

**Rust crate:**
- **No** - `cargo build` invokes CMake automatically via build.rs

**C++ executable:**
- **Yes** - Use CMake to build:
```bash
cmake -B build && cmake --build build && cmake --install build
```

**Node.js bindings:**
- **Linux/macOS**: Yes (requires pre-built libasterix.so/dylib)
- **Windows**: No (compiles sources directly)

---

### Why can't it find libasterix.so.2?

**Error:**
```
error while loading shared libraries: libasterix.so.2: cannot open shared object file
```

**Solution:** Add library path to `LD_LIBRARY_PATH`:

```bash
# Temporary
export LD_LIBRARY_PATH=/path/to/asterix/install/lib:$LD_LIBRARY_PATH

# Permanent (Linux)
sudo ldconfig /path/to/asterix/install/lib
```

See [TROUBLESHOOTING.md](TROUBLESHOOTING.md#shared-library-not-found) for detailed solutions.

---

## Usage & Features

### How do I parse a PCAP file?

**C++ executable:**
```bash
./asterix -P -f capture.pcap -jh
```

**Python:**
```python
import asterix

with open('capture.pcap', 'rb') as f:
    data = f.read()

records = asterix.parse(data)
for record in records:
    print(record)
```

**Rust:**
```rust
use asterix::{init_default, parse, ParseOptions};

init_default()?;
let data = std::fs::read("capture.pcap")?;
let records = parse(&data, ParseOptions::default())?;
```

---

### What output formats are supported?

**7 output formats**:

1. **Text** (default) - Human-readable
2. **Line-delimited** (`-l`) - Parseable, one item per line
3. **Compact JSON** (`-j`) - One object per line, suitable for streaming
4. **Human-readable JSON** (`-jh`) - Pretty-printed JSON
5. **Extensive JSON** (`-je`) - JSON with descriptions
6. **Compact XML** (`-x`) - Line-delimited XML
7. **Human-readable XML** (`-xh`) - Pretty-printed XML

**Recommendation**: Use `-j` (compact JSON) for production pipelines.

---

### Can I parse data from stdin?

**Yes**:

```bash
# PCAP from stdin
cat capture.pcap | ./asterix -P

# Raw ASTERIX from stdin
cat asterix.raw | ./asterix

# From network capture
tcpdump -i eth0 -w - 'udp port 21112' | ./asterix -P
```

---

### How do I receive multicast data?

```bash
# Syntax: -i MCAST_IP:INTERFACE_IP:PORT[:SOURCE_IP]
./asterix -i 239.0.0.1:192.168.1.100:21112

# With source filtering (SSM)
./asterix -i 239.0.0.1:192.168.1.100:21112:10.17.22.23

# Multiple groups
./asterix -i 239.0.0.1:192.168.1.100:21112@239.0.0.2:192.168.1.100:21113
```

**Requirements**:
- Correct network interface IP (not 0.0.0.0)
- Multicast routing enabled
- Firewall allows UDP traffic

See [TROUBLESHOOTING.md](TROUBLESHOOTING.md#network--multicast-issues) for diagnostics.

---

### Can I filter specific data items?

**Yes**, using a filter file:

**Create filter.txt:**
```
I010
I040
I220
```

**Apply filter:**
```bash
./asterix -P -f capture.pcap -LF filter.txt
```

**Result**: Only items I010, I040, I220 will be output.

---

### How do I process large files?

**Use incremental parsing**:

**Python:**
```python
import asterix

with open('large.pcap', 'rb') as f:
    data = f.read()

offset = 0
batch_size = 100

while offset < len(data):
    records = asterix.parse_with_offset(data, offset, batch_size)

    for record in records:
        process(record)

    offset += estimated_bytes(records)
```

**C++:**
```bash
# Process with compact output to reduce memory
./asterix -P -f large.pcap -j > output.json
```

---

## Performance

### How fast is the decoder?

**Benchmarks (validated)**:

| Metric | Target | Actual | Speedup |
|--------|--------|--------|---------|
| **JSON generation** | 50,000 rec/sec | 547,610 rec/sec | **10.95x** |
| **PCAP processing** | 1,500 Mbps | 8,780 Mbps | **5.85x** |
| **UDP latency (p95)** | 100 μs | 3.79 μs | **26.4x** |
| **Packet loss** | <1% | 0% | Perfect |

See [BENCHMARK_RESULTS_2025-10-20.md](BENCHMARK_RESULTS_2025-10-20.md) for detailed results.

---

### Which output format is fastest?

**Ranking (fastest to slowest)**:

1. **Text** (default) - Fastest
2. **Line-delimited** (`-l`) - Very fast
3. **Compact JSON** (`-j`) - Fast
4. **Compact XML** (`-x`) - Moderate
5. **Human-readable JSON** (`-jh`) - Slow
6. **Human-readable XML** (`-xh`) - Slow
7. **Extensive JSON** (`-je`) - Slowest

**Recommendation**: Use `-j` for production (good balance of speed and parseability).

---

### Can I process multiple files in parallel?

**Yes**, using GNU parallel:

```bash
find . -name "*.pcap" | \
  parallel -j 4 "./asterix -P -f {} -j > {.}.json"
```

**Or in Python** (multiprocessing):
```python
import multiprocessing
import asterix

def parse_file(filename):
    with open(filename, 'rb') as f:
        return asterix.parse(f.read())

with multiprocessing.Pool(processes=4) as pool:
    results = pool.map(parse_file, filenames)
```

---

### What causes slow performance?

**Common causes**:

1. **Slow output format** (use `-j` instead of `-jh` or `-je`)
2. **I/O bottleneck** (write to fast disk or /dev/null)
3. **CPU throttling** (set governor to `performance`)
4. **Small buffers** (increase socket buffer sizes for multicast)
5. **Debug build** (use Release build: `cmake -DCMAKE_BUILD_TYPE=Release`)

See [TROUBLESHOOTING.md](TROUBLESHOOTING.md#performance-issues) for diagnostics.

---

## Language Bindings

### Which language binding should I use?

**Choose based on your needs**:

| Binding | Best For | Pros | Cons |
|---------|----------|------|------|
| **C++** | CLI tools, system integration | Fastest, full control | Manual memory mgmt |
| **Python** | Scripts, data analysis, ML | Simple API, rapid development | Thread-unsafe |
| **Rust** | Safe systems programming | Memory-safe, zero-copy | Steeper learning curve |
| **Node.js** | Web services, async I/O | JavaScript ecosystem | Addon complexity |

See [LANGUAGE_BINDINGS_COMPARISON.md](LANGUAGE_BINDINGS_COMPARISON.md) for detailed comparison.

---

### Are the bindings compatible?

**Yes**, all bindings use the same C++ core and produce identical output:

```bash
# C++ executable
./asterix -P -f test.pcap -j > cpp.json

# Python
python -c "import asterix; import json; print(json.dumps(asterix.parse(open('test.pcap','rb').read())))" > python.json

# Rust
cargo run --example parse_pcap test.pcap > rust.json

# Results are identical
diff cpp.json python.json  # No differences
```

---

### Is the Python module thread-safe?

**No**. The Python module uses a global C++ singleton and is **NOT thread-safe**.

**Solutions**:

1. **Use external locking**:
```python
import threading
lock = threading.Lock()

with lock:
    asterix.parse(data)
```

2. **Use multiprocessing** (recommended):
```python
import multiprocessing

with multiprocessing.Pool(4) as pool:
    results = pool.map(parse_worker, data_chunks)
```

See [Python README](asterix/README.md#thread-safety) for details.

---

### Is the Rust crate thread-safe?

**No** (intentional design). The Rust crate wraps the same C++ singleton.

**Reason**: Global category definitions stored in singleton.

**Solution**: Use external synchronization or process-based parallelism.

See [Rust README](asterix-rs/README.md) for details.

---

## Security

### Is the decoder secure?

**Yes**, the project follows security best practices:

- ✅ **Input validation**: All FFI boundaries validated (19/19 tests passing)
- ✅ **Bounds checking**: Buffer overflows prevented
- ✅ **Memory safety**: Zero memory leaks (Valgrind verified)
- ✅ **ASAN clean**: No use-after-free or buffer overflows
- ✅ **Fuzzing**: Continuous fuzzing (cargo-fuzz, 1 hour nightly)
- ✅ **Security audits**: Regular audits with cargo-audit
- ✅ **Hardening**: Stack protector, fortify source, RELRO

See [SECURITY_AUDIT_REPORT.md](SECURITY_AUDIT_REPORT.md) for details.

---

### Can I use this in safety-critical systems?

**Yes**, with caveats:

**The project**:
- Follows DO-278A development process
- Has comprehensive test coverage (92.2%)
- Uses defensive programming
- Has detailed documentation
- Is used in operational ATM systems

**However**:
- You must conduct your own certification process
- Verify it meets your specific safety requirements
- Perform hazard analysis for your use case
- Maintain traceability documentation

See [DO-278_IMPLEMENTATION_GUIDE.md](DO-278_IMPLEMENTATION_GUIDE.md) for guidance.

---

### How are security vulnerabilities handled?

**Reporting**:
- Email security issues to maintainers (see [SECURITY.md](SECURITY.md))
- Do NOT file public issues for security vulnerabilities

**Response**:
- Acknowledged within 48 hours
- Fixed in priority order (critical → high → medium)
- Security patches released ASAP
- CVEs assigned when appropriate

---

### What about data privacy?

**The decoder**:
- Does **NOT** collect telemetry
- Does **NOT** phone home
- Does **NOT** store data (unless you redirect output)
- Processes data locally only

**Your responsibility**:
- ASTERIX data may contain sensitive operational information
- Follow your organization's data handling policies
- Encrypt network traffic if required
- Sanitize data before sharing publicly

---

## Contributing

### How can I contribute?

**Ways to contribute**:

1. **Report bugs**: https://github.com/montge/asterix/issues
2. **Suggest features**: https://github.com/montge/asterix/discussions
3. **Submit patches**: Create pull requests
4. **Improve documentation**: Fix typos, add examples
5. **Add test cases**: Expand coverage
6. **Performance improvements**: Profile and optimize
7. **Language bindings**: Add new bindings (Go, Java, Ruby)

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

---

### Do I need to sign a CLA?

**No**. This project does NOT require a Contributor License Agreement (CLA).

**License**: All contributions are under GPL-3.0-or-later.

---

### How do I add a new ASTERIX category?

**Option 1: Use upstream specs** (recommended):

1. Get JSON spec from https://zoranbosnjak.github.io/asterix-specs/
2. Convert to XML using `asterix-specs-converter`:
```bash
cd asterix-specs-converter
python3 asterixjson2xml.py < category.json > category.xml
```
3. Test with decoder
4. Submit PR

**Option 2: Write XML from scratch**:

1. Follow DTD specification in `asterix/config/asterix.dtd`
2. Create XML file following existing patterns
3. Validate against DTD
4. Test thoroughly
5. Submit PR

See [CONTRIBUTING.md](CONTRIBUTING.md) for details.

---

### What coding standards do you follow?

**C++**:
- C++23 standard (with C++17/20 fallback)
- Doxygen documentation for all public APIs
- No warnings (`-Wall -Wextra`)
- clang-format for formatting

**Python**:
- PEP 8 style guide
- Google-style docstrings
- Type hints where appropriate
- black or ruff for formatting

**Rust**:
- Rust 2021 edition
- cargo fmt for formatting
- cargo clippy (zero warnings)
- Comprehensive rustdoc

**All languages**:
- 80% minimum test coverage
- No memory leaks
- Security best practices

---

## Licensing

### What license is this under?

**GPL-3.0-or-later** (GNU General Public License version 3 or later).

**Key points**:
- ✅ Free to use, modify, and distribute
- ✅ Source code must remain open
- ⚠️ Derivative works must use GPL-compatible license
- ⚠️ Commercial use requires GPL compliance

See [LICENSE](LICENSE) for full text.

---

### Can I use this in commercial software?

**Yes**, but with conditions:

**If your software is open-source (GPL-compatible)**:
- ✅ Use freely
- Share your source code under GPL

**If your software is proprietary/closed-source**:
- ⚠️ GPL requires derivative works to be GPL
- Consider:
  - Using as separate process (IPC, not linking)
  - Dual licensing (contact maintainers)
  - Reimplementing cleanroom (not recommended)

**Consult a lawyer** if unsure about GPL compliance.

---

### Can I use this in a SaaS/cloud service?

**Yes**. AGPL would require source disclosure for SaaS, but **GPL-3.0 does not**.

**You can**:
- Run this decoder as a backend service
- Offer ASTERIX parsing as a service
- Include in cloud platforms

**You must**:
- Comply with GPL if you distribute the software
- Provide source code if you give binaries to customers

---

### Why GPL instead of MIT/Apache?

**Reasons**:
- Ensures improvements stay open-source
- Protects community contributions
- Follows upstream (CroatiaControlLtd/asterix) license
- Common for safety-critical aviation software

**Alternatives**:
- If GPL doesn't work, contact maintainers about dual licensing
- Consider using as a service (IPC) instead of linking

---

## Troubleshooting

### Where do I find more help?

**Documentation**:
1. [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Comprehensive troubleshooting guide
2. [README.md](README.md) - Project overview and quick start
3. Language-specific READMEs:
   - [Python README](asterix/README.md)
   - [Rust README](asterix-rs/README.md)
   - [C++ User Guide](docs/CPP_USER_GUIDE.md)
4. [BUILD.md](docs/BUILD.md) - Build instructions
5. [ARCHITECTURE.md](docs/ARCHITECTURE.md) - System design

**Community**:
- **GitHub Issues**: https://github.com/montge/asterix/issues
- **GitHub Discussions**: https://github.com/montge/asterix/discussions

**ASTERIX Protocol**:
- **Official docs**: http://www.eurocontrol.int/services/asterix
- **Specifications**: https://zoranbosnjak.github.io/asterix-specs/

---

### My question isn't answered here!

**Options**:

1. **Check troubleshooting guide**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
2. **Search existing issues**: https://github.com/montge/asterix/issues
3. **Ask in discussions**: https://github.com/montge/asterix/discussions
4. **File a new issue**: Include:
   - Environment (OS, compiler, version)
   - Reproduction steps
   - Error messages
   - Expected vs actual behavior

**Before asking**:
- Read relevant documentation
- Try troubleshooting steps
- Provide complete information (see bug reporting template)

---

## Quick Reference

### Common Commands

```bash
# Parse PCAP to JSON
./asterix -P -f capture.pcap -j

# Receive multicast
./asterix -i 239.0.0.1:192.168.1.100:21112

# List supported categories
./asterix -L

# Parse with custom config
./asterix -d custom/asterix.ini -P -f capture.pcap

# Enable verbose output
./asterix -v -P -f capture.pcap
```

---

### Common Python Usage

```python
import asterix

# Initialize (optional, auto-loads defaults)
asterix.init()

# Parse PCAP data
with open('capture.pcap', 'rb') as f:
    records = asterix.parse(f.read())

# Incremental parsing
records = asterix.parse_with_offset(data, offset, count)

# Get descriptions
desc = asterix.describe(62, "I010")
```

---

### Common Rust Usage

```rust
use asterix::{init_default, parse, ParseOptions};

// Initialize
init_default()?;

// Parse data
let data = std::fs::read("capture.pcap")?;
let records = parse(&data, ParseOptions::default())?;

// Process records
for record in records {
    println!("Category {}", record.category);
}
```

---

## Additional Resources

- **Performance Benchmarks**: [BENCHMARK_RESULTS_2025-10-20.md](BENCHMARK_RESULTS_2025-10-20.md)
- **Build Instructions**: [BUILD.md](docs/BUILD.md)
- **Architecture**: [ARCHITECTURE.md](docs/ARCHITECTURE.md)
- **Contributing**: [CONTRIBUTING.md](CONTRIBUTING.md)
- **Security**: [SECURITY.md](SECURITY.md)

---

**Last Updated**: 2025-11-16
**Version**: 2.8.10
