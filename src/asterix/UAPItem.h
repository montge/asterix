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
 * @file UAPItem.h
 * @brief UAP item entry - single FSPEC bit to data item mapping
 *
 * This file defines the UAPItem class which represents a single entry in
 * a UAP (User Application Profile), mapping one FSPEC bit position to a
 * specific data item ID.
 */

#ifndef UAPITEM_H_
#define UAPITEM_H_

#include "DataItemFormat.h"
#include "Tracer.h"


/**
 * @class UAPItem
 * @brief Single UAP entry mapping FSPEC bit to data item ID
 *
 * UAPItem represents one entry in a UAP, defining the relationship between:
 * - A bit position in the FSPEC bitmap
 * - A Field Reference Number (FRN)
 * - The data item ID (e.g., "010" for SAC/SIC)
 * - Whether this bit is the FX (Field Extension) bit
 *
 * UAPItem inherits from DataItemFormat for historical reasons (to use the
 * base class polymorphism), but most DataItemFormat methods are not applicable
 * and will log errors if called.
 *
 * @par FSPEC Mapping Example
 * For CAT062, the first FSPEC byte might map as:
 * @code
 * Bit 7 (FRN 1) -> "010" (SAC/SIC)
 * Bit 6 (FRN 2) -> "015" (Service Identification)
 * Bit 5 (FRN 3) -> "070" (Time of Track Information)
 * Bit 4 (FRN 4) -> "105" (Calculated Position)
 * ...
 * Bit 0 (FX)    -> Field Extension (not a data item)
 * @endcode
 *
 * @par Memory Management
 * - UAPItem objects are owned by the UAP class
 * - Deleted when UAP is destroyed
 *
 * @par Thread Safety
 * This class is NOT thread-safe. Do not access the same UAPItem instance
 * from multiple threads concurrently.
 *
 * @warning Most DataItemFormat virtual methods are not implemented and will
 *          log errors if called. UAPItem is NOT a format parser, despite
 *          inheriting from DataItemFormat.
 *
 * @see UAP For the container of UAPItem objects
 * @see DataItemFormat For the base class (note: most methods not applicable)
 */
class UAPItem : public DataItemFormat {
public:
    /**
     * @brief Construct an empty UAPItem
     *
     * Initializes all members to default values.
     * Members are set later during XML parsing.
     */
    UAPItem();

    /**
     * @brief Copy constructor
     *
     * @param obj UAPItem to copy from
     *
     * @note Used by clone() method.
     */
    UAPItem(const UAPItem &obj);

    /**
     * @brief Destructor
     */
    virtual
    ~UAPItem();

    /**
     * @brief Bit position in FSPEC byte (0-7)
     *
     * Position of this item in the FSPEC byte:
     * - 7: Bit 7 (first data item)
     * - 6: Bit 6 (second data item)
     * - ...
     * - 1: Bit 1 (seventh data item)
     * - 0: Bit 0 (FX - Field Extension bit)
     *
     * From XML attribute: <!ATTLIST UAPItem bit CDATA #REQUIRED>
     */
    int m_nBit;

    /**
     * @brief Field Reference Number (FRN) - sequential item number
     *
     * 1-based index of this item in the UAP sequence.
     * FRN 1 = first data item, FRN 2 = second data item, etc.
     *
     * Used to look up data items by their UAP position.
     *
     * From XML attribute: <!ATTLIST UAPItem frn CDATA #REQUIRED>
     */
    int m_nFRN;

    /**
     * @brief FX flag - true if this is the Field Extension bit
     *
     * The FX (Field Extension) bit indicates whether another FSPEC byte follows:
     * - FX=1: Another FSPEC byte follows
     * - FX=0: This is the last FSPEC byte
     *
     * The FX bit is always at position 0 (bit 0) in each FSPEC byte.
     *
     * From XML attribute: <!ATTLIST UAPItem fx CDATA "0">
     */
    bool m_bFX;

    /**
     * @brief Length of the data item in bytes (-1 if variable)
     *
     * - Positive value: Fixed-length item (e.g., 2 for SAC/SIC)
     * - -1: Variable-length item (length determined by item format)
     *
     * From XML attribute: <!ATTLIST UAPItem len CDATA "-">
     */
    int m_nLen;

    /**
     * @brief Data item ID string (e.g., "010", "040", "105")
     *
     * Three-digit ID of the data item this UAP entry references.
     * Empty for FX bit (m_bFX = true).
     *
     * From XML element: <!ELEMENT UAPItem (#PCDATA)>
     */
    std::string m_strItemID;

    /**
     * @brief Create a deep copy of this UAPItem
     *
     * @return Pointer to newly allocated UAPItem clone
     *
     * @note Implements DataItemFormat::clone() interface.
     */
    UAPItem *clone() const override { return new UAPItem(*this); }

    /**
     * @brief NOT APPLICABLE - logs error if called
     *
     * UAPItem is not a data parser, this method should not be called.
     *
     * @return Always returns 0 after logging error
     */
    long getLength(const unsigned char *) {
        Tracer::Error("UAPItem::getLength() should not be called!");
        return 0;
    }

    /**
     * @brief NOT APPLICABLE - logs error if called
     *
     * UAPItem is not a data parser, this method should not be called.
     *
     * @return Always returns false after logging error
     */
    bool getText(std::string &, std::string &, const unsigned int, unsigned char *, long) {
        Tracer::Error("UAPItem::getText() should not be called!");
        return false;
    }

    /**
     * @brief NOT APPLICABLE - logs error if called
     *
     * UAPItem is not a data parser, this method should not be called.
     *
     * @return Always returns empty string after logging error
     */
    std::string printDescriptors(std::string) {
        Tracer::Error("UAPItem::printDescriptors() should not be called!");
        return "";
    };

    /**
     * @brief NOT APPLICABLE - logs error if called
     *
     * UAPItem is not a data parser, this method should not be called.
     *
     * @return Always returns false after logging error
     */
    bool filterOutItem(const char *) {
        Tracer::Error("UAPItem::filterOutItem() should not be called!");
        return false;
    };

    /**
     * @brief NOT APPLICABLE - logs error if called
     *
     * UAPItem is not a data parser, this method should not be called.
     *
     * @return Always returns false after logging error
     */
    bool isFiltered(const char *) {
        Tracer::Error("UAPItem::isFiltered() should not be called!");
        return false;
    };

    /**
     * @brief NOT APPLICABLE - logs error if called
     *
     * UAPItem is not a data parser, this method should not be called.
     *
     * @return Always returns nullptr after logging error
     */
    const char *getDescription(const char * /*field*/, const char * /*value*/) {
        Tracer::Error("UAPItem::getDescription() should not be called!");
        return nullptr;
    };

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

    /**
     * @brief Get Wireshark dissector data structure (Wireshark plugin only)
     *
     * @param pData      Pointer to binary ASTERIX data
     * @param len        Length of data in bytes
     * @param byteoffset Byte offset in the packet for Wireshark display
     * @return Pointer to fulliautomatix_data structure for Wireshark tree
     *
     * @note This method is only available when compiled with WIRESHARK_WRAPPER
     *       or ETHEREAL_WRAPPER defined (plugin builds).
     */
    fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif
#if defined(PYTHON_WRAPPER)
    /**
     * @brief NOT APPLICABLE - returns nullptr (Python binding only)
     *
     * UAPItem is not a data parser, this method is not applicable.
     *
     * @return Always returns nullptr
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     */
    PyObject* getObject(unsigned char* pData, long nLength, int description) { return nullptr; };

    /**
     * @brief NOT APPLICABLE - no-op (Python binding only)
     *
     * UAPItem is not a data parser, this method is not applicable.
     *
     * @note This method is only available when compiled with PYTHON_WRAPPER
     *       defined (Python C extension module build).
     */
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int description) {};
#endif
};

#endif /* UAPITEM_H_ */
