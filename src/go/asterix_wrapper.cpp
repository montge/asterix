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

#include "asterix.h"

#include <cstring>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>
#include <memory>

#ifdef _WIN32
  #include "win32_compat.h"
#else
  #include <sys/time.h>
#endif

#include "AsterixDefinition.h"
#include "XMLParser.h"
#include "InputParser.h"
#include "Tracer.h"
#include "AsterixData.h"
#include "DataBlock.h"
#include "DataRecord.h"
#include "DataItem.h"
#include "DataItemDescription.h"
#include "Category.h"

// Global state
static std::unique_ptr<AsterixDefinition> g_pDefinition;
static std::unique_ptr<InputParser> g_pInputParser;
static bool g_bInitialized = false;
static std::string g_lastError;
static const char* g_version = "2.9.0";

// Extern declarations for globals defined in globals.cpp
extern bool gFiltering;
extern bool gSynchronous;
extern const char* gAsterixDefinitionsFile;
extern bool gVerbose;
extern bool gForceRouting;
extern int gHeartbeat;

// JSON format type from AsterixData
// CAsterixFormat::EJSON = 2, EJSONH = 3, EJSONE = 4
static const unsigned int FORMAT_JSON = 2;
static const unsigned int FORMAT_JSON_VERBOSE = 4;

// Debug trace function for Tracer
static void debug_trace(char const* format, ...) {
    // Capture error messages for g_lastError
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer) - 1, format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = '\0';
    g_lastError = buffer;
}

// Helper to allocate and copy string
static char* alloc_string(const std::string& str) {
    if (str.empty()) return nullptr;
    char* result = static_cast<char*>(malloc(str.size() + 1));
    if (result) {
        memcpy(result, str.c_str(), str.size() + 1);
    }
    return result;
}

// Get current timestamp in microseconds
static uint64_t get_timestamp_us() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

// CRC32 calculation (simple implementation)
static uint32_t crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            // Use conditional instead of branchless trick to avoid MSVC C4146 warning
            uint32_t mask = (crc & 1) ? 0xEDB88320 : 0;
            crc = (crc >> 1) ^ mask;
        }
    }
    return ~crc;
}

extern "C" {

int asterix_init(const char* config_path) {
    try {
        Tracer::Configure(debug_trace);

        if (!g_pDefinition) {
            g_pDefinition = std::make_unique<AsterixDefinition>();
        }

        if (!g_pInputParser) {
            g_pInputParser = std::make_unique<InputParser>(g_pDefinition.get());
        }

        if (config_path && strlen(config_path) > 0) {
            // Validate path length
            if (strlen(config_path) > ASTERIX_MAX_PATH_LENGTH) {
                g_lastError = "Configuration path too long";
                return ASTERIX_ERR_INVALID;
            }

            FILE* fp = fopen(config_path, "rt");
            if (!fp) {
                g_lastError = "Configuration file not found: " + std::string(config_path);
                return ASTERIX_ERR_INIT;
            }

            XMLParser parser;
            if (!parser.Parse(fp, g_pDefinition.get(), config_path)) {
                fclose(fp);
                g_lastError = "Failed to parse configuration file";
                return ASTERIX_ERR_PARSE;
            }
            fclose(fp);
        }

        g_bInitialized = true;
        return ASTERIX_OK;

    } catch (const std::bad_alloc&) {
        g_lastError = "Out of memory during initialization";
        return ASTERIX_ERR_MEMORY;
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception during initialization: ") + e.what();
        return ASTERIX_ERR_INIT;
    } catch (...) {
        g_lastError = "Unknown exception during initialization";
        return ASTERIX_ERR_INIT;
    }
}

int asterix_is_initialized(void) {
    return g_bInitialized ? 1 : 0;
}

AsterixParseResult* asterix_parse(const uint8_t* data, size_t length, int verbose) {
    return asterix_parse_with_offset(data, length, 0, 0, verbose);
}

AsterixParseResult* asterix_parse_with_offset(
    const uint8_t* data,
    size_t length,
    size_t offset,
    size_t max_blocks,
    int verbose
) {
    // Allocate result
    AsterixParseResult* result = static_cast<AsterixParseResult*>(
        calloc(1, sizeof(AsterixParseResult)));
    if (!result) {
        return nullptr;
    }

    // Validate state
    if (!g_bInitialized || !g_pInputParser) {
        result->error_code = ASTERIX_ERR_INIT;
        result->error_message = alloc_string("Parser not initialized");
        return result;
    }

    // Validate input
    if (!data || length == 0) {
        result->error_code = ASTERIX_ERR_INVALID;
        result->error_message = alloc_string("Invalid input data");
        return result;
    }

    if (length > ASTERIX_MAX_MESSAGE_SIZE) {
        result->error_code = ASTERIX_ERR_INVALID;
        result->error_message = alloc_string("Input data too large");
        return result;
    }

    if (offset >= length) {
        result->error_code = ASTERIX_ERR_INVALID;
        result->error_message = alloc_string("Offset exceeds data length");
        return result;
    }

    if (max_blocks == 0) {
        max_blocks = ASTERIX_MAX_BLOCKS;
    }

    try {
        uint64_t timestamp = get_timestamp_us();

        // Parse all blocks
        std::vector<AsterixRecord> records;
        size_t pos = offset;
        size_t block_count = 0;

        while (pos < length && block_count < max_blocks) {
            size_t remaining = length - pos;
            if (remaining < 3) break;  // Minimum ASTERIX header size

            const uint8_t* pData = data + pos;

            // Read ASTERIX header
            uint8_t category = pData[0];
            uint16_t block_length = (static_cast<uint16_t>(pData[1]) << 8) | pData[2];

            // Validate block length
            if (block_length < 3 || block_length > remaining) {
                break;  // Invalid block, stop parsing
            }

            // Parse the block using InputParser
            AsterixData* pAsterixData = g_pInputParser->parsePacket(pData, block_length, timestamp / 1000);

            if (pAsterixData) {
                // Get JSON representation using getText
                std::string jsonStr;
                unsigned int format = verbose ? FORMAT_JSON_VERBOSE : FORMAT_JSON;
                pAsterixData->getText(jsonStr, format);

                // Count records in this block
                for (auto* pBlock : pAsterixData->m_lDataBlocks) {
                    if (!pBlock) continue;

                    size_t numRecords = pBlock->m_lDataRecords.size();
                    for (size_t i = 0; i < numRecords; i++) {
                        AsterixRecord rec;
                        rec.category = category;
                        rec.length = block_length;
                        rec.timestamp_us = timestamp;
                        rec.crc = crc32(pData, block_length);
                        rec.json_data = alloc_string(jsonStr);  // Same JSON for all records in block
                        records.push_back(rec);
                    }
                }

                delete pAsterixData;
                block_count++;
            }

            pos += block_length;
        }

        // Copy records to result
        if (!records.empty()) {
            result->records = static_cast<AsterixRecord*>(
                calloc(records.size(), sizeof(AsterixRecord)));
            if (!result->records) {
                // Clean up any allocated json_data strings
                for (auto& rec : records) {
                    free(rec.json_data);
                }
                result->error_code = ASTERIX_ERR_MEMORY;
                result->error_message = alloc_string("Out of memory");
                return result;
            }

            memcpy(result->records, records.data(), records.size() * sizeof(AsterixRecord));
            result->count = records.size();
        }

        result->bytes_consumed = pos - offset;
        result->error_code = ASTERIX_OK;
        return result;

    } catch (const std::bad_alloc&) {
        result->error_code = ASTERIX_ERR_MEMORY;
        result->error_message = alloc_string("Out of memory during parsing");
        return result;
    } catch (const std::exception& e) {
        result->error_code = ASTERIX_ERR_PARSE;
        result->error_message = alloc_string(std::string("Parse exception: ") + e.what());
        return result;
    } catch (...) {
        result->error_code = ASTERIX_ERR_PARSE;
        result->error_message = alloc_string("Unknown exception during parsing");
        return result;
    }
}

char* asterix_describe(int category, const char* item, const char* field, const char* value) {
    if (!g_bInitialized || !g_pDefinition) {
        return nullptr;
    }

    if (category < 1 || category > 255) {
        return nullptr;
    }

    try {
        Category* pCat = g_pDefinition->getCategory(category);
        if (!pCat) {
            return nullptr;
        }

        std::string description;

        if (!item || strlen(item) == 0) {
            // Return category description
            description = pCat->m_strName;
        } else {
            // Find item
            DataItemDescription* pItem = pCat->getDataItemDescription(item);
            if (!pItem) {
                return nullptr;
            }

            if (!field || strlen(field) == 0) {
                // Return item description
                description = pItem->m_strName;
            } else {
                // Find field description (simplified - full implementation would search format)
                description = pItem->m_strName + " - " + std::string(field);
            }
        }

        return alloc_string(description);

    } catch (...) {
        return nullptr;
    }
}

int asterix_is_category_defined(int category) {
    if (!g_bInitialized || !g_pDefinition) {
        return 0;
    }

    if (category < 1 || category > 255) {
        return 0;
    }

    Category* pCat = g_pDefinition->getCategory(category);
    return pCat ? 1 : 0;
}

void asterix_free_result(AsterixParseResult* result) {
    if (!result) return;

    if (result->records) {
        for (size_t i = 0; i < result->count; i++) {
            free(result->records[i].json_data);
        }
        free(result->records);
    }

    free(result->error_message);
    free(result);
}

void asterix_free_string(char* str) {
    free(str);
}

const char* asterix_version(void) {
    return g_version;
}

const char* asterix_get_last_error(void) {
    return g_lastError.empty() ? nullptr : g_lastError.c_str();
}

} // extern "C"
