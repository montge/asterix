# C++ User Guide

Complete guide to using the ASTERIX decoder command-line tool and C++ library for high-performance surveillance data processing.

## Overview

The ASTERIX C++ implementation provides:
- **Command-line tool** (`asterix`) - Process files, streams, and multicast
- **Shared library** (`libasterix.so`) - Embed in your applications
- **Static library** (`libasterix.a`) - Link statically

**C++ Standard:** C++23 (Linux/macOS), C++20 (Windows/MSVC)

## Installation

### From Package Manager

**Ubuntu/Debian:**
```bash
sudo apt-get install asterix
```

**From Source:**
```bash
git clone https://github.com/montge/asterix.git
cd asterix

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local

# Build
cmake --build build --parallel

# Install
sudo cmake --install build
```

## Command-Line Tool

### Basic Usage

```bash
# Parse PCAP file, output as text
asterix -P -f capture.pcap

# Parse raw ASTERIX file, output as JSON
asterix -f data.asterix -j

# Parse from stdin
cat data.asterix | asterix -j
```

### Input Sources

| Flag | Description | Example |
|------|-------------|---------|
| `-f <file>` | Read from file | `asterix -f capture.pcap` |
| (none) | Read from stdin | `cat data.asterix \| asterix` |
| `-i m:i:p[:s]` | UDP multicast | `asterix -i 232.1.1.31:eth0:21131` |

### Input Formats

| Flag | Format | Description |
|------|--------|-------------|
| `-P` | PCAP | Standard PCAP encapsulation |
| `-R` | ORADIS PCAP | ORADIS variant |
| `-F` | FINAL | FINAL packet format |
| `-H` | HDLC | HDLC framed data |
| `-G` | GPS | GPS packet format |
| (none) | Raw | Raw ASTERIX binary |

### Output Formats

| Flag | Format | Description |
|------|--------|-------------|
| (none) | Text | Human-readable, multi-line |
| `-l` | Line | One line per item (parsable) |
| `-j` | JSON | Compact JSON, one object per line |
| `-jh` | JSON Human | Pretty-printed JSON |
| `-je` | JSON Extensive | JSON with descriptions |
| `-x` | XML | Compact XML |
| `-xh` | XML Human | Pretty-printed XML |

### Filtering

```bash
# Use filter file
asterix -P -f capture.pcap -LF filter.txt

# Show available filters
asterix -L
```

**Filter file format:**
```
# Include only specific categories
+CAT048
+CAT062

# Exclude categories
-CAT034

# Include specific items
+CAT048.I010
+CAT048.I140
```

### Configuration

```bash
# Specify config directory
asterix -d /path/to/config -P -f capture.pcap

# Default config location
asterix -d /usr/share/asterix/config -P -f capture.pcap
```

## Examples

### Parse PCAP to JSON

```bash
asterix -P -f radar_data.pcap -j > output.json
```

### Extract Category 48 Only

```bash
# Create filter
echo "+CAT048" > cat48_filter.txt

# Apply filter
asterix -P -f capture.pcap -LF cat48_filter.txt -j
```

### Real-time Multicast Processing

```bash
# Listen on multicast group 232.1.1.31, port 21131, interface eth0
asterix -i 232.1.1.31:eth0:21131 -j

# With source filter (SSM)
asterix -i 232.1.1.31:eth0:21131:10.0.0.1 -j
```

### Pipe to Processing Script

```bash
# Parse and process with jq
asterix -P -f capture.pcap -j | jq '.CAT048.I070.MODE3A'

# Count records per category
asterix -P -f capture.pcap -j | jq -r '.cat' | sort | uniq -c
```

### Batch Processing

```bash
#!/bin/bash
# Process all PCAP files in directory
for f in *.pcap; do
    echo "Processing $f..."
    asterix -P -f "$f" -j > "${f%.pcap}.json"
done
```

## Library Integration

### CMake Integration

```cmake
# Find the ASTERIX library
find_package(asterix REQUIRED)

# Link to your target
target_link_libraries(myapp PRIVATE asterix::asterix)
```

Or manually:

```cmake
# Find library
find_library(ASTERIX_LIB asterix PATHS /usr/local/lib)
find_path(ASTERIX_INCLUDE asterix PATHS /usr/local/include)

# Link
target_include_directories(myapp PRIVATE ${ASTERIX_INCLUDE})
target_link_libraries(myapp PRIVATE ${ASTERIX_LIB})
```

### Basic C++ Usage

```cpp
#include <asterix/AsterixDefinition.h>
#include <asterix/InputParser.h>
#include <asterix/XMLParser.h>
#include <fstream>
#include <vector>

int main() {
    // Initialize definitions
    AsterixDefinition* def = new AsterixDefinition();

    // Load category definitions
    XMLParser parser;
    std::ifstream catFile("/usr/share/asterix/config/asterix_cat048_1_30.xml");
    parser.Parse(catFile, def, "asterix_cat048_1_30.xml");
    catFile.close();

    // Parse ASTERIX data
    InputParser inputParser(def);

    std::vector<unsigned char> data = /* your ASTERIX bytes */;
    AsterixData* result = inputParser.parsePacket(data.data(), data.size());

    if (result) {
        // Process parsed data
        // ...
        delete result;
    }

    delete def;
    return 0;
}
```

### Using the Wireshark Wrapper API

For simpler integration, use the Wireshark wrapper API:

```cpp
#include <asterix/WiresharkWrapper.h>

int main() {
    // Initialize
    fulliautomatix_start(nullptr, "/usr/share/asterix/config");

    // Get definitions (optional)
    fulliautomatix_definitions* defs = fulliautomatix_get_definitions();

    // Parse data
    unsigned char data[] = {0x30, 0x00, 0x2A, /* ... */};
    fulliautomatix_data* result = fulliautomatix_parse(data, sizeof(data));

    if (result) {
        // Process result
        fulliautomatix_data* current = result;
        while (current) {
            if (current->description) {
                printf("%s\n", current->description);
            }
            current = current->next;
        }

        // Free result
        fulliautomatix_data_destroy(result);
    }

    // Cleanup definitions
    fulliautomatix_destroy_definitions(defs);

    return 0;
}
```

### Data Structures

```cpp
// Wireshark wrapper data structure
struct fulliautomatix_data {
    int tree;                    // 1=tree node, 0=leaf, -1=tree end
    int pid;                     // Protocol ID
    int bytenr;                  // Byte offset
    int length;                  // Data length
    int type;                    // Field type (FA_FT_*)
    int err;                     // Error level (0=ok, 1=warn, 2=error)
    char* description;           // Field description
    char* value_description;     // Value meaning
    union {
        unsigned long ul;        // Unsigned value
        signed long sl;          // Signed value
        char* str;               // String value
    } val;
    fulliautomatix_data* next;   // Next item in list
};

// Field types
enum {
    FA_FT_NONE = 0,
    FA_FT_UINT8, FA_FT_UINT16, FA_FT_UINT24, FA_FT_UINT32,
    FA_FT_INT8, FA_FT_INT16, FA_FT_INT24, FA_FT_INT32,
    FA_FT_STRING, FA_FT_STRINGZ,
    FA_FT_BYTES
};
```

## Configuration Files

### asterix.ini

Lists all category XML files to load:

```ini
asterix_cat001_1_4.xml
asterix_cat002_1_1.xml
asterix_cat048_1_30.xml
asterix_cat062_1_18.xml
# ... more categories
```

### Category XML Format

Categories are defined in XML following the DTD at `asterix/config/asterix.dtd`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Monoradar Target Reports" ver="1.30">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Data Source Identifier</DataItemName>
        <DataItemDefinition>
            Identification of the radar station
        </DataItemDefinition>
        <DataItemFormat desc="Two-byte fixed length">
            <Fixed length="2">
                <Bits from="16" to="9">
                    <BitsShortName>SAC</BitsShortName>
                    <BitsName>System Area Code</BitsName>
                </Bits>
                <Bits from="8" to="1">
                    <BitsShortName>SIC</BitsShortName>
                    <BitsName>System Identification Code</BitsName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <!-- More items... -->
</Category>
```

## Performance Optimization

The C++ implementation has been optimized for high throughput:

### Optimizations Applied
1. **String concatenation elimination** - 15% speedup
2. **String reserve() optimization** - 15% speedup
3. **Hex string loop reserve()** - 8% speedup
4. **PCAP buffer reuse** - 15-20% speedup
5. **UDP fd_set caching** - 2-3% speedup

**Total: 55-61% cumulative speedup**

### Performance Tips

1. **Use JSON output for parsing** - Most efficient for downstream processing
2. **Filter early** - Use `-LF` to reduce processing
3. **Batch files** - Process multiple files in sequence
4. **Use multicast directly** - Avoid PCAP overhead for live data

### Benchmarks

Typical performance on modern hardware:

| File Size | Format | Time | Throughput |
|-----------|--------|------|------------|
| 1 MB | PCAP→JSON | ~50ms | ~20 MB/s |
| 10 MB | PCAP→JSON | ~400ms | ~25 MB/s |
| 100 MB | PCAP→Text | ~3s | ~33 MB/s |

## Supported Categories

| Category | Description | Version |
|----------|-------------|---------|
| CAT 001 | Monoradar Target Reports | v1.4 |
| CAT 002 | Monoradar Service Messages | v1.1 |
| CAT 008 | Monoradar Target Reports (Enhanced) | v1.2 |
| CAT 021 | ADS-B Target Reports | v2.6 |
| CAT 023 | CNS/ATM Ground Station Service | v1.3 |
| CAT 030 | Server Status Messages | v7.0 |
| CAT 031 | Server Status Messages | v7.0 |
| CAT 034 | Monoradar Service Messages | v1.29 |
| CAT 048 | Monoradar Target Reports | v1.30 |
| CAT 062 | SDPS Track Messages | v1.18 |
| CAT 063 | Sensor Status Messages | v1.6 |
| CAT 065 | SDPS Service Status Messages | v1.5 |
| CAT 252 | Service Messages | v7.0 |

## Error Handling

### Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Configuration error |
| 2 | Input file error |
| 3 | Parse error |

### Common Errors

**"Failed to open asterix.ini"**
- Set correct config path with `-d`
- Default: `/usr/share/asterix/config`

**"Unknown category XX"**
- Category not defined in loaded XML files
- Add category XML to asterix.ini

**"Parse error at offset XX"**
- Malformed ASTERIX data
- Check input format flag (-P, -R, etc.)

## Building from Source

### Requirements

- **CMake** 3.20+
- **C++ Compiler**: GCC 13+, Clang 16+, MSVC 2019+
- **libexpat** (XML parsing)

### Build Commands

```bash
# Debug build
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug

# Release build with install
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build --parallel
sudo cmake --install build

# Build only shared library
cmake -B build -DBUILD_SHARED_LIBS=ON -DBUILD_EXECUTABLE=OFF
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Build type |
| `BUILD_SHARED_LIBS` | ON | Build shared library |
| `BUILD_STATIC_LIBS` | ON | Build static library |
| `BUILD_EXECUTABLE` | ON | Build CLI tool |

## Related Documentation

- [Build Guide](../BUILD.md)
- [Architecture Overview](../ARCHITECTURE.md)
- [Performance Optimizations](https://github.com/montge/asterix/blob/master/PERFORMANCE_OPTIMIZATIONS.md)
- [Wireshark Plugin Guide](WIRESHARK_GUIDE.md)

## Support

- **Issues**: https://github.com/montge/asterix/issues
- **Source**: `src/asterix/` and `src/main/` directories
