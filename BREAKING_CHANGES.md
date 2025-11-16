# Breaking Changes

This document tracks breaking changes and deprecations in the ASTERIX project.

---

## v2.8.11+ (November 2025) - Wireshark/Ethereal Plugin Deprecation

### Breaking Change: Legacy Wireshark Plugins Removed

**Effective Date:** November 2025
**Issue:** [#22](https://github.com/montge/asterix/issues/22)

### Summary

All legacy Wireshark and Ethereal plugin implementations have been **permanently removed** from the codebase. These outdated plugins are no longer buildable on modern systems and have been superseded by a future Wireshark 4.x-compatible dissector.

### Removed Components

The following directories and their contents (totaling ~850KB) have been deleted:

1. **`src/asterix/ethereal-plugin/`** - Ethereal v0.99.0 plugin (circa 2006)
   - Based on obsolete Ethereal (predecessor to Wireshark)
   - Required RPM-based build on ancient Linux distributions
   - **Last compatible version:** Ethereal 0.99.0 (released 2006)

2. **`src/asterix/wireshark-plugin/1.8.4/`** - Wireshark v1.8.4 plugin (circa 2012)
   - Required Microsoft Visual Studio 2008 Express Edition on Windows
   - Used deprecated nmake/configure.in build system
   - **Last compatible version:** Wireshark 1.8.x (EOL 2015)

3. **`src/asterix/wireshark-plugin/1.10.6/`** - Wireshark v1.10.6 plugin (circa 2013)
   - Similar constraints to 1.8.4 with minor API updates
   - **Last compatible version:** Wireshark 1.10.x (EOL 2016)

### Why This Change Was Necessary

1. **Build Failures:** Legacy plugins do not compile with modern Wireshark 4.x API
2. **Security:** Old Wireshark versions contain known vulnerabilities (CVE database)
3. **Maintenance Burden:** ~1MB of code that cannot be tested or maintained
4. **Platform Support:** Relied on obsolete tools (MSVC 2008, autotools, RPM)
5. **Documentation Debt:** Misleading users about plugin availability

### Migration Path

#### For Users Currently Using Legacy Plugins

**Option 1: Use C++ Executable Directly**
```bash
# Install the standalone ASTERIX decoder
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build

# Parse PCAP files
./install/bin/asterix -f capture.pcap -j > output.json
```

#### Option 2: Wait for Wireshark 4.x Plugin (Future Release)

A modern Wireshark 4.x-compatible dissector is planned for development:
- **Target Wireshark version:** 4.0.0+
- **Build system:** CMake (cross-platform)
- **Platforms:** Linux, Windows (MSVC 2019+), macOS
- **Status:** Design phase (see issue #22 for progress)

**Estimated timeline:** Q1-Q2 2026

#### Option 3: Extract ASTERIX from PCAP, Then Analyze

```bash
# Extract UDP payloads containing ASTERIX data
tshark -r capture.pcap -Y "udp.port == 8600" -T fields -e data > hex_dump.txt

# Parse with ASTERIX decoder
./install/bin/asterix -R < hex_dump.txt
```

### Impact Assessment

**Who is affected:**
- Users running Wireshark < 4.0 with ASTERIX plugins installed
- Build scripts referencing `src/asterix/ethereal-plugin` or `src/asterix/wireshark-plugin`
- Documentation mentioning Wireshark plugin builds

**Who is NOT affected:**
- Users of the C++ standalone executable
- Users of Python module (`pip install asterix_decoder`)
- Users of Rust crate (`cargo add asterix-decoder`)
- Users parsing ASTERIX from files/stdin/multicast without Wireshark

### Workarounds for Wireshark Users

**Temporary Solution - Text Extraction:**
```bash
# Step 1: Extract ASTERIX data from PCAP using tshark
tshark -r input.pcap -Y "udp" -T fields \
  -e frame.time_epoch -e data.data > extracted.txt

# Step 2: Parse with ASTERIX decoder
asterix -f extracted.txt --format raw
```

**Long-term Solution:**
- Monitor issue #22 for Wireshark 4.x plugin development
- Consider contributing to plugin development (see CONTRIBUTING.md)
- Use standalone decoder for batch processing + Wireshark for network capture

### References

- **Issue #22:** [Modernize Wireshark plugin: deprecate legacy versions and add support for Wireshark 4.x](https://github.com/montge/asterix/issues/22)
- **CONTRIBUTING.md:** Guidelines for contributing to plugin development
- **BUILD_COMMAND_REFERENCE.txt:** Instructions for building standalone executable

### Questions?

If you have questions about this deprecation or need assistance migrating:
1. Check the [GitHub Discussions](https://github.com/montge/asterix/discussions)
2. Open an issue tagged with `wireshark` and `migration`
3. Review the standalone executable documentation in README.md

---

## Future Breaking Changes

Future breaking changes will be documented here with advance notice where possible.

For upcoming changes, see:
- **Open Issues:** https://github.com/montge/asterix/issues
- **Roadmap:** Check project milestones for planned deprecations
