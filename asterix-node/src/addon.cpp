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

/**
 * @file addon.cpp
 * @brief N-API addon entry point for Node.js ASTERIX decoder bindings
 *
 * This file provides the N-API interface for the ASTERIX decoder, following
 * safety-critical design patterns from SAFETY_CRITICAL.md and BINDING_GUIDELINES.md.
 *
 * Key safety features:
 * - Comprehensive input validation at FFI boundary
 * - Integer overflow prevention
 * - Buffer bounds checking
 * - Null pointer validation
 * - Error code mapping to JavaScript exceptions
 *
 * @see docs/BINDING_GUIDELINES.md
 * @see docs/SAFETY_CRITICAL.md
 */

#include <napi.h>
#include "parser_wrapper.h"
#include <cstring>
#include <algorithm>

// Safety constants (from BINDING_GUIDELINES.md)
constexpr size_t MAX_ASTERIX_MESSAGE_SIZE = 65536;  // 64 KB
constexpr size_t MAX_BLOCKS_PER_CALL = 10000;       // DoS prevention

/**
 * Initialize ASTERIX parser with default config directory
 *
 * @param info.Env() - N-API environment
 * @param info[0] - Optional config directory path (string)
 * @returns undefined on success, throws on error
 */
Napi::Value Init(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    try {
        const char* config_dir = nullptr;
        std::string path_storage;  // Keep string alive for c_str()

        // Optional first argument: config directory path
        if (info.Length() > 0 && info[0].IsString()) {
            path_storage = info[0].As<Napi::String>().Utf8Value();
            const std::string& path = path_storage;

            // MEDIUM-004: Validate config path (defense-in-depth)
            if (path.empty()) {
                Napi::TypeError::New(env, "Config directory path cannot be empty")
                    .ThrowAsJavaScriptException();
                return env.Undefined();
            }

            // Path traversal check
            if (path.find("..") == 0 || path.rfind("/../", 0) == 0) {
                Napi::TypeError::New(env, "Invalid config path: path traversal detected")
                    .ThrowAsJavaScriptException();
                return env.Undefined();
            }

            // Length check
            if (path.length() > 4096) {
                Napi::TypeError::New(env, "Config directory path too long (max 4096 chars)")
                    .ThrowAsJavaScriptException();
                return env.Undefined();
            }

            config_dir = path_storage.c_str();
        }

        if (!asterix_wrapper_init(config_dir)) {
            const char* error = asterix_wrapper_get_last_error();
            std::string msg = "Failed to initialize ASTERIX parser";
            if (error && error[0] != '\0') {
                msg += ": ";
                msg += error;
            }
            Napi::Error::New(env, msg)
                .ThrowAsJavaScriptException();
            return env.Undefined();
        }

        return env.Undefined();
    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Initialization error: ") + e.what())
            .ThrowAsJavaScriptException();
        return env.Undefined();
    }
}

/**
 * Load a specific ASTERIX category definition file
 *
 * @param info[0] - XML category file path (string, required)
 * @returns undefined on success, throws on error
 */
Napi::Value LoadCategory(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Validate arguments
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Expected string argument: xml_path")
            .ThrowAsJavaScriptException();
        return env.Undefined();
    }

    try {
        std::string xml_path = info[0].As<Napi::String>().Utf8Value();

        // MEDIUM-004: Validate XML path
        if (xml_path.empty()) {
            Napi::TypeError::New(env, "XML path cannot be empty")
                .ThrowAsJavaScriptException();
            return env.Undefined();
        }

        // Path traversal check
        if (xml_path.find("..") == 0) {
            Napi::TypeError::New(env, "Invalid XML path: path traversal detected")
                .ThrowAsJavaScriptException();
            return env.Undefined();
        }

        // Length check
        if (xml_path.length() > 4096) {
            Napi::TypeError::New(env, "XML path too long (max 4096 chars)")
                .ThrowAsJavaScriptException();
            return env.Undefined();
        }

        if (!asterix_wrapper_load_category(xml_path.c_str())) {
            Napi::Error::New(env, std::string("Failed to load category file: ") + xml_path)
                .ThrowAsJavaScriptException();
            return env.Undefined();
        }

        return env.Undefined();
    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Error loading category: ") + e.what())
            .ThrowAsJavaScriptException();
        return env.Undefined();
    }
}

/**
 * Parse ASTERIX data from Buffer
 *
 * This is the main parsing function following the validated FFI boundary pattern
 * from BINDING_GUIDELINES.md Section 4 (FFI Boundary Design).
 *
 * @param info[0] - Buffer containing ASTERIX data (required)
 * @param info[1] - Options object { verbose: boolean } (optional)
 * @returns Array of parsed records on success, throws on error
 */
Napi::Value Parse(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // ========== INPUT VALIDATION (CRITICAL) ==========

    // 1. NULL POINTER CHECKS
    if (info.Length() < 1 || !info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Expected Buffer as first argument")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
    const uint8_t* data = buffer.Data();
    size_t length = buffer.Length();

    // 2. EMPTY INPUT CHECK (CRITICAL-005)
    if (length == 0) {
        Napi::TypeError::New(env, "Empty input data")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // 3. SIZE LIMITS (DoS prevention)
    if (length > MAX_ASTERIX_MESSAGE_SIZE) {
        Napi::TypeError::New(env,
            std::string("Input data too large: ") + std::to_string(length) +
            " bytes (maximum " + std::to_string(MAX_ASTERIX_MESSAGE_SIZE) + " bytes)")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // 4. Parse options (optional second parameter)
    bool verbose = false;
    if (info.Length() > 1 && info[1].IsObject()) {
        Napi::Object options = info[1].As<Napi::Object>();
        if (options.Has("verbose") && options.Get("verbose").IsBoolean()) {
            verbose = options.Get("verbose").As<Napi::Boolean>().Value();
        }
    }

    // ========== CALL SAFE CORE PARSER ==========

    try {
        AsterixRecords* records = nullptr;
        char error_buffer[1024] = {0};

        int result = asterix_wrapper_parse(
            data,
            length,
            verbose,
            &records,
            error_buffer,
            sizeof(error_buffer)
        );

        if (result != 0 || records == nullptr) {
            std::string error_msg = error_buffer[0] ? error_buffer : "Unknown parsing error";
            Napi::Error::New(env, error_msg).ThrowAsJavaScriptException();
            return env.Null();
        }

        // Convert C++ records to JavaScript array
        Napi::Array js_records = Napi::Array::New(env, records->count);

        for (size_t i = 0; i < records->count; i++) {
            const AsterixRecord& record = records->records[i];

            Napi::Object js_record = Napi::Object::New(env);
            js_record.Set("category", Napi::Number::New(env, record.category));
            js_record.Set("length", Napi::Number::New(env, record.length));
            js_record.Set("timestamp_ms", Napi::Number::New(env, record.timestamp_ms));
            js_record.Set("crc", Napi::Number::New(env, record.crc));

            if (record.hex_data) {
                js_record.Set("hex_data", Napi::String::New(env, record.hex_data));
            }

            if (record.json_data) {
                // Parse JSON string to JavaScript object
                std::string json_str(record.json_data);
                Napi::Value parsed = env.Global().Get("JSON")
                    .As<Napi::Object>().Get("parse")
                    .As<Napi::Function>().Call({Napi::String::New(env, json_str)});
                js_record.Set("items", parsed);
            } else {
                js_record.Set("items", Napi::Object::New(env));
            }

            js_records[i] = js_record;
        }

        // Cleanup C++ allocated memory
        asterix_wrapper_free_records(records);

        return js_records;
    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Parsing error: ") + e.what())
            .ThrowAsJavaScriptException();
        return env.Null();
    }
}

/**
 * Parse ASTERIX data with offset and block count for incremental parsing
 *
 * @param info[0] - Buffer containing ASTERIX data (required)
 * @param info[1] - Offset to start parsing from (number, required)
 * @param info[2] - Maximum blocks to parse (number, required)
 * @param info[3] - Options object { verbose: boolean } (optional)
 * @returns Object { records: Array, bytesConsumed: number, remainingBlocks: number }
 */
Napi::Value ParseWithOffset(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // ========== INPUT VALIDATION (CRITICAL) ==========

    // 1. Validate arguments count and types
    if (info.Length() < 3) {
        Napi::TypeError::New(env, "Expected at least 3 arguments: buffer, offset, blocksCount")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsBuffer()) {
        Napi::TypeError::New(env, "First argument must be Buffer")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[1].IsNumber()) {
        Napi::TypeError::New(env, "Second argument (offset) must be number")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[2].IsNumber()) {
        Napi::TypeError::New(env, "Third argument (blocksCount) must be number")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
    const uint8_t* data = buffer.Data();
    size_t length = buffer.Length();

    // CRITICAL-002: Validate offset
    uint32_t offset = info[1].As<Napi::Number>().Uint32Value();
    if (offset >= length) {
        Napi::TypeError::New(env,
            std::string("Offset ") + std::to_string(offset) +
            " exceeds data length " + std::to_string(length))
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // HIGH-002: Validate blocks_count
    uint32_t blocks_count = info[2].As<Napi::Number>().Uint32Value();
    if (blocks_count > MAX_BLOCKS_PER_CALL) {
        Napi::TypeError::New(env,
            std::string("blocksCount ") + std::to_string(blocks_count) +
            " exceeds maximum (" + std::to_string(MAX_BLOCKS_PER_CALL) + ")")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // Empty data check
    if (length == 0) {
        Napi::TypeError::New(env, "Empty input data")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // Size limit check
    if (length > MAX_ASTERIX_MESSAGE_SIZE) {
        Napi::TypeError::New(env,
            std::string("Input data too large: ") + std::to_string(length) + " bytes")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // Integer overflow check: offset + estimated data size
    if (offset > UINT32_MAX - (blocks_count * 256)) {
        Napi::TypeError::New(env, "Offset + blocksCount would overflow")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    // Parse options
    bool verbose = false;
    if (info.Length() > 3 && info[3].IsObject()) {
        Napi::Object options = info[3].As<Napi::Object>();
        if (options.Has("verbose") && options.Get("verbose").IsBoolean()) {
            verbose = options.Get("verbose").As<Napi::Boolean>().Value();
        }
    }

    // ========== CALL SAFE CORE PARSER ==========

    try {
        AsterixRecords* records = nullptr;
        size_t bytes_consumed = 0;
        size_t remaining_blocks = 0;
        char error_buffer[1024] = {0};

        int result = asterix_wrapper_parse_with_offset(
            data,
            length,
            offset,
            blocks_count,
            verbose,
            &records,
            &bytes_consumed,
            &remaining_blocks,
            error_buffer,
            sizeof(error_buffer)
        );

        if (result != 0 || records == nullptr) {
            std::string error_msg = error_buffer[0] ? error_buffer : "Unknown parsing error";
            Napi::Error::New(env, error_msg).ThrowAsJavaScriptException();
            return env.Null();
        }

        // Convert records to JavaScript array (same as Parse())
        Napi::Array js_records = Napi::Array::New(env, records->count);

        for (size_t i = 0; i < records->count; i++) {
            const AsterixRecord& record = records->records[i];

            Napi::Object js_record = Napi::Object::New(env);
            js_record.Set("category", Napi::Number::New(env, record.category));
            js_record.Set("length", Napi::Number::New(env, record.length));
            js_record.Set("timestamp_ms", Napi::Number::New(env, record.timestamp_ms));
            js_record.Set("crc", Napi::Number::New(env, record.crc));

            if (record.hex_data) {
                js_record.Set("hex_data", Napi::String::New(env, record.hex_data));
            }

            if (record.json_data) {
                std::string json_str(record.json_data);
                Napi::Value parsed = env.Global().Get("JSON")
                    .As<Napi::Object>().Get("parse")
                    .As<Napi::Function>().Call({Napi::String::New(env, json_str)});
                js_record.Set("items", parsed);
            } else {
                js_record.Set("items", Napi::Object::New(env));
            }

            js_records[i] = js_record;
        }

        // Build result object
        Napi::Object result_obj = Napi::Object::New(env);
        result_obj.Set("records", js_records);
        result_obj.Set("bytesConsumed", Napi::Number::New(env, bytes_consumed));
        result_obj.Set("remainingBlocks", Napi::Number::New(env, remaining_blocks));

        // Cleanup
        asterix_wrapper_free_records(records);

        return result_obj;
    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Parsing error: ") + e.what())
            .ThrowAsJavaScriptException();
        return env.Null();
    }
}

/**
 * Get description for category/item/field/value
 *
 * @param info[0] - Category number (required)
 * @param info[1] - Item ID (optional, null for category description)
 * @param info[2] - Field name (optional)
 * @param info[3] - Value (optional)
 * @returns String description or throws on error
 */
Napi::Value Describe(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // Validate arguments
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected number as first argument: category")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        uint8_t category = static_cast<uint8_t>(info[0].As<Napi::Number>().Uint32Value());

        // MEDIUM-003: Validate category range
        if (category == 0) {
            Napi::TypeError::New(env, "Invalid ASTERIX category: 0 (valid range: 1-255)")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        const char* item = nullptr;
        const char* field = nullptr;
        const char* value = nullptr;

        if (info.Length() > 1 && info[1].IsString()) {
            std::string item_str = info[1].As<Napi::String>().Utf8Value();
            item = item_str.c_str();
        }

        if (info.Length() > 2 && info[2].IsString()) {
            std::string field_str = info[2].As<Napi::String>().Utf8Value();
            field = field_str.c_str();
        }

        if (info.Length() > 3 && info[3].IsString()) {
            std::string value_str = info[3].As<Napi::String>().Utf8Value();
            value = value_str.c_str();
        }

        char* description = asterix_wrapper_describe(category, item, field, value);

        if (description == nullptr) {
            Napi::Error::New(env, "Failed to get description")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        Napi::String result = Napi::String::New(env, description);

        // Free C++ allocated string
        asterix_wrapper_free_string(description);

        return result;
    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Description error: ") + e.what())
            .ThrowAsJavaScriptException();
        return env.Null();
    }
}

/**
 * Check if a category is defined
 *
 * @param info[0] - Category number (required)
 * @returns Boolean true if category is defined, false otherwise
 */
Napi::Value IsCategoryDefined(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected number argument: category")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        uint8_t category = static_cast<uint8_t>(info[0].As<Napi::Number>().Uint32Value());
        bool is_defined = asterix_wrapper_is_category_defined(category);
        return Napi::Boolean::New(env, is_defined);
    } catch (const std::exception& e) {
        Napi::Error::New(env, std::string("Error checking category: ") + e.what())
            .ThrowAsJavaScriptException();
        return env.Null();
    }
}

/**
 * N-API module initialization
 *
 * Exports all public functions to JavaScript
 */
Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    // Export functions
    exports.Set("init", Napi::Function::New(env, Init));
    exports.Set("loadCategory", Napi::Function::New(env, LoadCategory));
    exports.Set("parse", Napi::Function::New(env, Parse));
    exports.Set("parseWithOffset", Napi::Function::New(env, ParseWithOffset));
    exports.Set("describe", Napi::Function::New(env, Describe));
    exports.Set("isCategoryDefined", Napi::Function::New(env, IsCategoryDefined));

    // Export version
    exports.Set("version", Napi::String::New(env, "2.8.10"));

    return exports;
}

NODE_API_MODULE(asterix, InitAll)
