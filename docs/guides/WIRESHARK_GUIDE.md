# Wireshark Plugin User Guide

This guide covers installation and usage of the ASTERIX Extended (ASTERIX_EXT) Wireshark plugin for dissecting ASTERIX protocol traffic.

## Overview

The ASTERIX_EXT plugin provides enhanced ASTERIX dissection capabilities for Wireshark 4.x, including:

- **Additional categories**: CAT 030, 031, 252, BDS (not in built-in dissector)
- **Newer specifications**: CAT 021 v2.6, CAT 034 v1.29
- **XML-based definitions**: Easy to update for new ASTERIX versions
- **Custom category support**: Load your own category definitions

> **Note:** This plugin registers as `asterix_ext` to coexist with Wireshark's built-in ASTERIX dissector.

## Requirements

- **Wireshark 4.0+** (4.2+ recommended)
- **libasterix** - ASTERIX parser library (from this project)
- **Operating Systems**: Linux, macOS, Windows

## Installation

### 1. Build the ASTERIX Library

First, build and install the ASTERIX library:

```bash
# Clone repository
git clone https://github.com/montge/asterix.git
cd asterix

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install
cmake --build build --parallel
cmake --install build
```

### 2. Install the Library System-Wide

**Linux:**
```bash
sudo cp install/lib/libasterix.so* /usr/local/lib/
sudo ldconfig
```

**macOS:**
```bash
sudo cp install/lib/libasterix.dylib /usr/local/lib/
```

**Windows:**
```powershell
copy install\bin\asterix.dll C:\Windows\System32\
# Or copy alongside the plugin
```

### 3. Build the Wireshark Plugin

```bash
cd src/asterix/wireshark-plugin/4.x
mkdir build && cd build

# Linux
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DASTERIX_LIBRARY=/usr/local/lib/libasterix.so \
  -DASTERIX_INCLUDE_DIR=../../install/include/asterix

# macOS
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DASTERIX_LIBRARY=/usr/local/lib/libasterix.dylib

cmake --build .
```

### 4. Install the Plugin

**Linux:**
```bash
# User installation (recommended)
mkdir -p ~/.local/lib/wireshark/plugins/4.2/epan/
cp asterix.so ~/.local/lib/wireshark/plugins/4.2/epan/

# System-wide installation
sudo cp asterix.so /usr/lib/x86_64-linux-gnu/wireshark/plugins/4.2/epan/
```

**macOS:**
```bash
mkdir -p ~/Library/Application\ Support/Wireshark/plugins/4.2/epan/
cp asterix.so ~/Library/Application\ Support/Wireshark/plugins/4.2/epan/
```

**Windows:**
```powershell
mkdir %APPDATA%\Wireshark\plugins\4.2\epan\
copy asterix.dll %APPDATA%\Wireshark\plugins\4.2\epan\
```

> **Note:** Replace `4.2` with your Wireshark version (check with `tshark --version`).

### 5. Install ASTERIX Category Definitions

```bash
sudo mkdir -p /usr/share/asterix/config/
sudo cp -r install/share/asterix/config/* /usr/share/asterix/config/
```

## Verification

Check that the plugin loads correctly:

```bash
# List registered plugins
tshark -G plugins | grep asterix

# Expected output:
# asterix.so    2.9.0    dissector    /home/user/.local/lib/wireshark/plugins/4.2/epan/asterix.so

# List registered protocols
tshark -G protocols | grep -i asterix

# Expected output:
# ASTERIX packet    ASTERIX    asterix    T    T    T
# ASTERIX Extended  ASTERIX_EXT asterix_ext    T    T    T
```

## Usage

### Basic Capture

1. Open Wireshark
2. Start capturing on the interface receiving ASTERIX traffic
3. ASTERIX packets on port 8600 (default) will be automatically dissected

### Display Filters

```
asterix_ext                      # All ASTERIX packets (this plugin)
asterix_ext.category == 48       # Category 48 only
asterix_ext.category == 62       # Category 62 only
asterix_ext.length > 100         # Packets larger than 100 bytes
```

### Decode Non-Standard Ports

If ASTERIX traffic uses a different port:

**GUI:**
1. Right-click on a packet
2. Select "Decode As..."
3. Set UDP port → ASTERIX_EXT

**CLI (tshark):**
```bash
# Decode port 22131 as ASTERIX_EXT
tshark -r capture.pcap -d udp.port==22131,asterix_ext

# Multiple ports
tshark -r capture.pcap -d udp.port==22131,asterix_ext -d udp.port==22112,asterix_ext
```

### JSON Export

Export ASTERIX fields to JSON:

```bash
# Export specific fields
tshark -r capture.pcap -d udp.port==22131,asterix_ext \
  -T json -e asterix_ext.category -e asterix_ext.length

# Export all fields
tshark -r capture.pcap -d udp.port==22131,asterix_ext -T json
```

### Statistics

```bash
# Show ASTERIX packet count
tshark -r capture.pcap -q -z io,stat,0,asterix_ext

# Count by category
tshark -r capture.pcap -Y "asterix_ext" -T fields -e asterix_ext.category | sort | uniq -c
```

## Configuration

### Preferences

In Wireshark: **Edit → Preferences → Protocols → ASTERIX_EXT**

| Setting | Default | Description |
|---------|---------|-------------|
| UDP Port | 8600 | Default port for ASTERIX traffic |
| Config Directory | /usr/share/asterix/config | Path to XML category definitions |

### Custom Category Definitions

To add custom ASTERIX categories:

1. Create XML definition following the DTD at `asterix/config/asterix.dtd`
2. Add the filename to `asterix.ini`
3. Place files in the config directory
4. Restart Wireshark

## Comparison: Built-in vs ASTERIX_EXT

| Feature | Built-in (asterix) | Extended (asterix_ext) |
|---------|-------------------|------------------------|
| **Categories** | Standard set | + CAT 030, 031, 252, BDS |
| **Spec Versions** | Older | CAT 021 v2.6, CAT 034 v1.29 |
| **Customization** | Fixed | XML-based, user-configurable |
| **Filter Name** | `asterix` | `asterix_ext` |

## Troubleshooting

### Plugin Not Loading

**Symptom:** Plugin doesn't appear in `tshark -G plugins`

**Solutions:**
1. Check Wireshark version matches plugin path
   ```bash
   tshark --version | head -1
   # Make sure plugin is in correct version directory (4.2, 4.4, etc.)
   ```

2. Check library dependencies
   ```bash
   ldd ~/.local/lib/wireshark/plugins/4.2/epan/asterix.so
   # All libraries should resolve (no "not found")
   ```

3. Verify libasterix is installed
   ```bash
   ldconfig -p | grep asterix
   # Should show: libasterix.so.2 => /usr/local/lib/libasterix.so.2
   ```

### Parse Errors

**Symptom:** Packets show as malformed or unparsed

**Solutions:**
1. Verify config directory is set correctly in preferences
2. Check that XML category definitions exist:
   ```bash
   ls /usr/share/asterix/config/*.xml
   ```
3. Ensure asterix.ini lists all category files

### No ASTERIX Dissection

**Symptom:** Packets show as UDP, not ASTERIX

**Solutions:**
1. Check if traffic is on default port (8600)
2. Use "Decode As" for non-standard ports
3. Verify plugin is actually loaded:
   ```bash
   tshark -G protocols | grep asterix_ext
   ```

## Performance Tips

For high-volume ASTERIX traffic:

1. **Use tshark** for bulk processing:
   ```bash
   tshark -r large_capture.pcap -Y "asterix_ext" -w filtered.pcap
   ```

2. **Disable name resolution**:
   ```bash
   tshark -n -r capture.pcap
   ```

3. **Filter during capture**:
   ```bash
   tshark -i eth0 -f "udp port 8600" -w capture.pcap
   ```

4. **Use display filters** to limit decoded packets:
   ```bash
   tshark -r capture.pcap -Y "asterix_ext.category == 48"
   ```

## Example Commands

```bash
# View first 10 ASTERIX packets
tshark -r capture.pcap -c 10 -d udp.port==22131,asterix_ext

# Count packets by category
tshark -r capture.pcap -d udp.port==22131,asterix_ext \
  -T fields -e asterix_ext.category | sort | uniq -c | sort -rn

# Export Category 48 to JSON
tshark -r capture.pcap -d udp.port==22131,asterix_ext \
  -Y "asterix_ext.category == 48" -T json > cat48.json

# Show packet timestamps and categories
tshark -r capture.pcap -d udp.port==22131,asterix_ext \
  -T fields -e frame.time -e asterix_ext.category

# Live capture with decode
tshark -i eth0 -f "udp port 22131" -d udp.port==22131,asterix_ext
```

## Related Documentation

- [Building ASTERIX](../BUILD.md)
- [ASTERIX Architecture](../ARCHITECTURE.md)
- [Wireshark Plugin Development](https://www.wireshark.org/docs/wsdg_html_chunked/ChapterDissection.html)
- [EUROCONTROL ASTERIX](https://www.eurocontrol.int/asterix)

## Support

- **Issues**: [GitHub Issues](https://github.com/montge/asterix/issues)
- **Plugin Source**: `src/asterix/wireshark-plugin/4.x/`
- **CI Status**: See `.github/workflows/wireshark-ci.yml`
