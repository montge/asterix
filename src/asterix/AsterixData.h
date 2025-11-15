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
 * @file AsterixData.h
 * @brief Top-level container for parsed ASTERIX data blocks
 *
 * This file defines the AsterixData class which serves as the root container
 * for all parsed ASTERIX data blocks. It represents the complete output from
 * parsing one or more ASTERIX data blocks from various input sources.
 */

#ifndef ASTERIXDATA_H_
#define ASTERIXDATA_H_

#include "AsterixDefinition.h"
#include "DataBlock.h"
#include <map>

/**
 * @class AsterixData
 * @brief Top-level container for parsed ASTERIX data from all sources
 *
 * AsterixData serves as the root data structure for the complete output of ASTERIX
 * parsing operations. It contains a collection of DataBlock objects extracted from
 * input sources such as files, network streams (multicast/TCP/UDP), or stdin.
 *
 * This class is the final aggregation point in the ASTERIX parsing pipeline:
 * @code
 * Input Source → Format Parser → AsterixData → DataBlock(s) → DataRecord(s) → DataItem(s)
 * @endcode
 *
 * Each AsterixData instance represents one complete parsing session and contains:
 * - Zero or more DataBlock objects (depending on input data)
 * - Methods to format all blocks as text, JSON, or XML
 * - Language-specific data extraction (Python, Wireshark)
 *
 * @par Parsing Pipeline
 * 1. Input data is read from source (file, network, stdin)
 * 2. Format-specific parser extracts raw ASTERIX data blocks
 * 3. Each data block is parsed and added to m_lDataBlocks
 * 4. Output formatter generates text/JSON/XML from all blocks
 *
 * @par Supported Input Formats
 * - Raw ASTERIX binary
 * - PCAP files (with optional ORADIS headers)
 * - HDLC framed data
 * - FINAL protocol encapsulation
 * - GPS timestamped data
 *
 * @par Output Formats
 * The getText() method supports multiple output formats via CAsterixFormat enum:
 * - CAsterixFormat::ETxt - Human-readable text with block separators
 * - CAsterixFormat::EJSON - Compact JSON (one object per line)
 * - CAsterixFormat::EJSONH - Human-readable JSON with indentation
 * - CAsterixFormat::EJSONE - Extensive JSON with descriptions
 * - CAsterixFormat::EXML - Compact XML (line-delimited)
 * - CAsterixFormat::EXMLH - Human-readable XML with indentation
 * - CAsterixFormat::EOut - One-line text format (easy parsing)
 *
 * @par Memory Management
 * - AsterixData owns all DataBlock objects in m_lDataBlocks
 * - DataBlocks are deleted in the destructor
 * - Each DataBlock owns its DataRecord objects (cascading ownership)
 * - Caller is responsible for AsterixData lifetime
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same AsterixData instance
 * from multiple threads concurrently. For multi-threaded parsing, create
 * separate AsterixData instances per thread.
 *
 * @par Example Usage - File Parsing
 * @code
 * // Parse ASTERIX data from PCAP file
 * AsterixData asterixData;
 *
 * // Assume format parser has extracted data blocks and added to asterixData
 * // (typically done by asterixpcapsubformat or asterixrawsubformat)
 *
 * // Generate JSON output for all data blocks
 * std::string jsonOutput;
 * if (asterixData.getText(jsonOutput, CAsterixFormat::EJSONH)) {
 *     std::cout << jsonOutput << std::endl;
 * }
 *
 * // Access individual blocks
 * for (const auto& block : asterixData.m_lDataBlocks) {
 *     std::cout << "Category: " << block->m_pCategory->m_nID << std::endl;
 *     std::cout << "Records: " << block->m_lDataRecords.size() << std::endl;
 * }
 * @endcode
 *
 * @par Example Usage - Network Stream
 * @code
 * // Parse ASTERIX data from UDP multicast stream
 * AsterixData asterixData;
 *
 * // Multicast device reads packets and format parser extracts blocks
 * // Each block is added to asterixData.m_lDataBlocks
 *
 * // Generate compact JSON for real-time processing
 * std::string compactJson;
 * asterixData.getText(compactJson, CAsterixFormat::EJSON);
 *
 * // Process each line (one JSON object per data block)
 * std::istringstream stream(compactJson);
 * std::string line;
 * while (std::getline(stream, line)) {
 *     // Send to downstream consumer (e.g., message queue, database)
 *     processJsonRecord(line);
 * }
 * @endcode
 *
 * @par Language Bindings
 * AsterixData provides specialized getData() methods for language-specific bindings:
 * - Python: Returns PyObject* (nested dictionaries/lists)
 * - Wireshark: Returns fulliautomatix_data* for protocol dissector tree
 *
 * These methods are only available when compiled with PYTHON_WRAPPER or
 * WIRESHARK_WRAPPER preprocessor flags.
 *
 * @see DataBlock For individual data block container
 * @see DataRecord For single ASTERIX record parsing
 * @see DataItem For individual data field extraction
 * @see Category For category definitions and UAP
 */
class AsterixData {
public:
    /**
     * @brief Default constructor - creates empty container
     *
     * Initializes an empty AsterixData container with no data blocks.
     * Data blocks are typically added during parsing by format-specific
     * parsers (asterixpcapsubformat, asterixrawsubformat, etc.).
     */
    AsterixData();

    /**
     * @brief Destructor - frees all data blocks
     *
     * Deletes all DataBlock objects in m_lDataBlocks. Each DataBlock
     * destructor will recursively delete its DataRecord objects, which
     * in turn delete their DataItem objects.
     *
     * @note This destructor performs deep cleanup of the entire parsing
     *       tree, potentially freeing significant memory.
     */
    virtual
    ~AsterixData();

    /**
     * @brief List of all parsed ASTERIX data blocks
     *
     * Contains all DataBlock objects extracted from the input source.
     * These blocks are owned by AsterixData and deleted in destructor.
     *
     * Each DataBlock corresponds to one ASTERIX data block in the input
     * (identified by category, length, and containing one or more records).
     *
     * @note Blocks are added during parsing and should not be manually
     *       deleted by external code.
     */
    std::list<DataBlock *> m_lDataBlocks;

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    /**
     * @brief Generate Wireshark dissector data structure (Wireshark plugin only)
     *
     * Converts all data blocks into a linked list of fulliautomatix_data structures
     * for display in the Wireshark protocol tree. This method iterates through all
     * blocks in m_lDataBlocks and chains their dissector data.
     *
     * @return Pointer to the first fulliautomatix_data structure in the linked list,
     *         or nullptr if no data blocks exist. Caller must manage memory.
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (Wireshark plugin builds).
     *
     * @note The byte offset is calculated automatically for each successive data block
     *       to ensure proper packet highlighting in Wireshark.
     *
     * @see DataBlock::getData(int) For individual block dissector data
     */
    fulliautomatix_data* getData();
#endif

#if defined(PYTHON_WRAPPER)
    /**
     * @brief Convert all data blocks to Python list (Python binding only)
     *
     * Generates a Python list containing all parsed ASTERIX data blocks.
     * Each data block is converted to a Python dictionary with nested
     * structures for records and data items.
     *
     * @param verbose If non-zero, include descriptive metadata (field names,
     *                units, descriptions). If zero, include only raw values.
     *
     * @return New PyObject* pointing to a Python list of dictionaries.
     *         Caller is responsible for decrementing reference count (Py_DECREF).
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     *
     * @par Example Output (verbose=1)
     * @code{.py}
     * [
     *     {
     *         "category": 62,
     *         "length": 128,
     *         "timestamp": 1234567890.5,
     *         "records": [
     *             {
     *                 "I062/010": {"SAC": 7, "SIC": 1},
     *                 "I062/040": {"TRK": 12345},
     *                 # ... more items
     *             }
     *         ]
     *     }
     *     # ... more blocks
     * ]
     * @endcode
     *
     * @see DataBlock::getData(PyObject*, int) For individual block conversion
     */
    PyObject* getData(int verbose);
#endif

    /**
     * @brief Generate formatted text output for all data blocks
     *
     * Converts all data blocks in m_lDataBlocks into a text representation
     * according to the specified format type. Output is APPENDED to strResult,
     * allowing accumulation of multiple parsing sessions.
     *
     * @param[out] strResult   Output string to which formatted data is appended.
     *                         Not cleared before appending.
     * @param[in]  formatType  Output format identifier from CAsterixFormat enum:
     *                         - CAsterixFormat::ETxt - Human-readable text with
     *                           "Data Block N" separators between blocks
     *                         - CAsterixFormat::EOut - One-line text format
     *                           (easy for parsing, no separators)
     *                         - CAsterixFormat::EJSON - Compact JSON
     *                           (one object per line)
     *                         - CAsterixFormat::EJSONH - Human-readable JSON
     *                           with indentation and newlines
     *                         - CAsterixFormat::EJSONE - Extensive JSON with
     *                           descriptions, hex values, and scaled values
     *                         - CAsterixFormat::EXML - Compact XML
     *                           (line-delimited, suitable for streaming)
     *                         - CAsterixFormat::EXMLH - Human-readable XML
     *                           with indentation and newlines
     *
     * @return true if formatting succeeded, false on error (malformed blocks)
     *
     * @note This method APPENDS to strResult, does not clear it. To get fresh
     *       output, clear strResult before calling: strResult.clear()
     *
     * @note For CAsterixFormat::ETxt format, data block numbers are incremented
     *       using a static counter. Block numbering persists across getText() calls
     *       within the same process.
     *
     * @note Empty m_lDataBlocks list produces empty output (no error).
     *
     * @par Example - JSON Output
     * @code
     * AsterixData asterixData;
     * // ... parsing populates asterixData.m_lDataBlocks
     *
     * std::string json;
     * if (asterixData.getText(json, CAsterixFormat::EJSONH)) {
     *     std::cout << json << std::endl;
     * }
     * @endcode
     *
     * @par Example - Text Output with Block Separators
     * @code
     * std::string text;
     * asterixData.getText(text, CAsterixFormat::ETxt);
     *
     * // Output:
     * // -------------------------
     * // Data Block 1
     * // ... block 1 records ...
     * //
     * // -------------------------
     * // Data Block 2
     * // ... block 2 records ...
     * @endcode
     *
     * @par Example - Compact JSON for Streaming
     * @code
     * std::string compactJson;
     * asterixData.getText(compactJson, CAsterixFormat::EJSON);
     *
     * // Output: one JSON object per line (newline-delimited JSON)
     * // {"category":62,"records":[...]}
     * // {"category":48,"records":[...]}
     * @endcode
     *
     * @see DataBlock::getText() For individual block formatting
     * @see CAsterixFormat For format type definitions
     */
    bool
    getText(std::string &strResult, const unsigned int formatType);
};

#endif /* ASTERIXDATA_H_ */
