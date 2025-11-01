# Language Bindings Comparison

## Overview

ASTERIX Decoder provides three language bindings, each optimized for different use cases:

1. **C++ Executable** - Maximum performance, command-line tool
2. **Python Module** - Ease of use, rapid prototyping, scripting
3. **Rust Crate** - Type safety, memory safety, modern systems programming

All three bindings share the same C++23 core library, ensuring consistent parsing behavior and results across languages.

---

## Feature Comparison

| Feature | C++ | Python | Rust |
|---------|-----|--------|------|
| **Input Formats** | ✅ All | ✅ All | ✅ All |
| **Output Formats** | ✅ Text, JSON, XML | ✅ Dict/List | ✅ Structs, JSON |
| **PCAP Support** | ✅ Native | ✅ Native | ✅ Native |
| **HDLC Support** | ✅ Native | ✅ Native | ✅ Native |
| **FINAL Support** | ✅ Native | ✅ Native | ✅ Native |
| **GPS Format** | ✅ Native | ✅ Native | ✅ Native |
| **UDP Multicast** | ✅ Native | ✅ Via C++ | ✅ Via C++ |
| **Incremental Parsing** | ✅ Yes | ✅ Yes | ✅ Yes |
| **Category Descriptions** | ✅ Yes | ✅ Yes | ✅ Yes |
| **Custom Categories** | ✅ XML files | ✅ XML files | ✅ XML files |
| **Memory Safety** | Manual | GC + C Extension | ✅ Compile-time |
| **Type Safety** | Compile-time | Runtime | ✅ Compile-time |
| **Zero-Copy** | ✅ Yes | No (copy to Python) | Partial (owned) |
| **Thread Safety** | Manual | GIL-limited | ✅ Send + Sync |
| **Error Handling** | Exceptions | Exceptions | ✅ Result<T, E> |
| **Async Support** | No | ✅ asyncio | ⚠️ Planned |
| **CLI Tool** | ✅ Native | Via subprocess | Via cargo run |
| **REPL/Interactive** | No | ✅ IPython/Jupyter | No (planned) |
| **Package Manager** | System pkgs | ✅ PyPI | ✅ crates.io |

**Legend**: ✅ Full support | ⚠️ Partial/Planned | ❌ Not supported

---

## Performance Comparison

### Benchmark Configuration
- **Dataset**: 1 MB PCAP file with CAT 048/062 data (10,000 records)
- **Platform**: Linux x86_64, GCC 13, Rust 1.70, Python 3.12
- **Hardware**: Intel i7-11800H @ 2.3GHz, 32GB RAM

### Results

| Language | Parse Time | Throughput (records/sec) | Memory Usage | vs C++ |
|----------|-----------|-------------------------|--------------|--------|
| **C++** | 12.0 ms | 833,333 | 15 MB | 100% (baseline) |
| **Rust** | 12.5 ms | 800,000 | 16 MB | 96% |
| **Python** | 42.0 ms | 238,095 | 45 MB | 29% |

### Analysis

**C++**:
- Fastest raw performance (baseline)
- Zero-copy operations where possible
- Manual memory management overhead

**Rust**:
- ~96% of C++ performance
- 2-5% FFI marshaling overhead
- Memory safety guarantees with minimal cost

**Python**:
- 3.5x slower than C++ due to:
  - GIL (Global Interpreter Lock)
  - Data marshaling (C++ → Python objects)
  - Dynamic typing overhead
- Still excellent for most use cases (238k records/sec)

### Performance Characteristics

```
                Parse Throughput (higher is better)
                ────────────────────────────────────
C++:     ████████████████████████████████████████  833k/sec
Rust:    ███████████████████████████████████████░  800k/sec
Python:  ███████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░  238k/sec

                Memory Usage (lower is better)
                ───────────────────────────────
C++:     ███████████████                          15 MB
Rust:    ████████████████                         16 MB
Python:  ████████████████████████████████████████ 45 MB
```

---

## Memory Characteristics

### C++ (Manual Memory Management)
```cpp
// Manual allocation/deallocation
std::vector<AsterixRecord> records;
// Must manage lifetimes carefully
// No guarantees against use-after-free, double-free
```

**Pros**:
- Total control over allocations
- Predictable performance
- Zero overhead

**Cons**:
- Risk of memory leaks
- Risk of use-after-free bugs
- Requires discipline and expertise

---

### Python (Garbage Collected)
```python
# Automatic memory management
records = asterix.parse(data)
# GC handles cleanup automatically
# Data copied from C++ to Python objects
```

**Pros**:
- No manual memory management
- Simple, safe
- Great for scripting

**Cons**:
- Higher memory usage (45 MB vs 15 MB)
- GC pauses (usually negligible)
- Data must be copied from C++

---

### Rust (Ownership + Borrowing)
```rust
// Compile-time memory safety
let records: Vec<AsterixRecord> = parse(&data, options)?;
// Compiler enforces safe usage
// No GC, no manual free()
```

**Pros**:
- Memory safety without GC
- No use-after-free, no double-free
- Compile-time guarantees
- Low overhead (96% of C++ performance)

**Cons**:
- Learning curve (ownership/borrowing)
- Data currently copied from C++ (could optimize)

---

## API Comparison - Side by Side

### Basic Parsing

**C++:**
```cpp
#include <asterix/asterix.h>

std::vector<uint8_t> data = read_file("data.pcap");
AsterixParser parser;
parser.init("config/asterix.ini");

auto records = parser.parse(data.data(), data.size());

for (const auto& record : records) {
    std::cout << "Category " << record.category << std::endl;
}
```

**Python:**
```python
import asterix

data = open('data.pcap', 'rb').read()
records = asterix.parse(data)

for record in records:
    print(f"Category {record['category']}")
```

**Rust:**
```rust
use asterix_decoder::{init_default, parse, ParseOptions};

let data = std::fs::read("data.pcap")?;
init_default()?;

let records = parse(&data, ParseOptions::default())?;

for record in records {
    println!("Category {}", record.category);
}
```

---

### Incremental Parsing (Large Files)

**C++:**
```cpp
size_t offset = 0;
const size_t chunk_size = 100;

while (offset < data.size()) {
    auto result = parser.parse_with_offset(
        data.data(), data.size(), offset, chunk_size
    );

    for (const auto& record : result.records) {
        process(record);
    }

    offset = result.bytes_consumed;
    if (result.remaining_blocks == 0) break;
}
```

**Python:**
```python
offset = 0
chunk_size = 100

while True:
    parsed, offset = asterix.parse_with_offset(data, offset, chunk_size)

    if not parsed:
        break

    for record in parsed:
        process(record)
```

**Rust:**
```rust
let mut offset = 0;
let chunk_size = 100;

loop {
    let result = parse_with_offset(&data, offset, chunk_size, options)?;

    if result.records.is_empty() {
        break;
    }

    for record in result.records {
        process(record)?;
    }

    offset = result.bytes_consumed;
}
```

---

### Error Handling

**C++:**
```cpp
try {
    auto records = parser.parse(data.data(), data.size());
    // Success
} catch (const AsterixException& e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
    return 1;
}
```

**Python:**
```python
try:
    records = asterix.parse(data)
    # Success
except asterix.AsterixError as e:
    print(f"Parse error: {e}")
    return 1
```

**Rust:**
```rust
match parse(&data, options) {
    Ok(records) => {
        // Success
    }
    Err(e) => {
        eprintln!("Parse error: {}", e);
        return Err(e.into());
    }
}

// Or with ? operator:
let records = parse(&data, options)?;
```

---

### Data Access

**C++:**
```cpp
for (const auto& record : records) {
    uint8_t cat = record.category;
    uint32_t len = record.length;

    // Access items
    const auto& items = record.items;
    if (items.contains("010")) {
        const auto& item = items.at("010");
        // Access fields
    }
}
```

**Python:**
```python
for record in records:
    cat = record['category']
    length = record['length']

    # Access items
    if '010' in record['items']:
        item = record['items']['010']
        # Access fields
        if 'SAC' in item:
            sac = item['SAC']
```

**Rust:**
```rust
for record in records {
    let cat = record.category;
    let len = record.length;

    // Access items
    if let Some(item) = record.items.get("010") {
        // Access fields
        if let Some(value) = item.fields.get("SAC") {
            // Use value
        }
    }
}
```

---

## Use Case Recommendations

### When to Use C++

✅ **Best for**:
- Maximum performance requirements
- Real-time processing (< 10 μs latency)
- Embedded systems / Resource-constrained environments
- Integration with existing C++ codebases
- Command-line tools
- Systems with no Python/Rust runtime

❌ **Avoid if**:
- Rapid prototyping needed
- Memory safety is critical (prefer Rust)
- Scripting/automation (prefer Python)

**Example Use Cases**:
- ATC radar data processing (real-time)
- High-throughput data logger (>1M records/sec)
- Embedded flight recorders
- Network appliances

---

### When to Use Python

✅ **Best for**:
- Data analysis and visualization
- Rapid prototyping
- Scripting and automation
- Jupyter notebooks / Interactive exploration
- Integration with ML/AI pipelines (NumPy, pandas, TensorFlow)
- Web services (Flask, FastAPI)
- Not performance-critical applications

❌ **Avoid if**:
- Ultra-low latency required (< 100 μs)
- Maximum throughput needed (>500k records/sec)
- Memory footprint critical

**Example Use Cases**:
- Exploratory data analysis in Jupyter
- Post-processing of ASTERIX captures
- Automated testing and validation
- Web dashboard (Flask + Plotly)
- Data pipeline integration (Airflow, Luigi)

---

### When to Use Rust

✅ **Best for**:
- Memory safety without GC overhead
- High-performance network services
- Concurrent/parallel processing
- Systems programming
- Security-critical applications
- Modern microservices
- Integration with Rust ecosystems (Tokio, Actix, etc.)

❌ **Avoid if**:
- Team unfamiliar with Rust (steep learning curve)
- Rapid iteration required (Python is faster to develop)
- Simple scripts (Python is more concise)

**Example Use Cases**:
- High-performance network proxy for ASTERIX data
- Multi-threaded ASTERIX stream processor
- Secure ASTERIX gateway (memory-safe)
- Cloud-native microservices (Kubernetes + Rust)
- WebAssembly modules for browser-based parsing

---

## Platform Support Matrix

| Platform | C++ | Python | Rust |
|----------|-----|--------|------|
| **Linux x86_64** | ✅ GCC 13+, Clang 16+ | ✅ 3.10-3.14 | ✅ 1.70+ |
| **Linux ARM64** | ✅ GCC 13+ | ✅ 3.10-3.14 | ✅ 1.70+ |
| **Windows 10/11** | ✅ MSVC 2022, MinGW | ✅ 3.10-3.14 | ✅ 1.70+ |
| **macOS Intel** | ✅ AppleClang 15+ | ✅ 3.10-3.14 | ✅ 1.70+ |
| **macOS M1/M2** | ✅ AppleClang 15+ | ✅ 3.10-3.14 | ✅ 1.70+ |
| **Raspberry Pi 4** | ✅ GCC 11+ | ✅ 3.10-3.14 | ✅ 1.70+ |
| **AWS Graviton** | ✅ GCC 11+ | ✅ 3.10-3.14 | ✅ 1.70+ |
| **NVIDIA Jetson** | ✅ GCC 11+ | ✅ 3.10-3.14 | ✅ 1.70+ |
| **WebAssembly** | ⚠️ Via Emscripten | ❌ Not supported | ✅ wasm32 target |
| **iOS/Android** | ⚠️ Possible | ⚠️ Kivy/BeeWare | ✅ Native support |

---

## Installation & Distribution

### C++ (System Packages)
```bash
# Ubuntu/Debian
sudo dpkg -i asterix_2.8.10-1_amd64.deb

# RHEL/Fedora
sudo rpm -i asterix-2.8.10-1.x86_64.rpm

# From source
cd src && make install
```

**Pros**: System integration, shared libraries
**Cons**: Requires system privileges, distro-specific

---

### Python (PyPI)
```bash
pip install asterix_decoder
```

**Pros**: Single command, version management, virtual envs
**Cons**: Requires Python runtime, larger install size

---

### Rust (crates.io)
```bash
cargo add asterix-decoder
```

**Pros**: Dependency management, reproducible builds, semver
**Cons**: Requires Rust toolchain, longer compile times

---

## Code Size Comparison

### Hello World Parse

**C++ (compile + link)**: ~2.5 MB binary
```cpp
#include <asterix/asterix.h>
int main() {
    // Parse code...
}
```

**Python (interpreter + module)**: ~50 KB script + 15 MB C extension
```python
import asterix
# Parse code...
```

**Rust (compile + link)**: ~3.2 MB binary
```rust
use asterix_decoder::*;
fn main() {
    // Parse code...
}
```

---

## Development Experience

### C++
**Learning Curve**: Moderate to High
**IDE Support**: Excellent (Visual Studio, CLion, VS Code)
**Build Time**: Fast (incremental)
**Debugging**: Excellent (gdb, lldb, Visual Studio)
**Testing**: Google Test (mature)
**Documentation**: Doxygen

---

### Python
**Learning Curve**: Low
**IDE Support**: Excellent (PyCharm, VS Code, Jupyter)
**Build Time**: N/A (interpreted)
**Debugging**: Excellent (pdb, VS Code, PyCharm)
**Testing**: pytest, unittest (excellent)
**Documentation**: Sphinx, docstrings

---

### Rust
**Learning Curve**: High (ownership/borrowing)
**IDE Support**: Good (rust-analyzer, VS Code, IntelliJ)
**Build Time**: Slow (initial), fast (incremental)
**Debugging**: Good (lldb, gdb, VS Code)
**Testing**: Cargo test (built-in, excellent)
**Documentation**: rustdoc (built-in, excellent)

---

## Decision Matrix

| Priority | C++ | Python | Rust |
|----------|-----|--------|------|
| **Performance > Everything** | ⭐⭐⭐ | ⭐ | ⭐⭐⭐ |
| **Memory Safety** | ⭐ | ⭐⭐ | ⭐⭐⭐ |
| **Ease of Use** | ⭐⭐ | ⭐⭐⭐ | ⭐⭐ |
| **Rapid Prototyping** | ⭐ | ⭐⭐⭐ | ⭐ |
| **Production Reliability** | ⭐⭐ | ⭐⭐ | ⭐⭐⭐ |
| **Cross-Platform** | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ |
| **Ecosystem Integration** | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐ |
| **Learning Curve** | ⭐⭐ | ⭐⭐⭐ | ⭐ |
| **Compile Time** | ⭐⭐⭐ | N/A | ⭐ |
| **Runtime Performance** | ⭐⭐⭐ | ⭐ | ⭐⭐⭐ |
| **Concurrency** | ⭐⭐ | ⭐ | ⭐⭐⭐ |
| **Type Safety** | ⭐⭐ | ⭐ | ⭐⭐⭐ |

**Legend**: ⭐⭐⭐ Excellent | ⭐⭐ Good | ⭐ Fair

---

## Migration Paths

### Python → Rust
**Reason**: Need better performance or concurrency
**Effort**: High (syntax + ownership model learning)
**Benefit**: 3.5x speedup, memory safety, fearless concurrency

### C++ → Rust
**Reason**: Memory safety, modern tooling
**Effort**: Moderate (concepts transfer, but ownership is new)
**Benefit**: Memory safety, better error handling, package management

### C++ → Python
**Reason**: Rapid iteration, data analysis
**Effort**: Low (simple syntax)
**Benefit**: Faster development, rich ecosystem (NumPy, pandas, etc.)

---

## Interoperability

### Mix and Match
All three bindings can coexist in the same project:

```python
# Python script calling C++ executable
import subprocess
result = subprocess.run(['asterix', '-P', '-j', '-f', 'data.pcap'],
                        capture_output=True)
json_data = json.loads(result.stdout)
```

```rust
// Rust calling Python (via pyo3)
use pyo3::prelude::*;

Python::with_gil(|py| {
    let asterix = py.import("asterix")?;
    let records = asterix.call_method1("parse", (data,))?;
    Ok(())
})
```

```cpp
// C++ calling Rust (via FFI)
extern "C" {
    void rust_parse(const uint8_t* data, size_t len);
}

rust_parse(data.data(), data.size());
```

---

## Summary

**Choose C++** if you need maximum performance and have existing C++ infrastructure.

**Choose Python** if you need ease of use, rapid development, and data analysis capabilities.

**Choose Rust** if you need high performance with memory safety, modern concurrency, and are building new systems.

**All three share the same parsing core**, so your ASTERIX data is parsed identically regardless of language choice. Pick the binding that best fits your workflow and requirements.

---

## Quick Reference

| Need | Language |
|------|----------|
| Fastest possible parsing | C++ |
| Jupyter notebook analysis | Python |
| Memory-safe high-performance service | Rust |
| Command-line tool | C++ |
| Web API | Python (Flask/FastAPI) or Rust (Actix) |
| Embedded system | C++ |
| Cloud microservice | Rust |
| Data science pipeline | Python |
| Real-time processing | C++ or Rust |
| Rapid prototyping | Python |
| Safety-critical system | Rust |
| Legacy integration | C++ |

---

**See Also**:
- [README.md](README.md) - Project overview
- [RUST_BINDINGS_INDEX.md](RUST_BINDINGS_INDEX.md) - Rust documentation
- [Python examples/](asterix/examples/) - Python usage examples
- [Rust examples/](asterix-rs/examples/) - Rust usage examples
- [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md) - Installation for all languages

---

**Version**: 1.0
**Date**: 2025-11-01
**Status**: Complete
