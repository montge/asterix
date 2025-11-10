# ASTERIX Decoder for Node.js

[![npm version](https://img.shields.io/npm/v/asterix-decoder.svg)](https://www.npmjs.com/package/asterix-decoder)
[![Node.js Version](https://img.shields.io/node/v/asterix-decoder.svg)](https://nodejs.org/)
[![License](https://img.shields.io/badge/license-GPL--3.0--or--later-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)

Node.js bindings for the ASTERIX decoder - a high-performance parser for EUROCONTROL ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) protocol data used in Air Traffic Management systems.

## Features

- **High Performance**: Native C++ core with N-API bindings
- **Safety-Critical Design**: Validated FFI boundary with comprehensive error handling
- **Multiple Formats**: Supports raw ASTERIX, PCAP, HDLC, FINAL, GPS encapsulation
- **67 Categories**: Complete support for ASTERIX categories (CAT001-CAT255)
- **TypeScript Support**: Full type definitions included
- **Cross-Platform**: Linux, macOS, Windows (Node.js 18.x, 20.x, 22.x)
- **Stream Processing**: Incremental parsing for large files
- **Memory Safe**: Zero memory leaks, comprehensive bounds checking

## Installation

### Prerequisites

- Node.js 18.x, 20.x, or 22.x (LTS versions)
- C++20 compiler (GCC 10+, Clang 12+, MSVC 2019+)
- libexpat-devel (for XML parsing)
- CMake 3.20+ or GNU Make

### Install from npm

```bash
npm install asterix-decoder
```

### Build from source

```bash
git clone https://github.com/montge/asterix.git
cd asterix/asterix-node
npm install
npm run build
```

### Platform-Specific Build Notes

The Node.js bindings use different build strategies per platform:

**Linux/macOS:**
- Links against pre-built shared library (`libasterix.so` / `libasterix.dylib`)
- Fast builds, small binary size
- Shared library must be built first: `cmake -B build && cmake --build build && cmake --install build`

**Windows:**
- Compiles ASTERIX C++ sources directly into the addon (22 source files)
- Self-contained build - no separate library build required
- Uses dynamic EXPAT library (`expat:x64-windows` via vcpkg)
- Pattern matches Python bindings approach for cross-platform consistency

Both approaches are fully supported and tested in CI across all Node.js versions (20, 22, 24).

## Quick Start

```javascript
const asterix = require('asterix-decoder');
const fs = require('fs');

// Initialize parser (auto-loads all default categories)
asterix.init();

// Parse ASTERIX data
const data = fs.readFileSync('sample.asterix');
const records = asterix.parse(data);

// Process records
for (const record of records) {
  console.log(`Category ${record.category}: ${Object.keys(record.items).length} items`);
  console.log(`Timestamp: ${new Date(record.timestamp_ms)}`);

  // Access data items
  for (const [itemId, itemData] of Object.entries(record.items)) {
    console.log(`  ${itemId}:`, itemData);
  }
}
```

## API Reference

### Initialization

#### `init([configDir])`

Initialize the ASTERIX parser with optional custom configuration directory.

**Parameters:**
- `configDir` (string, optional): Path to ASTERIX XML configuration directory

**Throws:**
- `TypeError`: If config directory is invalid
- `Error`: If initialization fails

**Example:**
```javascript
// Use default config
asterix.init();

// Use custom config directory
asterix.init('/path/to/asterix/config');
```

#### `loadCategory(xmlPath)`

Load a specific ASTERIX category definition file.

**Parameters:**
- `xmlPath` (string, required): Path to XML category definition file

**Throws:**
- `TypeError`: If XML path is invalid
- `Error`: If loading fails

**Example:**
```javascript
asterix.loadCategory('./config/asterix_cat062_1_19.xml');
```

### Parsing

#### `parse(data, [options])`

Parse ASTERIX data from a Buffer.

**Parameters:**
- `data` (Buffer, required): Buffer containing ASTERIX data
- `options` (Object, optional):
  - `verbose` (boolean): Enable verbose output (default: false)
  - `filterCategory` (number): Filter to specific category
  - `maxRecords` (number): Maximum records to parse

**Returns:** Array of `AsterixRecord` objects

**Throws:**
- `TypeError`: If input is not a Buffer or is empty
- `Error`: If parsing fails

**Example:**
```javascript
const records = asterix.parse(buffer, {
  verbose: true,
  filterCategory: 62,
  maxRecords: 1000
});
```

#### `parseWithOffset(data, offset, blocksCount, [options])`

Parse ASTERIX data incrementally for large files or streams.

**Parameters:**
- `data` (Buffer, required): Buffer containing ASTERIX data
- `offset` (number, required): Byte offset to start parsing from
- `blocksCount` (number, required): Maximum blocks to parse (0 = all)
- `options` (Object, optional): Same as `parse()`

**Returns:** `ParseResult` object:
```javascript
{
  records: Array<AsterixRecord>,    // Parsed records
  bytesConsumed: number,             // Bytes consumed from input
  remainingBlocks: number            // Estimated remaining blocks
}
```

**Example:**
```javascript
const data = fs.readFileSync('large_file.asterix');
let offset = 0;
const allRecords = [];

while (offset < data.length) {
  const result = asterix.parseWithOffset(data, offset, 100);
  allRecords.push(...result.records);
  offset = result.bytesConsumed;

  if (result.remainingBlocks === 0) {
    break;
  }
}
```

### Metadata

#### `describe(category, [item], [field], [value])`

Get human-readable description for ASTERIX elements.

**Parameters:**
- `category` (number, required): ASTERIX category (1-255)
- `item` (string, optional): Item ID (e.g., '010')
- `field` (string, optional): Field name (e.g., 'SAC')
- `value` (string, optional): Value

**Returns:** String description

**Example:**
```javascript
// Category description
asterix.describe(48);
// "Monoradar Target Reports"

// Item description
asterix.describe(48, '010');
// "Data Source Identifier"

// Field value description
asterix.describe(48, '010', 'SAC', '7');
// "System Area Code: 7"
```

#### `isCategoryDefined(category)`

Check if an ASTERIX category is loaded.

**Parameters:**
- `category` (number, required): ASTERIX category (1-255)

**Returns:** boolean (true if loaded, false otherwise)

**Example:**
```javascript
if (asterix.isCategoryDefined(62)) {
  console.log('CAT062 is available');
}
```

## TypeScript Support

Full TypeScript definitions are included:

```typescript
import * as asterix from 'asterix-decoder';

asterix.init();

const data: Buffer = fs.readFileSync('sample.asterix');
const records: asterix.AsterixRecord[] = asterix.parse(data, {
  verbose: true,
  filterCategory: 62
});

for (const record of records) {
  console.log(`Category ${record.category}`);
  // Full type safety for record properties
}
```

## Data Types

### AsterixRecord

```typescript
interface AsterixRecord {
  category: number;           // ASTERIX category (1-255)
  length: number;             // Data block length in bytes
  timestamp_ms: number;       // Timestamp in milliseconds
  crc: number;                // CRC checksum
  hex_data?: string;          // Hex representation of raw data
  items: Record<string, any>; // Parsed data items
}
```

### ParseResult

```typescript
interface ParseResult {
  records: AsterixRecord[];   // Parsed records
  bytesConsumed: number;      // Bytes consumed from input
  remainingBlocks: number;    // Estimated remaining blocks
}
```

## Examples

### Basic Parsing

```javascript
const asterix = require('asterix-decoder');
const fs = require('fs');

asterix.init();

const data = fs.readFileSync('sample.asterix');
const records = asterix.parse(data);

console.log(`Parsed ${records.length} records`);
```

### Stream Processing

```javascript
const asterix = require('asterix-decoder');
const fs = require('fs');
const { Transform } = require('stream');

asterix.init();

class AsterixParser extends Transform {
  constructor() {
    super({ objectMode: true });
    this.buffer = Buffer.alloc(0);
  }

  _transform(chunk, encoding, callback) {
    this.buffer = Buffer.concat([this.buffer, chunk]);

    try {
      const result = asterix.parseWithOffset(this.buffer, 0, 100);

      for (const record of result.records) {
        this.push(record);
      }

      // Keep unparsed data
      if (result.bytesConsumed > 0) {
        this.buffer = this.buffer.slice(result.bytesConsumed);
      }

      callback();
    } catch (err) {
      callback(err);
    }
  }
}

// Usage
fs.createReadStream('large_file.asterix')
  .pipe(new AsterixParser())
  .on('data', (record) => {
    console.log(`Category ${record.category}:`, record.items);
  })
  .on('end', () => {
    console.log('Parsing complete');
  });
```

### Express.js REST API

```javascript
const express = require('express');
const asterix = require('asterix-decoder');
const multer = require('multer');

asterix.init();

const app = express();
const upload = multer();

app.post('/parse', upload.single('file'), (req, res) => {
  try {
    const records = asterix.parse(req.file.buffer, {
      filterCategory: parseInt(req.query.category) || undefined,
      maxRecords: parseInt(req.query.limit) || undefined
    });

    res.json({
      success: true,
      count: records.length,
      records: records
    });
  } catch (err) {
    res.status(400).json({
      success: false,
      error: err.message
    });
  }
});

app.get('/categories/:category/describe', (req, res) => {
  try {
    const description = asterix.describe(
      parseInt(req.params.category),
      req.query.item,
      req.query.field,
      req.query.value
    );

    res.json({
      success: true,
      description: description
    });
  } catch (err) {
    res.status(400).json({
      success: false,
      error: err.message
    });
  }
});

app.listen(3000, () => {
  console.log('ASTERIX API listening on port 3000');
});
```

### File Watcher

```javascript
const asterix = require('asterix-decoder');
const chokidar = require('chokidar');
const fs = require('fs');

asterix.init();

chokidar.watch('*.asterix').on('add', (path) => {
  console.log(`Processing new file: ${path}`);

  const data = fs.readFileSync(path);
  const records = asterix.parse(data);

  console.log(`Parsed ${records.length} records`);

  // Process records...
  for (const record of records) {
    // Store in database, send to API, etc.
  }
});
```

## Supported ASTERIX Categories

The decoder supports 67 ASTERIX categories, including:

- **CAT001**: Monoradar Target Reports
- **CAT002**: Monoradar Target Reports (secondary)
- **CAT008**: Monoradar Network Reports
- **CAT010**: Transmission of Monoradar Service Messages
- **CAT019**: Multilateration System Status
- **CAT020**: Multilateration Target Reports
- **CAT021**: ADS-B Target Reports
- **CAT023**: CNS/ATM Ground Station Reports
- **CAT025**: CNS/ATM System Status
- **CAT034**: Monopulse Radar Messages
- **CAT048**: Transmission of Monoradar Data
- **CAT062**: System Track Data
- **CAT063**: Sensor Status Messages
- **CAT065**: SDPS Service Status Messages
- And many more...

Full list: https://www.eurocontrol.int/asterix

## Error Handling

All functions throw errors for invalid inputs or parsing failures:

```javascript
try {
  const records = asterix.parse(data);
} catch (err) {
  if (err instanceof TypeError) {
    console.error('Invalid input:', err.message);
  } else {
    console.error('Parsing error:', err.message);
  }
}
```

## Performance

The Node.js bindings use a high-performance C++ core with:

- **55-61% cumulative speedup** from recent optimizations
- Zero-copy buffer passing for input data
- Efficient N-API bindings (stable ABI)
- Memory-safe with comprehensive bounds checking
- Optimized for large file processing

**Typical Performance:**
- Parse 1 MB of ASTERIX data: ~5-10ms
- Parse 10,000 records: ~50-100ms
- Memory usage: ~50 MB peak

## Testing

Run the test suite:

```bash
npm test
```

Generate coverage report:

```bash
npm run test:coverage
```

Run benchmarks:

```bash
npm run benchmark
```

## License

GPL-3.0-or-later

See [LICENSE](../LICENSE) for details.

## Related Projects

- **Python Bindings**: `pip install asterix_decoder`
- **Rust Bindings**: `cargo add asterix-decoder`
- **C++ CLI**: Build from source in `install/bin/asterix`

## Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

## Support

- **Issues**: https://github.com/montge/asterix/issues
- **Documentation**: https://montge.github.io/asterix/
- **Source**: https://github.com/montge/asterix

## Acknowledgments

Based on the ASTERIX decoder by Croatia Control Ltd.

## Safety-Critical Use

This library follows safety-critical design patterns aligned with DO-278A guidelines for CNS/ATM systems:

- Validated FFI boundary with comprehensive input checking
- Integer overflow prevention
- Buffer bounds checking
- Memory safety (zero leaks)
- >80% test coverage
- Static analysis clean (ESLint, security audits)

See [SAFETY_CRITICAL.md](../docs/SAFETY_CRITICAL.md) for details.
