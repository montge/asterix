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

#ifndef DataItemFormatBDS_H_
#define DataItemFormatBDS_H_

#include "DataItemFormatFixed.h"
#include "cxx23_features.h"

/**
 * @class DataItemFormatBDS
 * @brief Parser for ASTERIX BDS (Binary Data Specification) register data
 *
 * BDS format represents Mode S transponder downlink data registers as defined
 * in ICAO Annex 10, Volume IV. Each BDS register is exactly 8 bytes (56 bits data + 8-bit register ID).
 * The last byte identifies which BDS register type the data represents.
 *
 * ## Structure
 *
 * ```
 * +----------------+
 * | Data Bytes 0-6 | (7 bytes: 56 bits of Mode S data)
 * +----------------+
 * | BDS Register ID| (1 byte: e.g., 0x20 for BDS 2,0)
 * +----------------+
 * Total: Always 8 bytes
 * ```
 *
 * ## Common BDS Registers
 *
 * - **BDS 1,0**: Data link capability report
 * - **BDS 2,0**: Aircraft identification (call sign)
 * - **BDS 3,0**: ACAS resolution advisory report
 * - **BDS 4,0**: Selected vertical intention
 * - **BDS 5,0**: Track and turn report
 * - **BDS 6,0**: Heading and speed report
 *
 * ## Mode S Context
 *
 * BDS registers are transmitted via Mode S downlink messages (Comm-B) in response
 * to interrogations. They provide detailed aircraft state and intent information
 * beyond basic surveillance data.
 *
 * ## Example Usage
 *
 * CAT021 I021/040 (Target Report Descriptor) and I021/020 (Emitter Category)
 * often contain BDS register data extracted from Mode S transponders.
 *
 * @note Always 8 bytes - this is fixed by ICAO standard
 * @note Last byte (pData[7]) identifies the BDS register type
 * @note Parser looks up appropriate BDS definition from XML configuration
 *
 * @see DataItemFormat
 * @see DataItemFormatFixed
 */
class DataItemFormatBDS : public DataItemFormat {
public:
    /**
     * @brief Construct a new BDS format parser
     * @param id Unique identifier for this format instance (default: 0)
     */
    DataItemFormatBDS(int id = 0);

    /**
     * @brief Copy constructor - performs deep copy of BDS definitions
     * @param obj Source BDS format object to copy
     */
    DataItemFormatBDS(const DataItemFormatBDS &obj);

    /**
     * @brief Destructor - cleans up BDS register definitions
     */
    virtual
    ~DataItemFormatBDS();

    /**
     * @brief Create a deep copy of this BDS format object
     * @return Pointer to newly allocated clone (caller must delete)
     * @note C++23: Uses deduced this for better devirtualization
     */
    // C++23 Quick Win: Deduced this allows better devirtualization
#if HAS_DEDUCED_THIS
    DataItemFormatBDS *clone(this const auto& self) const { return new DataItemFormatBDS(self); }
#else
    DataItemFormatBDS *clone() const { return new DataItemFormatBDS(*this); } // Return clone of object
#endif

    /**
     * @brief Get BDS register length (always 8 bytes)
     * @param pData Unused - BDS is always fixed length
     * @return Always returns 8
     * @note BDS registers are always 8 bytes per ICAO specification
     */
    long getLength(const unsigned char *pData);

    /**
     * @brief Extract and format BDS register data as text
     * @param strResult Output string to append formatted value
     * @param strHeader Header string for formatting context
     * @param formatType Output format type (text, JSON, XML, etc.)
     * @param pData Pointer to 8-byte BDS register data
     * @param nLength Length of data buffer (must be 8)
     * @return true if parsing successful, false on error
     * @note Reads pData[7] to identify BDS register type
     * @note Looks up corresponding BDS definition from m_lSubItems
     * @note Returns error if nLength != 8 (invalid BDS data)
     */
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength); // appends value description to strResult

    /**
     * @brief Generate human-readable BDS format descriptor string
     * @param header Prefix string for hierarchical formatting
     * @return String describing all defined BDS registers
     */
    std::string printDescriptors(std::string header); // print items format descriptors

    /**
     * @brief Mark a BDS register field for filtering (exclusion from output)
     * @param name Name/ID of BDS field to filter out
     * @return true if filter applied, false if field not found
     */
    bool filterOutItem(const char *name); // mark item for filtering

    /**
     * @brief Check if a BDS register field is currently filtered
     * @param name Name/ID of field to check
     * @return true if field is filtered (hidden), false otherwise
     */
    bool isFiltered(const char *name);

    /**
     * @brief Type identification - returns true for BDS format
     * @return Always returns true
     * @note Used for runtime type checking in parsing pipeline
     */
    bool isBDS() { return true; }; // true if this is BDS format

    /**
     * @brief Get human-readable description for a BDS field value
     * @param field Name of field within BDS register
     * @param value String representation of value
     * @return Description string, or nullptr if not found
     */
    const char *getDescription(const char *field, const char *value); // return description ef element

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    fulliautomatix_definitions* getWiresharkDefinitions();
    fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif
#if defined(PYTHON_WRAPPER)
    PyObject* getObject(unsigned char* pData, long nLength, int description);
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int description);
#endif
};

#endif /* DataItemFormatBDS_H_ */
