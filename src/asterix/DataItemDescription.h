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
 * @file DataItemDescription.h
 * @brief Metadata container for ASTERIX data item definitions
 *
 * This file defines the DataItemDescription class which holds all metadata
 * for a single ASTERIX data item from the XML category definitions, including
 * ID, name, format, and parsing rules.
 */

#ifndef DATAITEMDESCRIPTION_H_
#define DATAITEMDESCRIPTION_H_

#include "DataItemFormat.h"

/**
 * @class DataItemDescription
 * @brief Metadata container for a single ASTERIX data item definition
 *
 * DataItemDescription holds all information about a data item from the XML
 * category definition file, including:
 * - Item ID (e.g., "010" for SAC/SIC in CAT062)
 * - Human-readable name and definition
 * - Format type (Fixed, Variable, Compound, etc.)
 * - Optional/Mandatory rule
 * - Format object for parsing binary data
 *
 * These objects are created during XML parsing and owned by the Category class.
 *
 * @par Memory Management
 * - DataItemDescription owns the DataItemFormat object (m_pFormat)
 * - Owned by Category class and deleted when Category is destroyed
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same DataItemDescription
 * instance from multiple threads concurrently.
 *
 * @par Example Usage
 * @code
 * // Typical usage (internal to ASTERIX decoder):
 * Category* cat = globalDef.getCategory(62);
 * DataItemDescription* desc = cat->getDataItemDescription("010");
 *
 * if (desc) {
 *     std::cout << "Item: I062/" << desc->m_strID << " - "
 *               << desc->m_strName << std::endl;
 *     std::cout << "Format: " << desc->m_strFormat << std::endl;
 *     std::cout << "Rule: " << (desc->m_eRule == DATAITEM_MANDATORY ?
 *                               "Mandatory" : "Optional") << std::endl;
 * }
 * @endcode
 *
 * @see DataItemFormat For the format object that handles parsing
 * @see Category For the container owning DataItemDescription objects
 * @see XMLParser For loading item definitions from XML
 */
class DataItemDescription {
public:
    /**
     * @brief Construct a DataItemDescription with the specified ID
     *
     * @param id Data item ID string (e.g., "010", "040", "105")
     *           Without category prefix (e.g., "010" not "I062/010").
     *
     * @note The format (m_pFormat) must be set separately after construction,
     *       typically during XML parsing.
     */
    DataItemDescription(std::string id);

    /**
     * @brief Destructor - frees the format object
     *
     * Deletes the DataItemFormat object (m_pFormat).
     */
    virtual
    ~DataItemDescription();

    /**
     * @brief Data item ID string (e.g., "010" for SAC/SIC)
     *
     * Three-digit ID without category prefix.
     * Example: "010" for I062/010 (System Area Code / System Identification Code)
     */
    std::string m_strID;

    /**
     * @brief Data item ID as hexadecimal integer
     *
     * Numeric conversion of m_strID for efficient comparison.
     * Used primarily for BDS register decoding.
     */
    int m_nID;

    /**
     * @brief Set the human-readable name for this data item
     *
     * @param name Name string (e.g., "System Area Code / System Identification Code")
     *             Copied into m_strName. The pointer is not retained.
     */
    void setName(char *name) { m_strName = name; }

    /**
     * @brief Set the detailed definition/description for this data item
     *
     * @param definition Description string from XML definition
     *                   Copied into m_strDefinition. The pointer is not retained.
     */
    void setDefinition(char *definition) { m_strDefinition = definition; }

    /**
     * @brief Set the format type string for this data item
     *
     * @param format Format type string (e.g., "fixed", "variable", "compound")
     *               Copied into m_strFormat. The pointer is not retained.
     */
    void setFormat(char *format) { m_strFormat = format; }

    /**
     * @brief Parse and format data item (delegates to m_pFormat)
     *
     * Convenience wrapper that delegates to the DataItemFormat::getText() method.
     *
     * @param[out] strResult   Output string to which formatted data is appended
     * @param[out] strHeader   Output string to which header/metadata is appended
     * @param[in]  formatType  Output format identifier (TEXT/JSON/XML)
     * @param[in]  pData       Pointer to binary ASTERIX data for this item
     * @param[in]  nLength     Length of data in bytes
     *
     * @return true if formatting succeeded, false on error
     *
     * @note Requires m_pFormat to be set (non-null), otherwise will crash.
     */
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength)
    {
        return m_pFormat->getText(strResult, strHeader, formatType, pData, nLength);
    };

    /**
     * @brief Human-readable name of the data item
     *
     * Example: "System Area Code / System Identification Code" for I062/010
     * Set via setName() during XML parsing.
     */
    std::string m_strName;

    /**
     * @brief Detailed definition/description of the data item
     *
     * Longer description from XML category definition explaining the purpose
     * and meaning of this data item. Set via setDefinition() during XML parsing.
     */
    std::string m_strDefinition;

    /**
     * @brief Format type string (e.g., "fixed", "variable", "compound")
     *
     * Human-readable format type name from XML definition.
     * Set via setFormat() during XML parsing.
     */
    std::string m_strFormat;

    /**
     * @brief Additional notes or remarks about this data item
     *
     * Optional field for supplementary information from XML definition.
     */
    std::string m_strNote;

    /**
     * @brief Pointer to the format object that handles parsing
     *
     * Owned by this DataItemDescription and deleted in destructor.
     * Must be set after construction (typically during XML parsing).
     * Points to one of: DataItemFormatFixed, DataItemFormatVariable,
     * DataItemFormatCompound, DataItemFormatRepetitive, etc.
     */
    DataItemFormat *m_pFormat;

    /**
     * @brief Enumeration of data item rule types
     *
     * Indicates whether a data item is mandatory or optional in the UAP.
     */
    typedef enum {
        DATAITEM_UNKNOWN = 0,   ///< Unknown rule (not yet set)
        DATAITEM_OPTIONAL,      ///< Optional data item (may be absent)
        DATAITEM_MANDATORY      ///< Mandatory data item (always present)
    } _eRule;

    /**
     * @brief Rule indicating if this data item is mandatory or optional
     *
     * Set from UAP definition during XML parsing.
     * Determines if the item must always be present (MANDATORY) or
     * can be omitted (OPTIONAL).
     */
    _eRule m_eRule;

};

#endif /* DATAITEMDESCRIPTION_H_ */
