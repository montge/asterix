# ASTERIX Decoder Troubleshooting Guide

Comprehensive troubleshooting guide for common issues across all ASTERIX decoder bindings (C++, Python, Rust, Node.js).

---

## Table of Contents

- [Build Issues](#build-issues)
- [Runtime Issues](#runtime-issues)
- [Memory Issues](#memory-issues)
- [Network & Multicast Issues](#network--multicast-issues)
- [Platform-Specific Issues](#platform-specific-issues)
- [Performance Issues](#performance-issues)
- [Language-Specific Issues](#language-specific-issues)
- [Getting Help](#getting-help)

---

## Build Issues

### Shared Library Not Found

**Symptoms:**
```
./asterix: error while loading shared libraries: libasterix.so.2: cannot open shared object file
```

**Affected**: C++ executable, Node.js bindings (Linux/macOS), Rust bindings

**Solutions:**

#### Temporary Fix
```bash
# Linux
export LD_LIBRARY_PATH=/path/to/asterix/install/lib:$LD_LIBRARY_PATH

# macOS
export DYLD_LIBRARY_PATH=/path/to/asterix/install/lib:$DYLD_LIBRARY_PATH
```

#### Permanent Fix (Linux)
```bash
# Option 1: Update ldconfig
sudo ldconfig /path/to/asterix/install/lib

# Option 2: Add to ld.so.conf
echo "/path/to/asterix/install/lib" | sudo tee /etc/ld.so.conf.d/asterix.conf
sudo ldconfig
```

#### Permanent Fix (macOS)
```bash
# Add to ~/.zshrc or ~/.bashrc
export DYLD_LIBRARY_PATH=/path/to/asterix/install/lib:$DYLD_LIBRARY_PATH
```

---

### CMake Configuration Fails

**Symptoms:**
```
CMake Error: Could not find expat
```

**Cause**: Missing libexpat development files

**Solutions:**

```bash
# Ubuntu/Debian
sudo apt-get install libexpat1-dev

# Fedora/RHEL
sudo dnf install expat-devel

# macOS
brew install expat

# Windows (vcpkg)
vcpkg install expat:x64-windows
```

---

### C++ Compiler Version Too Old

**Symptoms:**
```
error: 'std::ranges' has not been declared
error: deduced 'this' is a C++23 extension
```

**Cause**: Compiler doesn't support C++23

**Solutions:**

```bash
# Check compiler version
g++ --version
clang++ --version

# Upgrade compiler (Ubuntu)
sudo apt-get update
sudo apt-get install g++-13

# Specify compiler
cmake -B build -DCMAKE_CXX_COMPILER=g++-13
```

**Requirements:**
- GCC 13+ (full C++23)
- Clang 16+ (full C++23)
- MSVC 2022+ (C++20, partial C++23)
- AppleClang 15+ (C++23)

---

### Python Build Fails

**Symptoms:**
```
error: command 'gcc' failed with exit status 1
```

**Common Causes:**

1. **Missing compiler**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential python3-dev

# macOS (install Xcode Command Line Tools)
xcode-select --install
```

2. **Missing headers**
```bash
# Install Python development headers
sudo apt-get install python3.12-dev  # Match your Python version
```

3. **Wrong Python version**
```bash
# Use supported Python (3.10-3.14)
python --version

# Create virtual environment
python3.12 -m venv venv
source venv/bin/activate
```

---

### Rust Build Fails

**Symptoms:**
```
error: failed to run custom build command for `asterix v0.1.0`
```

**Solutions:**

```bash
# Update Rust (requires 1.70+)
rustup update

# Check Rust version
rustc --version

# Clean and rebuild
cd asterix-rs
cargo clean
cargo build --release
```

**Common Issues:**
- **CXX bridge errors**: Ensure C++ compiler is available
- **CMake errors**: Install CMake 3.20+
- **Linking errors**: Check libexpat is installed

---

### Node.js Build Fails

**Symptoms:**
```
gyp ERR! build error
```

**Solutions:**

```bash
# Install node-gyp globally
npm install -g node-gyp

# Install build tools
# macOS
xcode-select --install

# Windows
npm install --global windows-build-tools

# Ubuntu/Debian
sudo apt-get install build-essential

# Rebuild
cd asterix-node
npm rebuild
```

---

## Runtime Issues

### Invalid ASTERIX Data Errors

**Symptoms:**
```
ERROR: Wrong length in DataItem format
ERROR: Record not parsed properly
ERROR: Not enough data for ASTERIX header
```

**Causes:**
1. Incorrect input format specified
2. Corrupted data file
3. Unsupported ASTERIX category

**Solutions:**

#### 1. Verify Input Format

```bash
# If it's a PCAP file, use -P flag
./asterix -P -f capture.pcap

# For raw ASTERIX binary
./asterix -f asterix.raw

# Enable verbose mode to see what's happening
./asterix -P -f capture.pcap -v
```

#### 2. Validate File Integrity

```bash
# Check file type
file capture.pcap

# Verify PCAP structure
tcpdump -r capture.pcap -c 10

# Check file size (should not be empty)
ls -lh capture.pcap
```

#### 3. Check Category Support

```bash
# List supported categories
./asterix -L

# Search for specific category
./asterix -L | grep "CAT XXX"
```

**Python:**
```python
import asterix

# List categories
asterix.describe(48)  # Check if CAT 048 is supported
```

**Rust:**
```rust
// Check available categories in XML config
asterix::init_default()?;
```

---

### Parse Function Returns Empty Results

**Symptoms**: Parse succeeds but returns empty list/array

**Causes:**
1. Data is valid but contains no ASTERIX blocks
2. Wrong offset specified
3. Data format mismatch

**Solutions:**

**Python:**
```python
import asterix

data = open('file.pcap', 'rb').read()

# Check data size
print(f"Data size: {len(data)} bytes")

# Try parsing
try:
    records = asterix.parse(data)
    print(f"Parsed {len(records)} records")
except Exception as e:
    print(f"Error: {e}")

# Try incremental parsing
records = asterix.parse_with_offset(data, 0, 100)
```

**C++:**
```bash
# Enable verbose output
./asterix -P -f file.pcap -v

# Check if data contains ASTERIX
hexdump -C file.pcap | head -100
```

---

### Configuration File Not Found

**Symptoms:**
```
Cannot open file: config/asterix.ini
```

**Solutions:**

#### C++ Executable
```bash
# Use absolute path
./asterix -d /path/to/asterix/install/share/asterix/config/asterix.ini -f file.pcap

# Or run from install directory
cd install
./bin/asterix -f ../file.pcap

# Or set working directory
cd /path/to/asterix/install
./bin/asterix -f /path/to/file.pcap
```

#### Python
```python
import asterix
import os

# Initialize with absolute path
config_path = '/path/to/asterix/asterix/config/asterix_cat062_1_19.xml'
asterix.init(config_path)
```

#### Rust
```rust
use asterix::{init_custom, Parser};

// Use custom config directory
let parser = Parser::new()
    .config_dir("/path/to/asterix/asterix/config")
    .build()?;
```

---

## Memory Issues

### Memory Leaks (Detected by Valgrind)

**Symptoms:**
```
==12345== LEAK SUMMARY:
==12345==    definitely lost: 1,024 bytes in 4 blocks
```

**Debugging:**

```bash
# Run with Valgrind
valgrind --leak-check=full --show-leak-kinds=all ./asterix -P -f test.pcap

# For Python
valgrind --leak-check=full python3 test_script.py

# Expected result (no leaks):
# All heap blocks were freed -- no leaks are possible
```

**Known Issues:**
- **Zero leaks expected**: This project has 0 memory leaks (verified in CI/CD)
- If you find leaks, report them as a bug

---

### ASAN Errors (AddressSanitizer)

**Symptoms:**
```
AddressSanitizer: heap-buffer-overflow
AddressSanitizer: use-after-free
```

**Debugging:**

```bash
# Build with ASAN
CC=clang CFLAGS="-fsanitize=address -fno-omit-frame-pointer -g" \
LDFLAGS="-fsanitize=address" \
python setup.py build_ext --inplace

# Run tests
ASAN_OPTIONS=detect_leaks=1:symbolize=1 python -m pytest
```

**Known Clean State:**
- All FFI security tests pass with ASAN (19/19 tests)
- If ASAN reports errors, it's likely a genuine bug - please report

---

### Python Module Crashes

**Symptoms**: Segmentation fault, Python interpreter crash

**Common Causes:**

1. **FFI boundary violation**
```python
# Bad: Passing invalid data types
asterix.parse(None)  # Will crash

# Good: Validate input
data = b"..."
if data:
    asterix.parse(data)
```

2. **Oversized data**
```python
# Bad: Data > 64KB
large_data = b'\x00' * 100000
asterix.parse(large_data)  # ValueError

# Good: Check size
if len(data) <= 65536:
    asterix.parse(data)
```

3. **Concurrent access**
```python
# Bad: Thread-unsafe access
import threading
threads = [threading.Thread(target=lambda: asterix.parse(data)) for _ in range(10)]
for t in threads: t.start()  # CRASH!

# Good: Use locking
import threading
lock = threading.Lock()
def safe_parse(data):
    with lock:
        return asterix.parse(data)
```

---

## Network & Multicast Issues

### No Data Received from Multicast

**Symptoms**: No output when using multicast input

**Diagnostics:**

```bash
# Check multicast group membership
ip maddr show

# Monitor network traffic
sudo tcpdump -i eth0 host 239.0.0.1 and port 21112

# Check routing
ip route get 239.0.0.1

# Verify interface
ip addr show eth0
```

**Common Causes:**

1. **Firewall Blocking**
```bash
# Check firewall rules
sudo iptables -L -n | grep 21112

# Allow multicast
sudo iptables -A INPUT -p udp --dport 21112 -j ACCEPT
```

2. **Wrong Interface**
```bash
# Use correct interface IP
./asterix -i 239.0.0.1:192.168.1.100:21112
#                        ^^^^^^^^^^^^^^^
#                        Use actual interface IP, not 0.0.0.0
```

3. **Multicast Routing Not Enabled**
```bash
# Enable multicast routing
sudo ip link set eth0 multicast on

# Join multicast group
sudo ip maddress add 239.0.0.1 dev eth0
```

---

### Packet Loss in Real-Time Processing

**Symptoms**: Missing records, gaps in data

**Causes:**
1. Network congestion
2. Slow processing
3. Small receive buffers

**Solutions:**

1. **Increase Socket Buffer Size**
```bash
# System-wide (Linux)
sudo sysctl -w net.core.rmem_max=8388608
sudo sysctl -w net.core.rmem_default=2097152
```

2. **Optimize Processing**
```bash
# Use faster output format
./asterix -i 239.0.0.1:192.168.1.100:21112 -j  # Compact JSON
# Not: -jh (human-readable is slower)
```

3. **Process in Separate Thread**
```python
import asterix
import socket
import queue
import threading

# Producer thread (network receive)
def receiver(q):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('', 21112))
    mreq = socket.inet_aton('239.0.0.1') + socket.inet_aton('192.168.1.100')
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    while True:
        data, _ = sock.recvfrom(65536)
        q.put(data)

# Consumer thread (parsing)
def parser(q):
    while True:
        data = q.get()
        records = asterix.parse(data)
        # Process records...

q = queue.Queue(maxsize=1000)
threading.Thread(target=receiver, args=(q,), daemon=True).start()
threading.Thread(target=parser, args=(q,), daemon=True).start()
```

---

## Platform-Specific Issues

### Windows: MSVC Compiler Errors

**Symptoms:**
```
error C2059: syntax error: 'this'
error C7555: use of designated initializers requires /std:c++20 or /std:c++latest
```

**Solutions:**

```batch
:: Use MSVC 2022 with C++20
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_CXX_STANDARD=20

:: Or use clang-cl
cmake -B build -G "Visual Studio 17 2022" -T ClangCL -A x64
```

**Note**: Windows uses C++20 (not C++23) due to MSVC compiler limitations

---

### macOS: Code Signing Issues

**Symptoms:**
```
"asterix" cannot be opened because the developer cannot be verified
```

**Solutions:**

```bash
# Allow execution
xattr -d com.apple.quarantine ./asterix

# Or build from source (automatically trusted)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build
```

---

### macOS: Apple Silicon (M1/M2) Compatibility

**Symptoms:**
```
Bad CPU type in executable
dyld: incompatible arch
```

**Solutions:**

```bash
# Build for ARM64
cmake -B build -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build

# Or universal binary (Intel + ARM)
cmake -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build
```

---

### Linux: GLIBC Version Mismatch

**Symptoms:**
```
version `GLIBC_2.34' not found
```

**Cause**: Binary built on newer system than runtime

**Solutions:**

1. **Build from source**
```bash
cmake -B build
cmake --build build
```

2. **Use older build environment**
```bash
# Use Ubuntu 20.04 for better compatibility
docker run -v $(pwd):/work ubuntu:20.04 bash -c "
  apt update && apt install -y build-essential cmake libexpat1-dev
  cd /work
  cmake -B build
  cmake --build build
"
```

---

## Performance Issues

### Slow PCAP Processing

**Symptoms**: Processing takes much longer than expected

**Benchmarks** (reference):
- **Target**: 1,500 Mbps throughput
- **Actual**: 8,780 Mbps (5.85x faster)

**Diagnostics:**

```bash
# Measure throughput
time ./asterix -P -f large.pcap > /dev/null

# Profile with perf (Linux)
perf record -g ./asterix -P -f large.pcap
perf report
```

**Common Causes:**

1. **Slow Output Format**
```bash
# Slow: Human-readable formats
./asterix -P -f large.pcap -jh  # JSON human-readable
./asterix -P -f large.pcap -je  # JSON extensive

# Fast: Compact formats
./asterix -P -f large.pcap -j   # JSON compact
./asterix -P -f large.pcap -l   # Line-delimited
```

2. **I/O Bottleneck**
```bash
# Bad: Writing to slow disk
./asterix -P -f large.pcap -j > /slow/disk/output.json

# Good: Writing to fast disk or /dev/null
./asterix -P -f large.pcap -j > /dev/null
./asterix -P -f large.pcap -j | gzip > output.json.gz
```

3. **CPU Frequency Scaling**
```bash
# Check CPU governor
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

# Set to performance
sudo cpufreq-set -g performance
```

---

### High Memory Usage

**Symptoms**: Excessive RAM consumption

**Expected Memory Usage**:
- **C++ executable**: ~10-50 MB
- **Python module**: ~20-100 MB
- **Rust crate**: ~15-60 MB

**Causes:**

1. **Large Files in Memory**
```python
# Bad: Load entire file
data = open('huge.pcap', 'rb').read()  # Loads all into RAM
records = asterix.parse(data)

# Good: Incremental processing
with open('huge.pcap', 'rb') as f:
    data = f.read()

offset = 0
batch_size = 100

while offset < len(data):
    records = asterix.parse_with_offset(data, offset, batch_size)
    # Process records immediately
    for record in records:
        process(record)
    del records  # Free memory
    offset += 1000  # Estimate
```

2. **Memory Leaks** (should not happen)
```bash
# Monitor memory
watch -n 1 'ps aux | grep asterix'

# Check for leaks
valgrind --leak-check=full ./asterix -P -f test.pcap
```

---

## Language-Specific Issues

### Python: Import Error

**Symptoms:**
```python
ModuleNotFoundError: No module named '_asterix'
```

**Solutions:**

```bash
# Ensure module is built
python setup.py build

# Check build output exists
ls -la build/lib.*/

# Install module
python setup.py install --user

# Or use pip
pip install -e .
```

---

### Python: Thread Safety Issues

**Symptoms**: Crashes, corrupted data, race conditions

**Cause**: ASTERIX Python module is **NOT thread-safe**

**Solution**: Use external locking

```python
import asterix
import threading

lock = threading.Lock()

def thread_safe_parse(data):
    with lock:
        return asterix.parse(data)

# Or use multiprocessing (isolated state)
import multiprocessing

def parse_worker(data):
    return asterix.parse(data)

with multiprocessing.Pool(processes=4) as pool:
    results = pool.map(parse_worker, data_chunks)
```

---

### Rust: Miri Undefined Behavior

**Symptoms:**
```
error: Undefined Behavior: Data race
```

**Cause**: Intentional design - library uses global C++ singleton

**Solution**: This is expected behavior

```rust
// Concurrent test is marked to skip under Miri
#[test]
#[cfg_attr(miri, ignore)]
fn test_concurrent_parsing() {
    // This is intentionally not thread-safe
}
```

**Note**: See `asterix-rs/src/lib.rs` thread safety documentation

---

### Node.js: Native Module Build Failures

**Symptoms:**
```
Error: Could not locate the bindings file
```

**Solutions:**

```bash
# Rebuild native module
npm rebuild

# Clear cache and reinstall
rm -rf node_modules package-lock.json
npm install

# Use node-gyp directly
node-gyp rebuild

# Check Node.js version (requires 18.x, 20.x, 22.x)
node --version
```

---

## Diagnostic Commands

### System Information

```bash
# OS version
uname -a
cat /etc/os-release  # Linux
sw_vers              # macOS

# Compiler versions
gcc --version
g++ --version
clang --version

# CMake version
cmake --version

# Python version
python --version
python3 --version

# Rust version
rustc --version
cargo --version

# Node.js version
node --version
npm --version
```

---

### Build Environment

```bash
# Check library locations
ldconfig -p | grep expat  # Linux
brew list expat            # macOS

# Check CMake configuration
cmake -B build -LA

# Check library dependencies
ldd ./install/bin/asterix  # Linux
otool -L ./install/bin/asterix  # macOS

# Check symbols
nm -D ./install/lib/libasterix.so | grep parse  # Linux
```

---

### Runtime Diagnostics

```bash
# Enable verbose output (C++)
./asterix -v -P -f test.pcap

# Python debugging
python -v -c "import asterix"

# Rust verbose build
cargo build --verbose

# Node.js debugging
node --trace-warnings test.js
```

---

## Getting Help

### Before Reporting Issues

1. **Check this troubleshooting guide**
2. **Search existing issues**: https://github.com/montge/asterix/issues
3. **Read documentation**:
   - [README.md](README.md)
   - [ARCHITECTURE.md](docs/ARCHITECTURE.md)
   - [BUILD.md](docs/BUILD.md)
   - [Python README](asterix/README.md)
   - [Rust README](asterix-rs/README.md)
   - [C++ User Guide](docs/CPP_USER_GUIDE.md)

### Reporting Bugs

When creating an issue, include:

1. **Environment**:
   ```
   - OS: Ubuntu 22.04 / macOS 14 / Windows 11
   - Compiler: GCC 13.2 / Clang 16 / MSVC 2022
   - Language binding: C++ / Python 3.12 / Rust 1.75 / Node.js 20.x
   - ASTERIX version: 2.8.10
   ```

2. **Reproduction steps**:
   ```bash
   git clone https://github.com/montge/asterix.git
   cd asterix
   cmake -B build
   cmake --build build
   ./install/bin/asterix -P -f test.pcap  # CRASHES HERE
   ```

3. **Error messages** (full text):
   ```
   Paste complete error output
   ```

4. **Expected vs actual behavior**

5. **Sample data** (if possible):
   - Minimal PCAP file
   - Anonymized data

### Community Support

- **GitHub Issues**: https://github.com/montge/asterix/issues
- **GitHub Discussions**: https://github.com/montge/asterix/discussions
- **ASTERIX Protocol**: http://www.eurocontrol.int/services/asterix

---

## Quick Reference

### Common Error Codes

| Error Message | Cause | Solution |
|---------------|-------|----------|
| `libasterix.so.2: not found` | Missing shared library | Set LD_LIBRARY_PATH |
| `Cannot open file: config/asterix.ini` | Wrong working directory | Use absolute paths or run from install/ |
| `Wrong length in DataItem` | Invalid ASTERIX data | Check input format, verify file integrity |
| `ValueError: Data too large` | Data > 64KB | Split into chunks |
| `TypeError: a bytes-like object is required` | Wrong Python type | Convert to bytes: `data.encode()` or `bytes(data)` |
| `Data race detected` | Miri undefined behavior | Expected - library not thread-safe |

---

### Performance Checklist

- [ ] Use compact output formats (`-j`, `-l`)
- [ ] Disable verbose mode in production
- [ ] Enable CPU performance governor
- [ ] Use SSD for output files
- [ ] Process incrementally for large files
- [ ] Minimize memory allocations
- [ ] Use appropriate buffer sizes

---

### Security Checklist

- [ ] Validate all input data
- [ ] Use latest version (security patches)
- [ ] Run with least privileges
- [ ] Enable ASAN in development
- [ ] Run Valgrind tests
- [ ] Review FFI boundaries
- [ ] Monitor for memory leaks

---

## Additional Resources

- **Performance Benchmarks**: [BENCHMARK_RESULTS_2025-10-20.md](BENCHMARK_RESULTS_2025-10-20.md)
- **Performance Optimizations**: [PERFORMANCE_OPTIMIZATIONS.md](PERFORMANCE_OPTIMIZATIONS.md)
- **Security Audit**: [SECURITY_AUDIT_REPORT.md](SECURITY_AUDIT_REPORT.md)
- **Contributing Guide**: [CONTRIBUTING.md](CONTRIBUTING.md)
- **Build Instructions**: [BUILD.md](docs/BUILD.md)

---

## License

This troubleshooting guide is part of the ASTERIX project and is licensed under **GPL-3.0-or-later**.

See [LICENSE](LICENSE) for details.
