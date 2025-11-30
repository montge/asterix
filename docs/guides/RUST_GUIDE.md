# Rust User Guide

Complete guide to using the ASTERIX decoder Rust crate for type-safe, memory-safe surveillance data parsing.

## Overview

The `asterix-decoder` Rust crate provides safe, idiomatic Rust bindings to the high-performance C++ ASTERIX parser. It uses the CXX crate for zero-cost FFI with full type safety.

**Minimum Rust version:** 1.70+ (2021 edition)

## Installation

### From crates.io

Add to your `Cargo.toml`:

```toml
[dependencies]
asterix-decoder = "2.8"
```

Or use cargo:

```bash
cargo add asterix-decoder
```

### From Source

```bash
git clone https://github.com/montge/asterix.git
cd asterix/asterix-rs
cargo build --release
```

## Quick Start

```rust
use asterix::{init_default, parse, ParseOptions};

fn main() -> Result<(), asterix::AsterixError> {
    // Initialize with default category definitions
    init_default()?;

    // Parse ASTERIX data
    let data = std::fs::read("sample.asterix")?;
    let records = parse(&data, ParseOptions::default())?;

    for record in records {
        println!("Category {}: {} bytes", record.category, record.length);
    }

    Ok(())
}
```

## API Reference

### Initialization

#### `init_default() -> Result<(), AsterixError>`

Initialize the parser with default category definitions.

```rust
use asterix::init_default;

fn main() -> Result<(), asterix::AsterixError> {
    init_default()?;
    // Parser is now ready
    Ok(())
}
```

#### `Parser::new().add_category(path).build()`

Create a parser with custom category definitions.

```rust
use asterix::Parser;

let parser = Parser::new()
    .add_category("/path/to/asterix_cat048_1_30.xml")?
    .add_category("/path/to/asterix_cat062_1_18.xml")?
    .build()?;
```

### Parsing

#### `parse(data: &[u8], options: ParseOptions) -> Result<Vec<AsterixRecord>, AsterixError>`

Parse ASTERIX data from a byte slice.

```rust
use asterix::{parse, ParseOptions};

let data: &[u8] = &[0x30, 0x00, 0x2A, ...];
let records = parse(data, ParseOptions::default())?;
```

#### `parse_with_offset(data: &[u8], offset: usize, count: usize, options: ParseOptions) -> Result<(Vec<AsterixRecord>, usize), AsterixError>`

Parse with offset for incremental/streaming parsing.

```rust
use asterix::{parse_with_offset, ParseOptions};

let data = std::fs::read("large_file.asterix")?;
let mut offset = 0;

while offset < data.len() {
    let (records, next_offset) = parse_with_offset(&data, offset, 100, ParseOptions::default())?;
    offset = next_offset;

    for record in records {
        // Process record
    }
}
```

### Data Types

#### `AsterixRecord`

Represents a parsed ASTERIX record.

```rust
pub struct AsterixRecord {
    pub id: u32,           // Sequence number
    pub category: u8,      // ASTERIX category (1-255)
    pub length: u16,       // Record length in bytes
    pub timestamp: f64,    // Unix timestamp
    pub crc: String,       // CRC checksum
    pub hex_data: String,  // Raw data as hex string
    pub items: Vec<DataItem>,  // Parsed data items
}
```

#### `DataItem`

Represents a data item within a record.

```rust
pub struct DataItem {
    pub id: String,        // Item ID (e.g., "I010", "I140")
    pub name: String,      // Human-readable name
    pub fields: Vec<Field>,// Fields within the item
}
```

#### `Field`

Represents a field within a data item.

```rust
pub struct Field {
    pub name: String,      // Field name
    pub value: FieldValue, // Typed value
    pub description: Option<String>,
}

pub enum FieldValue {
    Integer(i64),
    Float(f64),
    String(String),
    Bytes(Vec<u8>),
}
```

#### `ParseOptions`

Configuration for parsing.

```rust
pub struct ParseOptions {
    pub verbose: bool,     // Include descriptions
    pub strict: bool,      // Fail on unknown items
}

impl Default for ParseOptions {
    fn default() -> Self {
        ParseOptions {
            verbose: false,
            strict: false,
        }
    }
}
```

### Description

#### `describe(category: u8, item: Option<&str>, field: Option<&str>, value: Option<i64>) -> Result<String, AsterixError>`

Get human-readable descriptions.

```rust
use asterix::describe;

// Category description
let desc = describe(48, None, None, None)?;
println!("{}", desc);  // "Monoradar Target Reports"

// Item description
let desc = describe(48, Some("I010"), None, None)?;
println!("{}", desc);  // "Data Source Identifier"

// Value meaning
let desc = describe(48, Some("I020"), Some("TYP"), Some(5))?;
println!("{}", desc);  // "Single ModeS Roll-Call"
```

## Common Use Cases

### Parse File

```rust
use asterix::{init_default, parse, ParseOptions};
use std::fs;

fn parse_file(path: &str) -> Result<(), asterix::AsterixError> {
    init_default()?;

    let data = fs::read(path)?;
    let records = parse(&data, ParseOptions::default())?;

    println!("Parsed {} records", records.len());

    for record in records {
        println!(
            "Cat {}: {} items, {} bytes",
            record.category,
            record.items.len(),
            record.length
        );
    }

    Ok(())
}
```

### Extract Specific Fields

```rust
use asterix::{init_default, parse, ParseOptions, AsterixRecord};

fn extract_mode3a(record: &AsterixRecord) -> Option<String> {
    for item in &record.items {
        if item.id == "I070" {
            for field in &item.fields {
                if field.name == "MODE3A" {
                    if let asterix::FieldValue::String(s) = &field.value {
                        return Some(s.clone());
                    }
                }
            }
        }
    }
    None
}

fn main() -> Result<(), asterix::AsterixError> {
    init_default()?;

    let data = std::fs::read("radar_data.asterix")?;
    let records = parse(&data, ParseOptions::default())?;

    for record in records.iter().filter(|r| r.category == 48) {
        if let Some(mode3a) = extract_mode3a(record) {
            println!("Squawk: {}", mode3a);
        }
    }

    Ok(())
}
```

### Streaming Processing

```rust
use asterix::{init_default, parse_with_offset, ParseOptions};
use std::io::{BufReader, Read};
use std::fs::File;

fn process_stream(path: &str) -> Result<(), asterix::AsterixError> {
    init_default()?;

    // Read file in chunks
    let file = File::open(path)?;
    let mut reader = BufReader::new(file);
    let mut buffer = Vec::new();
    reader.read_to_end(&mut buffer)?;

    let mut offset = 0;
    let mut total = 0;

    while offset < buffer.len() {
        let (records, next_offset) = parse_with_offset(
            &buffer,
            offset,
            1000,
            ParseOptions::default()
        )?;

        total += records.len();
        offset = next_offset;

        // Process batch
        for record in records {
            // ...
        }
    }

    println!("Processed {} records", total);
    Ok(())
}
```

### Filter by Category

```rust
use asterix::{init_default, parse, ParseOptions, AsterixRecord};

fn filter_category(data: &[u8], cat: u8) -> Result<Vec<AsterixRecord>, asterix::AsterixError> {
    let records = parse(data, ParseOptions::default())?;
    Ok(records.into_iter().filter(|r| r.category == cat).collect())
}

fn main() -> Result<(), asterix::AsterixError> {
    init_default()?;

    let data = std::fs::read("mixed_data.asterix")?;

    // Get only CAT 062 (SDPS tracks)
    let tracks = filter_category(&data, 62)?;
    println!("Found {} SDPS tracks", tracks.len());

    Ok(())
}
```

### Convert to JSON

```rust
use asterix::{init_default, parse, ParseOptions};
use serde_json;

fn to_json(data: &[u8]) -> Result<String, Box<dyn std::error::Error>> {
    init_default()?;

    let records = parse(data, ParseOptions { verbose: true, strict: false })?;

    // AsterixRecord derives Serialize
    let json = serde_json::to_string_pretty(&records)?;
    Ok(json)
}
```

### Async Processing (tokio)

```rust
use asterix::{init_default, parse, ParseOptions};
use tokio::fs;
use tokio::net::UdpSocket;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    init_default()?;

    // Async file reading
    let data = fs::read("sample.asterix").await?;
    let records = parse(&data, ParseOptions::default())?;

    println!("Parsed {} records", records.len());
    Ok(())
}

async fn receive_udp() -> Result<(), Box<dyn std::error::Error>> {
    init_default()?;

    let socket = UdpSocket::bind("0.0.0.0:8600").await?;
    let mut buf = vec![0u8; 65535];

    loop {
        let (len, addr) = socket.recv_from(&mut buf).await?;
        let records = parse(&buf[..len], ParseOptions::default())?;

        for record in records {
            println!("[{}] Cat {}", addr, record.category);
        }
    }
}
```

## Error Handling

The crate uses a custom error type:

```rust
use asterix::AsterixError;

fn handle_errors() {
    match asterix::init_default() {
        Ok(_) => println!("Initialized"),
        Err(AsterixError::ConfigNotFound(path)) => {
            eprintln!("Config not found: {}", path);
        }
        Err(AsterixError::ParseError(msg)) => {
            eprintln!("Parse error: {}", msg);
        }
        Err(e) => {
            eprintln!("Other error: {}", e);
        }
    }
}
```

### Error Types

```rust
pub enum AsterixError {
    ConfigNotFound(String),
    ParseError(String),
    InvalidCategory(u8),
    InvalidData(String),
    IoError(std::io::Error),
}
```

## Performance

### Benchmarks

Run benchmarks:

```bash
cd asterix-rs
cargo bench
```

Typical results (M1 Mac):
- Small file (1KB): ~50μs
- Medium file (100KB): ~2ms
- Large file (10MB): ~150ms

### Optimization Tips

1. **Reuse parsed data** - parsing is the expensive operation
2. **Use `parse_with_offset` for large files** - better memory efficiency
3. **Batch processing** - parse in chunks rather than one record at a time
4. **Release mode** - always use `--release` for production

```bash
cargo build --release
```

## Testing

Run the test suite:

```bash
cd asterix-rs
cargo test --all-features
```

Example test:

```rust
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_cat048() {
        init_default().unwrap();

        let data = include_bytes!("../test_data/cat048.asterix");
        let records = parse(data, ParseOptions::default()).unwrap();

        assert!(!records.is_empty());
        assert_eq!(records[0].category, 48);
    }

    #[test]
    fn test_describe_category() {
        init_default().unwrap();

        let desc = describe(48, None, None, None).unwrap();
        assert!(desc.contains("Monoradar"));
    }
}
```

## Feature Flags

```toml
[dependencies]
asterix-decoder = { version = "2.8", features = ["serde"] }
```

| Feature | Description |
|---------|-------------|
| `serde` | Enable Serialize/Deserialize for data types |
| `async` | Async parsing support (requires tokio) |

## Supported Categories

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

## Related Documentation

- [Building from Source](../BUILD.md#rust-crate)
- [API Architecture](../ARCHITECTURE.md)
- [FFI Design](../BINDING_GUIDELINES.md)
- [docs.rs Documentation](https://docs.rs/asterix-decoder)

## Support

- **crates.io**: https://crates.io/crates/asterix-decoder
- **docs.rs**: https://docs.rs/asterix-decoder
- **Issues**: https://github.com/montge/asterix/issues
- **Source**: `asterix-rs/` directory in repository
