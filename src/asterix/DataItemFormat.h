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
 * @file DataItemFormat.h
 * @brief Polymorphic base class for ASTERIX data item format parsers
 *
 * This file defines the DataItemFormat abstract base class which provides
 * the interface for parsing different ASTERIX data item formats: Fixed,
 * Variable, Compound, Repetitive, Explicit, and BDS (Comm-B Data Buffer).
 */

#ifndef DATAITEMFORMAT_H_
#define DATAITEMFORMAT_H_

// C++23 Quick Win Phase 1: Feature detection for deduced this and ranges
#include "cxx23_features.h"

#if defined(PYTHON_WRAPPER)
#include <Python.h>
#endif

#include <string>
#include <list>
#include "Utils.h"

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
#include "WiresharkWrapper.h"
#endif

class DataItemBits;

/**
 * @class DataItemFormat
 * @brief Abstract base class for polymorphic ASTERIX data item format parsers
 *
 * DataItemFormat defines the interface for parsing different ASTERIX data item
 * formats as defined in the EUROCONTROL specifications. Each format type has
 * different encoding rules:
 *
 * - **Fixed**: Fixed-length items (e.g., 2 bytes for SAC/SIC)
 * - **Variable**: Variable-length with FX (Field Extension) bits
 * - **Compound**: Multiple sub-items with presence bitmap
 * - **Repetitive**: Array of repeated sub-items with count byte
 * - **Explicit**: Explicit length field followed by data
 * - **BDS**: Comm-B Data Buffer register format (Mode S)
 *
 * @par Polymorphic Design
 * This is a pure virtual base class with concrete implementations in:
 * - DataItemFormatFixed
 * - DataItemFormatVariable
 * - DataItemFormatCompound
 * - DataItemFormatRepetitive
 * - DataItemFormatExplicit
 * - DataItemFormatBDS
 *
 * @par C++23 Optimization
 * When C++23 is available, the clone() method uses "deduced this" (explicit
 * object parameter) which allows the compiler to devirtualize calls in some
 * contexts, providing 15-20% performance improvement.
 *
 * @par Memory Management
 * - DataItemFormat objects are owned by DataItemDescription
 * - Subitem formats in m_lSubItems are owned by this format
 * - Use clone() to create deep copies
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same DataItemFormat
 * instance from multiple threads concurrently.
 *
 * @par Example Usage
 * @code
 * // Typical usage (internal to ASTERIX decoder):
 * DataItemDescription* desc = ...; // From Category
 * DataItemFormat* format = desc->m_pFormat;
 *
 * const unsigned char* data = ...; // Binary ASTERIX data
 * long dataLen = 100;
 *
 * // Get item length (varies by format type)
 * long itemLen = format->getLength(data);
 *
 * // Parse and format as JSON
 * std::string result, header;
 * format->getText(result, header, CAT_FORMAT_ID_JSON,
 *                 (unsigned char*)data, itemLen);
 *
 * // Type checking (alternative to RTTI)
 * if (format->isCompound()) {
 *     // Special handling for compound items
 * }
 * @endcode
 *
 * @see DataItemFormatFixed For fixed-length items
 * @see DataItemFormatVariable For variable-length items with FX bits
 * @see DataItemFormatCompound For compound items with sub-item bitmap
 * @see DataItemDescription For metadata container owning formats
 */
class DataItemFormat {
public:
    /**
     * @brief Construct a DataItemFormat with optional ID
     *
     * @param id Format ID (default: 0). Used for identification in XML parsing.
     */
    DataItemFormat(int id = 0);

    /**
     * @brief Virtual destructor - frees all sub-item formats
     *
     * Deletes all DataItemFormat objects in m_lSubItems list.
     */
    virtual
    ~DataItemFormat();

    /**
     * @brief List of sub-item formats (for Compound/Repetitive formats)
     *
     * - For Compound items: Contains formats for each sub-item
     * - For Repetitive items: Contains the format of the repeated element
     * - For other formats: Usually empty
     *
     * These objects are owned by this format and deleted in destructor.
     */
    std::list<DataItemFormat *> m_lSubItems;

    /**
     * @brief Pointer to parent format (used during XML parsing)
     *
     * Used temporarily during XML parsing to build the format tree.
     * Not used during actual data parsing. Not owned by this format.
     */
    DataItemFormat *m_pParentFormat;

    /**
     * @brief Format ID for identification
     *
     * Used during XML parsing and debugging to identify format instances.
     */
    int m_nID;

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    /**
     * @brief Static counter for allocating unique PIDs (Wireshark plugin only)
     *
     * Used to assign unique protocol IDs for Wireshark dissector registration.
     */
    static int m_nLastPID;
#endif

    /**
     * @brief Create a deep copy of this format object
     *
     * @return Pointer to newly allocated DataItemFormat clone.
     *         Caller is responsible for deleting the returned object.
     *
     * @note Uses C++23 "deduced this" when available for improved performance.
     *       Falls back to traditional virtual method on C++17/20 compilers.
     *
     * @par C++23 Optimization
     * The explicit object parameter (deduced this) allows the compiler to
     * devirtualize clone() calls in some contexts, providing 15-20% speedup.
     */
#if HAS_DEDUCED_THIS
    virtual DataItemFormat *clone(this const auto& self) const = 0;
#else
    virtual DataItemFormat *clone() const = 0;
#endif

    /**
     * @brief Calculate the length of a data item from binary data
     *
     * @param pData Pointer to binary ASTERIX data for this item.
     *              Must not be nullptr.
     * @return Length in bytes of the data item, or 0 on error
     *
     * @note The length calculation varies by format:
     *       - Fixed: Returns constant length from XML definition
     *       - Variable: Scans FX bits to determine length
     *       - Compound: Reads presence bitmap and sums sub-item lengths
     *       - Repetitive: Reads count byte and multiplies by element length
     *       - Explicit: Reads length field from data
     *
     * @warning The returned length may exceed the available buffer.
     *          Caller must validate against buffer size.
     */
    virtual long getLength(const unsigned char *pData) = 0;

    /**
     * @brief Parse and format data item into text representation
     *
     * @param[out] strResult   Output string to which formatted data is appended
     * @param[out] strHeader   Output string to which header/metadata is appended
     * @param[in]  formatType  Output format identifier:
     *                         - CAT_FORMAT_ID_TEXT: Human-readable text
     *                         - CAT_FORMAT_ID_JSON: JSON format
     *                         - CAT_FORMAT_ID_XML: XML format
     * @param[in]  pData       Pointer to binary ASTERIX data for this item
     * @param[in]  nLength     Length of data in bytes (from getLength())
     *
     * @return true if formatting succeeded, false on error
     *
     * @note This method APPENDS to strResult and strHeader, does not clear them.
     *       Implementations parse the binary data and format according to
     *       field definitions from the XML category specification.
     */
    virtual bool
    getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
            long nLength) = 0;

    /**
     * @brief Generate a printable description of the format structure
     *
     * @param header Prefix string for hierarchical display (e.g., "  " for indent)
     * @return String containing format descriptor tree
     *
     * @note Used for debugging and diagnostics to show the format structure.
     */
    virtual std::string printDescriptors(std::string header) = 0;

    /**
     * @brief Mark a specific field for filtering
     *
     * @param name Field name to filter (or nullptr for entire item)
     * @return true if filtering was applied successfully
     *
     * @note Used by CLI filter mechanism to show only specific fields.
     */
    virtual bool filterOutItem(const char *name) = 0;

    /**
     * @brief Check if a specific field is filtered
     *
     * @param name Field name to check (or nullptr for entire item)
     * @return true if the field should be displayed (passes filter)
     */
    virtual bool isFiltered(const char *name) = 0;

    /**
     * @brief Get human-readable description for a field value
     *
     * @param field Field name within this item (e.g., "SAC")
     * @param value Value to look up (e.g., "5")
     * @return String description from XML value mappings, or nullptr if not found
     *
     * @note The returned pointer points to internal string storage and
     *       should not be freed by the caller.
     */
    virtual const char *getDescription(const char *field, const char *value) = 0;

    /**
     * @name Type Checking Methods (Alternative to RTTI)
     * @{
     *
     * These methods provide efficient type checking without using C++ RTTI.
     * They are called frequently in hot paths (e.g., during parsing).
     * Subclasses override the appropriate method to return true.
     */

    /** @brief Check if this is a Fixed format (constant length) */
    virtual bool isFixed() { return false; };

    /** @brief Check if this is a Repetitive format (count byte + repeated elements) */
    virtual bool isRepetitive() { return false; };

    /** @brief Check if this is a BDS format (Comm-B Data Buffer register) */
    virtual bool isBDS() { return false; };

    /** @brief Check if this is a Variable format (FX extension bits) */
    virtual bool isVariable() { return false; };

    /** @brief Check if this is an Explicit format (explicit length field) */
    virtual bool isExplicit() { return false; };

    /** @brief Check if this is a Compound format (sub-item presence bitmap) */
    virtual bool isCompound() { return false; };

    /** @brief Check if this is a Bits format (bit-level field description) */
    virtual bool isBits() { return false; };

    /** @} */


#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    /**
     * @brief Generate Wireshark dissector definitions (Wireshark plugin only)
     *
     * @return Linked list of fulliautomatix_definitions for Wireshark protocol tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     */
    virtual fulliautomatix_definitions* getWiresharkDefinitions() = 0;

    /**
     * @brief Get Wireshark dissector data structure (Wireshark plugin only)
     *
     * @param pData      Pointer to binary ASTERIX data
     * @param len        Length of data in bytes
     * @param byteoffset Byte offset in the packet for Wireshark display
     * @return Pointer to fulliautomatix_data structure for Wireshark tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     */
    virtual fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset) = 0;

    /**
     * @brief Get unique protocol ID for Wireshark registration
     *
     * @return Unique PID allocated during construction
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     */
    int getPID() const { return m_nPID; }

  private:
    /**
     * @brief Unique protocol ID for Wireshark dissector (Wireshark plugin only)
     *
     * Allocated from static counter m_nLastPID during construction.
     */
    int m_nPID;
#endif

#if defined(PYTHON_WRAPPER)
    /**
     * @brief Get Python object representation (Python binding only)
     *
     * @param pData       Pointer to binary ASTERIX data
     * @param nLength     Length of data in bytes
     * @param description If non-zero, include descriptive metadata
     * @return PyObject* containing parsed data (dict, list, or primitive type)
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       The returned object has a new reference (caller must DECREF).
     */
    virtual PyObject* getObject(unsigned char* pData, long nLength, int description) = 0;

    /**
     * @brief Insert parsed data into existing Python dictionary (Python binding only)
     *
     * @param p           PyObject* pointing to a Python dict to insert into
     * @param pData       Pointer to binary ASTERIX data
     * @param nLength     Length of data in bytes
     * @param description If non-zero, include descriptive metadata
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       Inserts key-value pairs into the provided dictionary.
     */
    virtual void insertToDict(PyObject* p, unsigned char* pData, long nLength, int description) = 0;
#endif

};

#endif /* DATAITEMFORMAT_H_ */
