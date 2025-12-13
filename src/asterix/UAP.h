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
 * @file UAP.h
 * @brief User Application Profile - FSPEC bit to data item mapping
 *
 * This file defines the UAP (User Application Profile) class which maps
 * FSPEC bitmap bits to specific ASTERIX data items according to the
 * category definition from XML.
 */

#ifndef UAP_H_
#define UAP_H_

#include "UAPItem.h"

/**
 * @class UAP
 * @brief User Application Profile - maps FSPEC bits to data items
 *
 * The UAP defines the mapping between FSPEC (Field Specification) bitmap bits
 * and ASTERIX data items. Each category can have one or more UAPs, selected
 * conditionally based on specific FSPEC bits.
 *
 * @par FSPEC Structure
 * The FSPEC is a variable-length bitmap at the start of each ASTERIX record:
 * @code
 * Bit:  7   6   5   4   3   2   1   0
 *      [FRN1][FRN2][FRN3][FRN4][FRN5][FRN6][FRN7][FX]
 *       Item Item Item Item Item Item Item  Extension
 *        #1   #2   #3   #4   #5   #6   #7    bit
 * @endcode
 *
 * - Bits 7-1: Indicate presence of data items (FRN - Field Reference Number)
 * - Bit 0 (FX): Field Extension - if set (1), another FSPEC byte follows
 *
 * @par UAP Selection
 * Some categories have multiple UAPs (e.g., CAT048 has different UAPs for
 * different modes). The UAP is selected based on:
 * - m_nUseIfByteNr: Which FSPEC byte to check
 * - m_nUseIfBitSet: Which bit in that byte must be set
 * - m_nIsSetTo: Expected value (0 or 1)
 *
 * @par Memory Management
 * - UAP owns all UAPItem objects in m_lUAPItems
 * - Owned by Category class and deleted when Category is destroyed
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same UAP instance
 * from multiple threads concurrently.
 *
 * @par Example Usage
 * @code
 * // Typical usage (internal to ASTERIX decoder):
 * Category* cat = globalDef.getCategory(62);
 * const unsigned char fspec[] = {0xFD, 0x00};  // FSPEC bytes
 * UAP* uap = cat->getUAP(fspec, sizeof(fspec));
 *
 * if (uap) {
 *     // Get data item ID for FSPEC bit position (FRN)
 *     std::string itemId = uap->getDataItemIDByUAPfrn(1);  // First item
 *     // Returns "010" for I062/010 (SAC/SIC)
 * }
 * @endcode
 *
 * @see UAPItem For individual UAP entry (bit -> item ID mapping)
 * @see Category For UAP container and selection logic
 * @see DataRecord For FSPEC parsing that uses UAP
 */
class UAP {
public:
    /**
     * @brief Construct an empty UAP
     *
     * Initializes selection criteria to defaults (use unconditionally).
     * UAPItems are added later via newUAPItem() during XML parsing.
     */
    UAP();

    /**
     * @brief Destructor - frees all UAP items
     *
     * Deletes all UAPItem objects in m_lUAPItems list.
     */
    virtual
    ~UAP();

    /**
     * @brief Bit position to check for UAP selection (0-7)
     *
     * Which bit in byte m_nUseIfByteNr must match m_nIsSetTo
     * for this UAP to be selected.
     *
     * @note Value of 0 means "use this UAP unconditionally" (most common).
     */
    unsigned long m_nUseIfBitSet;

    /**
     * @brief FSPEC byte number to check for UAP selection (0-based)
     *
     * Which FSPEC byte contains the selection bit.
     * - 0: First FSPEC byte
     * - 1: Second FSPEC byte (if FX bit was set in byte 0)
     * - etc.
     *
     * @note Value of 0 with m_nUseIfBitSet=0 means "use unconditionally".
     */
    unsigned long m_nUseIfByteNr;

    /**
     * @brief Expected bit value for UAP selection (0 or 1)
     *
     * The bit at m_nUseIfByteNr[m_nUseIfBitSet] must equal this value
     * for this UAP to be selected.
     *
     * @note Common values:
     *       - 0: Select this UAP if bit is NOT set
     *       - 1: Select this UAP if bit IS set
     */
    unsigned char m_nIsSetTo;

    /**
     * @brief List of UAP items (FRN -> data item ID mappings)
     *
     * Ordered list of UAPItem objects, each mapping a Field Reference Number
     * (FRN) to a data item ID. The list order corresponds to FSPEC bit order.
     *
     * These objects are owned by the UAP and deleted in destructor.
     *
     * @par Example
     * For CAT062, first few items might be:
     * - FRN 1 -> "010" (SAC/SIC)
     * - FRN 2 -> "015" (Service Identification)
     * - FRN 3 -> "070" (Time of Track Information)
     * - etc.
     */
    std::list<UAPItem *> m_lUAPItems;

    /**
     * @brief Create and add a new UAP item to this UAP
     *
     * @return Pointer to newly created UAPItem, owned by this UAP
     *
     * @note Used during XML parsing to populate the UAP.
     *       The UAPItem is added to m_lUAPItems and owned by this UAP.
     */
    UAPItem *newUAPItem();

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

    /**
     * @brief Get data item ID by Field Reference Number (FRN)
     *
     * @param uapfrn Field Reference Number (1-based index into UAP)
     *               Corresponds to position in FSPEC bitmap.
     * @return Data item ID string (e.g., "010"), or empty string if not found
     *
     * @note FRN is 1-based (FRN 1 = first item, FRN 2 = second item, etc.).
     *       This differs from 0-based array indexing.
     *
     * @par Example
     * @code
     * std::string itemId = uap->getDataItemIDByUAPfrn(1);
     * // For CAT062: returns "010" (SAC/SIC)
     *
     * std::string itemId3 = uap->getDataItemIDByUAPfrn(3);
     * // For CAT062: returns "070" (Time of Track Information)
     * @endcode
     */
    std::string getDataItemIDByUAPfrn(int uapfrn) const;
};

#endif /* UAP_H_ */
