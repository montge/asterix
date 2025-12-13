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
 * @file DataRecord.h
 * @brief ASTERIX data record parser and container
 *
 * This file defines the DataRecord class which represents a single ASTERIX
 * data record within a data block, containing multiple data items as specified
 * by the FSPEC (Field Specification).
 */

#ifndef DATARECORD_H_
#define DATARECORD_H_

#include "DataItem.h"
#include <memory>  // For std::unique_ptr

/**
 * @class DataRecord
 * @brief Represents a single ASTERIX data record with multiple data items
 *
 * A DataRecord is one complete message within an ASTERIX data block. Each record
 * contains:
 * - FSPEC (Field Specification) - Bitmap indicating which data items are present
 * - Data items - Variable-length fields as specified by the FSPEC
 *
 * The FSPEC is a variable-length bitmap where each bit corresponds to a data item
 * defined in the UAP (User Application Profile). The 7th bit of each FSPEC byte
 * is the FX (Field Extension) bit:
 * - FX=1: Another FSPEC byte follows
 * - FX=0: This is the last FSPEC byte
 *
 * @par Binary Structure
 * @code
 * +------------+------------+-----+------------+
 * | FSPEC byte | FSPEC byte | ... | Data Items |
 * |  (1+ bytes)|            |     | (variable) |
 * +------------+------------+-----+------------+
 *   ^                                ^
 *   |                                |
 *   FX bit determines continuation   Parsed according to FSPEC bits
 * @endcode
 *
 * @par Memory Management
 * - DataRecord owns all DataItem objects in m_lDataItems
 * - DataRecord owns m_pFSPECData and m_pHexData buffers
 * - DataRecord does NOT own the Category pointer (managed by AsterixDefinition)
 * - Caller is responsible for DataRecord lifetime
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same DataRecord instance
 * from multiple threads concurrently.
 *
 * @par Example Usage
 * @code
 * // Typical usage (internal to DataBlock parsing):
 * Category* cat = globalDef.getCategory(62);
 * const unsigned char* recordData = ...; // Points to FSPEC + data items
 * unsigned long recordLen = ...;         // Length from parent DataBlock
 * double timestamp = getCurrentTimestamp();
 *
 * DataRecord record(cat, 0, recordLen, recordData, timestamp);
 *
 * if (record.m_bFormatOK) {
 *     // Access specific data item
 *     DataItem* sacSic = record.getItem("010");
 *     if (sacSic) {
 *         std::string text, header;
 *         sacSic->getText(text, header, CAT_FORMAT_ID_TEXT);
 *         std::cout << "SAC/SIC: " << text << std::endl;
 *     }
 * }
 * @endcode
 *
 * @see DataItem For individual data item parsing
 * @see DataBlock For container of multiple records
 * @see UAP For FSPEC bit mapping to data items
 */
class DataRecord {
public:
    /**
     * @brief Construct and parse an ASTERIX data record
     *
     * Parses the FSPEC bitmap and extracts all present data items according
     * to the UAP definition for this category. The constructor automatically
     * parses the record and populates m_lDataItems.
     *
     * @param cat        Pointer to Category object for this record's category.
     *                   Must not be nullptr. Not owned by DataRecord.
     * @param id         Record ID within the parent DataBlock (0-based index)
     * @param len        Total length of the record in bytes (FSPEC + data items)
     * @param data       Pointer to binary ASTERIX record data (starts with FSPEC).
     *                   Must contain at least len bytes.
     * @param nTimestamp Capture timestamp in Unix epoch seconds.
     *                   Typically inherited from parent DataBlock.
     *
     * @note After construction, check m_bFormatOK to verify successful parsing.
     *       If m_bFormatOK is false, the record data was malformed.
     *
     * @par Example
     * @code
     * Category* cat = globalDef.getCategory(62);
     * const unsigned char record[] = {0xFD, 0x00, ...};  // FSPEC + items
     * DataRecord dr(cat, 0, recordLen, record, 1234567890.5);
     *
     * if (dr.m_bFormatOK) {
     *     printf("Parsed %zu items\n", dr.m_lDataItems.size());
     * }
     * @endcode
     */
    DataRecord(Category *cat, int id, unsigned long len, const unsigned char *data, double nTimestamp);

    /**
     * @brief Destructor - frees all data items and internal buffers
     *
     * Deletes all DataItem objects in m_lDataItems, m_pFSPECData, and m_pHexData.
     */
    virtual
    ~DataRecord();

    /**
     * @brief Pointer to the Category definition for this record
     *
     * Not owned by DataRecord (managed by AsterixDefinition).
     */
    Category *m_pCategory;

    /**
     * @brief Record ID within the parent DataBlock (0-based index)
     *
     * Used for identifying individual records when multiple records
     * are present in a DataBlock.
     */
    int m_nID;

    /**
     * @brief Total length of the record in bytes (FSPEC + data items)
     */
    unsigned long m_nLength;

    /**
     * @brief Length of the FSPEC in bytes
     *
     * The FSPEC is variable length depending on how many extension bits (FX) are set.
     * Typically 1-4 bytes for most ASTERIX categories.
     */
    unsigned long m_nFSPECLength;

    /**
     * @brief Copy of the FSPEC bitmap bytes
     *
     * Smart pointer to allocated buffer containing a copy of the FSPEC bytes from the record.
     * Size is m_nFSPECLength bytes. Owned by DataRecord and automatically freed.
     */
    std::unique_ptr<unsigned char[]> m_pFSPECData;

    /**
     * @brief Capture timestamp in Unix epoch seconds
     *
     * Date and time when this packet was captured.
     * Value is in seconds since January 1, 1970 00:00:00 GMT.
     * Inherited from parent DataBlock.
     */
    double m_nTimestamp;

    /**
     * @brief CRC-32 checksum of the record data
     *
     * Calculated over the entire record (FSPEC + data items).
     * Used for data integrity verification and duplicate detection.
     */
    uint32_t m_nCrc;

    /**
     * @brief Hexadecimal string representation of the record data
     *
     * Human-readable hex dump of the binary record data (FSPEC + items).
     * Smart pointer to allocated buffer owned by DataRecord and automatically freed.
     * Used for debug output and logging.
     */
    std::unique_ptr<char[]> m_pHexData;

    /**
     * @brief Parse status flag
     *
     * true if the record was parsed successfully, false if malformed.
     * Check this after construction before using m_lDataItems.
     */
    bool m_bFormatOK;

    /**
     * @brief List of parsed data items in this record
     *
     * Contains all DataItem objects extracted according to the FSPEC bitmap.
     * These objects are owned by the DataRecord and deleted in destructor.
     */
    std::list<DataItem *> m_lDataItems;

    /**
     * @brief Get the category number for this record
     *
     * @return Category number (0-255), or 0 if m_pCategory is nullptr
     *
     * @note Convenience method to avoid null pointer checks on m_pCategory.
     */
    int getCategory() const { return (m_pCategory) ? m_pCategory->m_id : 0; }

    /**
     * @brief Generate formatted output for all items in this record
     *
     * Converts all data items in the record into a text representation
     * according to the specified format type.
     *
     * @param[out] strResult   Output string to which formatted data is appended
     * @param[out] strHeader   Output string to which header/metadata is appended
     * @param[in]  formatType  Output format identifier:
     *                         - CAT_FORMAT_ID_TEXT: Human-readable text
     *                         - CAT_FORMAT_ID_JSON: JSON format
     *                         - CAT_FORMAT_ID_XML: XML format
     *
     * @return true if formatting succeeded, false on error
     *
     * @note This method APPENDS to strResult and strHeader, does not clear them.
     *       Iterates over all items in m_lDataItems and calls
     *       DataItem::getText() for each one.
     *
     * @par Example
     * @code
     * std::string json, header;
     * if (record.getText(json, header, CAT_FORMAT_ID_JSON)) {
     *     std::cout << json << std::endl;
     * }
     * @endcode
     */
    bool getText(std::string &strResult, std::string &strHeader,
                 const unsigned int formatType);

    /**
     * @brief Get a specific data item by its ID
     *
     * @param itemid Data item ID without category prefix (e.g., "010" for I062/010)
     * @return Pointer to DataItem if found, nullptr if not present in this record
     *
     * @note The returned pointer is owned by the DataRecord.
     *       Do not delete it.
     *
     * @par Example
     * @code
     * DataItem* sac_sic = record.getItem("010");
     * if (sac_sic) {
     *     std::string text, header;
     *     sac_sic->getText(text, header, CAT_FORMAT_ID_TEXT);
     *     printf("SAC/SIC: %s\n", text.c_str());
     * }
     * @endcode
     */
    DataItem *getItem(std::string itemid);

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
     * @brief Get Python dictionary representation (Python binding only)
     *
     * @param verbose If non-zero, include descriptive metadata
     * @return PyObject* containing nested dict/list structure with parsed data
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     */
    PyObject* getData(int verbose);
#endif
};

#endif /* DATARECORD_H_ */
