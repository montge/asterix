# ASTERIX C++ Executable User Guide

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://isocpp.org/std/the-standard)
[![GCC 13+](https://img.shields.io/badge/GCC-13%2B-brightgreen.svg)](https://gcc.gnu.org/)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](../LICENSE)

Comprehensive guide for using the ASTERIX command-line decoder tool for parsing Air Traffic Management surveillance data.

---

## Table of Contents

- [Overview](#overview)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Command-Line Reference](#command-line-reference)
- [Input Formats](#input-formats)
- [Output Formats](#output-formats)
- [Data Sources](#data-sources)
- [Examples](#examples)
- [Performance Tips](#performance-tips)
- [Troubleshooting](#troubleshooting)

---

## Overview

The ASTERIX command-line tool is a high-performance C++23 decoder for parsing ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) protocol data. It supports multiple input formats, output formats, and data sources including files and live network streams.

### Key Features

- **Multiple input formats**: PCAP, raw binary, FINAL, HDLC, GPS, ORADIS
- **Flexible output**: Text, line-delimited, JSON, XML (compact & human-readable)
- **Real-time processing**: UDP multicast streaming support
- **High performance**: 10.95x faster JSON generation, 5.85x faster PCAP processing
- **24 ASTERIX categories** supported (CAT 001-252)
- **Cross-platform**: Linux, Windows, macOS (x86_64 & ARM64)

### Version Information

```bash
./asterix --version
# Output: Asterix 2.8.10 Nov 16 2025
```

---

## Installation

### Pre-Built Binaries

Download platform-specific packages from [GitHub Releases](https://github.com/montge/asterix/releases):

| Platform | Installation |
|----------|--------------|
| **Ubuntu/Debian** | `sudo dpkg -i asterix_*.deb` |
| **RHEL/Fedora** | `sudo rpm -i asterix-*.rpm` |
| **Windows** | Extract ZIP or run MSI installer |
| **macOS** | Extract TAR.GZ or install PKG |

### Build from Source

```bash
# Configure build (out-of-source)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compile (parallel build)
cmake --build build --parallel

# Install to install/ directory
cmake --install build

# Executable location
./install/bin/asterix --help
```

**Dependencies:**
- C++23 compiler (GCC 13+, Clang 16+, MSVC 2022+, AppleClang 15+)
- CMake 3.20+
- libexpat-devel (XML parsing)

---

## Quick Start

### Parse PCAP File

```bash
# Parse PCAP file, output human-readable text
./asterix -P -f capture.pcap

# Parse PCAP, output compact JSON (one object per line)
./asterix -P -f capture.pcap -j

# Parse PCAP, output human-readable JSON
./asterix -P -f capture.pcap -jh
```

### Parse from Stdin

```bash
# Read raw ASTERIX data from stdin
cat asterix_data.raw | ./asterix

# Read PCAP from stdin
cat capture.pcap | ./asterix -P
```

### Real-Time UDP Multicast

```bash
# Listen to multicast stream
./asterix -i 239.0.0.1:192.168.1.100:21112

# Multiple multicast groups
./asterix -i 239.0.0.1:192.168.1.100:21112@239.0.0.2:192.168.1.100:21113
```

---

## Command-Line Reference

### General Options

```
-h, --help          Show help message and exit
-V, --version       Show version information
-v, --verbose       Enable verbose output (debugging)
-d, --def FILE      Custom XML config file (default: config/asterix.ini)
-L, --list          List all configured ASTERIX items
-LF, --filter FILE  Print only items listed in filter file
-o, --loop          Loop input file (repeat processing)
-s, --sync          Synchronous output (preserve timing from file)
```

### Input Format Options

Specify how ASTERIX data is encapsulated:

```
-P, --pcap          PCAP file (tcpdump/Wireshark capture)
-R, --oradispcap    PCAP with ORADIS encapsulation
-O, --oradis        Raw ORADIS encapsulation
-F, --final         FINAL packet encapsulation
-H, --hdlc          HDLC framing
-G, --gps           GPS packet encapsulation
(none)              Raw ASTERIX binary (default)
```

### Output Format Options

Control output representation:

```
(none)              Human-readable text (default)
-l, --line          One line per item (parseable)
-x, --xml           Compact XML (one object per line)
-xh, --xmlh         Human-readable XML
-j, --json          Compact JSON (one object per line)
-jh, --jsonh        Human-readable JSON
-je, --json-extensive  Extensive JSON with descriptions
```

### Data Source Options

```
-f FILE             Input file (PCAP, raw, FINAL, HDLC, GPS)
-i M:I:P[:S]        UDP multicast (mcast:interface:port[:source])
(none)              Read from stdin (default)
```

---

## Input Formats

### Raw ASTERIX Binary

Unencapsulated ASTERIX data blocks:

```bash
./asterix -f asterix_data.raw
```

**Format**: Direct ASTERIX binary (CAT + LEN + data blocks)

### PCAP (Packet Capture)

Standard Wireshark/tcpdump captures:

```bash
./asterix -P -f capture.pcap
```

**Format**: PCAP file with UDP payloads containing ASTERIX

**Use cases**:
- Network captures from tcpdump/Wireshark
- Replay recorded surveillance sessions
- Post-mortem analysis

### FINAL Format

FINAL packet encapsulation:

```bash
./asterix -F -f final_data.dat
```

**Format**: FINAL header + ASTERIX payload

### HDLC (High-Level Data Link Control)

HDLC framed serial data:

```bash
./asterix -H -f hdlc_data.dat
```

**Format**: HDLC frames with CRC-16 validation

**Use cases**:
- Serial port data (RS-232, RS-422)
- Radar site links
- Legacy ground-to-ground connections

### GPS Format

GPS packet encapsulation:

```bash
./asterix -G -f gps_data.dat
```

**Format**: GPS packets containing ASTERIX

### ORADIS Format

ORADIS encapsulation (raw or in PCAP):

```bash
# Raw ORADIS
./asterix -O -f oradis_data.dat

# ORADIS in PCAP
./asterix -R -f oradis_capture.pcap
```

---

## Output Formats

### Default (Human-Readable Text)

```bash
./asterix -P -f capture.pcap
```

**Output Example:**
```
-------------------------
Data Block 1
Category: 48
Len: 69
Timestamp: 45826414.000000
-------------------------
Data Record 1
Len: 23
CRC: 166B4DBF

Item 010 : Data Source Identifier
[ 19 C9 ]
    System Area Code: 25
    System Identification Code: 201
```

### Line-Delimited (Parseable)

```bash
./asterix -P -f capture.pcap -l
```

**Output Example:**
```
CAT048 I010 SAC:25 SIC:201
CAT048 I020 TYP:1 SIM:0 RDP:0
CAT048 I040 RHO:125.5 THETA:45.2
```

**Use case**: Pipe to grep, awk, or custom parsers

### Compact JSON (Line-Delimited)

```bash
./asterix -P -f capture.pcap -j
```

**Output Example (one object per line):**
```json
{"id":1,"cat":48,"length":23,"timestamp":45826414.0,"items":{"I010":{"SAC":25,"SIC":201},"I020":{"TYP":1}}}
{"id":2,"cat":48,"length":28,"timestamp":45826415.1,"items":{"I010":{"SAC":25,"SIC":201},"I040":{"RHO":125.5}}}
```

**Use case**: Streaming processing, log aggregation, SIEM integration

### Human-Readable JSON

```bash
./asterix -P -f capture.pcap -jh
```

**Output Example:**
```json
{
  "id": 1,
  "cat": 48,
  "length": 23,
  "timestamp": 45826414.000000,
  "items": {
    "I010": {
      "SAC": 25,
      "SIC": 201
    },
    "I020": {
      "TYP": 1,
      "SIM": 0
    }
  }
}
```

**Use case**: File storage, manual inspection, debugging

### Extensive JSON (with Descriptions)

```bash
./asterix -P -f capture.pcap -je
```

**Output Example:**
```json
{
  "id": 1,
  "cat": 48,
  "cat_description": "Monosensor Target Reports",
  "items": {
    "I010": {
      "name": "Data Source Identifier",
      "SAC": {
        "value": 25,
        "description": "System Area Code"
      },
      "SIC": {
        "value": 201,
        "description": "System Identification Code"
      }
    }
  }
}
```

**Use case**: Documentation, training, comprehensive analysis

### XML Formats

```bash
# Compact XML (line-delimited)
./asterix -P -f capture.pcap -x

# Human-readable XML
./asterix -P -f capture.pcap -xh
```

**Output Example (human-readable):**
```xml
<ASTERIX>
  <CATEGORY>48</CATEGORY>
  <LEN>23</LEN>
  <TIMESTAMP>45826414.000000</TIMESTAMP>
  <ITEMS>
    <I010>
      <SAC>25</SAC>
      <SIC>201</SIC>
    </I010>
  </ITEMS>
</ASTERIX>
```

---

## Data Sources

### File Input

```bash
# PCAP file
./asterix -P -f /path/to/capture.pcap

# Raw ASTERIX file
./asterix -f /path/to/asterix.raw

# FINAL format
./asterix -F -f /path/to/final.dat

# Loop file (repeat processing)
./asterix -P -f capture.pcap -o

# Synchronous output (preserve timing)
./asterix -P -f capture.pcap -s
```

### Standard Input

```bash
# Read from pipe
cat asterix_data.raw | ./asterix

# Read from process substitution
./asterix < <(generate_asterix_data)

# Combine with compression
zcat compressed.pcap.gz | ./asterix -P
```

### UDP Multicast

```bash
# Single multicast group
./asterix -i MCAST_IP:INTERFACE_IP:PORT

# Example: Listen on 239.0.0.1, interface 192.168.1.100, port 21112
./asterix -i 239.0.0.1:192.168.1.100:21112

# With source address filtering
./asterix -i 239.0.0.1:192.168.1.100:21112:10.17.22.23

# Multiple multicast groups (separate with @)
./asterix -i 239.0.0.1:192.168.1.100:21112@239.0.0.2:192.168.1.100:21113
```

**Multicast Parameters:**
- **MCAST_IP**: Multicast group address (224.0.0.0 - 239.255.255.255)
- **INTERFACE_IP**: Local network interface IP
- **PORT**: UDP port number
- **SOURCE** (optional): Source IP for SSM (Source-Specific Multicast)

---

## Examples

### Example 1: Parse PCAP to JSON

```bash
# Parse PCAP file, output compact JSON
./asterix -P -f asterix_capture.pcap -j > output.json

# Parse PCAP, output human-readable JSON
./asterix -P -f asterix_capture.pcap -jh > output_pretty.json

# Filter and parse (using jq)
./asterix -P -f capture.pcap -j | jq 'select(.cat == 48)'
```

### Example 2: Real-Time Multicast Monitoring

```bash
# Monitor multicast stream, output to console
./asterix -i 239.0.0.1:192.168.1.100:21112 -jh

# Monitor and log to file
./asterix -i 239.0.0.1:192.168.1.100:21112 -j >> asterix_log.json

# Monitor multiple groups
./asterix -i 239.0.0.1:192.168.1.100:21112@239.0.0.2:192.168.1.100:21113 -j
```

### Example 3: Replay PCAP with Timing

```bash
# Replay PCAP preserving original timing
./asterix -P -f capture.pcap -s

# Loop replay infinitely
./asterix -P -f capture.pcap -o -s
```

### Example 4: Filter Specific Items

Create filter file `filter.txt`:
```
I010
I040
I220
```

Apply filter:
```bash
./asterix -P -f capture.pcap -LF filter.txt
```

**Output**: Only items I010, I040, I220 will be printed

### Example 5: Custom Configuration

```bash
# Use custom ASTERIX category definitions
./asterix -P -f capture.pcap -d custom_config/asterix.ini
```

**custom_config/asterix.ini example:**
```
asterix_cat048_custom.xml
asterix_cat062_custom.xml
```

### Example 6: Pipeline Processing

```bash
# PCAP → JSON → filter by category → count records
./asterix -P -f capture.pcap -j | \
  jq 'select(.cat == 62)' | \
  wc -l

# Extract specific field from all records
./asterix -P -f capture.pcap -j | \
  jq -r '.items.I010 | "\(.SAC):\(.SIC)"' | \
  sort | uniq -c

# Convert PCAP to CSV (specific fields)
./asterix -P -f capture.pcap -j | \
  jq -r '[.timestamp, .cat, .items.I010.SAC, .items.I010.SIC] | @csv'
```

### Example 7: Performance Benchmarking

```bash
# Measure parsing throughput
time ./asterix -P -f large_capture.pcap > /dev/null

# Measure JSON generation rate
time ./asterix -P -f large_capture.pcap -j > /dev/null

# Compare output formats
for fmt in "" "-l" "-j" "-x"; do
  echo "Format: $fmt"
  time ./asterix -P -f capture.pcap $fmt > /dev/null
done
```

### Example 8: HDLC Serial Data

```bash
# Parse HDLC framed serial data
./asterix -H -f serial_data.hdlc

# Monitor serial port in real-time (requires socat)
socat /dev/ttyUSB0,raw,echo=0,b115200 - | ./asterix -H
```

### Example 9: Verbose Debugging

```bash
# Enable verbose output for troubleshooting
./asterix -P -f capture.pcap -v

# Verbose + list all items
./asterix -L -v
```

### Example 10: Quality Monitoring

```bash
# Real-time monitoring with timestamped logs
./asterix -i 239.0.0.1:192.168.1.100:21112 -j | \
  while read line; do
    echo "$(date -u +%Y-%m-%dT%H:%M:%S.%3NZ) $line"
  done | tee asterix_monitor.log
```

---

## Performance Tips

### 1. Choose Appropriate Output Format

```bash
# Fastest: Raw text output (default)
./asterix -P -f capture.pcap

# Fast: Line-delimited JSON
./asterix -P -f capture.pcap -j

# Slower: Human-readable formats
./asterix -P -f capture.pcap -jh  # Slower
./asterix -P -f capture.pcap -je  # Slowest (extensive)
```

**Recommendation**: Use `-j` for production pipelines, `-jh` for debugging

### 2. Optimize I/O

```bash
# Bad: Read small file multiple times
for i in {1..100}; do
  ./asterix -P -f small.pcap
done

# Good: Concatenate files first
cat *.pcap > combined.pcap
./asterix -P -f combined.pcap

# Better: Stream processing
for file in *.pcap; do
  ./asterix -P -f "$file"
done | process_output
```

### 3. Use Filtering

```bash
# Bad: Parse everything, filter later
./asterix -P -f capture.pcap -j | jq 'select(.cat == 48)'

# Good: Filter at source (if supported by data)
./asterix -P -f capture.pcap -LF filter.txt
```

### 4. Buffered Output

```bash
# Enable buffering for better throughput
./asterix -i 239.0.0.1:192.168.1.100:21112 -j | \
  stdbuf -oL cat >> output.json
```

### 5. Parallel Processing

```bash
# Process multiple files in parallel
find . -name "*.pcap" | \
  parallel -j 4 "./asterix -P -f {} -j > {.}.json"
```

---

## Troubleshooting

### Common Issues

#### 1. Shared Library Not Found

**Error:**
```
./asterix: error while loading shared libraries: libasterix.so.2: cannot open shared object file
```

**Solution:**
```bash
# Temporary fix
export LD_LIBRARY_PATH=/path/to/asterix/install/lib:$LD_LIBRARY_PATH
./asterix -h

# Permanent fix (Linux)
sudo ldconfig /path/to/asterix/install/lib

# Or add to /etc/ld.so.conf.d/asterix.conf
echo "/path/to/asterix/install/lib" | sudo tee /etc/ld.so.conf.d/asterix.conf
sudo ldconfig
```

#### 2. No Data Received from Multicast

**Symptoms**: No output when using `-i` option

**Checks:**
```bash
# 1. Verify multicast group membership
ip maddr show

# 2. Check firewall rules
sudo iptables -L -n | grep 21112

# 3. Test with tcpdump
sudo tcpdump -i eth0 host 239.0.0.1

# 4. Verify network interface
ip addr show
```

**Solution:**
```bash
# Allow multicast traffic
sudo iptables -A INPUT -p udp --dport 21112 -j ACCEPT

# Join multicast group manually
sudo ip maddress add 239.0.0.1 dev eth0
```

#### 3. Invalid ASTERIX Data

**Error:**
```
ERROR: Wrong length in DataItem format
ERROR: Record not parsed properly
```

**Causes:**
- Corrupted PCAP file
- Wrong input format specified
- Unsupported ASTERIX category

**Solutions:**
```bash
# 1. Verify file integrity
file capture.pcap

# 2. Try different input format
./asterix -P -f capture.pcap   # If it's really PCAP
./asterix -f capture.pcap      # If it's raw ASTERIX

# 3. Enable verbose mode
./asterix -P -f capture.pcap -v

# 4. Check category support
./asterix -L | grep "CAT XXX"
```

#### 4. Performance Issues

**Symptoms**: Slow processing, high CPU usage

**Solutions:**
```bash
# 1. Use compact output format
./asterix -P -f large.pcap -j  # Not -je

# 2. Reduce output buffering
./asterix -P -f large.pcap > /dev/null  # No display

# 3. Check system resources
top -p $(pgrep asterix)

# 4. Profile with perf (Linux)
perf record -g ./asterix -P -f large.pcap
perf report
```

#### 5. Configuration File Not Found

**Error:**
```
Cannot open file: config/asterix.ini
```

**Solution:**
```bash
# Check current directory
pwd
ls -la config/

# Use absolute path
./asterix -d /path/to/asterix/install/share/asterix/config/asterix.ini -P -f capture.pcap

# Or run from install directory
cd install
./bin/asterix -P -f ../capture.pcap
```

---

## Configuration

### Default Configuration

Default location: `install/share/asterix/config/asterix.ini`

**Contents:**
```
asterix_cat001_1_4.xml
asterix_cat002_1_1.xml
asterix_cat048_1_30.xml
asterix_cat062_1_19.xml
... (all supported categories)
```

### Custom Configuration

Create custom configuration file:

**custom_asterix.ini:**
```
/path/to/asterix_cat048_custom.xml
/path/to/asterix_cat062_custom.xml
```

Use custom config:
```bash
./asterix -d custom_asterix.ini -P -f capture.pcap
```

### Supported Categories

24 ASTERIX categories (see `--list` option):

| Category | Description | Version |
|----------|-------------|---------|
| CAT 001 | Monosensor Surface Movement | 1.4 |
| CAT 002 | Monosensor Surface Movement | 1.1 |
| CAT 048 | Monosensor Target Reports | 1.30 |
| CAT 062 | SDPS Track Messages | 1.19 |
| ... | (See `./asterix -L`) | ... |

---

## Integration Examples

### Elasticsearch

```bash
# Stream ASTERIX to Elasticsearch
./asterix -i 239.0.0.1:192.168.1.100:21112 -j | \
  while read line; do
    curl -X POST "localhost:9200/asterix/_doc" \
         -H 'Content-Type: application/json' \
         -d "$line"
  done
```

### Kafka

```bash
# Send to Kafka topic
./asterix -i 239.0.0.1:192.168.1.100:21112 -j | \
  kafka-console-producer --broker-list localhost:9092 \
                         --topic asterix-data
```

### InfluxDB

```bash
# Convert to InfluxDB line protocol
./asterix -i 239.0.0.1:192.168.1.100:21112 -j | \
  jq -r '"asterix,cat=\(.cat),sac=\(.items.I010.SAC),sic=\(.items.I010.SIC) value=1 \(.timestamp | floor)"' | \
  curl -XPOST 'http://localhost:8086/write?db=aviation' --data-binary @-
```

### Splunk

```bash
# Forward to Splunk HEC
./asterix -i 239.0.0.1:192.168.1.100:21112 -j | \
  while read line; do
    curl -X POST "https://splunk:8088/services/collector" \
         -H "Authorization: Splunk YOUR-TOKEN" \
         -d "{\"event\": $line}"
  done
```

---

## Additional Resources

### Documentation
- **Main README**: [../README.md](../README.md)
- **Architecture**: [ARCHITECTURE.md](ARCHITECTURE.md)
- **Build Guide**: [BUILD.md](BUILD.md)
- **Performance**: [../PERFORMANCE_OPTIMIZATIONS.md](../PERFORMANCE_OPTIMIZATIONS.md)
- **Python Module**: [../asterix/README.md](../asterix/README.md)
- **Rust Crate**: [../asterix-rs/README.md](../asterix-rs/README.md)

### Examples Directory
- **Test Scripts**: `install/test/test.sh`
- **Sample Data**: `install/share/asterix/samples/`
- **Sample Output**: `install/share/asterix/sample_output/`

### Related Tools
- **tcpdump**: Capture network traffic
- **Wireshark**: Analyze PCAP files
- **jq**: Process JSON output
- **socat**: Serial port I/O

---

## License

This C++ executable is part of the ASTERIX project and is licensed under **GPL-3.0-or-later**.

See [LICENSE](../LICENSE) for details.

---

## Support

- **Issues**: [GitHub Issues](https://github.com/montge/asterix/issues)
- **Discussions**: [GitHub Discussions](https://github.com/montge/asterix/discussions)
- **ASTERIX Protocol**: http://www.eurocontrol.int/services/asterix
