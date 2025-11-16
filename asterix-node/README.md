# ASTERIX Decoder - Node.js Bindings

[![npm version](https://img.shields.io/npm/v/asterix-decoder.svg)](https://www.npmjs.com/package/asterix-decoder)
[![License: GPL-3.0](https://img.shields.io/badge/License-GPL%203.0-blue.svg)](../LICENSE)
[![Node.js >= 20](https://img.shields.io/badge/node-%3E%3D20.0.0-brightgreen.svg)](https://nodejs.org/)

**High-performance Node.js bindings for the ASTERIX protocol decoder.**

ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is a binary protocol used worldwide for Air Traffic Management (ATM) surveillance data exchange. This package provides native Node.js bindings to the high-performance C++ ASTERIX decoder core.

---

## Features

- **Native Performance** - C++ core with N-API bindings for maximum throughput
- **Comprehensive Category Support** - 24 ASTERIX categories (CAT001-CAT252)
- **Memory Safe** - Automatic memory management, no leaks
- **TypeScript Support** - Full TypeScript definitions included
- **Incremental Parsing** - Memory-efficient streaming for large datasets
- **Flexible API** - Parse from files, buffers, streams, or network
- **Production Ready** - >80% test coverage, fuzz tested, memory checked

---

## Installation

### From npm (Recommended)

```bash
npm install asterix-decoder
```

### From Source

```bash
# Clone repository
git clone https://github.com/montge/asterix.git
cd asterix/asterix-node

# Install dependencies and build
npm install

# Run tests
npm test
```

**Requirements:**
- Node.js >= 20.0.0
- C++ compiler (GCC 7+, Clang 5+, MSVC 2019+)
- CMake 3.20+ (for building C++ core)
- libexpat-devel (XML parsing)

---

## Quick Start

### Basic Parsing

```javascript
const asterix = require('asterix-decoder');
const fs = require('fs');

// Initialize parser (optional - auto-initializes on first use)
asterix.init();

// Parse ASTERIX data from file
const data = fs.readFileSync('capture.asterix');
const records = asterix.parse(data, { verbose: true });

// Process records
for (const record of records) {
  console.log(`Category: ${record.category}`);
  console.log(`Items: ${JSON.stringify(record.items, null, 2)}`);
}
```

### Incremental Parsing (Large Files)

```javascript
const asterix = require('asterix-decoder');
const fs = require('fs');

const data = fs.readFileSync('large_file.asterix');
let offset = 0;
const chunkSize = 100; // Process 100 blocks at a time

while (offset < data.length) {
  const result = asterix.parseWithOffset(data, offset, chunkSize);

  // Process records
  for (const record of result.records) {
    console.log(`CAT${record.category}: ${Object.keys(record.items).length} items`);
  }

  // Update offset
  offset = result.bytesConsumed;

  // Check if done
  if (result.remainingBlocks === 0) {
    break;
  }
}
```

### TypeScript

```typescript
import * as asterix from 'asterix-decoder';
import { ParseOptions, AsterixRecord, ParseResult } from 'asterix-decoder';

// Parse with options
const options: ParseOptions = {
  verbose: true,
  filterCategory: 48,
  maxRecords: 100
};

const records: AsterixRecord[] = asterix.parse(buffer, options);

// Incremental parsing
const result: ParseResult = asterix.parseWithOffset(buffer, 0, 10);
```

---

## API Reference

### `init(configDir?: string): void`

Initialize the ASTERIX parser with category definitions.

**Parameters:**
- `configDir` (optional): Path to ASTERIX category XML configuration directory. If not provided, uses default bundled configurations.

**Example:**
```javascript
asterix.init(); // Use default config
asterix.init('/path/to/asterix/config'); // Custom config
```

**Throws:**
- `TypeError` - Invalid config directory path
- `Error` - Failed to initialize (missing/invalid config files)

---

### `parse(data: Buffer, options?: ParseOptions): AsterixRecord[]`

Parse ASTERIX data from a buffer.

**Parameters:**
- `data`: Buffer containing ASTERIX binary data
- `options` (optional): Parsing configuration
  - `verbose` (boolean): Include descriptions in output (default: `false`)
  - `filterCategory` (number): Only parse records of this category (1-255)
  - `maxRecords` (number): Limit number of records parsed

**Returns:** Array of `AsterixRecord` objects

**Example:**
```javascript
const records = asterix.parse(buffer, {
  verbose: true,
  filterCategory: 62,
  maxRecords: 1000
});
```

**Throws:**
- `TypeError` - Invalid input (not a Buffer, empty, or too large)
- `Error` - Parsing failed (invalid ASTERIX data)

---

### `parseWithOffset(data: Buffer, offset: number, blocksCount: number, options?: ParseOptions): ParseResult`

Parse ASTERIX data incrementally from a specific offset.

**Parameters:**
- `data`: Buffer containing ASTERIX binary data
- `offset`: Starting byte offset (0-indexed)
- `blocksCount`: Maximum number of data blocks to parse
- `options` (optional): Same as `parse()`

**Returns:** `ParseResult` object
- `records`: Array of parsed records
- `bytesConsumed`: Total bytes read (new offset)
- `remainingBlocks`: Number of unparsed blocks remaining

**Example:**
```javascript
let offset = 0;
while (offset < data.length) {
  const result = asterix.parseWithOffset(data, offset, 10);

  processRecords(result.records);

  offset = result.bytesConsumed;
  if (result.remainingBlocks === 0) break;
}
```

**Throws:**
- `TypeError` - Invalid parameters (offset exceeds data length, negative values)
- `Error` - Parsing failed

---

### `describe(category: number, item?: string, field?: string, value?: string): string`

Get human-readable descriptions for ASTERIX categories, data items, fields, or values.

**Parameters:**
- `category`: ASTERIX category number (1-255)
- `item` (optional): Data item number (e.g., "010" for I048/010)
- `field` (optional): Field name within the item
- `value` (optional): Value to describe

**Returns:** Description string

**Examples:**
```javascript
// Category description
const catDesc = asterix.describe(48);
// "Monoradar Target Reports"

// Item description
const itemDesc = asterix.describe(48, '010');
// "Data Source Identifier"

// Field value description
const valueDesc = asterix.describe(48, '020', 'TYP', '0');
// "Plot"
```

**Throws:**
- `TypeError` - Invalid category (not in range 1-255)
- `Error` - Category/item not defined

---

### `isCategoryDefined(category: number): boolean`

Check if an ASTERIX category is loaded and available.

**Parameters:**
- `category`: ASTERIX category number (1-255)

**Returns:** `true` if category is defined, `false` otherwise

**Example:**
```javascript
if (asterix.isCategoryDefined(48)) {
  console.log('CAT048 is available');
}
```

---

### `loadCategory(xmlPath: string): void`

Load a custom ASTERIX category definition from an XML file.

**Parameters:**
- `xmlPath`: Path to category XML definition file

**Example:**
```javascript
asterix.loadCategory('/path/to/asterix_cat048_custom.xml');
```

**Throws:**
- `TypeError` - Invalid path (path traversal, too long)
- `Error` - Failed to load (file not found, invalid XML)

---

### `version: string`

Package version string (e.g., "2.8.10")

**Example:**
```javascript
console.log(`ASTERIX Decoder v${asterix.version}`);
```

---

## Data Types

### `AsterixRecord`

```typescript
interface AsterixRecord {
  category: number;           // ASTERIX category (1-255)
  length: number;             // Record length in bytes
  timestamp?: string;         // Timestamp (if present)
  items: {
    [key: string]: any;       // Data items (e.g., "I048/010")
  };
}
```

### `ParseOptions`

```typescript
interface ParseOptions {
  verbose?: boolean;          // Include descriptions (default: false)
  filterCategory?: number;    // Filter by category (1-255)
  maxRecords?: number;        // Limit number of records
}
```

### `ParseResult`

```typescript
interface ParseResult {
  records: AsterixRecord[];   // Parsed records
  bytesConsumed: number;      // Total bytes read
  remainingBlocks: number;    // Unparsed blocks remaining
}
```

---

## Examples

The `examples/` directory contains comprehensive examples:

### 1. **parse_pcap.js** - Parse PCAP files
```javascript
const asterix = require('asterix-decoder');
const fs = require('fs');

const pcapData = fs.readFileSync('capture.pcap');
const records = asterix.parse(pcapData, { verbose: true });

console.log(`Parsed ${records.length} records`);
```

### 2. **incremental_parsing.js** - Memory-efficient chunked parsing
```javascript
// Process large files in chunks
let offset = 0;
while (offset < data.length) {
  const result = asterix.parseWithOffset(data, offset, 100);
  processChunk(result.records);
  offset = result.bytesConsumed;
}
```

### 3. **stream_processing.js** - Node.js streams
```javascript
const { Transform } = require('stream');

class AsterixParser extends Transform {
  _transform(chunk, encoding, callback) {
    this.buffer = Buffer.concat([this.buffer, chunk]);
    const result = asterix.parseWithOffset(this.buffer, 0, 10);

    for (const record of result.records) {
      this.push(JSON.stringify(record) + '\n');
    }

    this.buffer = this.buffer.subarray(result.bytesConsumed);
    callback();
  }
}

fs.createReadStream('data.asterix')
  .pipe(new AsterixParser())
  .pipe(fs.createWriteStream('output.jsonl'));
```

### 4. **express_api.js** - REST API server
```javascript
const express = require('express');
const app = express();

app.post('/parse', (req, res) => {
  const records = asterix.parse(req.body, { verbose: true });
  res.json({ count: records.length, records });
});

app.listen(3000);
```

### 5. **file_watcher.js** - Real-time directory monitoring
```javascript
const chokidar = require('chokidar');

const watcher = chokidar.watch('./data/*.asterix');

watcher.on('add', (filePath) => {
  const data = fs.readFileSync(filePath);
  const records = asterix.parse(data);
  console.log(`Processed ${filePath}: ${records.length} records`);
});
```

**Run examples:**
```bash
cd examples
node parse_pcap.js
node incremental_parsing.js
node stream_processing.js
node express_api.js
node file_watcher.js
```

---

## Testing

### Run Tests

```bash
# All tests (unit + integration)
npm test

# Unit tests only
npm run test:unit

# Integration tests only
npm run test:integration

# Coverage report
npm run test:coverage
```

### Benchmarks

```bash
# Run performance benchmarks
npm run benchmark

# Run with memory profiling
npm run benchmark:memory
```

**Expected Performance:**
- Parse throughput: 5-50 MB/s (depends on data complexity)
- Incremental parsing overhead: <10%
- Memory usage: Stable, no leaks

---

## Supported ASTERIX Categories

**24 Categories:**
- CAT001 - Monoradar Target Reports
- CAT002 - Monoradar Target Reports
- CAT004 - Safety Nets
- CAT008 - Monoradar Derived Weather Information
- CAT010 - Transmission of Monosensor Surface Movement Data
- CAT011 - A-SMGCS Messages
- CAT015 - Surveillance Data Exchange
- CAT019 - Multilateration System Status
- CAT020 - Multilateration Target Reports
- CAT021 - ADS-B Target Reports
- CAT023 - CNS/ATM Ground Station Status
- CAT025 - CNS/ATM Ground Station Service Status
- CAT030 - ARTAS Messages
- CAT031 - ARTAS Messages
- CAT032 - Miniplan Messages
- CAT034 - Transmission of Monosensor Service Messages
- CAT048 - Monoradar Target Reports
- CAT062 - System Track Data
- CAT063 - Sensor Status Messages
- CAT065 - SDPS Service Status Messages
- CAT205 - Unknown
- CAT240 - Radar Video Transmission
- CAT247 - Asterix Category 247
- CAT252 - Unknown

---

## Performance Tips

### 1. Use Incremental Parsing for Large Files

```javascript
// ❌ BAD - Loads entire file into memory
const records = asterix.parse(fs.readFileSync('huge_file.asterix'));

// ✅ GOOD - Process in chunks
const fd = fs.openSync('huge_file.asterix', 'r');
const chunkSize = 65536; // 64 KB
const buffer = Buffer.alloc(chunkSize);

let offset = 0;
while (true) {
  const bytesRead = fs.readSync(fd, buffer, 0, chunkSize, offset);
  if (bytesRead === 0) break;

  const result = asterix.parseWithOffset(buffer.subarray(0, bytesRead), 0, 100);
  processRecords(result.records);

  offset += result.bytesConsumed;
}
fs.closeSync(fd);
```

### 2. Disable Verbose Mode for Production

```javascript
// ❌ BAD - Adds 20-30% overhead
const records = asterix.parse(data, { verbose: true });

// ✅ GOOD - Fast parsing without descriptions
const records = asterix.parse(data);
```

### 3. Use Category Filtering

```javascript
// ❌ BAD - Parses all categories then filters
const records = asterix.parse(data).filter(r => r.category === 48);

// ✅ GOOD - Filters during parsing
const records = asterix.parse(data, { filterCategory: 48 });
```

### 4. Use maxRecords for Pagination

```javascript
// ❌ BAD - Loads all records then slices
const page1 = asterix.parse(data).slice(0, 100);

// ✅ GOOD - Stops parsing after limit
const page1 = asterix.parse(data, { maxRecords: 100 });
```

---

## Thread Safety

**The ASTERIX decoder is thread-safe** with the following considerations:

### Safe Operations (Concurrent)
- `parse()` - Safe to call from multiple worker threads
- `parseWithOffset()` - Safe to call concurrently
- `isCategoryDefined()` - Safe
- `describe()` - Safe
- `version` - Safe

### Unsafe Operations (Sequential Only)
- `init()` - Call once at startup before worker threads
- `loadCategory()` - Call during initialization only

### Worker Threads Example

```javascript
const { Worker } = require('worker_threads');
const asterix = require('asterix-decoder');

// Initialize in main thread
asterix.init();

// Spawn workers
const workers = [];
for (let i = 0; i < 4; i++) {
  workers.push(new Worker('./worker.js'));
}

// worker.js - Each worker can safely parse
const { parentPort } = require('worker_threads');
const asterix = require('asterix-decoder');

parentPort.on('message', (data) => {
  const records = asterix.parse(data);
  parentPort.postMessage(records);
});
```

---

## Troubleshooting

### Build Errors

**"node-gyp not found":**
```bash
npm install -g node-gyp
npm install
```

**"ASTERIX core library not found":**
```bash
# Build C++ core first
cd ..
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build

# Then build Node.js bindings
cd asterix-node
npm install
```

**"Cannot find module 'asterix-decoder'":**
```bash
# Rebuild native addon
npm run build
```

### Runtime Errors

**TypeError: data must be a Buffer:**
```javascript
// ❌ BAD
asterix.parse("not a buffer");

// ✅ GOOD
asterix.parse(Buffer.from([0x30, 0x00, 0x06]));
```

**Error: Parser not initialized:**
```javascript
// ✅ Initialize explicitly
asterix.init();
const records = asterix.parse(data);
```

**Error: Data too large (max 64KB):**
```javascript
// ✅ Use incremental parsing
const result = asterix.parseWithOffset(largeData, 0, 100);
```

---

## Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

**Development Workflow:**
```bash
# Build in debug mode
npm run build:debug

# Run linter
npm run lint

# Format code
npm run format

# Run all tests
npm test

# Run benchmarks
npm run benchmark
```

---

## License

GPL-3.0-or-later - Same as the main ASTERIX decoder project.

See [LICENSE](../LICENSE) for details.

---

## Support

- **Issues**: https://github.com/montge/asterix/issues
- **Documentation**: https://montge.github.io/asterix/
- **npm**: https://www.npmjs.com/package/asterix-decoder

---

## Acknowledgments

Based on the ASTERIX decoder by Croatia Control Ltd.

ASTERIX specifications maintained by EUROCONTROL.
