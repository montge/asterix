/**
 * FFI wrapper implementation for ASTERIX C++ core
 *
 * This file implements the C-compatible interface defined in ffi_wrapper.h
 */

#include "ffi_wrapper.h"
#include "AsterixDefinition.h"
#include "DataBlock.h"
#include "XMLParser.h"
#include "DataRecord.h"
#include "Tracer.h"
// AsterixData.h and InputParser.h already included via ffi_wrapper.h
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>

// Global ASTERIX definition (singleton pattern from original code)
static AsterixDefinition* g_asterix_definition = nullptr;

// Automatic cleanup on program exit to prevent memory leaks and use-after-free
struct AsterixGlobalCleanup {
    ~AsterixGlobalCleanup() {
        if (g_asterix_definition) {
            delete g_asterix_definition;
            g_asterix_definition = nullptr;
        }
    }
};
static AsterixGlobalCleanup g_cleanup;

namespace asterix {

// Initialization functions
extern "C" {

bool asterix_init(rust::Str config_dir) {
    try {
        std::string config_path = std::string(config_dir);

        if (config_path.empty()) {
            std::cerr << "Error: config_dir is empty" << std::endl;
            return false;
        }

        // Create new definition if not exists
        if (!g_asterix_definition) {
            g_asterix_definition = new AsterixDefinition();
        }

        // Load configuration files from directory
        // This is a simplified version - real implementation should scan directory
        // For now, we expect the config directory to contain asterix.ini

        // The original code uses XMLParser to load definitions
        // We'll return true for now - actual loading happens via asterix_load_category
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Exception in asterix_init: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception in asterix_init" << std::endl;
        return false;
    }
}

bool asterix_load_category(rust::Str xml_path) {
    try {
        std::string path = std::string(xml_path);

        if (path.empty()) {
            return false;
        }

        if (!g_asterix_definition) {
            g_asterix_definition = new AsterixDefinition();
        }

        // Use XMLParser to load category definition
        FILE* file = fopen(path.c_str(), "r");
        if (!file) {
            std::cerr << "Failed to open XML file: " << path << std::endl;
            return false;
        }

        XMLParser parser;
        bool result = parser.Parse(file, g_asterix_definition, path.c_str());
        fclose(file);

        return result;

    } catch (const std::exception& e) {
        std::cerr << "Exception in asterix_load_category: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception in asterix_load_category" << std::endl;
        return false;
    }
}

bool asterix_category_defined(uint8_t category) {
    if (!g_asterix_definition) {
        return false;
    }
    return g_asterix_definition->CategoryDefined(category);
}

void asterix_set_log_level(int level) {
    Tracer::SetLogLevel(level);
}

int asterix_get_log_level() {
    return Tracer::GetLogLevel();
}

} // extern "C"

// Parsing functions
extern "C" {

AsterixDataWrapper* asterix_parse(
    const uint8_t* data,
    size_t len,
    bool verbose
) {
    try {
        if (!data || len == 0) {
            return nullptr;
        }

        if (!g_asterix_definition) {
            std::cerr << "Error: ASTERIX not initialized. Call asterix_init first." << std::endl;
            return nullptr;
        }

        // Create parser with definition
        InputParser* parser = new InputParser(g_asterix_definition);

        // Parse packet
        AsterixData* asterix_data = parser->parsePacket(data, len, 0.0);

        if (!asterix_data) {
            delete parser;
            return nullptr;
        }

        // Wrap in our wrapper struct
        return new AsterixDataWrapper(asterix_data, parser);

    } catch (const std::exception& e) {
        std::cerr << "Exception in asterix_parse: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cerr << "Unknown exception in asterix_parse" << std::endl;
        return nullptr;
    }
}

AsterixDataWrapper* asterix_parse_offset(
    const uint8_t* data,
    size_t len,
    uint32_t offset,
    uint32_t blocks_count,
    bool verbose
) {
    try {
        if (!data || len == 0 || offset >= len) {
            return nullptr;
        }

        if (!g_asterix_definition) {
            std::cerr << "Error: ASTERIX not initialized" << std::endl;
            return nullptr;
        }

        // For now, just parse from offset to end
        // TODO: Implement proper incremental parsing with block count
        const uint8_t* data_ptr = data + offset;
        size_t remaining_len = len - offset;

        InputParser* parser = new InputParser(g_asterix_definition);
        AsterixData* asterix_data = parser->parsePacket(data_ptr, remaining_len, 0.0);

        if (!asterix_data) {
            delete parser;
            return nullptr;
        }

        return new AsterixDataWrapper(asterix_data, parser);

    } catch (const std::exception& e) {
        std::cerr << "Exception in asterix_parse_offset: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cerr << "Unknown exception in asterix_parse_offset" << std::endl;
        return nullptr;
    }
}

void asterix_free_data(AsterixDataWrapper* ptr) {
    if (ptr) {
        delete ptr;
    }
}

} // extern "C"

// Data access functions
extern "C" {

uint32_t asterix_data_block_count(const AsterixDataWrapper* data) {
    if (!data || !data->data) {
        return 0;
    }

    // AsterixData contains a list of DataBlocks
    // We need to count them
    std::list<DataBlock*> blocks = data->data->m_lDataBlocks;
    return static_cast<uint32_t>(blocks.size());
}

const DataBlockWrapper* asterix_get_data_block(
    const AsterixDataWrapper* data,
    uint32_t index
) {
    if (!data || !data->data) {
        return nullptr;
    }

    std::list<DataBlock*>& blocks = data->data->m_lDataBlocks;

    if (index >= blocks.size()) {
        return nullptr;
    }

    // Ensure we have cached all wrappers (done once per AsterixDataWrapper)
    if (data->block_wrappers.empty()) {
        data->block_wrappers.reserve(blocks.size());
        for (auto* block : blocks) {
            data->block_wrappers.emplace_back(block);
        }
    }

    // Return stable pointer to cached wrapper (lifetime tied to AsterixDataWrapper)
    return &data->block_wrappers[index];
}

uint8_t asterix_block_category(const DataBlockWrapper* block) {
    if (!block || !block->block || !block->block->m_pCategory) {
        return 0;
    }
    return static_cast<uint8_t>(block->block->m_pCategory->m_id);
}

uint32_t asterix_block_length(const DataBlockWrapper* block) {
    if (!block || !block->block) {
        return 0;
    }
    return static_cast<uint32_t>(block->block->m_nLength);
}

uint64_t asterix_block_timestamp_ms(const DataBlockWrapper* block) {
    if (!block || !block->block) {
        return 0;
    }
    // Convert timestamp from seconds to milliseconds
    return static_cast<uint64_t>(block->block->m_nTimestamp * 1000.0);
}

uint32_t asterix_block_crc(const DataBlockWrapper* block) {
    if (!block || !block->block) {
        return 0;
    }
    // CRC is stored on DataRecord, not DataBlock
    // Return CRC from first data record if available
    if (!block->block->m_lDataRecords.empty()) {
        DataRecord* firstRecord = block->block->m_lDataRecords.front();
        if (firstRecord) {
            return static_cast<uint32_t>(firstRecord->m_nCrc);
        }
    }
    return 0;
}

const uint8_t* asterix_block_hex_data(const DataBlockWrapper* block) {
    if (!block || !block->block) {
        return nullptr;
    }

    // Build hex string from data records (cached in wrapper for stable lifetime)
    block->hex_data_cache.clear();

    for (const auto& record : block->block->m_lDataRecords) {
        if (record && record->m_pHexData) {
            block->hex_data_cache += record->m_pHexData;
        }
    }

    return reinterpret_cast<const uint8_t*>(block->hex_data_cache.c_str());
}

uint8_t* asterix_block_to_json(const DataBlockWrapper* block) {
    if (!block || !block->block) {
        return nullptr;
    }

    try {
        // Generate JSON representation using getText with EJSON format
        std::string json;
        // EJSON = 6 (compact JSON format)
        block->block->getText(json, 6);

        // Allocate and copy string
        size_t len = json.length() + 1;
        uint8_t* result = new uint8_t[len];
        std::memcpy(result, json.c_str(), len);

        return result;

    } catch (const std::exception& e) {
        std::cerr << "Exception in asterix_block_to_json: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

uint8_t* asterix_block_to_text(const DataBlockWrapper* block) {
    if (!block || !block->block) {
        return nullptr;
    }

    try {
        // Generate text representation using getText with ETxt format
        std::string text;
        // ETxt = 2 (human readable text format)
        block->block->getText(text, 2);

        // Allocate and copy
        size_t len = text.length() + 1;
        uint8_t* result = new uint8_t[len];
        std::memcpy(result, text.c_str(), len);

        return result;

    } catch (const std::exception& e) {
        std::cerr << "Exception in asterix_block_to_text: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

void asterix_free_string(uint8_t* ptr) {
    if (ptr) {
        delete[] ptr;
    }
}

} // extern "C"

// Metadata functions
extern "C" {

uint8_t* asterix_describe(
    uint8_t category,
    const uint8_t* item,
    size_t item_len,
    const uint8_t* field,
    size_t field_len,
    const uint8_t* value,
    size_t value_len
) {
    try {
        if (!g_asterix_definition) {
            return nullptr;
        }

        // Convert byte arrays to C strings
        const char* item_str = (item && item_len > 0) ?
            reinterpret_cast<const char*>(item) : nullptr;
        const char* field_str = (field && field_len > 0) ?
            reinterpret_cast<const char*>(field) : nullptr;
        const char* value_str = (value && value_len > 0) ?
            reinterpret_cast<const char*>(value) : nullptr;

        // Call original getDescription
        const char* desc = g_asterix_definition->getDescription(
            category,
            item_str,
            field_str,
            value_str
        );

        if (!desc) {
            return nullptr;
        }

        // Allocate and copy result
        size_t len = std::strlen(desc) + 1;
        uint8_t* result = new uint8_t[len];
        std::memcpy(result, desc, len);

        return result;

    } catch (const std::exception& e) {
        std::cerr << "Exception in asterix_describe: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

} // extern "C"

} // namespace asterix
