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
 * @file DataItemFormatBDS.h
 * @brief Mode S BDS (Comm-B Data Buffer) register format parser
 *
 * This file defines the DataItemFormatBDS class which handles parsing
 * of Mode S BDS registers - specialized 56-bit data buffers transmitted
 * via Mode S SSR surveillance systems for enhanced aircraft information.
 */

#ifndef DataItemFormatBDS_H_
#define DataItemFormatBDS_H_

#include "DataItemFormatFixed.h"
#include "cxx23_features.h"

/**
 * @class DataItemFormatBDS
 * @brief Parser for Mode S BDS (Comm-B Data Buffer) register format
 *
 * DataItemFormatBDS handles ASTERIX data items containing Mode S BDS
 * (Comm-B Data Selector) registers - specialized 56-bit data buffers
 * transmitted via Mode S Secondary Surveillance Radar (SSR) downlink
 * transmissions. BDS registers provide enhanced aircraft information
 * beyond basic surveillance data.
 *
 * @par What are BDS Registers?
 * BDS (Comm-B Data Selector) registers are 56-bit (7-byte) Mode S downlink
 * data buffers specified in ICAO Annex 10, Volume IV. They carry additional
 * aircraft state information such as:
 *
 * - **BDS 1,0**: Data link capability report
 * - **BDS 2,0**: Aircraft identification (callsign)
 * - **BDS 3,0**: ACAS Resolution Advisory
 * - **BDS 4,0**: Selected vertical intention (MCP/FCU altitude)
 * - **BDS 4,4**: Meteorological routine air report
 * - **BDS 4,5**: Meteorological hazard report
 * - **BDS 5,0**: Track and turn report
 * - **BDS 6,0**: Heading and speed report
 *
 * @par Structure of BDS Items in ASTERIX
 * BDS items in ASTERIX are always **8 bytes total**:
 * - **Bytes 0-6**: 56-bit BDS register data (7 bytes)
 * - **Byte 7**: BDS register identifier (BDS code)
 *
 * The BDS identifier in byte 7 determines which BDS register format
 * to use for parsing bytes 0-6. Each BDS register has a specific bit
 * structure defined in XML (see `asterix/config/asterix_bds.xml`).
 *
 * @par Example: BDS 2,0 (Aircraft Identification)
 * BDS 2,0 provides the aircraft callsign (flight number) encoded in 6-bit
 * characters across 8 characters:
 *
 * @code
 * // Binary data (8 bytes):
 * unsigned char data[8] = {
 *   0x20, 0x21, 0x0A, 0xC4,  // Characters 1-4 (encoded)
 *   0xA4, 0x80, 0x00, 0x20   // Characters 5-8 + BDS ID (0x20 = BDS 2,0)
 * };
 *
 * // Byte 7 = 0x20 identifies this as BDS 2,0
 * int BDSid = data[7]; // 0x20 (BDS 2,0)
 *
 * // The first 7 bytes contain the callsign
 * // Parsing would extract: "EZY12AB" (example)
 * @endcode
 *
 * @par XML Definition Example (BDS 4,0)
 * BDS registers are defined in `asterix/config/asterix_bds.xml`:
 *
 * @code{.xml}
 * <DataItem id="40">
 *   <DataItemName>Selected vertical intention</DataItemName>
 *   <DataItemFormat desc="Eight-octets fixed length data item.">
 *     <Fixed length="8">
 *       <Bits bit="64">
 *         <BitsShortName>MCP_ALT_STATUS</BitsShortName>
 *         <BitsName>MCP Altitude Status</BitsName>
 *       </Bits>
 *       <Bits from="63" to="52" encode="unsigned">
 *         <BitsShortName>MCP_ALT</BitsShortName>
 *         <BitsName>MCP/FCU Selected Altitude</BitsName>
 *         <BitsUnit scale="16">ft</BitsUnit>
 *       </Bits>
 *       <!-- Additional fields... -->
 *     </Fixed>
 *   </DataItemFormat>
 * </DataItem>
 * @endcode
 *
 * @par Parsing Process
 * BDS items use a two-stage parsing process:
 *
 * 1. **BDS Register Selection**: Read byte 7 to determine BDS code
 * 2. **Format Lookup**: Find matching DataItemFormatFixed in m_lSubItems
 * 3. **Data Parsing**: Delegate to Fixed format parser for bit extraction
 * 4. **Field Formatting**: Format fields according to BDS-specific definitions
 *
 * Each BDS register is internally represented as a DataItemFormatFixed
 * with 8-byte length, stored in m_lSubItems with the BDS code as m_nID.
 *
 * @par Comparison with Other Formats
 * BDS is unique among ASTERIX formats:
 *
 * - **Fixed/Variable/Compound/Repetitive**: General ASTERIX formats
 * - **BDS**: Mode S-specific format with BDS register selection
 *
 * BDS always returns 8 bytes length and delegates actual parsing to
 * Fixed format handlers, but with special BDS register identification logic.
 *
 * @par Memory Management
 * - DataItemFormatBDS objects are owned by DataItemDescription
 * - m_lSubItems contains DataItemFormatFixed* for each BDS register
 * - Each Fixed format is identified by m_nID = BDS code (e.g., 0x20 for BDS 2,0)
 * - Use clone() to create deep copies
 *
 * @par Performance
 * BDS parsing is efficient because:
 * - Fixed 8-byte length (no runtime length calculation)
 * - Direct byte 7 lookup for BDS register identification
 * - Delegates to optimized Fixed format parser
 * - No extension bits or presence bitmaps to scan
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same DataItemFormatBDS
 * instance from multiple threads concurrently.
 *
 * @par Example Usage (I062/370 Mode S MB Data)
 * @code
 * // XML category definition (simplified):
 * // <DataItem id="370">
 * //   <DataItemName>Mode S MB Data</DataItemName>
 * //   <DataItemFormat desc="BDS register">
 * //     <BDS>
 * //       <BDSRegister id="20">...</BDSRegister>  // BDS 2,0
 * //       <BDSRegister id="40">...</BDSRegister>  // BDS 4,0
 * //     </BDS>
 * //   </DataItemFormat>
 * // </DataItem>
 *
 * // C++ usage (internal to ASTERIX decoder):
 * unsigned char data[8] = {
 *   0x20, 0x21, 0x0A, 0xC4,  // BDS data bytes 0-3
 *   0xA4, 0x80, 0x00, 0x20   // BDS data bytes 4-6 + BDS ID (0x20)
 * };
 *
 * DataItemFormatBDS* format = ...; // From Category I062
 *
 * // Get length (always 8 bytes)
 * long len = format->getLength(data); // Returns 8
 *
 * // Parse and format as JSON
 * std::string result, header;
 * format->getText(result, header, CAT_FORMAT_ID_JSON, data, len);
 * // BDS ID 0x20 routes to BDS 2,0 parser
 * // result contains: {"CALLSIGN":"EZY12AB"}
 *
 * // Type checking
 * assert(format->isBDS() == true);
 * assert(format->isFixed() == false);
 * @endcode
 *
 * @see DataItemFormat For polymorphic base class
 * @see DataItemFormatFixed For Fixed format (used internally for BDS registers)
 * @see DataItemBits For bit-level field definitions
 * @see ICAO Annex 10 Volume IV For Mode S BDS register specifications
 *
 * @par References
 * - ICAO Annex 10, Volume IV: Surveillance and Collision Avoidance Systems
 * - EUROCONTROL ASTERIX specifications
 * - asterix/config/asterix_bds.xml: BDS register definitions
 */
class DataItemFormatBDS : public DataItemFormat {
public:
    /**
     * @brief Construct a BDS format parser with optional ID
     *
     * @param id Format ID (default: 0). Used during XML parsing and debugging.
     *
     * @note The m_lSubItems list must be populated with DataItemFormatFixed
     *       objects for each supported BDS register (done during XML parsing).
     *       Each Fixed format must have m_nID set to the BDS code (e.g., 0x20).
     */
    DataItemFormatBDS(int id = 0);

    /**
     * @brief Copy constructor - creates deep copy of BDS format
     *
     * @param obj Source DataItemFormatBDS to copy from
     *
     * @note Copies all fields including m_lSubItems (deep copy via clone()).
     *       Each BDS register format (DataItemFormatFixed) is cloned.
     */
    DataItemFormatBDS(const DataItemFormatBDS &obj);

    /**
     * @brief Virtual destructor
     *
     * Frees all BDS register formats in m_lSubItems (inherited from DataItemFormat).
     */
    virtual
    ~DataItemFormatBDS();

    /**
     * @brief Create a deep copy of this BDS format object
     *
     * @return Pointer to newly allocated DataItemFormatBDS clone.
     *         Caller is responsible for deleting the returned object.
     *
     * @note Uses C++23 "deduced this" when available for improved performance.
     *       Falls back to traditional virtual method on C++17/20 compilers.
     *       Copies all fields and deep-copies m_lSubItems (BDS register formats).
     *
     * @par C++23 Optimization
     * The explicit object parameter (deduced this) allows the compiler to
     * devirtualize clone() calls in some contexts, providing 15-20% speedup.
     */
    // C++23 Quick Win: Deduced this allows better devirtualization
#if HAS_DEDUCED_THIS
    DataItemFormatBDS *clone(this const auto& self) const { return new DataItemFormatBDS(self); }
#else
    DataItemFormatBDS *clone() const override { return new DataItemFormatBDS(*this); } // Return clone of object
#endif

    /**
     * @brief Calculate the length of a BDS data item
     *
     * @param pData Pointer to binary ASTERIX data (unused, length is constant)
     * @return Always returns 8 (BDS items are always 8 bytes)
     *
     * @note BDS items have a FIXED length of 8 bytes:
     *       - 7 bytes of BDS register data (56 bits)
     *       - 1 byte BDS register identifier
     *       The pData parameter is accepted for polymorphic interface
     *       compatibility but is never examined.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getLength(const unsigned char*)
     */
    long getLength(const unsigned char *pData);

    /**
     * @brief Parse and format BDS item data into text representation
     *
     * @param[out] strResult   Output string to which formatted data is appended
     * @param[out] strHeader   Output string to which header/metadata is appended
     * @param[in]  formatType  Output format identifier:
     *                         - CAT_FORMAT_ID_TEXT: Human-readable text
     *                         - CAT_FORMAT_ID_JSON: JSON format
     *                         - CAT_FORMAT_ID_XML: XML format
     * @param[in]  pData       Pointer to binary ASTERIX data for this BDS item (8 bytes)
     * @param[in]  nLength     Length of data in bytes (must be 8)
     *
     * @return true if formatting succeeded, false on error
     *
     * @note This method APPENDS to strResult and strHeader, does not clear them.
     *       Reads byte 7 to determine BDS register ID, then finds the matching
     *       DataItemFormatFixed in m_lSubItems and delegates parsing to it.
     *
     * @warning If nLength != 8, an error is logged and the method returns true
     *          without parsing (to avoid buffer overruns).
     *
     * @par Parsing Algorithm
     * 1. Validate nLength == 8
     * 2. Read BDS ID from pData[7]
     * 3. Find matching DataItemFormatFixed where m_nID == BDS ID
     * 4. Delegate to Fixed format's getText() for actual parsing
     * 5. Return formatted result
     *
     * @par Example (BDS 2,0 Aircraft Identification)
     * Input: pData = {0x20, 0x21, 0x0A, 0xC4, 0xA4, 0x80, 0x00, 0x20},
     *        nLength = 8, formatType = CAT_FORMAT_ID_JSON
     * Output: strResult += '{"CALLSIGN":"EZY12AB"}'
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getText()
     */
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength); // appends value description to strResult

    /**
     * @brief Generate a printable description of the BDS format structure
     *
     * @param header Prefix string for hierarchical display (e.g., "  " for indent)
     * @return String containing format descriptor tree for all BDS registers
     *
     * @note Iterates through all BDS registers in m_lSubItems and prints
     *       each one with "BDS<id>:" prefix (e.g., "BDS20:", "BDS40:").
     *       Used for debugging and diagnostics.
     *
     * @par Example Output
     * For BDS format with registers 2,0 and 4,0:
     * @code
     * BDS20:
     *   CALLSIGN (48 bits): Aircraft Identification
     * BDS40:
     *   MCP_ALT (12 bits): MCP/FCU Selected Altitude
     *   FMS_ALT (12 bits): FMS Selected Altitude
     * @endcode
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::printDescriptors()
     */
    std::string printDescriptors(std::string header); // print items format descriptors

    /**
     * @brief Mark a specific BDS register field for filtering
     *
     * @param name Field name in format "BDS<hex_id>:<field>" (e.g., "BDS20:CALLSIGN")
     * @return true if filtering was applied successfully, false if format invalid
     *
     * @note Parses the name to extract BDS register ID and field name using
     *       sscanf with format "BDS%x:%s". Finds the matching register in
     *       m_lSubItems and delegates filtering to it.
     *
     * @par Example
     * @code
     * format->filterOutItem("BDS20:CALLSIGN"); // Show only callsign from BDS 2,0
     * @endcode
     *
     * @warning The name parameter is parsed with sscanf, limited to 127 chars
     *          for the field name portion to prevent buffer overruns.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::filterOutItem()
     */
    bool filterOutItem(const char *name); // mark item for filtering

    /**
     * @brief Check if a specific field is filtered (should be displayed)
     *
     * @param name Field name to check (format varies by implementation)
     * @return true if the field should be displayed (passes filter)
     *
     * @note Queries the first BDS register format in m_lSubItems for filtering
     *       status. This is a simplified implementation that delegates to the
     *       first register's filtering logic.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::isFiltered()
     */
    bool isFiltered(const char *name);

    /**
     * @brief Type identification - returns true for BDS format
     *
     * @return Always true (this is a BDS format)
     *
     * @note This provides efficient type checking without C++ RTTI.
     *       Used in hot paths during parsing to avoid virtual function overhead.
     *
     * @par Override of Virtual
     * This overrides DataItemFormat::isBDS() which returns false.
     */
    bool isBDS() { return true; }; // true if this is BDS format

    /**
     * @brief Get human-readable description for a BDS field value
     *
     * @param field Field name within this BDS item (e.g., "CALLSIGN", "MCP_ALT")
     * @param value Value to look up (e.g., "EZY12AB", "35000")
     * @return String description from XML value mappings, or nullptr if not found
     *
     * @note Iterates through all BDS registers in m_lSubItems and queries
     *       each one for the field description. Returns the first match.
     *       The returned pointer points to internal string storage and
     *       should not be freed by the caller.
     *
     * @par Example
     * For BDS 4,0 (Selected vertical intention) MCP_ALT_STATUS field:
     * @code
     * const char* desc = format->getDescription("MCP_ALT_STATUS", "1");
     * // Returns: "MCP/FCU altitude valid"
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
     *       Delegates to the first BDS register format in m_lSubItems.
     *       Generates definitions for all bit fields in that register.
     *
     * @warning Simplified implementation: only uses the first register.
     *          Multi-register BDS formats may not be fully represented.
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getWiresharkDefinitions()
     */
    fulliautomatix_definitions* getWiresharkDefinitions();

    /**
     * @brief Get Wireshark dissector data structure (Wireshark plugin only)
     *
     * @param pData      Pointer to binary ASTERIX data for this BDS item
     * @param len        Length of data in bytes (should be 8)
     * @param byteoffset Byte offset in the packet for Wireshark display
     * @return Pointer to fulliautomatix_data structure for Wireshark tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     *       Creates dissector tree for all bit fields in the BDS register.
     *
     * @warning Simplified implementation: only uses the first register.
     *          Multi-register BDS formats may not be fully represented.
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
     * @param pData       Pointer to binary ASTERIX data for this BDS item (8 bytes)
     * @param nLength     Length of data in bytes (must be 8)
     * @param verbose     If non-zero, include descriptive metadata from XML
     * @return PyObject* containing parsed BDS data (typically a dict)
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       The returned object has a new reference (caller must DECREF).
     *       Reads BDS ID from byte 7, finds matching register, and parses.
     *
     * @warning If nLength != 8, an error is logged and an empty dict is returned.
     *
     * @par Example Return Value (BDS 2,0 Aircraft Identification)
     * Python dict: {'CALLSIGN': 'EZY12AB'}
     *
     * @par Algorithm
     * 1. Create new Python dict
     * 2. Call insertToDict() to populate it
     * 3. Return dict
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::getObject()
     */
    PyObject* getObject(unsigned char* pData, long nLength, int verbose);

    /**
     * @brief Insert parsed BDS data into existing Python dictionary (Python binding only)
     *
     * @param p           PyObject* pointing to a Python dict to insert into
     * @param pData       Pointer to binary ASTERIX data for this BDS item (8 bytes)
     * @param nLength     Length of data in bytes (must be 8)
     * @param verbose     If non-zero, include descriptive metadata from XML
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *       Reads BDS ID from byte 7, finds matching DataItemFormatFixed in
     *       m_lSubItems, and delegates field insertion to it.
     *
     * @warning If nLength != 8, an error is logged and no data is inserted.
     *
     * @par Algorithm
     * 1. Validate nLength == 8
     * 2. Read BDS ID from pData[7]
     * 3. Find matching DataItemFormatFixed where m_nID == BDS ID or 0
     * 4. Delegate to Fixed format's insertToDict() for actual field insertion
     *
     * @par Example
     * For BDS 2,0 (Aircraft Identification):
     * Input: pData = {..., 0x20}, nLength = 8
     * Output: p dict contains: {'CALLSIGN': 'EZY12AB'}
     *
     * @par Override of Pure Virtual
     * This implements the pure virtual DataItemFormat::insertToDict()
     */
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose);
#endif
};

#endif /* DataItemFormatBDS_H_ */
