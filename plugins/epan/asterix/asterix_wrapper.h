/* asterix_wrapper.h
 *
 * C Wrapper for ASTERIX C++ Core Library
 * Provides C-compatible interface for Wireshark dissector
 *
 * Copyright (C) 2025
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ASTERIX_WRAPPER_H
#define ASTERIX_WRAPPER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle to ASTERIX parser (singleton) */
typedef struct asterix_parser_t asterix_parser_t;

/* Data item structure */
typedef struct {
    uint16_t item_number;          /* e.g., 10 for I048/010 */
    const char* item_name;         /* e.g., "Data Source Identifier" */
    const uint8_t* raw_data;       /* Raw binary data */
    size_t raw_length;             /* Length of raw data */
    const char* formatted_value;   /* Human-readable value */
    const char* description;       /* Full description */
} asterix_data_item_t;

/* Data record structure */
typedef struct {
    uint8_t category;              /* ASTERIX category */
    uint16_t length;               /* Record length in bytes */
    asterix_data_item_t* items;    /* Array of data items */
    size_t item_count;             /* Number of items */
} asterix_record_t;

/* Data block structure */
typedef struct {
    uint8_t category;              /* ASTERIX category */
    uint16_t length;               /* Block length in bytes */
    asterix_record_t* records;     /* Array of records */
    size_t record_count;           /* Number of records */
} asterix_data_block_t;

/**
 * Initialize ASTERIX parser
 *
 * @param config_dir Path to configuration directory (NULL for default)
 * @return Parser handle on success, NULL on failure
 */
asterix_parser_t* asterix_init(const char* config_dir);

/**
 * Check if ASTERIX category is defined
 *
 * @param parser Parser handle
 * @param category ASTERIX category (1-255)
 * @return 1 if defined, 0 otherwise
 */
int asterix_is_category_defined(asterix_parser_t* parser, uint8_t category);

/**
 * Get category description
 *
 * @param parser Parser handle
 * @param category ASTERIX category (1-255)
 * @return Description string (do not free), or NULL if not defined
 */
const char* asterix_get_category_description(asterix_parser_t* parser, uint8_t category);

/**
 * Parse ASTERIX data block
 *
 * @param parser Parser handle
 * @param data Raw ASTERIX data
 * @param data_len Length of data in bytes
 * @param block Output data block (caller must free with asterix_free_block)
 * @return 0 on success, negative on error
 */
int asterix_parse_block(asterix_parser_t* parser,
                        const uint8_t* data,
                        size_t data_len,
                        asterix_data_block_t** block);

/**
 * Parse single ASTERIX record from data
 *
 * @param parser Parser handle
 * @param category ASTERIX category
 * @param data Raw record data (after FSPEC)
 * @param data_len Length of data in bytes
 * @param fspec FSPEC bytes
 * @param fspec_len Length of FSPEC in bytes
 * @param record Output record (caller must free with asterix_free_record)
 * @return 0 on success, negative on error
 */
int asterix_parse_record(asterix_parser_t* parser,
                         uint8_t category,
                         const uint8_t* data,
                         size_t data_len,
                         const uint8_t* fspec,
                         size_t fspec_len,
                         asterix_record_t** record);

/**
 * Get description for specific data item field value
 *
 * @param parser Parser handle
 * @param category ASTERIX category
 * @param item_number Item number (e.g., 10 for I048/010)
 * @param field_name Field name (can be NULL)
 * @param value Value to describe
 * @return Description string (caller must free), or NULL on error
 */
char* asterix_describe_value(asterix_parser_t* parser,
                              uint8_t category,
                              uint16_t item_number,
                              const char* field_name,
                              int64_t value);

/**
 * Free data block
 *
 * @param block Data block to free
 */
void asterix_free_block(asterix_data_block_t* block);

/**
 * Free data record
 *
 * @param record Data record to free
 */
void asterix_free_record(asterix_record_t* record);

/**
 * Destroy parser (cleanup)
 *
 * @param parser Parser handle
 */
void asterix_destroy(asterix_parser_t* parser);

/**
 * Get last error message
 *
 * @param parser Parser handle
 * @return Error message string (do not free), or NULL if no error
 */
const char* asterix_get_last_error(asterix_parser_t* parser);

#ifdef __cplusplus
}
#endif

#endif /* ASTERIX_WRAPPER_H */
