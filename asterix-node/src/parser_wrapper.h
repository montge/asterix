/*
 *  Copyright (c) 2025 ASTERIX Contributors
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#ifndef ASTERIX_NODE_PARSER_WRAPPER_H
#define ASTERIX_NODE_PARSER_WRAPPER_H

#include <cstdint>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct AsterixRecord
 * @brief Represents a single parsed ASTERIX record
 */
struct AsterixRecord {
    uint8_t category;           ///< ASTERIX category (1-255)
    uint32_t length;            ///< Data block length in bytes
    uint64_t timestamp_ms;      ///< Timestamp in milliseconds since epoch
    uint32_t crc;               ///< CRC checksum
    char* hex_data;             ///< Hex representation of data (null-terminated)
    char* json_data;            ///< JSON representation of items (null-terminated)
};

/**
 * @struct AsterixRecords
 * @brief Container for multiple ASTERIX records
 */
struct AsterixRecords {
    AsterixRecord* records;     ///< Array of records
    size_t count;               ///< Number of records
};

/**
 * Initialize ASTERIX parser with optional config directory
 *
 * @param config_dir - Path to config directory (NULL = default)
 * @return true on success, false on failure
 */
bool asterix_wrapper_init(const char* config_dir);

/**
 * Load a specific ASTERIX category definition file
 *
 * @param xml_path - Path to XML category file
 * @return true on success, false on failure
 */
bool asterix_wrapper_load_category(const char* xml_path);

/**
 * Parse ASTERIX data from buffer
 *
 * @param data - Input data buffer
 * @param length - Length of input data
 * @param verbose - Enable verbose output
 * @param out_records - Output records (caller must free with asterix_wrapper_free_records)
 * @param error_buffer - Buffer for error messages
 * @param error_buffer_size - Size of error buffer
 * @return 0 on success, error code on failure
 */
int asterix_wrapper_parse(
    const uint8_t* data,
    size_t length,
    bool verbose,
    AsterixRecords** out_records,
    char* error_buffer,
    size_t error_buffer_size
);

/**
 * Parse ASTERIX data with offset and block count
 *
 * @param data - Input data buffer
 * @param length - Length of input data
 * @param offset - Byte offset to start parsing from
 * @param blocks_count - Maximum number of blocks to parse (0 = all)
 * @param verbose - Enable verbose output
 * @param out_records - Output records (caller must free)
 * @param out_bytes_consumed - Number of bytes consumed
 * @param out_remaining_blocks - Estimated remaining blocks
 * @param error_buffer - Buffer for error messages
 * @param error_buffer_size - Size of error buffer
 * @return 0 on success, error code on failure
 */
int asterix_wrapper_parse_with_offset(
    const uint8_t* data,
    size_t length,
    uint32_t offset,
    uint32_t blocks_count,
    bool verbose,
    AsterixRecords** out_records,
    size_t* out_bytes_consumed,
    size_t* out_remaining_blocks,
    char* error_buffer,
    size_t error_buffer_size
);

/**
 * Get description for category/item/field/value
 *
 * @param category - ASTERIX category (1-255)
 * @param item - Item ID (NULL for category description)
 * @param field - Field name (NULL for item description)
 * @param value - Value (NULL for field description)
 * @return Allocated description string (caller must free with asterix_wrapper_free_string)
 */
char* asterix_wrapper_describe(
    uint8_t category,
    const char* item,
    const char* field,
    const char* value
);

/**
 * Check if a category is defined
 *
 * @param category - ASTERIX category (1-255)
 * @return true if category is defined, false otherwise
 */
bool asterix_wrapper_is_category_defined(uint8_t category);

/**
 * Free records allocated by asterix_wrapper_parse*
 *
 * @param records - Records to free
 */
void asterix_wrapper_free_records(AsterixRecords* records);

/**
 * Free string allocated by asterix_wrapper_describe
 *
 * @param str - String to free
 */
void asterix_wrapper_free_string(char* str);

/**
 * Get last error message
 *
 * @return Last error message (internal buffer, do not free)
 */
const char* asterix_wrapper_get_last_error();

#ifdef __cplusplus
}
#endif

#endif // ASTERIX_NODE_PARSER_WRAPPER_H
