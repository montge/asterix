/**
 * Unit tests for DataBlock class
 *
 * These tests target DataBlock.cpp to improve coverage from 63.0% to 80%+
 *
 * Functions tested:
 * 1. Constructor - lines 31-66 (parsing records from data block)
 * 2. Destructor - lines 68-75 (cleanup of DataRecords)
 * 3. getText() - lines 77-112 (all output formats)
 *
 * ASTERIX DataBlock Format:
 * - Category (1 byte) - ASTERIX category number
 * - Length (2 bytes) - Total length including header (3 bytes)
 * - Data Records (variable) - One or more DataRecords
 *
 * DataBlock responsibilities:
 * - Parse raw data into multiple DataRecords
 * - Manage lifetime of DataRecords
 * - Format output in various formats (Text, JSON, XML)
 * - Handle filtering based on category
 * - Track timestamp from packet capture
 *
 * Test Strategy:
 * - Test constructor with valid/invalid data
 * - Test destructor properly cleans up records
 * - Test getText() with all output formats
 * - Test filtering behavior
 * - Test edge cases (no records, many records, unparsed data)
 *
 * Requirements Coverage:
 * - REQ-LLR-BLOCK-001: DataBlock parsing
 * - REQ-LLR-BLOCK-002: DataRecord management
 * - REQ-LLR-BLOCK-003: Text output generation
 * - REQ-LLR-BLOCK-004: Filtering support
 * - REQ-LLR-BLOCK-005: Error handling
 */

#include <gtest/gtest.h>
#include "../../src/asterix/DataBlock.h"
#include "../../src/asterix/Category.h"
#include "../../src/asterix/UAP.h"
#include "../../src/asterix/UAPItem.h"
#include "../../src/asterix/DataItemDescription.h"
#include "../../src/asterix/DataItemFormat.h"
#include "../../src/asterix/DataItemFormatFixed.h"
#include "../../src/asterix/DataItemBits.h"
#include "../../src/asterix/Tracer.h"
#include "../../src/asterix/asterixformat.hxx"
#include <cstring>

// Global variables required by ASTERIX library
bool gVerbose = false;
bool gFiltering = false;

class DataBlockTest : public ::testing::Test {
protected:
    Category* pCategory;
    UAP* pUAP;

    void SetUp() override {
        gVerbose = false;
        gFiltering = false;
        pCategory = nullptr;
        pUAP = nullptr;
    }

    void TearDown() override {
        if (pCategory) {
            delete pCategory;
            pCategory = nullptr;
        }
        gFiltering = false;
    }

    /**
     * Helper: Create a minimal Category with UAP
     */
    Category* createTestCategory(int catId) {
        Category* cat = new Category(catId);
        cat->m_strName = "Test Category";
        cat->m_strVer = "1.0";
        cat->m_bFiltered = false;

        // Create a default UAP
        pUAP = cat->newUAP();
        pUAP->m_nUseIfBitSet = 0;  // Default UAP (always used)

        return cat;
    }

    /**
     * Helper: Add a DataItem to the category
     */
    DataItemDescription* addDataItem(Category* cat, const char* itemId, int lengthBytes) {
        DataItemDescription* desc = cat->getDataItemDescription(itemId);
        desc->m_strName = std::string("Test Item ") + itemId;

        // Create a Fixed format
        DataItemFormatFixed* format = new DataItemFormatFixed(lengthBytes);
        format->m_nLength = lengthBytes;

        // Add DataItemBits for the bytes
        for (int i = 0; i < lengthBytes; i++) {
            DataItemBits* bits = new DataItemBits(8);
            bits->m_strShortName = "VALUE";
            bits->m_strName = "Value";
            bits->m_nFrom = i * 8;
            bits->m_nTo = (i * 8) + 7;
            format->m_lSubItems.push_back(bits);
        }

        desc->m_pFormat = format;
        return desc;
    }

    /**
     * Helper: Add UAP item mapping
     */
    void addUAPItem(UAP* uap, int frn, const char* itemId) {
        UAPItem* uapItem = uap->newUAPItem();
        uapItem->m_nFRN = frn;
        uapItem->m_strItemID = itemId;
    }
};

/**
 * Test Case: TC-CPP-BLOCK-001
 * Test constructor with valid single record
 */
TEST_F(DataBlockTest, ConstructorSingleRecord) {
    // Setup: Category with one data item
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    // Create data block data: FSPEC (0x80) + 2-byte item
    unsigned char data[] = {0x80, 0x12, 0x34};

    DataBlock block(pCategory, sizeof(data), data, 1234567890.0);

    // Verify
    EXPECT_EQ(block.m_pCategory, pCategory);
    EXPECT_EQ(block.m_nLength, 3);
    EXPECT_EQ(block.m_nTimestamp, 1234567890.0);
    EXPECT_TRUE(block.m_bFormatOK);
    EXPECT_EQ(block.m_lDataRecords.size(), 1);
}

/**
 * Test Case: TC-CPP-BLOCK-002
 * Test constructor with multiple records
 */
TEST_F(DataBlockTest, ConstructorMultipleRecords) {
    // Setup: Category with one data item
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    // Create data block with 3 records
    unsigned char data[] = {
        0x80, 0x12, 0x34,  // Record 1
        0x80, 0x56, 0x78,  // Record 2
        0x80, 0xAB, 0xCD   // Record 3
    };

    DataBlock block(pCategory, sizeof(data), data, 0.0);

    // Verify
    EXPECT_TRUE(block.m_bFormatOK);
    EXPECT_EQ(block.m_lDataRecords.size(), 3);
    EXPECT_EQ(block.m_nLength, 9);
}

/**
 * Test Case: TC-CPP-BLOCK-003
 * Test constructor with timestamp
 */
TEST_F(DataBlockTest, ConstructorWithTimestamp) {
    pCategory = createTestCategory(62);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {0x80, 0x11, 0x22};
    double timestamp = 1609459200.0; // 2021-01-01 00:00:00 GMT

    DataBlock block(pCategory, sizeof(data), data, timestamp);

    EXPECT_EQ(block.m_nTimestamp, timestamp);
    EXPECT_TRUE(block.m_bFormatOK);
}

/**
 * Test Case: TC-CPP-BLOCK-004
 * Test constructor with filtered category (gFiltering = true, category not filtered)
 */
TEST_F(DataBlockTest, ConstructorWithFilteredCategory) {
    pCategory = createTestCategory(48);
    pCategory->m_bFiltered = false;  // Category not in filter
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    gFiltering = true;  // Enable filtering
    unsigned char data[] = {0x80, 0x12, 0x34};

    DataBlock block(pCategory, sizeof(data), data, 0.0);

    // When filtering is enabled and category is not filtered,
    // constructor returns early without parsing records
    EXPECT_TRUE(block.m_bFormatOK);
    EXPECT_EQ(block.m_lDataRecords.size(), 0);

    gFiltering = false;
}

/**
 * Test Case: TC-CPP-BLOCK-005
 * Test constructor with filtered category (gFiltering = true, category filtered)
 */
TEST_F(DataBlockTest, ConstructorWithCategoryInFilter) {
    pCategory = createTestCategory(48);
    pCategory->m_bFiltered = true;  // Category IS in filter
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    gFiltering = true;  // Enable filtering
    unsigned char data[] = {0x80, 0x12, 0x34};

    DataBlock block(pCategory, sizeof(data), data, 0.0);

    // When category is in filter, records should be parsed
    EXPECT_TRUE(block.m_bFormatOK);
    EXPECT_EQ(block.m_lDataRecords.size(), 1);

    gFiltering = false;
}

/**
 * Test Case: TC-CPP-BLOCK-006
 * Test destructor properly cleans up DataRecords
 */
TEST_F(DataBlockTest, DestructorCleansUpRecords) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {
        0x80, 0x12, 0x34,
        0x80, 0x56, 0x78
    };

    // Create block on heap
    DataBlock* block = new DataBlock(pCategory, sizeof(data), data, 0.0);
    EXPECT_EQ(block->m_lDataRecords.size(), 2);

    // Delete should clean up all records
    delete block;

    // If we get here without crash, destructor worked correctly
    SUCCEED();
}

/**
 * Test Case: TC-CPP-BLOCK-007
 * Test getText with ETxt format
 */
TEST_F(DataBlockTest, GetTextFormatText) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {0x80, 0x12, 0x34};
    DataBlock block(pCategory, sizeof(data), data, 1234567890.5);

    std::string result;
    bool success = block.getText(result, CAsterixFormat::ETxt);

    EXPECT_TRUE(success);
    EXPECT_GT(result.length(), 0);

    // Verify output contains expected fields
    EXPECT_NE(result.find("Category: 48"), std::string::npos);
    EXPECT_NE(result.find("Len: 3"), std::string::npos);
    EXPECT_NE(result.find("Timestamp: 1234567890.5"), std::string::npos);
    EXPECT_NE(result.find("HexData:"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BLOCK-008
 * Test getText with EOut format
 */
TEST_F(DataBlockTest, GetTextFormatOut) {
    pCategory = createTestCategory(62);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {0x80, 0xFF, 0xEE};
    DataBlock block(pCategory, sizeof(data), data, 0.0);

    std::string result;
    bool success = block.getText(result, CAsterixFormat::EOut);

    EXPECT_TRUE(success);
    EXPECT_GT(result.length(), 0);

    // EOut format should include category prefix
    EXPECT_NE(result.find("Asterix.CAT062"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BLOCK-009
 * Test getText with EJSON format
 */
TEST_F(DataBlockTest, GetTextFormatJSON) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {0x80, 0x12, 0x34};
    DataBlock block(pCategory, sizeof(data), data, 0.0);

    std::string result;
    bool success = block.getText(result, CAsterixFormat::EJSON);

    EXPECT_TRUE(success);
    // JSON format is handled by DataRecord, just verify call succeeds
}

/**
 * Test Case: TC-CPP-BLOCK-010
 * Test getText with EJSONH format
 */
TEST_F(DataBlockTest, GetTextFormatJSONH) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {0x80, 0x12, 0x34};
    DataBlock block(pCategory, sizeof(data), data, 0.0);

    std::string result;
    bool success = block.getText(result, CAsterixFormat::EJSONH);

    EXPECT_TRUE(success);
}

/**
 * Test Case: TC-CPP-BLOCK-011
 * Test getText with EXML format
 */
TEST_F(DataBlockTest, GetTextFormatXML) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {0x80, 0x12, 0x34};
    DataBlock block(pCategory, sizeof(data), data, 0.0);

    std::string result;
    bool success = block.getText(result, CAsterixFormat::EXML);

    EXPECT_TRUE(success);
}

/**
 * Test Case: TC-CPP-BLOCK-012
 * Test getText with EXMLH format
 */
TEST_F(DataBlockTest, GetTextFormatXMLH) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {0x80, 0x12, 0x34};
    DataBlock block(pCategory, sizeof(data), data, 0.0);

    std::string result;
    bool success = block.getText(result, CAsterixFormat::EXMLH);

    EXPECT_TRUE(success);
}

/**
 * Test Case: TC-CPP-BLOCK-013
 * Test getText with filtered category (should return false)
 */
TEST_F(DataBlockTest, GetTextWithFilteredCategory) {
    pCategory = createTestCategory(48);
    pCategory->m_bFiltered = false;
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {0x80, 0x12, 0x34};
    DataBlock block(pCategory, sizeof(data), data, 0.0);

    gFiltering = true;
    std::string result;
    bool success = block.getText(result, CAsterixFormat::ETxt);

    // When filtering is enabled and category not in filter, getText returns false
    EXPECT_FALSE(success);

    gFiltering = false;
}

/**
 * Test Case: TC-CPP-BLOCK-014
 * Test getText when m_bFormatOK is false
 */
TEST_F(DataBlockTest, GetTextWithFormatNotOK) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {0x80, 0x12, 0x34};
    DataBlock block(pCategory, sizeof(data), data, 0.0);

    // Force m_bFormatOK to false
    block.m_bFormatOK = false;

    std::string result;
    bool success = block.getText(result, CAsterixFormat::ETxt);

    // getText should still return true, but with error message
    EXPECT_TRUE(success);
}

/**
 * Test Case: TC-CPP-BLOCK-015
 * Test getText with empty data records
 */
TEST_F(DataBlockTest, GetTextWithEmptyRecords) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    // Create block with minimal valid data
    unsigned char data[] = {0x80, 0x00, 0x00};
    DataBlock block(pCategory, sizeof(data), data, 0.0);

    std::string result;
    bool success = block.getText(result, CAsterixFormat::ETxt);

    EXPECT_TRUE(success);
    // Should contain header info even if no valid records
    EXPECT_NE(result.find("Category: 48"), std::string::npos);
}

/**
 * Test Case: TC-CPP-BLOCK-016
 * Test with zero-length data (edge case)
 */
TEST_F(DataBlockTest, ConstructorZeroLengthData) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {0x00};
    DataBlock block(pCategory, 0, data, 0.0);

    // Should handle gracefully
    EXPECT_TRUE(block.m_bFormatOK);
    EXPECT_EQ(block.m_lDataRecords.size(), 0);
    EXPECT_EQ(block.m_nLength, 0);
}

/**
 * Test Case: TC-CPP-BLOCK-017
 * Test with unparsed data remaining (error condition)
 */
TEST_F(DataBlockTest, ConstructorWithUnparsedData) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    // Create malformed data that will cause parsing to stop early
    // FSPEC indicates 2-byte item, but only provide 1 byte
    unsigned char data[] = {0x80, 0x12};  // Missing 1 byte

    DataBlock block(pCategory, sizeof(data), data, 0.0);

    // Constructor should handle error and adjust length
    EXPECT_TRUE(block.m_bFormatOK);
}

/**
 * Test Case: TC-CPP-BLOCK-018
 * Test with many records (stress test)
 */
TEST_F(DataBlockTest, ConstructorManyRecords) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 1);  // 1-byte item for compact data
    addUAPItem(pUAP, 1, "010");

    // Create 100 records
    const int NUM_RECORDS = 100;
    unsigned char data[NUM_RECORDS * 2];  // FSPEC + 1 byte each
    for (int i = 0; i < NUM_RECORDS; i++) {
        data[i * 2] = 0x80;      // FSPEC
        data[i * 2 + 1] = i & 0xFF;  // Data byte
    }

    DataBlock block(pCategory, sizeof(data), data, 0.0);

    EXPECT_TRUE(block.m_bFormatOK);
    EXPECT_EQ(block.m_lDataRecords.size(), NUM_RECORDS);
}

/**
 * Test Case: TC-CPP-BLOCK-019
 * Test category pointer is preserved
 */
TEST_F(DataBlockTest, CategoryPointerPreserved) {
    pCategory = createTestCategory(65);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {0x80, 0x12, 0x34};
    DataBlock block(pCategory, sizeof(data), data, 0.0);

    // Verify category pointer is correctly stored
    EXPECT_EQ(block.m_pCategory, pCategory);
    EXPECT_EQ(block.m_pCategory->m_id, 65);
}

/**
 * Test Case: TC-CPP-BLOCK-020
 * Test getText with multiple records generates output for all
 */
TEST_F(DataBlockTest, GetTextWithMultipleRecords) {
    pCategory = createTestCategory(48);
    addDataItem(pCategory, "010", 2);
    addUAPItem(pUAP, 1, "010");

    unsigned char data[] = {
        0x80, 0x11, 0x22,
        0x80, 0x33, 0x44,
        0x80, 0x55, 0x66
    };
    DataBlock block(pCategory, sizeof(data), data, 0.0);

    EXPECT_EQ(block.m_lDataRecords.size(), 3);

    std::string result;
    bool success = block.getText(result, CAsterixFormat::ETxt);

    EXPECT_TRUE(success);
    EXPECT_GT(result.length(), 0);
    // Should contain category header
    EXPECT_NE(result.find("Category: 48"), std::string::npos);
}
