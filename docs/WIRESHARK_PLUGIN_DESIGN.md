# Wireshark 4.x ASTERIX Dissector - Design Document

**Version:** 1.0
**Date:** 2025-11-16
**Status:** Design Phase
**Target:** Wireshark 4.0.0+
**Issue:** [#22](https://github.com/montge/asterix/issues/22) - Modernize Wireshark plugin

---

## Executive Summary

This document outlines the design for a modern Wireshark 4.x dissector plugin for the ASTERIX protocol. This plugin replaces the legacy Wireshark 1.8.4/1.10.6 and Ethereal 0.99.0 plugins that were removed in Issue #22 Phase 1.

**Key Differences from Legacy Plugins:**
- Modern Wireshark 4.x API (not 1.x/2.x)
- CMake build system (not autotools/nmake)
- Cross-platform support (Linux, Windows MSVC 2019+, macOS)
- Lua scripting support
- Integration with ASTERIX C++ core (shared code)

---

## Background

### Legacy Plugins (Removed)

Three legacy plugins were removed in November 2025:

1. **Ethereal 0.99.0 plugin** (circa 2006)
   - RPM-based build
   - Ancient Linux only
   - 15+ years out of date

2. **Wireshark 1.8.4 plugin** (circa 2012)
   - MSVC 2008 Express
   - nmake build system
   - EOL 2015

3. **Wireshark 1.10.6 plugin** (circa 2013)
   - Similar to 1.8.4
   - EOL 2016

**Why they were removed:**
- Do not compile with Wireshark 4.x API
- Security vulnerabilities in old Wireshark versions
- Unmaintainable (~1MB of dead code)
- Misleading users about plugin availability

**See:** [BREAKING_CHANGES.md](../BREAKING_CHANGES.md) for migration path

---

## Goals

### Primary Goals

1. **Wireshark 4.x Compatibility** - Support Wireshark 4.0.0 through latest 4.x
2. **Cross-Platform** - Linux, Windows (MSVC 2019+), macOS (AppleClang 15+)
3. **CMake Integration** - Modern build system, no autotools/nmake
4. **Code Reuse** - Share ASTERIX parsing code with C++ executable
5. **Comprehensive Category Support** - All 24 supported ASTERIX categories
6. **Production Quality** - Memory safe, crash-free, high performance

### Secondary Goals

1. **Lua Scripting** - Enable custom ASTERIX filters/post-processors
2. **Color Coding** - Visual differentiation by category/severity
3. **Statistics** - Protocol hierarchy statistics, IO graphs
4. **Export** - PDML, JSON, CSV export of ASTERIX data
5. **Preferences** - User-configurable display options

### Non-Goals

1. **Backward Compatibility** - No support for Wireshark < 4.0
2. **Custom UI** - Use standard Wireshark packet details pane
3. **Real-time Capture** - Wireshark already handles this

---

## Wireshark 4.x API Overview

### Key API Changes from 1.x/2.x

Wireshark 4.x introduced major API changes:

| Feature | Wireshark 1.x/2.x | Wireshark 4.x |
|---------|-------------------|---------------|
| **Registration** | `proto_register_protocol()` | Same, but different headers |
| **Dissection** | `dissect_*()` callbacks | Same API, different tvbuff functions |
| **Tree Building** | `proto_tree_add_*()` | Expanded with `proto_tree_add_item_ret_*()` |
| **Preferences** | `prefs_register_*()` | Same, but more types |
| **Expert Info** | `expert_add_info()` | Enhanced with more severity levels |
| **Build System** | autotools, nmake | **CMake only** |
| **C Standard** | C99 | **C11/C17** |
| **GLib Version** | GLib 2.32+ | **GLib 2.50+** |

**Critical Changes:**
- `tvb_get_*()` functions renamed/reorganized
- Header file reorganization (`epan/proto.h` → `epan/packet.h`)
- Plugin directory structure changed
- CMake is the ONLY supported build system

**Resources:**
- Wireshark Developer's Guide: https://www.wireshark.org/docs/wsdg_html_chunked/
- Example dissectors: `plugins/epan/` in Wireshark source tree
- API reference: `epan/proto.h`, `epan/packet.h`

---

## Architecture

### Component Overview

```
+-------------------+
|  Wireshark GUI    |
|  (Qt 5.15/6.x)    |
+-------------------+
         |
         v
+-------------------+
| ASTERIX Dissector |  <-- This plugin
| (plugin DLL/SO)   |
+-------------------+
         |
    +----+----+
    |         |
    v         v
+-------+  +---------------+
| Epan  |  | ASTERIX Core  |
| API   |  | (shared .so)  |
+-------+  +---------------+
```

**Components:**

1. **ASTERIX Dissector Plugin** (`epan/asterix/packet-asterix.c`)
   - Wireshark protocol dissector
   - Registers with Wireshark's dissector table
   - Calls ASTERIX core for parsing
   - Builds protocol tree for display

2. **ASTERIX Core Library** (`libasterix.so`)
   - Shared library from C++ executable build
   - Provides `parse()`, `init()`, `describe()` functions
   - XML-based category definitions
   - Platform-independent parsing logic

3. **CMake Build Integration**
   - Plugin CMakeLists.txt
   - Links against Wireshark libraries
   - Links against ASTERIX core library
   - Cross-platform compiler flags

---

### Plugin Directory Structure

```
plugins/epan/asterix/          # Wireshark plugin directory
├── CMakeLists.txt              # CMake build configuration
├── packet-asterix.c            # Main dissector implementation
├── packet-asterix.h            # Dissector headers
├── asterix-categories.c        # Category definitions (auto-generated)
├── asterix-categories.h        # Category headers
├── moduleinfo.h                # Plugin metadata
└── README.md                   # Plugin documentation
```

**Installation Paths:**
- Linux: `~/.local/lib/wireshark/plugins/4.0/epan/asterix.so`
- Windows: `%APPDATA%\Wireshark\plugins\4.0\epan\asterix.dll`
- macOS: `~/Library/Application Support/Wireshark/plugins/4.0/epan/asterix.so`

---

## Dissector Design

### Packet Dissection Flow

```
1. Wireshark captures UDP packet (e.g., port 8600)
2. UDP dissector calls ASTERIX dissector (heuristic match)
3. ASTERIX dissector:
   a. Extract ASTERIX data from UDP payload
   b. Parse ASTERIX data block header (CAT, LEN)
   c. Parse data records (FSPEC, data items)
   d. Build protocol tree for Wireshark display
   e. Add expert info for errors/warnings
```

### Heuristic Dissection

**Problem:** ASTERIX has no magic number or fixed port

**Solution:** Heuristic dissection based on data block structure

```c
static gboolean
dissect_asterix_heuristic(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data)
{
    // Minimum ASTERIX data block: 3 bytes (CAT + LEN)
    if (tvb_captured_length(tvb) < 3) {
        return FALSE;
    }

    // Check if first byte is valid ASTERIX category (1-255, not 0)
    guint8 category = tvb_get_guint8(tvb, 0);
    if (category == 0) {
        return FALSE;
    }

    // Check if length field is reasonable
    guint16 length = tvb_get_ntohs(tvb, 1);
    if (length < 3 || length > tvb_captured_length(tvb)) {
        return FALSE;
    }

    // High confidence - call full dissector
    dissect_asterix(tvb, pinfo, tree, data);
    return TRUE;
}
```

**Heuristic Registration:**
```c
void
proto_reg_handoff_asterix(void)
{
    heur_dissector_add("udp", dissect_asterix_heuristic, "ASTERIX over UDP", "asterix_udp", proto_asterix, HEURISTIC_ENABLE);
}
```

---

### Protocol Tree Construction

**Goal:** Display ASTERIX data in Wireshark's packet details pane

**Example Output:**
```
ASTERIX CAT 048 - Monoradar Target Reports
    Data Block: 128 bytes
    Category: 48 (0x30)
    Length: 128 bytes
    Data Records: 3
    Data Record 1
        FSPEC: 0xf0 (I010, I020, I040, I070)
        I048/010 - Data Source Identifier
            SAC: 7 (System Area Code)
            SIC: 15 (System Identification Code)
        I048/020 - Target Report Descriptor
            TYP: 0 (Single PSR target)
            SIM: 0 (Actual target report)
            RDP: 0 (Report from RDP Chain 1)
            SPI: 0 (Default)
            RAB: 0 (Report from aircraft transponder)
        I048/040 - Measured Position in Polar Coordinates
            RHO: 12.5 NM
            THETA: 45.3 degrees
        I048/070 - Mode-3/A Code in Octal Representation
            Code: 1234 (octal)
```

**Implementation:**
```c
static int
dissect_asterix(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data _U_)
{
    proto_item *ti;
    proto_tree *asterix_tree;
    proto_tree *record_tree;
    guint offset = 0;

    // Set protocol column
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "ASTERIX");

    // Create protocol tree
    ti = proto_tree_add_item(tree, proto_asterix, tvb, 0, -1, ENC_NA);
    asterix_tree = proto_item_add_subtree(ti, ett_asterix);

    // Parse data block header
    guint8 category = tvb_get_guint8(tvb, offset);
    proto_tree_add_item(asterix_tree, hf_asterix_category, tvb, offset, 1, ENC_BIG_ENDIAN);
    offset += 1;

    guint16 length = tvb_get_ntohs(tvb, offset);
    proto_tree_add_item(asterix_tree, hf_asterix_length, tvb, offset, 2, ENC_BIG_ENDIAN);
    offset += 2;

    // Update info column
    col_add_fstr(pinfo->cinfo, COL_INFO, "CAT%03d, %d bytes", category, length);

    // Parse data records
    while (offset < length) {
        // Create record subtree
        ti = proto_tree_add_item(asterix_tree, hf_asterix_record, tvb, offset, -1, ENC_NA);
        record_tree = proto_item_add_subtree(ti, ett_asterix_record);

        // Parse FSPEC
        guint fspec_len = parse_fspec(tvb, offset, record_tree);
        offset += fspec_len;

        // Parse data items based on FSPEC
        offset = parse_data_items(tvb, offset, category, record_tree);
    }

    return tvb_captured_length(tvb);
}
```

---

### Header Field Registration

**Goal:** Define all displayable fields for filtering and display

**Example:**
```c
static hf_register_info hf[] = {
    { &hf_asterix_category,
      { "Category", "asterix.category",
        FT_UINT8, BASE_DEC, VALS(asterix_category_vals), 0x0,
        "ASTERIX Category", HFILL }
    },
    { &hf_asterix_length,
      { "Length", "asterix.length",
        FT_UINT16, BASE_DEC, NULL, 0x0,
        "Data Block Length", HFILL }
    },
    { &hf_asterix_010_sac,
      { "SAC", "asterix.010.sac",
        FT_UINT8, BASE_DEC, NULL, 0x0,
        "System Area Code", HFILL }
    },
    { &hf_asterix_010_sic,
      { "SIC", "asterix.010.sic",
        FT_UINT8, BASE_DEC, NULL, 0x0,
        "System Identification Code", HFILL }
    },
    // ... hundreds more fields ...
};
```

**Value Strings (for enum display):**
```c
static const value_string asterix_category_vals[] = {
    { 1, "Monoradar Target Reports" },
    { 2, "Monoradar Target Messages" },
    { 48, "Monoradar Target Reports" },
    { 62, "System Track Data" },
    { 65, "SDPS Service Status Messages" },
    { 0, NULL }
};
```

**Filter Examples:**
- `asterix` - Show all ASTERIX packets
- `asterix.category == 48` - Show only CAT048
- `asterix.010.sac == 7` - Show records from SAC=7
- `asterix.040.rho > 50` - Show targets beyond 50 NM

---

## Integration with ASTERIX Core

### FFI Boundary

**Challenge:** Wireshark dissector (C) must call ASTERIX core (C++)

**Solution:** C-compatible wrapper functions

**Wrapper Interface (asterix_wrapper.h):**
```c
#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle to ASTERIX parser
typedef struct asterix_parser_t asterix_parser_t;

// Initialize ASTERIX parser
asterix_parser_t* asterix_init(const char* config_dir);

// Parse ASTERIX data block
typedef struct {
    uint8_t category;
    uint16_t length;
    const uint8_t* data;
    size_t data_len;
} asterix_data_block_t;

typedef struct {
    uint8_t item_id;       // e.g., 010 for I048/010
    const char* name;      // e.g., "Data Source Identifier"
    const uint8_t* value;  // Raw data
    size_t value_len;
    const char* description; // Human-readable
} asterix_data_item_t;

typedef struct {
    asterix_data_item_t* items;
    size_t item_count;
} asterix_record_t;

// Parse single data block
int asterix_parse_block(asterix_parser_t* parser,
                        const uint8_t* data,
                        size_t data_len,
                        asterix_record_t** records,
                        size_t* record_count);

// Free parsed records
void asterix_free_records(asterix_record_t* records, size_t record_count);

// Get category description
const char* asterix_describe_category(asterix_parser_t* parser, uint8_t category);

// Cleanup
void asterix_destroy(asterix_parser_t* parser);

#ifdef __cplusplus
}
#endif
```

**Wrapper Implementation (asterix_wrapper.cpp):**
```cpp
extern "C" {

asterix_parser_t* asterix_init(const char* config_dir) {
    try {
        AsterixDefinition::init(config_dir);
        return reinterpret_cast<asterix_parser_t*>(1); // Singleton
    } catch (...) {
        return nullptr;
    }
}

int asterix_parse_block(asterix_parser_t* parser,
                        const uint8_t* data,
                        size_t data_len,
                        asterix_record_t** records,
                        size_t* record_count) {
    // Validation
    if (!parser || !data || data_len < 3 || !records || !record_count) {
        return -1;
    }

    try {
        // Use existing ASTERIX parsing code
        DataBlock block;
        if (!block.parse(data, data_len)) {
            return -1;
        }

        // Convert to C-compatible structure
        *record_count = block.get_record_count();
        *records = (asterix_record_t*)calloc(*record_count, sizeof(asterix_record_t));

        for (size_t i = 0; i < *record_count; i++) {
            const DataRecord* rec = block.get_record(i);
            asterix_record_t* out_rec = &(*records)[i];

            out_rec->item_count = rec->get_item_count();
            out_rec->items = (asterix_data_item_t*)calloc(out_rec->item_count, sizeof(asterix_data_item_t));

            for (size_t j = 0; j < out_rec->item_count; j++) {
                const DataItem* item = rec->get_item(j);
                asterix_data_item_t* out_item = &out_rec->items[j];

                out_item->item_id = item->get_id();
                out_item->name = strdup(item->get_name().c_str());
                out_item->value = item->get_raw_data();
                out_item->value_len = item->get_raw_length();
                out_item->description = strdup(item->get_description().c_str());
            }
        }

        return 0;
    } catch (...) {
        return -1;
    }
}

void asterix_free_records(asterix_record_t* records, size_t record_count) {
    if (!records) return;

    for (size_t i = 0; i < record_count; i++) {
        asterix_record_t* rec = &records[i];
        for (size_t j = 0; j < rec->item_count; j++) {
            free((void*)rec->items[j].name);
            free((void*)rec->items[j].description);
        }
        free(rec->items);
    }
    free(records);
}

} // extern "C"
```

---

## CMake Build System

### Plugin CMakeLists.txt

```cmake
# plugins/epan/asterix/CMakeLists.txt

cmake_minimum_required(VERSION 3.12)

project(wireshark-asterix-plugin)

# Find Wireshark package
find_package(Wireshark REQUIRED)

if(NOT Wireshark_FOUND)
    message(FATAL_ERROR "Wireshark not found. Install Wireshark development packages.")
endif()

# Find ASTERIX library
find_library(ASTERIX_LIBRARY
    NAMES asterix libasterix
    PATHS ${CMAKE_SOURCE_DIR}/../../install/lib
    REQUIRED
)

# Find ASTERIX headers
find_path(ASTERIX_INCLUDE_DIR
    NAMES AsterixDefinition.h
    PATHS ${CMAKE_SOURCE_DIR}/../../src/asterix
    REQUIRED
)

# Plugin source files
set(PLUGIN_FILES
    packet-asterix.c
    asterix-categories.c
    asterix_wrapper.cpp
)

# Create plugin library
add_library(asterix MODULE ${PLUGIN_FILES})

# Include directories
target_include_directories(asterix PRIVATE
    ${Wireshark_INCLUDE_DIRS}
    ${ASTERIX_INCLUDE_DIR}
    ${GLIB2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(asterix
    ${Wireshark_LIBRARIES}
    ${ASTERIX_LIBRARY}
    ${GLIB2_LIBRARIES}
)

# Compiler flags
target_compile_options(asterix PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-parameter>
)

# C++ standard for wrapper
set_source_files_properties(asterix_wrapper.cpp PROPERTIES
    COMPILE_FLAGS "-std=c++17"
)

# Installation
install(TARGETS asterix
    LIBRARY DESTINATION ${Wireshark_PLUGIN_INSTALL_DIR}/epan
)

# Generate moduleinfo.h
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/moduleinfo.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/moduleinfo.h
    @ONLY
)
```

### Top-Level CMake Integration

Add to main `CMakeLists.txt`:

```cmake
option(BUILD_WIRESHARK_PLUGIN "Build Wireshark 4.x plugin" OFF)

if(BUILD_WIRESHARK_PLUGIN)
    find_package(Wireshark 4.0 REQUIRED)
    add_subdirectory(plugins/epan/asterix)
endif()
```

**Build Instructions:**
```bash
# Install Wireshark development packages
sudo apt-get install wireshark-dev  # Ubuntu/Debian
brew install wireshark               # macOS
# Windows: Build Wireshark from source or use SDK

# Build ASTERIX with Wireshark plugin
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_WIRESHARK_PLUGIN=ON
cmake --build build
sudo cmake --install build

# Install plugin
mkdir -p ~/.local/lib/wireshark/plugins/4.0/epan
cp build/plugins/epan/asterix/asterix.so ~/.local/lib/wireshark/plugins/4.0/epan/
```

---

## Testing Strategy

### Unit Tests

1. **Heuristic Dissection** - Test false positives/negatives
2. **Data Block Parsing** - Test all 24 categories
3. **FSPEC Parsing** - Test single/multi-octet FSPEC
4. **Error Handling** - Truncated data, invalid categories
5. **Memory Leaks** - Valgrind testing

### Integration Tests

1. **Real PCAP Files** - Test with actual network captures
2. **Wireshark CLI** - `tshark -r capture.pcap -V`
3. **Display Filters** - Test all filter expressions
4. **Export Formats** - PDML, JSON, CSV export
5. **Multi-Platform** - Linux, Windows, macOS

### Performance Tests

1. **Large PCAP Files** - 1GB+ captures
2. **High Packet Rate** - 10,000+ packets/sec
3. **Memory Usage** - Should not leak or grow unbounded
4. **UI Responsiveness** - Should not freeze Wireshark

---

## Implementation Phases

### Phase 1: Minimal Viable Plugin (MVP)
**Goal:** Basic dissection of CAT048

- [x] Research Wireshark 4.x API
- [ ] Create plugin directory structure
- [ ] Implement heuristic dissector
- [ ] Parse CAT048 data block header
- [ ] Display category and length
- [ ] CMake build integration
- [ ] Test with sample PCAP

**Deliverable:** Plugin that displays "ASTERIX CAT048" in packet list

---

### Phase 2: FSPEC and Data Items
**Goal:** Parse and display all CAT048 data items

- [ ] Implement FSPEC parser
- [ ] Implement data item parsers (I048/010, I048/020, etc.)
- [ ] Build protocol tree
- [ ] Add header fields for filtering
- [ ] Test with real CAT048 data

**Deliverable:** Full CAT048 dissection with all data items

---

### Phase 3: Multi-Category Support
**Goal:** Support all 24 ASTERIX categories

- [ ] Integrate with ASTERIX core library
- [ ] Auto-generate category definitions
- [ ] Test with CAT001, CAT002, CAT019, CAT020, CAT021, CAT062, CAT065
- [ ] Add category-specific preferences
- [ ] Expert info for errors/warnings

**Deliverable:** Plugin supporting all 24 categories

---

### Phase 4: Polish and Distribution
**Goal:** Production-ready plugin

- [ ] Color coding by category
- [ ] Statistics integration
- [ ] Comprehensive documentation
- [ ] Windows/macOS builds
- [ ] Publish to Wireshark plugin repository
- [ ] User guide and examples

**Deliverable:** Production-quality Wireshark plugin

---

## Timeline

**Estimated Effort:** 6-8 weeks (1 developer)

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| Phase 1: MVP | 1 week | Wireshark 4.x development environment |
| Phase 2: Data Items | 2 weeks | Phase 1 complete |
| Phase 3: Multi-Category | 2 weeks | ASTERIX core FFI wrapper |
| Phase 4: Polish | 1-2 weeks | Phase 3 complete |
| Testing & Documentation | 1 week | All phases complete |

**Target Release:** Q1 2026

---

## Resources

### Documentation
- Wireshark Developer's Guide: https://www.wireshark.org/docs/wsdg_html_chunked/
- Wireshark Wiki: https://gitlab.com/wireshark/wireshark/-/wikis/home
- Example Dissectors: `plugins/epan/` in Wireshark source
- ASTERIX Protocol Spec: http://www.eurocontrol.int/services/asterix

### Tools
- Wireshark 4.x source code: https://gitlab.com/wireshark/wireshark
- CMake 3.12+
- GCC 7+, Clang 5+, MSVC 2019+
- Valgrind (memory leak testing)

### Community
- Wireshark-dev mailing list: https://www.wireshark.org/lists/
- ASTERIX GitHub Issues: https://github.com/montge/asterix/issues

---

## Risk Assessment

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| Wireshark API changes in 4.x | High | Medium | Test with multiple Wireshark versions |
| ASTERIX core FFI complexity | Medium | Low | Start with simple wrapper, expand gradually |
| Cross-platform build issues | Medium | Medium | CI/CD testing on all platforms |
| Performance degradation | Medium | Low | Profile early, optimize hot paths |
| Plugin approval delay | Low | Medium | Follow Wireshark guidelines strictly |

---

## Open Questions

1. **Lua Scripting:** Should we support Lua post-dissectors?
   - **Answer:** Phase 4 feature, not MVP

2. **UDP Port Registration:** Should we register on default port 8600?
   - **Answer:** Yes, with heuristic as fallback

3. **Category Auto-Detection:** Should we support mixed-category UDP streams?
   - **Answer:** Yes, parse each data block independently

4. **Performance:** Can we achieve real-time dissection at 10 Gbps?
   - **Answer:** Profile and optimize in Phase 4

---

## Conclusion

This design provides a roadmap for building a modern Wireshark 4.x ASTERIX dissector plugin that:
- Replaces the removed legacy plugins
- Uses modern Wireshark 4.x API
- Integrates with existing ASTERIX C++ core
- Supports cross-platform builds via CMake
- Provides comprehensive ASTERIX protocol dissection

**Next Steps:**
1. Review and approve this design
2. Set up Wireshark 4.x development environment
3. Begin Phase 1: MVP implementation
4. Create tracking issue for implementation progress

---

**Document Version History:**
- v1.0 (2025-11-16): Initial design document
