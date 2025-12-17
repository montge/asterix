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

#include "DataItemFormat.h"
#include "DataItemBits.h"
#include "Tracer.h"
#include "Utils.h"
#include "asterixformat.hxx"
#include <sstream>  // PERFORMANCE: For efficient string building
#include <memory>   // For std::unique_ptr

extern bool gFiltering;

// Bit manipulation constants for binary field parsing
namespace {
    constexpr unsigned char BIT_MASK_MSB = 0x80;  // Most significant bit mask
    constexpr unsigned char BIT_MASK_LSB = 0x01;  // Least significant bit mask
    constexpr unsigned char BIT_MASK_CLEAR_MSB = 0x7F;  // Clear MSB, keep lower 7 bits
}

// Helper function to allocate error string with new[] (not strdup/malloc)
// This ensures consistent deallocation with delete[]
static unsigned char* newErrorString(const char* str) {
    if (str == nullptr) {
        str = "???";
    }
    size_t len = strlen(str) + 1;
    auto result = new unsigned char[len];
    memcpy(result, str, len);
    return result;
}

static char* newErrorStringChar(const char* str) {
    if (str == nullptr) {
        str = "???";
    }
    size_t len = strlen(str) + 1;
    auto result = new char[len];
    memcpy(result, str, len);
    return result;
}

static const char SIXBITCODE[] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
                                  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ', ' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ' ', ' ', ' ', ' ', ' ', ' '};

DataItemBits::DataItemBits(int id)
        : DataItemFormat(id), m_nFrom(0), m_nTo(0), m_eEncoding(DATAITEM_ENCODING_UNSIGNED), m_bIsConst(false),
          m_nConst(0), m_dScale(0.0), m_bMaxValueSet(false), m_dMaxValue(0.0), m_bMinValueSet(false), m_dMinValue(0.0),
          m_bExtension(false), m_nPresenceOfField(0), m_bFiltered(false) {

}

DataItemBits::DataItemBits(const DataItemBits &obj)
        : DataItemFormat(obj.m_nID) {
    for (const auto* subItem : obj.m_lSubItems) {
        m_lSubItems.push_back(subItem->clone());
    }

    m_pParentFormat = obj.m_pParentFormat;

    m_strShortName = obj.m_strShortName;
    m_strName = obj.m_strName;
    m_nFrom = obj.m_nFrom;
    m_nTo = obj.m_nTo;
    m_eEncoding = obj.m_eEncoding;
    m_bIsConst = obj.m_bIsConst;
    m_nConst = obj.m_nConst;
    m_strUnit = obj.m_strUnit;
    m_dScale = obj.m_dScale;
    m_bMaxValueSet = obj.m_bMaxValueSet;
    m_dMaxValue = obj.m_dMaxValue;
    m_bMinValueSet = obj.m_bMinValueSet;
    m_dMinValue = obj.m_dMinValue;
    m_bExtension = obj.m_bExtension;
    m_nPresenceOfField = obj.m_nPresenceOfField;

    for (const auto* bv : obj.m_lValue) {
        m_lValue.push_back(new BitsValue(bv->m_nVal, bv->m_strDescription));
    }
    m_bFiltered = obj.m_bFiltered;
}


DataItemBits::~DataItemBits() {
    deleteAndClear(m_lValue);
}

long DataItemBits::getLength(const unsigned char *) {
    Tracer::Error("DataItemBits::getLength Should not be called!");
    return 0;
}

unsigned char *DataItemBits::getBits(unsigned char *pData, int bytes, int frombit, int tobit) {
    /* example:
     * bytenr     2                               1
     * bitnr    [16][15][14][13][12][11][10][ 9][ 8][ 7][ 6][ 5][ 4][ 3][ 2][ 1]
     * value      1   1   0   0   1   0   0   1   1   1   1   0   1   0   0   1
     * getBits(pData, 2, 7, 10)
     *                                    |to         |from
     * returns                            0   1   1   1   0   0   0   0
     *
     */
    int numberOfBits = (tobit - frombit + 1);
    int numberOfBytes = (numberOfBits + 7) / 8;

    if (frombit > tobit || tobit < 1 || frombit < 1 || numberOfBytes > bytes) {
        Tracer::Error("Irregular request for getBits");
        return nullptr;
    }

    auto pVal = std::make_unique<unsigned char[]>(numberOfBytes);
    unsigned char *pTmp = pVal.get();
    memset(pVal.get(), 0, numberOfBytes);

    if (frombit == 1 && tobit == bytes * 8) {
        memcpy(pVal.get(), pData, bytes);
        return pVal.release();
    }

    unsigned char bitmask = BIT_MASK_MSB;
    unsigned char outbits = 0;
    for (int bit = bytes * 8; bit >= frombit; bit--) {
        if (bit <= tobit) {
            unsigned char bitval = *pData & bitmask;
            *pTmp <<= 1;
            if (bitval)
                *pTmp |= BIT_MASK_LSB;

            if (++outbits >= 8) {
                outbits = 0;
                pTmp++;
            }
        }

        bitmask >>= 1;
        bitmask &= BIT_MASK_CLEAR_MSB;

        if (bitmask == 0) {
            bitmask = BIT_MASK_MSB;
            pData++;
        }
    }

    if (pVal.get() + numberOfBytes - 1 >= pTmp)
        *pTmp <<= 8 - outbits;

    return pVal.release();
}

unsigned long DataItemBits::getUnsigned(unsigned char *pData, int bytes, int frombit, int tobit) {
    unsigned long val = 0;
    int numberOfBits = (tobit - frombit + 1);

    if (numberOfBits < 1 || numberOfBits > 32) {
        Tracer::Error(
                "DataItemBits::getUnsigned : Wrong parameter.m Number of bits = %d, and must be between 1 and 32. Currently is from %d to %d",
                numberOfBits, tobit, frombit);
    } else {
        unsigned char *pTmp = getBits(pData, bytes, frombit, tobit);

        if (!pTmp) {
            Tracer::Error("DataItemBits::getUnsigned : Error.");
            return 0;
        }

        if (numberOfBits == 8) {
            val = *pTmp;  // pTmp is already unsigned char*
        } else {
            unsigned char *pTmp2 = pTmp;
            unsigned char bitmask = BIT_MASK_MSB;
            while (numberOfBits--) {
                unsigned char bitval = *pTmp2 & bitmask;
                val <<= 1;
                if (bitval)
                    val |= BIT_MASK_LSB;

                bitmask >>= 1;
                bitmask &= BIT_MASK_CLEAR_MSB;
                if (bitmask == 0) {
                    bitmask = BIT_MASK_MSB;
                    pTmp2++;
                }
            }
        }
        delete[] pTmp;
    }
    return val;
}

unsigned long long DataItemBits::getUnsigned64(unsigned char *pData, int bytes, int frombit, int tobit) {
    unsigned long long val = 0;
    int numberOfBits = (tobit - frombit + 1);

    if (numberOfBits < 1 || numberOfBits > 64) {
        Tracer::Error(
                "DataItemBits::getUnsigned64 : Wrong parameter. Number of bits = %d, and must be between 1 and 64. Currently is from %d to %d",
                numberOfBits, tobit, frombit);
    } else {
        unsigned char *pTmp = getBits(pData, bytes, frombit, tobit);

        if (!pTmp) {
            Tracer::Error("DataItemBits::getUnsigned64 : Error.");
            return 0;
        }

        // Handle byte-aligned 64-bit values (optimization)
        if (numberOfBits == 64) {
            // Read 8 bytes in big-endian order
            for (int i = 0; i < 8; i++) {
                val = (val << 8) | pTmp[i];
            }
        } else {
            // Bit-by-bit extraction for non-byte-aligned or < 64 bits
            unsigned char *pTmp2 = pTmp;
            unsigned char bitmask = BIT_MASK_MSB;
            while (numberOfBits--) {
                unsigned char bitval = *pTmp2 & bitmask;
                val <<= 1;
                if (bitval)
                    val |= BIT_MASK_LSB;

                bitmask >>= 1;
                bitmask &= BIT_MASK_CLEAR_MSB;
                if (bitmask == 0) {
                    bitmask = BIT_MASK_MSB;
                    pTmp2++;
                }
            }
        }
        delete[] pTmp;
    }
    return val;
}

signed long DataItemBits::getSigned(unsigned char *pData, int bytes, int frombit, int tobit) {
    unsigned long ul = getUnsigned(pData, bytes, frombit, tobit);
    int numberOfBits = (tobit - frombit + 1);
    unsigned long maxval = 0x01;
    maxval <<= (numberOfBits - 1);
    if (ul >= maxval) {
        // calculate 2's complement to get negative value
        ul = ~ul;
        ul &= (maxval - 1);
        signed long sl = ul + 1;
        return (-sl);
    }
    return static_cast<signed long>(ul);
}

unsigned char *DataItemBits::getSixBitString(unsigned char *pData, int bytes, int frombit, int tobit) {
    int numberOfBits = (tobit - frombit + 1);
    if (!numberOfBits || numberOfBits % 6) {
        Tracer::Error("Six-bit char representation not valid");
        return newErrorString("???");
    }

    std::unique_ptr<unsigned char[]> pB(getBits(pData, bytes, frombit, tobit));

    if (!pB) {
        Tracer::Error("DATAITEM_ENCODING_SIX_BIT_CHAR : Error.");
        return newErrorString("???");
    }

    int numberOfCharacters = numberOfBits / 6;
    auto str = std::make_unique<unsigned char[]>(numberOfCharacters + 1);
    unsigned char *pStr = str.get();
    memset(str.get(), 0, numberOfCharacters + 1);

    unsigned char *pTmp = pB.get();
    const unsigned char *pBEnd = pB.get() + (numberOfBits + 7) / 8;  // Buffer end for bounds checking
    unsigned char bitmask = 0x80;
    int outbits = 0;
    unsigned char val = 0;
    while (numberOfBits--) {
        // Bounds check: ensure we don't read past buffer
        if (pTmp >= pBEnd) {
            Tracer::Error("Six-bit string: buffer overrun detected");
            return newErrorString("???");
        }
        unsigned char bitval = *pTmp & bitmask;
        val <<= 1;
        if (bitval)
            val |= 0x01;

        bitmask >>= 1;
        bitmask &= 0x7F;
        if (bitmask == 0) {
            bitmask = 0x80;
            pTmp++;
        }

        if (outbits++ == 5) {
            outbits = 0;
            *pStr = SIXBITCODE[val];
            val = 0;
            pStr++;
        }
    }
    return str.release();
}

unsigned char *DataItemBits::getHexBitString(unsigned char *pData, int bytes, int frombit, int tobit) {
    int numberOfBits = (tobit - frombit + 1);
    if (!numberOfBits || numberOfBits % 4) {
        Tracer::Error("Hex representation not valid");
        return newErrorString("???");
    }

    std::unique_ptr<unsigned char[]> pB(getBits(pData, bytes, frombit, tobit));

    if (!pB) {
        Tracer::Error("DATAITEM_ENCODING_HEX_BIT_CHAR : Error.");
        return newErrorString("???");
    }

    int numberOfCharacters = numberOfBits / 4;
    auto str = std::make_unique<unsigned char[]>(numberOfCharacters + 1);

    memset(str.get(), 0, numberOfCharacters + 1);

    int numberOfBytes = (numberOfCharacters + 1) / 2;
    int i;
    for (i = 0; i < numberOfBytes; i++) {
        // Security fix: Use snprintf to prevent buffer overflow
        snprintf(reinterpret_cast<char *>(&str.get()[i * 2]), 3, "%02X", pB[i]);
    }

    return str.release();
}

unsigned char *DataItemBits::getHexBitStringFullByte(unsigned char *pData, int bytes, int frombit, int tobit) {
    int numberOfBits = (tobit - frombit + 1);
    if (!numberOfBits) {
        Tracer::Error("Hex representation not valid");
        return newErrorString("???");
    }

    if (tobit%8) {
        tobit = (tobit/8 + 1)*8;
    }
    if ((frombit-1)%8) {
        frombit = ((frombit-1)/8)*8+1;
    }
    numberOfBits = (tobit - frombit + 1);

    std::unique_ptr<unsigned char[]> pB(getBits(pData, bytes, frombit, tobit));

    if (!pB) {
        Tracer::Error("DATAITEM_ENCODING_HEX_BIT_CHAR : Error.");
        return newErrorString("???");
    }

    int numberOfCharacters = numberOfBits / 4;
    auto str = std::make_unique<unsigned char[]>(numberOfCharacters + 1);

    memset(str.get(), 0, numberOfCharacters + 1);

    int numberOfBytes = (numberOfCharacters + 1) / 2;
    int i;
    for (i = 0; i < numberOfBytes; i++) {
        // Security fix: Use snprintf to prevent buffer overflow
        snprintf(reinterpret_cast<char *>(&str.get()[i * 2]), 3, "%02X", pB[i]);
    }

    return str.release();
}

unsigned char *DataItemBits::getHexBitStringMask([[maybe_unused]] int bytes, int frombit, int tobit) {
    int numberOfBits = (tobit - frombit + 1);
    if (!numberOfBits) {
        Tracer::Error("Hex representation not valid");
        return newErrorString("???");
    }

    int tobitStart = tobit;
    if (tobitStart%8) {
        tobitStart = (tobitStart/8 + 1)*8;
    }
    int frombitStart = frombit;
    if ((frombitStart-1)%8) {
        frombitStart = ((frombitStart-1)/8)*8+1;
    }
    numberOfBits = (tobitStart - frombitStart + 1);

    auto str = std::make_unique<unsigned char[]>(numberOfBits + 1);
    unsigned char *p = str.get();
    int indx = 0;
    for (int i = tobitStart; i >= frombitStart; --i) {
        if (i >= frombit && i <= tobit)
            p[indx++] = '1';
        else
            p[indx++] = '0';
    }
    p[indx] = 0;
    return str.release();
}


unsigned char *DataItemBits::getOctal(unsigned char *pData, int bytes, int frombit, int tobit) {
    int numberOfBits = (tobit - frombit + 1);
    if (!numberOfBits || numberOfBits % 3) {
        Tracer::Error("Octal representation not valid");
        return newErrorString("???");
    }

    std::unique_ptr<unsigned char[]> pB(getBits(pData, bytes, frombit, tobit));

    if (!pB) {
        Tracer::Error("DATAITEM_ENCODING_OCTAL : Error.");
        return newErrorString("???");
    }

    int numberOfCharacters = numberOfBits / 3;
    auto str = std::make_unique<unsigned char[]>(numberOfCharacters + 1);
    unsigned char *pStr = str.get();
    memset(str.get(), 0, numberOfCharacters + 1);

    unsigned char *pTmp = pB.get();
    const unsigned char *pBEnd = pB.get() + (numberOfBits + 7) / 8;  // Buffer end for bounds checking
    unsigned char bitmask = 0x80;
    int outbits = 0;
    unsigned char val = 0;

    while (numberOfBits--) {
        // Bounds check: ensure we don't read past buffer
        if (pTmp >= pBEnd) {
            Tracer::Error("Octal string: buffer overrun detected");
            return newErrorString("???");
        }
        unsigned char bitval = *pTmp & bitmask;
        val <<= 1;
        if (bitval)
            val |= 0x01;

        bitmask >>= 1;
        bitmask &= 0x7F;
        if (bitmask == 0) {
            bitmask = 0x80;
            pTmp++;
        }

        if (outbits++ == 2) {
            outbits = 0;
            *pStr = val + '0';
            val = 0;
            pStr++;
        }
    }

    return str.release();
}

char *DataItemBits::getASCII(unsigned char *pData, int bytes, int frombit, int tobit) {

    int numberOfBits = (tobit - frombit + 1);
    if (bytes < numberOfBits / 8 || !numberOfBits || numberOfBits % 8) {
        Tracer::Error("ASCII representation not valid");
        return newErrorStringChar("???");
    }

    std::unique_ptr<unsigned char[]> pTmp(getBits(pData, bytes, frombit, tobit));
    if (!pTmp) {
        Tracer::Error("DATAITEM_ENCODING_ASCII : Error.");
        return newErrorStringChar("???");
    }

    int numberOfBytes = numberOfBits/8;

    auto pStr = std::make_unique<char[]>(numberOfBytes + 1);
    char *ppStr = pStr.get();

    // replace non alphabetic ASCII characters with empty string
    for (int i = 0; i < numberOfBytes; i++) {
        if (*pData >= 32 && *pData <= 126)
            *ppStr++ = pTmp[i];
        else
            *ppStr++ = ' ';
    }

    *ppStr = 0;
    return pStr.release();
}


/**
 * @brief Get encoded string based on encoding type
 *
 * Unified helper function that returns the appropriate encoded string
 * for the given encoding type. Eliminates code duplication across
 * createWiresharkStringData(), insertStringToDict(), and similar functions.
 *
 * @param encoding The encoding type (SIX_BIT_CHAR, HEX_BIT_CHAR, OCTAL, ASCII)
 * @param pData Pointer to raw data buffer
 * @param nLength Length of data buffer in bytes
 * @return Allocated string (caller must delete[]), or nullptr for unsupported encoding
 */
char* DataItemBits::getEncodedString(_eEncoding encoding, unsigned char* pData, long nLength) {
    switch (encoding) {
        case DATAITEM_ENCODING_SIX_BIT_CHAR:
            return reinterpret_cast<char*>(getSixBitString(pData, nLength, m_nFrom, m_nTo));
        case DATAITEM_ENCODING_HEX_BIT_CHAR:
            return reinterpret_cast<char*>(getHexBitString(pData, nLength, m_nFrom, m_nTo));
        case DATAITEM_ENCODING_OCTAL:
            return reinterpret_cast<char*>(getOctal(pData, nLength, m_nFrom, m_nTo));
        case DATAITEM_ENCODING_ASCII:
            return getASCII(pData, nLength, m_nFrom, m_nTo);
        default:
            return nullptr;
    }
}


/**
 * @brief Formats ASTERIX data item bits into various output formats (text, JSON, XML)
 *
 * PERFORMANCE OPTIMIZATION:
 * Uses std::ostringstream instead of repeated string concatenation to avoid O(n²) behavior.
 *
 * WHY ostringstream is faster:
 * - String concatenation (operator+=) reallocates memory on each append (~70 times per call)
 * - ostringstream uses internal buffer that grows exponentially, minimizing reallocations
 * - Final ss.str() performs single append to strResult
 *
 * Impact: Reduces string reallocations from ~70 to ~4-5 for typical ASTERIX data items,
 * particularly beneficial for verbose output modes (JSON extensive, XML human-readable).
 *
 * @param strResult Output string to append formatted result
 * @param strHeader Header prefix for hierarchical field naming
 * @param formatType Output format (ETxt, EOut, EJSON, EJSONH, EJSONE, EXML, EXMLH)
 * @param pData Raw binary data buffer
 * @param nLength Data buffer length in bytes
 * @return true if formatted successfully, false if filtered out
 */
// Helper function to write opening tag based on format type
void DataItemBits::appendOpeningTag(std::ostringstream& ss, const unsigned int formatType) const {
    std::string indent("    ");

    switch (formatType) {
        case CAsterixFormat::EJSON:
            ss << format("\"%s\":", m_strShortName.c_str());
            break;
        case CAsterixFormat::EJSONH:
            ss << format("\n\t\t\"%s\":", m_strShortName.c_str());
            break;
        case CAsterixFormat::EJSONE:
            ss << format("\n\t\t\"%s\":{", m_strShortName.c_str());
            break;
        case CAsterixFormat::EXML:
            ss << format("<%s>", m_strShortName.c_str());
            break;
        case CAsterixFormat::EXMLH:
            ss << format("\n%s%s", indent.c_str(), indent.c_str());
            ss << format("<%s>", m_strShortName.c_str());
            break;
    }
}

// Helper function to write closing tag based on format type
void DataItemBits::appendClosingTag(std::ostringstream& ss, const unsigned int formatType) const {
    switch (formatType) {
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONH:
            ss << format(",");
            break;
        case CAsterixFormat::EJSONE:
            ss << format("},");
            break;
        case CAsterixFormat::EXML:
        case CAsterixFormat::EXMLH:
            ss << format("</%s>", m_strShortName.c_str());
            break;
    }
}

// Helper function to find value description
const char* DataItemBits::findValueDescription(unsigned long long value, bool& found) const {
    for (auto it = m_lValue.begin(); it != m_lValue.end(); ++it) {
        BitsValue* bv = *it;
        if (bv->m_nVal == static_cast<int>(value)) {
            found = true;
            return bv->m_strDescription.c_str();
        }
    }
    found = false;
    return "??????";
}

// Helper function to format unsigned value with metadata
void DataItemBits::formatUnsignedWithMeta(std::ostringstream& ss, unsigned long long value64,
                                          const unsigned int formatType, const std::string& strHeader,
                                          unsigned char* pData, long nLength) {
    bool descFound = false;
    const char* desc = nullptr;

    if (!m_lValue.empty()) {
        desc = findValueDescription(value64, descFound);
    }

    switch (formatType) {
        case CAsterixFormat::ETxt:
        case CAsterixFormat::EOut: {
            bool isOut = (formatType == CAsterixFormat::EOut);
            const char* prefix = isOut ? "\n" : "\n\t";

            if (isOut) {
                ss << format("%s%s.%s %llu", prefix, strHeader.c_str(), m_strShortName.c_str(), value64);
            } else {
                ss << format("%s%s: %llu", prefix, m_strName.c_str(), value64);
            }

            if (m_dScale != 0) {
                double scaled = value64 * m_dScale;
                ss << format(" (%.7lf %s)", scaled, m_strUnit.c_str());

                if (m_bMaxValueSet && scaled > m_dMaxValue) {
                    ss << format("%sWarning: Value larger than max (%.7lf)",
                                isOut ? " " : "\n\t", m_dMaxValue);
                }
                if (m_bMinValueSet && scaled < m_dMinValue) {
                    ss << format("%sWarning: Value smaller than min (%.7lf)",
                                isOut ? " " : "\n\t", m_dMinValue);
                }
            } else if (m_bIsConst && static_cast<int>(value64) != m_nConst) {
                ss << format("%sWarning: Value should be set to %d",
                            isOut ? " " : "\n\t", m_nConst);
            } else if (descFound) {
                ss << format(" (%s)", desc);
            } else if (!m_lValue.empty()) {
                ss << format(" ( ?????? )");
            }
            break;
        }
        case CAsterixFormat::EJSONE: {
            if (m_dScale != 0) {
                ss << format("\"val\"=%.7lf", value64 * m_dScale);
            } else {
                ss << format("\"val\"=%llu", value64);
            }

            unsigned char* hexstr = getHexBitStringFullByte(pData, nLength, m_nFrom, m_nTo);
            ss << format(", \"hex\"=\"%s\"", hexstr);
            delete[] hexstr;

            if ((m_nTo - m_nFrom + 1) % 8) {
                unsigned char* maskstr = getHexBitStringMask(nLength, m_nFrom, m_nTo);
                ss << format(", \"mask\"=\"%s\"", maskstr);
                delete[] maskstr;
            }

            ss << format(", \"name\"=\"%s\"", m_strName.c_str());

            if (descFound) {
                ss << format(", \"meaning\"=\"%s\"", desc);
            } else if (!m_lValue.empty()) {
                ss << format(" ( ?????? )");
            }
            break;
        }
        default: {
            if (m_dScale != 0) {
                ss << format("%.7lf", value64 * m_dScale);
            } else {
                ss << format("%llu", value64);
            }
            break;
        }
    }
}

// Helper function to format signed value with metadata
void DataItemBits::formatSignedWithMeta(std::ostringstream& ss, signed long value,
                                        const unsigned int formatType, const std::string& strHeader,
                                        unsigned char* pData, long nLength) {
    bool descFound = false;
    const char* desc = nullptr;

    if (!m_lValue.empty()) {
        desc = findValueDescription(static_cast<unsigned long long>(value), descFound);
    }

    switch (formatType) {
        case CAsterixFormat::ETxt:
            ss << format("\n\t%s: %ld", m_strName.c_str(), value);
            break;
        case CAsterixFormat::EOut:
            ss << format("\n%s.%s %ld", strHeader.c_str(), m_strShortName.c_str(), value);
            break;
        case CAsterixFormat::EJSONE:
            if (m_dScale != 0) {
                ss << format("\"val\"=%.7lf", value * m_dScale);
            } else {
                ss << format("\"val\"=%ld", value);
            }
            break;
        default:
            if (m_dScale != 0) {
                ss << format("%.7lf", value * m_dScale);
            } else {
                ss << format("%ld", value);
            }
            return;
    }

    // Add scaling/warnings for detailed formats
    if (formatType == CAsterixFormat::ETxt || formatType == CAsterixFormat::EOut) {
        if (m_dScale != 0) {
            double scaled = value * m_dScale;
            ss << format(" (%.7lf %s)", scaled, m_strUnit.c_str());

            bool isOut = (formatType == CAsterixFormat::EOut);
            if (m_bMaxValueSet && scaled > m_dMaxValue) {
                ss << format("%sWarning: Value larger than max (%.7lf)",
                            isOut ? " " : "\n\t", m_dMaxValue);
            }
            if (m_bMinValueSet && scaled < m_dMinValue) {
                ss << format("%sWarning: Value smaller than min (%.7lf)",
                            isOut ? " " : "\n\t", m_dMinValue);
            }
        }
    } else if (formatType == CAsterixFormat::EJSONE) {
        unsigned char* hexstr = getHexBitStringFullByte(pData, nLength, m_nFrom, m_nTo);
        ss << format(", \"hex\"=\"%s\"", hexstr);
        delete[] hexstr;

        if ((m_nTo - m_nFrom + 1) % 8) {
            unsigned char* maskstr = getHexBitStringMask(nLength, m_nFrom, m_nTo);
            ss << format(", \"mask\"=\"%s\"", maskstr);
            delete[] maskstr;
        }

        ss << format(", \"name\"=\"%s\"", m_strName.c_str());

        if (descFound) {
            ss << format(", \"meaning\"=\"%s\"", desc);
        } else if (!m_lValue.empty()) {
            ss << format(" ( ?????? )");
        }
    }
}

// Helper function to format string encodings
void DataItemBits::formatStringEncoding(std::ostringstream& ss, const unsigned char* str,
                                        unsigned char* pData, long nLength,
                                        const unsigned int formatType, const std::string& strHeader) {
    switch (formatType) {
        case CAsterixFormat::ETxt:
            ss << format("\n\t%s: %s", m_strName.c_str(), str);
            break;
        case CAsterixFormat::EOut:
            ss << format("\n%s.%s %s", strHeader.c_str(), m_strShortName.c_str(), str);
            break;
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONH:
            ss << format("\"%s\"", str);
            break;
        case CAsterixFormat::EJSONE: {
            ss << format("\"val\"=\"%s\"", str);

            unsigned char* hexstr = getHexBitStringFullByte(pData, nLength, m_nFrom, m_nTo);
            ss << format(", \"hex\"=\"%s\"", hexstr);
            delete[] hexstr;

            if ((m_nTo - m_nFrom + 1) % 8) {
                unsigned char* maskstr = getHexBitStringMask(nLength, m_nFrom, m_nTo);
                ss << format(", \"mask\"=\"%s\"", maskstr);
                delete[] maskstr;
            }

            ss << format(", \"name\"=\"%s\"", m_strName.c_str());
            break;
        }
        default:
            ss << format("%s", str);
            break;
    }
}

bool DataItemBits::getText(std::string &strResult, std::string &strHeader, const unsigned int formatType,
                           unsigned char *pData, long nLength) {
    // Early returns for filtering and validation
    if (gFiltering && !m_bFiltered) {
        return false;
    }

    // Ensure bit range is properly ordered
    if (m_nFrom > m_nTo) {
        std::swap(m_nFrom, m_nTo);
    }

    // Validate bit range
    if (m_nFrom < 1 || m_nTo > nLength * 8) {
        Tracer::Error("Wrong bit format!");
        return true;
    }

    // Ensure names are populated
    if (m_strName.empty()) {
        m_strName = m_strShortName;
    } else if (m_strShortName.empty()) {
        m_strShortName = m_strName;
    }

    std::ostringstream ss;

    appendOpeningTag(ss, formatType);

    // Process encoding types
    switch (m_eEncoding) {
        case DATAITEM_ENCODING_UNSIGNED: {
            int numberOfBits = (m_nTo - m_nFrom + 1);
            unsigned long long value64 = (numberOfBits > 32)
                ? getUnsigned64(pData, nLength, m_nFrom, m_nTo)
                : getUnsigned(pData, nLength, m_nFrom, m_nTo);
            formatUnsignedWithMeta(ss, value64, formatType, strHeader, pData, nLength);
            break;
        }
        case DATAITEM_ENCODING_SIGNED: {
            signed long value = getSigned(pData, nLength, m_nFrom, m_nTo);
            formatSignedWithMeta(ss, value, formatType, strHeader, pData, nLength);
            break;
        }
        case DATAITEM_ENCODING_SIX_BIT_CHAR: {
            unsigned char* str = getSixBitString(pData, nLength, m_nFrom, m_nTo);
            formatStringEncoding(ss, str, pData, nLength, formatType, strHeader);
            delete[] str;
            break;
        }
        case DATAITEM_ENCODING_HEX_BIT_CHAR: {
            unsigned char* str = getHexBitString(pData, nLength, m_nFrom, m_nTo);
            formatStringEncoding(ss, str, pData, nLength, formatType, strHeader);
            delete[] str;
            break;
        }
        case DATAITEM_ENCODING_OCTAL: {
            unsigned char* str = getOctal(pData, nLength, m_nFrom, m_nTo);
            formatStringEncoding(ss, str, pData, nLength, formatType, strHeader);
            delete[] str;
            break;
        }
        case DATAITEM_ENCODING_ASCII: {
            char* pStr = getASCII(pData, nLength, m_nFrom, m_nTo);
            if (formatType != CAsterixFormat::EJSONE && formatType != CAsterixFormat::ETxt &&
                formatType != CAsterixFormat::EOut && formatType != CAsterixFormat::EJSON &&
                formatType != CAsterixFormat::EJSONH) {
                // Default case - do nothing
            } else {
                formatStringEncoding(ss, (const unsigned char*)pStr, pData, nLength, formatType, strHeader);
            }
            delete[] pStr;
            break;
        }
        default:
            Tracer::Error("Unknown encoding");
            break;
    }

    appendClosingTag(ss, formatType);

    strResult += ss.str();
    return true;
}

std::string DataItemBits::printDescriptors(std::string header) {
    std::string strDes;

    if (gFiltering && !m_bFiltered) {
        strDes = "#";
    }

    strDes += header + m_strShortName;

    int fill = 60 - strDes.length();
    if (fill > 0) {
        std::string strFill(fill, ' ');
        strDes += strFill;
    }

    strDes += " " + m_strName + "\n";

    return strDes;
}

bool DataItemBits::filterOutItem(const char *name) {
    if (strncmp(name, m_strShortName.c_str(), m_strShortName.length()) == 0) {
        m_bFiltered = true;
        return true;
    }
    return false;
}

const char *DataItemBits::getDescription(const char *field, const char *value = nullptr) {
    if (m_strName.empty() && !m_strShortName.empty())
        m_strName = m_strShortName;
    else if (!m_strName.empty() && m_strShortName.empty())
        m_strShortName = m_strName;


    if (m_strShortName.compare(field) == 0) {
        if (value == nullptr) {
            return m_strName.c_str();
        } else {
            int val = atoi(value);
            if (!m_lValue.empty()) {
                for (const auto* bv : m_lValue) {
                    if (bv->m_nVal == val)
                        return bv->m_strDescription.c_str();
                }
            }
        }
    }
    return nullptr;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemBits::getWiresharkDefinitions()
{
if (m_strName.empty() && !m_strShortName.empty())
m_strName = m_strShortName;
else if (!m_strName.empty() && m_strShortName.empty())
m_strShortName = m_strName;

// Use calloc instead of malloc+memset for cleaner initialization (SonarCloud)
// Note: calloc is appropriate here for C interop with Wireshark plugin
fulliautomatix_definitions* def = static_cast<fulliautomatix_definitions*>(
    calloc(1, sizeof(fulliautomatix_definitions)));
if (def == nullptr) {
    return nullptr;
}
def->pid = getPID();
def->name = strdup(m_strName.c_str());

// abbrev can only contain -._ and chars
std::string strAbbrev = m_strShortName;
std::size_t index;
while((index=strAbbrev.find_first_not_of ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVXYZ_.-0123456789")) != std::string::npos)
strAbbrev.replace(index, 1, "_", 1);
strAbbrev = "asterix." + strAbbrev;

def->abbrev = strdup(strAbbrev.c_str());

def->strings = nullptr;

if (!m_lValue.empty())
{
def->strings = (fulliautomatix_value_string*)malloc((1+m_lValue.size()) * sizeof(fulliautomatix_value_string));
// Security fix: Check malloc return value to prevent null pointer dereference
if (def->strings == nullptr) {
    return def;
}

int i = 0;
for (const auto* bv : m_lValue) {
    def->strings[i].value = bv->m_nVal;
    def->strings[i].strptr = strdup(bv->m_strDescription.c_str());
    i++;
}
def->strings[i].value = 0;
def->strings[i].strptr = nullptr;
}

if (m_nFrom > m_nTo)
{ // just in case
int tmp = m_nFrom;
m_nFrom = m_nTo;
m_nTo = tmp;
}

switch(m_eEncoding)
{
case DATAITEM_ENCODING_UNSIGNED:
{
int numberOfBits = (m_nTo-m_nFrom+1);

if (numberOfBits < 8)
{
    def->type = FA_FT_UINT8;
    def->display = FA_BASE_DEC;

    def->bitmask = 0x01;
    def->bitmask <<= numberOfBits;
    def->bitmask--;
    def->bitmask <<= (m_nFrom-1)%8;

}
else if (numberOfBits == 8)
{
    def->type = FA_FT_UINT8;
    def->display = FA_BASE_DEC;
}
else if (numberOfBits < 16)
{
    def->type = FA_FT_UINT16;
    def->display = FA_BASE_DEC;
    def->bitmask = 0x01;
    def->bitmask <<= numberOfBits;
    def->bitmask--;
    def->bitmask <<= (m_nFrom-1)%16;
}
else if (numberOfBits == 16)
{
    def->type = FA_FT_UINT16;
    def->display = FA_BASE_DEC;
}
else if (numberOfBits < 24)
{
    def->type = FA_FT_UINT24;
    def->display = FA_BASE_DEC;
    def->bitmask = 0x01;
    def->bitmask <<= numberOfBits;
    def->bitmask--;
    def->bitmask <<= (m_nFrom-1);
}
else if (numberOfBits == 24)
{
    def->type = FA_FT_UINT24;
    def->display = FA_BASE_DEC;
}
else if (numberOfBits == 32)
{
    def->type = FA_FT_UINT32;
    def->display = FA_BASE_DEC;
}
else
{
    def->type = FA_FT_UINT32;
    def->display = FA_BASE_DEC;
    def->bitmask = 0x01;
    def->bitmask <<= numberOfBits;
    def->bitmask--;
    def->bitmask <<= (m_nFrom-1);
}
}
break;

case DATAITEM_ENCODING_SIGNED:
{
int numberOfBits = (m_nTo-m_nFrom+1);

if (numberOfBits <= 8)
{
    def->type = FA_FT_INT8;
    def->display = FA_BASE_DEC;
}
else if (numberOfBits <= 16)
{
    def->type = FA_FT_INT16;
    def->display = FA_BASE_DEC;
}
else if (numberOfBits <= 24)
{
    def->type = FA_FT_INT24;
    def->display = FA_BASE_DEC;
}
else
{
    def->type = FA_FT_INT32;
    def->display = FA_BASE_DEC;
}
}
break;

case DATAITEM_ENCODING_SIX_BIT_CHAR:
{
def->type = FA_FT_STRING;
def->display = FA_BASE_NONE;
}
break;
case DATAITEM_ENCODING_HEX_BIT_CHAR:
{
def->type = FA_FT_STRING;
def->display = FA_BASE_NONE;
}
break;
case DATAITEM_ENCODING_OCTAL:
{
def->type = FA_FT_STRING;
def->display = FA_BASE_NONE;
}
break;
case DATAITEM_ENCODING_ASCII:
{
def->type = FA_FT_STRING;
def->display = FA_BASE_NONE;
}
break;
default:
Tracer::Error("Unknown encoding");
break;
}

return def;
}

// Helper function to create value description for Wireshark
char* DataItemBits::createWiresharkValueDescription(double scaled, unsigned long long value64) {
    char tmp[128];
    bool isOutOfRange = (m_bMaxValueSet && scaled > m_dMaxValue) || (m_bMinValueSet && scaled < m_dMinValue);
    bool isWrongConst = m_bIsConst && static_cast<int>(value64) != m_nConst;

    if (isOutOfRange) {
        snprintf(tmp, sizeof(tmp), " (%.7lf %s) Warning! Value out of range (%.7lf to %.7lf)",
                 scaled, m_strUnit.c_str(), m_dMinValue, m_dMaxValue);
    } else if (isWrongConst) {
        snprintf(tmp, sizeof(tmp), " (%.7lf %s) Warning! Value should be %d",
                 scaled, m_strUnit.c_str(), m_nConst);
    } else {
        snprintf(tmp, sizeof(tmp), " (%.7lf %s)", scaled, m_strUnit.c_str());
    }

    return strdup(tmp);
}

// Helper function to process unsigned encoding for Wireshark
fulliautomatix_data* DataItemBits::createWiresharkUnsignedData(unsigned char* pData, long nLength,
                                                               int byteoffset, int firstByte,
                                                               int numberOfBits, int numberOfBytes) {
    unsigned long long value64 = (numberOfBits > 32)
        ? getUnsigned64(pData, nLength, m_nFrom, m_nTo)
        : getUnsigned(pData, nLength, m_nFrom, m_nTo);

    // Use auto to avoid redundant type specification (SonarCloud S5827)
    auto value = static_cast<unsigned long>(value64);

    // Adjust for bitmask presentation in Wireshark
    if (value && m_nFrom > 1 && numberOfBits % 8) {
        value <<= ((m_nFrom - 1) % (numberOfBytes * 8));
        value64 <<= ((m_nFrom - 1) % (numberOfBytes * 8));
    }

    fulliautomatix_data* pOutData = newDataUL(nullptr, getPID(), byteoffset + firstByte, numberOfBytes, value);

    if (m_dScale != 0 || m_bIsConst) {
        double scaled = value64 * m_dScale;
        pOutData->value_description = createWiresharkValueDescription(scaled, value64);

        bool hasError = (m_bMaxValueSet && scaled > m_dMaxValue) ||
                       (m_bMinValueSet && scaled < m_dMinValue) ||
                       (m_bIsConst && static_cast<int>(value64) != m_nConst);
        if (hasError) {
            pOutData->err = 1;
        }
    }

    return pOutData;
}

// Helper function to process signed encoding for Wireshark
fulliautomatix_data* DataItemBits::createWiresharkSignedData(unsigned char* pData, long nLength,
                                                             int byteoffset, int firstByte,
                                                             int numberOfBytes) {
    signed long value = getSigned(pData, nLength, m_nFrom, m_nTo);
    fulliautomatix_data* pOutData = newDataSL(nullptr, getPID(), byteoffset + firstByte, numberOfBytes, value);

    if (m_dScale != 0) {
        double scaled = value * m_dScale;
        pOutData->value_description = createWiresharkValueDescription(scaled, static_cast<unsigned long long>(value));

        bool hasError = (m_bMaxValueSet && scaled > m_dMaxValue) ||
                       (m_bMinValueSet && scaled < m_dMinValue) ||
                       (m_bIsConst && static_cast<int>(value) != m_nConst);
        if (hasError) {
            pOutData->err = 1;
        }
    }

    return pOutData;
}

// Helper function to process string encoding for Wireshark
fulliautomatix_data* DataItemBits::createWiresharkStringData(_eEncoding encoding, unsigned char* pData,
                                                             long nLength, int byteoffset,
                                                             int firstByte, int numberOfBytes) {
    char* str = getEncodedString(encoding, pData, nLength);
    if (!str) {
        return nullptr;
    }

    fulliautomatix_data* data = newDataString(nullptr, getPID(), byteoffset + firstByte, numberOfBytes, str);
    delete[] str;
    return data;
}

fulliautomatix_data* DataItemBits::getData(unsigned char* pData, long nLength, int byteoffset)
{
    // Ensure bit range is properly ordered
    if (m_nFrom > m_nTo) {
        std::swap(m_nFrom, m_nTo);
    }

    int firstByte = nLength - (m_nTo - 1) / 8 - 1;
    int numberOfBits = (m_nTo - m_nFrom + 1);
    int numberOfBytes = (numberOfBits + 7) / 8;

    // Ensure name is populated
    if (m_strName.empty()) {
        m_strName = m_strShortName;
    }

    // Process based on encoding type
    switch (m_eEncoding) {
        case DATAITEM_ENCODING_UNSIGNED:
            return createWiresharkUnsignedData(pData, nLength, byteoffset, firstByte, numberOfBits, numberOfBytes);

        case DATAITEM_ENCODING_SIGNED:
            return createWiresharkSignedData(pData, nLength, byteoffset, firstByte, numberOfBytes);

        case DATAITEM_ENCODING_SIX_BIT_CHAR:
        case DATAITEM_ENCODING_HEX_BIT_CHAR:
        case DATAITEM_ENCODING_OCTAL:
        case DATAITEM_ENCODING_ASCII: {
            fulliautomatix_data* result = createWiresharkStringData(m_eEncoding, pData, nLength,
                                                                     byteoffset, firstByte, numberOfBytes);
            if (result) {
                return result;
            }
            break;
        }

        default:
            Tracer::Error("Unknown encoding");
            break;
    }

    // Error case - unknown encoding
    return newDataMessage(nullptr, byteoffset + firstByte, numberOfBytes, 2,
                          "Error: Unknown encoding.");
}
#endif

#if defined(PYTHON_WRAPPER)

PyObject* DataItemBits::getObject(unsigned char* pData, long nLength, int verbose)
{
    PyObject* p = PyDict_New();
    insertToDict(p, pData, nLength, verbose);
    return p;
}

// Helper function to add a key-value pair to Python dictionary and handle reference counting
void DataItemBits::addPyDictItem(PyObject* dict, const char* key, PyObject* value) {
    PyObject* k = Py_BuildValue("s", key);
    PyDict_SetItem(dict, k, value);
    Py_DECREF(k);
    Py_DECREF(value);
}

// Helper function to process unsigned value for Python dict
void DataItemBits::insertUnsignedToDict(PyObject* pValue, unsigned long long value64, int verbose) {
    if (m_dScale != 0) {
        double scaled = value64 * m_dScale;
        addPyDictItem(pValue, "val", Py_BuildValue("d", scaled));

        if (verbose) {
            if (m_bMaxValueSet) {
                addPyDictItem(pValue, "max", Py_BuildValue("d", m_dMaxValue));
            }
            if (m_bMinValueSet) {
                addPyDictItem(pValue, "min", Py_BuildValue("d", m_dMinValue));
            }
        }
    } else if (m_bIsConst) {
        addPyDictItem(pValue, "val", Py_BuildValue("K", value64));
        if (verbose) {
            addPyDictItem(pValue, "const", Py_BuildValue("k", m_nConst));
        }
    } else if (!m_lValue.empty()) {
        bool descFound = false;
        const char* desc = findValueDescription(value64, descFound);

        addPyDictItem(pValue, "val", Py_BuildValue("K", value64));

        if (verbose) {
            addPyDictItem(pValue, "meaning", Py_BuildValue("s", descFound ? desc : "???"));
        }
    } else {
        addPyDictItem(pValue, "val", Py_BuildValue("K", value64));
    }
}

// Helper function to process signed value for Python dict
void DataItemBits::insertSignedToDict(PyObject* pValue, signed long value, int verbose) {
    if (m_dScale != 0) {
        double scaled = value * m_dScale;
        addPyDictItem(pValue, "val", Py_BuildValue("d", scaled));

        if (verbose) {
            if (m_bMaxValueSet) {
                addPyDictItem(pValue, "max", Py_BuildValue("d", m_dMaxValue));
            }
            if (m_bMinValueSet) {
                addPyDictItem(pValue, "min", Py_BuildValue("d", m_dMinValue));
            }
        }
    } else if (m_bIsConst) {
        addPyDictItem(pValue, "val", Py_BuildValue("d", value));
        if (verbose) {
            addPyDictItem(pValue, "const", Py_BuildValue("k", m_nConst));
        }
    } else if (!m_lValue.empty()) {
        bool descFound = false;
        const char* desc = findValueDescription(static_cast<unsigned long long>(value), descFound);

        addPyDictItem(pValue, "val", Py_BuildValue("d", value));

        if (verbose) {
            addPyDictItem(pValue, "meaning", Py_BuildValue("s", descFound ? desc : "???"));
        }
    } else {
        addPyDictItem(pValue, "val", Py_BuildValue("l", value));
    }
}

// Helper function to process string encodings for Python dict
void DataItemBits::insertStringToDict(PyObject* pValue, _eEncoding encoding,
                                      unsigned char* pData, long nLength) {
    char* str = getEncodedString(encoding, pData, nLength);
    if (!str) {
        addPyDictItem(pValue, "val", Py_BuildValue("s", "???"));
        return;
    }

    addPyDictItem(pValue, "val", Py_BuildValue("s", str));
    delete[] str;
}

void DataItemBits::insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose)
{
    // Create new dictionary for this item
    PyObject* pValue = PyDict_New();
    addPyDictItem(p, m_strShortName.c_str(), pValue);
    // Note: addPyDictItem already DECREF'd pValue, but PyDict_SetItem incremented it, so we still have a reference

    // Add description if verbose
    if (verbose) {
        const char* desc = m_strName.empty() ? m_strShortName.c_str() : m_strName.c_str();
        addPyDictItem(pValue, "desc", Py_BuildValue("s", desc));
    }

    // Ensure bit range is properly ordered
    if (m_nFrom > m_nTo) {
        std::swap(m_nFrom, m_nTo);
    }

    // Process based on encoding type
    switch (m_eEncoding) {
        case DATAITEM_ENCODING_UNSIGNED: {
            int numberOfBits = (m_nTo - m_nFrom + 1);
            unsigned long long value64 = (numberOfBits > 32)
                ? getUnsigned64(pData, nLength, m_nFrom, m_nTo)
                : getUnsigned(pData, nLength, m_nFrom, m_nTo);
            insertUnsignedToDict(pValue, value64, verbose);
            break;
        }
        case DATAITEM_ENCODING_SIGNED: {
            signed long value = getSigned(pData, nLength, m_nFrom, m_nTo);
            insertSignedToDict(pValue, value, verbose);
            break;
        }
        case DATAITEM_ENCODING_SIX_BIT_CHAR:
        case DATAITEM_ENCODING_HEX_BIT_CHAR:
        case DATAITEM_ENCODING_OCTAL:
        case DATAITEM_ENCODING_ASCII:
            insertStringToDict(pValue, m_eEncoding, pData, nLength);
            break;
        default:
            addPyDictItem(pValue, "val", Py_BuildValue("s", "???"));
            break;
    }
}
#endif
