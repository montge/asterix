# Troubleshooting Guide

Solutions to common issues when using the ASTERIX decoder.

## Table of Contents

- [Build Issues](#build-issues)
- [Runtime Issues](#runtime-issues)
- [Parsing Issues](#parsing-issues)
- [Wireshark Plugin Issues](#wireshark-plugin-issues)
- [Python Module Issues](#python-module-issues)
- [Rust Crate Issues](#rust-crate-issues)
- [Performance Issues](#performance-issues)

---

## Build Issues

### CMake: "Could not find expat"

**Error:**
```
CMake Error: Could not find expat library
```

**Solution:**
```bash
# Ubuntu/Debian
sudo apt-get install libexpat1-dev

# Fedora/RHEL
sudo dnf install expat-devel

# macOS
brew install expat
```

### CMake: C++23 not supported

**Error:**
```
CMake Error: The compiler does not support C++23
```

**Solution:**
The code automatically falls back to C++17/20. If you need specific features:

```bash
# Install newer compiler
sudo apt-get install g++-13

# Or specify compiler
cmake -B build -DCMAKE_CXX_COMPILER=g++-13
```

### Windows: MSVC build fails

**Error:**
```
error C2039: 'format': is not a member of 'std'
```

**Solution:**
MSVC uses C++20 mode. The code includes fallbacks, but ensure you have Visual Studio 2019 v16.10+ or Visual Studio 2022.

### Linker: undefined reference to `expat`

**Error:**
```
undefined reference to `XML_ParserCreate'
```

**Solution:**
```bash
# Ensure expat is linked
cmake -B build -DEXPAT_LIBRARY=/usr/lib/x86_64-linux-gnu/libexpat.so
```

---

## Runtime Issues

### "Failed to open asterix.ini"

**Error:**
```
Failed to open /usr/share/asterix/config/asterix.ini
```

**Solutions:**

1. **Specify config path:**
   ```bash
   asterix -d /path/to/config -P -f file.pcap
   ```

2. **Install config files:**
   ```bash
   sudo cmake --install build
   # Or manually:
   sudo mkdir -p /usr/share/asterix/config
   sudo cp asterix/config/* /usr/share/asterix/config/
   ```

3. **Check config exists:**
   ```bash
   ls -la /usr/share/asterix/config/asterix.ini
   ```

### "error while loading shared libraries: libasterix.so"

**Error:**
```
./asterix: error while loading shared libraries: libasterix.so.2: cannot open shared object file
```

**Solutions:**

1. **Set LD_LIBRARY_PATH:**
   ```bash
   export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
   ./asterix -P -f file.pcap
   ```

2. **Update ldconfig:**
   ```bash
   sudo ldconfig
   ```

3. **Install to system path:**
   ```bash
   sudo cp install/lib/libasterix.so* /usr/local/lib/
   sudo ldconfig
   ```

### Multicast: "No data received"

**Problem:** No output when using multicast input.

**Solutions:**

1. **Check interface name:**
   ```bash
   ip addr show  # Find correct interface
   asterix -i 232.1.1.31:eth0:21131  # Use correct interface
   ```

2. **Check multicast routing:**
   ```bash
   # Add multicast route if needed
   sudo ip route add 224.0.0.0/4 dev eth0
   ```

3. **Check firewall:**
   ```bash
   sudo ufw allow in on eth0 to 232.1.1.31 port 21131
   ```

4. **Verify traffic exists:**
   ```bash
   tcpdump -i eth0 -n host 232.1.1.31 and port 21131
   ```

---

## Parsing Issues

### "Unknown category XX"

**Error:**
```
Unknown ASTERIX category: 99
```

**Solutions:**

1. **Check if category is supported:**
   ```bash
   ls /usr/share/asterix/config/asterix_cat*.xml
   ```

2. **Add category to asterix.ini:**
   ```bash
   echo "asterix_cat099_1_0.xml" >> /usr/share/asterix/config/asterix.ini
   ```

3. **Create custom category definition** following DTD format.

### Empty output / No records parsed

**Problem:** Parser runs but produces no output.

**Solutions:**

1. **Check input format flag:**
   ```bash
   # PCAP file needs -P
   asterix -P -f capture.pcap

   # ORADIS PCAP needs -R
   asterix -R -f oradis.pcap

   # Raw ASTERIX - no flag
   asterix -f data.asterix
   ```

2. **Verify file contains ASTERIX:**
   ```bash
   xxd capture.pcap | head -50
   # Look for ASTERIX category byte (e.g., 0x30 for CAT 48)
   ```

3. **Check file isn't empty:**
   ```bash
   ls -la capture.pcap
   file capture.pcap
   ```

### Malformed data errors

**Error:**
```
Parse error: Invalid block length at offset 1234
```

**Solutions:**

1. **Check data integrity:**
   ```bash
   # Verify PCAP is valid
   capinfos capture.pcap
   ```

2. **Try different input format:**
   ```bash
   # Maybe it's raw, not PCAP
   asterix -f file.bin
   ```

3. **Check for truncation:**
   - Ensure file transfer completed
   - Check for partial packets

### Incorrect field values

**Problem:** Parsed values don't match expected.

**Solutions:**

1. **Check category version:**
   - Different versions have different field definitions
   - Ensure XML matches data version

2. **Verify endianness:**
   - ASTERIX is big-endian
   - Check source system encoding

3. **Use extensive JSON for debugging:**
   ```bash
   asterix -P -f file.pcap -je | head -100
   ```

---

## Wireshark Plugin Issues

### Plugin not loading

**Problem:** `tshark -G plugins` doesn't show asterix.

**Solutions:**

1. **Check plugin path matches Wireshark version:**
   ```bash
   tshark --version | head -1
   # If 4.2.x, plugin goes in plugins/4.2/epan/
   ```

2. **Check library dependencies:**
   ```bash
   ldd ~/.local/lib/wireshark/plugins/4.2/epan/asterix.so
   # All libraries should resolve
   ```

3. **Install libasterix system-wide:**
   ```bash
   sudo cp install/lib/libasterix.so* /usr/local/lib/
   sudo ldconfig
   ```

### "Duplicate protocol short_name"

**Error:**
```
Dissector bug: Duplicate protocol short_name "ASTERIX"
```

**Solution:**
This means both built-in and our plugin are loading. Our plugin is now named `ASTERIX_EXT`:
- Filter name: `asterix_ext`
- Both can coexist

### Plugin loads but doesn't dissect

**Problem:** ASTERIX traffic shows as UDP, not ASTERIX.

**Solutions:**

1. **Check port (default 8600):**
   ```bash
   # If using different port, use decode-as
   tshark -r capture.pcap -d udp.port==22131,asterix_ext
   ```

2. **Verify config files installed:**
   ```bash
   ls /usr/share/asterix/config/asterix.ini
   ```

3. **Set config path in Wireshark preferences:**
   - Edit → Preferences → Protocols → ASTERIX_EXT
   - Set Configuration Directory

---

## Python Module Issues

### ImportError: No module named 'asterix'

**Solutions:**

1. **Install from PyPI:**
   ```bash
   pip install asterix-decoder
   ```

2. **Check Python version (3.10+ required):**
   ```bash
   python --version
   ```

3. **Check installation:**
   ```bash
   pip show asterix-decoder
   ```

### "Failed to initialize ASTERIX parser"

**Solutions:**

1. **Module auto-initializes, but check config:**
   ```python
   import asterix
   # If custom config needed:
   asterix.init("/path/to/custom_category.xml")
   ```

2. **Verify shared library:**
   ```bash
   python -c "import asterix; print(asterix.__file__)"
   ldd /path/to/_asterix.so
   ```

### Parse returns empty list

**Solutions:**

1. **Ensure data is bytes:**
   ```python
   # Correct
   records = asterix.parse(b'\x30\x00\x2a...')

   # Wrong - string
   records = asterix.parse('300002a...')
   ```

2. **Check data format:**
   ```python
   # Raw ASTERIX bytes expected, not PCAP
   # For PCAP, extract UDP payload first
   ```

---

## Rust Crate Issues

### Build fails: "cmake not found"

**Error:**
```
error: failed to run custom build command for `asterix-decoder`
CMAKE not found
```

**Solution:**
```bash
# Install cmake
sudo apt-get install cmake

# Or on macOS
brew install cmake
```

### Build fails: "expat not found"

**Solution:**
```bash
sudo apt-get install libexpat1-dev
```

### "Failed to initialize" at runtime

**Solutions:**

1. **Call init_default first:**
   ```rust
   use asterix::init_default;

   fn main() -> Result<(), asterix::AsterixError> {
       init_default()?;  // Must call before parsing
       // ...
   }
   ```

2. **Check config files are bundled:**
   - Build script should copy configs
   - Check `OUT_DIR` during build

---

## Performance Issues

### Slow parsing

**Solutions:**

1. **Use release build:**
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cargo build --release
   ```

2. **Use efficient output format:**
   ```bash
   # JSON is fastest for processing
   asterix -P -f file.pcap -j

   # Avoid pretty-printing for large files
   # -jh is slower than -j
   ```

3. **Filter unnecessary categories:**
   ```bash
   asterix -P -f file.pcap -LF filter.txt
   ```

4. **For streaming, use incremental parsing:**
   ```python
   offset = 0
   while offset < len(data):
       records, offset = asterix.parse_with_offset(data, offset, 1000)
   ```

### High memory usage

**Solutions:**

1. **Process in chunks:**
   - Use `parse_with_offset` for large files
   - Don't load entire file into memory

2. **Use streaming output:**
   ```bash
   asterix -P -f huge.pcap -j | process_line_by_line.py
   ```

3. **Filter early:**
   - Apply category filters to reduce data

### Multicast packet loss

**Solutions:**

1. **Increase socket buffer:**
   ```bash
   sudo sysctl -w net.core.rmem_max=26214400
   sudo sysctl -w net.core.rmem_default=26214400
   ```

2. **Use dedicated interface:**
   - Separate multicast traffic from other network load

3. **Check system load:**
   - High CPU can cause drops
   - Consider dedicated processing thread

---

## Getting Help

If your issue isn't covered here:

1. **Check existing issues:** https://github.com/montge/asterix/issues
2. **Search discussions:** https://github.com/montge/asterix/discussions
3. **File a bug report** with:
   - Operating system and version
   - ASTERIX version (`asterix --version`)
   - Steps to reproduce
   - Error message or unexpected behavior
   - Sample data (if possible)

## Related Documentation

- [Build Guide](BUILD.md)
- [C++ User Guide](guides/CPP_GUIDE.md)
- [Python User Guide](guides/PYTHON_GUIDE.md)
- [Rust User Guide](guides/RUST_GUIDE.md)
- [Wireshark Plugin Guide](guides/WIRESHARK_GUIDE.md)
