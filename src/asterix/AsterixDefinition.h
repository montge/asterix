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
 * @file AsterixDefinition.h
 * @brief Global registry of all ASTERIX category definitions
 *
 * This file defines the AsterixDefinition class which acts as a global
 * singleton managing all loaded ASTERIX category definitions from XML files.
 */

#ifndef ASTERIXDEFINITION_H_
#define ASTERIXDEFINITION_H_

#include "Category.h"

/**
 * @brief Maximum number of ASTERIX categories (0-255 plus BDS at 256)
 */
#define MAX_CATEGORIES  256+1

/**
 * @brief Special category ID for BDS (Comm-B Data Buffer) registers
 *
 * BDS registers are Mode S downlink data formats (e.g., BDS 2,0 for aircraft
 * identification) and are stored at index 256 in the category array.
 */
#define BDS_CAT_ID    256

/**
 * @class AsterixDefinition
 * @brief Singleton managing all ASTERIX category definitions
 *
 * AsterixDefinition is the global registry of all loaded ASTERIX categories.
 * It provides access to Category objects by category number and manages
 * their lifecycle.
 *
 * @par Singleton Pattern
 * This class is typically used as a singleton (though the current implementation
 * allows multiple instances). In practice, one global instance is created and
 * initialized at program startup via InputParser::init().
 *
 * @par Thread Safety
 * **WARNING: This class is NOT thread-safe.**
 *
 * The global singleton pattern means all parsing operations share the same
 * category definitions. Concurrent access from multiple threads will cause
 * race conditions. See the Rust bindings documentation for thread-safe usage patterns.
 *
 * @par Initialization
 * Categories are loaded from XML files via XMLParser during initialization:
 * @code
 * // Typical initialization (in main or language binding init):
 * AsterixDefinition globalDef;  // Create instance
 * InputParser::init("asterix.ini", &globalDef);  // Load all categories from config
 * @endcode
 *
 * @par Usage
 * @code
 * // Get a category for parsing:
 * Category* cat062 = globalDef.getCategory(62);
 * if (cat062) {
 *     // Use category to parse data blocks
 * }
 *
 * // Check if a category is loaded:
 * if (globalDef.CategoryDefined(48)) {
 *     // CAT048 is available
 * }
 *
 * // Get human-readable description for a field value:
 * const char* desc = globalDef.getDescription(62, "080", "TYP", "0");
 * // Returns: "Confirmed track" (or similar from XML)
 * @endcode
 *
 * @par Memory Management
 * - AsterixDefinition owns all Category objects in m_pCategory array
 * - Categories are deleted in destructor
 * - Do not delete Category pointers returned by getCategory()
 *
 * @see Category For individual category definitions
 * @see XMLParser For loading categories from XML files
 * @see InputParser For initialization from asterix.ini
 */
class AsterixDefinition {
public:
    /**
     * @brief Construct an empty AsterixDefinition
     *
     * Initializes the category array with all nullptr entries.
     * Categories must be loaded via XMLParser::Parse() after construction.
     *
     * @note Typically called once at program startup to create the global instance.
     */
    AsterixDefinition();

    /**
     * @brief Destructor - frees all loaded categories
     *
     * Deletes all Category objects in the m_pCategory array.
     */
    virtual
    ~AsterixDefinition();

    /**
     * @brief Get a category by its number
     *
     * @param i Category number (0-255 for standard categories, 256 for BDS registers)
     * @return Pointer to Category object if loaded, nullptr if not defined
     *
     * @note The returned pointer is owned by AsterixDefinition.
     *       Do not delete it.
     *
     * @par Example
     * @code
     * Category* cat = globalDef.getCategory(62);
     * if (cat) {
     *     std::cout << "Category " << cat->m_id << ": "
     *               << cat->m_strName << " v" << cat->m_strVer << std::endl;
     * }
     * @endcode
     */
    Category *getCategory(int i);

    /**
     * @brief Register a category in the global registry
     *
     * @param newCategory Pointer to Category object to register.
     *                    Ownership is transferred to AsterixDefinition.
     *
     * @note The category's m_id field determines which slot it occupies.
     *       If a category with that ID already exists, the old one is deleted.
     *
     * @warning After calling this, do not delete newCategory. It is owned
     *          by the AsterixDefinition and will be freed in the destructor.
     *
     * @par Example
     * @code
     * Category* cat = new Category(62);
     * // ... populate category via XMLParser ...
     * globalDef.setCategory(cat);  // Transfer ownership
     * @endcode
     */
    void setCategory(Category *newCategory);

    /**
     * @brief Check if a category is loaded
     *
     * @param i Category number to check (0-255, or 256 for BDS)
     * @return true if the category is defined (not nullptr), false otherwise
     *
     * @par Example
     * @code
     * if (globalDef.CategoryDefined(48)) {
     *     // Safe to call getCategory(48) and use it
     * }
     * @endcode
     */
    bool CategoryDefined(int i);

    /**
     * @brief Generate a printable list of all loaded category descriptors
     *
     * @return String containing all category IDs, names, and versions
     *
     * @note Used for debugging and diagnostics to show what categories
     *       are currently loaded.
     */
    std::string printDescriptors();

    /**
     * @brief Mark a specific item/field in a category for filtering
     *
     * @param cat  Category number (0-255)
     * @param item Data item ID (e.g., "010")
     * @param name Field name within the item (or nullptr for entire item)
     * @return true if filtering was applied successfully
     *
     * @note Delegates to Category::filterOutItem() for the specified category.
     *       Used by CLI filter mechanism to show only specific data items.
     */
    bool filterOutItem(int cat, std::string item, const char *name);

    /**
     * @brief Check if a specific item/field in a category is filtered
     *
     * @param cat  Category number (0-255)
     * @param item Data item ID (e.g., "010")
     * @param name Field name within the item (or nullptr for entire item)
     * @return true if the item/field should be displayed (passes filter)
     *
     * @note Delegates to Category::isFiltered() for the specified category.
     */
    bool isFiltered(int cat, std::string item, const char *name);

    /**
     * @brief Get human-readable description for a field value in any category
     *
     * @param category Category number (0-255, or 256 for BDS)
     * @param item     Data item ID (e.g., "010")
     * @param field    Field name within the item (e.g., "SAC")
     * @param value    Value to look up (e.g., "5")
     * @return String description from XML value mappings, or nullptr if not found
     *
     * @par Example
     * @code
     * const char* desc = globalDef.getDescription(62, "080", "TYP", "0");
     * if (desc) {
     *     printf("Track Type: %s\n", desc);  // "Confirmed track"
     * }
     * @endcode
     *
     * @note Delegates to Category::getDescription() for the specified category.
     *       The returned pointer points to internal string storage and
     *       should not be freed by the caller.
     */
    const char *getDescription(int category, const char *item, const char *field, const char *value);

private:
    /**
     * @brief Array of category pointers indexed by category number
     *
     * - Indices 0-255: Standard ASTERIX categories
     * - Index 256: BDS (Comm-B Data Buffer) register definitions
     *
     * Entries are nullptr for undefined categories.
     * All non-null entries are owned by AsterixDefinition and deleted in destructor.
     */
    Category *m_pCategory[MAX_CATEGORIES];
};

#endif /* ASTERIXDEFINITION_H_ */
