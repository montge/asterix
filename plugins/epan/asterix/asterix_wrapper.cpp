/* asterix_wrapper.cpp
 *
 * C Wrapper Implementation for ASTERIX C++ Core Library
 * Bridges C++ ASTERIX core with C Wireshark dissector
 *
 * Copyright (C) 2025
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "asterix_wrapper.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <exception>

// Include ASTERIX C++ headers
#include <AsterixDefinition.h>
#include <Category.h>
#include <DataBlock.h>
#include <DataRecord.h>
#include <DataItem.h>
#include <DataItemDescription.h>

// Thread-local error message
static thread_local std::string g_last_error;

// Parser handle (singleton wrapper)
struct asterix_parser_t {
    bool initialized;
    std::string config_dir;
};

/**
 * Set last error message
 */
static void set_error(const char* msg) {
    if (msg) {
        g_last_error = msg;
    } else {
        g_last_error.clear();
    }
}

/**
 * Set last error from exception
 */
static void set_error(const std::exception& e) {
    g_last_error = e.what();
}

extern "C" {

/**
 * Initialize ASTERIX parser
 */
asterix_parser_t* asterix_init(const char* config_dir) {
    try {
        asterix_parser_t* parser = new asterix_parser_t();
        parser->initialized = false;

        if (config_dir) {
            parser->config_dir = config_dir;
        } else {
            // Use default config directory
            // Assuming install/share/asterix/config relative to plugin
            parser->config_dir = "../../../install/share/asterix/config";
        }

        // Initialize ASTERIX definition singleton
        if (!AsterixDefinition::instance()->init(parser->config_dir.c_str())) {
            set_error("Failed to initialize ASTERIX definitions");
            delete parser;
            return nullptr;
        }

        parser->initialized = true;
        set_error(nullptr);
        return parser;

    } catch (const std::exception& e) {
        set_error(e);
        return nullptr;
    }
}

/**
 * Check if category is defined
 */
int asterix_is_category_defined(asterix_parser_t* parser, uint8_t category) {
    if (!parser || !parser->initialized) {
        return 0;
    }

    try {
        Category* cat = AsterixDefinition::instance()->getCategory(category);
        return (cat != nullptr) ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

/**
 * Get category description
 */
const char* asterix_get_category_description(asterix_parser_t* parser, uint8_t category) {
    if (!parser || !parser->initialized) {
        return nullptr;
    }

    try {
        Category* cat = AsterixDefinition::instance()->getCategory(category);
        if (cat) {
            static thread_local std::string desc;
            desc = cat->getDescription();
            return desc.c_str();
        }
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

/**
 * Parse ASTERIX data block
 */
int asterix_parse_block(asterix_parser_t* parser,
                        const uint8_t* data,
                        size_t data_len,
                        asterix_data_block_t** block) {
    if (!parser || !parser->initialized || !data || !block) {
        set_error("Invalid parameters");
        return -1;
    }

    if (data_len < 3) {
        set_error("Data too short (minimum 3 bytes)");
        return -1;
    }

    try {
        // Parse data block using ASTERIX core
        DataBlock* db = new DataBlock();
        if (!db->parse(data, data_len)) {
            set_error("Failed to parse data block");
            delete db;
            return -1;
        }

        // Allocate output structure
        asterix_data_block_t* output = (asterix_data_block_t*)calloc(1, sizeof(asterix_data_block_t));
        if (!output) {
            set_error("Memory allocation failed");
            delete db;
            return -1;
        }

        output->category = db->getCategory();
        output->length = db->getLength();
        output->record_count = db->getRecordCount();

        // Allocate records array
        if (output->record_count > 0) {
            output->records = (asterix_record_t*)calloc(output->record_count, sizeof(asterix_record_t));
            if (!output->records) {
                set_error("Memory allocation failed");
                free(output);
                delete db;
                return -1;
            }

            // Convert each record
            for (size_t i = 0; i < output->record_count; i++) {
                DataRecord* rec = db->getRecord(i);
                asterix_record_t* out_rec = &output->records[i];

                out_rec->category = output->category;
                out_rec->length = rec->getLength();
                out_rec->item_count = rec->getItemCount();

                // Allocate items array
                if (out_rec->item_count > 0) {
                    out_rec->items = (asterix_data_item_t*)calloc(out_rec->item_count, sizeof(asterix_data_item_t));
                    if (!out_rec->items) {
                        set_error("Memory allocation failed");
                        asterix_free_block(output);
                        delete db;
                        return -1;
                    }

                    // Convert each data item
                    for (size_t j = 0; j < out_rec->item_count; j++) {
                        DataItem* item = rec->getItem(j);
                        asterix_data_item_t* out_item = &out_rec->items[j];

                        DataItemDescription* desc = item->getDescription();
                        out_item->item_number = desc ? desc->getItemNumber() : 0;
                        out_item->item_name = desc ? strdup(desc->getName().c_str()) : nullptr;
                        out_item->raw_data = item->getData();
                        out_item->raw_length = item->getLength();
                        out_item->formatted_value = strdup(item->toString().c_str());
                        out_item->description = desc ? strdup(desc->getDescription().c_str()) : nullptr;
                    }
                }
            }
        }

        *block = output;
        delete db;
        set_error(nullptr);
        return 0;

    } catch (const std::exception& e) {
        set_error(e);
        return -1;
    }
}

/**
 * Parse single ASTERIX record
 */
int asterix_parse_record(asterix_parser_t* parser,
                         uint8_t category,
                         const uint8_t* data,
                         size_t data_len,
                         const uint8_t* fspec,
                         size_t fspec_len,
                         asterix_record_t** record) {
    if (!parser || !parser->initialized || !data || !fspec || !record) {
        set_error("Invalid parameters");
        return -1;
    }

    try {
        Category* cat = AsterixDefinition::instance()->getCategory(category);
        if (!cat) {
            set_error("Category not defined");
            return -1;
        }

        // Create data record
        DataRecord* rec = new DataRecord(cat);

        // Parse FSPEC
        if (!rec->parseFSPEC(fspec, fspec_len)) {
            set_error("Failed to parse FSPEC");
            delete rec;
            return -1;
        }

        // Parse data items
        if (!rec->parseDataItems(data, data_len)) {
            set_error("Failed to parse data items");
            delete rec;
            return -1;
        }

        // Allocate output structure
        asterix_record_t* output = (asterix_record_t*)calloc(1, sizeof(asterix_record_t));
        if (!output) {
            set_error("Memory allocation failed");
            delete rec;
            return -1;
        }

        output->category = category;
        output->length = rec->getLength();
        output->item_count = rec->getItemCount();

        // Allocate items array
        if (output->item_count > 0) {
            output->items = (asterix_data_item_t*)calloc(output->item_count, sizeof(asterix_data_item_t));
            if (!output->items) {
                set_error("Memory allocation failed");
                free(output);
                delete rec;
                return -1;
            }

            // Convert each data item
            for (size_t i = 0; i < output->item_count; i++) {
                DataItem* item = rec->getItem(i);
                asterix_data_item_t* out_item = &output->items[i];

                DataItemDescription* desc = item->getDescription();
                out_item->item_number = desc ? desc->getItemNumber() : 0;
                out_item->item_name = desc ? strdup(desc->getName().c_str()) : nullptr;
                out_item->raw_data = item->getData();
                out_item->raw_length = item->getLength();
                out_item->formatted_value = strdup(item->toString().c_str());
                out_item->description = desc ? strdup(desc->getDescription().c_str()) : nullptr;
            }
        }

        *record = output;
        delete rec;
        set_error(nullptr);
        return 0;

    } catch (const std::exception& e) {
        set_error(e);
        return -1;
    }
}

/**
 * Get description for value
 */
char* asterix_describe_value(asterix_parser_t* parser,
                              uint8_t category,
                              uint16_t item_number,
                              const char* field_name,
                              int64_t value) {
    if (!parser || !parser->initialized) {
        return nullptr;
    }

    try {
        Category* cat = AsterixDefinition::instance()->getCategory(category);
        if (!cat) {
            return nullptr;
        }

        DataItemDescription* desc = cat->getDataItemDescription(item_number);
        if (!desc) {
            return nullptr;
        }

        // Get value description
        std::string value_desc;
        if (field_name) {
            value_desc = desc->getFieldValueDescription(field_name, value);
        } else {
            value_desc = desc->getValueDescription(value);
        }

        if (value_desc.empty()) {
            return nullptr;
        }

        return strdup(value_desc.c_str());

    } catch (...) {
        return nullptr;
    }
}

/**
 * Free data block
 */
void asterix_free_block(asterix_data_block_t* block) {
    if (!block) {
        return;
    }

    if (block->records) {
        for (size_t i = 0; i < block->record_count; i++) {
            asterix_free_record(&block->records[i]);
        }
        free(block->records);
    }

    free(block);
}

/**
 * Free data record
 */
void asterix_free_record(asterix_record_t* record) {
    if (!record) {
        return;
    }

    if (record->items) {
        for (size_t i = 0; i < record->item_count; i++) {
            asterix_data_item_t* item = &record->items[i];
            free((void*)item->item_name);
            free((void*)item->formatted_value);
            free((void*)item->description);
        }
        free(record->items);
    }

    // Don't free record itself if it's part of an array
}

/**
 * Destroy parser
 */
void asterix_destroy(asterix_parser_t* parser) {
    if (parser) {
        delete parser;
    }
}

/**
 * Get last error message
 */
const char* asterix_get_last_error(asterix_parser_t* parser) {
    (void)parser; // Unused
    return g_last_error.empty() ? nullptr : g_last_error.c_str();
}

} // extern "C"
