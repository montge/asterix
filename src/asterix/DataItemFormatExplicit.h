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

#ifndef DATAITEMFORMATEXPLICIT_H_
#define DATAITEMFORMATEXPLICIT_H_

#include "DataItemFormat.h"
#include "DataItemFormatFixed.h"
#include "cxx23_features.h"

/**
 * @class DataItemFormatExplicit
 * @brief Parser for ASTERIX Explicit data items
 *
 * Explicit format represents variable-length data items where the first byte
 * explicitly specifies the total length of the item (including the length byte itself).
 * This format is used when the structure is known but the total size varies based
 * on optional sub-fields or data content.
 *
 * ## Structure
 *
 * ```
 * +----------------+
 * | Length (1)     | (1 byte: total length including this byte, 0-255)
 * +----------------+
 * | Data           | (Variable length sub-items)
 * | Sub-Items      |
 * | ...            |
 * +----------------+
 * ```
 *
 * ## Length Encoding
 *
 * - Length byte = total bytes including itself
 * - Length = 1 means only the length byte (no data)
 * - Length = N means (N-1) bytes of actual data follow
 * - Maximum item size: 255 bytes total
 *
 * ## Example Usage
 *
 * CAT062 I062/295 (Track Data Ages) uses explicit format:
 * - Byte 0: Length (e.g., 5 = 1 length byte + 4 data bytes)
 * - Bytes 1-4: Age data for various track components
 *
 * @note Used when item structure is known but size varies
 * @note Differs from Variable format (FSPEC-based) and Compound (primary/secondary)
 * @note Maximum size limited to 255 bytes due to 1-byte length field
 *
 * @see DataItemFormat
 * @see DataItemFormatVariable
 * @see DataItemFormatCompound
 */
class DataItemFormatExplicit : public DataItemFormat {
public:
    /**
     * @brief Construct a new Explicit format parser
     * @param id Unique identifier for this format instance (default: 0)
     */
    DataItemFormatExplicit(int id = 0);

    /**
     * @brief Copy constructor - performs deep copy of all sub-items
     * @param obj Source Explicit format object to copy
     */
    DataItemFormatExplicit(const DataItemFormatExplicit &obj);

    /**
     * @brief Destructor - cleans up sub-item list
     */
    virtual
    ~DataItemFormatExplicit();

    /**
     * @brief Create a deep copy of this Explicit format object
     * @return Pointer to newly allocated clone (caller must delete)
     * @note C++23: Uses deduced this for better devirtualization
     */
    // C++23 Quick Win: Deduced this allows better devirtualization
#if HAS_DEDUCED_THIS
    DataItemFormatExplicit *clone(this const auto& self) const { return new DataItemFormatExplicit(self); }
#else
    DataItemFormatExplicit *clone() const { return new DataItemFormatExplicit(*this); } // Return clone of object
#endif

    /**
     * @brief Read total item length from the first byte
     * @param pData Pointer to start of explicit item (first byte = length)
     * @return Total length in bytes as specified in first byte
     * @note Simply returns value of first byte (*pData)
     */
    long getLength(const unsigned char *pData);

    /**
     * @brief Extract and format explicit item data as text
     * @param strResult Output string to append formatted value
     * @param strHeader Header string for formatting context
     * @param formatType Output format type (text, JSON, XML, etc.)
     * @param pData Pointer to raw binary data
     * @param nLength Length of data buffer in bytes
     * @return true if parsing successful, false on error
     * @note Skips first byte (length), parses sub-items from pData+1
     * @note Returns false if nLength <= 1 (no data after length byte)
     */
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength); // appends value description to strResult

    /**
     * @brief Generate human-readable format descriptor string
     * @param header Prefix string for hierarchical formatting
     * @return String describing the explicit structure and sub-items
     */
    std::string printDescriptors(std::string header); // print items format descriptors

    /**
     * @brief Mark a sub-item field for filtering (exclusion from output)
     * @param name Name/ID of field to filter out
     * @return true if filter applied, false if field not found
     */
    bool filterOutItem(const char *name); // mark item for filtering

    /**
     * @brief Check if a sub-item field is currently filtered
     * @param name Name/ID of field to check
     * @return true if field is filtered (hidden), false otherwise
     */
    bool isFiltered(const char *name);

    /**
     * @brief Type identification - returns true for Explicit format
     * @return Always returns true
     * @note Used for runtime type checking in parsing pipeline
     */
    bool isExplicit() { return true; }; // true if this is Explicit format

    /**
     * @brief Get human-readable description for a field value
     * @param field Name of field within explicit item
     * @param value String representation of value
     * @return Description string, or nullptr if not found
     */
    const char *getDescription(const char *field, const char *value); // return description ef element

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    fulliautomatix_definitions* getWiresharkDefinitions();
    fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif

#if defined(PYTHON_WRAPPER)
    PyObject* getObject(unsigned char* pData, long nLength, int description);
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int description);
#endif
};

#endif /* DATAITEMFORMATEXPLICIT_H_ */
