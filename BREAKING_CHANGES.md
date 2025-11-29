# Breaking Changes

This document tracks breaking changes in the ASTERIX project.

## Version 2.9.0 (Unreleased)

### Removed: Legacy Wireshark and Ethereal Plugins

**Impact:** Users relying on the bundled Wireshark/Ethereal plugins must migrate to alternatives.

#### What Was Removed

The following legacy plugin directories have been removed:

| Directory | Target Version | Original Date | Size |
|-----------|---------------|---------------|------|
| `src/asterix/ethereal-plugin/` | Ethereal v0.99.0 | 2006 | 296KB |
| `src/asterix/wireshark-plugin/1.8.4/` | Wireshark v1.8.4 | 2012 | ~350KB |
| `src/asterix/wireshark-plugin/1.10.6/` | Wireshark v1.10.6 | 2013 | ~400KB |

**Total removed:** ~1MB of legacy code

#### Why This Change Was Made

1. **Obsolete target versions:**
   - Ethereal was renamed to Wireshark in 2006 (18+ years ago)
   - Wireshark 1.8.4 and 1.10.6 are 11-12 years old and unsupported
   - Current Wireshark is v4.4.x with a completely different API

2. **Build system incompatibility:**
   - Legacy plugins don't compile on modern systems
   - Referenced MSVC 2008 (Visual Studio 2008) for Windows builds
   - Used obsolete autotools configurations

3. **Security concerns:**
   - Old Wireshark versions have known security vulnerabilities
   - No security updates available for these versions

4. **Maintenance burden:**
   - Code was unmaintained for 10+ years
   - No CI/CD integration
   - Documentation referenced obsolete paths and tools

#### Migration Options

**Option 1: Use Wireshark's Built-in ASTERIX Dissector (Recommended)**

Wireshark 3.0+ includes a built-in ASTERIX dissector that covers most use cases:

```bash
# Capture ASTERIX traffic
tshark -i eth0 -f "udp port 8600" -Y "asterix"

# Decode PCAP file
tshark -r capture.pcap -Y "asterix" -T fields -e asterix.category
```

**Option 2: Use the Standalone ASTERIX Parser**

The command-line parser provides full ASTERIX decoding with multiple output formats:

```bash
# Parse PCAP to JSON
./asterix -P -j -f capture.pcap

# Parse with human-readable JSON
./asterix -P -jh -f capture.pcap

# Parse to XML
./asterix -P -x -f capture.pcap
```

**Option 3: Use Language Bindings**

Parse ASTERIX data programmatically:

```python
# Python
import asterix
records = asterix.parse(data)
```

```javascript
// Node.js
const asterix = require('asterix-decoder');
const records = asterix.parse(data);
```

```rust
// Rust
use asterix::parse;
let records = parse(&data, Default::default())?;
```

#### Future Plans

A modern Wireshark 4.x plugin may be developed in the future. Track progress at:
- GitHub Issue #22: Modernize Wireshark plugin

---

## Reporting Issues

If this breaking change affects your workflow, please:
1. Open an issue at https://github.com/montge/asterix/issues
2. Describe your use case and requirements
3. We'll help identify the best migration path
