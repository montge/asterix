# Development Guide - ASTERIX Ruby Bindings

This document provides guidance for developing and extending the ASTERIX Ruby bindings.

## Architecture Overview

The Ruby bindings follow a three-layer architecture:

```
┌─────────────────────────────────────┐
│   Ruby API (lib/asterix.rb)         │  ← Idiomatic Ruby interface
│   - Type checking                   │
│   - Encoding conversion             │
│   - Auto-initialization             │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│   C Extension (ext/asterix/)        │  ← FFI Boundary (CRITICAL)
│   - Input validation                │
│   - Buffer bounds checking          │
│   - Error mapping (C++ → Ruby)      │
│   - Memory management               │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│   C++ Core Parser                   │  ← Shared ASTERIX engine
│   (../../src/asterix/)              │
│   - ASTERIX protocol parsing        │
│   - XML category definitions        │
└─────────────────────────────────────┘
```

## File Structure

```
asterix-ruby/
├── ext/asterix/
│   ├── asterix_ext.c          # Ruby C API wrapper (FFI boundary)
│   ├── ruby_parser.h          # C++ bridge header
│   ├── ruby_parser.cpp        # C++ bridge implementation
│   └── extconf.rb             # Build configuration
├── lib/
│   └── asterix.rb             # Ruby wrapper module
├── spec/
│   ├── spec_helper.rb         # RSpec configuration
│   └── asterix_spec.rb        # Test suite
├── examples/
│   ├── parse_basic.rb         # Basic parsing example
│   ├── parse_incremental.rb   # Incremental parsing example
│   ├── describe_category.rb   # Category descriptions example
│   └── custom_category.rb     # Custom category loading example
├── asterix.gemspec            # Gem specification
├── Rakefile                   # Build tasks
├── Gemfile                    # Development dependencies
├── README.md                  # User documentation
├── CHANGELOG.md               # Version history
└── DEVELOPMENT.md             # This file
```

## Building from Source

### Prerequisites

- Ruby 3.0+ with development headers
- C++17+ compiler (GCC 7+, Clang 5+, MSVC 2019+)
- libexpat development package
- CMake 3.20+ (optional, for full C++ core rebuild)

### Build Steps

```bash
# Install dependencies
bundle install

# Compile the C extension
rake compile

# Run tests
rake spec

# Run linter
rake rubocop

# Run security audit
rake audit

# Run all checks
rake check
```

### Interactive Development

```bash
# Start IRB console with asterix loaded
rake console

# In the console:
> data = File.binread('/path/to/sample.pcap')
> records = Asterix.parse(data)
> puts records.first[:category]
```

## Adding New Features

### Adding a New Ruby Method

1. **Add to C extension** (`ext/asterix/asterix_ext.c`):

```c
static VALUE asterix_new_method(VALUE self, VALUE arg) {
    // 1. Validate inputs
    Check_Type(arg, T_STRING);

    // 2. Call C++ core
    VALUE result = ruby_new_method(...);

    // 3. Return result
    return result;
}

void Init_asterix_ext(void) {
    VALUE mAsterix = rb_define_module("AsterixNative");
    rb_define_module_function(mAsterix, "new_method", asterix_new_method, 1);
}
```

2. **Add wrapper** (`lib/asterix.rb`):

```ruby
module Asterix
  def self.new_method(arg)
    raise ArgumentError, 'Arg must be a String' unless arg.is_a?(String)
    AsterixNative.new_method(arg)
  end
end
```

3. **Add tests** (`spec/asterix_spec.rb`):

```ruby
describe '.new_method' do
  it 'does something' do
    result = Asterix.new_method('test')
    expect(result).to be_a(String)
  end
end
```

### FFI Boundary Validation Checklist

All C extension functions MUST validate inputs:

- [ ] NULL pointer checks
- [ ] Type checks (Check_Type)
- [ ] Buffer length validation
- [ ] Offset bounds checking
- [ ] Integer overflow prevention
- [ ] Error code mapping (C++ exceptions → Ruby exceptions)

Example:

```c
static VALUE asterix_parse(int argc, VALUE *argv, VALUE self) {
    VALUE data, verbose_val;
    rb_scan_args(argc, argv, "11", &data, &verbose_val);

    // ✅ Type check
    Check_Type(data, T_STRING);

    // ✅ Length validation
    size_t len = RSTRING_LEN(data);
    if (len == 0) {
        rb_raise(rb_eArgError, "Empty input data");
    }
    if (len > MAX_ASTERIX_MESSAGE_SIZE) {
        rb_raise(rb_eArgError, "Input data too large: %zu bytes", len);
    }

    // ✅ Call C++ core
    VALUE result = ruby_parse(RSTRING_PTR(data), len, verbose);
    return result;
}
```

## Testing

### Running Tests

```bash
# Run all tests
bundle exec rspec

# Run with coverage
bundle exec rspec
# View coverage/index.html

# Run specific test
bundle exec rspec spec/asterix_spec.rb:42

# Run with documentation format
bundle exec rspec --format documentation
```

### Writing Tests

Follow the pattern in `spec/asterix_spec.rb`:

```ruby
describe '.method_name' do
  context 'with valid input' do
    it 'returns expected result' do
      result = Asterix.method_name('valid_input')
      expect(result).to eq('expected_output')
    end
  end

  context 'with invalid input' do
    it 'raises ArgumentError' do
      expect { Asterix.method_name(nil) }.to raise_error(ArgumentError)
    end
  end
end
```

### Coverage Requirements

- Overall coverage: >80%
- Per-file coverage: >70%
- Critical paths (FFI validation): 100%

## Memory Management

### Ownership Rules

1. **Input buffers:**
   - Ruby owns the input data (passed via RSTRING_PTR)
   - C extension borrows pointer (zero-copy)
   - Do NOT free input buffers

2. **Output data:**
   - C++ allocates AsterixData
   - C++ converts to Ruby objects (Hash, Array)
   - Ruby GC manages lifecycle
   - C++ deletes temporary objects

3. **Error strings:**
   - Use Ruby's exception mechanism
   - rb_raise automatically manages memory

### Example: Memory-Safe Parsing

```c
VALUE result = ruby_parse(buffer, len, verbose);
if (NIL_P(result)) {
    // ruby_parse already raised exception
    return Qnil;
}
// Ruby now owns result, GC will clean up
return result;
```

## Error Handling

### C++ Exceptions → Ruby Exceptions

Map C++ error codes to Ruby exception classes:

```cpp
// In ruby_parser.cpp
try {
    // C++ code
} catch (const std::bad_alloc& e) {
    rb_raise(rb_eNoMemError, "Out of memory: %s", e.what());
} catch (const std::exception& e) {
    rb_raise(rb_eRuntimeError, "C++ exception: %s", e.what());
}
```

### Ruby Exception Classes

- `ArgumentError` - Invalid input parameters
- `IOError` - File I/O errors
- `SyntaxError` - XML parsing errors
- `NoMemError` - Out of memory
- `RuntimeError` - General C++ exceptions

## Performance Optimization

### Best Practices

1. **Avoid unnecessary copies:**
   ```ruby
   # ✅ Good: Zero-copy input
   data = File.binread('file.pcap')
   records = Asterix.parse(data)

   # ❌ Bad: Extra copy
   records = Asterix.parse(data.dup)
   ```

2. **Use verbose: false for large datasets:**
   ```ruby
   # Faster, less memory
   records = Asterix.parse(data, verbose: false)
   ```

3. **Incremental parsing for large files:**
   ```ruby
   offset = 0
   while offset < data.bytesize
     records, offset = Asterix.parse_with_offset(data, offset: offset)
     process(records)
   end
   ```

## Debugging

### Enable Debug Logging

```bash
# Compile with debug symbols
DEBUG=1 rake compile

# Run tests with debug output
DEBUG=1 bundle exec rspec
```

### GDB Debugging

```bash
# Attach GDB to Ruby process
gdb --args ruby test_script.rb

# Set breakpoints
(gdb) break ruby_parse
(gdb) run
```

### Memory Leak Detection

```bash
# Run with Valgrind (Linux)
valgrind --leak-check=full ruby test_script.rb

# Check for leaks in extension
cd ../../install/test
./valgrind_test.sh
```

## Contributing

### Pre-Commit Checklist

- [ ] All tests pass (`rake spec`)
- [ ] Code coverage >80% (`rake spec` then check coverage/)
- [ ] RuboCop passes (`rake rubocop`)
- [ ] Security audit passes (`rake audit`)
- [ ] Documentation updated (YARD comments)
- [ ] CHANGELOG.md updated
- [ ] Examples updated if API changed

### Code Style

Follow Ruby style guide:
- Use `rubocop` for linting
- Maximum line length: 120 characters
- Use descriptive variable names
- Add YARD comments to all public methods

### Commit Messages

```
Brief description (50 chars or less)

Relates to #<issue-number>

- Detailed point 1
- Detailed point 2
- Testing performed
```

## Release Process

1. Update version in `lib/asterix.rb`
2. Update CHANGELOG.md
3. Commit changes
4. Create git tag: `git tag -a v2.8.10 -m "Release v2.8.10"`
5. Build gem: `rake build`
6. Test gem: `gem install pkg/asterix_decoder-2.8.10.gem`
7. Publish: `gem push pkg/asterix_decoder-2.8.10.gem`

## CI/CD Integration

See `.github/workflows/ruby.yml` (to be created) for CI/CD pipeline:

- Build on: Ubuntu, macOS, Windows
- Ruby versions: 3.0, 3.1, 3.2, 3.3
- Run: compile, spec, rubocop, audit
- Upload coverage to Codecov

## References

- **BINDING_GUIDELINES.md** - Language binding requirements
- **ARCHITECTURE.md** - Overall system architecture
- **Ruby C API** - https://docs.ruby-lang.org/en/master/extension_rdoc.html
- **YARD Documentation** - https://yardoc.org/

## Getting Help

- GitHub Issues: https://github.com/montge/asterix/issues
- GitHub Discussions: https://github.com/montge/asterix/discussions
- Email: asterix@crocontrol.hr
