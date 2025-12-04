# Implementation Notes - Ruby Bindings

## Critical Changes Required in C++ Core

To complete the Ruby bindings implementation, the following changes must be made to the C++ core:

### 1. Add getRubyData Method to AsterixData Class

**File:** `src/asterix/AsterixData.h`

Add this method declaration:

```cpp
#ifdef __cplusplus
extern "C" {
#endif

VALUE getRubyData(int verbose);  // Ruby-specific data conversion

#ifdef __cplusplus
}
#endif
```

**File:** `src/asterix/AsterixData.cpp`

Implement the method (similar to existing `getData` for Python):

```cpp
#include <ruby.h>

VALUE AsterixData::getRubyData(int verbose) {
    VALUE rb_records = rb_ary_new();

    std::list<DataBlock*>::iterator it;
    for (it = m_lDataBlocks.begin(); it != m_lDataBlocks.end(); it++) {
        DataBlock* block = (DataBlock*)(*it);

        std::list<DataRecord*>::iterator it2;
        for (it2 = block->m_lDataRecords.begin(); it2 != block->m_lDataRecords.end(); it2++) {
            DataRecord* record = (DataRecord*)(*it2);
            VALUE rb_record = record->getRubyRecord(verbose);
            rb_ary_push(rb_records, rb_record);
        }
    }

    return rb_records;
}
```

### 2. Add getRubyRecord Method to DataRecord Class

**File:** `src/asterix/DataRecord.h`

```cpp
VALUE getRubyRecord(int verbose);
```

**File:** `src/asterix/DataRecord.cpp`

```cpp
VALUE DataRecord::getRubyRecord(int verbose) {
    VALUE rb_record = rb_hash_new();

    // Add basic fields
    rb_hash_aset(rb_record, ID2SYM(rb_intern("category")), INT2NUM(m_nCategory));
    rb_hash_aset(rb_record, ID2SYM(rb_intern("len")), LONG2NUM(m_nLength));
    rb_hash_aset(rb_record, ID2SYM(rb_intern("ts")), ULONG2NUM(m_nTimestamp));
    rb_hash_aset(rb_record, ID2SYM(rb_intern("crc")), rb_str_new_cstr(m_strCrc.c_str()));
    rb_hash_aset(rb_record, ID2SYM(rb_intern("hexdata")), rb_str_new_cstr(m_strHexData.c_str()));

    // Add data items
    VALUE rb_items = rb_hash_new();
    std::list<DataItem*>::iterator it;
    for (it = m_lDataItems.begin(); it != m_lDataItems.end(); it++) {
        DataItem* item = (DataItem*)(*it);
        VALUE rb_item = item->getRubyItem(verbose);
        VALUE item_name = rb_str_new_cstr(item->m_pDescription->m_strID.c_str());
        rb_hash_aset(rb_items, item_name, rb_item);
    }
    rb_hash_aset(rb_record, ID2SYM(rb_intern("items")), rb_items);

    return rb_record;
}
```

### 3. Add getRubyItem Method to DataItem Class

**File:** `src/asterix/DataItem.h`

```cpp
VALUE getRubyItem(int verbose);
```

**File:** `src/asterix/DataItem.cpp`

```cpp
VALUE DataItem::getRubyItem(int verbose) {
    // Convert DataItem to Ruby Hash
    // This follows the same pattern as getPyObject() but returns VALUE
    // Implementation depends on item format type

    if (verbose) {
        // Include descriptions, min/max, units
        VALUE rb_item = rb_hash_new();
        // ... populate with detailed info
        return rb_item;
    } else {
        // Return only values
        // ... return compact representation
    }
}
```

## Alternative Implementation (Simpler)

If modifying the C++ core is not desired initially, the Ruby bindings can use the existing Python-style conversion by adding a thin adapter layer:

**File:** `ext/asterix/ruby_adapter.cpp`

```cpp
// Convert Python PyObject to Ruby VALUE
VALUE python_to_ruby(PyObject* py_obj) {
    if (PyDict_Check(py_obj)) {
        VALUE rb_hash = rb_hash_new();
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(py_obj, &pos, &key, &value)) {
            VALUE rb_key = python_to_ruby(key);
            VALUE rb_value = python_to_ruby(value);
            rb_hash_aset(rb_hash, rb_key, rb_value);
        }
        return rb_hash;
    }
    // ... handle other Python types
}
```

However, this approach requires Python development headers and adds complexity. The recommended approach is to implement native Ruby conversion methods.

## Build System Integration

### Option 1: Standalone Build (Current)

The Ruby bindings compile all ASTERIX C++ source files directly into the extension. This is the approach used in `ext/asterix/extconf.rb`.

**Pros:**
- Self-contained, no external dependencies
- Easy to distribute as a gem

**Cons:**
- Longer build times
- Duplicate compilation if multiple bindings installed

### Option 2: Shared Library Build (Future)

Link against a pre-built libasterix.so shared library:

```ruby
# extconf.rb
have_library('asterix') or abort "libasterix not found"
create_makefile('asterix_ext')
```

**Pros:**
- Faster builds
- Smaller gem size
- Shared code between bindings

**Cons:**
- Requires libasterix installed system-wide
- More complex distribution

## Testing the Implementation

### Unit Tests

Run the test suite:

```bash
bundle exec rspec
```

Expected coverage: >80%

### Integration Tests

Test with real ASTERIX data:

```bash
# Basic parsing
ruby examples/parse_basic.rb ../sample_data/cat048.pcap

# Incremental parsing
ruby examples/parse_incremental.rb ../sample_data/large_file.asterix 100

# Category descriptions
ruby examples/describe_category.rb

# Custom categories
ruby examples/custom_category.rb /path/to/cat999.xml data.asterix
```

### Memory Leak Tests

Run Valgrind to check for memory leaks:

```bash
valgrind --leak-check=full ruby examples/parse_basic.rb ../sample_data/cat048.pcap
```

Expected result: 0 leaks

## Performance Benchmarks

Compare Ruby bindings performance to Python:

```ruby
# benchmark.rb
require 'benchmark'
require 'asterix'

data = File.binread('large_file.asterix')

Benchmark.bm(20) do |x|
  x.report("parse (verbose):")   { Asterix.parse(data, verbose: true) }
  x.report("parse (compact):")   { Asterix.parse(data, verbose: false) }
  x.report("incremental:")       {
    offset = 0
    while offset < data.bytesize
      records, offset = Asterix.parse_with_offset(data, offset: offset)
      break if records.empty?
    end
  }
end
```

Expected throughput: 50-150 MB/s (comparable to Python bindings)

## Known Limitations

1. **Thread Safety:** Not thread-safe due to global state in C++ core (AsterixDefinition singleton)
2. **Encoding:** Input data must be binary (ASCII-8BIT encoding)
3. **Platform Support:** Currently tested on Linux and macOS; Windows support requires MSVC 2019+
4. **Ruby Versions:** Requires Ruby 3.0+; older versions not supported

## Future Enhancements

1. **Streaming API:** Support for parsing from IO streams (files, sockets)
2. **JSON Export:** Direct JSON serialization without intermediate Ruby objects
3. **Custom Callbacks:** User-defined callbacks for record processing
4. **Multi-threading:** Thread-safe parser instances (requires C++ core refactoring)
5. **JRuby Support:** Native Java binding alternative to C extension

## References

- **Issue #30:** https://github.com/montge/asterix/issues/30
- **BINDING_GUIDELINES.md:** Comprehensive binding requirements
- **Python bindings:** Reference implementation (src/python/)
- **Rust bindings:** Alternative architecture (asterix-rs/)
