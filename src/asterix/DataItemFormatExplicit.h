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
 * @file DataItemFormatExplicit.h
 * @brief Explicit-length ASTERIX data item format parser
 *
 * This file defines the DataItemFormatExplicit class which handles parsing
 * of ASTERIX data items with an explicit length indicator - items that begin
 * with a length byte followed by the specified number of payload bytes.
 */

#ifndef DATAITEMFORMATEXPLICIT_H_
#define DATAITEMFORMATEXPLICIT_H_

#include "DataItemFormat.h"
#include "DataItemFormatFixed.h"
#include "cxx23_features.h"

/**
 * @class DataItemFormatExplicit
 * @brief Parser for ASTERIX data items with explicit length indicator
 *
 * DataItemFormatExplicit handles ASTERIX data items that begin with an explicit
 * length byte followed by payload data of the specified length. This format is
 * used for user-defined data fields, reserved expansion fields, and special
 * purpose fields where the data structure is not predefined in the specification.
 *
 * @par What are Explicit-Length Items?
 * Explicit items are self-describing data fields used when:
 * - The data structure is user-defined or implementation-specific
 * - Reserved expansion fields (RE) need to carry custom data
 * - Special purpose fields (SP) carry proprietary information
 * - Forward compatibility requires unknown data to be skipped
 *
 * **Binary Structure:**
 * @code
 * +----------------+------------------+------------------+-----+
 * | Length byte    | Payload byte 1   | Payload byte 2   | ... |
 * | (N bytes)      | (Data)           | (Data)           |     |
 * +----------------+------------------+------------------+-----+
 *  <--- 1 byte --> <------------- N bytes ----------------->
 * @endcode
 *
 * The length byte specifies the **total** length of the item including the
 * length byte itself. Therefore, the payload is (length - 1) bytes.
 *
 * **Common Examples:**
 * - **I048/RE (Reserved Expansion Field)**: Variable-length user-defined data
 * - **I048/SP (Special Purpose Field)**: Variable-length proprietary data
 * - **I062/RE (Reserved Expansion Field)**: Extension fields for future use
 *
 * @par Real-World Example (I048/RE Reserved Expansion)
 * @code{.xml}
 * <DataItem id="RE">
 *   <DataItemName>Reserved Expansion</DataItemName>
 *   <DataItemDefinition>Reserved Expansion Field</DataItemDefinition>
 *   <DataItemFormat desc="Explicit data item.">
 *     <Explicit>
 *       <Fixed length="1">
 *         <Bits from="8" to="1">
 *           <BitsShortName>VAL</BitsShortName>
 *         </Bits>
 *       </Fixed>
 *     </Explicit>
 *   </DataItemFormat>
 * </DataItem>
 * @endcode
 *
 * **Binary Example (5 bytes total):**
 * @code
 * Hex:  05 41 42 43 44
 *       ^  ^-----------^
 *       |  Payload (4 bytes): "ABCD"
 *       Length = 5 bytes total
 *
 * Parsing:
 * 1. Read first byte: length = 5
 * 2. Read next (5-1) = 4 bytes: {0x41, 0x42, 0x43, 0x44}
 * 3. Parse payload according to sub-item format (typically Fixed)
 * @endcode
 *
 * @par Comparison with Other Formats
 * ASTERIX defines 6 primary format types:
 *
 * 1. **Fixed** (DataItemFormatFixed): Constant length, no length indicator
 *    - Example: I010/010 (SAC/SIC) = 2 bytes always
 *    - Parsing: Read constant m_nLength bytes
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
 * 5. **Explicit** (this class): Explicit length field + payload
 *    - Example: I048/RE (Reserved Expansion) = 1 + len
 *    - Parsing: Read length byte, then parse (length - 1) bytes
 *
 * 6. **BDS** (DataItemFormatBDS): Mode S Comm-B Data Buffer registers
 *    - Example: I062/370 (Mode S MB Data) = 8 bytes (BDS register)
 *    - Parsing: Special handling for Mode S registers
 *
 * @par Parsing Algorithm
 * The parsing process for Explicit items:
 *
 * 1. **Length Extraction**: getLength() reads the first byte
 *    - Returns the total item length including the length byte itself
 *    - Range: 1-255 bytes (single-byte length indicator)
 *
 * 2. **Payload Extraction**: Skip the length byte, read (length - 1) bytes
 *    - Actual payload is (total_length - 1) bytes
 *
 * 3. **Sub-Item Parsing**: Parse payload according to m_lSubItems
 *    - Typically contains a single Fixed format sub-item
 *    - Sub-item defines the structure of the payload data
 *
 * 4. **Repetition Handling**: If payload is a multiple of sub-item length
 *    - The same sub-item structure repeats multiple times
 *    - Each repetition is parsed and formatted as an array element
 *
 * 5. **Formatting**: Format fields as text/JSON/XML via getText()
 *    - Single occurrence: Direct field output
 *    - Multiple occurrences: Array output in JSON/XML
 *
 * @par Use Cases
 *
 * **1. Reserved Expansion Fields (RE):**
 * Used for future extensions to ASTERIX categories without breaking compatibility.
 * Parsers that don't understand the new fields can skip them using the length.
 *
 * **2. Special Purpose Fields (SP):**
 * Proprietary data exchanged between specific systems. The Explicit format
 * allows arbitrary data to be carried without affecting standard parsers.
 *
 * **3. User-Defined Data:**
 * Custom fields added by implementations for local requirements.
 *
 * **4. Forward Compatibility:**
 * Newer ASTERIX versions can add fields that older parsers safely ignore.
 *
 * @par Memory Management
 * - DataItemFormatExplicit objects are owned by DataItemDescription
 * - m_lSubItems typically contains one Fixed format sub-item defining payload structure
 * - Use clone() to create deep copies
 *
 * @par Performance
 * Explicit format has moderate parsing overhead:
 * - ✓ Length determination is O(1) - single byte read
 * - ✓ No extension bit scanning (vs Variable)
 * - ✓ No presence bitmap processing (vs Compound)
 * - ✗ Requires length byte validation
 * - ✗ Sub-item parsing required for payload
 *
 * Performance is similar to Repetitive format, faster than Compound/Variable.
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same DataItemFormatExplicit
 * instance from multiple threads concurrently.
 *
 * @par Example Usage (I048/RE Reserved Expansion)
 * @code
 * // XML definition (simplified):
 * // <Explicit>
 * //   <Fixed length="1">
 * //     <Bits from="8" to="1"><BitsShortName>VAL</BitsShortName></Bits>
 * //   </Fixed>
 * // </Explicit>
 *
 * // C++ usage (internal to ASTERIX decoder):
 * unsigned char data[5] = {0x05, 0x41, 0x42, 0x43, 0x44};
 * //                        ^     ^-----------------^
 * //                        len   payload (4 bytes)
 *
 * DataItemFormatExplicit* format = ...; // From Category I048
 *
 * // Get total length (reads first byte)
 * long len = format->getLength(data); // Returns 5
 *
 * // Parse and format as JSON
 * std::string result, header;
 * format->getText(result, header, CAT_FORMAT_ID_JSON, data, len);
 * // result contains: {"VAL":[65,66,67,68]}
 * // (Payload parsed as array of bytes: 0x41='A', 0x42='B', etc.)
 *
 * // Type checking
 * assert(format->isExplicit() == true);
 * assert(format->isFixed() == false);
 * @endcode
 *
 * @par Validation and Error Handling
 * The parser validates:
 * - Length byte is present (nLength > 0)
 * - Payload length is at least 1 byte (nLength > 1)
 * - Payload is a multiple of sub-item body length (for repeated structures)
 *
 * Invalid data triggers error logging via Tracer::Error() and returns false.
 *
 * @see DataItemFormat For polymorphic base class
 * @see DataItemFormatFixed For fixed-length items (often used as payload)
 * @see DataItemFormatRepetitive For repetitive items with count byte
 * @see DataItemFormatCompound For compound items with sub-item bitmap
 */
class DataItemFormatExplicit : public DataItemFormat {
public:
    /**
     * @brief Construct an Explicit format parser with optional ID
     *
     * @param id Format ID (default: 0). Used during XML parsing and debugging.
     *
     * @note The m_lSubItems list must be populated separately (during XML parsing)
     *       with the sub-item format(s) that define the payload structure.
     *       Typically, this is a single Fixed format item.
     */
    DataItemFormatExplicit(int id = 0);

    /**
     * @brief Copy constructor - creates deep copy of Explicit format
     *
     * @param obj Source DataItemFormatExplicit to copy from
     *
     * @note Copies all fields including m_lSubItems and m_pParentFormat.
     *       Sub-items in m_lSubItems are deep-copied via clone().
     */
    DataItemFormatExplicit(const DataItemFormatExplicit &obj);

    /**
     * @brief Virtual destructor
     *
     * Frees all sub-item formats in m_lSubItems (inherited from DataItemFormat).
     */
    virtual
    ~DataItemFormatExplicit();

    /**
     * @brief Create a deep copy of this Explicit format object
     *
     * @return Pointer to newly allocated DataItemFormatExplicit clone.
     *         Caller is responsible for deleting the returned object.
     *
     * @note Uses C++23 "deduced this" when available for improved performance.
     *       Falls back to traditional virtual method on C++17/20 compilers.
     *       Copies all fields including m_lSubItems and deep-copies sub-items.
     *
     * @par C++23 Optimization
     * The explicit object parameter (deduced this) allows the compiler to
     * devirtualize clone() calls in some contexts, providing 15-20% speedup.
     */
    // C++23 Quick Win: Deduced this allows better devirtualization
#if HAS_DEDUCED_THIS
    DataItemFormatExplicit *clone(this const auto& self) const { return new DataItemFormatExplicit(self); }
#else
    DataItemFormatExplicit *clone() const override { return new DataItemFormatExplicit(*this); } // Return clone of object
#endif

    /**
     * @brief Extract the total item length from the explicit length byte
     *
     * @param pData Pointer to binary ASTERIX data (first byte is the length)
     * @return Total length in bytes including the length byte itself (1-255)
     *
     * @note This method reads the first byte of pData which contains the total
     *       item length. The actual payload is (returned_value - 1) bytes.
     *
     * @warning The caller must ensure pData points to at least 1 valid byte.
     *          No bounds checking is performed.
     *
     * @par Example
     * @code
     * unsigned char data[5] = {0x05, 0x41, 0x42, 0x43, 0x44};
     * long len = format->getLength(data); // Returns 5
     * // Payload length = 5 - 1 = 4 bytes
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getLength(const unsigned char*)
     */
    long getLength(const unsigned char *pData);

    /**
     * @brief Parse and format Explicit item data into text representation
     *
     * @param[out] strResult   Output string to which formatted data is appended
     * @param[out] strHeader   Output string to which header/metadata is appended
     * @param[in]  formatType  Output format identifier:
     *                         - CAT_FORMAT_ID_TEXT: Human-readable text
     *                         - CAT_FORMAT_ID_JSON: JSON format
     *                         - CAT_FORMAT_ID_XML: XML format
     * @param[in]  pData       Pointer to binary ASTERIX data (starts with length byte)
     * @param[in]  nLength     Total length of data in bytes (should match first byte)
     *
     * @return true if formatting succeeded, false on error
     *
     * @note This method APPENDS to strResult and strHeader, does not clear them.
     *       The parsing process:
     *       1. Skips the length byte (first byte of pData)
     *       2. Calculates sub-item body length from m_lSubItems
     *       3. Parses payload as repetitions of the sub-item structure
     *       4. Formats as array if multiple repetitions, single object otherwise
     *
     * @warning If nLength <= 1, returns false and logs error (no payload present).
     *          If payload length is not a multiple of sub-item body length,
     *          returns false and logs error (malformed data).
     *
     * @par Example Output (I048/RE with 4-byte payload, JSON format)
     * Input: data = {0x05, 0x41, 0x42, 0x43, 0x44}, nLength = 5
     * Output: strResult += '{"VAL":[65,66,67,68]}'
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getText()
     */
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength); // appends value description to strResult

    /**
     * @brief Generate a printable description of the Explicit format structure
     *
     * @param header Prefix string for hierarchical display (e.g., "  " for indent)
     * @return String containing format descriptor tree showing payload structure
     *
     * @note Used for debugging and diagnostics. Shows the format type (Explicit)
     *       and the sub-item format definitions from m_lSubItems.
     *
     * @par Example Output
     * For I048/RE (Reserved Expansion):
     * @code
     * Explicit:
     *   Fixed(1 byte):
     *     VAL (8 bits): Value
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::printDescriptors()
     */
    std::string printDescriptors(std::string header); // print items format descriptors

    /**
     * @brief Mark a specific field for filtering
     *
     * @param name Field name to filter (e.g., "VAL"), or nullptr for entire item
     * @return true if filtering was applied successfully
     *
     * @note Used by CLI filter mechanism to show only specific fields.
     *       Delegates to sub-item formats in m_lSubItems for field-level filtering.
     *
     * @par Example
     * @code
     * format->filterOutItem("VAL"); // Show only VAL field
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::filterOutItem()
     */
    bool filterOutItem(const char *name); // mark item for filtering

    /**
     * @brief Check if a specific field is filtered (should be displayed)
     *
     * @param name Field name to check (e.g., "VAL"), or nullptr for entire item
     * @return true if the field should be displayed (passes filter)
     *
     * @note Queries sub-item formats in m_lSubItems for field-level filtering.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::isFiltered()
     */
    bool isFiltered(const char *name);

    /**
     * @brief Type identification - returns true for Explicit format
     *
     * @return Always true (this is an Explicit format)
     *
     * @note This provides efficient type checking without C++ RTTI.
     *       Used in hot paths during parsing to avoid virtual function overhead.
     *
     * @par Override of Virtual
     * This overrides DataItemFormat::isExplicit() which returns false.
     */
    bool isExplicit() const override { return true; }

    /**
     * @brief Get human-readable description for a field value
     *
     * @param field Field name within the payload (e.g., "VAL")
     * @param value Value to look up (e.g., "65")
     * @return String description from XML value mappings, or nullptr if not found
     *
     * @note Delegates to sub-item formats in m_lSubItems for field-level lookups.
     *       The returned pointer points to internal string storage and
     *       should not be freed by the caller.
     *
     * @par Example
     * For I048/RE with enumerated VAL field:
     * @code
     * const char* desc = format->getDescription("VAL", "0");
     * // Returns description from XML if defined, nullptr otherwise
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
     *       Generates definitions for payload sub-items via m_lSubItems.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getWiresharkDefinitions()
     */
    fulliautomatix_definitions* getWiresharkDefinitions();

    /**
     * @brief Get Wireshark dissector data structure (Wireshark plugin only)
     *
     * @param pData      Pointer to binary ASTERIX data (starts with length byte)
     * @param len        Total length of data in bytes (should match first byte)
     * @param byteoffset Byte offset in the packet for Wireshark display
     * @return Pointer to fulliautomatix_data structure for Wireshark tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     *       Creates dissector tree for payload sub-items via m_lSubItems.
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
     * @param pData       Pointer to binary ASTERIX data (starts with length byte)
     * @param nLength     Total length of data in bytes (should match first byte)
     * @param description If non-zero, include descriptive metadata from XML
     * @return PyObject* containing parsed data (typically a dict or list)
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       The returned object has a new reference (caller must DECREF).
     *       For Explicit items with single occurrence, returns a dict.
     *       For multiple repetitions, returns a list of dicts.
     *
     * @par Example Return Value (I048/RE with 4-byte payload)
     * Python dict: {'VAL': [65, 66, 67, 68]}
     * (If multiple repetitions: list of dicts)
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getObject()
     */
    PyObject* getObject(unsigned char* pData, long nLength, int description);

    /**
     * @brief Insert parsed data into existing Python dictionary (Python binding only)
     *
     * @param p           PyObject* pointing to a Python dict to insert into
     * @param pData       Pointer to binary ASTERIX data (starts with length byte)
     * @param nLength     Total length of data in bytes (should match first byte)
     * @param description If non-zero, include descriptive metadata from XML
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       Inserts field name-value pairs from payload into the provided dictionary.
     *       For Explicit items, parses payload according to m_lSubItems format.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::insertToDict()
     */
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int description);
#endif
};

#endif /* DATAITEMFORMATEXPLICIT_H_ */
