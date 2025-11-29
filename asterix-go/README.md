# asterix-go

Go bindings for the ASTERIX ATM surveillance protocol decoder.

## Overview

This package provides Go bindings for parsing ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) binary data. ASTERIX is the standard format for Air Traffic Management surveillance data exchange in Europe and worldwide.

## Requirements

- Go 1.21 or later
- C++ compiler (GCC 9+, Clang 10+, or MSVC 2019+)
- CMake 3.20+
- libexpat development files

### Linux (Debian/Ubuntu)

```bash
sudo apt-get install build-essential cmake libexpat1-dev
```

### Linux (Fedora/RHEL)

```bash
sudo dnf install gcc-c++ cmake expat-devel
```

### macOS

```bash
brew install cmake expat
```

## Building

The Go bindings use cgo to wrap the C++ ASTERIX parser. Before using the package, you need to build the C++ library:

```bash
# From the repository root
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build
```

Then you can use the Go package:

```bash
cd asterix-go
go build
go test
```

## Usage

```go
package main

import (
    "fmt"
    "log"

    asterix "github.com/montge/asterix/asterix-go"
)

func main() {
    // Initialize with default configuration
    if err := asterix.Init(""); err != nil {
        log.Fatal(err)
    }

    // Parse ASTERIX data
    data := []byte{0x30, 0x00, 0x08, 0x80, 0x00, 0x01, 0x00, 0x02}
    records, err := asterix.Parse(data)
    if err != nil {
        log.Fatal(err)
    }

    for _, rec := range records {
        fmt.Printf("Category %d: %s\n", rec.Category, rec.JSON)
    }
}
```

## API Reference

### Initialization

```go
// Init initializes the parser with optional XML configuration
func Init(configPath string) error

// IsInitialized returns true if the parser has been initialized
func IsInitialized() bool

// Version returns the library version string
func Version() string
```

### Parsing

```go
// Parse parses ASTERIX data and returns records
func Parse(data []byte) ([]Record, error)

// ParseWithOptions parses with verbose flag control
func ParseWithOptions(data []byte, verbose bool) ([]Record, error)

// ParseWithOffset parses incrementally for large data streams
func ParseWithOffset(data []byte, offset, maxBlocks int) (*ParseResult, error)
```

### Description

```go
// Describe returns description for category/item/field
func Describe(category int, item, field, value string) (string, error)

// IsCategoryDefined returns true if category is defined
func IsCategoryDefined(category int) bool
```

### Types

```go
type Record struct {
    Category  uint8     // ASTERIX category (1-255)
    Length    uint16    // Record length in bytes
    Timestamp time.Time // Parse timestamp
    CRC       uint32    // Record checksum
    JSON      string    // Parsed data as JSON
}

// Items returns parsed data items as a map (cached)
func (r *Record) Items() (map[string]interface{}, error)

type ParseResult struct {
    Records         []Record
    BytesConsumed   int
    RemainingBlocks int
}
```

## Error Handling

The package defines the following errors:

- `ErrNotInitialized` - Parser not initialized, call Init() first
- `ErrInvalidData` - Invalid or empty input data
- `ErrParseFailed` - Parsing failed (check error message for details)
- `ErrMemory` - Memory allocation failed

## Thread Safety

The parser uses global state with mutex protection. Multiple goroutines can safely call Parse() concurrently after initialization. Init() should be called once at startup.

## Examples

See the `examples/` directory for complete examples:

- `parse_raw/` - Parse raw ASTERIX bytes
- `parse_file/` - Parse ASTERIX data from file

## Testing

```bash
go test -v
go test -bench=.
```

## License

GNU General Public License v3.0 - see the LICENSE file in the repository root.

## Related

- [Python bindings](../asterix/) - `pip install asterix-decoder`
- [Rust bindings](../asterix-rs/) - `cargo add asterix-decoder`
- [Node.js bindings](../asterix-node/) - N-API based bindings
