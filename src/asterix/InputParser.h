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

/**
 * @file InputParser.h
 * @brief ASTERIX binary data parser using category definitions
 *
 * This file defines the InputParser class which provides the main parsing
 * interface for ASTERIX binary data. InputParser uses category definitions
 * loaded from XML files (via XMLParser and AsterixDefinition) to decode
 * ASTERIX data blocks and records into structured data.
 *
 * @par Initialization
 * Category definitions are loaded at program startup from asterix.ini:
 * - asterix.ini lists all XML category definition files (one per line)
 * - Each XML file is parsed by XMLParser into a Category object
 * - Categories are registered in an AsterixDefinition singleton
 * - InputParser holds a reference to this AsterixDefinition
 *
 * @par Configuration File Format (asterix.ini)
 * The asterix.ini file contains a simple list of XML filenames, one per line:
 * @code
 * asterix_bds.xml
 * asterix_cat001_1_4.xml
 * asterix_cat002_1_1.xml
 * asterix_cat048_1_30.xml
 * asterix_cat062_1_19.xml
 * ...
 * @endcode
 *
 * @par Parsing Process
 * 1. Binary ASTERIX data is received (from file, network, or stdin)
 * 2. InputParser::parsePacket() is called with the raw buffer
 * 3. Parser extracts ASTERIX data blocks (Category + Length + Data)
 * 4. For each data block:
 *    - Category number determines which Category definition to use
 *    - DataBlock constructor parses records using the Category's UAP
 *    - Each DataRecord parses data items using DataItemFormat parsers
 * 5. Parsed data is returned as AsterixData (contains list of DataBlocks)
 *
 * @see AsterixDefinition For global category registry
 * @see XMLParser For loading XML category definitions
 * @see Category For individual category structure
 * @see DataBlock For ASTERIX data block representation
 * @see AsterixData For parsed output container
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
 * InputParser is responsible for parsing ASTERIX binary data into structured
 * representations. It uses category definitions (loaded from XML files via
 * AsterixDefinition) to decode ASTERIX data blocks and records.
 *
 * @par Role in ASTERIX System
 * InputParser sits at the core of the ASTERIX decoding pipeline:
 * - **Input**: Raw binary ASTERIX data (from files, network, stdin)
 * - **Processing**: Extracts data blocks, validates structure, parses records
 * - **Output**: AsterixData containing list of parsed DataBlocks
 *
 * @par Category Definitions
 * InputParser requires an initialized AsterixDefinition to function:
 * @code
 * // 1. Create and populate AsterixDefinition from asterix.ini
 * AsterixDefinition* globalDef = new AsterixDefinition();
 * FILE* fpini = fopen("config/asterix.ini", "rt");
 * char line[256];
 * while (fgets(line, sizeof(line), fpini)) {
 *     // Remove trailing newline
 *     line[strcspn(line, "\r\n")] = 0;
 *     if (strlen(line) == 0) continue;
 *
 *     FILE* fp = fopen(line, "rt");
 *     XMLParser parser;
 *     parser.Parse(fp, globalDef, line);
 *     fclose(fp);
 * }
 * fclose(fpini);
 *
 * // 2. Create parser with loaded definitions
 * InputParser parser(globalDef);
 *
 * // 3. Parse ASTERIX data
 * AsterixData* result = parser.parsePacket(buffer, bufferSize);
 * @endcode
 *
 * @par Typical Usage - File Parsing
 * @code
 * // Assume globalDef is initialized as shown above
 * InputParser parser(globalDef);
 *
 * // Read ASTERIX binary file
 * FILE* fp = fopen("sample.asterix", "rb");
 * unsigned char buffer[65536];
 * size_t bytesRead = fread(buffer, 1, sizeof(buffer), fp);
 * fclose(fp);
 *
 * // Parse the data
 * AsterixData* asterixData = parser.parsePacket(buffer, bytesRead);
 *
 * // Access parsed data blocks
 * for (auto* dataBlock : asterixData->m_lDataBlocks) {
 *     std::cout << "Category: " << dataBlock->m_nCategory << std::endl;
 *     // Process data records...
 * }
 *
 * delete asterixData;
 * @endcode
 *
 * @par Incremental Parsing
 * For large data streams, use parse_next_data_block() to parse one block at a time:
 * @code
 * unsigned int pos = 0;
 * unsigned int dataLength = totalSize;
 * while (pos < totalSize) {
 *     if (dataLength <= 3) break; // Need at least 3 bytes for header
 *
 *     DataBlock* block = parser.parse_next_data_block(
 *         buffer, pos, totalSize, 0.0, dataLength
 *     );
 *
 *     if (block && block->m_bFormatOK) {
 *         // Process block
 *         delete block;
 *     } else {
 *         delete block;
 *         break; // Error occurred
 *     }
 * }
 * @endcode
 *
 * @par ASTERIX Data Format
 * Each ASTERIX data block has this structure:
 * - Byte 0: Category number (e.g., 48, 62)
 * - Bytes 1-2: Block length in bytes (big-endian, includes 3-byte header)
 * - Bytes 3+: Data records (parsed according to category UAP)
 *
 * @par Error Handling
 * The parser includes security fixes for validation:
 * - Data length must be > 3 bytes (header size)
 * - Data length must not exceed available buffer
 * - DataBlock parsing failures stop processing immediately
 * - Invalid blocks are deleted and not added to results
 *
 * Errors are logged via Tracer::Error() and parsing stops on first error
 * to prevent processing corrupted data.
 *
 * @par Thread Safety
 * **WARNING: InputParser is NOT thread-safe.**
 *
 * - Multiple threads must NOT share the same InputParser instance
 * - The underlying AsterixDefinition singleton is also not thread-safe
 * - For multi-threaded parsing, use separate InputParser instances per thread
 *   with mutex protection around the shared AsterixDefinition
 *
 * @par Memory Management
 * - InputParser does NOT own the AsterixDefinition pointer
 * - Caller must ensure AsterixDefinition outlives all InputParser instances
 * - parsePacket() returns a new AsterixData* - caller must delete
 * - parse_next_data_block() returns a new DataBlock* - caller must delete
 *
 * @see AsterixDefinition For category registry and initialization
 * @see XMLParser For loading XML category definitions
 * @see DataBlock For individual data block representation
 * @see AsterixData For complete parsed data container
 * @see Category For category-specific parsing rules
 */
class InputParser {
public:
    /**
     * @brief Construct an InputParser with category definitions
     *
     * @param pDefinition Pointer to initialized AsterixDefinition containing
     *                    all loaded category definitions. Must not be null.
     *                    InputParser does NOT take ownership - caller must
     *                    ensure pDefinition outlives this InputParser instance.
     *
     * @note The AsterixDefinition must be fully initialized (all required
     *       categories loaded via XMLParser) before creating an InputParser.
     *
     * @par Example
     * @code
     * AsterixDefinition* globalDef = new AsterixDefinition();
     * // ... load categories via XMLParser ...
     * InputParser parser(globalDef);
     * @endcode
     */
    InputParser(AsterixDefinition *pDefinition);

    /**
     * @brief Destructor
     *
     * Cleans up the InputParser. Does NOT delete m_pDefinition as it is
     * owned by the caller (typically a global singleton shared across all
     * parser instances).
     *
     * @note This is intentionally empty - the AsterixDefinition singleton
     *       should be deleted separately when the program exits.
     */
    ~InputParser() { /* m_pDefinition is owned by AsterixDefinition singleton */ }

    /**
     * @brief Parse a complete ASTERIX packet (buffer)
     *
     * Parses all ASTERIX data blocks from a binary buffer. Each data block
     * consists of a 3-byte header (Category + Length) followed by data records.
     * The parser continues until the entire buffer is consumed or an error occurs.
     *
     * @param m_pBuffer Pointer to raw ASTERIX binary data buffer
     * @param m_nBufferSize Size of the buffer in bytes
     * @param nTimestamp Optional timestamp to associate with parsed data (default: 0.0).
     *                   Used for synchronizing playback or logging.
     *
     * @return Pointer to new AsterixData object containing all successfully
     *         parsed DataBlocks. Returns non-null even if parsing fails
     *         (may contain partial results). **Caller must delete** the
     *         returned pointer.
     *
     * @par ASTERIX Packet Structure
     * A packet may contain multiple concatenated data blocks:
     * @code
     * [Cat|Len|Data][Cat|Len|Data]...
     *  \-----------/ \-----------/
     *   Data Block 1  Data Block 2
     * @endcode
     *
     * @par Error Handling
     * - Logs errors via Tracer::Error() for invalid data
     * - Stops parsing on first error (VULN-004 security fix)
     * - Returns partial results (blocks parsed before error)
     * - Empty m_lDataBlocks list if no valid blocks found
     *
     * @par Example - Parse PCAP-extracted ASTERIX data
     * @code
     * // Assume buffer contains ASTERIX data extracted from PCAP
     * AsterixData* result = parser.parsePacket(buffer, bufferSize, 123.456);
     *
     * std::cout << "Parsed " << result->m_lDataBlocks.size()
     *           << " data blocks" << std::endl;
     *
     * for (auto* block : result->m_lDataBlocks) {
     *     std::cout << "Category " << (int)block->m_nCategory
     *               << ", " << block->m_lDataRecords.size()
     *               << " records" << std::endl;
     * }
     *
     * delete result; // Caller must free
     * @endcode
     *
     * @see parse_next_data_block() For incremental parsing (one block at a time)
     * @see AsterixData For the returned data structure
     * @see DataBlock For individual block representation
     */
    AsterixData *parsePacket(const unsigned char *m_pBuffer, unsigned int m_nBufferSize, double nTimestamp = 0.0);

    /**
     * @brief Parse the next single ASTERIX data block from a buffer
     *
     * Parses one ASTERIX data block at the current position and advances
     * the position pointer. Useful for incremental parsing of large streams
     * or when you need fine-grained control over parsing.
     *
     * @param m_pData Pointer to raw ASTERIX binary data buffer (current position)
     * @param m_nPos [in/out] Current position in buffer. Updated to point
     *               past the parsed data block.
     * @param m_nBufferSize Total buffer size (used for bounds checking)
     * @param nTimestamp Optional timestamp to associate with the parsed block
     * @param m_nDataLength [in/out] Remaining data length. Updated after parsing
     *                      to reflect bytes consumed.
     *
     * @return Pointer to new DataBlock object if parsing succeeds, or
     *         NULL if parsing fails (invalid data, insufficient bytes, etc.).
     *         **Caller must delete** the returned pointer.
     *
     * @par Difference from parsePacket()
     * - parsePacket() parses ALL blocks in a buffer (loop)
     * - parse_next_data_block() parses ONE block (single iteration)
     * - parse_next_data_block() gives caller control over iteration
     *
     * @par Error Handling
     * - Returns NULL if data length < 3 (insufficient for header)
     * - Returns NULL if block length exceeds available data
     * - Logs errors via Tracer::Error()
     * - Position and length are NOT updated if parsing fails
     *
     * @par Example - Incremental Parsing
     * @code
     * unsigned int pos = 0;
     * unsigned int dataLen = totalSize;
     *
     * while (pos < totalSize && dataLen > 3) {
     *     DataBlock* block = parser.parse_next_data_block(
     *         buffer, pos, totalSize, 0.0, dataLen
     *     );
     *
     *     if (!block) {
     *         std::cerr << "Parsing failed at position " << pos << std::endl;
     *         break;
     *     }
     *
     *     // Process block
     *     std::cout << "Category: " << (int)block->m_nCategory << std::endl;
     *     delete block;
     * }
     * @endcode
     *
     * @note Originally contributed by Krzysztof Rutkowski (ICM UW, krutk@icm.edu.pl)
     *
     * @see parsePacket() For parsing all blocks in one call
     * @see DataBlock For the returned data structure
     */
    DataBlock *parse_next_data_block(const unsigned char *m_pData, unsigned int &m_nPos, unsigned int m_nBufferSize,
                                     double nTimestamp, unsigned int &m_nDataLength);

    /**
     * @brief Get a printable description of all loaded category definitions
     *
     * Generates a human-readable string listing all loaded ASTERIX categories,
     * including their IDs, names, and versions. Useful for debugging and
     * diagnostics to verify which categories are available for parsing.
     *
     * @return String containing formatted category information
     *
     * @par Example
     * @code
     * std::cout << parser.printDefinition() << std::endl;
     * // Output:
     * // Category 48: Monoradar Target Reports v1.30
     * // Category 62: System Track Data v1.19
     * // ...
     * @endcode
     *
     * @note Delegates to AsterixDefinition::printDescriptors()
     *
     * @see AsterixDefinition::printDescriptors()
     */
    std::string printDefinition();

    /**
     * @brief Mark a specific data item/field for filtering (exclusion from output)
     *
     * Adds a filter to exclude a specific ASTERIX data item or field from
     * output. Used by the CLI filtering mechanism to show only specific
     * data items of interest.
     *
     * @param cat Category number (0-255)
     * @param item Data item ID (e.g., "010", "080")
     * @param name Field name within the item (e.g., "SAC", "TYP"),
     *             or nullptr to filter entire item
     *
     * @return true if filtering was applied successfully, false if category
     *         is not loaded
     *
     * @par Example - Filter out all but I062/010 (Data Source Identifier)
     * @code
     * // Filter out everything first
     * parser.filterOutItem(62, "", nullptr);
     *
     * // Then explicitly include I062/010
     * // (actual filter inversion depends on Category implementation)
     * @endcode
     *
     * @note Delegates to AsterixDefinition::filterOutItem() which delegates
     *       to Category::filterOutItem()
     *
     * @see isFiltered() To check if an item passes the filter
     * @see AsterixDefinition::filterOutItem()
     */
    bool filterOutItem(int cat, std::string item, const char *name);

    /**
     * @brief Check if a specific data item/field should be displayed (passes filter)
     *
     * Tests whether a data item or field passes the current filter settings.
     * Items that pass the filter should be included in output.
     *
     * @param cat Category number (0-255)
     * @param item Data item ID (e.g., "010", "080")
     * @param name Field name within the item (e.g., "SAC", "TYP"),
     *             or nullptr to check entire item
     *
     * @return true if the item should be displayed (passes filter),
     *         false if filtered out
     *
     * @par Example - Check before printing
     * @code
     * if (parser.isFiltered(62, "010", "SAC")) {
     *     std::cout << "SAC: " << sacValue << std::endl;
     * }
     * @endcode
     *
     * @note Delegates to AsterixDefinition::isFiltered() which delegates
     *       to Category::isFiltered()
     *
     * @see filterOutItem() To set up filters
     * @see AsterixDefinition::isFiltered()
     */
    bool isFiltered(int cat, std::string item, const char *name);

private:
    /**
     * @brief Reference to global category definitions registry
     *
     * Pointer to AsterixDefinition singleton containing all loaded
     * ASTERIX category definitions. Used to look up category-specific
     * parsing rules when processing data blocks.
     *
     * @note InputParser does NOT own this pointer. The caller (typically
     *       the main application or language binding) is responsible for
     *       creating and destroying the AsterixDefinition singleton.
     */
    AsterixDefinition *m_pDefinition;

};

#endif /* INPUTPARSER_H_ */
