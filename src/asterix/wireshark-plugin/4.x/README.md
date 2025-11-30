# ASTERIX Wireshark 4.x Plugin

Protocol dissector for ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) data in Wireshark 4.x.

## Requirements

- **Wireshark 4.0+** (development headers required for building)
- **CMake 3.20+**
- **GLib 2.50+**
- **ASTERIX library** (libasterix from the parent project)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt install wireshark-dev libglib2.0-dev cmake build-essential
```

**Fedora/RHEL:**
```bash
sudo dnf install wireshark-devel glib2-devel cmake gcc
```

**macOS:**
```bash
brew install wireshark cmake glib
```

## Building

First, build the main ASTERIX library from the repository root:

```bash
# From repository root
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install
cmake --build build --parallel
cmake --install build
```

Then build the Wireshark plugin:

```bash
cd src/asterix/wireshark-plugin/4.x
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Installation

### Prerequisites

The plugin requires the ASTERIX shared library (`libasterix.so`) to be installed in a location where the dynamic linker can find it:

**Linux:**
```bash
# Install ASTERIX library system-wide
sudo cp install/lib/libasterix.so* /usr/local/lib/
sudo ldconfig
```

**macOS:**
```bash
# Install ASTERIX library
sudo cp install/lib/libasterix.dylib /usr/local/lib/
```

**Windows:**
```powershell
# Copy DLL alongside plugin or to system path
copy install\bin\asterix.dll %APPDATA%\Wireshark\plugins\4.x\epan\
```

### User Installation (Recommended)

```bash
cmake --install . --prefix ~/.local/lib/wireshark/plugins/4.x/epan/
```

### System-Wide Installation

```bash
sudo cmake --install . --prefix /usr/lib/x86_64-linux-gnu/wireshark/plugins/4.x/epan/
```

### Manual Installation

Copy the built plugin to your Wireshark plugins directory:

**Linux:**
```bash
mkdir -p ~/.local/lib/wireshark/plugins/4.x/epan/
cp asterix.so ~/.local/lib/wireshark/plugins/4.x/epan/
```

**macOS:**
```bash
mkdir -p ~/Library/Application\ Support/Wireshark/plugins/4.x/epan/
cp asterix.so ~/Library/Application\ Support/Wireshark/plugins/4.x/epan/
```

**Windows:**
```powershell
mkdir %APPDATA%\Wireshark\plugins\4.x\epan\
copy asterix.dll %APPDATA%\Wireshark\plugins\4.x\epan\
```

## Configuration

### ASTERIX Category Definitions

The plugin requires XML category definitions. By default, it looks in:
- `/usr/share/asterix/config/`
- Custom path via Wireshark preferences

To configure in Wireshark:
1. Go to **Edit > Preferences > Protocols > ASTERIX**
2. Set **Configuration Directory** to your ASTERIX config path
3. Set **UDP Port** if not using default (8600)

### Default UDP Port

The plugin listens on UDP port **8600** by default. Change this in:
- Wireshark Preferences: **Edit > Preferences > Protocols > ASTERIX > UDP Port**

## Usage

### GUI (Wireshark)

1. Start Wireshark
2. Capture traffic on the interface receiving ASTERIX data
3. Or open a PCAP file containing ASTERIX traffic
4. Filter with: `asterix`

### CLI (tshark)

```bash
# Display ASTERIX packets
tshark -r capture.pcap -Y "asterix"

# Export specific fields
tshark -r capture.pcap -T fields -e asterix.category -e asterix.length

# Show ASTERIX statistics
tshark -r capture.pcap -q -z io,stat,0,asterix
```

### Display Filters

```
asterix                    # All ASTERIX packets
asterix.category == 48     # Category 48 only
asterix.category == 62     # Category 62 only
```

## Supported Categories

The plugin supports all ASTERIX categories defined in the XML configuration files:
- CAT 001 - Monoradar Target Reports
- CAT 002 - Monoradar Service Messages
- CAT 008 - Monoradar Target Reports (Enhanced)
- CAT 021 - ADS-B Target Reports
- CAT 023 - CNS/ATM Ground Station Service Messages
- CAT 034 - Monoradar Service Messages
- CAT 048 - Monoradar Target Reports
- CAT 062 - SDPS Track Messages
- CAT 063 - Sensor Status Messages
- CAT 065 - SDPS Service Status Messages
- And more...

## Troubleshooting

### Plugin Not Loading

1. Check Wireshark version: `wireshark --version`
2. Verify plugin path matches version (e.g., `4.4` for Wireshark 4.4.x)
3. Check plugin loads: `tshark -G plugins | grep asterix`

### Parse Errors

1. Verify ASTERIX config directory is set correctly
2. Check that XML category definitions exist
3. Enable debug logging in Wireshark

### Performance Issues

For high-volume ASTERIX traffic:
1. Use display filters to limit what's decoded
2. Consider using tshark for bulk processing
3. Use `-n` flag to disable name resolution

## Building on Windows

Use Visual Studio 2019/2022:

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## License

GPL-3.0-or-later - See LICENSE file in repository root.

## Related

- [Main ASTERIX Project](https://github.com/montge/asterix)
- [Wireshark Plugin Development](https://www.wireshark.org/docs/wsdg_html_chunked/ChapterDissection.html)
- [EUROCONTROL ASTERIX](https://www.eurocontrol.int/asterix)
