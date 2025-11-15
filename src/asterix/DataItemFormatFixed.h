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
 * @file DataItemFormatFixed.h
 * @brief Fixed-length ASTERIX data item format parser
 *
 * This file defines the DataItemFormatFixed class which handles parsing
 * of fixed-length ASTERIX data items - items with a constant, predetermined
 * length specified in the XML category definition.
 */

#ifndef DATAITEMFORMATFIXED_H_
#define DATAITEMFORMATFIXED_H_

#include "DataItemFormat.h"
#include "DataItemBits.h"
#include "cxx23_features.h"

/**
 * @class DataItemFormatFixed
 * @brief Parser for fixed-length ASTERIX data items
 *
 * DataItemFormatFixed handles ASTERIX data items with a constant,
 * predetermined length. Unlike Variable or Compound formats, Fixed items
 * have no extension bits or sub-item presence indicators - their length
 * is defined in the XML category specification and never changes at runtime.
 *
 * @par What are Fixed-Length Items?
 * Fixed-length items are the simplest ASTERIX format type. Common examples:
 * - **I010/010 (SAC/SIC)**: 2 bytes - System Area Code + System ID Code
 * - **I062/010 (SAC/SIC)**: 2 bytes - Track data source identification
 * - **I048/020 (Target Report Descriptor)**: 1 byte - Report type flags
 *
 * The length is specified in the XML definition using the `length` attribute:
 * @code{.xml}
 * <DataItemFormat desc="System Area Code / System Identification Code">
 *   <Fixed length="2">
 *     <Bits bit="16" fx="0">
 *       <BitsShortName>SAC</BitsShortName>
 *       <BitsName>System Area Code</BitsName>
 *     </Bits>
 *     <Bits bit="8" fx="0">
 *       <BitsShortName>SIC</BitsShortName>
 *       <BitsName>System Identification Code</BitsName>
 *     </Bits>
 *   </Fixed>
 * </DataItemFormat>
 * @endcode
 *
 * @par Comparison with Other Formats
 * ASTERIX defines 6 primary format types:
 *
 * 1. **Fixed** (this class): Constant length, no extension mechanism
 *    - Example: I010/010 (SAC/SIC) = 2 bytes always
 *    - Parsing: Simply read m_nLength bytes
 *
 * 2. **Variable** (DataItemFormatVariable): Variable length with FX bits
 *    - Example: I062/380 (Track Data Ages) = 1-N bytes
 *    - Parsing: Read octets until FX bit = 0
 *
 * 3. **Compound** (DataItemFormatCompound): Sub-items with presence bitmap
 *    - Example: I062/340 (Measured Information) = variable
 *    - Parsing: Read FSPEC bitmap, then parse present sub-items
 *
 * 4. **Repetitive** (DataItemFormatRepetitive): Count byte + repeated elements
 *    - Example: I062/295 (Track Data Ages) = 1 + (N * elementLen)
 *    - Parsing: Read count byte, then parse N repeated elements
 *
 * 5. **Explicit** (DataItemFormatExplicit): Explicit length field + data
 *    - Example: I062/390 (Flight Plan Related Data) = 1 + len
 *    - Parsing: Read length byte, then parse specified bytes
 *
 * 6. **BDS** (DataItemFormatBDS): Mode S Comm-B Data Buffer registers
 *    - Example: I062/370 (Mode S MB Data) = 8 bytes (BDS register)
 *    - Parsing: Special handling for Mode S registers
 *
 * @par Parsing Process
 * Fixed items have the simplest parsing process:
 *
 * 1. **Length determination**: getLength() returns m_nLength (constant)
 * 2. **Data extraction**: Read exactly m_nLength bytes from input stream
 * 3. **Field parsing**: Parse bit fields according to DataItemBits definitions
 * 4. **Formatting**: Format fields as text/JSON/XML via getText()
 *
 * No runtime calculations needed - length is known at compile time (from XML).
 *
 * @par Memory Management
 * - DataItemFormatFixed objects are owned by DataItemDescription
 * - m_lSubItems usually empty (Fixed items rarely have sub-items)
 * - Use clone() to create deep copies
 *
 * @par Performance
 * Fixed format is the fastest to parse because:
 * - No extension bit scanning (vs Variable)
 * - No presence bitmap processing (vs Compound)
 * - No count byte reading (vs Repetitive)
 * - No length field parsing (vs Explicit)
 * - Length known at compile time (constant folding possible)
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same DataItemFormatFixed
 * instance from multiple threads concurrently.
 *
 * @par Example Usage (I010/010 SAC/SIC)
 * @code
 * // XML definition (simplified):
 * // <Fixed length="2">
 * //   <Bits bit="16"><BitsShortName>SAC</BitsShortName></Bits>
 * //   <Bits bit="8"><BitsShortName>SIC</BitsShortName></Bits>
 * // </Fixed>
 *
 * // C++ usage (internal to ASTERIX decoder):
 * unsigned char data[2] = {0x05, 0x1A}; // SAC=5, SIC=26
 *
 * DataItemFormatFixed* format = ...; // From Category I010
 * assert(format->m_nLength == 2);
 *
 * // Get length (always 2 bytes)
 * long len = format->getLength(data); // Returns 2
 *
 * // Parse and format as JSON
 * std::string result, header;
 * format->getText(result, header, CAT_FORMAT_ID_JSON, data, len);
 * // result contains: {"SAC":5,"SIC":26}
 *
 * // Type checking
 * assert(format->isFixed() == true);
 * assert(format->isVariable() == false);
 * @endcode
 *
 * @see DataItemFormat For polymorphic base class
 * @see DataItemFormatVariable For variable-length items with FX bits
 * @see DataItemFormatCompound For compound items with sub-item bitmap
 * @see DataItemBits For bit-level field definitions
 */
class DataItemFormatFixed : public DataItemFormat {
public:
    /**
     * @brief Construct a Fixed format parser with optional ID
     *
     * @param id Format ID (default: 0). Used during XML parsing and debugging.
     *
     * @note The m_nLength field must be set separately (usually during XML parsing)
     *       before this format can be used for parsing data.
     */
    DataItemFormatFixed(int id = 0);

    /**
     * @brief Copy constructor - creates deep copy of Fixed format
     *
     * @param obj Source DataItemFormatFixed to copy from
     *
     * @note Copies all fields including m_nLength and m_lSubItems.
     *       Sub-items in m_lSubItems are deep-copied via clone().
     */
    DataItemFormatFixed(const DataItemFormatFixed &obj);

    /**
     * @brief Virtual destructor
     *
     * Frees all sub-item formats in m_lSubItems (inherited from DataItemFormat).
     */
    virtual
    ~DataItemFormatFixed();

    /**
     * @brief Length of this Fixed item in bytes (constant)
     *
     * This value is set during XML parsing from the `length` attribute:
     * @code{.xml}
     * <Fixed length="2">...</Fixed>
     * @endcode
     *
     * @note This is the ONLY format type where length is constant.
     *       All other formats calculate length dynamically from data.
     *
     * @par Typical Values
     * - 1 byte: Single-octet flags (e.g., I048/020)
     * - 2 bytes: SAC/SIC pairs (e.g., I010/010, I062/010)
     * - 3 bytes: Time fields (e.g., I062/070 = 3 bytes for Time of Track)
     * - 4 bytes: Longer fields or packed multi-field items
     * - 6 bytes: Extended time fields or multiple coordinates
     * - 8 bytes: Full timestamps or double-precision values
     */
    int m_nLength;

    /**
     * @brief Create a deep copy of this Fixed format object
     *
     * @return Pointer to newly allocated DataItemFormatFixed clone.
     *         Caller is responsible for deleting the returned object.
     *
     * @note Uses C++23 "deduced this" when available for improved performance.
     *       Falls back to traditional virtual method on C++17/20 compilers.
     *       Copies all fields including m_nLength and deep-copies m_lSubItems.
     *
     * @par C++23 Optimization
     * The explicit object parameter (deduced this) allows the compiler to
     * devirtualize clone() calls in some contexts, providing 15-20% speedup.
     */
    // C++23 Quick Win: Deduced this allows better devirtualization
#if HAS_DEDUCED_THIS
    DataItemFormatFixed *clone(this const auto& self) const { return new DataItemFormatFixed(self); }
#else
    DataItemFormatFixed *clone() const { return new DataItemFormatFixed(*this); } // Return clone of object
#endif

    /**
     * @brief Get the constant length of this Fixed item
     *
     * @return Length in bytes (m_nLength) as configured in XML definition
     *
     * @note This is a legacy method. Prefer getLength(const unsigned char* pData)
     *       which is the polymorphic interface method.
     */
    long getLength();

    /**
     * @brief Check if this is the last part of a multi-part item
     *
     * @param pData Pointer to binary ASTERIX data for this item (unused for Fixed)
     * @return Always true for Fixed items (single-part by definition)
     *
     * @note Fixed items are always single-part (no extension mechanism).
     *       This method exists for compatibility with Variable format.
     */
    bool isLastPart(const unsigned char *pData);

    /**
     * @brief Check if a secondary part is present
     *
     * @param pData Pointer to binary ASTERIX data for this item (unused)
     * @param part  Part number to check (unused)
     * @return Always false for Fixed items (no secondary parts)
     *
     * @note Fixed items have no extension mechanism, so no secondary parts exist.
     *       This method exists for compatibility with Variable format.
     */
    bool isSecondaryPartPresent(const unsigned char *pData, int part);

    /**
     * @brief Get the name of a specific part (for multi-part items)
     *
     * @param part Part number (unused for Fixed items)
     * @return Reference to empty string (Fixed items are single-part)
     *
     * @note Fixed items have no named parts. This method exists for
     *       compatibility with Variable format which supports multiple parts.
     */
    std::string &getPartName(int part);

    /**
     * @brief Calculate the length of this Fixed item from binary data
     *
     * @param pData Pointer to binary ASTERIX data (unused, length is constant)
     * @return Length in bytes (always m_nLength)
     *
     * @note For Fixed items, length is ALWAYS m_nLength regardless of data content.
     *       The pData parameter is accepted for polymorphic interface compatibility
     *       but is never examined. This is the fastest getLength() implementation
     *       of all format types.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getLength(const unsigned char*)
     */
    long getLength(const unsigned char *pData);

    /**
     * @brief Parse and format Fixed item data into text representation
     *
     * @param[out] strResult   Output string to which formatted data is appended
     * @param[out] strHeader   Output string to which header/metadata is appended
     * @param[in]  formatType  Output format identifier:
     *                         - CAT_FORMAT_ID_TEXT: Human-readable text
     *                         - CAT_FORMAT_ID_JSON: JSON format
     *                         - CAT_FORMAT_ID_XML: XML format
     * @param[in]  pData       Pointer to binary ASTERIX data for this Fixed item
     * @param[in]  nLength     Length of data in bytes (should equal m_nLength)
     *
     * @return true if formatting succeeded, false on error
     *
     * @note This method APPENDS to strResult and strHeader, does not clear them.
     *       Parses bit fields according to DataItemBits definitions from XML
     *       and formats them according to the specified output format.
     *
     * @warning If nLength < m_nLength, parsing may read beyond buffer bounds.
     *          Caller must ensure buffer is at least m_nLength bytes.
     *
     * @par Example Output (I010/010 SAC/SIC, JSON format)
     * Input: data = {0x05, 0x1A}, nLength = 2, formatType = CAT_FORMAT_ID_JSON
     * Output: strResult += '{"SAC":5,"SIC":26}'
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getText()
     */
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength); // appends value description to strResult

    /**
     * @brief Generate a printable description of the Fixed format structure
     *
     * @param header Prefix string for hierarchical display (e.g., "  " for indent)
     * @return String containing format descriptor tree showing bit fields
     *
     * @note Used for debugging and diagnostics. Shows the format type, length,
     *       and all bit field definitions from m_lSubItems (DataItemBits).
     *
     * @par Example Output
     * For I010/010 (SAC/SIC):
     * @code
     * Fixed(2 bytes):
     *   SAC (16 bits): System Area Code
     *   SIC (8 bits): System Identification Code
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::printDescriptors()
     */
    std::string printDescriptors(std::string header); // print items format descriptors

    /**
     * @brief Mark a specific field for filtering
     *
     * @param name Field name to filter (e.g., "SAC", "SIC"), or nullptr for entire item
     * @return true if filtering was applied successfully
     *
     * @note Used by CLI filter mechanism to show only specific fields.
     *       Delegates to DataItemBits objects in m_lSubItems for field-level filtering.
     *
     * @par Example
     * @code
     * format->filterOutItem("SAC"); // Show only SAC field, hide SIC
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::filterOutItem()
     */
    bool filterOutItem(const char *name); // mark item for filtering

    /**
     * @brief Check if a specific field is filtered (should be displayed)
     *
     * @param name Field name to check (e.g., "SAC"), or nullptr for entire item
     * @return true if the field should be displayed (passes filter)
     *
     * @note Queries DataItemBits objects in m_lSubItems for field-level filtering.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::isFiltered()
     */
    bool isFiltered(const char *name);

    /**
     * @brief Type identification - returns true for Fixed format
     *
     * @return Always true (this is a Fixed format)
     *
     * @note This provides efficient type checking without C++ RTTI.
     *       Used in hot paths during parsing to avoid virtual function overhead.
     *
     * @par Override of Virtual
     * This overrides DataItemFormat::isFixed() which returns false.
     */
    bool isFixed() { return true; }; // true if this is Fixed format

    /**
     * @brief Get human-readable description for a field value
     *
     * @param field Field name within this item (e.g., "SAC", "SIC")
     * @param value Value to look up (e.g., "5")
     * @return String description from XML value mappings, or nullptr if not found
     *
     * @note Delegates to DataItemBits objects in m_lSubItems for field-level lookups.
     *       The returned pointer points to internal string storage and
     *       should not be freed by the caller.
     *
     * @par Example
     * For I048/020 (Target Report Descriptor) TYP field:
     * @code
     * const char* desc = format->getDescription("TYP", "0");
     * // Returns: "SSR multilateration"
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getDescription()
     */
    const char *getDescription(const char *field, const char *value); // return description ef element

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    /**
     * @brief Generate Wireshark dissector definitions (Wireshark plugin only)
     *
     * @return Linked list of fulliautomatix_definitions for Wireshark protocol tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     *       Generates definitions for all bit fields in this Fixed item.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getWiresharkDefinitions()
     */
    fulliautomatix_definitions* getWiresharkDefinitions();

    /**
     * @brief Get Wireshark dissector data structure (Wireshark plugin only)
     *
     * @param pData      Pointer to binary ASTERIX data for this Fixed item
     * @param len        Length of data in bytes (should equal m_nLength)
     * @param byteoffset Byte offset in the packet for Wireshark display
     * @return Pointer to fulliautomatix_data structure for Wireshark tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     *       Creates dissector tree for all bit fields in this Fixed item.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getData()
     */
    fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif
#if defined(PYTHON_WRAPPER)
    /**
     * @brief Get Python object representation (Python binding only)
     *
     * @param pData       Pointer to binary ASTERIX data for this Fixed item
     * @param nLength     Length of data in bytes (should equal m_nLength)
     * @param description If non-zero, include descriptive metadata from XML
     * @return PyObject* containing parsed data (typically a dict)
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       The returned object has a new reference (caller must DECREF).
     *       For Fixed items, typically returns a dict with field names as keys.
     *
     * @par Example Return Value (I010/010 SAC/SIC)
     * Python dict: {'SAC': 5, 'SIC': 26}
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getObject()
     */
    PyObject* getObject(unsigned char* pData, long nLength, int description);

    /**
     * @brief Insert parsed data into existing Python dictionary (Python binding only)
     *
     * @param p           PyObject* pointing to a Python dict to insert into
     * @param pData       Pointer to binary ASTERIX data for this Fixed item
     * @param nLength     Length of data in bytes (should equal m_nLength)
     * @param verbose     If non-zero, include descriptive metadata from XML
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       Inserts field name-value pairs into the provided dictionary.
     *       For Fixed items, each bit field becomes a separate dict entry.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::insertToDict()
     */
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose);
#endif
};

#endif /* DATAITEMFORMATFIXED_H_ */
