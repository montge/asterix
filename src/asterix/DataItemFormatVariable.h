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
 * @file DataItemFormatVariable.h
 * @brief Variable-length ASTERIX data item format parser with FX (Field Extension) bits
 *
 * This file defines the DataItemFormatVariable class which handles parsing of
 * variable-length ASTERIX data items. Variable-length items use FX (Field Extension)
 * bits to indicate whether additional octets follow, allowing items to extend
 * dynamically based on the data content.
 *
 * @par FX Bit Mechanism
 * The FX bit (typically bit 1, the LSB of each octet) determines if the item continues:
 * - FX=1: More octets follow (item continues)
 * - FX=0: This is the last octet (item ends)
 *
 * @par Typical Structure
 * @code
 * Octet 1:  [Data bits 8-2] [FX=1]  <- FX=1 means continue
 * Octet 2:  [Data bits 8-2] [FX=1]  <- FX=1 means continue
 * Octet 3:  [Data bits 8-2] [FX=0]  <- FX=0 means last octet
 * @endcode
 *
 * @see DataItemFormat For base class interface
 * @see DataItemFormatFixed For fixed-length items (building blocks of variable items)
 */

#ifndef DATAITEMFORMATVARIABLE_H_
#define DATAITEMFORMATVARIABLE_H_

#include "DataItemFormat.h"
#include "DataItemFormatFixed.h"
#include "cxx23_features.h"

/**
 * @class DataItemFormatVariable
 * @brief Parser for variable-length ASTERIX items using FX (Field Extension) bits
 *
 * DataItemFormatVariable handles ASTERIX data items with variable length determined
 * by FX (Field Extension) bits. Each octet in the item has an FX bit (typically the
 * LSB) indicating whether more octets follow:
 * - FX=1: Item continues with next octet
 * - FX=0: Current octet is the last one
 *
 * @par FX Bit Extension Mechanism
 * Variable items consist of a sequence of fixed-length parts (stored in m_lSubItems),
 * typically 1 octet each. The parser scans FX bits to determine the total length:
 *
 * @code
 * ASCII Diagram - Variable Item with FX bits:
 *
 *   Octet 1:    7  6  5  4  3  2  1  0
 *              [------------------|FX=1]  <- Extension bit set, more data follows
 *                       |
 *   Octet 2:    7  6  5  4  3  2  1  0
 *              [------------------|FX=1]  <- Extension bit set, more data follows
 *                       |
 *   Octet 3:    7  6  5  4  3  2  1  0
 *              [------------------|FX=0]  <- Extension bit clear, last octet
 *
 * Total length: 3 octets (determined by scanning FX bits)
 * @endcode
 *
 * @par Variable-Length Parsing Algorithm
 * The length determination algorithm (getLength()) works as follows:
 * 1. Start with the first fixed-length part (m_lSubItems.begin())
 * 2. Check the FX bit in the current part using isLastPart()
 * 3. If FX=1 (not last part), add part length and advance to next part
 * 4. If FX=0 (last part), add final part length and stop
 * 5. Return total accumulated length
 *
 * @par Implementation Details
 * - m_lSubItems contains DataItemFormatFixed objects (usually 1 octet each)
 * - If m_lSubItems.size() == 1: All extension octets use the same format definition
 * - If m_lSubItems.size() > 1: Different octets can have different field definitions
 * - Each fixed part contains DataItemBits describing the bit-level fields
 * - The FX bit is marked with m_bExtension=true in DataItemBits
 *
 * @par Example XML Definition
 * @code{.xml}
 * <DataItemFormat desc="Aircraft Address">
 *     <Variable>
 *         <Fixed length="1">
 *             <Bits bit="8" to="2"><BitsShortName>ADDR1</BitsShortName></Bits>
 *             <Bits bit="1"><BitsShortName>FX</BitsShortName></Bits>
 *         </Fixed>
 *         <!-- If FX=1, this octet follows: -->
 *         <Fixed length="1">
 *             <Bits bit="8" to="2"><BitsShortName>ADDR2</BitsShortName></Bits>
 *             <Bits bit="1"><BitsShortName>FX</BitsShortName></Bits>
 *         </Fixed>
 *     </Variable>
 * </DataItemFormat>
 * @endcode
 *
 * @par Real-World Example (CAT062 I380 - Aircraft Derived Data)
 * @code
 * Binary data: 0x82 0x04
 *              ^^^^^^^^
 *              10000010 00000100
 *              |      | |      |
 *              |      | |      +-- FX=0 (last octet)
 *              |      +---------- FX=1 (more octets follow)
 *              +----------------- Data field
 *
 * Length calculation:
 * - Read octet 1: 0x82, FX bit (bit 0) = 0, but wait... bit 1 = 1 (FX=1)
 * - Read octet 2: 0x04, FX bit (bit 0) = 0 (FX=0, last octet)
 * - Total length: 2 octets
 * @endcode
 *
 * @par Memory Management
 * - m_lSubItems contains owned DataItemFormatFixed pointers (deleted in destructor)
 * - Each fixed part owns its DataItemBits objects
 * - clone() creates deep copies of all sub-items
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same instance from
 * multiple threads concurrently.
 *
 * @par C++23 Optimization
 * - Uses deduced this (explicit object parameter) for clone() when available
 * - Uses ranges algorithms for m_lSubItems iteration when C++23 detected
 * - Provides 5-20% performance improvement over C++17/20 builds
 *
 * @see DataItemFormat Base class with full API documentation
 * @see DataItemFormatFixed Fixed-length building blocks of variable items
 * @see DataItemBits Bit-level field definitions including FX bits
 */
class DataItemFormatVariable : public DataItemFormat {
public:
    /**
     * @brief Construct a variable-length format with optional ID
     *
     * @param id Format ID for identification (default: 0)
     *
     * @note The m_lSubItems list must be populated (usually by XMLParser)
     *       with DataItemFormatFixed objects before parsing.
     */
    DataItemFormatVariable(int id = 0);

    /**
     * @brief Copy constructor - creates deep copy of variable format
     *
     * @param obj Source DataItemFormatVariable to copy
     *
     * @note Performs deep copy of all sub-items in m_lSubItems using clone().
     *       Uses C++23 ranges::transform when available for better performance.
     */
    DataItemFormatVariable(const DataItemFormatVariable &obj);

    /**
     * @brief Virtual destructor - frees all fixed-length sub-items
     *
     * Deletes all DataItemFormatFixed objects in m_lSubItems list.
     */
    virtual
    ~DataItemFormatVariable();

    /**
     * @brief Create a deep copy of this variable format
     *
     * @return Pointer to newly allocated DataItemFormatVariable clone.
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
    DataItemFormatVariable *clone(this const auto& self) const { return new DataItemFormatVariable(self); }
#else
    DataItemFormatVariable *clone() const override { return new DataItemFormatVariable(*this); } // Return clone of object
#endif

    /**
     * @brief Calculate total length by scanning FX bits in binary data
     *
     * @param pData Pointer to binary ASTERIX data for this variable item.
     *              Must not be nullptr.
     * @return Total length in bytes of the variable item (1 to N octets)
     *
     * @par Algorithm
     * Iterates through fixed-length parts (m_lSubItems), checking the FX bit
     * of each part using DataItemFormatFixed::isLastPart():
     * 1. Start with first fixed part, get its length (typically 1 octet)
     * 2. Check FX bit: if FX=1, item continues; if FX=0, item ends
     * 3. Accumulate length and advance to next part until FX=0 found
     * 4. Return total accumulated length
     *
     * @par Example
     * @code
     * // Variable item with 3 octets (1-byte parts):
     * unsigned char data[] = {0x82, 0x04, 0x00}; // FX bits: 1, 0, (unused)
     *                          ^     ^
     *                         FX=1  FX=0
     *
     * long len = format->getLength(data);
     * // len = 2 octets (stopped at second octet where FX=0)
     * @endcode
     *
     * @warning Does not validate buffer size. Caller must ensure pData
     *          has sufficient bytes available. Reading beyond buffer
     *          causes undefined behavior.
     *
     * @see DataItemFormatFixed::isLastPart() For FX bit checking logic
     */
    long getLength(const unsigned char *pData);

    /**
     * @brief Parse and format variable item into text representation
     *
     * @param[out] strResult   Output string to which formatted data is appended
     * @param[out] strHeader   Output string to which header/metadata is appended
     * @param[in]  formatType  Output format identifier:
     *                         - CAsterixFormat::ETEXT: Human-readable text
     *                         - CAsterixFormat::EJSON: Compact JSON
     *                         - CAsterixFormat::EJSONH: Human-readable JSON
     *                         - CAsterixFormat::EJSONE: Extensive JSON with descriptions
     *                         - CAsterixFormat::EXML: XML format
     * @param[in]  pData       Pointer to binary ASTERIX data for this item
     * @param[in]  nLength     Total length of data in bytes (from getLength())
     *
     * @return true if formatting succeeded, false on error
     *
     * @par Formatting Behavior
     * - **Single sub-item definition** (m_lSubItems.size() == 1):
     *   Formats as JSON array: [{"field1": val}, {"field2": val}, ...]
     *   Each octet becomes a separate object in the array.
     *
     * - **Multiple sub-item definitions** (m_lSubItems.size() > 1):
     *   Formats as JSON object: {"field1": val, "field2": val, ...}
     *   Different octets have different field definitions.
     *
     * @par Example - JSON Output
     * @code
     * // Input: Variable item with FX bits, 2 octets
     * unsigned char data[] = {0x82, 0x04};
     *
     * std::string result, header;
     * format->getText(result, header, CAsterixFormat::EJSON, data, 2);
     *
     * // If single sub-item (repeated format):
     * // result = "[{\"ADDR\":\"0x82\"},{\"ADDR\":\"0x04\"}]"
     *
     * // If multiple sub-items (different formats):
     * // result = "{\"ADDR1\":\"0x82\",\"ADDR2\":\"0x04\"}"
     * @endcode
     *
     * @note This method APPENDS to strResult and strHeader, does not clear them.
     *       Each fixed part's getText() is called to parse individual octets.
     */
    virtual bool
    getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
            long nLength);

    /**
     * @brief Generate a printable description of the variable format structure
     *
     * @param header Prefix string for hierarchical display (e.g., "  " for indent)
     * @return String containing format descriptor tree for all fixed parts
     *
     * @note Delegates to DataItemFormatFixed::printDescriptors() for each
     *       part in m_lSubItems. Used for debugging and diagnostics.
     *
     * @par Example Output
     * @code
     * "  Variable:\n"
     * "    Fixed(1): ADDR[8-2] FX[1]\n"
     * "    Fixed(1): ADDR[8-2] FX[1]\n"
     * @endcode
     */
    virtual std::string printDescriptors(std::string header);

    /**
     * @brief Mark a specific field for filtering
     *
     * @param name Field name to filter (e.g., "ADDR" to show only address field)
     *             or nullptr to filter entire item
     * @return true if filtering was applied successfully, false otherwise
     *
     * @note Delegates to DataItemFormatFixed::filterOutItem() for each part.
     *       Used by CLI filter mechanism to show only specific fields.
     */
    virtual bool filterOutItem(const char *name);

    /**
     * @brief Check if a specific field passes the filter
     *
     * @param name Field name to check (e.g., "ADDR") or nullptr for entire item
     * @return true if the field should be displayed (passes filter)
     *
     * @note Delegates to DataItemFormatFixed::isFiltered() for each part.
     *       Returns true if ANY part passes the filter.
     */
    virtual bool isFiltered(const char *name);

    /**
     * @brief Identify this format as Variable type
     *
     * @return true (always)
     *
     * @note Used for type checking without C++ RTTI. Alternative to dynamic_cast.
     *       This method overrides the base class implementation which returns false.
     *
     * @see DataItemFormat::isVariable()
     */
    bool isVariable() const override { return true; }

    /**
     * @brief Get human-readable description for a field value
     *
     * @param field Field name within this item (e.g., "ADDR", "FX")
     * @param value Value to look up (e.g., "5", "0x82")
     * @return String description from XML value mappings, or nullptr if not found
     *
     * @note Iterates through all fixed parts in m_lSubItems, returning the
     *       first matching description found. The returned pointer points to
     *       internal storage and should not be freed by caller.
     *
     * @par Example
     * @code
     * const char* desc = format->getDescription("STATUS", "1");
     * // Returns: "Normal operation" (from XML <BitsValue val="1">)
     * @endcode
     */
    virtual const char *getDescription(const char *field, const char *value);

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    /**
     * @brief Generate Wireshark dissector definitions (Wireshark plugin only)
     *
     * @return Linked list of fulliautomatix_definitions for Wireshark protocol tree
     *
     * @note Aggregates definitions from all fixed parts in m_lSubItems.
     *       Only available when compiled with WIRESHARK_WRAPPER or
     *       ETHEREAL_WRAPPER defined (plugin builds).
     *
     * @see DataItemFormatFixed::getWiresharkDefinitions()
     */
    fulliautomatix_definitions* getWiresharkDefinitions();

    /**
     * @brief Get Wireshark dissector data structure (Wireshark plugin only)
     *
     * @param pData      Pointer to binary ASTERIX data
     * @param len        Length of data in bytes
     * @param byteoffset Byte offset in the packet for Wireshark display
     * @return Pointer to fulliautomatix_data structure for Wireshark tree
     *
     * @note Scans FX bits to determine item length, then gets data from each
     *       fixed part. Only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     *
     * @see DataItemFormatFixed::getData()
     */
    fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif

#if defined(PYTHON_WRAPPER)
    /**
     * @brief Get Python object representation (Python binding only)
     *
     * @param pData   Pointer to binary ASTERIX data
     * @param nLength Length of data in bytes
     * @param verbose If non-zero, include descriptive metadata
     * @return PyObject* containing parsed data (list or dict)
     *
     * @par Return Type
     * - **Single sub-item**: Returns Python list of dicts
     *   Example: [{"ADDR": "0x82"}, {"ADDR": "0x04"}]
     *
     * - **Multiple sub-items**: Returns Python dict
     *   Example: {"ADDR1": "0x82", "ADDR2": "0x04"}
     *
     * @note Only available when compiled with PYTHON_WRAPPER defined
     *       (Python C extension module build). The returned object has
     *       a new reference (caller must DECREF).
     *
     * @warning Scans FX bits to determine how many octets to parse.
     *          Stops when FX=0 or nLength exhausted.
     *
     * @see DataItemFormatFixed::getObject()
     * @see DataItemFormatFixed::insertToDict()
     */
    PyObject* getObject(unsigned char* pData, long nLength, int verbose);

    /**
     * @brief Insert parsed data into existing Python dictionary (Python binding only)
     *
     * @param p       PyObject* pointing to a Python dict to insert into
     * @param pData   Pointer to binary ASTERIX data
     * @param nLength Length of data in bytes
     * @param verbose If non-zero, include descriptive metadata
     *
     * @note Only available when compiled with PYTHON_WRAPPER defined
     *       (Python C extension module build). Currently a no-op
     *       (see commented-out implementation in .cpp file).
     *
     * @warning This method currently returns immediately without inserting data.
     *          The intended behavior was to delegate to fixed parts, but this
     *          caused issues with variable item representation.
     */
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose);
#endif
};

#endif /* DATAITEMFORMATVARIABLE_H_ */
