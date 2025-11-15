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
 * @file DataBlock.h
 * @brief ASTERIX data block container and parser
 *
 * This file defines the DataBlock class which represents a single ASTERIX
 * data block containing one or more data records, all of the same category.
 */

#ifndef DATABLOCK_H_
#define DATABLOCK_H_

#include "Category.h"
#include "DataRecord.h"

/**
 * @class DataBlock
 * @brief Container for a single ASTERIX data block with multiple records
 *
 * An ASTERIX data block is the fundamental transmission unit in the protocol.
 * Each data block contains:
 * - Category identifier (CAT field, 1 byte)
 * - Length field (2 bytes, total block length including header)
 * - One or more data records (all of the same category)
 *
 * Data blocks are extracted from various transport protocols (raw ASTERIX,
 * PCAP, HDLC, FINAL, GPS) and parsed into individual DataRecord objects.
 *
 * @par Binary Structure
 * @code
 * +----------+----------+----------+-------------------+
 * | Category | Length (MSB) | Length (LSB) | Data Records...   |
 * |  (1 byte)|  (1 byte)    |  (1 byte)    | (variable length) |
 * +----------+--------------+--------------+-------------------+
 * @endcode
 *
 * @par Memory Management
 * - DataBlock owns all DataRecord objects in m_lDataRecords
 * - DataBlock does NOT own the Category pointer (managed by AsterixDefinition)
 * - Caller is responsible for DataBlock lifetime
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same DataBlock instance
 * from multiple threads concurrently.
 *
 * @par Example Usage
 * @code
 * // Typical usage (internal to ASTERIX decoder):
 * Category* cat = globalDef.getCategory(62);
 * const unsigned char* blockData = ...; // Points to ASTERIX data block
 * unsigned long blockLen = (blockData[1] << 8) | blockData[2];
 * double timestamp = getCurrentTimestamp();
 *
 * DataBlock block(cat, blockLen, blockData, timestamp);
 *
 * if (block.m_bFormatOK) {
 *     std::string output;
 *     block.getText(output, CAT_FORMAT_ID_JSON);
 *     std::cout << output << std::endl;
 * }
 * @endcode
 *
 * @see DataRecord For individual record parsing
 * @see Category For category definitions
 */
class DataBlock {
public:
    /**
     * @brief Construct and parse an ASTERIX data block
     *
     * Parses the binary ASTERIX data block and extracts all data records.
     * The constructor automatically parses the block and populates m_lDataRecords.
     *
     * @param cat        Pointer to Category object for this data block's category.
     *                   Must not be nullptr. Not owned by DataBlock.
     * @param len        Total length of the data block in bytes (from length field)
     * @param data       Pointer to binary ASTERIX data block (including 3-byte header).
     *                   Must contain at least len bytes.
     * @param nTimestamp Capture timestamp in Unix epoch seconds (default: 0.0).
     *                   Typically from PCAP or system clock.
     *
     * @note After construction, check m_bFormatOK to verify successful parsing.
     *       If m_bFormatOK is false, the block data was malformed.
     *
     * @par Example
     * @code
     * Category* cat = globalDef.getCategory(62);
     * const unsigned char block[] = {0x3E, 0x00, 0x0C, ...};  // CAT062, 12 bytes
     * DataBlock db(cat, 12, block, 1234567890.5);
     *
     * if (db.m_bFormatOK) {
     *     printf("Parsed %zu records\n", db.m_lDataRecords.size());
     * }
     * @endcode
     */
    DataBlock(Category *cat, unsigned long len, const unsigned char *data, double nTimestamp = 0.0);

    /**
     * @brief Destructor - frees all data records
     *
     * Deletes all DataRecord objects in m_lDataRecords.
     */
    virtual
    ~DataBlock();

    /**
     * @brief Pointer to the Category definition for this data block
     *
     * All records in this block belong to the same category.
     * Not owned by DataBlock (managed by AsterixDefinition).
     */
    Category *m_pCategory;

    /**
     * @brief Total length of the data block in bytes (from length field)
     */
    unsigned long m_nLength;

    /**
     * @brief Capture timestamp in Unix epoch seconds
     *
     * Date and time when this packet was captured.
     * Value is in seconds since January 1, 1970 00:00:00 GMT.
     * Typically extracted from PCAP header or set by input device.
     */
    double m_nTimestamp;

    /**
     * @brief Parse status flag
     *
     * true if the data block was parsed successfully, false if malformed.
     * Check this after construction before using m_lDataRecords.
     */
    bool m_bFormatOK;

    /**
     * @brief List of parsed data records in this block
     *
     * Contains all DataRecord objects extracted from the data block.
     * These objects are owned by the DataBlock and deleted in destructor.
     */
    std::list<DataRecord *> m_lDataRecords;

    /**
     * @brief Generate formatted output for all records in this block
     *
     * Converts all data records in the block into a text representation
     * according to the specified format type.
     *
     * @param[out] strResult   Output string to which formatted data is appended
     * @param[in]  formatType  Output format identifier:
     *                         - CAT_FORMAT_ID_TEXT: Human-readable text
     *                         - CAT_FORMAT_ID_JSON: JSON format
     *                         - CAT_FORMAT_ID_XML: XML format
     *
     * @return true if formatting succeeded, false on error
     *
     * @note This method APPENDS to strResult, does not clear it.
     *       Iterates over all records in m_lDataRecords and calls
     *       DataRecord::getText() for each one.
     *
     * @par Example
     * @code
     * std::string json;
     * if (block.getText(json, CAT_FORMAT_ID_JSON)) {
     *     std::cout << json << std::endl;
     * }
     * @endcode
     */
    bool
    getText(std::string &strResult, const unsigned int formatType);

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    /**
     * @brief Get Wireshark dissector data structure (Wireshark plugin only)
     *
     * @param byteoffset Byte offset in the packet for Wireshark display
     * @return Pointer to fulliautomatix_data structure for Wireshark tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     */
    fulliautomatix_data* getData(int byteoffset);
#endif
#if defined(PYTHON_WRAPPER)
    /**
     * @brief Append Python list representation to existing list (Python binding only)
     *
     * @param plist   PyObject* pointing to a Python list to append to
     * @param verbose If non-zero, include descriptive metadata
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     */
    void getData(PyObject* plist, int verbose);
#endif

};

#endif /* DATABLOCK_H_ */
