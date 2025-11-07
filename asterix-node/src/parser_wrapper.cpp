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

#ifdef _WIN32
  #include "../engine/win32_compat.h"
#endif

#include "parser_wrapper.h"
#include "AsterixDefinition.h"
#include "XMLParser.h"
#include "InputParser.h"
#include "AsterixData.h"
#include "DataBlock.h"
#include "DataRecord.h"
#include "Tracer.h"
#include "asterixformat.hxx"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <sstream>

#ifdef _WIN32
  #include <time.h>
  #include <direct.h>  // for _getcwd, _chdir
  #define getcwd _getcwd
  #define chdir _chdir
#else
  #include <sys/time.h>
  #include <unistd.h>  // for getcwd, chdir
#endif

// Global state (singleton pattern, matches Python implementation)
static AsterixDefinition *pDefinition = NULL;
static InputParser *inputParser = NULL;

// Error callback for Tracer (captures errors during parsing)
static char last_error_buffer[512] = {0};

static void error_trace_callback(char const *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(last_error_buffer, sizeof(last_error_buffer) - 1, format, args);
    va_end(args);
}

// Helper: Get current timestamp in milliseconds
static unsigned long get_timestamp_ms() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

// Helper: Convert AsterixData to AsterixRecords struct
static bool convert_asterix_data_to_records(
    AsterixData* pData,
    bool verbose,
    AsterixRecords** out_records
) {
    if (!pData || !out_records) {
        return false;
    }

    size_t total_records = 0;

    // Count total records across all data blocks
    for (auto it = pData->m_lDataBlocks.begin(); it != pData->m_lDataBlocks.end(); ++it) {
        DataBlock* db = *it;
        if (db) {
            total_records += db->m_lDataRecords.size();
        }
    }

    if (total_records == 0) {
        // Allocate empty records structure
        *out_records = (AsterixRecords*)calloc(1, sizeof(AsterixRecords));
        if (!*out_records) {
            return false;
        }
        (*out_records)->records = NULL;
        (*out_records)->count = 0;
        return true;
    }

    // Allocate AsterixRecords structure
    AsterixRecords* records = (AsterixRecords*)calloc(1, sizeof(AsterixRecords));
    if (!records) {
        return false;
    }

    // Allocate array of AsterixRecord
    records->records = (AsterixRecord*)calloc(total_records, sizeof(AsterixRecord));
    if (!records->records) {
        free(records);
        return false;
    }
    records->count = total_records;

    // Fill in the records
    size_t record_idx = 0;
    for (auto it = pData->m_lDataBlocks.begin(); it != pData->m_lDataBlocks.end(); ++it) {
        DataBlock* db = *it;
        if (!db) continue;

        for (auto rit = db->m_lDataRecords.begin(); rit != db->m_lDataRecords.end(); ++rit) {
            DataRecord* dr = *rit;
            if (!dr || record_idx >= total_records) continue;

            AsterixRecord* rec = &records->records[record_idx++];

            // Fill basic fields
            rec->category = (uint8_t)(dr->m_pCategory ? dr->m_pCategory->m_id : 0);
            rec->length = (uint32_t)dr->m_nLength;
            rec->timestamp_ms = (uint64_t)(dr->m_nTimestamp * 1000.0); // Convert seconds to milliseconds
            rec->crc = dr->m_nCrc;

            // Copy hex data
            if (dr->m_pHexData) {
                rec->hex_data = strdup(dr->m_pHexData);
            } else {
                rec->hex_data = strdup("");
            }

            // Generate JSON representation using getText
            std::string json_str;
            std::string header = "";
            bool success = dr->getText(json_str, header, CAsterixFormat::EJSON);

            if (success && !json_str.empty()) {
                rec->json_data = strdup(json_str.c_str());
            } else {
                rec->json_data = strdup("{}");
            }

            // Check allocation failures
            if (!rec->hex_data || !rec->json_data) {
                // Cleanup and return error
                asterix_wrapper_free_records(records);
                return false;
            }
        }
    }

    *out_records = records;
    return true;
}

// Initialize ASTERIX parser with config directory
bool asterix_wrapper_init(const char* config_dir) {
    try {
        // Clear last error
        last_error_buffer[0] = '\0';

        // Configure tracer to capture errors
        Tracer::Configure(error_trace_callback);

        // Create singleton instances if not already created
        if (!pDefinition) {
            pDefinition = new AsterixDefinition();
        }

        if (!inputParser) {
            inputParser = new InputParser(pDefinition);
        }

        // Determine config directory path
        std::string config_path;
        if (config_dir && config_dir[0] != '\0') {
            config_path = config_dir;
        } else {
            // Default to asterix/config/ relative to working directory
            config_path = "asterix/config";
        }

        // Ensure path ends with separator
        if (config_path.back() != '/' && config_path.back() != '\\') {
            config_path += "/";
        }

        // Construct path to asterix.ini
        std::string ini_path = config_path + "asterix.ini";

        // Open asterix.ini file
        FILE *fpini = fopen(ini_path.c_str(), "rt");
        if (!fpini) {
            snprintf(last_error_buffer, sizeof(last_error_buffer),
                    "Failed to open asterix.ini at: %s", ini_path.c_str());
            return false;
        }

        // Save current working directory and change to config directory
        // This is needed because XML files reference asterix.dtd as a relative path
        char old_cwd[4096];
        if (!getcwd(old_cwd, sizeof(old_cwd))) {
            fclose(fpini);
            snprintf(last_error_buffer, sizeof(last_error_buffer),
                    "Failed to get current working directory");
            return false;
        }

        if (chdir(config_path.c_str()) != 0) {
            fclose(fpini);
            snprintf(last_error_buffer, sizeof(last_error_buffer),
                    "Failed to change to config directory: %s", config_path.c_str());
            return false;
        }

        // Read and parse each XML file listed in asterix.ini
        char xml_file[256];
        bool parse_success = true;

        while (fgets(xml_file, sizeof(xml_file), fpini)) {
            // Remove trailing whitespace/newline
            size_t len = strlen(xml_file);
            while (len > 0 && (xml_file[len-1] == '\n' || xml_file[len-1] == '\r' ||
                               xml_file[len-1] == ' ' || xml_file[len-1] == '\t')) {
                xml_file[--len] = '\0';
            }

            // Skip empty lines and comments
            if (len == 0 || xml_file[0] == '#') {
                continue;
            }

            // Create fresh parser for each XML file (prevents state accumulation)
            XMLParser Parser;

            // Open with just filename (since we're in config directory)
            // But pass full path to Parser for proper error reporting
            std::string xml_full_path = config_path + xml_file;

            FILE *fp = fopen(xml_file, "rt");
            if (!fp) {
                snprintf(last_error_buffer, sizeof(last_error_buffer),
                        "Failed to open XML file: %s", xml_file);
                parse_success = false;
                break;
            }

            if (!Parser.Parse(fp, pDefinition, xml_full_path.c_str())) {
                fclose(fp);
                snprintf(last_error_buffer, sizeof(last_error_buffer),
                        "Failed to parse XML file: %s", xml_file);
                parse_success = false;
                break;
            }

            fclose(fp);
        }

        fclose(fpini);

        // Restore original working directory
        if (chdir(old_cwd) != 0) {
            snprintf(last_error_buffer, sizeof(last_error_buffer),
                    "Warning: Failed to restore working directory to: %s", old_cwd);
            // Continue anyway - parser is initialized
        }

        return parse_success;

    } catch (const std::bad_alloc& e) {
        snprintf(last_error_buffer, sizeof(last_error_buffer),
                "Out of memory during initialization");
        return false;
    } catch (const std::exception& e) {
        snprintf(last_error_buffer, sizeof(last_error_buffer),
                "C++ exception during initialization: %s", e.what());
        return false;
    } catch (...) {
        snprintf(last_error_buffer, sizeof(last_error_buffer),
                "Unknown C++ exception during initialization");
        return false;
    }
}

// Load a specific ASTERIX category definition file
bool asterix_wrapper_load_category(const char* xml_path) {
    try {
        // SAFETY: Null pointer check
        if (!xml_path) {
            snprintf(last_error_buffer, sizeof(last_error_buffer),
                    "NULL xml_path provided");
            return false;
        }

        // Clear last error
        last_error_buffer[0] = '\0';

        // Ensure parser is initialized
        if (!pDefinition) {
            pDefinition = new AsterixDefinition();
        }

        // Open XML file
        FILE *fp = fopen(xml_path, "rt");
        if (!fp) {
            snprintf(last_error_buffer, sizeof(last_error_buffer),
                    "Failed to open XML file: %s", xml_path);
            return false;
        }

        // Parse XML
        XMLParser Parser;
        bool success = Parser.Parse(fp, pDefinition, xml_path);
        fclose(fp);

        if (!success) {
            snprintf(last_error_buffer, sizeof(last_error_buffer),
                    "Failed to parse XML file: %s", xml_path);
            return false;
        }

        return true;

    } catch (const std::bad_alloc& e) {
        snprintf(last_error_buffer, sizeof(last_error_buffer),
                "Out of memory while loading category");
        return false;
    } catch (const std::exception& e) {
        snprintf(last_error_buffer, sizeof(last_error_buffer),
                "C++ exception while loading category: %s", e.what());
        return false;
    } catch (...) {
        snprintf(last_error_buffer, sizeof(last_error_buffer),
                "Unknown C++ exception while loading category");
        return false;
    }
}

// Parse ASTERIX data from buffer
int asterix_wrapper_parse(
    const uint8_t* data,
    size_t length,
    bool verbose,
    AsterixRecords** out_records,
    char* error_buffer,
    size_t error_buffer_size
) {
    try {
        // SAFETY: Input validation
        if (!data) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "NULL data buffer provided");
            }
            return -1;
        }

        if (!out_records) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "NULL out_records pointer provided");
            }
            return -2;
        }

        if (length == 0) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "Zero length data");
            }
            return -3;
        }

        // SAFETY: Check for integer overflow
        if (length > (size_t)0x7FFFFFFF) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "Data length exceeds maximum");
            }
            return -4;
        }

        // Ensure parser is initialized
        if (!inputParser) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "Parser not initialized. Call asterix_wrapper_init() first.");
            }
            return -5;
        }

        // Clear last error and error buffer
        last_error_buffer[0] = '\0';
        if (error_buffer && error_buffer_size > 0) {
            error_buffer[0] = '\0';
        }

        // Get timestamp
        unsigned long timestamp = get_timestamp_ms();

        // Parse the packet
        AsterixData *pData = inputParser->parsePacket(data, (unsigned int)length, timestamp);

        if (!pData) {
            if (error_buffer && error_buffer_size > 0) {
                if (last_error_buffer[0] != '\0') {
                    snprintf(error_buffer, error_buffer_size, "%s", last_error_buffer);
                } else {
                    snprintf(error_buffer, error_buffer_size, "Failed to parse ASTERIX data");
                }
            }
            return -6;
        }

        // Convert AsterixData to AsterixRecords
        bool success = convert_asterix_data_to_records(pData, verbose, out_records);

        // Clean up
        delete pData;

        if (!success) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "Failed to convert parsed data to records");
            }
            return -7;
        }

        return 0; // Success

    } catch (const std::bad_alloc& e) {
        if (error_buffer && error_buffer_size > 0) {
            snprintf(error_buffer, error_buffer_size, "Out of memory during parsing");
        }
        return -8;
    } catch (const std::exception& e) {
        if (error_buffer && error_buffer_size > 0) {
            snprintf(error_buffer, error_buffer_size, "C++ exception during parsing: %s", e.what());
        }
        return -9;
    } catch (...) {
        if (error_buffer && error_buffer_size > 0) {
            snprintf(error_buffer, error_buffer_size, "Unknown C++ exception during parsing");
        }
        return -10;
    }
}

// Parse ASTERIX data with offset and block count
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
) {
    try {
        // SAFETY: Input validation
        if (!data) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "NULL data buffer provided");
            }
            return -1;
        }

        if (!out_records) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "NULL out_records pointer provided");
            }
            return -2;
        }

        // SAFETY: Bounds checking
        if (offset >= length) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "Offset (%u) >= length (%zu)", offset, length);
            }
            // Return empty result for out-of-bounds
            *out_records = (AsterixRecords*)calloc(1, sizeof(AsterixRecords));
            if (out_bytes_consumed) *out_bytes_consumed = 0;
            if (out_remaining_blocks) *out_remaining_blocks = 0;
            return 0;
        }

        // SAFETY: Check for integer overflow
        if ((size_t)offset + blocks_count > (size_t)0x7FFFFFFF) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "Offset + blocks_count would overflow");
            }
            return -3;
        }

        // Ensure parser is initialized
        if (!inputParser) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "Parser not initialized. Call asterix_wrapper_init() first.");
            }
            return -4;
        }

        // Clear last error and error buffer
        last_error_buffer[0] = '\0';
        if (error_buffer && error_buffer_size > 0) {
            error_buffer[0] = '\0';
        }

        // Get timestamp
        unsigned long timestamp = get_timestamp_ms();

        // Create AsterixData container
        AsterixData *pData = new AsterixData();
        unsigned int pos = offset;
        unsigned int current_blocks_count = 0;

        // Parse data blocks incrementally
        while (pos < length && (blocks_count == 0 || current_blocks_count < blocks_count)) {
            // SAFETY: Explicit bounds check before subtraction
            if (pos >= length) {
                break;
            }

            unsigned int data_length = length - pos;

            // Ensure we have minimum data to parse (at least 3 bytes for ASTERIX header: CAT + LEN)
            if (data_length <= 3) {
                break;
            }

            const unsigned char *pBuf_offset = data + pos;
            DataBlock *block = inputParser->parse_next_data_block(
                    pBuf_offset, pos, length, timestamp, data_length);

            if (block) {
                pData->m_lDataBlocks.push_back(block);
                current_blocks_count++;
            } else {
                // No more blocks could be parsed
                break;
            }
        }

        // Convert AsterixData to AsterixRecords
        bool success = convert_asterix_data_to_records(pData, verbose, out_records);

        // Set output parameters
        if (out_bytes_consumed) {
            *out_bytes_consumed = (size_t)(pos - offset);
        }

        if (out_remaining_blocks) {
            // Estimate remaining blocks (this is approximate)
            size_t remaining_bytes = (pos < length) ? (length - pos) : 0;
            // Assume average block size of 64 bytes (rough estimate)
            *out_remaining_blocks = (remaining_bytes > 0) ? (remaining_bytes / 64) : 0;
        }

        // Clean up
        delete pData;

        if (!success) {
            if (error_buffer && error_buffer_size > 0) {
                snprintf(error_buffer, error_buffer_size, "Failed to convert parsed data to records");
            }
            return -5;
        }

        return 0; // Success

    } catch (const std::bad_alloc& e) {
        if (error_buffer && error_buffer_size > 0) {
            snprintf(error_buffer, error_buffer_size, "Out of memory during parsing with offset");
        }
        return -6;
    } catch (const std::exception& e) {
        if (error_buffer && error_buffer_size > 0) {
            snprintf(error_buffer, error_buffer_size, "C++ exception during parsing with offset: %s", e.what());
        }
        return -7;
    } catch (...) {
        if (error_buffer && error_buffer_size > 0) {
            snprintf(error_buffer, error_buffer_size, "Unknown C++ exception during parsing with offset");
        }
        return -8;
    }
}

// Get description for category/item/field/value
char* asterix_wrapper_describe(
    uint8_t category,
    const char* item,
    const char* field,
    const char* value
) {
    try {
        if (!pDefinition) {
            return strdup("Not initialized");
        }

        const char *description = pDefinition->getDescription(
            (int)category, item, field, value);

        if (description == NULL) {
            return strdup("");
        }

        return strdup(description);

    } catch (const std::exception& e) {
        return strdup("Error getting description");
    } catch (...) {
        return strdup("Unknown error");
    }
}

// Check if a category is defined
bool asterix_wrapper_is_category_defined(uint8_t category) {
    try {
        if (!pDefinition) {
            return false;
        }

        return pDefinition->CategoryDefined((int)category);

    } catch (...) {
        return false;
    }
}

// Free records allocated by asterix_wrapper_parse*
void asterix_wrapper_free_records(AsterixRecords* records) {
    if (!records) {
        return;
    }

    // Free each record's allocated strings
    if (records->records) {
        for (size_t i = 0; i < records->count; i++) {
            AsterixRecord* rec = &records->records[i];

            if (rec->hex_data) {
                free(rec->hex_data);
                rec->hex_data = NULL;
            }

            if (rec->json_data) {
                free(rec->json_data);
                rec->json_data = NULL;
            }
        }

        // Free the records array
        free(records->records);
        records->records = NULL;
    }

    // Free the AsterixRecords structure itself
    free(records);
}

// Free string allocated by asterix_wrapper_describe
void asterix_wrapper_free_string(char* str) {
    if (str) {
        free(str);
    }
}

// Get last error message
const char* asterix_wrapper_get_last_error() {
    return last_error_buffer;
}
