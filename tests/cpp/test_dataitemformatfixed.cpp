/**
 * Unit tests for DataItemFormatFixed class
 *
 * These tests comprehensively cover DataItemFormatFixed.cpp to improve
 * coverage from ~78% toward 90%+ target (expected +0.6 pp overall impact).
 *
 * DataItemFormatFixed is the simplest ASTERIX data format:
 * - Fixed-length data items (1-N bytes)
 * - Contains multiple bit fields (DataItemBits)
 * - Length determined at configuration time
 * - No dynamic sizing or extensions
 *
 * Test Coverage Areas:
 * 1. Constructor/Destructor (4 tests)
 * 2. Length Operations (4 tests)
 * 3. Bit Management (5 tests)
 * 4. getText() Method (8 tests)
 * 5. Edge Cases (4 tests)
 *
 * Requirements Coverage:
 * - REQ-LLR-FIXED-001: Fixed format parsing and length management
 * - REQ-LLR-FIXED-002: Bit field management and iteration
 * - REQ-LLR-FIXED-003: Output formatting in multiple formats
 * - REQ-LLR-FIXED-004: Extension bit handling (FX)
 * - REQ-LLR-FIXED-005: Filtering and descriptor management
 */

#include <gtest/gtest.h>
#include "../../src/asterix/DataItemFormatFixed.h"
#include "../../src/asterix/DataItemBits.h"
#include "../../src/asterix/Tracer.h"
#include "../../src/asterix/asterixformat.hxx"
#include <cstring>

// Global variables required by ASTERIX library
bool gVerbose = false;
bool gFiltering = false;

class DataItemFormatFixedTest : public ::testing::Test {
protected:
    void SetUp() override {
        gVerbose = false;
        gFiltering = false;
    }

    void TearDown() override {
        gFiltering = false;
    }

    /**
     * Helper: Create simple 1-byte fixed format with one 8-bit field
     */
    DataItemFormatFixed* createSimpleFixed() {
        DataItemFormatFixed* format = new DataItemFormatFixed(1);
        format->m_nLength = 1;

        DataItemBits* bits = new DataItemBits(8);
        bits->m_nFrom = 8;
        bits->m_nTo = 1;
        bits->m_strShortName = "DATA";
        bits->m_strName = "Data Field";
        bits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;

        format->m_lSubItems.push_back(bits);
        return format;
    }

    /**
     * Helper: Create 2-byte fixed format with two 8-bit fields
     */
    DataItemFormatFixed* createTwoByteFixed() {
        DataItemFormatFixed* format = new DataItemFormatFixed(2);
        format->m_nLength = 2;

        // First byte (bits 16-9)
        DataItemBits* bits1 = new DataItemBits(8);
        bits1->m_nFrom = 16;
        bits1->m_nTo = 9;
        bits1->m_strShortName = "BYTE1";
        bits1->m_strName = "First Byte";
        bits1->m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;
        format->m_lSubItems.push_back(bits1);

        // Second byte (bits 8-1)
        DataItemBits* bits2 = new DataItemBits(8);
        bits2->m_nFrom = 8;
        bits2->m_nTo = 1;
        bits2->m_strShortName = "BYTE2";
        bits2->m_strName = "Second Byte";
        bits2->m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;
        format->m_lSubItems.push_back(bits2);

        return format;
    }

    /**
     * Helper: Create format with 16-bit field
     */
    DataItemFormatFixed* create16BitFixed() {
        DataItemFormatFixed* format = new DataItemFormatFixed(3);
        format->m_nLength = 2;

        DataItemBits* bits = new DataItemBits(16);
        bits->m_nFrom = 16;
        bits->m_nTo = 1;
        bits->m_strShortName = "WORD";
        bits->m_strName = "16-bit Word";
        bits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;

        format->m_lSubItems.push_back(bits);
        return format;
    }
};

// ============================================================================
// Test Group 1: Constructor/Destructor (4 tests)
// ============================================================================

/**
 * Test Case: TC-CPP-FIXED-001
 * Requirement: REQ-LLR-FIXED-001
 *
 * Test default constructor
 */
TEST_F(DataItemFormatFixedTest, DefaultConstructor) {
    DataItemFormatFixed format;

    EXPECT_EQ(format.m_nID, 0);
    EXPECT_EQ(format.m_nLength, 0);
    EXPECT_TRUE(format.m_lSubItems.empty());
    EXPECT_TRUE(format.isFixed());
}

/**
 * Test Case: TC-CPP-FIXED-002
 * Requirement: REQ-LLR-FIXED-001
 *
 * Test constructor with ID
 */
TEST_F(DataItemFormatFixedTest, ConstructorWithID) {
    DataItemFormatFixed format(42);

    EXPECT_EQ(format.m_nID, 42);
    EXPECT_EQ(format.m_nLength, 0);
    EXPECT_TRUE(format.m_lSubItems.empty());
}

/**
 * Test Case: TC-CPP-FIXED-003
 * Requirement: REQ-LLR-FIXED-001
 *
 * Test copy constructor creates deep copy
 */
TEST_F(DataItemFormatFixedTest, CopyConstructorDeepCopy) {
    DataItemFormatFixed* original = createSimpleFixed();

    // Copy constructor
    DataItemFormatFixed copy(*original);

    // Verify copied values
    EXPECT_EQ(copy.m_nID, original->m_nID);
    EXPECT_EQ(copy.m_nLength, original->m_nLength);
    EXPECT_EQ(copy.m_lSubItems.size(), original->m_lSubItems.size());

    // Verify deep copy (different objects)
    EXPECT_NE(&copy.m_lSubItems, &original->m_lSubItems);

    // Verify bit items are cloned (not same pointer)
    if (!copy.m_lSubItems.empty() && !original->m_lSubItems.empty()) {
        DataItemBits* copyBits = (DataItemBits*)copy.m_lSubItems.front();
        DataItemBits* origBits = (DataItemBits*)original->m_lSubItems.front();
        EXPECT_NE(copyBits, origBits) << "Bit items should be cloned, not shared";
        EXPECT_EQ(copyBits->m_strShortName, origBits->m_strShortName);
    }

    delete original;
}

/**
 * Test Case: TC-CPP-FIXED-004
 * Requirement: REQ-LLR-FIXED-001
 *
 * Test destructor cleans up bit items
 */
TEST_F(DataItemFormatFixedTest, DestructorCleansUpBitItems) {
    DataItemFormatFixed* format = createTwoByteFixed();

    EXPECT_EQ(format->m_lSubItems.size(), 2);

    // Delete should clean up all bit items
    delete format;

    // If we get here without crash, destructor worked
    SUCCEED();
}

// ============================================================================
// Test Group 2: Length Operations (4 tests)
// ============================================================================

/**
 * Test Case: TC-CPP-FIXED-005
 * Requirement: REQ-LLR-FIXED-001
 *
 * Test getLength() returns correct value for 1 byte
 */
TEST_F(DataItemFormatFixedTest, GetLengthOneByte) {
    DataItemFormatFixed format(1);
    format.m_nLength = 1;

    EXPECT_EQ(format.getLength(), 1);
}

/**
 * Test Case: TC-CPP-FIXED-006
 * Requirement: REQ-LLR-FIXED-001
 *
 * Test getLength() for multi-byte formats
 */
TEST_F(DataItemFormatFixedTest, GetLengthMultiBytes) {
    // Test 2 bytes
    DataItemFormatFixed format2(2);
    format2.m_nLength = 2;
    EXPECT_EQ(format2.getLength(), 2);

    // Test 4 bytes
    DataItemFormatFixed format4(3);
    format4.m_nLength = 4;
    EXPECT_EQ(format4.getLength(), 4);

    // Test 8 bytes
    DataItemFormatFixed format8(4);
    format8.m_nLength = 8;
    EXPECT_EQ(format8.getLength(), 8);
}

/**
 * Test Case: TC-CPP-FIXED-007
 * Requirement: REQ-LLR-FIXED-001
 *
 * Test getLength(pData) returns same value regardless of data
 */
TEST_F(DataItemFormatFixedTest, GetLengthWithDataParameter) {
    DataItemFormatFixed* format = createTwoByteFixed();

    unsigned char data1[] = {0x00, 0x00};
    unsigned char data2[] = {0xFF, 0xFF};
    unsigned char data3[] = {0x12, 0x34};

    // Fixed format always returns same length
    EXPECT_EQ(format->getLength(data1), 2);
    EXPECT_EQ(format->getLength(data2), 2);
    EXPECT_EQ(format->getLength(data3), 2);

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-008
 * Requirement: REQ-LLR-FIXED-001
 *
 * Test getLength() with zero length
 */
TEST_F(DataItemFormatFixedTest, GetLengthZero) {
    DataItemFormatFixed format;
    format.m_nLength = 0;

    EXPECT_EQ(format.getLength(), 0);

    unsigned char data[] = {0x00};
    EXPECT_EQ(format.getLength(data), 0);
}

// ============================================================================
// Test Group 3: Bit Management (5 tests)
// ============================================================================

/**
 * Test Case: TC-CPP-FIXED-009
 * Requirement: REQ-LLR-FIXED-002
 *
 * Test adding single bit field
 */
TEST_F(DataItemFormatFixedTest, AddSingleBitField) {
    DataItemFormatFixed format(1);
    format.m_nLength = 1;

    DataItemBits* bits = new DataItemBits(8);
    bits->m_strShortName = "DATA";
    format.m_lSubItems.push_back(bits);

    EXPECT_EQ(format.m_lSubItems.size(), 1);
}

/**
 * Test Case: TC-CPP-FIXED-010
 * Requirement: REQ-LLR-FIXED-002
 *
 * Test adding multiple bit fields
 */
TEST_F(DataItemFormatFixedTest, AddMultipleBitFields) {
    DataItemFormatFixed format(2);
    format.m_nLength = 2;

    // Add 8-bit field
    DataItemBits* bits1 = new DataItemBits(8);
    bits1->m_strShortName = "BYTE1";
    format.m_lSubItems.push_back(bits1);

    // Add another 8-bit field
    DataItemBits* bits2 = new DataItemBits(8);
    bits2->m_strShortName = "BYTE2";
    format.m_lSubItems.push_back(bits2);

    EXPECT_EQ(format.m_lSubItems.size(), 2);
}

/**
 * Test Case: TC-CPP-FIXED-011
 * Requirement: REQ-LLR-FIXED-002
 *
 * Test 16-bit field spanning 2 bytes
 */
TEST_F(DataItemFormatFixedTest, BitFieldSpanningBytes) {
    DataItemFormatFixed* format = create16BitFixed();

    EXPECT_EQ(format->m_lSubItems.size(), 1);

    DataItemBits* bits = (DataItemBits*)format->m_lSubItems.front();
    EXPECT_STREQ(bits->m_strShortName.c_str(), "WORD");

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-012
 * Requirement: REQ-LLR-FIXED-002
 *
 * Test multiple bit fields with different sizes
 */
TEST_F(DataItemFormatFixedTest, MultipleBitFieldsDifferentSizes) {
    DataItemFormatFixed format(3);
    format.m_nLength = 4;

    // 16-bit field
    DataItemBits* bits16 = new DataItemBits(16);
    bits16->m_strShortName = "WORD";
    format.m_lSubItems.push_back(bits16);

    // 8-bit field
    DataItemBits* bits8 = new DataItemBits(8);
    bits8->m_strShortName = "BYTE";
    format.m_lSubItems.push_back(bits8);

    // 4-bit field
    DataItemBits* bits4 = new DataItemBits(4);
    bits4->m_strShortName = "NIBBLE";
    format.m_lSubItems.push_back(bits4);

    EXPECT_EQ(format.m_lSubItems.size(), 3);
}

/**
 * Test Case: TC-CPP-FIXED-013
 * Requirement: REQ-LLR-FIXED-002
 *
 * Test bit field ordering maintained
 */
TEST_F(DataItemFormatFixedTest, BitFieldOrderingMaintained) {
    DataItemFormatFixed format(4);
    format.m_nLength = 3;

    DataItemBits* bits1 = new DataItemBits(1);
    bits1->m_strShortName = "FIRST";
    format.m_lSubItems.push_back(bits1);

    DataItemBits* bits2 = new DataItemBits(2);
    bits2->m_strShortName = "SECOND";
    format.m_lSubItems.push_back(bits2);

    DataItemBits* bits3 = new DataItemBits(3);
    bits3->m_strShortName = "THIRD";
    format.m_lSubItems.push_back(bits3);

    // Check order
    auto it = format.m_lSubItems.begin();
    EXPECT_STREQ(((DataItemBits*)*it)->m_strShortName.c_str(), "FIRST");
    ++it;
    EXPECT_STREQ(((DataItemBits*)*it)->m_strShortName.c_str(), "SECOND");
    ++it;
    EXPECT_STREQ(((DataItemBits*)*it)->m_strShortName.c_str(), "THIRD");
}

// ============================================================================
// Test Group 4: getText() Method (8 tests)
// ============================================================================

/**
 * Test Case: TC-CPP-FIXED-014
 * Requirement: REQ-LLR-FIXED-003
 *
 * Test getText() with ETxt format
 */
TEST_F(DataItemFormatFixedTest, GetTextTxtFormat) {
    DataItemFormatFixed* format = createSimpleFixed();

    unsigned char data[] = {0x42};
    std::string result, header;

    bool success = format->getText(result, header, CAsterixFormat::ETxt, data, 1);

    EXPECT_TRUE(success);
    EXPECT_GT(result.length(), 0);
    EXPECT_NE(result.find("Data Field"), std::string::npos);

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-015
 * Requirement: REQ-LLR-FIXED-003
 *
 * Test getText() with JSON format
 */
TEST_F(DataItemFormatFixedTest, GetTextJSONFormat) {
    DataItemFormatFixed* format = createSimpleFixed();

    unsigned char data[] = {0x42};
    std::string result, header;

    bool success = format->getText(result, header, CAsterixFormat::EJSON, data, 1);

    EXPECT_TRUE(success);
    EXPECT_GT(result.length(), 0);
    // JSON should start with { and end with }
    EXPECT_EQ(result[0], '{');
    EXPECT_EQ(result[result.length() - 1], '}');
    EXPECT_NE(result.find("\"DATA\""), std::string::npos);

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-016
 * Requirement: REQ-LLR-FIXED-003
 *
 * Test getText() with JSONH format
 */
TEST_F(DataItemFormatFixedTest, GetTextJSONHFormat) {
    DataItemFormatFixed* format = createSimpleFixed();

    unsigned char data[] = {0x42};
    std::string result, header;

    bool success = format->getText(result, header, CAsterixFormat::EJSONH, data, 1);

    EXPECT_TRUE(success);
    EXPECT_GT(result.length(), 0);
    EXPECT_EQ(result[0], '{');
    EXPECT_EQ(result[result.length() - 1], '}');

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-017
 * Requirement: REQ-LLR-FIXED-003
 *
 * Test getText() with EJSONE format
 */
TEST_F(DataItemFormatFixedTest, GetTextJSONExtensiveFormat) {
    DataItemFormatFixed* format = createSimpleFixed();

    unsigned char data[] = {0x42};
    std::string result, header;

    bool success = format->getText(result, header, CAsterixFormat::EJSONE, data, 1);

    EXPECT_TRUE(success);
    EXPECT_GT(result.length(), 0);
    EXPECT_EQ(result[0], '{');
    EXPECT_EQ(result[result.length() - 1], '}');

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-018
 * Requirement: REQ-LLR-FIXED-003
 *
 * Test getText() with multiple bit fields
 */
TEST_F(DataItemFormatFixedTest, GetTextMultipleBitFields) {
    DataItemFormatFixed* format = createTwoByteFixed();

    unsigned char data[] = {0x12, 0x34};
    std::string result, header;

    bool success = format->getText(result, header, CAsterixFormat::ETxt, data, 2);

    EXPECT_TRUE(success);
    EXPECT_GT(result.length(), 0);
    EXPECT_NE(result.find("First Byte"), std::string::npos);
    EXPECT_NE(result.find("Second Byte"), std::string::npos);

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-019
 * Requirement: REQ-LLR-FIXED-003
 *
 * Test getText() with length mismatch (error case)
 */
TEST_F(DataItemFormatFixedTest, GetTextLengthMismatch) {
    DataItemFormatFixed* format = createTwoByteFixed();

    unsigned char data[] = {0x12}; // Only 1 byte, but format expects 2
    std::string result, header;

    // Should return false due to length mismatch
    bool success = format->getText(result, header, CAsterixFormat::ETxt, data, 1);

    EXPECT_FALSE(success);

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-020
 * Requirement: REQ-LLR-FIXED-003
 *
 * Test getText() with 16-bit field
 */
TEST_F(DataItemFormatFixedTest, GetText16BitField) {
    DataItemFormatFixed* format = create16BitFixed();

    unsigned char data[] = {0x12, 0x34};
    std::string result, header;

    bool success = format->getText(result, header, CAsterixFormat::ETxt, data, 2);

    EXPECT_TRUE(success);
    EXPECT_GT(result.length(), 0);
    EXPECT_NE(result.find("16-bit Word"), std::string::npos);

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-021
 * Requirement: REQ-LLR-FIXED-003
 *
 * Test getText() all format types
 */
TEST_F(DataItemFormatFixedTest, GetTextAllFormats) {
    DataItemFormatFixed* format = createSimpleFixed();

    unsigned char data[] = {0x42};

    // Test all applicable output formats
    std::vector<int> formats = {
        CAsterixFormat::ETxt,
        CAsterixFormat::EJSON,
        CAsterixFormat::EJSONH,
        CAsterixFormat::EJSONE,
        CAsterixFormat::EXML,
        CAsterixFormat::EXMLH
    };

    for (int fmt : formats) {
        std::string result, header;
        bool success = format->getText(result, header, fmt, data, 1);
        EXPECT_TRUE(success) << "Failed for format " << fmt;
        EXPECT_GT(result.length(), 0) << "Empty result for format " << fmt;
    }

    delete format;
}

// ============================================================================
// Test Group 5: Advanced Features (7 tests)
// ============================================================================

/**
 * Test Case: TC-CPP-FIXED-022
 * Requirement: REQ-LLR-FIXED-001
 *
 * Test clone() creates polymorphic copy
 */
TEST_F(DataItemFormatFixedTest, CloneCreatesPolymorphicCopy) {
    DataItemFormatFixed* original = createSimpleFixed();

    DataItemFormatFixed* copy = original->clone();

    ASSERT_NE(copy, nullptr);
    EXPECT_NE(copy, original) << "Clone should be different object";
    EXPECT_EQ(copy->m_nID, original->m_nID);
    EXPECT_EQ(copy->m_nLength, original->m_nLength);
    EXPECT_EQ(copy->m_lSubItems.size(), original->m_lSubItems.size());

    delete original;
    delete copy;
}

/**
 * Test Case: TC-CPP-FIXED-023
 * Requirement: REQ-LLR-FIXED-005
 *
 * Test printDescriptors() output
 */
TEST_F(DataItemFormatFixedTest, PrintDescriptors) {
    DataItemFormatFixed* format = createSimpleFixed();

    std::string result = format->printDescriptors("  ");

    EXPECT_GT(result.length(), 0);
    EXPECT_NE(result.find("DATA"), std::string::npos);

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-024
 * Requirement: REQ-LLR-FIXED-005
 *
 * Test filterOutItem() functionality
 */
TEST_F(DataItemFormatFixedTest, FilterOutItem) {
    DataItemFormatFixed* format = createTwoByteFixed();

    // Filter for BYTE1
    bool filtered = format->filterOutItem("BYTE1");

    EXPECT_TRUE(filtered);

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-025
 * Requirement: REQ-LLR-FIXED-005
 *
 * Test isFiltered() functionality
 */
TEST_F(DataItemFormatFixedTest, IsFiltered) {
    DataItemFormatFixed* format = createTwoByteFixed();

    // Filter for BYTE1
    format->filterOutItem("BYTE1");

    // Check if filtered
    bool filtered = format->isFiltered("BYTE1");

    EXPECT_TRUE(filtered);

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-026
 * Requirement: REQ-LLR-FIXED-005
 *
 * Test getDescription() functionality
 */
TEST_F(DataItemFormatFixedTest, GetDescription) {
    DataItemFormatFixed* format = createSimpleFixed();

    const char* desc = format->getDescription("DATA", NULL);

    ASSERT_NE(desc, nullptr);
    EXPECT_STREQ(desc, "Data Field");

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-027
 * Requirement: REQ-LLR-FIXED-004
 *
 * Test isLastPart() with extension bit set
 */
TEST_F(DataItemFormatFixedTest, IsLastPartWithExtensionBitSet) {
    DataItemFormatFixed format(1);
    format.m_nLength = 1;

    // Add FX bit (extension bit)
    DataItemBits* bits = new DataItemBits(1);
    bits->m_nFrom = 1;
    bits->m_nTo = 1;
    bits->m_strShortName = "FX";
    bits->m_bExtension = true;
    format.m_lSubItems.push_back(bits);

    // Data with FX bit = 1 (more data follows)
    unsigned char data1[] = {0x01};
    EXPECT_FALSE(format.isLastPart(data1)) << "FX=1 means more data follows";

    // Data with FX bit = 0 (last part)
    unsigned char data2[] = {0x00};
    EXPECT_TRUE(format.isLastPart(data2)) << "FX=0 means last part";
}

/**
 * Test Case: TC-CPP-FIXED-028
 * Requirement: REQ-LLR-FIXED-004
 *
 * Test isLastPart() without extension bit
 */
TEST_F(DataItemFormatFixedTest, IsLastPartNoExtensionBit) {
    DataItemFormatFixed* format = createSimpleFixed();

    unsigned char data[] = {0x42};

    // No extension bit means it's always last part
    EXPECT_TRUE(format->isLastPart(data));

    delete format;
}

/**
 * Test Case: TC-CPP-FIXED-029
 * Requirement: REQ-LLR-FIXED-004
 *
 * Test isSecondaryPartPresent() functionality
 */
TEST_F(DataItemFormatFixedTest, IsSecondaryPartPresent) {
    DataItemFormatFixed format(2);
    format.m_nLength = 1;

    // Add bit with presence field
    DataItemBits* bits = new DataItemBits(1);
    bits->m_nFrom = 2;
    bits->m_nTo = 2;
    bits->m_strShortName = "PRESENT";
    bits->m_nPresenceOfField = 1;
    format.m_lSubItems.push_back(bits);

    // Data with bit 2 set (present)
    unsigned char data1[] = {0x02};
    EXPECT_TRUE(format.isSecondaryPartPresent(data1, 1));

    // Data with bit 2 clear (not present)
    unsigned char data2[] = {0x00};
    EXPECT_FALSE(format.isSecondaryPartPresent(data2, 1));
}

/**
 * Test Case: TC-CPP-FIXED-030
 * Requirement: REQ-LLR-FIXED-004
 *
 * Test getPartName() functionality
 */
TEST_F(DataItemFormatFixedTest, GetPartName) {
    DataItemFormatFixed format(3);
    format.m_nLength = 1;

    // Add bit with presence field
    DataItemBits* bits = new DataItemBits(1);
    bits->m_nFrom = 1;
    bits->m_nTo = 1;
    bits->m_strShortName = "PART1";
    bits->m_strName = "First Part";
    bits->m_nPresenceOfField = 1;
    format.m_lSubItems.push_back(bits);

    std::string& name = format.getPartName(1);

    EXPECT_EQ(name, "PART1");
}

/**
 * Test Case: TC-CPP-FIXED-031
 * Requirement: REQ-LLR-FIXED-001
 *
 * Test isFixed() returns true
 */
TEST_F(DataItemFormatFixedTest, IsFixedReturnsTrue) {
    DataItemFormatFixed format;

    EXPECT_TRUE(format.isFixed());
    EXPECT_FALSE(format.isVariable());
    EXPECT_FALSE(format.isRepetitive());
    EXPECT_FALSE(format.isCompound());
    EXPECT_FALSE(format.isExplicit());
    EXPECT_FALSE(format.isBDS());
    EXPECT_FALSE(format.isBits());
}

/**
 * Test Case: TC-CPP-FIXED-032
 * Requirement: REQ-LLR-FIXED-003
 *
 * Test getText() with empty format (no bits)
 */
TEST_F(DataItemFormatFixedTest, GetTextEmptyFormat) {
    DataItemFormatFixed format(10);
    format.m_nLength = 1;
    // No bits added

    unsigned char data[] = {0x42};
    std::string result, header;

    bool success = format.getText(result, header, CAsterixFormat::ETxt, data, 1);

    // Should return false because no bits to process
    EXPECT_FALSE(success);
}

/**
 * Test Case: TC-CPP-FIXED-033
 * Requirement: REQ-LLR-FIXED-004
 *
 * Test isLastPart() with extension bit in multi-byte format
 */
TEST_F(DataItemFormatFixedTest, IsLastPartMultiByteExtension) {
    DataItemFormatFixed format(11);
    format.m_nLength = 2;

    // Add FX bit at bit position 1 (last bit of second byte)
    DataItemBits* bits = new DataItemBits(1);
    bits->m_nFrom = 1;
    bits->m_nTo = 1;
    bits->m_strShortName = "FX";
    bits->m_bExtension = true;
    format.m_lSubItems.push_back(bits);

    // Data with FX bit = 1 in second byte
    unsigned char data1[] = {0x00, 0x01};
    EXPECT_FALSE(format.isLastPart(data1));

    // Data with FX bit = 0 in second byte
    unsigned char data2[] = {0x00, 0x00};
    EXPECT_TRUE(format.isLastPart(data2));
}

/**
 * Test Case: TC-CPP-FIXED-034
 * Requirement: REQ-LLR-FIXED-004
 *
 * Test isLastPart() with extension bit at different positions
 */
TEST_F(DataItemFormatFixedTest, IsLastPartExtensionBitPositions) {
    // Test FX at bit 8 (first byte, last bit)
    DataItemFormatFixed format1(12);
    format1.m_nLength = 1;

    DataItemBits* bits1 = new DataItemBits(1);
    bits1->m_nFrom = 8;
    bits1->m_nTo = 8;
    bits1->m_strShortName = "FX";
    bits1->m_bExtension = true;
    format1.m_lSubItems.push_back(bits1);

    unsigned char data_fx8_set[] = {0x80};
    unsigned char data_fx8_clear[] = {0x00};

    EXPECT_FALSE(format1.isLastPart(data_fx8_set));
    EXPECT_TRUE(format1.isLastPart(data_fx8_clear));
}

/**
 * Test Case: TC-CPP-FIXED-035
 * Requirement: REQ-LLR-FIXED-003
 *
 * Test getText() with JSON format and trailing comma removal
 */
TEST_F(DataItemFormatFixedTest, GetTextJSONTrailingCommaRemoval) {
    DataItemFormatFixed* format = createTwoByteFixed();

    unsigned char data[] = {0x12, 0x34};
    std::string result, header;

    bool success = format->getText(result, header, CAsterixFormat::EJSON, data, 2);

    EXPECT_TRUE(success);
    // Should not end with ",}" - comma should be replaced
    EXPECT_EQ(result.find(",}"), std::string::npos);
    // Should end with }
    EXPECT_EQ(result[result.length() - 1], '}');

    delete format;
}

// Run all tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
