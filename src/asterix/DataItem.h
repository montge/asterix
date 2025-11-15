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
 * @file DataItem.h
 * @brief Core data structure representing a single ASTERIX data item
 *
 * This file defines the DataItem class which encapsulates a single field
 * within an ASTERIX data record, including its binary data, parsed values,
 * and associated metadata from the XML category definitions.
 */

#ifndef DATAITEM_H_
#define DATAITEM_H_

#include "DataItemDescription.h"
#include <string>

/**
 * @class DataItem
 * @brief Represents a single ASTERIX data item (field) within a data record
 *
 * DataItem is the fundamental unit of ASTERIX data, representing a single field
 * such as SAC/SIC (I062/010), target position (I062/105), or track number (I062/040).
 *
 * Each DataItem contains:
 * - Binary data extracted from the ASTERIX stream
 * - Length information (can be fixed or variable depending on item format)
 * - Reference to DataItemDescription (metadata from XML category definition)
 * - Parsing logic delegated to DataItemFormat subclasses
 *
 * @par Memory Management
 * - DataItem owns the binary data buffer (m_pData) allocated during parsing
 * - DataItemDescription is NOT owned (managed by Category class)
 * - Caller is responsible for DataItem lifetime
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same DataItem instance
 * from multiple threads concurrently.
 *
 * @par Example Usage
 * @code
 * // Typical usage pattern (internal to ASTERIX decoder):
 * DataItemDescription* desc = category->getDataItemDescription("010");
 * DataItem item(desc);
 *
 * const unsigned char* data = ...; // Binary ASTERIX data
 * long bytesConsumed = item.parse(data, dataLength);
 *
 * if (bytesConsumed > 0) {
 *     std::string text, header;
 *     item.getText(text, header, CAT_FORMAT_ID_TEXT);
 *     printf("Item: %s\n", text.c_str());
 * }
 * @endcode
 *
 * @see DataItemDescription For metadata and format information
 * @see DataItemFormat For parsing logic (Fixed, Variable, Compound, etc.)
 * @see DataRecord For the container holding multiple DataItem instances
 */
class DataItem {
public:
    /**
     * @brief Construct a DataItem with associated metadata
     *
     * @param pDesc Pointer to DataItemDescription containing XML metadata
     *              (item ID, format, encoding rules). Must not be null.
     *              The DataItem does NOT take ownership of this pointer.
     *
     * @note The DataItemDescription must remain valid for the lifetime
     *       of this DataItem instance.
     */
    DataItem(DataItemDescription *pDesc);

    /**
     * @brief Destructor - frees allocated binary data buffer
     *
     * Releases the internal m_pData buffer allocated during parse().
     * Does NOT delete m_pDescription (not owned by DataItem).
     */
    virtual
    ~DataItem();

    /**
     * @brief Metadata describing this data item's structure and encoding
     *
     * Points to the DataItemDescription from the Category's XML definition.
     * This includes:
     * - Item ID (e.g., "010" for SAC/SIC in CAT062)
     * - Format type (Fixed, Variable, Compound, Repetitive, etc.)
     * - Bit-level field definitions
     * - Value encoding and units
     *
     * @warning This pointer is NOT owned by DataItem. Do not delete.
     */
    DataItemDescription *m_pDescription;

    /**
     * @brief Generate human-readable or formatted output for this data item
     *
     * Converts the parsed binary data into a text representation according
     * to the specified format type (text, JSON, XML).
     *
     * @param[out] strResult Output string to which formatted data is appended
     * @param[out] strHeader Output string to which header/metadata is appended
     * @param[in]  formatType Output format identifier:
     *                        - CAT_FORMAT_ID_TEXT: Human-readable text
     *                        - CAT_FORMAT_ID_JSON: JSON format
     *                        - CAT_FORMAT_ID_XML: XML format
     *
     * @return true if formatting succeeded, false on error
     *
     * @note This method APPENDS to strResult and strHeader, does not clear them.
     *       The actual formatting is delegated to the DataItemFormat instance
     *       associated with m_pDescription.
     *
     * @par Example
     * @code
     * std::string text, header;
     * if (item.getText(text, header, CAT_FORMAT_ID_TEXT)) {
     *     std::cout << header << ": " << text << std::endl;
     * }
     * @endcode
     */
    bool getText(std::string &strResult, std::string &strHeader,
                 const unsigned int formatType);

    /**
     * @brief Parse binary ASTERIX data into this DataItem
     *
     * Extracts and validates binary data according to the format specified
     * in m_pDescription. The actual parsing logic is delegated to the
     * appropriate DataItemFormat subclass (Fixed, Variable, Compound, etc.).
     *
     * @param[in] pData Pointer to binary ASTERIX data buffer. Must not be null.
     * @param[in] len   Number of bytes available in pData buffer
     *
     * @return Number of bytes consumed from pData buffer (>0 on success),
     *         or 0 on parse error
     *
     * @note After successful parsing:
     *       - m_pData contains a copy of the consumed bytes
     *       - m_nLength contains the number of bytes consumed
     *       - The DataItemFormat has extracted individual field values
     *
     * @warning The input buffer pData must contain at least the minimum
     *          number of bytes required for this item's format, otherwise
     *          parsing will fail and return 0.
     *
     * @par Example
     * @code
     * const unsigned char data[] = {0x30, 0x00, 0x10, ...};
     * long consumed = item.parse(data, sizeof(data));
     * if (consumed > 0) {
     *     printf("Parsed %ld bytes\n", consumed);
     * } else {
     *     fprintf(stderr, "Parse error\n");
     * }
     * @endcode
     */
    long parse(const unsigned char *pData, long len);

    /**
     * @brief Get the length in bytes of the parsed data item
     *
     * @return Number of bytes consumed during parse(), or 0 if not yet parsed
     *
     * @note This is the actual length from the binary stream, which may differ
     *       from the declared length in the XML definition for variable-length items.
     */
    long getLength() { return m_nLength; }

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

private:
    /**
     * @brief Binary data buffer containing the raw ASTERIX bytes
     *
     * Allocated and populated during parse() call. Contains exactly
     * m_nLength bytes copied from the input stream.
     */
    unsigned char *m_pData;

    /**
     * @brief Length in bytes of the parsed data item
     *
     * Set during parse() to indicate how many bytes were consumed
     * from the input buffer. This is the actual length from the binary
     * stream (may be variable for some item formats).
     */
    long m_nLength;

};

#endif /* DATAITEM_H_ */
