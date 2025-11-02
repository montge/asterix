/**
 * FFI wrapper for ASTERIX C++ core
 *
 * This header provides a C-compatible interface to the C++ ASTERIX library
 * for safe interop with Rust via the cxx crate.
 */

#ifndef ASTERIX_FFI_WRAPPER_H
#define ASTERIX_FFI_WRAPPER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Include complete type definitions needed for std::unique_ptr
#include "../src/asterix/AsterixData.h"
#include "../src/asterix/InputParser.h"

// Forward declarations of other C++ ASTERIX types
class DataBlock;
class AsterixDefinition;

namespace asterix {

/**
 * Wrapper for AsterixData to provide stable ABI
 */
struct AsterixDataWrapper {
    std::unique_ptr<AsterixData> data;
    std::unique_ptr<InputParser> parser;

    AsterixDataWrapper(AsterixData* d, InputParser* p)
        : data(d), parser(p) {}
};

/**
 * Wrapper for DataBlock (doesn't own the data)
 */
struct DataBlockWrapper {
    DataBlock* block;  // Non-owning pointer

    explicit DataBlockWrapper(DataBlock* b) : block(b) {}
};

// Initialization functions
extern "C" {

/**
 * Initialize ASTERIX parser with config directory
 * Returns true on success, false on failure
 */
bool asterix_init(const char* config_dir);

/**
 * Load a specific category definition file
 * Returns true on success, false on failure
 */
bool asterix_load_category(const char* xml_path);

/**
 * Check if a category is defined
 */
bool asterix_category_defined(uint8_t category);

} // extern "C"

// Parsing functions
extern "C" {

/**
 * Parse raw ASTERIX data
 *
 * @param data Pointer to raw data
 * @param len Length of data in bytes
 * @param verbose Enable verbose output
 * @return Pointer to AsterixDataWrapper (must be freed with asterix_free_data)
 */
AsterixDataWrapper* asterix_parse(
    const uint8_t* data,
    size_t len,
    bool verbose
);

/**
 * Parse with offset for incremental parsing
 *
 * @param data Pointer to complete data buffer
 * @param len Total length of buffer
 * @param offset Byte offset to start parsing
 * @param blocks_count Max number of blocks to parse (0 = all)
 * @param verbose Enable verbose output
 * @return Pointer to AsterixDataWrapper (must be freed)
 */
AsterixDataWrapper* asterix_parse_offset(
    const uint8_t* data,
    size_t len,
    uint32_t offset,
    uint32_t blocks_count,
    bool verbose
);

/**
 * Free AsterixDataWrapper allocated by parse functions
 */
void asterix_free_data(AsterixDataWrapper* ptr);

} // extern "C"

// Data access functions
extern "C" {

/**
 * Get number of data blocks in parsed result
 */
uint32_t asterix_data_block_count(const AsterixDataWrapper* data);

/**
 * Get a specific data block by index
 * Returns NULL if index is out of bounds
 */
const DataBlockWrapper* asterix_get_data_block(
    const AsterixDataWrapper* data,
    uint32_t index
);

/**
 * Get category number from data block
 */
uint8_t asterix_block_category(const DataBlockWrapper* block);

/**
 * Get length in bytes from data block
 */
uint32_t asterix_block_length(const DataBlockWrapper* block);

/**
 * Get timestamp in milliseconds
 */
uint64_t asterix_block_timestamp_ms(const DataBlockWrapper* block);

/**
 * Get CRC checksum
 */
uint32_t asterix_block_crc(const DataBlockWrapper* block);

/**
 * Get hexadecimal representation of data block
 * Returns pointer to null-terminated string (lifetime tied to block)
 */
const uint8_t* asterix_block_hex_data(const DataBlockWrapper* block);

/**
 * Get JSON representation of data block
 * Returns allocated string (must be freed with asterix_free_string)
 */
uint8_t* asterix_block_to_json(const DataBlockWrapper* block);

/**
 * Get text representation of data block
 * Returns allocated string (must be freed with asterix_free_string)
 */
uint8_t* asterix_block_to_text(const DataBlockWrapper* block);

/**
 * Free string allocated by C++ side
 */
void asterix_free_string(uint8_t* ptr);

} // extern "C"

// Metadata functions
extern "C" {

/**
 * Get description of category/item/field/value
 *
 * @param category Category number
 * @param item Item name (NULL or empty for category description)
 * @param item_len Length of item string
 * @param field Field name (NULL or empty if not needed)
 * @param field_len Length of field string
 * @param value Value (NULL or empty if not needed)
 * @param value_len Length of value string
 * @return Allocated string (must be freed with asterix_free_string)
 */
uint8_t* asterix_describe(
    uint8_t category,
    const uint8_t* item,
    size_t item_len,
    const uint8_t* field,
    size_t field_len,
    const uint8_t* value,
    size_t value_len
);

} // extern "C"

} // namespace asterix

#endif // ASTERIX_FFI_WRAPPER_H
