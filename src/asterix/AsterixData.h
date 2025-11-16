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

#ifndef ASTERIXDATA_H_
#define ASTERIXDATA_H_

#include "AsterixDefinition.h"
#include "DataBlock.h"
#include <map>

/**
 * @class AsterixData
 * @brief Container for parsed ASTERIX data blocks
 *
 * AsterixData holds the result of parsing a complete ASTERIX packet,
 * which may contain one or more DataBlock instances. This is the top-level
 * structure returned by InputParser::parsePacket().
 *
 * ## Structure Hierarchy
 *
 * ```
 * AsterixData
 *  └─ DataBlock 1 (e.g., CAT048)
 *      └─ DataRecord 1
 *      └─ DataRecord 2
 *  └─ DataBlock 2 (e.g., CAT062)
 *      └─ DataRecord 1
 * ```
 *
 * ## Memory Management
 *
 * AsterixData **owns** all DataBlock objects in m_lDataBlocks and deletes
 * them in its destructor. Caller is responsible for deleting the AsterixData
 * instance after use.
 *
 * ## Usage Example
 *
 * ```cpp
 * InputParser parser(def);
 * const unsigned char* buffer = ...;
 * unsigned int size = ...;
 *
 * // Parse packet - returns new AsterixData
 * AsterixData* data = parser.parsePacket(buffer, size);
 *
 * // Access parsed blocks
 * for (auto* block : data->m_lDataBlocks) {
 *     int category = block->m_nCategory;
 *     // Process block...
 * }
 *
 * // Cleanup - deletes all DataBlocks
 * delete data;
 * ```
 *
 * ## Output Formats
 *
 * AsterixData can be formatted as:
 * - **Text**: Human-readable multi-line output
 * - **JSON**: Structured data (compact or pretty-printed)
 * - **XML**: Hierarchical markup
 *
 * @note Destructor automatically cleans up all contained DataBlock objects
 * @note Multi-category packets create multiple DataBlock entries
 * @note Empty AsterixData (no blocks) is valid for malformed input
 *
 * @see InputParser
 * @see DataBlock
 * @see DataRecord
 */
class AsterixData {
public:
    /**
     * @brief Construct an empty AsterixData container
     * @note Typically created by InputParser::parsePacket()
     */
    AsterixData();

    /**
     * @brief Destructor - deletes all contained DataBlock objects
     * @note Caller must delete the AsterixData instance after use
     */
    virtual
    ~AsterixData();

    std::list<DataBlock *> m_lDataBlocks; //!< List of parsed data blocks (owned by this object)

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    /**
     * @brief Convert to Wireshark dissector format
     * @return Wireshark-compatible data structure
     * @note Only available when compiled with WIRESHARK_WRAPPER
     */
    fulliautomatix_data* getData();
#endif

#if defined(PYTHON_WRAPPER)
    /**
     * @brief Convert to Python nested dictionary/list structure
     * @param verbose Include human-readable descriptions (0=false, 1=true)
     * @return PyObject* containing nested dicts/lists (new reference)
     * @note Only available when compiled with PYTHON_WRAPPER
     * @note Caller must DECREF the returned PyObject when done
     */
    PyObject* getData(int verbose);
#endif

    /**
     * @brief Format all data blocks as text
     * @param strResult String to append formatted output
     * @param formatType Output format (text, JSON, XML, etc.)
     * @return true if formatting successful, false on error
     * @note Iterates through all data blocks and concatenates their output
     */
    bool
    getText(std::string &strResult, const unsigned int formatType);
};

#endif /* ASTERIXDATA_H_ */
