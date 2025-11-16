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
 *
 * AUTHORS: Damir Salantic, Croatia Control Ltd.
 *
 */

#ifndef INPUTPARSER_H_
#define INPUTPARSER_H_

#include "AsterixDefinition.h"
#include "AsterixData.h"
#include "DataBlock.h"
#include <ios>
#include <iostream>
#include <iomanip>
#include <sstream>

/**
 * @class InputParser
 * @brief Main ASTERIX binary data parser
 *
 * InputParser is the primary entry point for parsing raw ASTERIX binary data.
 * It processes packets containing one or more ASTERIX data blocks, validates
 * data integrity, and produces structured AsterixData results.
 *
 * ## Parsing Pipeline
 *
 * ```
 * Raw Binary → parsePacket() → [DataBlock 1, DataBlock 2, ...] → AsterixData
 * ```
 *
 * Each ASTERIX data block has the structure:
 * ```
 * +----------+----------+----------+
 * | Category | Length   | Records  |
 * | (1 byte) | (2 bytes)| (N bytes)|
 * +----------+----------+----------+
 * ```
 *
 * ## Key Features
 *
 * - **Multi-block Parsing**: Handles packets with multiple concatenated data blocks
 * - **Security Validation**: Checks buffer boundaries and data length validity (VULN-004 fix)
 * - **Timestamp Support**: Associates timestamps with parsed data blocks
 * - **Format Filtering**: Allows selective filtering of data items
 * - **Category Detection**: Automatically identifies ASTERIX category from first byte
 *
 * ## Usage Example
 *
 * ```cpp
 * AsterixDefinition* def = AsterixDefinition::getInstance();
 * InputParser parser(def);
 *
 * // Parse raw ASTERIX data
 * const unsigned char* buffer = ...; // Raw binary ASTERIX data
 * unsigned int size = ...;
 * double timestamp = 1234567890.123;
 *
 * AsterixData* data = parser.parsePacket(buffer, size, timestamp);
 * // Process data...
 * delete data;
 * ```
 *
 * ## Thread Safety
 *
 * InputParser is **NOT thread-safe** due to shared AsterixDefinition singleton.
 * Create separate parser instances per thread or use external synchronization.
 *
 * @note The parser does NOT own the AsterixDefinition - it's a singleton
 * @note Caller must delete the returned AsterixData object
 * @note All ASTERIX categories must be pre-loaded into AsterixDefinition
 *
 * @see AsterixDefinition
 * @see AsterixData
 * @see DataBlock
 */
class InputParser {
public:
    /**
     * @brief Construct a new ASTERIX parser
     * @param pDefinition Pointer to AsterixDefinition singleton with loaded categories
     * @note The parser does NOT take ownership of pDefinition (shared singleton)
     */
    InputParser(AsterixDefinition *pDefinition);

    /**
     * @brief Destructor - does NOT delete AsterixDefinition (singleton)
     * @note Fixed to avoid double-free when multiple parsers exist
     */
    // FIXED: Don't delete m_pDefinition - it's a singleton shared across all parsers
    // Deleting it causes double-free when multiple InputParser instances exist
    ~InputParser() { /* m_pDefinition is owned by AsterixDefinition singleton */ }

    /**
     * @brief Parse a complete ASTERIX packet (one or more data blocks)
     * @param m_pBuffer Pointer to raw ASTERIX binary data
     * @param m_nBufferSize Size of buffer in bytes
     * @param nTimestamp Optional timestamp to associate with parsed blocks (default: 0.0)
     * @return Pointer to new AsterixData containing all parsed blocks (caller must delete)
     * @note Returns partial data if parsing encounters errors (stops at first error)
     * @note Validates buffer boundaries and data block lengths (security hardened)
     * @note Returns empty AsterixData if buffer too small (<3 bytes per block)
     */
    AsterixData *parsePacket(const unsigned char *m_pBuffer, unsigned int m_nBufferSize, double nTimestamp = 0.0);

    /**
     * @brief Parse a single ASTERIX data block from buffer (internal helper)
     * @param m_pData Pointer to buffer containing ASTERIX data
     * @param m_nPos Current position in buffer (updated on success)
     * @param m_nBufferSize Total buffer size in bytes
     * @param nTimestamp Timestamp to associate with this block
     * @param m_nDataLength Remaining bytes to parse (updated on success)
     * @return Pointer to new DataBlock, or nullptr on error
     * @note This is an internal method typically called by parsePacket()
     * @note Validates category byte and data block length before parsing
     */
    DataBlock *parse_next_data_block(const unsigned char *m_pData, unsigned int &m_nPos, unsigned int m_nBufferSize,
                                     double nTimestamp, unsigned int &m_nDataLength);

    /**
     * @brief Generate human-readable description of all loaded ASTERIX categories
     * @return String containing category definitions and data item formats
     * @note Useful for debugging and introspection
     */
    std::string printDefinition();

    /**
     * @brief Mark a data item for filtering (exclusion from output)
     * @param cat ASTERIX category number (e.g., 48, 62)
     * @param item Data item ID (e.g., "010", "220")
     * @param name Field name within item to filter
     * @return true if filter applied successfully, false if item/field not found
     */
    bool filterOutItem(int cat, std::string item, const char *name);

    /**
     * @brief Check if a data item field is currently filtered
     * @param cat ASTERIX category number
     * @param item Data item ID
     * @param name Field name to check
     * @return true if field is filtered (hidden), false otherwise
     */
    bool isFiltered(int cat, std::string item, const char *name);

private:
    AsterixDefinition *m_pDefinition; //!< Pointer to singleton ASTERIX category definitions

};

#endif /* INPUTPARSER_H_ */
