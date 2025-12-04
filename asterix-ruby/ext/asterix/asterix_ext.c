/*
 *  Copyright (c) 2025 ASTERIX Contributors
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
 */

#include <ruby.h>
#include "ruby_parser.h"
#include <string.h>

// Safety limits for FFI boundary validation
#define MAX_ASTERIX_MESSAGE_SIZE (65536)  // 64 KB - reasonable max for ASTERIX message
#define MAX_BLOCKS_PER_CALL (10000)       // Maximum blocks to parse in single call
#define MAX_FILENAME_LENGTH (4096)        // Maximum config file path length

static int bInitialized = 0;

/*
 * Initialize ASTERIX parser with XML category definition
 *
 * @param filename [String] Path to XML configuration file
 * @return [Integer] 0 if successful
 * @raise [ArgumentError] if filename is invalid
 * @raise [IOError] if file cannot be read
 * @raise [SyntaxError] if XML is malformed
 */
static VALUE asterix_init(VALUE self, VALUE filename) {
    // MEDIUM-004 FIX: Validate filename parameter type
    Check_Type(filename, T_STRING);

    const char *config_file = StringValueCStr(filename);
    size_t len = RSTRING_LEN(filename);

    // MEDIUM-004 FIX: Validate filename not empty
    if (len == 0) {
        rb_raise(rb_eArgError, "Filename cannot be empty");
    }

    // MEDIUM-004 FIX: Check for path traversal attacks
    // Defense-in-depth: Block obvious traversal attempts
    if (strncmp(config_file, "../", 3) == 0 ||
        strncmp(config_file, "..\\", 3) == 0 ||
        strcmp(config_file, "..") == 0) {
        rb_raise(rb_eArgError, "Invalid filename: path traversal detected (..)");
    }

    // MEDIUM-004 FIX: Validate filename length
    if (len > MAX_FILENAME_LENGTH) {
        rb_raise(rb_eArgError, "Filename too long (maximum %d characters)", MAX_FILENAME_LENGTH);
    }

    int ret = ruby_init_asterix(config_file);
    if (ret == 0) {
        bInitialized = 1;
        return INT2NUM(0);
    }

    // Error already raised by ruby_init_asterix via rb_raise
    return Qnil;
}

/*
 * Parse ASTERIX data from binary string
 *
 * @param data [String] Binary ASTERIX data (must be ASCII-8BIT encoding)
 * @param verbose [Boolean] Include descriptions in output (default: true)
 * @return [Array<Hash>] Parsed ASTERIX records
 * @raise [ArgumentError] if data is invalid
 * @raise [RuntimeError] if parser not initialized
 */
static VALUE asterix_parse(int argc, VALUE *argv, VALUE self) {
    VALUE data, verbose_val;
    int verbose = 1;

    // Parse arguments: data (required), verbose (optional, default true)
    rb_scan_args(argc, argv, "11", &data, &verbose_val);

    if (!NIL_P(verbose_val)) {
        verbose = RTEST(verbose_val) ? 1 : 0;
    }

    // Validate data is a string
    Check_Type(data, T_STRING);

    const unsigned char *buffer = (const unsigned char *)RSTRING_PTR(data);
    size_t len = RSTRING_LEN(data);

    // CRITICAL-003 FIX: Validate buffer length
    if (len == 0) {
        rb_raise(rb_eArgError, "Empty input data");
    }

    if (len > MAX_ASTERIX_MESSAGE_SIZE) {
        rb_raise(rb_eArgError, "Input data too large: %zu bytes (maximum %d bytes)",
                 len, MAX_ASTERIX_MESSAGE_SIZE);
    }

    // HIGH-001 FIX: Check initialization status
    if (!bInitialized) {
        rb_raise(rb_eRuntimeError,
                 "ASTERIX parser not initialized. Call Asterix.init first.");
    }

    VALUE result = ruby_parse(buffer, len, verbose);
    if (NIL_P(result)) {
        return rb_ary_new();
    }
    return result;
}

/*
 * Parse ASTERIX data with incremental offset tracking
 *
 * @param data [String] Binary ASTERIX data
 * @param offset [Integer] Byte offset to start parsing (default: 0)
 * @param blocks_count [Integer] Maximum blocks to parse (default: 1000)
 * @param verbose [Boolean] Include descriptions (default: true)
 * @return [Array(Array<Hash>, Integer)] Tuple of [records, new_offset]
 * @raise [ArgumentError] if parameters are invalid
 */
static VALUE asterix_parse_with_offset(int argc, VALUE *argv, VALUE self) {
    VALUE data, offset_val, blocks_val, verbose_val;
    unsigned int offset = 0;
    unsigned int blocks_count = 1000;
    int verbose = 1;

    // Parse arguments: data, offset (optional), blocks_count (optional), verbose (optional)
    rb_scan_args(argc, argv, "13", &data, &offset_val, &blocks_val, &verbose_val);

    // Validate data is a string
    Check_Type(data, T_STRING);

    const unsigned char *buffer = (const unsigned char *)RSTRING_PTR(data);
    size_t len = RSTRING_LEN(data);

    // Parse optional offset
    if (!NIL_P(offset_val)) {
        Check_Type(offset_val, T_FIXNUM);
        offset = NUM2UINT(offset_val);
    }

    // Parse optional blocks_count
    if (!NIL_P(blocks_val)) {
        Check_Type(blocks_val, T_FIXNUM);
        blocks_count = NUM2UINT(blocks_val);
    }

    // Parse optional verbose
    if (!NIL_P(verbose_val)) {
        verbose = RTEST(verbose_val) ? 1 : 0;
    }

    // CRITICAL-002 FIX: Validate offset bounds
    if (offset >= len) {
        rb_raise(rb_eArgError, "Offset %u exceeds data length %zu", offset, len);
    }

    // HIGH-002 FIX: Validate blocks_count
    if (blocks_count == 0) {
        rb_raise(rb_eArgError, "blocks_count must be > 0");
    }

    if (blocks_count > MAX_BLOCKS_PER_CALL) {
        rb_raise(rb_eArgError, "blocks_count %u exceeds maximum (%d)",
                 blocks_count, MAX_BLOCKS_PER_CALL);
    }

    // HIGH-001 FIX: Check initialization status
    if (!bInitialized) {
        rb_raise(rb_eRuntimeError,
                 "ASTERIX parser not initialized. Call Asterix.init first.");
    }

    VALUE result = ruby_parse_with_offset(buffer, len, offset, blocks_count, verbose);
    if (NIL_P(result)) {
        // Return empty array and original offset
        VALUE empty = rb_ary_new();
        VALUE offset_num = UINT2NUM(offset);
        return rb_ary_new_from_args(2, empty, offset_num);
    }
    return result;
}

/*
 * Get human-readable description for ASTERIX category/item/field/value
 *
 * @param category [Integer] ASTERIX category (1-255)
 * @param item [String, nil] Item name (e.g., "010"), optional
 * @param field [String, nil] Field name, optional
 * @param value [String, nil] Field value, optional
 * @return [String] Description text
 * @raise [ArgumentError] if category is invalid
 */
static VALUE asterix_describe(int argc, VALUE *argv, VALUE self) {
    VALUE category_val, item_val, field_val, value_val;
    int category;
    const char *item = NULL;
    const char *field = NULL;
    const char *value = NULL;

    // Parse arguments: category (required), item, field, value (all optional)
    rb_scan_args(argc, argv, "13", &category_val, &item_val, &field_val, &value_val);

    // Validate category
    Check_Type(category_val, T_FIXNUM);
    category = NUM2INT(category_val);

    // MEDIUM-003 FIX: Validate category is within valid ASTERIX range
    if (category < 1 || category > 255) {
        rb_raise(rb_eArgError, "Invalid ASTERIX category: %d (valid range: 1-255)", category);
    }

    // Parse optional string arguments
    if (!NIL_P(item_val)) {
        Check_Type(item_val, T_STRING);
        item = StringValueCStr(item_val);
    }

    if (!NIL_P(field_val)) {
        Check_Type(field_val, T_STRING);
        field = StringValueCStr(field_val);
    }

    if (!NIL_P(value_val)) {
        Check_Type(value_val, T_STRING);
        value = StringValueCStr(value_val);
    }

    return ruby_describe(category, item, field, value);
}

/*
 * Initialize the Asterix module
 */
void Init_asterix_ext(void) {
    VALUE mAsterix = rb_define_module("AsterixNative");

    rb_define_module_function(mAsterix, "init", asterix_init, 1);
    rb_define_module_function(mAsterix, "parse", asterix_parse, -1);
    rb_define_module_function(mAsterix, "parse_with_offset", asterix_parse_with_offset, -1);
    rb_define_module_function(mAsterix, "describe", asterix_describe, -1);
}
