/**
 * FFI wrapper implementation for ASTERIX C++ core
 *
 * This file implements the C-compatible interface defined in ffi_wrapper.h
 */

#include "ffi_wrapper.h"
#include "AsterixDefinition.h"
#include "AsterixData.h"
#include "DataBlock.h"
#include "InputParser.h"
#include "XMLParser.h"
#include <cstring>
#include <sstream>
#include <iostream>

// Global ASTERIX definition (singleton pattern from original code)
static AsterixDefinition* g_asterix_definition = nullptr;

namespace asterix {

// Initialization functions
extern "C" {

bool asterix_init(const char* config_dir) {
    try {
        if (!config_dir || strlen(config_dir) == 0) {
            std::cerr << "Error: config_dir is null or empty" << std::endl;
            return false;
        }

        // Create new definition if not exists
        if (!g_asterix_definition) {
            g_asterix_definition = new AsterixDefinition();
        }

        // Load configuration files from directory
        // This is a simplified version - real implementation should scan directory
        // For now, we expect the config directory to contain asterix.ini
        std::string config_path = std::string(config_dir);

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

bool asterix_load_category(const char* xml_path) {
    try {
        if (!xml_path || strlen(xml_path) == 0) {
            return false;
        }

        if (!g_asterix_definition) {
            g_asterix_definition = new AsterixDefinition();
        }

        // Use XMLParser to load category definition
        XMLParser parser;
        Category* cat = parser.parseCategory(xml_path);

        if (cat) {
            g_asterix_definition->setCategory(cat);
            return true;
        }

        return false;

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

    std::list<DataBlock*> blocks = data->data->m_lDataBlocks;

    if (index >= blocks.size()) {
        return nullptr;
    }

    // Navigate to the indexed element
    auto it = blocks.begin();
    std::advance(it, index);

    // Return wrapped block (non-owning)
    static DataBlockWrapper wrapper(nullptr);
    wrapper.block = *it;
    return &wrapper;
}

uint8_t asterix_block_category(const DataBlockWrapper* block) {
    if (!block || !block->block) {
        return 0;
    }
    return static_cast<uint8_t>(block->block->m_nCategory);
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
    // Convert timestamp to milliseconds
    return static_cast<uint64_t>(block->block->m_dTimestamp * 1000.0);
}

uint32_t asterix_block_crc(const DataBlockWrapper* block) {
    if (!block || !block->block) {
        return 0;
    }
    return static_cast<uint32_t>(block->block->m_nCRC);
}

const uint8_t* asterix_block_hex_data(const DataBlockWrapper* block) {
    if (!block || !block->block) {
        return nullptr;
    }

    // Get hex representation
    std::string hex = block->block->getHexString();

    // Return pointer to internal string
    // WARNING: This has lifetime issues - the string will be destroyed
    // when the function returns. In production, we should cache this.
    static std::string cached_hex;
    cached_hex = hex;
    return reinterpret_cast<const uint8_t*>(cached_hex.c_str());
}

uint8_t* asterix_block_to_json(const DataBlockWrapper* block) {
    if (!block || !block->block) {
        return nullptr;
    }

    try {
        // Generate JSON representation
        std::string json = block->block->toJSON();

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
        // Generate text representation
        std::string text = block->block->toString();

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
