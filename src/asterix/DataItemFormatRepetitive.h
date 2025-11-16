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

#ifndef DATAITEMFORMATREPETITIVE_H_
#define DATAITEMFORMATREPETITIVE_H_

#include "DataItemFormatFixed.h"
#include "cxx23_features.h"

/**
 * @class DataItemFormatRepetitive
 * @brief Parser for ASTERIX Repetitive data items
 *
 * Repetitive format represents data items that consist of a repetition counter
 * followed by N identical fixed-length sub-items. This format is commonly used
 * for lists of similar data elements (e.g., multiple aircraft positions, waypoints).
 *
 * ## Structure
 *
 * ```
 * +----------------------+
 * | Repetition Count (1) | (1 byte: number of sub-items, 0-255)
 * +----------------------+
 * | Sub-Item 1           | (Fixed length)
 * +----------------------+
 * | Sub-Item 2           | (Fixed length)
 * +----------------------+
 * | ...                  |
 * +----------------------+
 * | Sub-Item N           | (Fixed length)
 * +----------------------+
 * ```
 *
 * ## Example Usage
 *
 * CAT048 I048/040 (Measured Position in Slant Polar Coordinates) can be repetitive:
 * - Byte 0: Count of measurements (e.g., 3)
 * - Bytes 1-8: First measurement (rho, theta)
 * - Bytes 9-16: Second measurement
 * - Bytes 17-24: Third measurement
 *
 * @note The sub-item format is always DataItemFormatFixed (constant length)
 * @note Includes overflow protection for repetition count × sub-item length
 * @note Maximum 255 repetitions due to 1-byte counter
 *
 * @see DataItemFormat
 * @see DataItemFormatFixed
 */
class DataItemFormatRepetitive : public DataItemFormat {
public:
    /**
     * @brief Construct a new Repetitive format parser
     * @param id Unique identifier for this format instance (default: 0)
     */
    DataItemFormatRepetitive(int id = 0);

    /**
     * @brief Copy constructor - performs deep copy of sub-item format
     * @param obj Source Repetitive format object to copy
     */
    DataItemFormatRepetitive(const DataItemFormatRepetitive &obj);

    /**
     * @brief Destructor - cleans up sub-item list
     */
    virtual
    ~DataItemFormatRepetitive();

    /**
     * @brief Create a deep copy of this Repetitive format object
     * @return Pointer to newly allocated clone (caller must delete)
     * @note C++23: Uses deduced this for better devirtualization
     */
    // C++23 Quick Win: Deduced this allows better devirtualization
#if HAS_DEDUCED_THIS
    DataItemFormatRepetitive *clone(this const auto& self) const { return new DataItemFormatRepetitive(self); }
#else
    DataItemFormatRepetitive *clone() const { return new DataItemFormatRepetitive(*this); } // Return clone of object
#endif

    /**
     * @brief Calculate total length of repetitive item from binary data
     * @param pData Pointer to start of repetitive item (first byte = count)
     * @return Total length in bytes (1 + count × sub-item_length)
     * @note Returns 0 on error (missing sub-item format)
     * @note Includes integer overflow protection (VULN-001 fix)
     */
    long getLength(const unsigned char *pData);

    /**
     * @brief Extract and format all repetitive sub-items as text
     * @param strResult Output string to append formatted values
     * @param strHeader Header string for formatting context
     * @param formatType Output format type (text, JSON, XML, etc.)
     * @param pData Pointer to raw binary data
     * @param nLength Length of data buffer in bytes
     * @return true if parsing successful, false on error
     * @note Parses first byte for count, then iterates through sub-items
     */
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength); // appends value description to strResult

    /**
     * @brief Generate human-readable format descriptor string
     * @param header Prefix string for hierarchical formatting
     * @return String describing the repetitive structure and sub-item format
     */
    std::string printDescriptors(std::string header); // print items format descriptors

    /**
     * @brief Mark the sub-item field for filtering (exclusion from output)
     * @param name Name/ID of field to filter out
     * @return true if filter applied, false if field not found
     */
    bool filterOutItem(const char *name); // mark item for filtering

    /**
     * @brief Check if the sub-item field is currently filtered
     * @param name Name/ID of field to check
     * @return true if field is filtered (hidden), false otherwise
     */
    bool isFiltered(const char *name);

    /**
     * @brief Type identification - returns true for Repetitive format
     * @return Always returns true
     * @note Used for runtime type checking in parsing pipeline
     */
    bool isRepetitive() { return true; }; // true if this is Repetitive format

    /**
     * @brief Get human-readable description for a field value
     * @param field Name of field within sub-item
     * @param value String representation of value
     * @return Description string, or nullptr if not found
     */
    const char *getDescription(const char *field, const char *value); // return description ef element

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    fulliautomatix_definitions* getWiresharkDefinitions();
    fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif
#if defined(PYTHON_WRAPPER)
    PyObject* getObject(unsigned char* pData, long nLength, int verbose);
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose);
#endif
};

#endif /* DATAITEMFORMATREPETITIVE_H_ */
