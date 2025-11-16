# asterix-rs - Rust Bindings for ASTERIX Decoder

[![Rust CI/CD](https://github.com/CroatiaControlLtd/asterix/actions/workflows/rust-ci.yml/badge.svg)](https://github.com/CroatiaControlLtd/asterix/actions/workflows/rust-ci.yml)
[![codecov](https://codecov.io/gh/CroatiaControlLtd/asterix/branch/master/graph/badge.svg?flag=rust)](https://codecov.io/gh/CroatiaControlLtd/asterix)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Rust Version](https://img.shields.io/badge/rust-1.70%2B-blue.svg)](https://www.rust-lang.org)
[![crates.io](https://img.shields.io/badge/crates.io-asterix--decoder-orange.svg)](https://crates.io/)

> Type-safe, memory-safe Rust bindings for the ASTERIX ATM surveillance protocol decoder

## Overview

`asterix-rs` provides safe, idiomatic Rust bindings to the high-performance C++ ASTERIX decoder library. **ASTERIX** (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) is the binary protocol used worldwide for Air Traffic Management (ATM) surveillance data exchange.

This crate enables Rust developers to parse and process ASTERIX data with:
- **Type safety** - Strongly typed data structures with Rust's type system
- **Memory safety** - Zero unsafe code in user-facing API, safe FFI via CXX crate
- **Performance** - Near-native C++ performance with zero-copy operations where possible
- **Ergonomics** - Idiomatic Rust API with `Result<T, E>` error handling

## Features

- ✅ **24 ASTERIX categories** supported (CAT 001-252)
- ✅ **Multiple input formats**: Raw binary, PCAP, HDLC, FINAL, GPS
- ✅ **Safe FFI** via CXX crate (type-safe C++/Rust interop)
- ✅ **Comprehensive error handling** with detailed `AsterixError` enum
- ✅ **Incremental parsing** for large files and streaming data
- ✅ **Metadata queries** for category/item/field descriptions
- ✅ **Optional serde support** for JSON serialization (enable `serde` feature)
- ✅ **92.2% test coverage** | 560 passing tests | 0 memory leaks
- ✅ **Cross-platform**: Linux, Windows, macOS (Intel & ARM M1)

## Installation

### From crates.io (Recommended)

```toml
[dependencies]
asterix-decoder = "0.1.0"

# Optional: Enable serde support for JSON serialization
asterix-decoder = { version = "0.1.0", features = ["serde"] }
```

### From Source

```bash
git clone https://github.com/montge/asterix.git
cd asterix/asterix-rs
cargo build --release
```

### Build Requirements

**Rust:**
- Rust 1.70+ (2021 edition)
- Cargo build system

**C++ Compiler:**
- GCC 7+ (Linux)
- Clang 5+ (macOS)
- MSVC 2017+ (Windows)
- C++17 support required

**System Libraries:**
- `libexpat-devel` - XML parsing library

**Installation:**
```bash
# Ubuntu/Debian
sudo apt-get install libexpat1-dev

# macOS
brew install expat

# Windows (vcpkg)
vcpkg install expat:x64-windows
```

## Quick Start

```rust
use asterix_decoder::{init_default, parse, ParseOptions};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize ASTERIX with default config
    init_default()?;

    // Read ASTERIX data
    let data = std::fs::read("sample.asterix")?;

    // Parse with default options
    let records = parse(&data, ParseOptions::default())?;

    // Process parsed records
    for record in records {
        println!("Category {}: {} items", record.category, record.items.len());

        for item in &record.items {
            println!("  {}: {}", item.description, item.value);
        }
    }

    Ok(())
}
```

## API Reference

### Initialization Functions

#### `init_default() -> Result<(), AsterixError>`

Initialize ASTERIX decoder with default category definitions.

**Returns:**
- `Ok(())` - Initialization successful
- `Err(AsterixError::InitError)` - Failed to load default configurations

**Example:**
```rust
use asterix_decoder::init_default;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    init_default()?;
    println!("ASTERIX decoder initialized with default categories");
    Ok(())
}
```

**Note:** Call this **once** at program startup before parsing any data.

---

#### `Parser::new().add_category(path).build() -> Result<(), AsterixError>`

Initialize ASTERIX decoder with custom category definitions.

**Arguments:**
- `path: &str` - Path to custom ASTERIX category XML file

**Returns:**
- `Ok(())` - Initialization successful
- `Err(AsterixError::InitError)` - Failed to load custom configuration

**Example:**
```rust
use asterix_decoder::Parser;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize with custom category definition
    Parser::new()
        .add_category("/path/to/custom_asterix_cat062.xml")?
        .build()?;

    println!("ASTERIX decoder initialized with custom categories");
    Ok(())
}
```

**Use Cases:**
- Loading custom ASTERIX category definitions
- Using non-standard or experimental ASTERIX categories
- Development/testing with modified category XMLs

---

### Parsing Functions

#### `parse(data: &[u8], options: ParseOptions) -> Result<Vec<AsterixRecord>, AsterixError>`

Parse ASTERIX data from raw bytes.

**Arguments:**
- `data: &[u8]` - Raw ASTERIX binary data
- `options: ParseOptions` - Parsing configuration (format, verbosity)

**Returns:**
- `Ok(Vec<AsterixRecord>)` - Parsed ASTERIX records
- `Err(AsterixError::ParseError)` - Invalid ASTERIX data or parsing failure

**Example:**
```rust
use asterix_decoder::{parse, ParseOptions};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    asterix_decoder::init_default()?;

    // Read raw ASTERIX data
    let data = std::fs::read("capture.asterix")?;

    // Parse with default options
    let records = parse(&data, ParseOptions::default())?;

    println!("Parsed {} ASTERIX records", records.len());
    Ok(())
}
```

**Supported Input Formats:**
```rust
use asterix_decoder::{parse, ParseOptions, InputFormat};

// Raw ASTERIX binary (default)
let records = parse(&data, ParseOptions::default())?;

// PCAP packet capture
let records = parse(&pcap_data, ParseOptions {
    format: InputFormat::Pcap,
    verbose: true,
})?;

// HDLC framed data
let records = parse(&hdlc_data, ParseOptions {
    format: InputFormat::Hdlc,
    verbose: false,
})?;
```

---

#### `parse_with_offset(data: &[u8], offset: usize, count: usize, options: ParseOptions) -> Result<ParseResult, AsterixError>`

Parse ASTERIX data incrementally (streaming/large files).

**Arguments:**
- `data: &[u8]` - Raw ASTERIX binary data
- `offset: usize` - Byte offset to start parsing from
- `count: usize` - Maximum number of data blocks to parse
- `options: ParseOptions` - Parsing configuration

**Returns:**
- `Ok(ParseResult)` - Parsed records and bytes consumed
- `Err(AsterixError::ParseError)` - Invalid ASTERIX data or parsing failure

**ParseResult Structure:**
```rust
pub struct ParseResult {
    pub records: Vec<AsterixRecord>,  // Parsed ASTERIX records
    pub bytes_consumed: usize,         // Number of bytes consumed from data
}
```

**Example (Incremental Parsing):**
```rust
use asterix_decoder::{parse_with_offset, ParseOptions};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    asterix_decoder::init_default()?;

    let data = std::fs::read("large_capture.asterix")?;
    let mut offset = 0;
    let blocks_per_chunk = 100;

    loop {
        let result = parse_with_offset(&data, offset, blocks_per_chunk, ParseOptions::default())?;

        if result.records.is_empty() {
            break;  // No more data
        }

        // Process chunk
        for record in result.records {
            println!("CAT {}: {} items", record.category, record.items.len());
        }

        offset = result.bytes_consumed;
    }

    Ok(())
}
```

**Use Cases:**
- Processing large ASTERIX files that don't fit in memory
- Real-time streaming data (network multicast, live captures)
- Memory-constrained environments (embedded systems, IoT devices)

---

### Metadata Functions

#### `describe(category: u8, item: &str, field: &str, value: i64) -> Result<String, AsterixError>`

Get human-readable description for ASTERIX value.

**Arguments:**
- `category: u8` - ASTERIX category number (e.g., 48, 62)
- `item: &str` - Data item number (e.g., "010", "220")
- `field: &str` - Field name (e.g., "SAC", "WGS84")
- `value: i64` - Numeric value to describe

**Returns:**
- `Ok(String)` - Human-readable description
- `Err(AsterixError::DescribeError)` - Unknown category/item/field

**Example:**
```rust
use asterix_decoder::describe;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    asterix_decoder::init_default()?;

    // Get description for CAT 048, I010/SAC with value 7
    let description = describe(48, "010", "SAC", 7)?;
    println!("SAC=7: {}", description);

    // Example output: "SAC=7: System Area Code 7 (Paris ACC)"
    Ok(())
}
```

**Common Use Cases:**
```rust
// Decode surveillance tracker ID
let tracker_desc = describe(62, "040", "TRK", 1234)?;

// Decode flight level
let fl_desc = describe(48, "090", "FL", 350)?;

// Decode ground speed
let speed_desc = describe(21, "150", "GS", 450)?;
```

---

### Data Structures

#### `AsterixRecord`

Represents a single parsed ASTERIX record.

```rust
pub struct AsterixRecord {
    pub category: u8,               // ASTERIX category (e.g., 48, 62, 21)
    pub length: usize,              // Record length in bytes
    pub timestamp: Option<f64>,     // Optional timestamp (seconds)
    pub items: Vec<DataItem>,       // Parsed data items
}
```

**Example:**
```rust
for record in records {
    println!("Category: {}", record.category);
    println!("Length: {} bytes", record.length);

    if let Some(ts) = record.timestamp {
        println!("Timestamp: {:.3}s", ts);
    }

    println!("Items: {}", record.items.len());
}
```

---

#### `DataItem`

Represents a single data item within an ASTERIX record.

```rust
pub struct DataItem {
    pub description: String,        // Human-readable description (e.g., "Data Source Identifier")
    pub value: String,              // Parsed value as string
}
```

**Example:**
```rust
for item in &record.items {
    println!("{}: {}", item.description, item.value);
}

// Output:
// Data Source Identifier: SAC=7, SIC=15
// Target Report Descriptor: SSR=Yes, Plot=Yes
// Measured Position: Lat=45.123°, Lon=15.456°
```

---

#### `ParseOptions`

Configuration options for ASTERIX parsing.

```rust
pub struct ParseOptions {
    pub format: InputFormat,        // Input format (Raw, Pcap, Hdlc, Final, Gps)
    pub verbose: bool,              // Include detailed descriptions
}

impl Default for ParseOptions {
    fn default() -> Self {
        ParseOptions {
            format: InputFormat::Raw,
            verbose: true,
        }
    }
}
```

**Example:**
```rust
use asterix_decoder::{ParseOptions, InputFormat};

// Default options (Raw format, verbose descriptions)
let opts1 = ParseOptions::default();

// PCAP format with verbose descriptions
let opts2 = ParseOptions {
    format: InputFormat::Pcap,
    verbose: true,
};

// Raw format without descriptions (faster)
let opts3 = ParseOptions {
    format: InputFormat::Raw,
    verbose: false,
};
```

---

#### `InputFormat`

Enumeration of supported input formats.

```rust
pub enum InputFormat {
    Raw,     // Raw ASTERIX binary
    Pcap,    // PCAP packet capture (tcpdump/Wireshark)
    Hdlc,    // HDLC framed data (serial lines)
    Final,   // FINAL packet format
    Gps,     // GPS packet format
}
```

---

#### `AsterixError`

Comprehensive error types for ASTERIX operations.

```rust
pub enum AsterixError {
    InitError(String),         // Initialization failure
    ParseError(String),        // Parsing failure
    IoError(String),           // I/O error
    DescribeError(String),     // Metadata lookup failure
    ConfigError(String),       // Configuration error
}

impl std::fmt::Display for AsterixError {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            AsterixError::InitError(msg) => write!(f, "Initialization error: {}", msg),
            AsterixError::ParseError(msg) => write!(f, "Parse error: {}", msg),
            AsterixError::IoError(msg) => write!(f, "I/O error: {}", msg),
            AsterixError::DescribeError(msg) => write!(f, "Describe error: {}", msg),
            AsterixError::ConfigError(msg) => write!(f, "Configuration error: {}", msg),
        }
    }
}

impl std::error::Error for AsterixError {}
```

**Example (Error Handling):**
```rust
use asterix_decoder::{init_default, parse, ParseOptions, AsterixError};

fn process_asterix() -> Result<(), AsterixError> {
    init_default()?;

    let data = std::fs::read("capture.asterix")
        .map_err(|e| AsterixError::IoError(e.to_string()))?;

    let records = parse(&data, ParseOptions::default())?;

    for record in records {
        println!("CAT {}", record.category);
    }

    Ok(())
}

fn main() {
    match process_asterix() {
        Ok(_) => println!("Success"),
        Err(AsterixError::InitError(msg)) => eprintln!("Init failed: {}", msg),
        Err(AsterixError::ParseError(msg)) => eprintln!("Parse failed: {}", msg),
        Err(e) => eprintln!("Error: {}", e),
    }
}
```

---

## Working Examples

### Example 1: Parse Raw ASTERIX File

```rust
use asterix_decoder::{init_default, parse, ParseOptions};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize decoder
    init_default()?;

    // Read raw ASTERIX binary file
    let data = std::fs::read("capture.asterix")?;

    // Parse
    let records = parse(&data, ParseOptions::default())?;

    // Display results
    println!("Parsed {} ASTERIX records", records.len());

    for record in records {
        println!("\n=== Category {} ===", record.category);
        for item in &record.items {
            println!("  {}: {}", item.description, item.value);
        }
    }

    Ok(())
}
```

---

### Example 2: Parse PCAP Capture File

```rust
use asterix_decoder::{init_default, parse, ParseOptions, InputFormat};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    init_default()?;

    // Read PCAP file (tcpdump/Wireshark capture)
    let pcap_data = std::fs::read("capture.pcap")?;

    // Parse PCAP format
    let records = parse(&pcap_data, ParseOptions {
        format: InputFormat::Pcap,
        verbose: true,
    })?;

    println!("Parsed {} records from PCAP", records.len());

    // Count records by category
    let mut counts = std::collections::HashMap::new();
    for record in &records {
        *counts.entry(record.category).or_insert(0) += 1;
    }

    for (cat, count) in counts {
        println!("Category {}: {} records", cat, count);
    }

    Ok(())
}
```

---

### Example 3: Incremental Parsing (Large Files/Streaming)

```rust
use asterix_decoder::{init_default, parse_with_offset, ParseOptions};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    init_default()?;

    // Read large ASTERIX file
    let data = std::fs::read("large_capture.asterix")?;

    let mut offset = 0;
    let blocks_per_chunk = 100;  // Process 100 blocks at a time
    let mut total_records = 0;

    loop {
        // Parse next chunk
        let result = parse_with_offset(&data, offset, blocks_per_chunk, ParseOptions::default())?;

        if result.records.is_empty() {
            break;  // No more data
        }

        // Process chunk
        total_records += result.records.len();
        println!("Processed chunk: {} records, {} bytes consumed",
                 result.records.len(), result.bytes_consumed);

        // Update offset for next chunk
        offset = result.bytes_consumed;
    }

    println!("Total records processed: {}", total_records);
    Ok(())
}
```

---

### Example 4: Real-Time Network Stream Processing

```rust
use asterix_decoder::{init_default, parse, ParseOptions, InputFormat};
use std::net::UdpSocket;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    init_default()?;

    // Bind to multicast address
    let socket = UdpSocket::bind("0.0.0.0:21112")?;
    socket.join_multicast_v4(&"239.0.0.1".parse()?, &"0.0.0.0".parse()?)?;

    println!("Listening for ASTERIX multicast on 239.0.0.1:21112...");

    let mut buf = vec![0u8; 65536];

    loop {
        // Receive UDP packet
        let (amt, _src) = socket.recv_from(&mut buf)?;

        // Parse ASTERIX data from packet
        let records = parse(&buf[..amt], ParseOptions::default())?;

        // Process records
        for record in records {
            println!("CAT {}: {} items", record.category, record.items.len());
        }
    }
}
```

---

### Example 5: JSON Export with Serde

```rust
use asterix_decoder::{init_default, parse, ParseOptions};
use serde_json;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    init_default()?;

    let data = std::fs::read("capture.asterix")?;
    let records = parse(&data, ParseOptions::default())?;

    // Serialize to JSON (requires "serde" feature)
    let json = serde_json::to_string_pretty(&records)?;

    // Write to file
    std::fs::write("output.json", json)?;

    println!("Exported {} records to output.json", records.len());
    Ok(())
}
```

**Enable serde feature in Cargo.toml:**
```toml
[dependencies]
asterix-decoder = { version = "0.1.0", features = ["serde"] }
serde_json = "1.0"
```

---

### Example 6: Custom Category Definitions

```rust
use asterix_decoder::Parser;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize with custom category XML
    Parser::new()
        .add_category("config/custom_asterix_cat062.xml")?
        .add_category("config/experimental_cat999.xml")?
        .build()?;

    println!("Loaded custom ASTERIX categories");

    // Now parse with custom categories
    let data = std::fs::read("custom_data.asterix")?;
    let records = asterix_decoder::parse(&data, asterix_decoder::ParseOptions::default())?;

    println!("Parsed {} records with custom categories", records.len());
    Ok(())
}
```

---

### Example 7: Error Handling and Recovery

```rust
use asterix_decoder::{init_default, parse, ParseOptions, AsterixError};

fn process_asterix_file(path: &str) -> Result<usize, AsterixError> {
    // Read file
    let data = std::fs::read(path)
        .map_err(|e| AsterixError::IoError(format!("Failed to read {}: {}", path, e)))?;

    // Parse
    let records = parse(&data, ParseOptions::default())?;

    Ok(records.len())
}

fn main() {
    // Initialize once
    if let Err(e) = init_default() {
        eprintln!("FATAL: Failed to initialize ASTERIX decoder: {}", e);
        std::process::exit(1);
    }

    // Process multiple files with error recovery
    let files = vec!["capture1.asterix", "capture2.asterix", "capture3.asterix"];

    for file in files {
        match process_asterix_file(file) {
            Ok(count) => println!("{}: {} records", file, count),
            Err(AsterixError::IoError(msg)) => eprintln!("ERROR: {}", msg),
            Err(AsterixError::ParseError(msg)) => eprintln!("PARSE ERROR in {}: {}", file, msg),
            Err(e) => eprintln!("ERROR processing {}: {}", file, e),
        }
    }
}
```

---

## Thread Safety

**⚠️ WARNING: This library is NOT thread-safe.**

The underlying C++ decoder uses global singletons for category definitions and internal state. **Do NOT call ASTERIX functions from multiple threads concurrently.**

### ❌ Unsafe (Race Conditions)

```rust
use asterix_decoder::{init_default, parse, ParseOptions};
use std::thread;

// DANGER: This will cause race conditions and data corruption!
fn main() {
    init_default().unwrap();

    let data1 = vec![0x30, 0x00, 0x30];
    let data2 = vec![0x3e, 0x00, 0x3e];

    let handle1 = thread::spawn(move || {
        parse(&data1, ParseOptions::default()).unwrap();  // ❌ UNSAFE
    });

    let handle2 = thread::spawn(move || {
        parse(&data2, ParseOptions::default()).unwrap();  // ❌ UNSAFE
    });

    handle1.join().unwrap();
    handle2.join().unwrap();
}
```

### ✅ Solution 1: External Locking (Mutex)

```rust
use asterix_decoder::{init_default, parse, ParseOptions};
use std::sync::{Arc, Mutex};
use std::thread;

// Wrap ASTERIX decoder in a Mutex for thread-safe access
struct AsterixDecoder;

impl AsterixDecoder {
    fn parse(&self, data: &[u8]) -> Result<Vec<asterix_decoder::AsterixRecord>, asterix_decoder::AsterixError> {
        parse(data, ParseOptions::default())
    }
}

fn main() {
    init_default().unwrap();

    let decoder = Arc::new(Mutex::new(AsterixDecoder));

    let mut handles = vec![];

    for i in 0..4 {
        let decoder = Arc::clone(&decoder);
        let data = std::fs::read(format!("capture{}.asterix", i)).unwrap();

        let handle = thread::spawn(move || {
            let decoder = decoder.lock().unwrap();
            decoder.parse(&data).unwrap()
        });

        handles.push(handle);
    }

    for handle in handles {
        let records = handle.join().unwrap();
        println!("Parsed {} records", records.len());
    }
}
```

### ✅ Solution 2: Single-Threaded with Async/Await

```rust
use asterix_decoder::{init_default, parse, ParseOptions};
use tokio;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    init_default()?;

    // Process files sequentially in async context (no thread safety issues)
    let files = vec!["capture1.asterix", "capture2.asterix", "capture3.asterix"];

    for file in files {
        let data = tokio::fs::read(file).await?;
        let records = parse(&data, ParseOptions::default())?;
        println!("{}: {} records", file, records.len());
    }

    Ok(())
}
```

### ✅ Solution 3: Process-Level Parallelism

```rust
// Use multiple processes instead of threads
use std::process::Command;

fn main() {
    let files = vec!["capture1.asterix", "capture2.asterix", "capture3.asterix"];

    let mut children = vec![];

    for file in files {
        let child = Command::new("./parse_asterix")
            .arg(file)
            .spawn()
            .expect("Failed to spawn process");
        children.push(child);
    }

    for mut child in children {
        child.wait().expect("Process failed");
    }
}
```

**Recommendation:** If you need parallelism, use **Solution 1 (Mutex)** for simplicity or **Solution 3 (multiprocessing)** for best performance.

---

## Performance Optimization

### 1. Disable Verbose Descriptions for High-Throughput

```rust
use asterix_decoder::{parse, ParseOptions, InputFormat};

// 10-15% faster parsing without verbose descriptions
let opts = ParseOptions {
    format: InputFormat::Raw,
    verbose: false,  // Disable descriptions for speed
};

let records = parse(&data, opts)?;
```

**Use Cases:**
- Real-time streaming (multicast, live captures)
- High-throughput batch processing
- Memory-constrained environments

---

### 2. Use Incremental Parsing for Large Files

```rust
use asterix_decoder::parse_with_offset;

let mut offset = 0;
let chunk_size = 1000;  // Tune based on available memory

loop {
    let result = parse_with_offset(&data, offset, chunk_size, ParseOptions::default())?;

    if result.records.is_empty() {
        break;
    }

    // Process chunk immediately (reduces memory usage)
    process_chunk(result.records);

    offset = result.bytes_consumed;
}
```

**Benefits:**
- Constant memory usage (no need to load entire file)
- Lower latency (start processing before file is fully parsed)
- Better cache locality

---

### 3. Preallocate Buffers for Network Streams

```rust
use std::net::UdpSocket;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let socket = UdpSocket::bind("0.0.0.0:21112")?;

    // Preallocate buffer (avoid reallocation)
    let mut buf = vec![0u8; 65536];  // Max UDP packet size

    loop {
        let (amt, _) = socket.recv_from(&mut buf)?;
        let records = asterix_decoder::parse(&buf[..amt], ParseOptions::default())?;
        // Process records...
    }
}
```

---

### 4. Batch Processing for Large Datasets

```rust
use rayon::prelude::*;
use std::sync::Mutex;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    asterix_decoder::init_default()?;

    let files: Vec<_> = std::fs::read_dir("./captures")?
        .filter_map(|e| e.ok())
        .collect();

    // Use Mutex for thread-safe parsing
    let decoder = Mutex::new(());

    // Process files in parallel
    let results: Vec<_> = files.par_iter().map(|entry| {
        let data = std::fs::read(entry.path()).unwrap();

        // Lock decoder for thread safety
        let _lock = decoder.lock().unwrap();
        asterix_decoder::parse(&data, ParseOptions::default()).unwrap()
    }).collect();

    println!("Processed {} files", results.len());
    Ok(())
}
```

**Requires Cargo.toml:**
```toml
[dependencies]
rayon = "1.7"
```

---

### 5. Memory-Mapped Files for Very Large Files

```rust
use memmap2::Mmap;
use std::fs::File;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    asterix_decoder::init_default()?;

    // Memory-map large file (avoids loading into RAM)
    let file = File::open("huge_capture.asterix")?;
    let mmap = unsafe { Mmap::map(&file)? };

    // Parse directly from memory-mapped region
    let records = asterix_decoder::parse(&mmap, ParseOptions::default())?;

    println!("Parsed {} records", records.len());
    Ok(())
}
```

**Requires Cargo.toml:**
```toml
[dependencies]
memmap2 = "0.7"
```

---

## Supported ASTERIX Categories

**24 Categories Fully Supported** (as of v2.8.10):

| Category | Version | Description |
|----------|---------|-------------|
| CAT 001 | v1.2 | Monoradar Target Reports |
| CAT 002 | v1.0 | Monoradar Target Messages |
| CAT 004 | v1.12 | Safety Net Messages |
| CAT 008 | v1.0 | Monoradar Derived Weather Information |
| CAT 010 | v1.1 | Monoradar Service Messages |
| CAT 011 | v1.2 | Monoradar Alert Messages |
| CAT 015 | v1.2 | INCS Target Reports |
| CAT 019 | v1.3 | Multilateration System Status |
| CAT 020 | v1.10 | Multilateration Target Reports |
| CAT 021 | v2.6 | ADS-B Target Reports |
| CAT 023 | v1.3 | CNS/ATM Ground Station Status |
| CAT 025 | v1.5 | CNS/ATM Service Status Reports |
| CAT 030 | v6.2 | ARTAS FPSDI |
| CAT 031 | v6.2 | ARTAS Picture Integrity |
| CAT 032 | v7.0 | ARTAS Miniplan |
| CAT 034 | v1.27 | Monoradar Service Messages |
| CAT 048 | v1.21 | Monoradar Target Reports |
| CAT 062 | v1.18 | System Track Data |
| CAT 063 | v1.3 | Sensor Status Messages |
| CAT 065 | v1.3 | SDPS Service Status Messages |
| CAT 205 | v1.0 | Area Proximity Warnings |
| CAT 240 | v1.3 | Radar Video Transmission |
| CAT 247 | v1.2 | Fixed Transponder Information |
| CAT 252 | v7.0 | ARTAS Operational Service Configuration |

See the [main README](../README.md) for the complete category support matrix and version gaps.

---

## Testing

This crate maintains high quality standards with comprehensive testing:

### Run Tests

```bash
# Unit + integration tests
cargo test

# Run tests with all features
cargo test --all-features

# Run specific test
cargo test test_parse_cat062

# Run with verbose output
cargo test -- --nocapture
```

### Code Coverage

```bash
# Using cargo-tarpaulin
cargo install cargo-tarpaulin
cargo tarpaulin --all-features --out Html

# Using cargo-llvm-cov
cargo install cargo-llvm-cov
cargo llvm-cov --all-features --html
```

**Coverage Requirements:**
- Minimum threshold: 80% per module
- Target coverage: 90% overall
- CI enforcement: Builds fail if coverage drops below 80%

### Benchmarks

```bash
# Run performance benchmarks
cargo bench

# Run specific benchmark
cargo bench parse_cat062
```

### Code Quality

```bash
# Format code
cargo fmt

# Check formatting
cargo fmt --check

# Clippy lints (zero warnings policy)
cargo clippy -- -D warnings

# Security audit
cargo install cargo-audit
cargo audit
```

---

## Build System

The Rust crate uses a hybrid build system:

1. **build.rs** - Build script that invokes CMake to compile C++ core
2. **CXX crate** - Type-safe C++/Rust FFI bridge
3. **Cargo** - Rust package manager and build orchestrator

**Build Process:**
```
cargo build → build.rs → CMake → compile C++ → CXX bridge → link Rust
```

**Dependencies:**
- C++ compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.20+ (automatically invoked by build.rs)
- libexpat (XML parsing)

**Custom Build Flags:**
```bash
# Debug build with symbols
cargo build

# Release build with optimizations
cargo build --release

# Enable all features (serde, etc.)
cargo build --all-features
```

---

## Troubleshooting

### Build Failures

**Problem:** `error: linking with 'cc' failed`

**Solution:**
```bash
# Install C++ compiler
sudo apt-get install build-essential  # Ubuntu/Debian
brew install gcc                       # macOS
```

---

**Problem:** `error: failed to run custom build command for 'asterix-rs'`

**Solution:**
```bash
# Install CMake
sudo apt-get install cmake  # Ubuntu/Debian
brew install cmake          # macOS

# Verify CMake version (3.20+ required)
cmake --version
```

---

**Problem:** `error: could not find native library 'expat'`

**Solution:**
```bash
# Install expat library
sudo apt-get install libexpat1-dev  # Ubuntu/Debian
brew install expat                  # macOS
vcpkg install expat:x64-windows     # Windows
```

---

### Runtime Errors

**Problem:** `AsterixError::InitError("Failed to load category definitions")`

**Solution:**
```rust
// Ensure you call init_default() before parsing
asterix_decoder::init_default()?;

// Verify config files exist
std::path::Path::new("asterix/config/asterix.ini").exists()
```

---

**Problem:** `AsterixError::ParseError("Invalid ASTERIX data block")`

**Causes:**
- Corrupted data file
- Wrong input format (forgot to set `InputFormat::Pcap` for PCAP files)
- Unsupported ASTERIX category

**Solution:**
```rust
// For PCAP files, specify format
let records = parse(&data, ParseOptions {
    format: InputFormat::Pcap,
    verbose: true,
})?;

// Verify file integrity
let data = std::fs::read("capture.asterix")?;
println!("File size: {} bytes", data.len());
```

---

**Problem:** Segmentation fault or crash

**Causes:**
- Thread safety violation (concurrent parsing from multiple threads)
- Calling parse() before init_default()
- Memory corruption in FFI boundary

**Solution:**
```rust
// 1. Always call init_default() first
asterix_decoder::init_default()?;

// 2. Use Mutex for thread safety
use std::sync::Mutex;
let decoder = Mutex::new(());

// 3. Report bug with backtrace
RUST_BACKTRACE=1 cargo run
```

---

### Performance Issues

**Problem:** Slow parsing (< 100,000 records/sec)

**Solutions:**
```rust
// 1. Disable verbose descriptions
let opts = ParseOptions {
    format: InputFormat::Raw,
    verbose: false,  // 10-15% faster
};

// 2. Use incremental parsing
let result = parse_with_offset(&data, offset, 1000, opts)?;

// 3. Use release build
cargo build --release  // 10x faster than debug build

// 4. Profile with cargo flamegraph
cargo install flamegraph
cargo flamegraph --bench parse_benchmark
```

---

For more comprehensive troubleshooting, see:
- [Main Troubleshooting Guide](../TROUBLESHOOTING.md) - Build, runtime, memory, network issues
- [FAQ](../FAQ.md) - Frequently asked questions

---

## Quality Standards

This project maintains high quality standards matching the C++ and Python implementations:

### Code Coverage
- **Minimum threshold**: 80% per module
- **Current coverage**: 92.2%
- **Target coverage**: 90% overall
- **Tools**: Both `cargo-tarpaulin` and `cargo-llvm-cov`
- **CI enforcement**: Builds fail if coverage drops below 80%

### Code Quality
- **Formatting**: Enforced via `cargo fmt` (rustfmt)
- **Linting**: Zero warnings policy with `cargo clippy -D warnings`
- **Documentation**: All public APIs documented, warnings treated as errors
- **Security**: Regular audits with `cargo-audit`
- **Dependencies**: Monitored for outdated packages with `cargo-outdated`

### Testing
- Unit tests for all core functionality
- Integration tests with real ASTERIX data
- Property-based testing where applicable
- Memory safety verified with Valgrind
- Cross-platform testing (Linux, macOS, Windows)

### Continuous Integration
All PRs must pass:
- ✅ Test suite (stable and nightly Rust)
- ✅ Coverage checks (80% minimum)
- ✅ Clippy lints (zero warnings)
- ✅ Format verification
- ✅ Documentation build
- ✅ Security audit
- ✅ Memory safety checks

---

## Contributing

Contributions are welcome! Please follow these guidelines:

### Before Submitting PRs

1. **Run full test suite:**
   ```bash
   cargo test --all-features
   cargo bench
   ```

2. **Check code quality:**
   ```bash
   cargo fmt --check
   cargo clippy -- -D warnings
   cargo audit
   ```

3. **Verify coverage:**
   ```bash
   cargo tarpaulin --all-features
   # Ensure coverage remains above 80%
   ```

4. **Update documentation:**
   ```bash
   cargo doc --no-deps --open
   # Verify all public APIs are documented
   ```

5. **Add tests for new functionality**

6. **Follow existing code style**

See the [Contributing Guide](../CONTRIBUTING.md) for complete guidelines.

---

## License

This program is free software; you can redistribute it and/or modify it under the terms of the **GNU General Public License v3.0 or later** as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but **WITHOUT ANY WARRANTY**; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See [LICENSE](../LICENSE) for details.

**Key Points:**
- ✅ Free to use for any purpose (commercial, personal, research)
- ✅ Free to modify and distribute
- ⚠️ Derivative works must also be GPL-3.0-or-later
- ⚠️ Linking/using this library makes your code GPL-3.0-or-later

For questions about licensing, see [FAQ](../FAQ.md) or consult a lawyer.

---

## Additional Resources

### Documentation
- **Main README**: [../README.md](../README.md) - Complete project overview
- **Python Module Guide**: [../asterix/README.md](../asterix/README.md) - Python API reference
- **C++ Executable Guide**: [../docs/CPP_USER_GUIDE.md](../docs/CPP_USER_GUIDE.md) - Command-line reference
- **Troubleshooting Guide**: [../TROUBLESHOOTING.md](../TROUBLESHOOTING.md) - Comprehensive troubleshooting
- **FAQ**: [../FAQ.md](../FAQ.md) - Frequently asked questions
- **Architecture**: [../CLAUDE.md](../CLAUDE.md) - Technical architecture
- **Contributing**: [../CONTRIBUTING.md](../CONTRIBUTING.md) - Development guidelines

### External Resources
- **ASTERIX Protocol**: http://www.eurocontrol.int/services/asterix
- **Specifications**: https://zoranbosnjak.github.io/asterix-specs/
- **Issue Tracker**: https://github.com/montge/asterix/issues
- **Discussions**: https://github.com/montge/asterix/discussions

---

## Support

- **Report Issues**: [GitHub Issues](https://github.com/montge/asterix/issues)
- **Discussions**: [GitHub Discussions](https://github.com/montge/asterix/discussions)
- **Crate**: [crates.io/crates/asterix-decoder](https://crates.io/crates/asterix-decoder)

---

## Credits

**Originally developed by**: [Croatia Control Ltd.](https://github.com/CroatiaControlLtd)
**Current maintainer**: [@montge](https://github.com/montge)

For questions about the ASTERIX protocol or this tool, feel free to contact the original author:
[Damir Salantic](https://hr.linkedin.com/in/damirsalantic)

---

**Star this repository if you find it useful!**
