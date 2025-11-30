# Python User Guide

Complete guide to using the ASTERIX decoder Python module for parsing surveillance data.

## Overview

The `asterix-decoder` Python package provides a simple, intuitive API for parsing ASTERIX binary data. It wraps the high-performance C++ parser with Pythonic interfaces.

**Supported Python versions:** 3.10, 3.11, 3.12, 3.13, 3.14

## Installation

### From PyPI (Recommended)

```bash
pip install asterix-decoder
```

### From Source

```bash
git clone https://github.com/montge/asterix.git
cd asterix
pip install .
```

### Development Installation

```bash
pip install -e ".[dev]"
```

## Quick Start

```python
import asterix

# Parse raw ASTERIX bytes
with open("sample.asterix", "rb") as f:
    data = f.read()

records = asterix.parse(data)

for record in records:
    print(f"Category: {record['cat']}")
    print(f"Length: {record['length']}")
```

## API Reference

### Core Functions

#### `parse(data: bytes) -> list`

Parse ASTERIX data from raw bytes.

**Parameters:**
- `data` - Raw ASTERIX binary data

**Returns:**
- List of parsed records (dictionaries)

**Example:**
```python
import asterix

data = bytes.fromhex("30002a...")  # ASTERIX bytes
records = asterix.parse(data)

for record in records:
    category = record['cat']
    print(f"Category {category}: {record}")
```

#### `parse_with_offset(data: bytes, offset: int, count: int) -> tuple`

Parse ASTERIX data with offset for incremental/streaming parsing.

**Parameters:**
- `data` - Raw ASTERIX binary data
- `offset` - Starting byte offset
- `count` - Maximum number of blocks to parse

**Returns:**
- Tuple of (records, next_offset)

**Example:**
```python
import asterix

with open("large_file.asterix", "rb") as f:
    data = f.read()

offset = 0
while offset < len(data):
    records, offset = asterix.parse_with_offset(data, offset, 100)
    for record in records:
        process(record)
```

#### `describe(category: int, item: str = None, field: str = None, value: int = None) -> str`

Get human-readable descriptions for ASTERIX fields and values.

**Parameters:**
- `category` - ASTERIX category number
- `item` - Data item ID (e.g., "010", "140")
- `field` - Field name within item
- `value` - Numeric value to describe

**Returns:**
- Description string

**Example:**
```python
import asterix

# Get category description
print(asterix.describe(48))
# Output: "Monoradar Target Reports"

# Get item description
print(asterix.describe(48, "010"))
# Output: "Data Source Identifier"

# Get value meaning
print(asterix.describe(48, "020", "TYP", 5))
# Output: "Single ModeS Roll-Call"
```

#### `init(filename: str) -> bool`

Load a custom ASTERIX category definition file.

**Parameters:**
- `filename` - Path to XML category definition

**Returns:**
- True if successful

**Example:**
```python
import asterix

# Load custom category definition
asterix.init("/path/to/asterix_cat999_custom.xml")
```

### Record Structure

Parsed records are Python dictionaries with the following structure:

```python
{
    "id": 1,                    # Record sequence number
    "cat": 48,                  # ASTERIX category
    "length": 45,               # Record length in bytes
    "crc": "AB659C3E",          # CRC checksum
    "timestamp": 27356508.0,    # Timestamp
    "hexdata": "300030...",     # Raw hex data
    "CAT048": {                 # Category-specific data
        "I010": {               # Data Source Identifier
            "SAC": 25,
            "SIC": 201
        },
        "I140": {               # Time of Day
            "ToD": 27354.6015625
        },
        # ... more items
    }
}
```

## Common Use Cases

### Parse PCAP File

```python
import asterix
from scapy.all import rdpcap, UDP

# Read PCAP with scapy
packets = rdpcap("capture.pcap")

for pkt in packets:
    if UDP in pkt:
        data = bytes(pkt[UDP].payload)
        if len(data) >= 3:  # Minimum ASTERIX block
            try:
                records = asterix.parse(data)
                for record in records:
                    print(f"Cat {record['cat']}: {record.get('CAT048', {})}")
            except Exception as e:
                print(f"Parse error: {e}")
```

### Extract Specific Fields

```python
import asterix

def extract_target_reports(data: bytes) -> list:
    """Extract Mode-3/A codes from CAT 048 records."""
    reports = []
    records = asterix.parse(data)

    for record in records:
        if record['cat'] == 48:
            cat048 = record.get('CAT048', {})

            # Extract Mode-3/A code
            i070 = cat048.get('I070', {})
            mode3a = i070.get('MODE3A')

            # Extract position
            i040 = cat048.get('I040', {})
            rho = i040.get('RHO')
            theta = i040.get('THETA')

            if mode3a:
                reports.append({
                    'mode3a': mode3a,
                    'range_nm': rho,
                    'azimuth_deg': theta
                })

    return reports

# Usage
with open("radar_data.asterix", "rb") as f:
    targets = extract_target_reports(f.read())
    for t in targets:
        print(f"Squawk {t['mode3a']}: {t['range_nm']:.1f} NM @ {t['azimuth_deg']:.1f}°")
```

### Streaming Large Files

```python
import asterix

def process_large_file(filename: str, batch_size: int = 1000):
    """Process large ASTERIX file in chunks."""
    with open(filename, "rb") as f:
        data = f.read()

    offset = 0
    total_records = 0

    while offset < len(data):
        records, offset = asterix.parse_with_offset(data, offset, batch_size)

        for record in records:
            # Process each record
            yield record
            total_records += 1

    print(f"Processed {total_records} records")

# Usage
for record in process_large_file("large_capture.asterix"):
    if record['cat'] == 62:  # SDPS track
        print(record['CAT062'].get('I105'))  # Position
```

### Convert to JSON

```python
import asterix
import json

def asterix_to_json(input_file: str, output_file: str):
    """Convert ASTERIX binary to JSON."""
    with open(input_file, "rb") as f:
        data = f.read()

    records = asterix.parse(data)

    with open(output_file, "w") as f:
        json.dump(records, f, indent=2)

# Usage
asterix_to_json("sample.asterix", "sample.json")
```

### Filter by Category

```python
import asterix
from typing import Generator

def filter_categories(data: bytes, categories: list[int]) -> Generator:
    """Yield only records from specified categories."""
    for record in asterix.parse(data):
        if record['cat'] in categories:
            yield record

# Usage - extract only CAT 062 (SDPS tracks)
with open("mixed_data.asterix", "rb") as f:
    for record in filter_categories(f.read(), [62]):
        track_num = record['CAT062'].get('I040', {}).get('TN')
        print(f"Track {track_num}")
```

### UDP Multicast Receiver

```python
import asterix
import socket
import struct

def receive_multicast(group: str, port: int, interface: str = ""):
    """Receive ASTERIX data from UDP multicast."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('', port))

    # Join multicast group
    mreq = struct.pack("4s4s", socket.inet_aton(group),
                       socket.inet_aton(interface) if interface else socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    print(f"Listening on {group}:{port}")

    while True:
        data, addr = sock.recvfrom(65535)
        try:
            records = asterix.parse(data)
            for record in records:
                print(f"[{addr[0]}] Cat {record['cat']}: {len(record)} bytes")
        except Exception as e:
            print(f"Parse error: {e}")

# Usage
receive_multicast("232.1.1.31", 21131)
```

## Supported Categories

The module supports all ASTERIX categories with XML definitions:

| Category | Description | Version |
|----------|-------------|---------|
| CAT 001 | Monoradar Target Reports | v1.4 |
| CAT 002 | Monoradar Service Messages | v1.1 |
| CAT 008 | Monoradar Target Reports (Enhanced) | v1.2 |
| CAT 021 | ADS-B Target Reports | v2.6 |
| CAT 023 | CNS/ATM Ground Station Service Messages | v1.3 |
| CAT 034 | Monoradar Service Messages | v1.29 |
| CAT 048 | Monoradar Target Reports | v1.30 |
| CAT 062 | SDPS Track Messages | v1.18 |
| CAT 063 | Sensor Status Messages | v1.6 |
| CAT 065 | SDPS Service Status Messages | v1.5 |

## Error Handling

```python
import asterix

try:
    records = asterix.parse(data)
except ValueError as e:
    print(f"Invalid ASTERIX data: {e}")
except RuntimeError as e:
    print(f"Parser error: {e}")
```

## Performance Tips

1. **Use `parse_with_offset` for large files** - avoids loading everything into memory
2. **Batch processing** - process records in chunks rather than one at a time
3. **Filter early** - check category before extracting fields
4. **Reuse parsed data** - cache results if processing multiple times

## Testing

```python
import asterix
import unittest

class TestAsterixParsing(unittest.TestCase):
    def test_parse_cat048(self):
        # Sample CAT 048 data
        data = bytes.fromhex("30002afdf70219c9...")
        records = asterix.parse(data)

        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]['cat'], 48)
        self.assertIn('CAT048', records[0])

    def test_describe_category(self):
        desc = asterix.describe(48)
        self.assertIn("Monoradar", desc)

if __name__ == '__main__':
    unittest.main()
```

## Related Documentation

- [Building from Source](../BUILD.md#python-module)
- [API Architecture](../ARCHITECTURE.md)
- [ASTERIX Protocol Reference](https://www.eurocontrol.int/asterix)

## Support

- **PyPI**: https://pypi.org/project/asterix-decoder/
- **Issues**: https://github.com/montge/asterix/issues
- **Source**: `asterix/` directory in repository
