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
 * @file Category.h
 * @brief ASTERIX category definition and data item management
 *
 * This file defines the Category class which represents a complete ASTERIX
 * category (e.g., CAT062 for System Track Data) including all its data items,
 * UAP (User Application Profile) definitions, and metadata from XML configuration.
 */

#ifndef CATEGORY_H_
#define CATEGORY_H_

#include "DataItemDescription.h"
#include "UAP.h"
#include <string>
#include <list>

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
#include "WiresharkWrapper.h"
#endif

/**
 * @class Category
 * @brief Represents a complete ASTERIX category with its data items and UAP definitions
 *
 * A Category corresponds to one ASTERIX category type (e.g., CAT048 for Monoradar,
 * CAT062 for System Tracks, CAT065 for SDPS Status). Each category is defined by
 * an XML configuration file that specifies:
 *
 * - Category number (0-255, plus 256 for BDS registers)
 * - Version and name
 * - Data item descriptions (I062/010, I062/040, etc.)
 * - UAP (User Application Profile) definitions
 * - Field encodings and value mappings
 *
 * @par Memory Management
 * - Category owns all DataItemDescription objects in m_lDataItems
 * - Category owns all UAP objects in m_lUAPs
 * - Managed by AsterixDefinition singleton
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Categories are typically created once during
 * initialization and should not be modified concurrently.
 *
 * @par Example Usage
 * @code
 * // Typical usage (internal to ASTERIX decoder):
 * AsterixDefinition& def = AsterixDefinition::getInstance();
 * Category* cat062 = def.getCategory(62);
 *
 * if (cat062) {
 *     std::cout << "Category: " << cat062->m_strName
 *               << " v" << cat062->m_strVer << std::endl;
 *
 *     // Get specific data item description
 *     DataItemDescription* sac_sic = cat062->getDataItemDescription("010");
 *
 *     // Get appropriate UAP based on FSPEC
 *     UAP* uap = cat062->getUAP(fspecData, fspecLength);
 * }
 * @endcode
 *
 * @see DataItemDescription For individual data item metadata
 * @see UAP For User Application Profile definitions
 * @see AsterixDefinition For category management
 * @see XMLParser For loading categories from XML files
 */
class Category {
public:
    /**
     * @brief Construct a Category with the specified ID
     *
     * @param id Category number (0-255 for standard categories, 256 for BDS registers)
     *
     * @note Data items and UAPs are added later via XML parsing.
     *       Use XMLParser to populate the category from an XML file.
     */
    explicit Category(int id);

    /**
     * @brief Destructor - frees all data items and UAPs
     *
     * Deletes all DataItemDescription objects in m_lDataItems and
     * all UAP objects in m_lUAPs.
     */
    virtual
    ~Category();

    /**
     * @brief Category number (0-255, or 256 for BDS registers)
     *
     * Standard ASTERIX categories include:
     * - CAT001: Monoradar Target Reports
     * - CAT048: Monoradar Target Reports (newer)
     * - CAT062: System Track Data
     * - CAT065: SDPS Service Status Reports
     * - 256: BDS (Comm-B Data Buffer) register definitions
     */
    unsigned int m_id;

    /**
     * @brief Filter status - true if at least one item should be printed
     *
     * Used by the filter mechanism to determine if this category
     * should produce output when filtering is applied.
     */
    bool m_bFiltered;

    /**
     * @brief Human-readable category name (from XML)
     *
     * Examples: "Monoradar Target Reports", "System Track Data"
     */
    std::string m_strName;

    /**
     * @brief Category version string (from XML)
     *
     * Examples: "1.2", "1.18", "2.1"
     * Corresponds to EUROCONTROL specification version.
     */
    std::string m_strVer;

    /**
     * @brief List of all data item descriptions for this category
     *
     * Contains DataItemDescription objects for all items defined in the
     * XML category file (e.g., I062/010, I062/040, I062/105, etc.).
     * These objects are owned by the Category and deleted in destructor.
     */
    std::list<DataItemDescription *> m_lDataItems;

    /**
     * @brief List of UAP (User Application Profile) definitions
     *
     * Most categories have one UAP, but some may have multiple
     * (selected based on conditions in the FSPEC).
     * These objects are owned by the Category and deleted in destructor.
     */
    std::list<UAP *> m_lUAPs;

    /**
     * @brief Get or create a data item description by ID
     *
     * @param id Data item ID without category prefix (e.g., "010" for I062/010)
     * @return Pointer to DataItemDescription (created if it doesn't exist)
     *
     * @note If the item doesn't exist, a new DataItemDescription is created
     *       and added to m_lDataItems. The returned pointer is owned by the
     *       Category and should not be deleted by the caller.
     */
    DataItemDescription *
    getDataItemDescription(std::string id);

    /**
     * @brief Create and return a new UAP for this category
     *
     * @return Pointer to newly created UAP, owned by this Category
     *
     * @note Used during XML parsing to add UAP definitions.
     *       The UAP is added to m_lUAPs and owned by the Category.
     */
    UAP *newUAP();

    /**
     * @brief Select the appropriate UAP based on FSPEC data
     *
     * @param data Pointer to FSPEC (Field Specification) data bytes
     * @param len  Length of FSPEC data in bytes
     * @return Pointer to the matching UAP, or nullptr if no match
     *
     * @note Some categories have conditional UAPs selected based on
     *       specific bits in the FSPEC. This method implements that
     *       selection logic.
     */
    UAP *getUAP(const unsigned char *data, unsigned long len) const;

    /**
     * @brief Generate a printable list of all item descriptors
     *
     * @return String containing all data item IDs and names
     *
     * @note Used for debugging and diagnostics to show what items
     *       are defined for this category.
     */
    std::string printDescriptors() const;

    /**
     * @brief Mark a specific item/field for filtering
     *
     * @param item Data item ID (e.g., "010")
     * @param name Field name within the item (or nullptr for entire item)
     * @return true if filtering was applied successfully
     *
     * @note Used by the CLI filter mechanism to show only specific
     *       data items or fields in the output.
     */
    bool filterOutItem(std::string item, const char *name);

    /**
     * @brief Check if a specific item/field is filtered
     *
     * @param item Data item ID (e.g., "010")
     * @param name Field name within the item (or nullptr for entire item)
     * @return true if the item/field should be displayed (passes filter)
     */
    bool isFiltered(std::string item, const char *name) const;

    /**
     * @brief Get human-readable description for a field value
     *
     * @param item  Data item ID (e.g., "010")
     * @param field Field name within the item (e.g., "SAC")
     * @param value Value to look up (e.g., "5")
     * @return String description from XML value mappings, or nullptr if not found
     *
     * @par Example
     * For I062/080 (Track Status), value "0" might return "Confirmed track".
     *
     * @note The returned pointer points to internal string storage and
     *       should not be freed by the caller.
     */
    const char *
    getDescription(const char *item, const char *field, const char *value) const;

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    /**
     * @brief Generate Wireshark dissector definitions (Wireshark plugin only)
     *
     * @return Linked list of fulliautomatix_definitions for Wireshark protocol tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     */
    fulliautomatix_definitions* getWiresharkDefinitions();
#endif
};

#endif /* CATEGORY_H_ */
