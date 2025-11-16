# ASTERIX Python Module

[![PyPI](https://img.shields.io/pypi/v/asterix_decoder.svg)](https://pypi.org/project/asterix_decoder/)
[![Python Versions](https://img.shields.io/badge/python-3.10%20%7C%203.11%20%7C%203.12%20%7C%203.13%20%7C%203.14-blue)](https://pypi.org/project/asterix_decoder/)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](../LICENSE)

Python bindings for the ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) protocol decoder. This module provides a simple, intuitive API for parsing Air Traffic Management surveillance data.

## Quick Start

### Installation

```bash
pip install asterix_decoder
```

### Basic Usage

```python
import asterix

# Parse raw ASTERIX data
with open('capture.pcap', 'rb') as f:
    data = f.read()

parsed_records = asterix.parse(data)

# Display results
for record in parsed_records:
    print(f"Category {record['category']}: {len(record['items'])} items")
    print(asterix.describe(record))
```

## API Reference

### Core Functions

#### `parse(data: bytes) -> list`

Parse raw ASTERIX binary data and return parsed records.

**Parameters:**
- `data` (bytes): Raw ASTERIX binary data (max 65,536 bytes)

**Returns:**
- `list`: List of parsed ASTERIX records as dictionaries

**Raises:**
- `ValueError`: If data is empty or exceeds 64KB limit
- `RuntimeError`: If parsing fails due to invalid ASTERIX format

**Example:**
```python
import asterix

# Read PCAP file with ASTERIX data
with open('asterix_capture.pcap', 'rb') as f:
    pcap_data = f.read()

# Parse ASTERIX records
try:
    records = asterix.parse(pcap_data)
    print(f"Parsed {len(records)} ASTERIX records")

    for record in records:
        cat = record.get('category')
        print(f"Category {cat}: {record}")
except ValueError as e:
    print(f"Invalid input: {e}")
except RuntimeError as e:
    print(f"Parse error: {e}")
```

---

#### `parse_with_offset(data: bytes, offset: int, blocks_count: int) -> list`

Incremental parsing of ASTERIX data from a specific offset.

**Parameters:**
- `data` (bytes): Raw ASTERIX binary data
- `offset` (int): Starting position in bytes (0-indexed)
- `blocks_count` (int): Maximum number of data blocks to parse (max 10,000)

**Returns:**
- `list`: List of parsed ASTERIX records

**Raises:**
- `ValueError`: If offset exceeds data length or blocks_count > 10,000

**Example:**
```python
import asterix

# Large ASTERIX data stream
data = read_large_asterix_stream()

# Parse in chunks
offset = 0
chunk_size = 100  # Parse 100 blocks at a time

while offset < len(data):
    try:
        records = asterix.parse_with_offset(data, offset, chunk_size)

        for record in records:
            process_record(record)

        # Update offset based on records processed
        offset += calculate_bytes_consumed(records)
    except ValueError:
        break  # Reached end of valid data
```

---

#### `describe(category: int, item: str = None, field: str = None, value: str = None) -> str`

Get human-readable descriptions for ASTERIX categories, data items, fields, and values.

**Parameters:**
- `category` (int): ASTERIX category number (1-255)
- `item` (str, optional): Data item identifier (e.g., "I010")
- `field` (str, optional): Field name within the data item
- `value` (str, optional): Specific value to describe

**Returns:**
- `str`: Human-readable description

**Raises:**
- `ValueError`: If category is 0 or invalid

**Examples:**
```python
import asterix

# Describe a category
desc = asterix.describe(62)
print(desc)  # "CAT 062: SDPS Track Messages"

# Describe a data item
desc = asterix.describe(62, "I010")
print(desc)  # "Data Source Identifier"

# Describe a field value
desc = asterix.describe(48, "I020", "TYP", "1")
print(desc)  # "SSR multilateration"
```

---

#### `init(filename: str) -> int`

Load custom ASTERIX category definition from XML file.

**Parameters:**
- `filename` (str): Path to ASTERIX XML configuration file

**Returns:**
- `int`: 0 on success

**Raises:**
- `ValueError`: If filename is empty, too long (>4096 chars), or contains path traversal (`../`)
- `IOError`: If file cannot be read
- `SyntaxError`: If XML is malformed

**Security Notes:**
- Path traversal is blocked (leading `..` rejected)
- Maximum filename length: 4,096 characters
- XML syntax errors are caught and reported

**Example:**
```python
import asterix

# Load custom category definition
try:
    asterix.init('config/asterix_cat062_custom.xml')
    print("Custom configuration loaded")
except ValueError as e:
    print(f"Invalid file path: {e}")
except SyntaxError as e:
    print(f"XML parse error: {e}")
except IOError as e:
    print(f"File error: {e}")
```

---

### Low-Level FFI Functions

Direct access to C++ FFI layer (advanced users only):

#### `_asterix.describe(category, item=None, field=None, value=None) -> str`

Low-level describe function with strict input validation.

#### `_asterix.parse_asterix(data: bytes) -> list`

Low-level parse function without input size validation.

> **Warning:** Direct FFI functions bypass Python-level validation. Use the high-level API (`asterix.parse`, `asterix.describe`, `asterix.init`) for production code.

---

## Data Format

### Parsed Record Structure

Each parsed ASTERIX record is returned as a dictionary:

```python
{
    'category': int,           # ASTERIX category (e.g., 48, 62)
    'length': int,             # Record length in bytes
    'timestamp': float,        # Unix timestamp (seconds since epoch)
    'items': {                 # Data items dictionary
        'I010': {...},         # Data Source Identifier
        'I020': {...},         # Target Report Descriptor
        'I040': {...},         # Measured Position (polar)
        # ... additional items
    }
}
```

### Data Item Structure

Each data item contains parsed fields:

```python
{
    'SAC': 25,                # System Area Code
    'SIC': 100                # System Identification Code
}
```

---

## Examples

### Example 1: Parse PCAP File

```python
import asterix

# Parse PCAP file containing ASTERIX data
with open('asterix_capture.pcap', 'rb') as f:
    data = f.read()

records = asterix.parse(data)

print(f"Total records: {len(records)}")

for record in records:
    cat = record.get('category')
    timestamp = record.get('timestamp', 0)
    items = record.get('items', {})

    print(f"\nCategory {cat} @ {timestamp}")

    # Extract data source
    if 'I010' in items:
        sac = items['I010'].get('SAC')
        sic = items['I010'].get('SIC')
        print(f"  Source: SAC={sac}, SIC={sic}")

    # Extract position (if available)
    if 'I040' in items:
        rho = items['I040'].get('RHO')  # Range (nautical miles)
        theta = items['I040'].get('THETA')  # Azimuth (degrees)
        print(f"  Position: {rho} NM @ {theta}°")
```

### Example 2: Incremental Parsing

```python
import asterix

def parse_large_file(filename):
    """Parse large ASTERIX file incrementally"""
    with open(filename, 'rb') as f:
        data = f.read()

    offset = 0
    batch_size = 100
    total_records = 0

    while offset < len(data):
        try:
            # Parse next batch
            records = asterix.parse_with_offset(data, offset, batch_size)

            if not records:
                break

            total_records += len(records)

            # Process batch
            for record in records:
                process_record(record)

            # Calculate next offset (estimate based on avg record size)
            bytes_per_record = 50  # Approximate
            offset += len(records) * bytes_per_record

        except ValueError:
            # Reached end or invalid offset
            break

    return total_records

# Process large file
count = parse_large_file('large_capture.pcap')
print(f"Processed {count} records")
```

### Example 3: Filter by Category

```python
import asterix

def filter_by_category(data, target_category):
    """Extract only records from specific ASTERIX category"""
    records = asterix.parse(data)

    filtered = [r for r in records if r.get('category') == target_category]

    print(f"Found {len(filtered)} CAT {target_category} records")

    return filtered

# Read data
with open('mixed_categories.pcap', 'rb') as f:
    data = f.read()

# Extract only CAT 062 (SDPS Track Messages)
cat062_records = filter_by_category(data, 62)

for record in cat062_records:
    # Process CAT 062 specific fields
    if 'I010' in record['items']:
        print(record['items']['I010'])
```

### Example 4: Custom Configuration

```python
import asterix
import os

# Load custom ASTERIX category definitions
config_dir = 'config/'
custom_configs = [
    'asterix_cat062_custom_1_20.xml',
    'asterix_cat048_custom_1_31.xml'
]

for config in custom_configs:
    config_path = os.path.join(config_dir, config)

    try:
        asterix.init(config_path)
        print(f"Loaded: {config}")
    except Exception as e:
        print(f"Failed to load {config}: {e}")

# Now parse with custom categories
data = read_asterix_data()
records = asterix.parse(data)

# Custom category fields will be parsed according to XML definitions
for record in records:
    print(asterix.describe(record))
```

### Example 5: Real-Time UDP Multicast

```python
import socket
import asterix

def receive_asterix_multicast(mcast_group, port, interface='0.0.0.0'):
    """Receive ASTERIX data from UDP multicast stream"""

    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Bind to multicast group
    sock.bind((interface, port))

    # Join multicast group
    mreq = socket.inet_aton(mcast_group) + socket.inet_aton(interface)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    print(f"Listening on {mcast_group}:{port}")

    try:
        while True:
            # Receive datagram (max 65KB)
            data, addr = sock.recvfrom(65536)

            # Parse ASTERIX data
            try:
                records = asterix.parse(data)

                for record in records:
                    process_record(record)

            except (ValueError, RuntimeError) as e:
                print(f"Parse error: {e}")

    finally:
        sock.close()

# Example usage
receive_asterix_multicast('239.0.0.1', 21112)
```

---

## Error Handling

### Common Exceptions

| Exception | Cause | Solution |
|-----------|-------|----------|
| `ValueError` | Empty data, oversized data (>64KB), invalid offset, category 0 | Validate input size and content |
| `RuntimeError` | Invalid ASTERIX format, parsing errors | Check data integrity, verify ASTERIX spec |
| `SyntaxError` | Malformed XML configuration | Validate XML against `asterix.dtd` |
| `IOError` | File not found, permission denied | Check file path and permissions |
| `OverflowError` | Negative values for unsigned parameters | Use positive integers only |

### Best Practices

```python
import asterix

def safe_parse(data):
    """Safely parse ASTERIX data with comprehensive error handling"""

    # Input validation
    if not data:
        raise ValueError("Data cannot be empty")

    if len(data) > 65536:
        raise ValueError("Data exceeds 64KB limit")

    try:
        records = asterix.parse(data)
        return records

    except ValueError as e:
        # Input validation error
        print(f"Invalid input: {e}")
        return []

    except RuntimeError as e:
        # ASTERIX format error
        print(f"Parse error: {e}")
        return []

    except Exception as e:
        # Unexpected error
        print(f"Unexpected error: {e}")
        return []

# Usage
data = read_asterix_data()
records = safe_parse(data)
```

---

## Thread Safety

**WARNING:** The Python module is **NOT thread-safe** due to the shared C++ singleton state (`AsterixDefinition`).

### Thread Safety Issues

- **Global category state**: All category definitions stored in singleton
- **Concurrent access**: Simultaneous `init()` or `parse()` calls can corrupt state
- **Race conditions**: No internal locking mechanism

### Solutions

#### Option 1: External Locking

```python
import asterix
import threading

# Global lock for ASTERIX operations
asterix_lock = threading.Lock()

def thread_safe_parse(data):
    """Thread-safe parse with external locking"""
    with asterix_lock:
        return asterix.parse(data)

# Use in multithreaded context
threads = []
for data_chunk in data_chunks:
    t = threading.Thread(target=lambda: thread_safe_parse(data_chunk))
    threads.append(t)
    t.start()

for t in threads:
    t.join()
```

#### Option 2: Process-Based Parallelism

```python
import asterix
import multiprocessing

def parse_worker(data):
    """Worker process for parsing (isolated state)"""
    return asterix.parse(data)

# Use multiprocessing for isolated state
with multiprocessing.Pool(processes=4) as pool:
    results = pool.map(parse_worker, data_chunks)
```

#### Option 3: Pre-fork Server

```python
import asterix
from multiprocessing import Process, Queue

def parser_server(input_queue, output_queue):
    """Dedicated parser process"""
    # Initialize once per process
    asterix.init('config/asterix.ini')

    while True:
        data = input_queue.get()
        if data is None:
            break

        records = asterix.parse(data)
        output_queue.put(records)

# Start parser server
in_q = Queue()
out_q = Queue()

parser = Process(target=parser_server, args=(in_q, out_q))
parser.start()

# Send data to parser
in_q.put(asterix_data)
results = out_q.get()

# Shutdown
in_q.put(None)
parser.join()
```

---

## Performance Tips

### 1. Reuse Connections

```python
# BAD: Open file in loop
for i in range(1000):
    with open(f'file_{i}.ast', 'rb') as f:
        asterix.parse(f.read())

# GOOD: Read all files first
data_list = []
for i in range(1000):
    with open(f'file_{i}.ast', 'rb') as f:
        data_list.append(f.read())

for data in data_list:
    asterix.parse(data)
```

### 2. Batch Processing

```python
# Process in batches for better cache locality
def batch_parse(file_list, batch_size=100):
    for i in range(0, len(file_list), batch_size):
        batch = file_list[i:i+batch_size]

        for filename in batch:
            with open(filename, 'rb') as f:
                records = asterix.parse(f.read())
                yield from records
```

### 3. Incremental Parsing

```python
# For large files, use parse_with_offset
def parse_large_file(filename):
    with open(filename, 'rb') as f:
        data = f.read()

    offset = 0
    while offset < len(data):
        records = asterix.parse_with_offset(data, offset, 100)
        yield from records
        offset += estimate_bytes_consumed(records)
```

### 4. Memory Management

```python
# Clear large data structures explicitly
def process_and_clear(filename):
    with open(filename, 'rb') as f:
        data = f.read()

    records = asterix.parse(data)

    # Process immediately
    for record in records:
        process_record(record)

    # Clear data
    del data
    del records
```

---

## Supported ASTERIX Categories

The module supports **24 ASTERIX categories**:

| Category | Description | Version |
|----------|-------------|---------|
| **CAT 001** | Monosensor Surface Movement Data | 1.4 |
| **CAT 002** | Monosensor Surface Movement Data | 1.1 |
| **CAT 004** | Safety Net Messages | 1.12 |
| **CAT 008** | Monosensor Surface Movement Data | 1.3 |
| **CAT 010** | Monosensor Radar Data | 1.1 |
| **CAT 011** | Monosensor Track Data | 1.3 |
| **CAT 015** | INCS Target Reports | 1.2 |
| **CAT 019** | Multilateration System Status Messages | 1.3 |
| **CAT 020** | Multilateration Target Reports | 1.10 |
| **CAT 021** | ADS-B Target Reports | 2.6 |
| **CAT 023** | CNS/ATM Ground Station Status | 1.3 |
| **CAT 025** | CNS/ATM System Status Messages | 1.5 |
| **CAT 030** | ARTAS System Status | 6.2 |
| **CAT 031** | ARTAS Track State | 6.2 |
| **CAT 032** | Miniplan Messages | 7.0 |
| **CAT 034** | Monosensor Service Messages | 1.29 |
| **CAT 048** | Monosensor Target Reports | 1.30 |
| **CAT 062** | SDPS Track Messages | 1.19 |
| **CAT 063** | Sensor Status Messages | 1.6 |
| **CAT 065** | SDPS Service Messages | 1.5 |
| **CAT 205** | Ground Station Operational Messages | 1.0 |
| **CAT 240** | Radar Video | 1.3 |
| **CAT 247** | Reserved | 1.3 |
| **CAT 252** | BDS Registers | 7.0 |

**Category Support Matrix:** See [ASTERIX_CATEGORY_SUPPORT_MATRIX.md](../ASTERIX_CATEGORY_SUPPORT_MATRIX.md) for detailed field coverage.

---

## Additional Resources

### Documentation
- **Main Project**: [README.md](../README.md)
- **Architecture**: [docs/ARCHITECTURE.md](../docs/ARCHITECTURE.md)
- **Build Guide**: [docs/BUILD.md](../docs/BUILD.md)
- **Contributing**: [CONTRIBUTING.md](../CONTRIBUTING.md)
- **Python Examples**: [asterix/examples/](examples/)

### Examples Directory

Full working examples available in `asterix/examples/`:

- **read_raw_bytes.py**: Parse raw ASTERIX binary data
- **read_raw_file.py**: Parse ASTERIX file
- **read_pcap_file.py**: Parse PCAP file containing ASTERIX
- **read_final_file.py**: Parse FINAL format data
- **multicast_receive.py**: Receive UDP multicast ASTERIX stream
- **multicast_send_receive.py**: Full multicast send/receive example
- **xml_parser.py**: Custom XML category configuration

### Related Projects
- **Rust Bindings**: [asterix-rs/](../asterix-rs/) - Type-safe Rust API with zero-copy parsing
- **C++ Executable**: [src/main/](../src/main/) - High-performance CLI tool
- **Spec Converter**: [asterix-specs-converter/](../asterix-specs-converter/) - JSON to XML category converter

---

## License

This Python module is part of the ASTERIX project and is licensed under **GPL-3.0-or-later**.

See [LICENSE](../LICENSE) for details.

---

## Support

- **Issues**: [GitHub Issues](https://github.com/montge/asterix/issues)
- **Discussions**: [GitHub Discussions](https://github.com/montge/asterix/discussions)
- **ASTERIX Protocol**: http://www.eurocontrol.int/services/asterix
