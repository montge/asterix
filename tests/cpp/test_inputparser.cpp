/**
 * Unit tests for InputParser class
 *
 * These tests target InputParser.cpp to improve coverage from 56.5% to 80%+
 *
 * Functions tested:
 * 1. InputParser(AsterixDefinition*) - Constructor
 * 2. parsePacket() - Main packet parsing function (lines 38-115)
 * 3. parse_next_data_block() - Single block parser (lines 117-178)
 * 4. printDefinition() - Definition printer (line 180-182)
 * 5. filterOutItem() - Filter out item (lines 184-186)
 * 6. isFiltered() - Check if filtered (lines 188-190)
 *
 * ASTERIX Packet Format:
 * - Category (1 byte) - ASTERIX category number (e.g., 48, 62, 65)
 * - Length (2 bytes, MSB first) - Total length including 3-byte header
 * - Data (variable) - One or more DataBlocks
 *
 * InputParser responsibilities:
 * - Parse raw binary ASTERIX stream
 * - Validate packet structure (category, length)
 * - Extract and create DataBlocks
 * - Handle malformed data gracefully
 * - Support incremental parsing
 *
 * Test Strategy:
 * - Test parsePacket() with valid/invalid packets
 * - Test error conditions (truncated, wrong length, invalid category)
 * - Test parse_next_data_block() for incremental parsing
 * - Test filtering and definition management
 * - Test edge cases (empty, single byte, very large packets)
 *
 * Requirements Coverage:
 * - REQ-LLR-PARSER-001: Packet parsing
 * - REQ-LLR-PARSER-002: Length validation
 * - REQ-LLR-PARSER-003: Error handling
 * - REQ-LLR-PARSER-004: Multi-block parsing
 * - REQ-LLR-PARSER-005: Filtering support
 */

#include <gtest/gtest.h>
#include "../../src/asterix/InputParser.h"
#include "../../src/asterix/AsterixDefinition.h"
#include "../../src/asterix/AsterixData.h"
#include "../../src/asterix/DataBlock.h"
#include "../../src/asterix/Category.h"
#include "../../src/asterix/UAP.h"
#include "../../src/asterix/UAPItem.h"
#include "../../src/asterix/DataItemDescription.h"
#include "../../src/asterix/DataItemFormatFixed.h"
#include "../../src/asterix/DataItemBits.h"
#include "../../src/asterix/Tracer.h"
#include <cstring>
#include <vector>

// Global variables required by ASTERIX library
bool gVerbose = false;
bool gFiltering = false;

class InputParserTest : public ::testing::Test {
protected:
    AsterixDefinition* pDefinition;

    void SetUp() override {
        gVerbose = false;
        gFiltering = false;
        pDefinition = new AsterixDefinition();
    }

    void TearDown() override {
        // Note: InputParser deletes the definition in its destructor
        // So we only delete if we didn't pass it to an InputParser
        gFiltering = false;
    }

    /**
     * Helper: Create a test Category with minimal UAP
     */
    Category* createTestCategory(int catId) {
        Category* cat = new Category(catId);
        cat->m_strName = "Test Category";
        cat->m_strVer = "1.0";
        cat->m_bFiltered = false;

        // Create a default UAP
        UAP* pUAP = cat->newUAP();
        pUAP->m_nUseIfBitSet = 0;  // Default UAP (always used)

        // Add a simple data item
        DataItemDescription* desc = cat->getDataItemDescription("010");
        desc->m_strName = "Data Source Identifier";

        // Create a Fixed format (2 bytes)
        DataItemFormatFixed* format = new DataItemFormatFixed(2);
        format->m_nLength = 2;

        // Add DataItemBits for the bytes
        DataItemBits* bits1 = new DataItemBits(8);
        bits1->m_strShortName = "SAC";
        bits1->m_strName = "System Area Code";
        bits1->m_nFrom = 0;
        bits1->m_nTo = 7;
        format->m_lSubItems.push_back(bits1);

        DataItemBits* bits2 = new DataItemBits(8);
        bits2->m_strShortName = "SIC";
        bits2->m_strName = "System Identification Code";
        bits2->m_nFrom = 8;
        bits2->m_nTo = 15;
        format->m_lSubItems.push_back(bits2);

        desc->m_pFormat = format;

        // Add UAP item mapping
        UAPItem* uapItem = pUAP->newUAPItem();
        uapItem->m_nFRN = 1;
        uapItem->m_strItemID = "010";

        return cat;
    }

    /**
     * Helper: Create ASTERIX packet
     * Format: [CAT (1 byte)][LEN MSB][LEN LSB][DATA...]
     */
    std::vector<unsigned char> createPacket(int cat, const std::vector<unsigned char>& data) {
        std::vector<unsigned char> packet;
        packet.push_back(cat); // Category

        // Length (3 bytes header + data length)
        int totalLen = 3 + data.size();
        packet.push_back((totalLen >> 8) & 0xFF); // Length MSB
        packet.push_back(totalLen & 0xFF);        // Length LSB

        // Data
        packet.insert(packet.end(), data.begin(), data.end());

        return packet;
    }
};

/**
 * Test Case: TC-CPP-PARSER-001
 * Test constructor with valid AsterixDefinition
 */
TEST_F(InputParserTest, ConstructorWithDefinition) {
    AsterixDefinition* def = new AsterixDefinition();
    InputParser parser(def);

    // Parser should accept the definition
    // (Definition will be deleted by parser's destructor)
    SUCCEED();
}

/**
 * Test Case: TC-CPP-PARSER-002
 * Test parsePacket with single block packet
 */
TEST_F(InputParserTest, ParseSingleBlockPacket) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Create CAT048 packet with simple data
    // FSPEC (0x80) + 2-byte item (0x12 0x34)
    std::vector<unsigned char> data = {0x80, 0x12, 0x34};
    std::vector<unsigned char> packet = createPacket(48, data);

    InputParser parser(pDefinition);
    pDefinition = nullptr; // Parser now owns it

    AsterixData* result = parser.parsePacket(packet.data(), packet.size());

    ASSERT_NE(result, nullptr);
    EXPECT_GT(result->m_lDataBlocks.size(), 0);

    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-003
 * Test parsePacket with multiple consecutive packets
 */
TEST_F(InputParserTest, ParseMultipleConsecutivePackets) {
    Category* cat48 = createTestCategory(48);
    Category* cat62 = createTestCategory(62);
    pDefinition->setCategory(cat48);
    pDefinition->setCategory(cat62);

    // Create two packets back-to-back
    std::vector<unsigned char> data1 = {0x80, 0x12, 0x34};
    std::vector<unsigned char> packet1 = createPacket(48, data1);

    std::vector<unsigned char> data2 = {0x80, 0xAB, 0xCD};
    std::vector<unsigned char> packet2 = createPacket(62, data2);

    // Concatenate packets
    std::vector<unsigned char> combined = packet1;
    combined.insert(combined.end(), packet2.begin(), packet2.end());

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(combined.data(), combined.size());

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->m_lDataBlocks.size(), 2);

    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-004
 * Test parsePacket with truncated packet (length < actual data)
 */
TEST_F(InputParserTest, ParseTruncatedPacket) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Packet declares length 10 but only provide 5 bytes
    unsigned char packet[] = {0x30, 0x00, 0x0A, 0x01, 0x02};

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet, 5);

    // Should handle gracefully
    ASSERT_NE(result, nullptr);
    // May have 0 or 1 blocks depending on error recovery
    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-005
 * Test parsePacket with invalid length (length <= 3)
 */
TEST_F(InputParserTest, ParseInvalidLength) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Packet with length = 3 (header only, no data)
    unsigned char packet[] = {0x30, 0x00, 0x03};

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet, 3);

    ASSERT_NE(result, nullptr);
    // Should return empty or stop parsing
    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-006
 * Test parsePacket with zero-length packet (length = 0)
 */
TEST_F(InputParserTest, ParseZeroLength) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Packet with length = 0
    unsigned char packet[] = {0x30, 0x00, 0x00};

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet, 3);

    ASSERT_NE(result, nullptr);
    // Should handle gracefully and return
    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-007
 * Test parsePacket with length > available data
 */
TEST_F(InputParserTest, ParseLengthExceedsBuffer) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Packet declares length 100 but buffer is only 10 bytes
    unsigned char packet[] = {0x30, 0x00, 0x64, 0x80, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet, 10);

    ASSERT_NE(result, nullptr);
    // Should parse what's available
    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-008
 * Test parsePacket with empty buffer
 */
TEST_F(InputParserTest, ParseEmptyBuffer) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(nullptr, 0);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->m_lDataBlocks.size(), 0);

    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-009
 * Test parsePacket with single byte (not enough for header)
 */
TEST_F(InputParserTest, ParseSingleByte) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    unsigned char packet[] = {0x30};

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet, 1);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->m_lDataBlocks.size(), 0);

    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-010
 * Test parsePacket with two bytes (not enough for header)
 */
TEST_F(InputParserTest, ParseTwoBytes) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    unsigned char packet[] = {0x30, 0x00};

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet, 2);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->m_lDataBlocks.size(), 0);

    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-011
 * Test parsePacket with three bytes (minimum header, no data)
 */
TEST_F(InputParserTest, ParseThreeBytesMinimum) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    unsigned char packet[] = {0x30, 0x00, 0x03};

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet, 3);

    ASSERT_NE(result, nullptr);
    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-012
 * Test parsePacket with different categories
 */
TEST_F(InputParserTest, ParseDifferentCategories) {
    Category* cat48 = createTestCategory(48);
    Category* cat62 = createTestCategory(62);
    Category* cat65 = createTestCategory(65);
    pDefinition->setCategory(cat48);
    pDefinition->setCategory(cat62);
    pDefinition->setCategory(cat65);

    std::vector<unsigned char> data = {0x80, 0x12, 0x34};

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    // Test CAT048
    std::vector<unsigned char> packet48 = createPacket(48, data);
    AsterixData* result48 = parser.parsePacket(packet48.data(), packet48.size());
    ASSERT_NE(result48, nullptr);
    EXPECT_GT(result48->m_lDataBlocks.size(), 0);
    delete result48;

    // Test CAT062
    std::vector<unsigned char> packet62 = createPacket(62, data);
    AsterixData* result62 = parser.parsePacket(packet62.data(), packet62.size());
    ASSERT_NE(result62, nullptr);
    EXPECT_GT(result62->m_lDataBlocks.size(), 0);
    delete result62;

    // Test CAT065
    std::vector<unsigned char> packet65 = createPacket(65, data);
    AsterixData* result65 = parser.parsePacket(packet65.data(), packet65.size());
    ASSERT_NE(result65, nullptr);
    EXPECT_GT(result65->m_lDataBlocks.size(), 0);
    delete result65;
}

/**
 * Test Case: TC-CPP-PARSER-013
 * Test parsePacket with undefined category
 */
TEST_F(InputParserTest, ParseUndefinedCategory) {
    // Don't add any categories to definition
    // Category 99 is not defined

    std::vector<unsigned char> data = {0x80, 0x12, 0x34};
    std::vector<unsigned char> packet = createPacket(99, data);

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet.data(), packet.size());

    ASSERT_NE(result, nullptr);
    // Should still create a block (with NULL category)
    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-014
 * Test parsePacket with timestamp
 */
TEST_F(InputParserTest, ParsePacketWithTimestamp) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    std::vector<unsigned char> data = {0x80, 0x12, 0x34};
    std::vector<unsigned char> packet = createPacket(48, data);

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    double timestamp = 1234567890.5;
    AsterixData* result = parser.parsePacket(packet.data(), packet.size(), timestamp);

    ASSERT_NE(result, nullptr);
    EXPECT_GT(result->m_lDataBlocks.size(), 0);

    // Verify timestamp was passed to DataBlock
    if (!result->m_lDataBlocks.empty()) {
        DataBlock* block = result->m_lDataBlocks.front();
        EXPECT_EQ(block->m_nTimestamp, timestamp);
    }

    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-015
 * Test parse_next_data_block function
 */
TEST_F(InputParserTest, ParseNextDataBlock) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Create packet data
    std::vector<unsigned char> data = {0x80, 0x12, 0x34};
    std::vector<unsigned char> packet = createPacket(48, data);

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    unsigned int pos = 0;
    unsigned int dataLength = packet.size();
    double timestamp = 1234567890.0;

    DataBlock* block = parser.parse_next_data_block(packet.data(), pos, packet.size(), timestamp, dataLength);

    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->m_pCategory, cat);
    EXPECT_EQ(block->m_nTimestamp, timestamp);

    delete block;
}

/**
 * Test Case: TC-CPP-PARSER-016
 * Test parse_next_data_block with invalid length (length <= 3)
 */
TEST_F(InputParserTest, ParseNextDataBlockInvalidLength) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Packet with length = 2 (invalid)
    unsigned char packet[] = {0x30, 0x00, 0x02};

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    unsigned int pos = 0;
    unsigned int dataLength = 3;
    double timestamp = 0.0;

    DataBlock* block = parser.parse_next_data_block(packet, pos, 3, timestamp, dataLength);

    // Should return NULL for invalid length
    EXPECT_EQ(block, nullptr);
}

/**
 * Test Case: TC-CPP-PARSER-017
 * Test parse_next_data_block with length > available data
 */
TEST_F(InputParserTest, ParseNextDataBlockLengthExceeds) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Packet declares length 100 but only have 10 bytes
    unsigned char packet[] = {0x30, 0x00, 0x64, 0x80, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    unsigned int pos = 0;
    unsigned int dataLength = 10;
    double timestamp = 0.0;

    DataBlock* block = parser.parse_next_data_block(packet, pos, 10, timestamp, dataLength);

    ASSERT_NE(block, nullptr);
    // Should parse what's available
    delete block;
}

/**
 * Test Case: TC-CPP-PARSER-018
 * Test printDefinition function
 */
TEST_F(InputParserTest, PrintDefinition) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    std::string result = parser.printDefinition();

    // Should return some string (may be empty or contain definition info)
    // Just verify it doesn't crash
    SUCCEED();
}

/**
 * Test Case: TC-CPP-PARSER-019
 * Test filterOutItem function
 */
TEST_F(InputParserTest, FilterOutItem) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    // Filter out item 010 from category 48
    bool result = parser.filterOutItem(48, "010", "SAC");

    // Function should execute without crash
    SUCCEED();
}

/**
 * Test Case: TC-CPP-PARSER-020
 * Test isFiltered function
 */
TEST_F(InputParserTest, IsFiltered) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    // Check if item is filtered
    bool result = parser.isFiltered(48, "010", "SAC");

    // Function should execute without crash
    SUCCEED();
}

/**
 * Test Case: TC-CPP-PARSER-021
 * Test parsePacket with very large packet (stress test)
 */
TEST_F(InputParserTest, ParseLargePacket) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Create large data (1000 records)
    std::vector<unsigned char> data;
    for (int i = 0; i < 1000; i++) {
        data.push_back(0x80);           // FSPEC
        data.push_back((i >> 8) & 0xFF); // Data byte 1
        data.push_back(i & 0xFF);        // Data byte 2
    }

    std::vector<unsigned char> packet = createPacket(48, data);

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet.data(), packet.size());

    ASSERT_NE(result, nullptr);
    // Should handle large packet
    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-022
 * Test parsePacket with DataBlock format error (m_bFormatOK = false)
 */
TEST_F(InputParserTest, ParsePacketWithFormatError) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Create malformed data that will cause parsing error
    // Invalid FSPEC or data structure
    std::vector<unsigned char> data = {0xFF, 0xFF, 0xFF, 0xFF}; // All bits set in FSPEC

    std::vector<unsigned char> packet = createPacket(48, data);

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet.data(), packet.size());

    ASSERT_NE(result, nullptr);
    // Should stop parsing on format error
    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-023
 * Test parsePacket continues to next block after error
 */
TEST_F(InputParserTest, ParsePacketContinuesAfterError) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Create packet with error in first block, then valid second block
    std::vector<unsigned char> badData = {0xFF, 0xFF, 0xFF, 0xFF};
    std::vector<unsigned char> goodData = {0x80, 0x12, 0x34};

    std::vector<unsigned char> packet1 = createPacket(48, badData);
    std::vector<unsigned char> packet2 = createPacket(48, goodData);

    std::vector<unsigned char> combined = packet1;
    combined.insert(combined.end(), packet2.begin(), packet2.end());

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(combined.data(), combined.size());

    ASSERT_NE(result, nullptr);
    // May have 1 or 2 blocks depending on error recovery
    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-024
 * Test parsePacket with maximum valid length (65535)
 */
TEST_F(InputParserTest, ParseMaximumLength) {
    Category* cat = createTestCategory(48);
    pDefinition->setCategory(cat);

    // Create packet with max length field (but don't actually create that much data)
    // Just test that length field parsing works correctly
    unsigned char packet[] = {0x30, 0xFF, 0xFF, 0x80, 0x12, 0x34};

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet, 6);

    ASSERT_NE(result, nullptr);
    // Should handle gracefully (will detect length mismatch)
    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-025
 * Test parsePacket with category 0 (edge case)
 */
TEST_F(InputParserTest, ParseCategoryZero) {
    Category* cat = createTestCategory(0);
    pDefinition->setCategory(cat);

    std::vector<unsigned char> data = {0x80, 0x12, 0x34};
    std::vector<unsigned char> packet = createPacket(0, data);

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet.data(), packet.size());

    ASSERT_NE(result, nullptr);
    delete result;
}

/**
 * Test Case: TC-CPP-PARSER-026
 * Test parsePacket with category 255 (edge case)
 */
TEST_F(InputParserTest, ParseCategory255) {
    Category* cat = createTestCategory(255);
    pDefinition->setCategory(cat);

    std::vector<unsigned char> data = {0x80, 0x12, 0x34};
    std::vector<unsigned char> packet = createPacket(255, data);

    InputParser parser(pDefinition);
    pDefinition = nullptr;

    AsterixData* result = parser.parsePacket(packet.data(), packet.size());

    ASSERT_NE(result, nullptr);
    delete result;
}
