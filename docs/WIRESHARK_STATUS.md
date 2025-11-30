# Wireshark Plugin Status

## Current Status: Legacy Plugins Deprecated

**Date:** November 2025

### Removed Legacy Code

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

### Current Architecture

The `WiresharkWrapper.h` and `WiresharkWrapper.cpp` files in `src/asterix/` define a C-compatible FFI boundary that can be used to build Wireshark plugins. These files are **retained** for future Wireshark 4.x plugin development.

### Future Plans

A modern Wireshark 4.x plugin is planned. See [GitHub Issue #22](https://github.com/montge/asterix/issues/22) for:

- Wireshark 4.x dissector API support
- Cross-platform builds (Linux, Windows, macOS)
- CMake-based plugin build system
- tshark CLI support
- CI/CD integration

### Migration Guide

**For users on legacy Wireshark versions:**

1. Upgrade to Wireshark 4.x (current stable)
2. Use the standalone ASTERIX decoder for parsing:
   ```bash
   # Parse ASTERIX from PCAP
   asterix -f capture.pcap -P -j
   ```
3. For programmatic access, use the language bindings:
   - **Python:** `pip install asterix-decoder`
   - **Rust:** `cargo add asterix-decoder`
   - **Node.js:** `npm install asterix-decoder`
   - **Go:** See `asterix-go/` directory

### Wireshark Alternatives

Until the Wireshark 4.x plugin is ready, you can:

1. **Export to JSON and post-process:**
   ```bash
   asterix -f capture.pcap -P -j > parsed.json
   ```

2. **Use tshark with raw export + ASTERIX decoder:**
   ```bash
   tshark -r capture.pcap -T fields -e data | asterix -R -j
   ```

3. **Use the Python bindings for scripted analysis:**
   ```python
   import asterix
   with open('capture.pcap', 'rb') as f:
       records = asterix.parse(f.read())
   ```

## Related Issues

- [#22 - Modernize Wireshark plugin](https://github.com/montge/asterix/issues/22)
