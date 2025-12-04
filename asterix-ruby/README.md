# ASTERIX Ruby Decoder

Ruby bindings for the ASTERIX (All Purpose STructured EUROCONTROL SuRveillance Information EXchange) decoder - a parser for EUROCONTROL ASTERIX protocol used in Air Traffic Management (ATM) and Air Traffic Control (ATC) systems.

[![Gem Version](https://badge.fury.io/rb/asterix_decoder.svg)](https://badge.fury.io/rb/asterix_decoder)
[![Documentation](https://img.shields.io/badge/docs-rubydoc-blue.svg)](https://rubydoc.info/gems/asterix_decoder)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

## Features

- **Native C extension** - High-performance parsing using the battle-tested ASTERIX C++ core
- **Full ASTERIX support** - Categories 001-252 with XML-based category definitions
- **Format support** - PCAP, HDLC, FINAL, GPS encapsulation formats
- **Incremental parsing** - Process large files without loading all data into memory
- **Thread-safe boundaries** - Comprehensive input validation at FFI layer (see [Thread Safety](#thread-safety))
- **Ruby 3.0+ compatible** - Supports Ruby 3.0, 3.1, 3.2, 3.3+
- **Idiomatic API** - Feels natural to Ruby developers

## Installation

### From RubyGems (recommended)

```bash
gem install asterix_decoder
```

Or add to your Gemfile:

```ruby
gem 'asterix_decoder'
```

### System Requirements

- **Ruby** 3.0 or later
- **libexpat** - XML parsing library
  - Debian/Ubuntu: `sudo apt-get install libexpat-dev`
  - RHEL/Fedora: `sudo dnf install expat-devel`
  - macOS: `brew install expat`
- **C++17+ compiler**
  - GCC 7+ or Clang 5+ (Linux/macOS)
  - MSVC 2019+ (Windows)

### From Source

```bash
git clone https://github.com/montge/asterix.git
cd asterix/asterix-ruby
bundle install
rake compile
```

## Quick Start

```ruby
require 'asterix'

# Read ASTERIX data from file
data = File.binread('sample.pcap')

# Parse the data
records = Asterix.parse(data)

# Process records
records.each do |record|
  puts "Category #{record[:category]}: #{record[:items].length} items"

  # Access data items
  record[:items].each do |item_name, item_data|
    puts "  #{item_name}: #{item_data}"
  end
end
```

## Usage Examples

### Basic Parsing

```ruby
require 'asterix'

# Parse with descriptions (default)
records = Asterix.parse(data)

# Parse without descriptions (faster, less memory)
records = Asterix.parse(data, verbose: false)

# Access record fields
record = records.first
puts "Category:  #{record[:category]}"
puts "Length:    #{record[:len]} bytes"
puts "Timestamp: #{record[:ts]} ms"
puts "CRC:       #{record[:crc]}"
puts "Hex data:  #{record[:hexdata]}"
```

### Incremental Parsing (Large Files)

For large ASTERIX data streams, use incremental parsing to avoid loading all data into memory:

```ruby
require 'asterix'

data = File.binread('large_file.asterix')
offset = 0
all_records = []

while offset < data.bytesize
  # Parse 100 blocks at a time
  records, offset = Asterix.parse_with_offset(
    data,
    offset: offset,
    blocks_count: 100,
    verbose: false
  )

  break if records.empty?
  all_records.concat(records)

  puts "Parsed #{records.length} records, offset: #{offset}"
end

puts "Total records: #{all_records.length}"
```

### Category and Field Descriptions

```ruby
require 'asterix'

# Get category description
desc = Asterix.describe(48)
# => "Monoradar Target Reports"

# Get item description
desc = Asterix.describe(48, '010')
# => "Data Source Identifier"

# Get field description
desc = Asterix.describe(48, '010', 'SAC')
# => "System Area Code"

# Get field value description
desc = Asterix.describe(48, '010', 'SAC', '7')
# => "System Area Code: 7"
```

### Custom Category Definitions

```ruby
require 'asterix'

# Load custom category definition
Asterix.init('/path/to/asterix_cat999_1_0.xml')

# Now you can parse CAT999 data
data = File.binread('cat999_data.asterix')
records = Asterix.parse(data)
```

### List Available Configurations

```ruby
require 'asterix'

# List all configuration files
Asterix.list_configuration_files.each do |file|
  puts File.basename(file)
end

# Get specific configuration file
config = Asterix.get_configuration_file('cat048')
puts config  # => "/path/to/asterix/config/asterix_cat048_1_30.xml"
```

## API Reference

### Module Methods

#### `Asterix.parse(data, verbose: true)`

Parse ASTERIX data from binary string.

- **Parameters:**
  - `data` (String) - Binary ASTERIX data (must be ASCII-8BIT encoding)
  - `verbose` (Boolean) - Include descriptions in output (default: true)
- **Returns:** Array of Hash records
- **Raises:**
  - `ArgumentError` - if data is empty or too large (>64KB)
  - `RuntimeError` - if parser not initialized

#### `Asterix.parse_with_offset(data, offset: 0, blocks_count: 1000, verbose: true)`

Parse ASTERIX data with incremental offset tracking.

- **Parameters:**
  - `data` (String) - Binary ASTERIX data
  - `offset` (Integer) - Byte offset to start parsing (default: 0)
  - `blocks_count` (Integer) - Maximum blocks to parse (default: 1000)
  - `verbose` (Boolean) - Include descriptions (default: true)
- **Returns:** Array `[records, new_offset]`
- **Raises:** `ArgumentError` if parameters are invalid

#### `Asterix.describe(category, item = nil, field = nil, value = nil)`

Get human-readable description for ASTERIX category/item/field/value.

- **Parameters:**
  - `category` (Integer) - ASTERIX category (1-255)
  - `item` (String, nil) - Item name (e.g., "010"), optional
  - `field` (String, nil) - Field name, optional
  - `value` (String, nil) - Field value, optional
- **Returns:** String description
- **Raises:** `ArgumentError` if category is invalid

#### `Asterix.init(filename)`

Initialize ASTERIX parser with XML category definition.

- **Parameters:**
  - `filename` (String) - Path to XML configuration file
- **Returns:** 0 if successful
- **Raises:**
  - `ArgumentError` - if filename is invalid
  - `IOError` - if file cannot be read
  - `SyntaxError` - if XML is malformed

#### `Asterix.list_configuration_files`

List all configuration files in the gem's config directory.

- **Returns:** Array of absolute paths to XML files

#### `Asterix.get_configuration_file(match)`

Get configuration file matching search string.

- **Parameters:**
  - `match` (String) - Search string
- **Returns:** String path or nil if not found

## Record Structure

Each parsed ASTERIX record is a Hash with the following keys:

```ruby
{
  category: 48,                    # ASTERIX category (Integer)
  len: 128,                        # Record length in bytes (Integer)
  ts: 1234567890123,               # Timestamp in milliseconds (Integer)
  crc: "0x1234",                   # CRC checksum (String)
  hexdata: "30003C...",            # Hexadecimal representation (String)
  items: {                         # Data items (Hash)
    'I010' => { ... },             # Data Source Identifier
    'I040' => { ... },             # Measured Position
    # ...
  }
}
```

## Thread Safety

**IMPORTANT:** This module is **NOT thread-safe**. The ASTERIX parser uses global state to manage category definitions (AsterixDefinition singleton in C++). Concurrent calls to `parse()`, `init()`, or `describe()` from multiple threads will cause race conditions and undefined behavior.

### Safe Approaches for Multi-Threaded Applications

**Option 1: Use a Mutex**

```ruby
require 'asterix'
require 'thread'

$asterix_mutex = Mutex.new

# In your threads:
$asterix_mutex.synchronize do
  records = Asterix.parse(data)
  # Process records...
end
```

**Option 2: Use Separate Processes**

```ruby
require 'asterix'

# Fork separate processes instead of threads
pid = fork do
  records = Asterix.parse(data)
  # Process records...
end

Process.wait(pid)
```

**Option 3: Single-Threaded Processing**

Ensure all ASTERIX operations happen in a single dedicated thread.

## Performance

The Ruby bindings provide near-native performance due to the C extension architecture:

- **Zero-copy input** - Data is passed directly to C++ without copying
- **Efficient memory management** - Ruby GC handles parsed data lifecycle
- **Buffer reuse** - Internal optimizations reduce allocations
- **Incremental parsing** - Process large streams with constant memory usage

### Benchmarks (Typical)

- **Small files (<1MB):** ~50-100 MB/s parsing throughput
- **Large files (>10MB):** ~80-150 MB/s with incremental parsing
- **Memory overhead:** ~2-3x the input data size (with verbose=true)

## Development

### Building from Source

```bash
git clone https://github.com/montge/asterix.git
cd asterix/asterix-ruby

# Install dependencies
bundle install

# Compile extension
rake compile

# Run tests
rake spec

# Check coverage
rake spec
# Open coverage/index.html
```

### Running Tests

```bash
# Run all tests
bundle exec rspec

# Run with coverage
bundle exec rspec --format documentation

# Run specific test file
bundle exec rspec spec/asterix_spec.rb
```

### Code Quality

```bash
# Lint with RuboCop
bundle exec rubocop

# Security audit
bundle exec bundler-audit

# Generate documentation
bundle exec yard doc
```

## Examples

See the `examples/` directory for complete working examples:

- **parse_basic.rb** - Simple parsing from file
- **parse_incremental.rb** - Incremental parsing for large files
- **describe_category.rb** - Get category and item descriptions
- **custom_category.rb** - Load custom category definitions

Run examples:

```bash
cd examples
ruby parse_basic.rb ../sample_data/cat048.pcap
ruby parse_incremental.rb ../sample_data/large_file.asterix 100
ruby describe_category.rb
```

## Supported ASTERIX Categories

The gem includes definitions for all standard EUROCONTROL ASTERIX categories:

- **CAT001** - Monoradar Track Messages
- **CAT002** - Monoradar Plot Messages
- **CAT019** - Multilateration System Status Messages
- **CAT020** - Multilateration Target Reports
- **CAT021** - ADS-B Target Reports
- **CAT023** - CNS/ATM Ground Station Reports
- **CAT034** - Monoradar Service Messages
- **CAT048** - Monoradar Target Reports
- **CAT062** - System Track Data
- **CAT063** - Sensor Status Reports
- **CAT065** - SDPS Service Status Reports
- **CAT252** - Reserved for Military Use
- ... and many more

Custom categories can be added by loading XML definition files.

## Troubleshooting

### Installation Errors

**Error: libexpat not found**

Install the expat development package:

```bash
# Debian/Ubuntu
sudo apt-get install libexpat-dev

# RHEL/Fedora
sudo dnf install expat-devel

# macOS
brew install expat
```

**Error: C++ compiler not found**

Install build tools:

```bash
# Debian/Ubuntu
sudo apt-get install build-essential

# RHEL/Fedora
sudo dnf groupinstall "Development Tools"

# macOS
xcode-select --install
```

### Runtime Errors

**ArgumentError: Empty input data**

Ensure your input data is not empty:

```ruby
data = File.binread('sample.pcap')
raise 'File is empty' if data.empty?
records = Asterix.parse(data)
```

**RuntimeError: ASTERIX parser not initialized**

This should not happen as categories are auto-loaded. If it does, manually initialize:

```ruby
Asterix.init('/path/to/asterix_cat048_1_30.xml')
```

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

### Reporting Issues

- Use GitHub Issues: https://github.com/montge/asterix/issues
- Include Ruby version, OS, and gem version
- Provide sample data if possible (anonymized)

### Development Workflow

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Make changes and add tests
4. Ensure tests pass (`rake spec`)
5. Check code quality (`rubocop`)
6. Commit with descriptive messages
7. Push and create a Pull Request

## License

This project is licensed under the GNU General Public License v3.0 or later - see the [LICENSE](../COPYING) file for details.

## Related Projects

- **Python bindings:** `pip install asterix_decoder`
- **Rust bindings:** `cargo add asterix-decoder`
- **C++ CLI:** Build from source in parent directory

## References

- **ASTERIX Protocol:** https://www.eurocontrol.int/services/asterix
- **Project Homepage:** https://github.com/montge/asterix
- **Documentation:** https://rubydoc.info/gems/asterix_decoder
- **Issue Tracker:** https://github.com/montge/asterix/issues

## Credits

- **Original ASTERIX decoder:** Croatia Control Ltd. (www.crocontrol.hr)
- **Ruby bindings:** ASTERIX Contributors
- **Maintainer:** See AUTHORS file

## Support

For questions and support:

- GitHub Discussions: https://github.com/montge/asterix/discussions
- Issue Tracker: https://github.com/montge/asterix/issues
- Email: asterix@crocontrol.hr

---

**Made with ❤️ for the Aviation Community**
