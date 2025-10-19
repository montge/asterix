/**
 * Unit tests for DataItemBits class - PUBLIC methods only
 *
 * These tests target the 4 UNCOVERED PUBLIC functions in DataItemBits.cpp
 * to improve coverage from 43.4% toward 60% milestone.
 *
 * Uncovered PUBLIC functions tested:
 * 1. getLength() - line 89
 * 2. printDescriptors() - line 831
 * 3. filterOutItem() - line 851
 * 4. getDescription() - line 859
 *
 * Note: getHexBitStringFullByte() and getHexBitStringMask() are PRIVATE
 * and cannot be tested directly (they must be tested through public API).
 *
 * Requirements Coverage:
 * - REQ-LLR-BITS-001: Bit extraction and manipulation
 * - REQ-LLR-BITS-003: Descriptor management
 *
 * Test Cases:
 * - TC-CPP-BITS-001: Test getLength() (error path)
 * - TC-CPP-BITS-002: Test printDescriptors() (descriptor formatting)
 * - TC-CPP-BITS-003: Test printDescriptors() with filtering
 * - TC-CPP-BITS-004: Test filterOutItem() (name matching)
 * - TC-CPP-BITS-005: Test filterOutItem() (prefix matching)
 * - TC-CPP-BITS-006: Test getDescription() (field lookup)
 * - TC-CPP-BITS-007: Test getDescription() with value lookup
 */

#include <gtest/gtest.h>
#include "../../src/asterix/DataItemFormat.h"
#include "../../src/asterix/DataItemBits.h"
#include "../../src/asterix/Tracer.h"
#include "../../src/asterix/asterixformat.hxx"
#include <cstring>

// Global variables required by ASTERIX library
bool gVerbose = false;
bool gFiltering = false;

class DataItemBitsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure clean state
        gVerbose = false;
        gFiltering = false;
    }

    void TearDown() override {
        gFiltering = false;
    }
};

/**
 * Test Case: TC-CPP-BITS-001
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getLength() - This function should not be called directly
 * (it logs an error and returns 0), but we test it for coverage.
 */
TEST_F(DataItemBitsTest, GetLengthReturnsZero) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_strName = "System Area Code";

    unsigned char data[10] = {0x12, 0x34, 0x56, 0x78, 0x9A};

    // getLength() should return 0 and log an error
    // (This is an error path - the function shouldn't be called)
    long length = bits.getLength(data);

    EXPECT_EQ(length, 0) << "getLength() should return 0 (error path)";
}

/**
 * Test Case: TC-CPP-BITS-002
 * Requirement: REQ-LLR-BITS-003
 *
 * Test printDescriptors() - Formats descriptor string with name
 */
TEST_F(DataItemBitsTest, PrintDescriptors) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_strName = "System Area Code";
    bits.m_bFiltered = false;

    // Print descriptors with header
    std::string result = bits.printDescriptors("  ");

    EXPECT_GT(result.length(), 0) << "Descriptor should not be empty";
    EXPECT_NE(result.find("SAC"), std::string::npos) << "Should contain short name 'SAC'";
    EXPECT_NE(result.find("System Area Code"), std::string::npos) << "Should contain full name";
    EXPECT_NE(result.find("\n"), std::string::npos) << "Should end with newline";
}

/**
 * Test Case: TC-CPP-BITS-003
 * Requirement: REQ-LLR-BITS-003
 *
 * Test printDescriptors() with filtering enabled
 */
TEST_F(DataItemBitsTest, PrintDescriptorsWithFiltering) {
    // Test with filtering enabled but item not filtered (should prefix with '#')
    {
        DataItemBits bits(8);
        bits.m_strShortName = "SIC";
        bits.m_strName = "System Identification Code";
        bits.m_bFiltered = false;

        gFiltering = true;

        std::string result = bits.printDescriptors("  ");
        EXPECT_GT(result.length(), 0) << "Result should not be empty";
        EXPECT_NE(result.find("#"), std::string::npos)
            << "Should contain '#' when filtering enabled but item not filtered";

        gFiltering = false;
    }

    // Test with filtering enabled and item filtered (should NOT prefix with '#')
    {
        DataItemBits bits2(8);
        bits2.m_strShortName = "SIC";
        bits2.m_strName = "System Identification Code";
        bits2.m_bFiltered = true;

        gFiltering = true;

        std::string result2 = bits2.printDescriptors("  ");
        EXPECT_GT(result2.length(), 0) << "Result should not be empty";
        // When m_bFiltered is true, no '#' prefix
        size_t hashPos = result2.find("#");
        bool hasHashAtStart = (hashPos == 0 || (hashPos == 2)); // After "  " header
        EXPECT_FALSE(hasHashAtStart || result2.find("#  ") != std::string::npos)
            << "Should NOT contain '#' prefix when item is filtered";

        gFiltering = false;
    }
}

/**
 * Test Case: TC-CPP-BITS-004
 * Requirement: REQ-LLR-BITS-003
 *
 * Test filterOutItem() - Filters items by exact name match
 */
TEST_F(DataItemBitsTest, FilterOutItemExactMatch) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_bFiltered = false;

    // Should match when names match exactly
    bool filtered = bits.filterOutItem("SAC");

    EXPECT_TRUE(filtered) << "Should return true for matching name";
    EXPECT_TRUE(bits.m_bFiltered) << "Filtered flag should be set to true";
}

/**
 * Test Case: TC-CPP-BITS-005
 * Requirement: REQ-LLR-BITS-003
 *
 * Test filterOutItem() - No match for different name
 */
TEST_F(DataItemBitsTest, FilterOutItemNoMatch) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_bFiltered = false;

    // Should not match different name
    bool filtered = bits.filterOutItem("SIC");

    EXPECT_FALSE(filtered) << "Should return false for non-matching name";
    EXPECT_FALSE(bits.m_bFiltered) << "Filtered flag should remain false";
}

/**
 * Test Case: TC-CPP-BITS-006
 * Requirement: REQ-LLR-BITS-003
 *
 * Test filterOutItem() - Prefix match (uses strncmp)
 */
TEST_F(DataItemBitsTest, FilterOutItemPrefixMatch) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_bFiltered = false;

    // Test prefix match (strncmp compares up to length of m_strShortName)
    // "SAC/SIC" starts with "SAC" so should match
    bool filtered = bits.filterOutItem("SAC/SIC");

    EXPECT_TRUE(filtered) << "Should match prefix 'SAC' in 'SAC/SIC'";
    EXPECT_TRUE(bits.m_bFiltered) << "Filtered flag should be set";
}

/**
 * Test Case: TC-CPP-BITS-007
 * Requirement: REQ-LLR-BITS-003
 *
 * Test getDescription() - Returns description for matching field (no value)
 */
TEST_F(DataItemBitsTest, GetDescriptionFieldOnly) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_strName = "System Area Code";

    // Get description for matching field (NULL value means return field description)
    const char* desc = bits.getDescription("SAC", NULL);

    ASSERT_NE(desc, nullptr) << "Description should not be NULL for matching field";
    EXPECT_STREQ(desc, "System Area Code") << "Should return full name as description";
}

/**
 * Test Case: TC-CPP-BITS-008
 * Requirement: REQ-LLR-BITS-003
 *
 * Test getDescription() - Returns NULL for non-matching field
 */
TEST_F(DataItemBitsTest, GetDescriptionNoMatch) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_strName = "System Area Code";

    // Get description for non-matching field
    const char* desc = bits.getDescription("SIC", NULL);

    EXPECT_EQ(desc, nullptr) << "Should return NULL for non-matching field";
}

/**
 * Test Case: TC-CPP-BITS-009
 * Requirement: REQ-LLR-BITS-003
 *
 * Test getDescription() - With empty names (copies short to full or vice versa)
 */
TEST_F(DataItemBitsTest, GetDescriptionEmptyNames) {
    // Test with short name only (full name empty)
    {
        DataItemBits bits(8);
        bits.m_strShortName = "SAC";
        bits.m_strName = "";

        const char* desc = bits.getDescription("SAC", NULL);
        ASSERT_NE(desc, nullptr);
        // After getDescription is called, m_strName should be copied from m_strShortName
        EXPECT_STREQ(desc, "SAC") << "Should use short name when full name empty";
    }

    // Test with full name only (short name empty)
    {
        DataItemBits bits2(8);
        bits2.m_strShortName = "";
        bits2.m_strName = "System Area Code";

        const char* desc2 = bits2.getDescription("System Area Code", NULL);
        ASSERT_NE(desc2, nullptr);
        EXPECT_STREQ(desc2, "System Area Code");
    }
}

/**
 * Test Case: TC-CPP-BITS-010
 * Requirement: REQ-LLR-BITS-003
 *
 * Test getDescription() - With value parameter (looks up BitsValue descriptions)
 */
TEST_F(DataItemBitsTest, GetDescriptionWithValue) {
    DataItemBits bits(8);
    bits.m_strShortName = "TYP";
    bits.m_strName = "Target Type";

    // Add a BitsValue for testing (value=1, description="Primary Target")
    BitsValue *bv = new BitsValue(1, "Primary Target");
    bits.m_lValue.push_back(bv);

    // Get description for field with specific value
    // Note: getDescription compares string value "1" against m_nVal converted to string
    const char* desc = bits.getDescription("TYP", "1");

    ASSERT_NE(desc, nullptr) << "Should find description for value '1'";
    EXPECT_STREQ(desc, "Primary Target") << "Should return value description";
}

/**
 * Test Case: TC-CPP-BITS-011
 * Requirement: REQ-LLR-BITS-003
 *
 * Test getDescription() - Value not found returns NULL
 */
TEST_F(DataItemBitsTest, GetDescriptionValueNotFound) {
    DataItemBits bits(8);
    bits.m_strShortName = "TYP";
    bits.m_strName = "Target Type";

    // Add a BitsValue for testing (value=1)
    BitsValue *bv = new BitsValue(1, "Primary Target");
    bits.m_lValue.push_back(bv);

    // Get description for non-existent value (999)
    const char* desc = bits.getDescription("TYP", "999");

    EXPECT_EQ(desc, nullptr) << "Should return NULL for non-existent value";
}

/**
 * Test Case: TC-CPP-BITS-012
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with SIGNED encoding - negative values
 */
TEST_F(DataItemBitsTest, GetTextSignedNegative) {
    DataItemBits bits(8);
    bits.m_strShortName = "DELTA";
    bits.m_strName = "Delta Value";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_SIGNED;

    // 0xFF = -1 in signed 8-bit
    unsigned char data[] = {0xFF};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("Delta Value"), std::string::npos);
    EXPECT_NE(result.find("-1"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BITS-013
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with SIGNED encoding and scaling
 */
TEST_F(DataItemBitsTest, GetTextSignedWithScaling) {
    DataItemBits bits(16);
    bits.m_strShortName = "LAT";
    bits.m_strName = "Latitude";
    bits.m_nFrom = 1;
    bits.m_nTo = 16;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_SIGNED;
    bits.m_dScale = 0.0000215; // degrees
    bits.m_strUnit = "deg";

    // Positive value: 10000
    unsigned char data[] = {0x27, 0x10}; // 10000 in big-endian
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 2);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("Latitude"), std::string::npos);
    EXPECT_NE(result.find("deg"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BITS-014
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with HEX_BIT_CHAR encoding
 */
TEST_F(DataItemBitsTest, GetTextHexBitChar) {
    DataItemBits bits(16);
    bits.m_strShortName = "MODE3A";
    bits.m_strName = "Mode-3/A Code";
    bits.m_nFrom = 1;
    bits.m_nTo = 16;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_HEX_BIT_CHAR;

    unsigned char data[] = {0x12, 0x34};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 2);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("Mode-3/A Code"), std::string::npos);
    EXPECT_NE(result.find("1234"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BITS-015
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with OCTAL encoding
 */
TEST_F(DataItemBitsTest, GetTextOctal) {
    DataItemBits bits(12);
    bits.m_strShortName = "CODE";
    bits.m_strName = "Octal Code";
    bits.m_nFrom = 1;
    bits.m_nTo = 12;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_OCTAL;

    // 12 bits = 4 octal digits (0777 = 0x1FF)
    unsigned char data[] = {0x01, 0xFF};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 2);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("Octal Code"), std::string::npos);
    // Should contain octal representation
    EXPECT_GT(result.length(), 0);
}

/**
 * Test Case: TC-CPP-BITS-016
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with SIX_BIT_CHAR encoding
 */
TEST_F(DataItemBitsTest, GetTextSixBitChar) {
    DataItemBits bits(48);
    bits.m_strShortName = "CALLSIGN";
    bits.m_strName = "Aircraft Callsign";
    bits.m_nFrom = 1;
    bits.m_nTo = 48;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_SIX_BIT_CHAR;

    // 48 bits = 8 six-bit characters
    // Encoding "TEST" in 6-bit: T=20, E=5, S=19, T=20
    unsigned char data[] = {0x51, 0x4D, 0x50, 0x00, 0x00, 0x00};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 6);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("Aircraft Callsign"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BITS-017
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with ASCII encoding
 */
TEST_F(DataItemBitsTest, GetTextASCII) {
    DataItemBits bits(32);
    bits.m_strShortName = "TEXT";
    bits.m_strName = "ASCII Text";
    bits.m_nFrom = 1;
    bits.m_nTo = 32;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_ASCII;

    unsigned char data[] = {'T', 'E', 'S', 'T'};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 4);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("ASCII Text"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BITS-018
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with XML output format
 */
TEST_F(DataItemBitsTest, GetTextXMLFormat) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_strName = "System Area Code";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;

    unsigned char data[] = {0x42};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::EXML, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("<SAC>"), std::string::npos);
    EXPECT_NE(result.find("</SAC>"), std::string::npos);
    EXPECT_NE(result.find("66"), std::string::npos); // 0x42 = 66
}

/**
 * Test Case: TC-CPP-BITS-019
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with human-readable XML output format
 */
TEST_F(DataItemBitsTest, GetTextXMLHFormat) {
    DataItemBits bits(8);
    bits.m_strShortName = "SIC";
    bits.m_strName = "System Identification Code";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;

    unsigned char data[] = {0x10};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::EXMLH, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("<SIC>"), std::string::npos);
    EXPECT_NE(result.find("</SIC>"), std::string::npos);
    EXPECT_NE(result.find("16"), std::string::npos); // 0x10 = 16
}

/**
 * Test Case: TC-CPP-BITS-020
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with JSON output format
 */
TEST_F(DataItemBitsTest, GetTextJSONFormat) {
    DataItemBits bits(8);
    bits.m_strShortName = "TYP";
    bits.m_strName = "Target Type";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;

    unsigned char data[] = {0x05};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::EJSON, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("\"TYP\":"), std::string::npos);
    EXPECT_NE(result.find("5"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BITS-021
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with human-readable JSON output format
 */
TEST_F(DataItemBitsTest, GetTextJSONHFormat) {
    DataItemBits bits(16);
    bits.m_strShortName = "ALT";
    bits.m_strName = "Altitude";
    bits.m_nFrom = 1;
    bits.m_nTo = 16;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;
    bits.m_dScale = 25.0; // feet
    bits.m_strUnit = "ft";

    unsigned char data[] = {0x01, 0x00}; // 256
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::EJSONH, data, 2);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("\"ALT\":"), std::string::npos);
    // Should contain scaled value 256 * 25 = 6400
}

/**
 * Test Case: TC-CPP-BITS-022
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with extensive JSON format (EJSONE) - triggers private functions
 */
TEST_F(DataItemBitsTest, GetTextJSONExtensiveFormat) {
    DataItemBits bits(8);
    bits.m_strShortName = "STATUS";
    bits.m_strName = "Target Status";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;

    unsigned char data[] = {0xAB};
    std::string result, header;

    // EJSONE format triggers getHexBitStringFullByte()
    bool success = bits.getText(result, header, CAsterixFormat::EJSONE, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("\"STATUS\":"), std::string::npos);
    EXPECT_NE(result.find("\"val\""), std::string::npos);
    EXPECT_NE(result.find("\"hex\""), std::string::npos);
    EXPECT_NE(result.find("\"name\""), std::string::npos);
    EXPECT_NE(result.find("AB"), std::string::npos); // Hex value
}

/**
 * Test Case: TC-CPP-BITS-023
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with EJSONE format and non-byte-aligned bits - triggers mask
 */
TEST_F(DataItemBitsTest, GetTextJSONExtensiveNonByteAligned) {
    DataItemBits bits(5);
    bits.m_strShortName = "FLAG";
    bits.m_strName = "Status Flag";
    bits.m_nFrom = 2; // Non-byte aligned
    bits.m_nTo = 6;   // 5 bits total
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;

    unsigned char data[] = {0b01111000}; // Bits 2-6 = 11110 = 30
    std::string result, header;

    // EJSONE with non-byte-aligned bits triggers getHexBitStringMask()
    bool success = bits.getText(result, header, CAsterixFormat::EJSONE, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("\"FLAG\":"), std::string::npos);
    EXPECT_NE(result.find("\"val\""), std::string::npos);
    EXPECT_NE(result.find("\"hex\""), std::string::npos);
    EXPECT_NE(result.find("\"mask\""), std::string::npos); // Mask should be present
}

/**
 * Test Case: TC-CPP-BITS-024
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with EOut format (one-line output)
 */
TEST_F(DataItemBitsTest, GetTextOutFormat) {
    DataItemBits bits(8);
    bits.m_strShortName = "CNT";
    bits.m_strName = "Counter";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;

    unsigned char data[] = {0x7F};
    std::string result, header = "048.020";

    bool success = bits.getText(result, header, CAsterixFormat::EOut, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("048.020.CNT"), std::string::npos);
    EXPECT_NE(result.find("127"), std::string::npos); // 0x7F = 127
}

/**
 * Test Case: TC-CPP-BITS-025
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with max value validation warning
 */
TEST_F(DataItemBitsTest, GetTextMaxValueWarning) {
    DataItemBits bits(8);
    bits.m_strShortName = "SPEED";
    bits.m_strName = "Speed";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;
    bits.m_dScale = 1.0;
    bits.m_strUnit = "kt";
    bits.m_bMaxValueSet = true;
    bits.m_dMaxValue = 100.0;

    // Value 200 exceeds max 100
    unsigned char data[] = {200};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("Warning"), std::string::npos);
    EXPECT_NE(result.find("larger than max"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BITS-026
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with min value validation warning
 */
TEST_F(DataItemBitsTest, GetTextMinValueWarning) {
    DataItemBits bits(8);
    bits.m_strShortName = "TEMP";
    bits.m_strName = "Temperature";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_SIGNED;
    bits.m_dScale = 1.0;
    bits.m_strUnit = "C";
    bits.m_bMinValueSet = true;
    bits.m_dMinValue = -50.0;

    // Value -100 is below min -50
    unsigned char data[] = {0x9C}; // -100 in signed 8-bit
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("Warning"), std::string::npos);
    EXPECT_NE(result.find("smaller than min"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BITS-027
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with const value validation warning
 */
TEST_F(DataItemBitsTest, GetTextConstValueWarning) {
    DataItemBits bits(8);
    bits.m_strShortName = "FX";
    bits.m_strName = "Field Extension";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;
    bits.m_bIsConst = true;
    bits.m_nConst = 0;

    // Value should be 0, but is 1
    unsigned char data[] = {0x01};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("Warning"), std::string::npos);
    EXPECT_NE(result.find("should be set to"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BITS-028
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with multi-byte field (32-bit value)
 */
TEST_F(DataItemBitsTest, GetTextMultiByteField) {
    DataItemBits bits(32);
    bits.m_strShortName = "TIME";
    bits.m_strName = "Time of Day";
    bits.m_nFrom = 1;
    bits.m_nTo = 24; // 3 bytes
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;
    bits.m_dScale = 0.0078125; // 1/128 seconds
    bits.m_strUnit = "s";

    unsigned char data[] = {0x01, 0x00, 0x00, 0x00};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 4);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("Time of Day"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BITS-029
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with value lookup (BitsValue) - matching value
 */
TEST_F(DataItemBitsTest, GetTextValueLookupMatch) {
    DataItemBits bits(8);
    bits.m_strShortName = "TYPE";
    bits.m_strName = "Target Type";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;

    // Add value descriptions
    bits.m_lValue.push_back(new BitsValue(0, "Unknown"));
    bits.m_lValue.push_back(new BitsValue(1, "Primary"));
    bits.m_lValue.push_back(new BitsValue(2, "Secondary"));

    unsigned char data[] = {0x02}; // Value 2 = "Secondary"
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("Secondary"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BITS-030
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with value lookup - no match (unknown value)
 */
TEST_F(DataItemBitsTest, GetTextValueLookupNoMatch) {
    DataItemBits bits(8);
    bits.m_strShortName = "TYPE";
    bits.m_strName = "Target Type";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;

    // Add value descriptions
    bits.m_lValue.push_back(new BitsValue(1, "Primary"));
    bits.m_lValue.push_back(new BitsValue(2, "Secondary"));

    unsigned char data[] = {0xFF}; // Value 255 - not in list
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 1);
    EXPECT_TRUE(success);
    EXPECT_NE(result.find("??????"), std::string::npos); // Unknown value marker
}

/**
 * Test Case: TC-CPP-BITS-031
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with filtering enabled - should skip filtered item
 */
TEST_F(DataItemBitsTest, GetTextWithFilteringSkipped) {
    DataItemBits bits(8);
    bits.m_strShortName = "SKIP";
    bits.m_strName = "Skipped Field";
    bits.m_nFrom = 1;
    bits.m_nTo = 8;
    bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;
    bits.m_bFiltered = false; // Not in filter list

    gFiltering = true;

    unsigned char data[] = {0x42};
    std::string result, header;

    bool success = bits.getText(result, header, CAsterixFormat::ETxt, data, 1);
    EXPECT_FALSE(success); // Should return false when filtered out

    gFiltering = false;
}

/**
 * Test Case: TC-CPP-BITS-032
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getText() with all encoding types in EJSONE format
 */
TEST_F(DataItemBitsTest, GetTextAllEncodingsEJSONE) {
    // Test SIGNED with EJSONE
    {
        DataItemBits bits(8);
        bits.m_strShortName = "SIGNED";
        bits.m_strName = "Signed Value";
        bits.m_nFrom = 1;
        bits.m_nTo = 8;
        bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_SIGNED;

        unsigned char data[] = {0xFE}; // -2
        std::string result, header;

        bool success = bits.getText(result, header, CAsterixFormat::EJSONE, data, 1);
        EXPECT_TRUE(success);
        EXPECT_NE(result.find("\"hex\""), std::string::npos);
    }

    // Test SIX_BIT_CHAR with EJSONE
    {
        DataItemBits bits(24);
        bits.m_strShortName = "SIXBIT";
        bits.m_strName = "Six Bit";
        bits.m_nFrom = 1;
        bits.m_nTo = 24;
        bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_SIX_BIT_CHAR;

        unsigned char data[] = {0x51, 0x4D, 0x50};
        std::string result, header;

        bool success = bits.getText(result, header, CAsterixFormat::EJSONE, data, 3);
        EXPECT_TRUE(success);
        EXPECT_NE(result.find("\"hex\""), std::string::npos);
    }

    // Test OCTAL with EJSONE
    {
        DataItemBits bits(12);
        bits.m_strShortName = "OCT";
        bits.m_strName = "Octal";
        bits.m_nFrom = 1;
        bits.m_nTo = 12;
        bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_OCTAL;

        unsigned char data[] = {0x07, 0xFF};
        std::string result, header;

        bool success = bits.getText(result, header, CAsterixFormat::EJSONE, data, 2);
        EXPECT_TRUE(success);
        EXPECT_NE(result.find("\"hex\""), std::string::npos);
    }

    // Test ASCII with EJSONE
    {
        DataItemBits bits(32);
        bits.m_strShortName = "ASCII";
        bits.m_strName = "ASCII Text";
        bits.m_nFrom = 1;
        bits.m_nTo = 32;
        bits.m_eEncoding = DataItemBits::DATAITEM_ENCODING_ASCII;

        unsigned char data[] = {'A', 'B', 'C', 'D'};
        std::string result, header;

        bool success = bits.getText(result, header, CAsterixFormat::EJSONE, data, 4);
        EXPECT_TRUE(success);
        EXPECT_NE(result.find("\"hex\""), std::string::npos);
    }
}

// Run all tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
