/*
 *  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Asterix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 *
 * C wrapper for ASTERIX parser - for use with Go cgo
 */

#ifndef ASTERIX_GO_H
#define ASTERIX_GO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
#define ASTERIX_OK              0
#define ASTERIX_ERR_INIT       -1
#define ASTERIX_ERR_PARSE      -2
#define ASTERIX_ERR_INVALID    -3
#define ASTERIX_ERR_MEMORY     -4

/* Maximum sizes for safety */
#define ASTERIX_MAX_MESSAGE_SIZE  65536
#define ASTERIX_MAX_PATH_LENGTH   4096
#define ASTERIX_MAX_BLOCKS        10000

/* Opaque parser handle */
typedef struct AsterixParser AsterixParser;

/* Record structure returned by parsing */
typedef struct {
    uint8_t category;
    uint16_t length;
    uint64_t timestamp_us;  /* Microseconds since epoch */
    char* json_data;        /* JSON representation of parsed data */
    uint32_t crc;
} AsterixRecord;

/* Parse result */
typedef struct {
    AsterixRecord* records;
    size_t count;
    size_t bytes_consumed;
    int error_code;
    char* error_message;
} AsterixParseResult;

/**
 * Initialize the ASTERIX parser with configuration
 * @param config_path Path to asterix.ini or XML config file
 * @return ASTERIX_OK on success, error code on failure
 */
int asterix_init(const char* config_path);

/**
 * Check if parser is initialized
 * @return 1 if initialized, 0 otherwise
 */
int asterix_is_initialized(void);

/**
 * Parse ASTERIX data
 * @param data Raw ASTERIX binary data
 * @param length Length of data in bytes
 * @param verbose Include descriptions in output (1=yes, 0=no)
 * @return Parse result (caller must free with asterix_free_result)
 */
AsterixParseResult* asterix_parse(const uint8_t* data, size_t length, int verbose);

/**
 * Parse ASTERIX data with offset and block limit
 * @param data Raw ASTERIX binary data
 * @param length Length of data in bytes
 * @param offset Byte offset to start parsing
 * @param max_blocks Maximum number of blocks to parse (0 = all)
 * @param verbose Include descriptions in output
 * @return Parse result (caller must free with asterix_free_result)
 */
AsterixParseResult* asterix_parse_with_offset(
    const uint8_t* data,
    size_t length,
    size_t offset,
    size_t max_blocks,
    int verbose
);

/**
 * Get description for ASTERIX category/item/field
 * @param category ASTERIX category (1-255)
 * @param item Item ID (e.g., "I010") or NULL
 * @param field Field name or NULL
 * @param value Field value or NULL
 * @return Description string (caller must free) or NULL
 */
char* asterix_describe(int category, const char* item, const char* field, const char* value);

/**
 * Check if a category is defined
 * @param category ASTERIX category (1-255)
 * @return 1 if defined, 0 otherwise
 */
int asterix_is_category_defined(int category);

/**
 * Free parse result
 * @param result Result to free
 */
void asterix_free_result(AsterixParseResult* result);

/**
 * Free string returned by asterix_describe
 * @param str String to free
 */
void asterix_free_string(char* str);

/**
 * Get library version
 * @return Version string (do not free)
 */
const char* asterix_version(void);

/**
 * Get last error message
 * @return Error message (do not free) or NULL
 */
const char* asterix_get_last_error(void);

#ifdef __cplusplus
}
#endif

#endif /* ASTERIX_GO_H */
