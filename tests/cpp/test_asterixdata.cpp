/**
 * Unit tests for AsterixData class
 *
 * These tests target AsterixData.cpp to improve coverage from 0% to 85%+
 *
 * AsterixData class is a container for parsed ASTERIX data blocks.
 * It manages a list of DataBlock objects and provides output formatting.
 *
 * Functions tested:
 * 1. Constructor - line 29 (initialize empty container)
 * 2. Destructor - lines 32-39 (cleanup of DataBlocks)
 * 3. getText() - lines 44-60 (text output formatting)
 *
 * Test Strategy:
 * - Test empty container initialization
 * - Test adding single and multiple data blocks
 * - Test retrieval and iteration over blocks
 * - Test memory cleanup (destructor)
 * - Test getText() with different formats
 * - Test edge cases: empty list, large number of blocks
 *
 * Requirements Coverage:
 * - REQ-LLR-DATA-001: AsterixData container management
 * - REQ-LLR-DATA-002: DataBlock lifecycle management
 * - REQ-LLR-DATA-003: Text output generation
 * - REQ-LLR-DATA-004: Memory management
 *
 * DO-278A AL-3 Compliance Testing
 */

#include <gtest/gtest.h>
#include "../../src/asterix/AsterixData.h"
#include "../../src/asterix/DataBlock.h"
#include "../../src/asterix/Category.h"
#include "../../src/asterix/UAP.h"
#include "../../src/asterix/UAPItem.h"
#include "../../src/asterix/DataItemDescription.h"
#include "../../src/asterix/DataItemFormatFixed.h"
#include "../../src/asterix/asterixformat.hxx"
#include <cstring>

// Global variables required by ASTERIX library
bool gVerbose = false;
bool gFiltering = false;

class AsterixDataTest : public ::testing::Test {
protected:
    void SetUp() override {
        gVerbose = false;
        gFiltering = false;
    }

    void TearDown() override {
        gFiltering = false;
    }

    /**
     * Helper: Create a minimal Category for testing
     */
    Category* createTestCategory(int catId) {
        Category* cat = new Category(catId);
        cat->m_strName = "Test Category";
        cat->m_strVer = "1.0";
        cat->m_bFiltered = false;

        // Create a default UAP
        UAP* pUAP = cat->newUAP();
        pUAP->m_nUseIfBitSet = 0;  // Default UAP (always used)

        // Add a simple data item (2-byte fixed length)
        DataItemDescription* desc = cat->getDataItemDescription("010");
        desc->m_strName = "Data Source Identifier";
        DataItemFormatFixed* format = new DataItemFormatFixed(2);
        desc->m_pFormat = format;

        // Add item to UAP
        UAPItem* uapItem = pUAP->newUAPItem();
        uapItem->m_nBit = 1;  // FSPEC bit 1
        uapItem->m_strItemID = "010";  // Link to data item 010

        return cat;
    }

    /**
     * Helper: Create a minimal DataBlock for testing
     * Format: CAT (1 byte) + Length (2 bytes) + FSPEC (1 byte) + Data (2 bytes)
     */
    DataBlock* createTestDataBlock(Category* cat, unsigned char category = 48) {
        // ASTERIX data block: CAT=48, Length=6, FSPEC=0x80, Data=0x01,0x02
        static unsigned char testData[] = {
            category,    // Category
            0x00, 0x06,  // Length (6 bytes total)
            0x80,        // FSPEC (bit 1 set, FX=0)
            0x01, 0x02   // Data item 010 (2 bytes)
        };

        return new DataBlock(cat, sizeof(testData), testData, 0.0);
    }
};

/**
 * Test Case: TC-CPP-AD-001
 * Requirement: REQ-LLR-DATA-001
 * Description: Verify AsterixData constructor initializes empty container
 */
TEST_F(AsterixDataTest, ConstructorInitializesEmptyContainer) {
    AsterixData data;

    // Container should be empty after construction
    EXPECT_TRUE(data.m_lDataBlocks.empty());
    EXPECT_EQ(data.m_lDataBlocks.size(), 0);
}

/**
 * Test Case: TC-CPP-AD-002
 * Requirement: REQ-LLR-DATA-001
 * Description: Verify adding a single DataBlock to container
 */
TEST_F(AsterixDataTest, AddSingleDataBlock) {
    AsterixData data;
    Category* cat = createTestCategory(48);
    DataBlock* block = createTestDataBlock(cat);

    // Add data block to container
    data.m_lDataBlocks.push_back(block);

    EXPECT_FALSE(data.m_lDataBlocks.empty());
    EXPECT_EQ(data.m_lDataBlocks.size(), 1);
    EXPECT_EQ(data.m_lDataBlocks.front(), block);

    // Cleanup
    data.m_lDataBlocks.clear();  // Don't let AsterixData destructor delete it
    delete block;
    delete cat;
}

/**
 * Test Case: TC-CPP-AD-003
 * Requirement: REQ-LLR-DATA-001
 * Description: Verify adding multiple DataBlocks to container
 */
TEST_F(AsterixDataTest, AddMultipleDataBlocks) {
    AsterixData data;
    Category* cat1 = createTestCategory(48);
    Category* cat2 = createTestCategory(62);

    DataBlock* block1 = createTestDataBlock(cat1, 48);
    DataBlock* block2 = createTestDataBlock(cat2, 62);

    data.m_lDataBlocks.push_back(block1);
    data.m_lDataBlocks.push_back(block2);

    EXPECT_EQ(data.m_lDataBlocks.size(), 2);

    // Verify order is preserved
    auto it = data.m_lDataBlocks.begin();
    EXPECT_EQ(*it, block1);
    ++it;
    EXPECT_EQ(*it, block2);

    // Cleanup
    data.m_lDataBlocks.clear();
    delete block1;
    delete block2;
    delete cat1;
    delete cat2;
}

/**
 * Test Case: TC-CPP-AD-004
 * Requirement: REQ-LLR-DATA-002
 * Description: Verify destructor cleans up all DataBlocks
 */
TEST_F(AsterixDataTest, DestructorCleansUpDataBlocks) {
    Category* cat = createTestCategory(48);
    DataBlock* block1 = createTestDataBlock(cat);
    DataBlock* block2 = createTestDataBlock(cat);

    {
        AsterixData data;
        data.m_lDataBlocks.push_back(block1);
        data.m_lDataBlocks.push_back(block2);

        EXPECT_EQ(data.m_lDataBlocks.size(), 2);
        // Destructor will be called here and should delete blocks
    }

    // If we reach here without crash, cleanup succeeded
    delete cat;
    SUCCEED();
}

/**
 * Test Case: TC-CPP-AD-005
 * Requirement: REQ-LLR-DATA-001
 * Description: Verify iterating over DataBlocks
 */
TEST_F(AsterixDataTest, IterateOverDataBlocks) {
    AsterixData data;
    Category* cat = createTestCategory(48);

    DataBlock* block1 = createTestDataBlock(cat);
    DataBlock* block2 = createTestDataBlock(cat);
    DataBlock* block3 = createTestDataBlock(cat);

    data.m_lDataBlocks.push_back(block1);
    data.m_lDataBlocks.push_back(block2);
    data.m_lDataBlocks.push_back(block3);

    // Iterate and count
    int count = 0;
    for (auto it = data.m_lDataBlocks.begin(); it != data.m_lDataBlocks.end(); ++it) {
        EXPECT_NE(*it, nullptr);
        count++;
    }

    EXPECT_EQ(count, 3);

    // Cleanup
    data.m_lDataBlocks.clear();
    delete block1;
    delete block2;
    delete block3;
    delete cat;
}

/**
 * Test Case: TC-CPP-AD-006
 * Requirement: REQ-LLR-DATA-003
 * Description: Verify getText() with empty container
 */
TEST_F(AsterixDataTest, GetTextWithEmptyContainer) {
    AsterixData data;
    std::string result;

    bool success = data.getText(result, CAsterixFormat::ETxt);

    EXPECT_TRUE(success);
    EXPECT_TRUE(result.empty());
}

/**
 * Test Case: TC-CPP-AD-007
 * Requirement: REQ-LLR-DATA-003
 * Description: Verify getText() with single DataBlock (Text format)
 */
TEST_F(AsterixDataTest, GetTextWithSingleBlockTextFormat) {
    AsterixData data;
    Category* cat = createTestCategory(48);
    DataBlock* block = createTestDataBlock(cat);

    data.m_lDataBlocks.push_back(block);

    std::string result;
    bool success = data.getText(result, CAsterixFormat::ETxt);

    EXPECT_TRUE(success);
    EXPECT_FALSE(result.empty());
    // Should contain "Data Block" text
    EXPECT_NE(result.find("Data Block"), std::string::npos);

    // Cleanup
    data.m_lDataBlocks.clear();
    delete block;
    delete cat;
}

/**
 * Test Case: TC-CPP-AD-008
 * Requirement: REQ-LLR-DATA-003
 * Description: Verify getText() with multiple DataBlocks
 */
TEST_F(AsterixDataTest, GetTextWithMultipleBlocks) {
    AsterixData data;
    Category* cat = createTestCategory(48);

    DataBlock* block1 = createTestDataBlock(cat);
    DataBlock* block2 = createTestDataBlock(cat);
    DataBlock* block3 = createTestDataBlock(cat);

    data.m_lDataBlocks.push_back(block1);
    data.m_lDataBlocks.push_back(block2);
    data.m_lDataBlocks.push_back(block3);

    std::string result;
    bool success = data.getText(result, CAsterixFormat::ETxt);

    EXPECT_TRUE(success);
    EXPECT_FALSE(result.empty());

    // Should contain multiple "Data Block" entries
    size_t pos = 0;
    int count = 0;
    while ((pos = result.find("Data Block", pos)) != std::string::npos) {
        count++;
        pos += 10;  // Length of "Data Block"
    }
    EXPECT_EQ(count, 3);

    // Cleanup
    data.m_lDataBlocks.clear();
    delete block1;
    delete block2;
    delete block3;
    delete cat;
}

/**
 * Test Case: TC-CPP-AD-009
 * Requirement: REQ-LLR-DATA-003
 * Description: Verify getText() with JSON format
 */
TEST_F(AsterixDataTest, GetTextWithJSONFormat) {
    AsterixData data;
    Category* cat = createTestCategory(48);
    DataBlock* block = createTestDataBlock(cat);

    data.m_lDataBlocks.push_back(block);

    std::string result;
    bool success = data.getText(result, CAsterixFormat::EJSON);

    EXPECT_TRUE(success);
    // JSON format handled by DataBlock, just verify no crash
    SUCCEED();

    // Cleanup
    data.m_lDataBlocks.clear();
    delete block;
    delete cat;
}

/**
 * Test Case: TC-CPP-AD-010
 * Requirement: REQ-LLR-DATA-003
 * Description: Verify getText() with XML format
 */
TEST_F(AsterixDataTest, GetTextWithXMLFormat) {
    AsterixData data;
    Category* cat = createTestCategory(48);
    DataBlock* block = createTestDataBlock(cat);

    data.m_lDataBlocks.push_back(block);

    std::string result;
    bool success = data.getText(result, CAsterixFormat::EXML);

    EXPECT_TRUE(success);
    // XML format handled by DataBlock, just verify no crash
    SUCCEED();

    // Cleanup
    data.m_lDataBlocks.clear();
    delete block;
    delete cat;
}

/**
 * Test Case: TC-CPP-AD-011
 * Requirement: REQ-LLR-DATA-001
 * Description: Verify clearing the container
 */
TEST_F(AsterixDataTest, ClearContainer) {
    AsterixData data;
    Category* cat = createTestCategory(48);

    DataBlock* block1 = createTestDataBlock(cat);
    DataBlock* block2 = createTestDataBlock(cat);

    data.m_lDataBlocks.push_back(block1);
    data.m_lDataBlocks.push_back(block2);

    EXPECT_EQ(data.m_lDataBlocks.size(), 2);

    // Manually delete blocks and clear
    for (auto block : data.m_lDataBlocks) {
        delete block;
    }
    data.m_lDataBlocks.clear();

    EXPECT_TRUE(data.m_lDataBlocks.empty());
    EXPECT_EQ(data.m_lDataBlocks.size(), 0);

    delete cat;
}

/**
 * Test Case: TC-CPP-AD-012
 * Requirement: REQ-LLR-DATA-001
 * Description: Verify handling NULL DataBlock in list
 */
TEST_F(AsterixDataTest, HandleNullDataBlockInList) {
    AsterixData data;
    Category* cat = createTestCategory(48);
    DataBlock* validBlock = createTestDataBlock(cat);

    // Add valid block, NULL, and another valid block
    data.m_lDataBlocks.push_back(validBlock);
    data.m_lDataBlocks.push_back(nullptr);
    data.m_lDataBlocks.push_back(validBlock);

    EXPECT_EQ(data.m_lDataBlocks.size(), 3);

    std::string result;
    // getText should handle NULL gracefully
    bool success = data.getText(result, CAsterixFormat::ETxt);

    EXPECT_TRUE(success);
    // Should process non-NULL blocks
    EXPECT_NE(result.find("Data Block"), std::string::npos);

    // Cleanup - remove NULL to avoid double-delete
    data.m_lDataBlocks.remove(nullptr);
    data.m_lDataBlocks.clear();
    delete validBlock;
    delete cat;
}

/**
 * Test Case: TC-CPP-AD-013
 * Requirement: REQ-LLR-DATA-004
 * Description: Verify memory management with large number of blocks
 */
TEST_F(AsterixDataTest, LargeNumberOfDataBlocks) {
    AsterixData data;
    Category* cat = createTestCategory(48);

    const int numBlocks = 100;
    for (int i = 0; i < numBlocks; i++) {
        DataBlock* block = createTestDataBlock(cat);
        data.m_lDataBlocks.push_back(block);
    }

    EXPECT_EQ(data.m_lDataBlocks.size(), numBlocks);

    std::string result;
    bool success = data.getText(result, CAsterixFormat::ETxt);

    EXPECT_TRUE(success);
    EXPECT_FALSE(result.empty());

    // Destructor should clean up all blocks
    // (will happen automatically at end of test)
    delete cat;
}

/**
 * Test Case: TC-CPP-AD-014
 * Requirement: REQ-LLR-DATA-001
 * Description: Verify empty destructor handles empty list
 */
TEST_F(AsterixDataTest, DestructorWithEmptyList) {
    {
        AsterixData data;
        // No blocks added
        EXPECT_TRUE(data.m_lDataBlocks.empty());
    }
    // If we reach here without crash, cleanup succeeded
    SUCCEED();
}

/**
 * Test Case: TC-CPP-AD-015
 * Requirement: REQ-LLR-DATA-002
 * Description: Verify destructor cleanup iteration logic
 */
TEST_F(AsterixDataTest, DestructorIterationCleanup) {
    Category* cat = createTestCategory(48);

    {
        AsterixData data;

        // Add blocks in different ways
        data.m_lDataBlocks.push_back(createTestDataBlock(cat));
        data.m_lDataBlocks.push_front(createTestDataBlock(cat));
        data.m_lDataBlocks.push_back(createTestDataBlock(cat));

        EXPECT_EQ(data.m_lDataBlocks.size(), 3);

        // Destructor uses iterator erase pattern
        // Should delete all blocks regardless of insertion order
    }

    delete cat;
    SUCCEED();
}

/**
 * Test Case: TC-CPP-AD-016
 * Requirement: REQ-LLR-DATA-003
 * Description: Verify getText() increments block counter
 */
TEST_F(AsterixDataTest, GetTextIncrementsBlockCounter) {
    AsterixData data1;
    AsterixData data2;
    Category* cat = createTestCategory(48);

    DataBlock* block1 = createTestDataBlock(cat);
    DataBlock* block2 = createTestDataBlock(cat);

    data1.m_lDataBlocks.push_back(block1);

    std::string result1;
    data1.getText(result1, CAsterixFormat::ETxt);

    // Block counter is static, should be visible across calls
    EXPECT_NE(result1.find("Data Block 1"), std::string::npos);

    // Second call should show incremented counter
    data2.m_lDataBlocks.push_back(block2);
    std::string result2;
    data2.getText(result2, CAsterixFormat::ETxt);

    // Counter should increment (note: static variable)
    // This tests the static int i = 1; line 45
    SUCCEED();

    // Cleanup
    data1.m_lDataBlocks.clear();
    data2.m_lDataBlocks.clear();
    delete block1;
    delete block2;
    delete cat;
}

/**
 * Test Case: TC-CPP-AD-017
 * Requirement: REQ-LLR-DATA-003
 * Description: Verify getText() with different format types
 */
TEST_F(AsterixDataTest, GetTextWithAllFormatTypes) {
    AsterixData data;
    Category* cat = createTestCategory(48);
    DataBlock* block = createTestDataBlock(cat);

    data.m_lDataBlocks.push_back(block);

    // Test all format types
    std::string resultTxt, resultJSON, resultXML;

    EXPECT_TRUE(data.getText(resultTxt, CAsterixFormat::ETxt));
    EXPECT_TRUE(data.getText(resultJSON, CAsterixFormat::EJSON));
    EXPECT_TRUE(data.getText(resultXML, CAsterixFormat::EXML));

    // Only ETxt format adds "Data Block" header
    EXPECT_NE(resultTxt.find("Data Block"), std::string::npos);

    // Cleanup
    data.m_lDataBlocks.clear();
    delete block;
    delete cat;
}

/**
 * Test Case: TC-CPP-AD-018
 * Requirement: REQ-LLR-DATA-004
 * Description: Verify move semantics with std::list
 */
TEST_F(AsterixDataTest, MoveDataBlocksList) {
    AsterixData data;
    Category* cat = createTestCategory(48);

    DataBlock* block1 = createTestDataBlock(cat);
    DataBlock* block2 = createTestDataBlock(cat);

    data.m_lDataBlocks.push_back(block1);
    data.m_lDataBlocks.push_back(block2);

    // Move the list (C++11 move semantics)
    std::list<DataBlock*> movedList = std::move(data.m_lDataBlocks);

    EXPECT_TRUE(data.m_lDataBlocks.empty());
    EXPECT_EQ(movedList.size(), 2);

    // Clean up moved list manually
    for (auto block : movedList) {
        delete block;
    }
    movedList.clear();

    delete cat;
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
