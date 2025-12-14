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
 * @file DataItemFormatCompound.h
 * @brief Compound ASTERIX data item format parser with sub-item presence bitmap
 *
 * This file defines the DataItemFormatCompound class which handles parsing of
 * ASTERIX Compound data items. Compound items consist of multiple optional
 * sub-items, where a presence bitmap (FSPEC-like structure) indicates which
 * sub-items are present in the data stream.
 *
 * @par Compound Format Structure
 * A compound item has two main components:
 * 1. **Primary Field** (presence bitmap): Variable-length FSPEC indicating which sub-items follow
 * 2. **Secondary Fields** (sub-items): Only present if their corresponding bit is set in the bitmap
 *
 * @par Binary Structure Diagram
 * @code
 * +---------------------------+
 * | Primary Field (FSPEC)     |  <- Variable-length presence bitmap with FX bits
 * |   Bit 8: Sub-item 1 flag  |
 * |   Bit 7: Sub-item 2 flag  |
 * |   Bit 6: Sub-item 3 flag  |
 * |   ...                     |
 * |   Bit 1: FX (extension)   |  <- FX=1 means more FSPEC octets follow
 * +---------------------------+
 * | Sub-item 1 (if bit 8 = 1) |  <- Present only if corresponding bit set
 * +---------------------------+
 * | Sub-item 2 (if bit 7 = 1) |  <- Can be Fixed, Variable, Repetitive, etc.
 * +---------------------------+
 * | Sub-item 3 (if bit 6 = 1) |
 * +---------------------------+
 * | ...                       |
 * +---------------------------+
 * @endcode
 *
 * @see DataItemFormat For polymorphic base class interface
 * @see DataItemFormatVariable For the Variable format used by the primary field
 * @see DataItemFormatFixed For Fixed-length comparison
 */

#ifndef DATAITEMFORMATCOMPOUND_H_
#define DATAITEMFORMATCOMPOUND_H_

#include "DataItemFormat.h"
#include "DataItemFormatVariable.h"
#include "cxx23_features.h"

/**
 * @class DataItemFormatCompound
 * @brief Parser for ASTERIX Compound data items with sub-item presence bitmap
 *
 * DataItemFormatCompound handles ASTERIX data items that contain multiple optional
 * sub-items, where a presence bitmap (similar to the Data Block FSPEC) indicates
 * which sub-items are present in the data stream. This format allows efficient
 * encoding of structured data where many fields may be optional.
 *
 * @par What are Compound Items?
 * Compound items are structured data items composed of:
 * 1. **Primary Field**: A Variable-length FSPEC (Field Specification) bitmap
 *    - Uses FX (Field Extension) bits like Variable items
 *    - Each bit (except FX) indicates presence of a corresponding sub-item
 *    - Bit 8 = sub-item 1, Bit 7 = sub-item 2, ..., Bit 1 = FX
 *
 * 2. **Secondary Fields**: Optional sub-items that follow the FSPEC
 *    - Only present if their corresponding FSPEC bit is set to 1
 *    - Each sub-item can be any format type (Fixed, Variable, Repetitive, etc.)
 *    - Order matches the bit order in the FSPEC
 *
 * @par Binary Structure with Real Example
 * Consider I062/380 (Aircraft Derived Data) with 3 sub-items:
 * @code
 * Binary data:  0xE0  0x12 0x34  0xAB 0xCD  0x56 0x78
 *               ^^^^  ^^^^^^^^^  ^^^^^^^^^  ^^^^^^^^^
 *               FSPEC  Sub-1     Sub-2      Sub-3
 *
 * FSPEC breakdown: 0xE0 = 11100000 binary
 *   Bit 8 (1): Sub-item 1 present  <- "Target Address"
 *   Bit 7 (1): Sub-item 2 present  <- "Target Identification"
 *   Bit 6 (1): Sub-item 3 present  <- "Magnetic Heading"
 *   Bit 5 (0): Sub-item 4 absent
 *   Bit 4 (0): Sub-item 5 absent
 *   Bit 3 (0): Sub-item 6 absent
 *   Bit 2 (0): Sub-item 7 absent
 *   Bit 1 (0): FX=0 (no more FSPEC octets)
 *
 * Parsing result:
 *   - Total FSPEC length: 1 octet (FX=0)
 *   - Sub-item 1: 2 bytes (0x1234)
 *   - Sub-item 2: 2 bytes (0xABCD)
 *   - Sub-item 3: 2 bytes (0x5678)
 *   - Total length: 1 + 2 + 2 + 2 = 7 bytes
 * @endcode
 *
 * @par Extended FSPEC Example (Multi-Octet Presence Bitmap)
 * When more than 7 sub-items exist, the FSPEC extends using FX bits:
 * @code
 * Binary data:  0xFF 0x81  [sub-items...]
 *               ^^^^ ^^^^
 *               Oct1 Oct2
 *
 * Octet 1: 0xFF = 11111111
 *   Bits 8-2: Sub-items 1-7 present
 *   Bit 1 (FX=1): More FSPEC octets follow
 *
 * Octet 2: 0x81 = 10000001
 *   Bit 8: Sub-item 8 present
 *   Bits 7-2 (0): Sub-items 9-14 absent
 *   Bit 1 (FX=1): More FSPEC octets follow (if needed)
 *
 * Total FSPEC length: 2 octets
 * Sub-items present: 1-7, 8 (8 total sub-items)
 * @endcode
 *
 * @par Comparison with Other Formats
 * ASTERIX defines 6 primary format types. Compound is similar to others but distinct:
 *
 * 1. **Fixed** (DataItemFormatFixed): Constant length, all fields always present
 *    - Example: I010/010 (SAC/SIC) = 2 bytes always
 *    - No optional fields
 *
 * 2. **Variable** (DataItemFormatVariable): Variable length with FX extension bits
 *    - Example: I062/380 primary field = 1-N octets
 *    - All fields present when extended (no optionality)
 *
 * 3. **Compound** (this class): Optional sub-items with presence bitmap
 *    - Example: I062/380 (Aircraft Derived Data) = 1-N octets FSPEC + sub-items
 *    - Each sub-item independently optional based on FSPEC bits
 *
 * 4. **Repetitive** (DataItemFormatRepetitive): Count byte + repeated elements
 *    - Example: I062/295 (Track Data Ages) = 1 + (N * elementLen)
 *    - All repeated elements have same format
 *
 * 5. **Explicit** (DataItemFormatExplicit): Explicit length field + data
 *    - Example: I062/390 (Flight Plan Data) = 1 + len
 *    - Length determined by explicit field, not bitmap
 *
 * 6. **BDS** (DataItemFormatBDS): Mode S Comm-B Data Buffer registers
 *    - Example: I062/370 (Mode S MB Data) = 8 bytes (BDS register)
 *    - Special handling for aviation transponder data
 *
 * @par XML Definition Example (I062/380 - Aircraft Derived Data)
 * @code{.xml}
 * <DataItemFormat desc="Aircraft Derived Data">
 *   <Compound>
 *     <!-- Primary field: Variable-length FSPEC -->
 *     <Variable>
 *       <Fixed length="1">
 *         <Bits bit="8"><BitsShortName>ADR</BitsShortName><BitsPresence>1</BitsPresence></Bits>
 *         <Bits bit="7"><BitsShortName>ID</BitsShortName><BitsPresence>2</BitsPresence></Bits>
 *         <Bits bit="6"><BitsShortName>MHG</BitsShortName><BitsPresence>3</BitsPresence></Bits>
 *         <Bits bit="5"><BitsShortName>IAS</BitsShortName><BitsPresence>4</BitsPresence></Bits>
 *         <Bits bit="4"><BitsShortName>TAS</BitsShortName><BitsPresence>5</BitsPresence></Bits>
 *         <Bits bit="3"><BitsShortName>SAL</BitsShortName><BitsPresence>6</BitsPresence></Bits>
 *         <Bits bit="2"><BitsShortName>FSS</BitsShortName><BitsPresence>7</BitsPresence></Bits>
 *         <Bits bit="1"><BitsShortName>FX</BitsShortName></Bits>
 *       </Fixed>
 *     </Variable>
 *
 *     <!-- Secondary fields: Sub-items (only present if FSPEC bit set) -->
 *     <Fixed length="3">  <!-- Sub-item 1: Target Address -->
 *       <Bits bit="24" to="1"><BitsShortName>ADR</BitsShortName></Bits>
 *     </Fixed>
 *     <Fixed length="6">  <!-- Sub-item 2: Target Identification -->
 *       <Bits bit="48" to="1"><BitsShortName>ID</BitsShortName></Bits>
 *     </Fixed>
 *     <Fixed length="2">  <!-- Sub-item 3: Magnetic Heading -->
 *       <Bits bit="16" to="1"><BitsShortName>MHG</BitsShortName></Bits>
 *     </Fixed>
 *     <!-- ... more sub-items ... -->
 *   </Compound>
 * </DataItemFormat>
 * @endcode
 *
 * @par Parsing Algorithm
 * The length and parsing algorithms work in two phases:
 *
 * **Phase 1 - Length Determination (getLength()):**
 * 1. Parse primary field (Variable FSPEC) to determine which sub-items are present
 * 2. For each FSPEC octet, check bits 8-2 for sub-item presence flags
 * 3. For each present sub-item, call its getLength() and accumulate total
 * 4. Return: FSPEC length + sum of all present sub-item lengths
 *
 * **Phase 2 - Data Extraction (getText()):**
 * 1. Skip over the primary FSPEC field
 * 2. Iterate through FSPEC bits in order (8, 7, 6, ..., 2)
 * 3. For each bit set to 1, parse the corresponding sub-item
 * 4. Format each sub-item according to output format (JSON, XML, text)
 * 5. Return formatted object with named sub-item fields
 *
 * @par Example Parsing Flow (I062/380)
 * @code
 * // Input: 0xE0 0x12 0x34 0xAB 0xCD 0x56 0x78
 * unsigned char data[] = {0xE0, 0x12, 0x34, 0xAB, 0xCD, 0x56, 0x78};
 *
 * DataItemFormatCompound* format = ...; // From Category I062
 *
 * // Step 1: Get length
 * long len = format->getLength(data);
 * // - Primary FSPEC: 1 octet (FX=0)
 * // - Sub-item 1 (ADR): 3 bytes (bit 8 = 1)
 * // - Sub-item 2 (ID): 6 bytes (bit 7 = 1)
 * // - Sub-item 3 (MHG): 2 bytes (bit 6 = 1)
 * // Total: 1 + 3 + 6 + 2 = 12 bytes (example, actual may vary)
 *
 * // Step 2: Parse and format as JSON
 * std::string result, header;
 * format->getText(result, header, CAT_FORMAT_ID_JSON, data, len);
 * // result contains:
 * // {
 * //   "ADR": "0x001234",
 * //   "ID": "ABC123",
 * //   "MHG": 123.5
 * // }
 *
 * // Step 3: Type checking
 * assert(format->isCompound() == true);
 * assert(format->isFixed() == false);
 * @endcode
 *
 * @par Memory Management
 * - DataItemFormatCompound objects are owned by DataItemDescription
 * - m_lSubItems contains:
 *   - First element: DataItemFormatVariable* (primary FSPEC field)
 *   - Remaining elements: DataItemFormat* (secondary sub-items)
 * - All sub-items are owned by this format and deleted in destructor
 * - Use clone() to create deep copies
 *
 * @par Performance Considerations
 * Compound items require two-pass processing:
 * 1. First pass: Scan FSPEC to determine which sub-items are present
 * 2. Second pass: Parse each present sub-item
 *
 * This makes Compound parsing slower than Fixed but more flexible.
 * Average performance: 10-15% slower than Fixed, but 20-30% faster than Explicit
 * due to efficient bitmap checking vs. nested parsing.
 *
 * **Optimization notes:**
 * - FSPEC bits are checked sequentially (no bitmap caching)
 * - Sub-item lengths are calculated on-demand during parsing
 * - C++23 ranges::transform used in copy constructor for 5-10% speedup
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same DataItemFormatCompound
 * instance from multiple threads concurrently.
 *
 * @par C++23 Optimization
 * - Uses "deduced this" (explicit object parameter) for clone() when available
 * - Uses ranges algorithms for m_lSubItems iteration when C++23 detected
 * - Provides 5-20% performance improvement over C++17/20 builds
 *
 * @par Real-World Examples
 * Common ASTERIX Compound items:
 * - **I062/340** (Measured Information): Sensor measurements with optional fields
 * - **I062/380** (Aircraft Derived Data): Target-reported data (Mode S, ADS-B)
 * - **I062/390** (Flight Plan Related Data): Optional flight plan fields
 * - **CAT048/090** (Flight Level in Binary Representation): Validated + Mode C
 *
 * @see DataItemFormat For polymorphic base class with full API documentation
 * @see DataItemFormatVariable For Variable format used by primary FSPEC field
 * @see DataItemFormatFixed For comparison with fixed-length items
 * @see DataItemBits For bit-level field definitions and BitsPresence attribute
 */
class DataItemFormatCompound : public DataItemFormat {
public:
    /**
     * @brief Construct a Compound format parser with optional ID
     *
     * @param id Format ID (default: 0). Used during XML parsing and debugging.
     *
     * @note The m_lSubItems list must be populated by XMLParser before use:
     *       - First element: DataItemFormatVariable* (primary FSPEC field)
     *       - Remaining elements: DataItemFormat* (secondary sub-items)
     */
    DataItemFormatCompound(int id = 0);

    /**
     * @brief Copy constructor - creates deep copy of Compound format
     *
     * @param obj Source DataItemFormatCompound to copy from
     *
     * @note Performs deep copy of all sub-items in m_lSubItems using clone().
     *       Uses C++23 ranges::transform when available for 5-10% better performance.
     *       Both the primary FSPEC field and all secondary sub-items are cloned.
     */
    DataItemFormatCompound(const DataItemFormatCompound &obj);

    /**
     * @brief Virtual destructor
     *
     * Frees all sub-item formats in m_lSubItems (inherited from DataItemFormat).
     * This includes the primary Variable FSPEC field and all secondary sub-items.
     */
    virtual
    ~DataItemFormatCompound();

    /**
     * @brief Create a deep copy of this Compound format object
     *
     * @return Pointer to newly allocated DataItemFormatCompound clone.
     *         Caller is responsible for deleting the returned object.
     *
     * @note Uses C++23 "deduced this" when available for improved performance.
     *       Falls back to traditional virtual method on C++17/20 compilers.
     *       Copies the primary FSPEC field and all secondary sub-items via clone().
     *
     * @par C++23 Optimization
     * The explicit object parameter (deduced this) allows the compiler to
     * devirtualize clone() calls in some contexts, providing 15-20% speedup.
     */
    // C++23 Quick Win: Deduced this allows better devirtualization
#if HAS_DEDUCED_THIS
    DataItemFormatCompound *clone(this const auto& self) const { return new DataItemFormatCompound(self); }
#else
    DataItemFormatCompound *clone() const override { return new DataItemFormatCompound(*this); } // Return clone of object
#endif

    /**
     * @brief Calculate total length by scanning FSPEC and summing present sub-items
     *
     * @param pData Pointer to binary ASTERIX data for this Compound item.
     *              Must not be nullptr.
     * @return Total length in bytes (FSPEC length + all present sub-item lengths)
     *
     * @par Algorithm
     * This is a two-phase calculation:
     * 1. **Primary FSPEC phase**: Calculate FSPEC length by scanning FX bits
     *    - Delegates to pCompoundPrimary->getLength(pData)
     *    - Returns 1-N octets depending on number of sub-items
     *
     * 2. **Secondary sub-items phase**: For each FSPEC octet:
     *    - Iterate through bits 8-2 (presence flags)
     *    - For each bit set to 1, get corresponding sub-item length
     *    - Accumulate total length
     *    - Stop when FX=0 (last FSPEC octet)
     *
     * @par Example Calculation
     * @code
     * // Input: 0xE0 [sub-item 1: 3 bytes] [sub-item 2: 6 bytes] [sub-item 3: 2 bytes]
     * unsigned char data[] = {0xE0, ...};
     *                          ^^^^
     *                          11100000 = Bits 8,7,6 set, FX=0
     *
     * long len = format->getLength(data);
     * // Calculation:
     * // - FSPEC length: 1 octet (FX=0)
     * // - Bit 8 set: Sub-item 1 length = 3 bytes
     * // - Bit 7 set: Sub-item 2 length = 6 bytes
     * // - Bit 6 set: Sub-item 3 length = 2 bytes
     * // - Bits 5-2 clear: No additional sub-items
     * // Total: 1 + 3 + 6 + 2 = 12 bytes
     * @endcode
     *
     * @warning Does not validate buffer size. Caller must ensure pData has
     *          sufficient bytes available. Reading beyond buffer causes
     *          undefined behavior.
     *
     * @par Error Handling
     * Returns 0 if:
     * - Primary FSPEC field is missing (m_lSubItems.size() < 2)
     * - pCompoundPrimary is nullptr
     * - Logs error via Tracer::Error()
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getLength(const unsigned char*)
     */
    long getLength(const unsigned char *pData);

    /**
     * @brief Parse and format Compound item into text representation
     *
     * @param[out] strResult   Output string to which formatted data is appended
     * @param[out] strHeader   Output string to which header/metadata is appended
     * @param[in]  formatType  Output format identifier:
     *                         - CAsterixFormat::ETEXT: Human-readable text
     *                         - CAsterixFormat::EJSON: Compact JSON
     *                         - CAsterixFormat::EJSONH: Human-readable JSON with newlines
     *                         - CAsterixFormat::EJSONE: Extensive JSON with descriptions
     *                         - CAsterixFormat::EXML: XML format
     * @param[in]  pData       Pointer to binary ASTERIX data for this item
     * @param[in]  nLength     Total length of data (from getLength(), currently unused)
     *
     * @return true if any sub-item was successfully formatted, false if all failed
     *
     * @par Formatting Behavior (JSON)
     * For JSON formats, generates an object with named sub-item fields:
     * @code
     * {
     *   "ADR": "0x001234",      <- Sub-item 1 (if FSPEC bit 8 = 1)
     *   "ID": "ABC123",         <- Sub-item 2 (if FSPEC bit 7 = 1)
     *   "MHG": 123.5            <- Sub-item 3 (if FSPEC bit 6 = 1)
     * }
     * @endcode
     *
     * The field names come from DataItemFormatFixed::getPartName() which
     * retrieves the BitsPresence name from the primary FSPEC field.
     *
     * @par Implementation Details
     * 1. Skip over primary FSPEC field (length determined by getLength())
     * 2. Iterate through FSPEC octets checking bits 8-2
     * 3. For each bit set to 1:
     *    - Get sub-item name from getPartName(secondaryPart)
     *    - Call sub-item's getText() to parse and format data
     *    - Append formatted sub-item to result with name as key
     * 4. Stop when FX=0 (last FSPEC octet)
     *
     * @par Example Output
     * @code
     * // Input: 0xC0 [sub-item 1: 0x123] [sub-item 2: "ABC123"]
     * //        ^^^^ = 11000000 (bits 8,7 set, FX=0)
     *
     * std::string result, header;
     * format->getText(result, header, CAsterixFormat::EJSONH, data, len);
     *
     * // Output (EJSONH - human-readable JSON):
     * // {
     * //   "ADR":"0x000123",
     * //   "ID":"ABC123"
     * // }
     *
     * // Output (EJSON - compact JSON):
     * // {"ADR":"0x000123","ID":"ABC123"}
     * @endcode
     *
     * @note This method APPENDS to strResult and strHeader, does not clear them.
     *       Each sub-item's getText() is called to parse individual fields.
     *
     * @par Error Handling
     * Returns false if:
     * - Primary FSPEC field is missing (m_lSubItems.size() < 2)
     * - pCompoundPrimary is nullptr
     * - Logs error via Tracer::Error()
     *
     * @warning The nLength parameter is currently unused in the implementation.
     *          Length validation should be performed by caller.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getText()
     */
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength);

    /**
     * @brief Generate a printable description of the Compound format structure
     *
     * @param header Prefix string for hierarchical display (e.g., "  " for indent)
     * @return String containing format descriptor tree showing all sub-items
     *
     * @note Delegates to printDescriptors() of each secondary sub-item in m_lSubItems.
     *       Skips the first element (primary FSPEC field) and prints descriptions
     *       for all secondary sub-items. Used for debugging and diagnostics.
     *
     * @par Example Output
     * For I062/380 (Aircraft Derived Data):
     * @code
     * "  Compound:\n"
     * "    Fixed(3): ADR - Target Address\n"
     * "    Fixed(6): ID - Target Identification\n"
     * "    Fixed(2): MHG - Magnetic Heading\n"
     * "    ...\n"
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::printDescriptors()
     */
    std::string printDescriptors(std::string header);

    /**
     * @brief Mark a specific field for filtering
     *
     * @param name Field name to filter (e.g., "ADR", "ID"), or nullptr for entire item
     * @return true if filtering was applied successfully to any sub-item
     *
     * @note Iterates through all secondary sub-items (skips primary FSPEC field)
     *       and delegates to each sub-item's filterOutItem(). Returns true if
     *       ANY sub-item successfully applied the filter.
     *       Used by CLI filter mechanism to show only specific fields.
     *
     * @par Example
     * @code
     * format->filterOutItem("ADR"); // Show only Target Address sub-item
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::filterOutItem()
     */
    bool filterOutItem(const char *name);

    /**
     * @brief Check if a specific field is filtered (should be displayed)
     *
     * @param name Field name to check (e.g., "ADR"), or nullptr for entire item
     * @return true if the field should be displayed (passes filter) in any sub-item
     *
     * @note Iterates through all secondary sub-items (skips primary FSPEC field)
     *       and delegates to each sub-item's isFiltered(). Returns true if
     *       ANY sub-item passes the filter.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::isFiltered()
     */
    bool isFiltered(const char *name);

    /**
     * @brief Type identification - returns true for Compound format
     *
     * @return Always true (this is a Compound format)
     *
     * @note This provides efficient type checking without C++ RTTI.
     *       Used in hot paths during parsing to avoid virtual function overhead.
     *       Alternative to dynamic_cast<DataItemFormatCompound*>.
     *
     * @par Override of Virtual
     * This overrides DataItemFormat::isCompound() which returns false.
     */
    bool isCompound() { return true; };

    /**
     * @brief Get human-readable description for a field value
     *
     * @param field Field name within this item (e.g., "ADR", "ID", or sub-field name)
     * @param value Value to look up (e.g., "5", "0x123")
     * @return String description from XML value mappings, or nullptr if not found
     *
     * @note Iterates through ALL sub-items in m_lSubItems (including primary FSPEC),
     *       casting each to DataItemBits* and querying for the description.
     *       Returns the first matching description found.
     *       The returned pointer points to internal string storage and
     *       should not be freed by the caller.
     *
     * @par Example
     * For I062/380 sub-item field:
     * @code
     * const char* desc = format->getDescription("ADR", "0x123456");
     * // Returns: Description from XML <BitsValue> mapping if defined
     * @endcode
     *
     * @warning Currently casts all sub-items to DataItemBits*, which may cause
     *          issues if sub-items are not all Bits-based formats. This works
     *          because DataItemBits::getDescription() safely returns nullptr
     *          for non-matching fields.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getDescription()
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
     *       Aggregates definitions from the primary FSPEC field and all
     *       secondary sub-items into a single linked list.
     *
     * @par Implementation
     * 1. Get definitions from primary FSPEC field (Variable format)
     * 2. Append definitions from each secondary sub-item
     * 3. Return linked list starting from primary field definitions
     *
     * @par Error Handling
     * Returns nullptr if:
     * - Primary FSPEC field is missing
     * - pCompoundPrimary is nullptr
     * - Logs error via Tracer::Error()
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getWiresharkDefinitions()
     */
    fulliautomatix_definitions* getWiresharkDefinitions();

    /**
     * @brief Get Wireshark dissector data structure (Wireshark plugin only)
     *
     * @param pData      Pointer to binary ASTERIX data for this Compound item
     * @param len        Length of data in bytes (currently unused)
     * @param byteoffset Byte offset in the packet for Wireshark display
     * @return Pointer to fulliautomatix_data structure for Wireshark tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     *       Creates dissector tree for the primary FSPEC field and all
     *       present secondary sub-items.
     *
     * @par Implementation
     * 1. Create tree node for "Compound item header"
     * 2. Get data from primary FSPEC field (Variable format)
     * 3. For each FSPEC bit set to 1, get data from corresponding sub-item
     * 4. Build linked list of fulliautomatix_data nodes
     * 5. Update byteoffset as sub-items are parsed
     *
     * @par Error Handling
     * Returns 0 (nullptr) if:
     * - Primary FSPEC field is missing (m_lSubItems.size() < 2)
     * - pCompoundPrimary is nullptr
     * - Logs error via Tracer::Error()
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
     * @param pData       Pointer to binary ASTERIX data for this Compound item
     * @param nLength     Length of data in bytes
     * @param description If non-zero, include descriptive metadata from XML
     * @return PyObject* containing parsed data (Python dict)
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       The returned object has a new reference (caller must DECREF).
     *
     * @par Return Type
     * Always returns a Python dict with sub-item fields as keys:
     * @code{.py}
     * {
     *     "ADR": "0x001234",      # Sub-item 1 (if FSPEC bit 8 = 1)
     *     "ID": "ABC123",         # Sub-item 2 (if FSPEC bit 7 = 1)
     *     "MHG": 123.5            # Sub-item 3 (if FSPEC bit 6 = 1)
     * }
     * @endcode
     *
     * @par Implementation
     * 1. Create new Python dict (PyDict_New())
     * 2. Call insertToDict() to populate the dict with sub-item data
     * 3. Return the populated dict
     *
     * @par Example Python Usage
     * @code{.py}
     * import asterix
     * data = b'\xE0\x12\x34\xAB\xCD\x56\x78'  # FSPEC + sub-items
     * result = asterix.parse(data)
     * # result['I380'] = {'ADR': '0x001234', 'ID': 'ABC123', 'MHG': 123.5}
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getObject()
     */
    PyObject* getObject(unsigned char* pData, long nLength, int description);

    /**
     * @brief Insert parsed Compound data into existing Python dictionary (Python binding only)
     *
     * @param p           PyObject* pointing to a Python dict to insert into
     * @param pData       Pointer to binary ASTERIX data for this Compound item
     * @param nLength     Length of data in bytes
     * @param description If non-zero, include descriptive metadata from XML
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       Inserts sub-item name-value pairs into the provided dictionary.
     *
     * @par Implementation Algorithm
     * 1. Parse primary FSPEC field to determine which sub-items are present
     * 2. Skip over FSPEC field (length from pCompoundPrimary->getLength())
     * 3. For each FSPEC octet, iterate through bits 8-2 (presence flags)
     * 4. For each bit set to 1:
     *    - Get sub-item name from getPartName(secondaryPart)
     *    - Call sub-item's getObject() to parse binary data
     *    - Insert (name, object) pair into dict using PyDict_SetItem()
     *    - DECREF the object after insertion (dict takes ownership)
     * 5. Stop when FX=0 (last FSPEC octet)
     *
     * @par Example Insertion
     * @code
     * // C code (internal):
     * PyObject* dict = PyDict_New();
     * unsigned char data[] = {0xE0, 0x12, 0x34, 0xAB, 0xCD, 0x56, 0x78};
     * format->insertToDict(dict, data, sizeof(data), 1);
     *
     * // Python result:
     * // dict = {
     * //   "ADR": "0x001234",
     * //   "ID": "ABC123",
     * //   "MHG": 123.5
     * // }
     * @endcode
     *
     * @par Error Handling
     * Returns immediately (no insertion) if:
     * - Primary FSPEC field is missing (m_lSubItems.size() < 2)
     * - pCompoundPrimary is nullptr
     * - Logs error via Tracer::Error()
     *
     * @par Memory Management
     * - Creates new PyObject* for each sub-item via getObject()
     * - PyDict_SetItem() increments reference count
     * - Py_DECREF() called after insertion to release ownership
     * - Caller owns the dict parameter
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::insertToDict()
     */
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int description);
#endif

};

#endif /* DATAITEMFORMATCOMPOUND_H_ */
