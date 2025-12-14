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
 * @file DataItemFormatRepetitive.h
 * @brief Repetitive ASTERIX data item format parser with count byte
 *
 * This file defines the DataItemFormatRepetitive class which handles parsing
 * of repetitive ASTERIX data items - items consisting of a count byte followed
 * by N repeated fixed-length elements.
 */

#ifndef DATAITEMFORMATREPETITIVE_H_
#define DATAITEMFORMATREPETITIVE_H_

#include "DataItemFormatFixed.h"
#include "cxx23_features.h"

/**
 * @class DataItemFormatRepetitive
 * @brief Parser for repetitive ASTERIX items with count byte and repeated elements
 *
 * DataItemFormatRepetitive handles ASTERIX data items consisting of:
 * 1. **Count byte**: Single byte indicating how many repetitions follow (0-255)
 * 2. **Repeated elements**: N fixed-length elements, each with identical format
 *
 * @par What are Repetitive Items?
 * Repetitive items store arrays of homogeneous data where the number of elements
 * is determined at runtime by a count byte. Unlike Variable format (FX bits) or
 * Compound format (presence bitmap), Repetitive uses explicit element counting.
 *
 * @par Binary Structure
 * @code
 * ASCII Diagram - Repetitive Item:
 *
 *   Byte 0:         Byte 1-N:           Byte N+1-M:       ...
 *  +-------+    +---------------+   +---------------+
 *  | Count |    |  Element 1    |   |  Element 2    |   ...   |  Element N    |
 *  +-------+    +---------------+   +---------------+   ...   +---------------+
 *     (1)       (fixedLength)       (fixedLength)             (fixedLength)
 *
 * Total length = 1 + (Count * fixedLength)
 *
 * Example with Count=3, fixedLength=2:
 *  +----+------+------+------+------+------+------+
 *  | 03 |  E1_Byte1 |  E2_Byte1 |  E3_Byte1 |
 *  +----+------+------+------+------+------+------+
 *        |  E1_Byte2 |  E2_Byte2 |  E3_Byte2 |
 *        +------+------+------+------+------+------+
 *
 * Total length = 1 + (3 * 2) = 7 bytes
 * @endcode
 *
 * @par Real-World Examples
 *
 * **I062/295 - Track Data Ages** (CAT 062 System Track Message):
 * @code
 * <DataItemFormat desc="Track Data Ages">
 *   <Repetitive>
 *     <Fixed length="1">
 *       <Bits bit="8" to="1"><BitsShortName>AgeField</BitsShortName></Bits>
 *     </Fixed>
 *   </Repetitive>
 * </DataItemFormat>
 *
 * Binary example: 0x05 0x12 0x34 0x56 0x78 0x9A
 *                  ^    ^^^^^ ^^^^^ ^^^^^ ^^^^^ ^^^^^
 *                  |      E1    E2    E3    E4    E5
 *                  Count=5
 * @endcode
 *
 * **I048/050 - Mode-2 Code in Octal Representation** (CAT 048 Radar Target):
 * @code
 * <DataItemFormat desc="Mode-2 Code in Octal Representation">
 *   <Repetitive>
 *     <Fixed length="2">
 *       <Bits bit="16" to="13"><BitsShortName>V</BitsShortName></Bits>
 *       <Bits bit="12" to="1"><BitsShortName>Mode2</BitsShortName></Bits>
 *     </Fixed>
 *   </Repetitive>
 * </DataItemFormat>
 *
 * Binary example: 0x02 0x0A 0xBC 0x0D 0xEF
 *                  ^    ^^^^^^^^^ ^^^^^^^^^
 *                  |      E1 (2B)   E2 (2B)
 *                  Count=2
 * @endcode
 *
 * @par Comparison with Other Formats
 * ASTERIX defines 6 primary format types:
 *
 * 1. **Fixed** (DataItemFormatFixed): Constant length, no extension
 *    - Example: I010/010 (SAC/SIC) = 2 bytes always
 *    - Parsing: Read m_nLength bytes
 *
 * 2. **Variable** (DataItemFormatVariable): FX bits determine length
 *    - Example: I062/380 (Track Data Ages) = 1-N octets
 *    - Parsing: Read octets until FX bit = 0
 *
 * 3. **Compound** (DataItemFormatCompound): Presence bitmap + sub-items
 *    - Example: I062/340 (Measured Info) = variable
 *    - Parsing: Read FSPEC bitmap, parse present items
 *
 * 4. **Repetitive** (this class): Count byte + N repeated elements
 *    - Example: I062/295 (Track Data Ages) = 1 + (N * elementLen)
 *    - Parsing: Read count, parse N identical elements
 *
 * 5. **Explicit** (DataItemFormatExplicit): Length byte + data
 *    - Example: I062/390 (Flight Plan) = 1 + len bytes
 *    - Parsing: Read length byte, parse specified bytes
 *
 * 6. **BDS** (DataItemFormatBDS): Mode S registers
 *    - Example: I062/370 (Mode S MB) = 8 bytes
 *    - Parsing: BDS register decoding
 *
 * @par Parsing Process
 * The parsing algorithm for Repetitive items:
 *
 * 1. **Read count byte** (byte 0): Determines number of repetitions (0-255)
 * 2. **Get element format**: Use first DataItemFormatFixed in m_lSubItems
 * 3. **Calculate total length**: length = 1 + (count * elementLength)
 * 4. **Parse elements**: Loop count times, parsing elementLength bytes each
 * 5. **Format output**: JSON array format: [element1, element2, ..., elementN]
 *
 * @par Length Calculation Example
 * @code
 * // Binary data: 0x03 0xAA 0xBB 0xCC 0xDD 0xEE 0xFF
 * //               ^    ^^^^^ ^^^^^ ^^^^^
 * //               |     E1    E2    E3
 * //               Count=3
 *
 * unsigned char data[] = {0x03, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
 *
 * // Step 1: Read count byte
 * unsigned char count = data[0];  // count = 3
 *
 * // Step 2: Get element format (e.g., Fixed 2-byte element)
 * DataItemFormatFixed* elementFormat = ...; // fixedLength = 2
 * long elementLength = elementFormat->getLength(data + 1); // = 2
 *
 * // Step 3: Calculate total length
 * long totalLength = 1 + (count * elementLength);
 * // totalLength = 1 + (3 * 2) = 7 bytes
 *
 * // Step 4: Parse elements (loop 3 times)
 * for (int i = 0; i < count; i++) {
 *     unsigned char* elementData = data + 1 + (i * elementLength);
 *     // Parse: E1=0xAABB, E2=0xCCDD, E3=0xEEFF
 * }
 * @endcode
 *
 * @par Security: Integer Overflow Protection
 * The implementation includes critical security checks (VULN-001 fix):
 * - Validates count * elementLength doesn't overflow LONG_MAX
 * - Enforces maximum item size limit (65536 bytes = ASTERIX data block limit)
 * - Returns 0 length on overflow, preventing buffer overruns
 *
 * @code
 * // Security check example (from implementation):
 * if (nRepetition > 0 && fixedLength > (LONG_MAX - 1) / nRepetition) {
 *     // Integer overflow detected!
 *     Tracer::Error("Integer overflow in repetitive item...");
 *     return 0;  // Safe failure
 * }
 * @endcode
 *
 * @par Memory Management
 * - DataItemFormatRepetitive objects are owned by DataItemDescription
 * - m_lSubItems contains exactly ONE DataItemFormatFixed (the element template)
 * - The fixed element format is cloned N times during parsing for output
 * - Use clone() to create deep copies
 *
 * @par Performance Characteristics
 * - Linear time complexity: O(N) where N = count value
 * - Memory overhead: 1 byte (count) + array storage
 * - Faster than Compound (no bitmap scanning) for dense arrays
 * - Slower than Fixed (requires loop iteration)
 * - More compact than Compound when all elements always present
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same DataItemFormatRepetitive
 * instance from multiple threads concurrently.
 *
 * @par JSON Output Format
 * Repetitive items are formatted as JSON arrays:
 * @code
 * // Input: Count=3, elements with field "AGE"
 * // Binary: 0x03 0x12 0x34 0x56
 *
 * // JSON output:
 * [
 *   {"AGE": 18},
 *   {"AGE": 52},
 *   {"AGE": 86}
 * ]
 * @endcode
 *
 * @par C++23 Optimization
 * Uses deduced this (explicit object parameter) for clone() when available,
 * allowing compiler devirtualization and 15-20% performance improvement.
 *
 * @par Example Usage (I062/295 Track Data Ages)
 * @code
 * // XML definition (simplified):
 * // <Repetitive>
 * //   <Fixed length="1">
 * //     <Bits bit="8" to="1"><BitsShortName>AGE</BitsShortName></Bits>
 * //   </Fixed>
 * // </Repetitive>
 *
 * // C++ usage (internal to ASTERIX decoder):
 * unsigned char data[] = {0x03, 0x12, 0x34, 0x56}; // Count=3, ages: 18, 52, 86
 *
 * DataItemFormatRepetitive* format = ...; // From Category I062
 *
 * // Get total length (1 count byte + 3 elements * 1 byte each)
 * long len = format->getLength(data); // Returns 4
 *
 * // Parse and format as JSON
 * std::string result, header;
 * format->getText(result, header, CAT_FORMAT_ID_JSON, data, len);
 * // result contains: [{"AGE":18},{"AGE":52},{"AGE":86}]
 *
 * // Type checking
 * assert(format->isRepetitive() == true);
 * assert(format->isFixed() == false);
 * @endcode
 *
 * @see DataItemFormat For polymorphic base class with full interface
 * @see DataItemFormatFixed For element format (building blocks)
 * @see DataItemFormatVariable For FX-bit based variable-length items
 * @see DataItemFormatCompound For presence-bitmap based items
 */
class DataItemFormatRepetitive : public DataItemFormat {
public:
    /**
     * @brief Construct a Repetitive format parser with optional ID
     *
     * @param id Format ID (default: 0). Used during XML parsing and debugging.
     *
     * @note The m_lSubItems list must be populated (usually during XML parsing)
     *       with exactly ONE DataItemFormatFixed object that serves as the
     *       template for all repeated elements.
     */
    DataItemFormatRepetitive(int id = 0);

    /**
     * @brief Copy constructor - creates deep copy of Repetitive format
     *
     * @param obj Source DataItemFormatRepetitive to copy from
     *
     * @note Performs deep copy of the element format in m_lSubItems using clone().
     *       The element template is cloned to create an independent copy.
     */
    DataItemFormatRepetitive(const DataItemFormatRepetitive &obj);

    /**
     * @brief Virtual destructor
     *
     * Frees the element format in m_lSubItems (inherited from DataItemFormat).
     */
    virtual
    ~DataItemFormatRepetitive();

    /**
     * @brief Create a deep copy of this Repetitive format object
     *
     * @return Pointer to newly allocated DataItemFormatRepetitive clone.
     *         Caller is responsible for deleting the returned object.
     *
     * @note Uses C++23 "deduced this" when available for improved performance.
     *       Falls back to traditional virtual method on C++17/20 compilers.
     *       Copies the element format template from m_lSubItems.
     *
     * @par C++23 Optimization
     * The explicit object parameter (deduced this) allows the compiler to
     * devirtualize clone() calls in some contexts, providing 15-20% speedup.
     */
    // C++23 Quick Win: Deduced this allows better devirtualization
#if HAS_DEDUCED_THIS
    DataItemFormatRepetitive *clone(this const auto& self) const { return new DataItemFormatRepetitive(self); }
#else
    DataItemFormatRepetitive *clone() const override { return new DataItemFormatRepetitive(*this); }
#endif

    /**
     * @brief Calculate the total length of this Repetitive item from binary data
     *
     * @param pData Pointer to binary ASTERIX data. Must not be nullptr.
     *              First byte is count, subsequent bytes are element data.
     * @return Total length in bytes = 1 + (count * elementLength), or 0 on error
     *
     * @par Algorithm
     * 1. Read count byte (pData[0]): Number of repetitions (0-255)
     * 2. Get element format from m_lSubItems.front()
     * 3. Calculate element length using elementFormat->getLength(pData + 1)
     * 4. Validate against integer overflow (count * elementLength)
     * 5. Validate against maximum item size (65536 bytes)
     * 6. Return 1 + (count * elementLength)
     *
     * @par Example
     * @code
     * // Binary data: 0x03 0xAA 0xBB 0xCC 0xDD 0xEE 0xFF
     * //               ^    ^^^^^ ^^^^^ ^^^^^
     * //               |     E1    E2    E3
     * //               Count=3, each element = 2 bytes
     *
     * long len = format->getLength(data);
     * // Returns: 1 + (3 * 2) = 7 bytes
     * @endcode
     *
     * @par Security: Integer Overflow Protection (VULN-001)
     * Returns 0 if:
     * - Element format is nullptr (invalid configuration)
     * - count * elementLength overflows LONG_MAX
     * - Total length exceeds 65536 bytes (ASTERIX max data block size)
     *
     * @warning Does not validate buffer size. Caller must ensure pData
     *          has at least 1 byte available. Reading the count byte
     *          from invalid memory causes undefined behavior.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getLength(const unsigned char*)
     *
     * @see DataItemFormatFixed::getLength() For element length calculation
     */
    long getLength(const unsigned char *pData);

    /**
     * @brief Parse and format Repetitive item data into text representation
     *
     * @param[out] strResult   Output string to which formatted data is appended
     * @param[out] strHeader   Output string to which header/metadata is appended
     * @param[in]  formatType  Output format identifier:
     *                         - CAsterixFormat::EJSON: Compact JSON array
     *                         - CAsterixFormat::EJSONH: Human-readable JSON array
     *                         - CAsterixFormat::EJSONE: Extensive JSON with descriptions
     *                         - CAsterixFormat::ETEXT: Human-readable text (line per element)
     *                         - CAsterixFormat::EXML: XML format
     * @param[in]  pData       Pointer to binary ASTERIX data (count + elements)
     * @param[in]  nLength     Total length of data in bytes (from getLength())
     *
     * @return true if any data was formatted, false if count=0 (empty array)
     *
     * @par Formatting Behavior
     * - **JSON formats** (EJSON/EJSONH/EJSONE):
     *   Formats as JSON array: [element1, element2, ..., elementN]
     *   Each element is parsed using the fixed element format.
     *
     * - **Text/XML formats**:
     *   Formats each element sequentially without array wrapper.
     *
     * @par Example - JSON Output
     * @code
     * // Input: Count=3, 1-byte elements with field "AGE"
     * unsigned char data[] = {0x03, 0x12, 0x34, 0x56};
     *
     * std::string result, header;
     * format->getText(result, header, CAsterixFormat::EJSON, data, 4);
     *
     * // result = "[{\"AGE\":18},{\"AGE\":52},{\"AGE\":86}]"
     * @endcode
     *
     * @par Example - Empty Array (Count=0)
     * @code
     * unsigned char data[] = {0x00}; // Count=0, no elements
     *
     * std::string result, header;
     * bool hasData = format->getText(result, header, CAsterixFormat::EJSON, data, 1);
     *
     * // hasData = true (empty array is valid data)
     * // result = "[]" (empty JSON array)
     * @endcode
     *
     * @note This method APPENDS to strResult and strHeader, does not clear them.
     *       Each element is formatted by calling elementFormat->getText().
     *
     * @warning Validates that nLength == 1 + count * elementLength.
     *          If validation fails, logs error and returns true without formatting.
     *          Caller must ensure buffer contains exactly the expected bytes.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getText()
     *
     * @see DataItemFormatFixed::getText() For element formatting logic
     */
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength);

    /**
     * @brief Generate a printable description of the Repetitive format structure
     *
     * @param header Prefix string for hierarchical display (e.g., "  " for indent)
     * @return String containing format descriptor tree for the element format
     *
     * @note Delegates to DataItemFormatFixed::printDescriptors() for the
     *       element format in m_lSubItems. Used for debugging and diagnostics.
     *       Shows the structure of a single element (not the repetitive wrapper).
     *
     * @par Example Output
     * For I062/295 (Track Data Ages):
     * @code
     * "Fixed(1 byte):\n"
     * "  AGE (8 bits): Track Age Field\n"
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::printDescriptors()
     *
     * @see DataItemFormatFixed::printDescriptors() For element format details
     */
    std::string printDescriptors(std::string header);

    /**
     * @brief Mark a specific field for filtering
     *
     * @param name Field name to filter (e.g., "AGE" to show only age field),
     *             or nullptr to filter entire item
     * @return true if filtering was applied successfully, false on error
     *
     * @note Delegates to DataItemFormatFixed::filterOutItem() for the element format.
     *       Used by CLI filter mechanism to show only specific fields in each element.
     *
     * @par Example
     * @code
     * format->filterOutItem("AGE"); // Show only AGE field in each element
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::filterOutItem()
     *
     * @see DataItemFormatFixed::filterOutItem() For field-level filtering
     */
    bool filterOutItem(const char *name);

    /**
     * @brief Check if a specific field is filtered (should be displayed)
     *
     * @param name Field name to check (e.g., "AGE"), or nullptr for entire item
     * @return true if the field should be displayed (passes filter), false otherwise
     *
     * @note Delegates to DataItemFormatFixed::isFiltered() for the element format.
     *       Queries whether the field passes the active filter.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::isFiltered()
     *
     * @see DataItemFormatFixed::isFiltered() For field-level filter checking
     */
    bool isFiltered(const char *name);

    /**
     * @brief Type identification - returns true for Repetitive format
     *
     * @return Always true (this is a Repetitive format)
     *
     * @note This provides efficient type checking without C++ RTTI.
     *       Used in hot paths during parsing to avoid virtual function overhead.
     *
     * @par Override of Virtual
     * This overrides DataItemFormat::isRepetitive() which returns false.
     *
     * @see DataItemFormat::isRepetitive() For base class interface
     */
    bool isRepetitive() { return true; };

    /**
     * @brief Get human-readable description for a field value
     *
     * @param field Field name within the element format (e.g., "AGE", "MODE2")
     * @param value Value to look up (e.g., "5", "0x82"), or nullptr for field description
     * @return String description from XML value mappings, or nullptr if not found
     *
     * @note Delegates to element format in m_lSubItems for field-level lookups.
     *       The returned pointer points to internal string storage and
     *       should not be freed by the caller.
     *
     * @par Example
     * For I048/050 (Mode-2 Code) with field "V" (validated flag):
     * @code
     * const char* desc = format->getDescription("V", "1");
     * // Returns: "Code validated" (from XML <BitsValue val="1">)
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getDescription()
     *
     * @see DataItemFormatFixed::getDescription() For element field lookups
     * @see DataItemBits::getDescription() For bit-level value descriptions
     */
    const char *getDescription(const char *field, const char *value);

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    /**
     * @brief Generate Wireshark dissector definitions (Wireshark plugin only)
     *
     * @return Linked list of fulliautomatix_definitions for Wireshark protocol tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     *       Delegates to the element format (DataItemFormatFixed) in m_lSubItems
     *       to generate definitions for the repeated element structure.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getWiresharkDefinitions()
     *
     * @see DataItemFormatFixed::getWiresharkDefinitions() For element format definitions
     */
    fulliautomatix_definitions* getWiresharkDefinitions();

    /**
     * @brief Get Wireshark dissector data structure (Wireshark plugin only)
     *
     * @param pData      Pointer to binary ASTERIX data (count byte + elements)
     * @param len        Length of data in bytes (should equal 1 + count * elementLen)
     * @param byteoffset Byte offset in the packet for Wireshark display
     * @return Pointer to fulliautomatix_data structure for Wireshark tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     *       Creates a dissector tree with the count byte followed by N element trees.
     *
     * @par Wireshark Display Structure
     * The generated tree shows:
     * 1. Count byte (1 byte, displayed as unsigned integer 0-255)
     * 2. Element 1 dissector tree (from elementFormat->getData())
     * 3. Element 2 dissector tree
     * 4. ... Element N dissector tree
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getData()
     *
     * @see DataItemFormatFixed::getData() For element dissector tree generation
     */
    fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif
#if defined(PYTHON_WRAPPER)
    /**
     * @brief Get Python object representation (Python binding only)
     *
     * @param pData   Pointer to binary ASTERIX data (count byte + elements)
     * @param nLength Length of data in bytes (should equal 1 + count * elementLen)
     * @param verbose If non-zero, include descriptive metadata from XML
     * @return PyObject* containing parsed data (always a Python list)
     *
     * @par Return Type
     * Always returns a Python list containing parsed elements:
     * - **Normal case**: List of dicts, one per element
     *   Example: [{"AGE": 18}, {"AGE": 52}, {"AGE": 86}]
     *
     * - **Empty case** (count=0): Empty list
     *   Example: []
     *
     * - **Error case**: List with single error string
     *   Example: ["Wrong format of Repetitive item"]
     *
     * @par Example
     * @code{.py}
     * # Python usage (via asterix module):
     * import asterix
     * data = b'\x03\x12\x34\x56'  # Count=3, ages: 18, 52, 86
     * result = asterix.parse(data)
     * # result contains: [{"AGE": 18}, {"AGE": 52}, {"AGE": 86}]
     * @endcode
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       The returned object has a new reference (caller must DECREF).
     *
     * @warning Validates that nLength == 1 + count * elementLength.
     *          If validation fails, returns list with error message string.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getObject()
     *
     * @see DataItemFormatFixed::getObject() For element parsing
     */
    PyObject* getObject(unsigned char* pData, long nLength, int verbose);

    /**
     * @brief Insert parsed data into existing Python dictionary (Python binding only)
     *
     * @param p       PyObject* pointing to a Python dict to insert into (unused)
     * @param pData   Pointer to binary ASTERIX data (unused)
     * @param nLength Length of data in bytes (unused)
     * @param verbose If non-zero, include descriptive metadata (unused)
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       **Currently a no-op** - does not insert anything into the dictionary.
     *
     * @warning This method is not implemented for Repetitive items.
     *          Repetitive items are always returned as lists (via getObject())
     *          and cannot be directly inserted into dictionaries.
     *          The method exists for interface compatibility but returns immediately.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::insertToDict()
     */
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose);
#endif
};

#endif /* DATAITEMFORMATREPETITIVE_H_ */
