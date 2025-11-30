# Wireshark Plugin Status

## Current Status: Wireshark 4.x Plugin Available

**Date:** November 2025

### Wireshark 4.x Plugin

A modern Wireshark 4.x plugin is now available in `src/asterix/wireshark-plugin/4.x/`.

**Features:**
- Wireshark 4.x dissector API support
- Cross-platform builds (Linux, Windows, macOS)
- CMake-based plugin build system
- tshark CLI support
- Uses WiresharkWrapper API for full ASTERIX parsing

**Build and Install:**
```bash
# Build main ASTERIX library first
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install
cmake --build build && cmake --install build

# Build Wireshark plugin
cd src/asterix/wireshark-plugin/4.x
mkdir build && cd build
cmake .. && cmake --build .

# Install to user directory
cmake --install . --prefix ~/.local/lib/wireshark/plugins/4.x/epan/
```

See `src/asterix/wireshark-plugin/4.x/README.md` for detailed instructions.

---

## Legacy Plugins (Removed)

The following obsolete Wireshark/Ethereal plugins have been removed:

| Plugin | Version | Year | Status |
|--------|---------|------|--------|
| Ethereal | 0.99.0 | 2006 | Removed (Ethereal renamed to Wireshark in 2006) |
| Wireshark | 1.8.4 | 2012 | Removed |
| Wireshark | 1.10.6 | 2013 | Removed |

**Reason for removal:**
- These plugins are 11-18+ years old and incompatible with modern systems
- Legacy plugins don't compile on modern systems (glib API changes, deprecated functions)
- No support for Wireshark 4.x's modern dissector API
- Security vulnerabilities in old Wireshark versions

### Architecture

The `WiresharkWrapper.h` and `WiresharkWrapper.cpp` files in `src/asterix/` define a C-compatible FFI boundary used by the Wireshark plugin. Key APIs:

- `fulliautomatix_start()` - Initialize parser with config directory
- `fulliautomatix_get_definitions()` - Get protocol field definitions
- `fulliautomatix_parse()` - Parse ASTERIX data to protocol tree
- `fulliautomatix_data_destroy()` - Cleanup parsed data

### Alternatives

If you cannot use the Wireshark plugin, you can:

1. **Use Wireshark's built-in ASTERIX dissector** (Wireshark 3.0+)

2. **Export to JSON and post-process:**
   ```bash
   asterix -f capture.pcap -P -j > parsed.json
   ```

3. **Use language bindings:**
   - **Python:** `pip install asterix-decoder`
   - **Rust:** `cargo add asterix-decoder`
   - **Node.js:** `npm install asterix-decoder`
   - **Go:** See `asterix-go/` directory

## Related Issues

- [#22 - Modernize Wireshark plugin](https://github.com/montge/asterix/issues/22)
