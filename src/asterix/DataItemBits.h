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
 * @file DataItemBits.h
 * @brief Bit-level field descriptor for ASTERIX data item parsing
 *
 * This file defines the DataItemBits class which represents individual bit-level
 * fields within ASTERIX data items. It handles extraction of specific bit ranges
 * from binary data and conversion to various data types (unsigned, signed, ASCII,
 * octal, six-bit character codes, etc.).
 */

#ifndef DATAITEMBITS_H_
#define DATAITEMBITS_H_

#include <string>
#include <list>

/**
 * @class BitsValue
 * @brief Value-to-description mapping for bit field enumerated values
 *
 * BitsValue represents a single entry in a value lookup table for bit fields.
 * It maps a numeric value to its human-readable description as defined in the
 * ASTERIX XML category specifications.
 *
 * @par Example
 * For a "Target Type" field:
 * - Value 0 → "Unknown"
 * - Value 1 → "Aircraft"
 * - Value 2 → "Ground Vehicle"
 */
class BitsValue {
public:
    /**
     * @brief Construct a BitsValue with only a numeric value
     * @param val The numeric value
     */
    explicit BitsValue(int val) : m_nVal(val) {}

    /**
     * @brief Construct a BitsValue with value and description
     * @param val The numeric value
     * @param strDescription Human-readable description of this value
     */
    BitsValue(int val, std::string strDescription)
        : m_nVal(val), m_strDescription(std::move(strDescription)) {}

    int m_nVal;                    //!< Numeric value (e.g., 0, 1, 2)
    std::string m_strDescription;  //!< Human-readable description (e.g., "Aircraft", "Ground Vehicle")
};

/**
 * @class DataItemBits
 * @brief Bit-level field descriptor for extracting specific bit ranges from ASTERIX data
 *
 * DataItemBits represents individual bit-level fields within ASTERIX data items. It handles
 * extraction of specific bit ranges from binary data and conversion to various data types.
 *
 * @par Bit Numbering Convention
 * ASTERIX uses MSB-first bit numbering within each octet:
 * - Bit 8 (0x80) is the most significant bit (leftmost)
 * - Bit 1 (0x01) is the least significant bit (rightmost)
 * - Multi-octet fields maintain this convention across octets
 *
 * @par Encoding Types
 * DataItemBits supports multiple encoding types via the _eEncoding enum:
 * - DATAITEM_ENCODING_UNSIGNED: Unsigned integer (e.g., Track Number)
 * - DATAITEM_ENCODING_SIGNED: Two's complement signed integer (e.g., Latitude/Longitude)
 * - DATAITEM_ENCODING_SIX_BIT_CHAR: 6-bit character encoding (ICAO alphabet)
 * - DATAITEM_ENCODING_HEX_BIT_CHAR: Hexadecimal character representation
 * - DATAITEM_ENCODING_OCTAL: Octal representation (e.g., Mode A codes)
 * - DATAITEM_ENCODING_ASCII: 7-bit ASCII character encoding
 *
 * @par Scaling and Units
 * Physical values are computed using the formula:
 * @code
 * physical_value = raw_value * m_dScale
 * @endcode
 * - m_dScale: Scaling factor (e.g., 180/2^23 for WGS-84 latitude)
 * - m_strUnit: Unit string (e.g., "deg", "m", "m/s", "FL")
 * - m_dMinValue, m_dMaxValue: Optional range constraints for validation
 *
 * @par Field Extension Bits (FX)
 * Variable-length items use the FX bit (m_bExtension = true) to indicate continuation:
 * - FX = 0: Last octet of the item
 * - FX = 1: Additional octet follows
 * The FX bit is typically bit 1 (LSB) of each octet.
 *
 * @par Example Usage
 * Extracting SAC/SIC (System Area Code / System Identification Code) from I010:
 * @code
 * // I010 structure: 2 octets
 * // Octet 1, bits 8-1: SAC (System Area Code)
 * // Octet 2, bits 8-1: SIC (System Identification Code)
 *
 * DataItemBits sacField(1);
 * sacField.m_strShortName = "SAC";
 * sacField.m_strName = "System Area Code";
 * sacField.m_nFrom = 16;  // Start at bit 16 (octet 1, bit 8)
 * sacField.m_nTo = 9;     // End at bit 9 (octet 1, bit 1)
 * sacField.m_eEncoding = DATAITEM_ENCODING_UNSIGNED;
 *
 * DataItemBits sicField(2);
 * sicField.m_strShortName = "SIC";
 * sicField.m_strName = "System Identification Code";
 * sicField.m_nFrom = 8;   // Start at bit 8 (octet 2, bit 8)
 * sicField.m_nTo = 1;     // End at bit 1 (octet 2, bit 1)
 * sicField.m_eEncoding = DATAITEM_ENCODING_UNSIGNED;
 *
 * // Parse binary data [0x0A, 0x14] -> SAC=10, SIC=20
 * unsigned char data[] = {0x0A, 0x14};
 * std::string result, header;
 * sacField.getText(result, header, CAsterixFormatDescriptor::FORMAT_UTR, data, 2);
 * // result contains: "SAC=10"
 * @endcode
 *
 * @par Thread Safety
 * This class is NOT thread-safe. External synchronization required for concurrent access.
 *
 * @see DataItemFormat
 * @see BitsValue
 */
class DataItemBits : public DataItemFormat {
public:
    /**
     * @brief Construct a DataItemBits descriptor
     * @param id Unique identifier for this bit field
     */
    DataItemBits(int id = 0);

    /**
     * @brief Copy constructor
     * @param obj Source object to copy from
     */
    DataItemBits(const DataItemBits &obj);

    /**
     * @brief Destructor - cleans up BitsValue entries in m_lValue
     */
    virtual
    ~DataItemBits();

    std::string m_strShortName;  //!< Short name for this field (e.g., "SAC", "SIC", "TYP")
    std::string m_strName;       //!< Full descriptive name (e.g., "System Area Code")
    int m_nFrom;                 //!< Starting bit position (MSB-first, e.g., 16 for octet 1 bit 8)
    int m_nTo;                   //!< Ending bit position (MSB-first, e.g., 9 for octet 1 bit 1)

    /**
     * @brief Encoding type enumeration for bit field interpretation
     */
    typedef enum {
        DATAITEM_ENCODING_UNSIGNED = 0,  //!< Unsigned integer (e.g., Track Number)
        DATAITEM_ENCODING_SIGNED,        //!< Two's complement signed integer (e.g., Latitude)
        DATAITEM_ENCODING_SIX_BIT_CHAR,  //!< 6-bit ICAO character encoding
        DATAITEM_ENCODING_HEX_BIT_CHAR,  //!< Hexadecimal character representation
        DATAITEM_ENCODING_OCTAL,         //!< Octal representation (e.g., Mode A codes)
        DATAITEM_ENCODING_ASCII          //!< 7-bit ASCII character encoding
    } _eEncoding;

    _eEncoding m_eEncoding;  //!< Encoding type for this bit field

    bool m_bIsConst;  //!< If true, this field has a fixed constant value (from m_nConst)
    int m_nConst;     //!< Constant value when m_bIsConst is true

    std::string m_strUnit;  //!< Physical unit string (e.g., "deg", "m", "m/s", "FL")
    double m_dScale;        //!< Scaling factor: physical_value = raw_value * m_dScale
    bool m_bMaxValueSet;    //!< True if m_dMaxValue contains a valid maximum constraint
    double m_dMaxValue;     //!< Maximum allowed value (when m_bMaxValueSet is true)
    bool m_bMinValueSet;    //!< True if m_dMinValue contains a valid minimum constraint
    double m_dMinValue;     //!< Minimum allowed value (when m_bMinValueSet is true)

    bool m_bExtension;          //!< True if this is an FX (extension) bit for variable-length items
    int m_nPresenceOfField;     //!< Presence indicator for compound data items (bit mask)
    std::list<BitsValue *> m_lValue;  //!< List of value-to-description mappings for enumerated fields

    bool m_bFiltered;  //!< True if this item should be printed when output filter is applied

    /**
     * @brief Create a deep copy of this DataItemBits object
     * @return Pointer to newly allocated clone (caller owns memory)
     */
    DataItemBits *clone() const override { return new DataItemBits(*this); }

    /**
     * @brief Extract and format the bit field value from binary data
     * @param strResult Output string to append formatted value to
     * @param strHeader Output string to append field header/name to
     * @param formatType Output format type (e.g., FORMAT_UTR, FORMAT_JSON)
     * @param pData Pointer to binary data buffer
     * @param nLength Length of data buffer in bytes
     * @return true if extraction successful, false otherwise
     *
     * @par Example
     * @code
     * unsigned char data[] = {0x0A, 0x14};
     * std::string result, header;
     * sacField.getText(result, header, CAsterixFormatDescriptor::FORMAT_UTR, data, 2);
     * // result: "SAC=10", header: "SAC"
     * @endcode
     */
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength);

    /**
     * @brief Generate a human-readable description of this bit field's format
     * @param header Prefix string for indentation/hierarchy
     * @return Formatted descriptor string
     */
    std::string printDescriptors(std::string header);

    /**
     * @brief Mark this bit field for filtering (selective output)
     * @param name Field name to match for filtering
     * @return true if field matches and was marked, false otherwise
     */
    bool filterOutItem(const char *name);

    /**
     * @brief Check if this bit field is marked for filtered output
     * @param Unused parameter (reserved for future use)
     * @return true if field should be printed when filter is active
     */
    bool isFiltered(const char *) { return m_bFiltered; }

    /**
     * @brief Runtime type identification - always returns true for DataItemBits
     * @return true (this is a bit-level descriptor)
     */
    bool isBits() const override { return true; }

    /**
     * @brief Get human-readable description for a specific field value
     * @param field Field name (e.g., "TYP")
     * @param value String representation of value (e.g., "1")
     * @return Description string from m_lValue lookup, or nullptr if not found
     *
     * @par Example
     * @code
     * // For a "Target Type" field with BitsValue entries:
     * // 0 -> "Unknown", 1 -> "Aircraft", 2 -> "Ground Vehicle"
     * const char* desc = typField.getDescription("TYP", "1");
     * // Returns: "Aircraft"
     * @endcode
     */
    const char *getDescription(const char *field, const char *value);

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    /**
     * @brief Generate Wireshark field definitions for this bit field
     * @return Pointer to Wireshark/Ethereal field definition structure
     * @note Only available when compiled with WIRESHARK_WRAPPER or ETHEREAL_WRAPPER defined
     */
    fulliautomatix_definitions* getWiresharkDefinitions();

    /**
     * @brief Extract data for Wireshark dissector
     * @param pData Pointer to binary data buffer
     * @param len Length of data buffer
     * @param byteoffset Byte offset within the packet
     * @return Pointer to Wireshark data structure
     * @note Only available when compiled with WIRESHARK_WRAPPER or ETHEREAL_WRAPPER defined
     */
    fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif

#if defined(PYTHON_WRAPPER)
    /**
     * @brief Extract bit field value as Python object
     * @param pData Pointer to binary data buffer
     * @param nLength Length of data buffer
     * @param description If non-zero, include description in output (for extensive JSON)
     * @return PyObject* containing the extracted value (PyLong, PyFloat, PyUnicode, or PyDict)
     * @note Only available when compiled with PYTHON_WRAPPER defined
     * @warning Caller must handle Python reference counting (object is a new reference)
     */
    PyObject* getObject(unsigned char* pData, long nLength, int description);

    /**
     * @brief Insert bit field value into Python dictionary
     * @param p Python dictionary object to insert into
     * @param pData Pointer to binary data buffer
     * @param nLength Length of data buffer
     * @param description If non-zero, include description in output (for extensive JSON)
     * @note Only available when compiled with PYTHON_WRAPPER defined
     * @warning Assumes p is a valid PyDict object (no type checking)
     */
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int description);
#endif

    /**
     * @brief Calculate the length of this bit field in bytes
     * @param pData Pointer to binary data (may be used for variable-length items)
     * @return Length in bytes
     * @note For fixed bit fields, returns ceil((m_nFrom - m_nTo + 1) / 8)
     */
    long getLength(const unsigned char *pData);

private:
    // Helper methods for getText() to reduce cognitive complexity
    void appendOpeningTag(std::ostringstream& ss, const unsigned int formatType) const;
    void appendClosingTag(std::ostringstream& ss, const unsigned int formatType) const;
    const char* findValueDescription(unsigned long long value, bool& found) const;
    void formatUnsignedWithMeta(std::ostringstream& ss, unsigned long long value64,
                                const unsigned int formatType, const std::string& strHeader,
                                unsigned char* pData, long nLength);
    void formatSignedWithMeta(std::ostringstream& ss, signed long value,
                              const unsigned int formatType, const std::string& strHeader,
                              unsigned char* pData, long nLength);
    void formatStringEncoding(std::ostringstream& ss, const unsigned char* str,
                              unsigned char* pData, long nLength,
                              const unsigned int formatType, const std::string& strHeader);

    // Unified helper for string encoding - eliminates code duplication across getText(),
    // createWiresharkStringData(), and insertStringToDict()
    char* getEncodedString(_eEncoding encoding, unsigned char* pData, long nLength);

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    // Helper methods for getData() (Wireshark) to reduce cognitive complexity
    char* createWiresharkValueDescription(double scaled, unsigned long long value64);
    fulliautomatix_data* createWiresharkUnsignedData(unsigned char* pData, long nLength,
                                                     int byteoffset, int firstByte,
                                                     int numberOfBits, int numberOfBytes);
    fulliautomatix_data* createWiresharkSignedData(unsigned char* pData, long nLength,
                                                   int byteoffset, int firstByte,
                                                   int numberOfBytes);
    fulliautomatix_data* createWiresharkStringData(_eEncoding encoding, unsigned char* pData,
                                                   long nLength, int byteoffset,
                                                   int firstByte, int numberOfBytes);
#endif

#if defined(PYTHON_WRAPPER)
    // Helper methods for insertToDict() (Python) to reduce cognitive complexity
    void addPyDictItem(PyObject* dict, const char* key, PyObject* value);
    void insertUnsignedToDict(PyObject* pValue, unsigned long long value64, int verbose);
    void insertSignedToDict(PyObject* pValue, signed long value, int verbose);
    void insertStringToDict(PyObject* pValue, _eEncoding encoding,
                            unsigned char* pData, long nLength);
#endif

    /**
     * @brief Extract raw bits from binary data
     * @param pData Pointer to binary data buffer
     * @param bytes Number of bytes in the data buffer
     * @param frombit Starting bit position (MSB-first, 1-based)
     * @param tobit Ending bit position (MSB-first, 1-based)
     * @return Pointer to newly allocated byte array containing extracted bits (caller must free)
     * @note Bits are extracted and right-aligned in the returned buffer
     */
    unsigned char *getBits(unsigned char *pData, int bytes, int frombit, int tobit);

    /**
     * @brief Extract unsigned integer value from bit range
     * @param pData Pointer to binary data buffer
     * @param bytes Number of bytes in the data buffer
     * @param frombit Starting bit position (MSB-first, 1-based)
     * @param tobit Ending bit position (MSB-first, 1-based)
     * @return Unsigned integer value (up to 32 bits)
     * @note Used for fields like Track Number, SAC, SIC, etc.
     */
    unsigned long getUnsigned(unsigned char *pData, int bytes, int frombit, int tobit);

    /**
     * @brief Extract 64-bit unsigned integer value from bit range
     * @param pData Pointer to binary data buffer
     * @param bytes Number of bytes in the data buffer
     * @param frombit Starting bit position (MSB-first, 1-based)
     * @param tobit Ending bit position (MSB-first, 1-based)
     * @return 64-bit unsigned integer value
     * @note Used for large fields that exceed 32 bits
     */
    unsigned long long getUnsigned64(unsigned char *pData, int bytes, int frombit, int tobit);

    /**
     * @brief Extract signed integer value from bit range (two's complement)
     * @param pData Pointer to binary data buffer
     * @param bytes Number of bytes in the data buffer
     * @param frombit Starting bit position (MSB-first, 1-based)
     * @param tobit Ending bit position (MSB-first, 1-based)
     * @return Signed integer value (two's complement encoding)
     * @note Used for fields like Latitude, Longitude, Rate of Climb/Descent
     */
    signed long getSigned(unsigned char *pData, int bytes, int frombit, int tobit);

    /**
     * @brief Extract 6-bit character string (ICAO alphabet)
     * @param pData Pointer to binary data buffer
     * @param bytes Number of bytes in the data buffer
     * @param frombit Starting bit position (MSB-first, 1-based)
     * @param tobit Ending bit position (MSB-first, 1-based)
     * @return Pointer to null-terminated ASCII string (caller must free)
     * @note 6-bit encoding: 0=' ', 1-26='A'-'Z', 48-57='0'-'9'
     * @warning Used for aircraft callsigns and other ICAO-encoded fields
     */
    unsigned char *getSixBitString(unsigned char *pData, int bytes, int frombit, int tobit);

    /**
     * @brief Extract hexadecimal string representation
     * @param pData Pointer to binary data buffer
     * @param bytes Number of bytes in the data buffer
     * @param frombit Starting bit position (MSB-first, 1-based)
     * @param tobit Ending bit position (MSB-first, 1-based)
     * @return Pointer to null-terminated hex string (e.g., "A5F3") (caller must free)
     */
    unsigned char *getHexBitString(unsigned char *pData, int bytes, int frombit, int tobit);

    /**
     * @brief Extract octal string representation
     * @param pData Pointer to binary data buffer
     * @param bytes Number of bytes in the data buffer
     * @param frombit Starting bit position (MSB-first, 1-based)
     * @param tobit Ending bit position (MSB-first, 1-based)
     * @return Pointer to null-terminated octal string (e.g., "7654") (caller must free)
     * @note Used primarily for Mode A codes (SSR transponder codes)
     */
    unsigned char *getOctal(unsigned char *pData, int bytes, int frombit, int tobit);

    /**
     * @brief Extract hexadecimal string for full bytes only
     * @param pData Pointer to binary data buffer
     * @param bytes Number of bytes in the data buffer
     * @param frombit Starting bit position (must be byte-aligned)
     * @param tobit Ending bit position (must be byte-aligned)
     * @return Pointer to null-terminated hex string (caller must free)
     * @warning frombit and tobit must align to byte boundaries
     */
    unsigned char *getHexBitStringFullByte(unsigned char *pData, int bytes, int frombit, int tobit);

    /**
     * @brief Generate bit mask for hexadecimal extraction
     * @param bytes Number of bytes in the field
     * @param frombit Starting bit position (MSB-first, 1-based)
     * @param tobit Ending bit position (MSB-first, 1-based)
     * @return Pointer to byte array containing the bit mask (caller must free)
     * @note Used internally by getHexBitString to mask out irrelevant bits
     */
    unsigned char *getHexBitStringMask(int bytes, int frombit, int tobit);

    /**
     * @brief Extract ASCII character string
     * @param pData Pointer to binary data buffer
     * @param bytes Number of bytes in the data buffer
     * @param frombit Starting bit position (MSB-first, 1-based)
     * @param tobit Ending bit position (MSB-first, 1-based)
     * @return Pointer to null-terminated ASCII string (caller must free)
     * @note Used for 7-bit or 8-bit ASCII-encoded text fields
     */
    char *getASCII(unsigned char *pData, int bytes, int frombit, int tobit);


};

#endif /* DATAITEMBITS_H_ */
