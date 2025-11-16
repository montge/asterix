# Wireshark ASTERIX Dissector Plugin

**Version:** 1.0.0
**Wireshark:** 4.0.0+
**Status:** Phase 1 MVP (Basic Dissection)

---

## Overview

This plugin adds ASTERIX protocol dissection support to Wireshark 4.x.

**ASTERIX** (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is a binary protocol used worldwide for Air Traffic Management (ATM) surveillance data exchange.

**Key Features:**
- ✅ Wireshark 4.0+ compatibility
- ✅ Heuristic dissection (auto-detect ASTERIX in UDP)
- ✅ Support for all 24 ASTERIX categories
- ✅ Data block parsing (category, length, FSPEC)
- ⏳ Data item parsing (Phase 2)
- ⏳ Color coding (Phase 4)
- ⏳ Statistics (Phase 4)

---

## Supported ASTERIX Categories

**24 Categories:**
- CAT001, CAT002, CAT004, CAT008, CAT010, CAT011, CAT015
- CAT019, CAT020, CAT021, CAT023, CAT025, CAT030, CAT031
- CAT032, CAT034, CAT048, CAT062, CAT063, CAT065, CAT205
- CAT240, CAT247, CAT252

---

## Installation

### Prerequisites

**Wireshark Development Packages:**

```bash
# Ubuntu/Debian
sudo apt-get install wireshark-dev libwireshark-dev libglib2.0-dev

# Fedora/RHEL
sudo dnf install wireshark-devel glib2-devel

# macOS
brew install wireshark pkg-config glib
```

**Build Tools:**
- CMake 3.12+
- GCC 7+ / Clang 5+ / MSVC 2019+
- GLib 2.50+

---

### Build from Source

```bash
# From repository root
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_WIRESHARK_PLUGIN=ON
cmake --build build

# The plugin will be built as:
# build/plugins/epan/asterix/asterix.so (Linux/macOS)
# build/plugins/epan/asterix/asterix.dll (Windows)
```

---

### Install Plugin

**Linux:**
```bash
mkdir -p ~/.local/lib/wireshark/plugins/4.0/epan
cp build/plugins/epan/asterix/asterix.so ~/.local/lib/wireshark/plugins/4.0/epan/
```

**macOS:**
```bash
mkdir -p ~/Library/Application\ Support/Wireshark/plugins/4.0/epan
cp build/plugins/epan/asterix/asterix.so ~/Library/Application\ Support/Wireshark/plugins/4.0/epan/
```

**Windows:**
```cmd
mkdir %APPDATA%\Wireshark\plugins\4.0\epan
copy build\plugins\epan\asterix\asterix.dll %APPDATA%\Wireshark\plugins\4.0\epan\
```

---

### Verify Installation

1. Launch Wireshark
2. Go to **Help → About Wireshark → Plugins**
3. Search for "asterix"
4. You should see:
   ```
   asterix  1.0.0  epan  ASTERIX Protocol Dissector
   ```

---

## Usage

### Capture ASTERIX Traffic

**Common ASTERIX UDP Ports:**
- **8600** (default)
- **21112** (multicast)

**Capture Filter:**
```
udp port 8600 or udp port 21112
```

**Display Filter:**
```
asterix
asterix.category == 48
asterix.category == 62
asterix.length > 100
```

---

### Display Filters

| Filter | Description |
|--------|-------------|
| `asterix` | Show all ASTERIX packets |
| `asterix.category == 48` | Show only CAT048 (Monoradar Target Reports) |
| `asterix.category == 62` | Show only CAT062 (System Track Data) |
| `asterix.length > 100` | Show data blocks larger than 100 bytes |

---

### Example: Parse PCAP with tshark

```bash
# Display ASTERIX packets in detail
tshark -r capture.pcap -Y "asterix" -V

# Count ASTERIX packets by category
tshark -r capture.pcap -Y "asterix" -T fields -e asterix.category | sort | uniq -c

# Export ASTERIX packets to JSON
tshark -r capture.pcap -Y "asterix" -T json > asterix.json

# Statistics
tshark -r capture.pcap -q -z "proto,colinfo,asterix,asterix.category"
```

---

## Protocol Hierarchy

```
ASTERIX CAT 048 - Monoradar Target Reports
    Data Block: 128 bytes
    Category: 48 (0x30)
    Length: 128 bytes
    Data Records: 3
    Data Record #1
        FSPEC: 0xf0 (I010, I020, I040, I070)
        FSPEC Byte 1: 0xf0 (FX=0, last byte)
        Data Record: [raw bytes]  ← Phase 2: will be parsed into items
```

---

## Current Limitations (Phase 1 MVP)

**⚠️ This is Phase 1 MVP:**
- ✅ Data block parsing (category, length)
- ✅ FSPEC parsing (field specification)
- ❌ Data item parsing (Phase 2)
- ❌ Value decoding (Phase 2)
- ❌ Color coding (Phase 4)
- ❌ Statistics (Phase 4)

**Workaround:** Use the standalone ASTERIX decoder for detailed parsing:
```bash
# Extract ASTERIX from PCAP and parse
tshark -r capture.pcap -Y "asterix" -T fields -e data | \
  xxd -r -p | \
  asterix -j
```

---

## Troubleshooting

### Plugin Not Loaded

**Check Plugin Directory:**
```bash
# Linux
ls -la ~/.local/lib/wireshark/plugins/4.0/epan/

# macOS
ls -la ~/Library/Application\ Support/Wireshark/plugins/4.0/epan/
```

**Check Wireshark Version:**
```bash
wireshark --version
# Must be 4.0.0 or later
```

**Check Plugin:**
```bash
# In Wireshark: Help → About Wireshark → Plugins
# Or via CLI:
tshark -G plugins | grep asterix
```

---

### No ASTERIX Packets Detected

**Enable Heuristic Dissection:**
1. Edit → Preferences → Protocols → ASTERIX
2. Check "Enable heuristic dissection"
3. Apply

**Check UDP Ports:**
- The plugin registers on ports 8600 and 21112
- For other ports, heuristic dissection must be enabled

**Verify Packet Format:**
- ASTERIX data blocks start with category (1 byte, 1-255)
- Followed by length (2 bytes, big-endian)
- Minimum data block size: 3 bytes

---

### Build Errors

**"Wireshark not found":**
```bash
# Install Wireshark development packages
sudo apt-get install wireshark-dev libwireshark-dev

# Or specify Wireshark manually:
cmake -B build -DWireshark_DIR=/usr/lib/x86_64-linux-gnu/cmake/wireshark
```

**"GLib not found":**
```bash
# Install GLib development packages
sudo apt-get install libglib2.0-dev

# Or specify GLib manually:
cmake -B build -DGLIB2_DIR=/usr/lib/x86_64-linux-gnu/pkgconfig
```

---

## Development

### Project Structure

```
plugins/epan/asterix/
├── CMakeLists.txt           # Build configuration
├── packet-asterix.c         # Main dissector implementation
├── packet-asterix.h         # Dissector headers
├── moduleinfo.h.in          # Plugin metadata template
└── README.md                # This file
```

---

### Building for Development

```bash
# Debug build with symbols
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_WIRESHARK_PLUGIN=ON
cmake --build build-debug

# Run Wireshark with debug plugin
wireshark -o "plugins.epan:${PWD}/build-debug/plugins/epan/asterix"
```

---

### Running Tests

```bash
# Test with sample PCAP
tshark -r install/share/asterix/samples/cat_062_065.pcap -Y "asterix" -V

# Check for memory leaks
valgrind tshark -r sample.pcap -Y "asterix" > /dev/null
```

---

## Roadmap

### Phase 1: MVP (Current)
- [x] Basic data block dissection
- [x] FSPEC parsing
- [x] Heuristic dissection
- [x] CMake build system

### Phase 2: Data Items (Next)
- [ ] Parse all CAT048 data items
- [ ] Integrate with ASTERIX core library
- [ ] Value decoding and display
- [ ] Filtering by data item

### Phase 3: Multi-Category
- [ ] Support all 24 categories
- [ ] Auto-generate category definitions
- [ ] Expert info for errors
- [ ] Category-specific preferences

### Phase 4: Polish
- [ ] Color coding by category
- [ ] Statistics integration
- [ ] Windows/macOS builds
- [ ] Publish to Wireshark plugin repository

---

## Contributing

See [CONTRIBUTING.md](../../../CONTRIBUTING.md) for development guidelines.

**Design Document:** [docs/WIRESHARK_PLUGIN_DESIGN.md](../../../docs/WIRESHARK_PLUGIN_DESIGN.md)

---

## License

GPL-3.0-or-later (same as Wireshark)

See [LICENSE](../../../LICENSE) for details.

---

## References

- **Wireshark Developer's Guide:** https://www.wireshark.org/docs/wsdg_html_chunked/
- **ASTERIX Protocol:** http://www.eurocontrol.int/services/asterix
- **ASTERIX Specifications:** https://zoranbosnjak.github.io/asterix-specs/
- **Issue #22:** https://github.com/montge/asterix/issues/22

---

## Acknowledgments

Based on the ASTERIX decoder by Croatia Control Ltd.

Replaces legacy Wireshark 1.8.4/1.10.6 and Ethereal 0.99.0 plugins (removed November 2025).
