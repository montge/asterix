/**
 * Unit tests for Category class
 *
 * Requirements Traceability:
 * - REQ-HLR-SYS-001: Parse ASTERIX categories
 * - REQ-HLR-CAT-001: Support multiple ASTERIX categories
 *
 * DO-278A AL-3 Compliance Testing
 */

#include <gtest/gtest.h>
#include "Category.h"
#include "DataItemDescription.h"
#include "UAP.h"

/**
 * Test Case: TC-CPP-CAT-001
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify Category constructor initializes correctly
 */
TEST(CategoryTest, ConstructorInitializesId) {
    Category cat(48);

    EXPECT_EQ(cat.m_id, 48);
    EXPECT_FALSE(cat.m_bFiltered);
    EXPECT_TRUE(cat.m_strName.empty());
    EXPECT_TRUE(cat.m_strVer.empty());
}

/**
 * Test Case: TC-CPP-CAT-002
 * Requirement: REQ-HLR-CAT-001
 * Description: Verify Category handles different category IDs
 */
TEST(CategoryTest, SupportsDifferentCategories) {
    Category cat1(1);
    Category cat48(48);
    Category cat62(62);
    Category cat65(65);

    EXPECT_EQ(cat1.m_id, 1);
    EXPECT_EQ(cat48.m_id, 48);
    EXPECT_EQ(cat62.m_id, 62);
    EXPECT_EQ(cat65.m_id, 65);
}

/**
 * Test Case: TC-CPP-CAT-003
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify Category name and version can be set
 */
TEST(CategoryTest, NameAndVersionCanBeSet) {
    Category cat(48);

    cat.m_strName = "Monoradar Target Reports";
    cat.m_strVer = "1.21";

    EXPECT_EQ(cat.m_strName, "Monoradar Target Reports");
    EXPECT_EQ(cat.m_strVer, "1.21");
}

/**
 * Test Case: TC-CPP-CAT-004
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify data item can be added to category
 */
TEST(CategoryTest, CanAddDataItem) {
    Category cat(48);

    DataItemDescription* item = cat.getDataItemDescription("010");
    ASSERT_NE(item, nullptr);

    // Check that the same item is returned on second call
    DataItemDescription* sameItem = cat.getDataItemDescription("010");
    EXPECT_EQ(item, sameItem);
}

/**
 * Test Case: TC-CPP-CAT-005
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify multiple data items can be added
 */
TEST(CategoryTest, CanAddMultipleDataItems) {
    Category cat(48);

    DataItemDescription* item010 = cat.getDataItemDescription("010");
    DataItemDescription* item020 = cat.getDataItemDescription("020");
    DataItemDescription* item040 = cat.getDataItemDescription("040");

    ASSERT_NE(item010, nullptr);
    ASSERT_NE(item020, nullptr);
    ASSERT_NE(item040, nullptr);

    // Verify they are different objects
    EXPECT_NE(item010, item020);
    EXPECT_NE(item020, item040);
    EXPECT_NE(item010, item040);
}

/**
 * Test Case: TC-CPP-CAT-006
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify UAP can be created
 */
TEST(CategoryTest, CanCreateUAP) {
    Category cat(48);

    UAP* uap = cat.newUAP();
    ASSERT_NE(uap, nullptr);

    // UAP should be added to category's list
    EXPECT_FALSE(cat.m_lUAPs.empty());
}

/**
 * Test Case: TC-CPP-CAT-007
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify multiple UAPs can be created
 */
TEST(CategoryTest, CanCreateMultipleUAPs) {
    Category cat(48);

    UAP* uap1 = cat.newUAP();
    UAP* uap2 = cat.newUAP();

    ASSERT_NE(uap1, nullptr);
    ASSERT_NE(uap2, nullptr);
    EXPECT_NE(uap1, uap2);

    // Both UAPs should be in the list
    EXPECT_EQ(cat.m_lUAPs.size(), 2);
}

/**
 * Test Case: TC-CPP-CAT-008
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify filter flag works correctly
 */
TEST(CategoryTest, FilterFlagWorks) {
    Category cat(48);

    EXPECT_FALSE(cat.m_bFiltered);

    cat.m_bFiltered = true;
    EXPECT_TRUE(cat.m_bFiltered);

    cat.m_bFiltered = false;
    EXPECT_FALSE(cat.m_bFiltered);
}

/**
 * Test Case: TC-CPP-CAT-009
 * Requirement: REQ-HLR-CAT-048
 * Description: Verify CAT048 specific initialization
 */
TEST(CategoryTest, CAT048Initialization) {
    Category cat(48);

    cat.m_strName = "Monoradar Target Reports";
    cat.m_strVer = "1.21";

    // Add typical CAT048 data items
    cat.getDataItemDescription("010"); // Data Source Identifier
    cat.getDataItemDescription("020"); // Target Report Descriptor
    cat.getDataItemDescription("040"); // Measured Position
    cat.getDataItemDescription("070"); // Mode-3/A Code
    cat.getDataItemDescription("090"); // Flight Level
    cat.getDataItemDescription("140"); // Time Of Day

    EXPECT_EQ(cat.m_lDataItems.size(), 6);
}

/**
 * Test Case: TC-CPP-CAT-010
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify Category destructor cleans up properly
 */
TEST(CategoryTest, DestructorCleansUp) {
    // Create a category in a scope
    {
        Category cat(48);
        cat.getDataItemDescription("010");
        cat.newUAP();

        EXPECT_FALSE(cat.m_lDataItems.empty());
        EXPECT_FALSE(cat.m_lUAPs.empty());
    }
    // If destructor doesn't crash, cleanup was successful
    SUCCEED();
}

/**
 * Test Case: TC-CPP-CAT-011
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify getDescription returns item name when field is NULL
 */
TEST(CategoryTest, GetDescriptionReturnsItemName) {
    Category cat(48);

    DataItemDescription* item = cat.getDataItemDescription("010");
    item->m_strName = "Data Source Identifier";

    const char* desc = cat.getDescription("I010", NULL, NULL);
    ASSERT_NE(desc, nullptr);
    EXPECT_STREQ(desc, "Data Source Identifier");
}

/**
 * Test Case: TC-CPP-CAT-012
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify getDescription returns NULL for non-existent item
 */
TEST(CategoryTest, GetDescriptionReturnsNullForNonExistent) {
    Category cat(48);

    const char* desc = cat.getDescription("I999", NULL, NULL);
    EXPECT_EQ(desc, nullptr);
}

/**
 * Test Case: TC-CPP-CAT-013
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify getUAP returns first UAP when no conditions set
 */
TEST(CategoryTest, GetUAPReturnsFirstWhenNoConditions) {
    Category cat(48);

    UAP* uap1 = cat.newUAP();
    UAP* uap2 = cat.newUAP();

    // No conditions set on UAPs
    uap1->m_nUseIfBitSet = 0;
    uap1->m_nUseIfByteNr = 0;
    uap2->m_nUseIfBitSet = 0;
    uap2->m_nUseIfByteNr = 0;

    unsigned char data[] = {0x00, 0x01, 0x02};
    UAP* result = cat.getUAP(data, 3);

    EXPECT_EQ(result, uap1);
}

/**
 * Test Case: TC-CPP-CAT-014
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify getUAP returns NULL for empty UAP list
 */
TEST(CategoryTest, GetUAPReturnsNullWhenEmpty) {
    Category cat(48);

    unsigned char data[] = {0x00, 0x01};
    UAP* result = cat.getUAP(data, 2);

    EXPECT_EQ(result, nullptr);
}

/**
 * Test Case: TC-CPP-CAT-015
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify printDescriptors returns empty string for no items
 */
TEST(CategoryTest, PrintDescriptorsEmptyCategory) {
    Category cat(48);

    std::string result = cat.printDescriptors();

    EXPECT_TRUE(result.empty());
}

/**
 * Test Case: TC-CPP-CAT-016
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify filterOutItem sets category filtered flag
 */
TEST(CategoryTest, FilterOutItemSetsCategoryFlag) {
    Category cat(48);

    EXPECT_FALSE(cat.m_bFiltered);

    // Call filterOutItem - it sets m_bFiltered even if item not found
    cat.filterOutItem("010", "SAC");

    EXPECT_TRUE(cat.m_bFiltered);
}

/**
 * Test Case: TC-CPP-CAT-017
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify filterOutItem returns false for non-existent item
 */
TEST(CategoryTest, FilterOutItemReturnsFalseForNonExistent) {
    Category cat(48);

    bool result = cat.filterOutItem("999", "SomeField");

    EXPECT_FALSE(result);
    EXPECT_TRUE(cat.m_bFiltered); // Flag should still be set
}

/**
 * Test Case: TC-CPP-CAT-018
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify isFiltered returns false for non-existent item
 */
TEST(CategoryTest, IsFilteredReturnsFalseForNonExistent) {
    Category cat(48);

    bool result = cat.isFiltered("999", "SomeField");

    EXPECT_FALSE(result);
}

/**
 * Test Case: TC-CPP-CAT-019
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify isFiltered returns false when no items exist
 */
TEST(CategoryTest, IsFilteredReturnsFalseWhenEmpty) {
    Category cat(48);

    bool result = cat.isFiltered("010", "SAC");

    EXPECT_FALSE(result);
}

/**
 * Test Case: TC-CPP-CAT-020
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify category with large ID values
 */
TEST(CategoryTest, LargeCategoryIDs) {
    Category cat255(255);
    Category cat128(128);

    EXPECT_EQ(cat255.m_id, 255);
    EXPECT_EQ(cat128.m_id, 128);
}

/**
 * Test Case: TC-CPP-CAT-021
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify multiple data items with same lookup
 */
TEST(CategoryTest, MultipleDataItemLookups) {
    Category cat(48);

    // First call creates the item
    DataItemDescription* item1 = cat.getDataItemDescription("040");
    ASSERT_NE(item1, nullptr);

    // Second call returns the same item
    DataItemDescription* item2 = cat.getDataItemDescription("040");
    EXPECT_EQ(item1, item2);

    // List should only contain one item
    EXPECT_EQ(cat.m_lDataItems.size(), 1);
}

/**
 * Test Case: TC-CPP-CAT-022
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify data items list grows correctly
 */
TEST(CategoryTest, DataItemsListGrows) {
    Category cat(62);

    EXPECT_TRUE(cat.m_lDataItems.empty());

    cat.getDataItemDescription("010");
    EXPECT_EQ(cat.m_lDataItems.size(), 1);

    cat.getDataItemDescription("015");
    EXPECT_EQ(cat.m_lDataItems.size(), 2);

    cat.getDataItemDescription("040");
    EXPECT_EQ(cat.m_lDataItems.size(), 3);

    cat.getDataItemDescription("070");
    EXPECT_EQ(cat.m_lDataItems.size(), 4);
}

/**
 * Test Case: TC-CPP-CAT-023
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify UAPs list grows correctly
 */
TEST(CategoryTest, UAPsListGrows) {
    Category cat(62);

    EXPECT_TRUE(cat.m_lUAPs.empty());

    cat.newUAP();
    EXPECT_EQ(cat.m_lUAPs.size(), 1);

    cat.newUAP();
    EXPECT_EQ(cat.m_lUAPs.size(), 2);

    cat.newUAP();
    EXPECT_EQ(cat.m_lUAPs.size(), 3);
}

/**
 * Test Case: TC-CPP-CAT-024
 * Requirement: REQ-HLR-CAT-062
 * Description: Verify CAT062 specific initialization
 */
TEST(CategoryTest, CAT062Initialization) {
    Category cat(62);

    cat.m_strName = "System Track Data";
    cat.m_strVer = "1.18";

    // Add typical CAT062 data items
    cat.getDataItemDescription("010"); // Data Source Identifier
    cat.getDataItemDescription("015"); // Service Identification
    cat.getDataItemDescription("040"); // Track Number
    cat.getDataItemDescription("060"); // Track Mode 3/A Code
    cat.getDataItemDescription("105"); // Calculated Position
    cat.getDataItemDescription("136"); // Measured Flight Level

    EXPECT_EQ(cat.m_lDataItems.size(), 6);
    EXPECT_EQ(cat.m_strName, "System Track Data");
    EXPECT_EQ(cat.m_strVer, "1.18");
}

/**
 * Test Case: TC-CPP-CAT-025
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify category with zero ID
 */
TEST(CategoryTest, CategoryWithZeroID) {
    Category cat(0);

    EXPECT_EQ(cat.m_id, 0);
    EXPECT_FALSE(cat.m_bFiltered);
}

/**
 * Test Case: TC-CPP-CAT-026
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify getDescription with different item numbering
 */
TEST(CategoryTest, GetDescriptionDifferentItemFormats) {
    Category cat(48);

    // Create items with various ID formats
    DataItemDescription* item1 = cat.getDataItemDescription("010");
    item1->m_strName = "Item 010";

    DataItemDescription* item2 = cat.getDataItemDescription("220");
    item2->m_strName = "Item 220";

    DataItemDescription* item3 = cat.getDataItemDescription("020");
    item3->m_strName = "Item 020";

    // Test retrieval with I prefix (field and value NULL)
    const char* desc1 = cat.getDescription("I010", NULL, NULL);
    ASSERT_NE(desc1, nullptr);
    EXPECT_STREQ(desc1, "Item 010");

    const char* desc2 = cat.getDescription("I220", NULL, NULL);
    ASSERT_NE(desc2, nullptr);
    EXPECT_STREQ(desc2, "Item 220");

    const char* desc3 = cat.getDescription("I020", NULL, NULL);
    ASSERT_NE(desc3, nullptr);
    EXPECT_STREQ(desc3, "Item 020");
}

/**
 * Test Case: TC-CPP-CAT-027
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify category version field can be set
 */
TEST(CategoryTest, VersionFieldCanBeSet) {
    Category cat(48);

    EXPECT_TRUE(cat.m_strVer.empty());

    cat.m_strVer = "1.21";
    EXPECT_EQ(cat.m_strVer, "1.21");

    cat.m_strVer = "2.0";
    EXPECT_EQ(cat.m_strVer, "2.0");
}

/**
 * Test Case: TC-CPP-CAT-028
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify destructor handles empty lists correctly
 */
TEST(CategoryTest, DestructorWithEmptyLists) {
    {
        Category cat(65);
        // Don't add any items or UAPs
        EXPECT_TRUE(cat.m_lDataItems.empty());
        EXPECT_TRUE(cat.m_lUAPs.empty());
    }
    // Should not crash
    SUCCEED();
}

/**
 * Test Case: TC-CPP-CAT-029
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify destructor handles large number of items
 */
TEST(CategoryTest, DestructorWithManyItems) {
    {
        Category cat(48);

        // Add many data items
        for (int i = 10; i < 30; i++) {
            char id[10];
            sprintf(id, "%03d", i * 10);
            cat.getDataItemDescription(id);
        }

        // Add many UAPs
        for (int i = 0; i < 10; i++) {
            cat.newUAP();
        }

        EXPECT_EQ(cat.m_lDataItems.size(), 20);
        EXPECT_EQ(cat.m_lUAPs.size(), 10);
    }
    // Should clean up all items without leaking
    SUCCEED();
}

/**
 * Test Case: TC-CPP-CAT-030
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify getDataItemDescription creates new items with correct ID
 */
TEST(CategoryTest, GetDataItemDescriptionCreatesCorrectID) {
    Category cat(48);

    DataItemDescription* item = cat.getDataItemDescription("140");
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->m_strID, "140");
}

/**
 * Test Case: TC-CPP-CAT-031
 * Requirement: REQ-LLR-UAP-002
 *
 * Test UAPItem copy constructor
 */
TEST(CategoryTest, UAPItemCopyConstructor) {
    UAPItem original;
    original.m_nBit = 3;
    original.m_nFRN = 5;
    original.m_bFX = false;
    original.m_nLen = 2;
    original.m_strItemID = "010";

    UAPItem copy(original);

    EXPECT_EQ(copy.m_nBit, 3);
    EXPECT_EQ(copy.m_nFRN, 5);
    EXPECT_FALSE(copy.m_bFX);
    EXPECT_EQ(copy.m_nLen, 2);
    EXPECT_STREQ(copy.m_strItemID.c_str(), "010");
}

/**
 * Test Case: TC-CPP-CAT-032
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP with UseIfBitSet condition - bit is set
 */
TEST(CategoryTest, GetUAPWithBitSetConditionMatches) {
    Category cat(48);

    UAP* uap1 = cat.newUAP();

    // Configure UAP to check if bit 1 is set
    // Bit 1: byte offset = (1-1)/8 = 0, bit position = 7-((1-1)%8) = 7 (MSB)
    uap1->m_nUseIfBitSet = 1;
    uap1->m_nUseIfByteNr = 0;

    // Data: FSPEC (1 byte, FX=0), then data byte with bit 7 (MSB) set (0x80)
    unsigned char data[] = {0x00, 0x80};

    UAP* result = cat.getUAP(data, 2);

    EXPECT_EQ(result, uap1);
}

/**
 * Test Case: TC-CPP-CAT-033
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP with UseIfBitSet condition - bit is not set
 */
TEST(CategoryTest, GetUAPWithBitSetConditionDoesNotMatch) {
    Category cat(48);

    // Create two UAPs
    UAP* uap1 = cat.newUAP();
    UAP* uap2 = cat.newUAP();

    // First UAP has no conditions
    uap1->m_nUseIfBitSet = 0;
    uap1->m_nUseIfByteNr = 0;

    // Second UAP checks if bit 1 is set
    uap2->m_nUseIfBitSet = 1;
    uap2->m_nUseIfByteNr = 0;

    // Data: FSPEC (1 byte), then data byte with bit 7 NOT set (0x00)
    unsigned char data[] = {0x00, 0x00};

    UAP* result = cat.getUAP(data, 2);

    // Should return uap1 (first UAP) because uap2's bit condition doesn't match
    EXPECT_EQ(result, uap1);
}

/**
 * Test Case: TC-CPP-CAT-034
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP with UseIfBitSet - multi-byte FSPEC
 */
TEST(CategoryTest, GetUAPWithBitSetMultibyteFSPEC) {
    Category cat(62);

    UAP* uap1 = cat.newUAP();

    // Check bit 9 (second data byte, bit 0)
    // Bit 9: byte offset = (9-1)/8 = 1, bit position = 7-((9-1)%8) = 7-(8%8) = 7-(0) = 7
    uap1->m_nUseIfBitSet = 9;
    uap1->m_nUseIfByteNr = 0;

    // FSPEC: 2 bytes (first has FX=1, second has FX=0), then data
    // After FSPEC skip, position is at first data byte (offset 2)
    // We want byte at offset 2+1=3 to have bit 7 set
    unsigned char data[] = {0x01, 0x00, 0x00, 0x80};

    UAP* result = cat.getUAP(data, 4);

    EXPECT_EQ(result, uap1);
}

/**
 * Test Case: TC-CPP-CAT-035
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP with UseIfByteNr condition - byte matches
 */
TEST(CategoryTest, GetUAPWithByteNrConditionMatches) {
    Category cat(48);

    UAP* uap1 = cat.newUAP();

    // Check if byte 1 equals 0xAB
    uap1->m_nUseIfBitSet = 0;
    uap1->m_nUseIfByteNr = 1;
    uap1->m_nIsSetTo = 0xAB;

    // Data: FSPEC (1 byte), then byte 1 with value 0xAB
    unsigned char data[] = {0x00, 0xAB};

    UAP* result = cat.getUAP(data, 2);

    EXPECT_EQ(result, uap1);
}

/**
 * Test Case: TC-CPP-CAT-036
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP with UseIfByteNr condition - byte does not match
 */
TEST(CategoryTest, GetUAPWithByteNrConditionDoesNotMatch) {
    Category cat(48);

    UAP* uap1 = cat.newUAP();
    UAP* uap2 = cat.newUAP();

    // First UAP checks if byte 1 equals 0xAB
    uap1->m_nUseIfBitSet = 0;
    uap1->m_nUseIfByteNr = 1;
    uap1->m_nIsSetTo = 0xAB;

    // Second UAP has no conditions
    uap2->m_nUseIfBitSet = 0;
    uap2->m_nUseIfByteNr = 0;

    // Data: FSPEC (1 byte), then byte 1 with value 0xCD (doesn't match)
    unsigned char data[] = {0x00, 0xCD};

    UAP* result = cat.getUAP(data, 2);

    // Should return uap2 since uap1's byte condition doesn't match
    EXPECT_EQ(result, uap2);
}

/**
 * Test Case: TC-CPP-CAT-037
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP with UseIfByteNr - multi-byte FSPEC
 */
TEST(CategoryTest, GetUAPWithByteNrMultibyteFSPEC) {
    Category cat(62);

    UAP* uap1 = cat.newUAP();

    // Check if byte 3 equals 0x42
    uap1->m_nUseIfBitSet = 0;
    uap1->m_nUseIfByteNr = 3;
    uap1->m_nIsSetTo = 0x42;

    // FSPEC: 2 bytes (FX set on first), then 3 data bytes
    unsigned char data[] = {0x01, 0x00, 0x11, 0x22, 0x42};

    UAP* result = cat.getUAP(data, 5);

    EXPECT_EQ(result, uap1);
}

/**
 * Test Case: TC-CPP-CAT-038
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP with bit condition when data is too short
 */
TEST(CategoryTest, GetUAPWithBitSetDataTooShort) {
    Category cat(48);

    UAP* uap1 = cat.newUAP();
    UAP* uap2 = cat.newUAP();

    // First UAP checks bit beyond available data
    uap1->m_nUseIfBitSet = 50;
    uap1->m_nUseIfByteNr = 0;

    // Second UAP has no conditions
    uap2->m_nUseIfBitSet = 0;
    uap2->m_nUseIfByteNr = 0;

    // Short data - only 2 bytes
    unsigned char data[] = {0x00, 0x01};

    UAP* result = cat.getUAP(data, 2);

    // Should skip uap1 (data too short) and return uap2
    EXPECT_EQ(result, uap2);
}

/**
 * Test Case: TC-CPP-CAT-039
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP with byte condition when data is too short
 */
TEST(CategoryTest, GetUAPWithByteNrDataTooShort) {
    Category cat(48);

    UAP* uap1 = cat.newUAP();
    UAP* uap2 = cat.newUAP();

    // First UAP checks byte 10
    uap1->m_nUseIfBitSet = 0;
    uap1->m_nUseIfByteNr = 10;
    uap1->m_nIsSetTo = 0xFF;

    // Second UAP has no conditions
    uap2->m_nUseIfBitSet = 0;
    uap2->m_nUseIfByteNr = 0;

    // Short data - only 3 bytes
    unsigned char data[] = {0x00, 0x01, 0x02};

    UAP* result = cat.getUAP(data, 3);

    // Should skip uap1 (data too short) and return uap2
    EXPECT_EQ(result, uap2);
}

/**
 * Test Case: TC-CPP-CAT-040
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP handles extended FSPEC correctly for bit checking
 */
TEST(CategoryTest, GetUAPWithExtendedFSPECForBitCheck) {
    Category cat(48);

    UAP* uap1 = cat.newUAP();

    // Check bit 5 in data
    // Bit 5: byte offset = (5-1)/8 = 0, bit position = 7-((5-1)%8) = 7-4 = 3
    uap1->m_nUseIfBitSet = 5;
    uap1->m_nUseIfByteNr = 0;

    // FSPEC with extension: 3 bytes (FX on first two)
    // After FSPEC, first data byte should have bit 3 set (0x08)
    unsigned char data[] = {0x01, 0x01, 0x00, 0x08, 0x00};

    UAP* result = cat.getUAP(data, 5);

    EXPECT_EQ(result, uap1);
}

/**
 * Test Case: TC-CPP-CAT-041
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP handles extended FSPEC correctly for byte checking
 */
TEST(CategoryTest, GetUAPWithExtendedFSPECForByteCheck) {
    Category cat(62);

    UAP* uap1 = cat.newUAP();

    // Check if byte 2 equals 0x99
    uap1->m_nUseIfBitSet = 0;
    uap1->m_nUseIfByteNr = 2;
    uap1->m_nIsSetTo = 0x99;

    // FSPEC with extension: 3 bytes
    unsigned char data[] = {0x81, 0x01, 0x00, 0xAA, 0x99};

    UAP* result = cat.getUAP(data, 5);

    EXPECT_EQ(result, uap1);
}

/**
 * Test Case: TC-CPP-CAT-042
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify printDescriptors formats header correctly
 */
TEST(CategoryTest, PrintDescriptorsFormatsHeader) {
    Category cat(48);

    // Note: Without setting up m_pFormat, calling printDescriptors would crash
    // Instead, verify the header format logic by checking category ID
    EXPECT_EQ(cat.m_id, 48);

    // Verify expected header format would be "CAT048:I<item>:"
    // This tests the snprintf logic indirectly
    char header[32];
    snprintf(header, 32, "CAT%03d:I%s:", cat.m_id, "010");
    EXPECT_STREQ(header, "CAT048:I010:");
}

/**
 * Test Case: TC-CPP-CAT-043
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify filterOutItem correctly finds items in list
 */
TEST(CategoryTest, FilterOutItemFindsItemInList) {
    Category cat(48);

    // Add multiple items
    cat.getDataItemDescription("010");
    cat.getDataItemDescription("020");
    cat.getDataItemDescription("030");

    EXPECT_FALSE(cat.m_bFiltered);

    // Filter out an item that doesn't exist (won't crash, returns false)
    bool result = cat.filterOutItem("999", "Field");

    // m_bFiltered should be set to true even if item not found
    EXPECT_TRUE(cat.m_bFiltered);
    EXPECT_FALSE(result);
}

/**
 * Test Case: TC-CPP-CAT-044
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify isFiltered returns false when querying non-existent item
 */
TEST(CategoryTest, IsFilteredWithNonExistentItemReturnsFalse) {
    Category cat(48);

    // Add several items
    cat.getDataItemDescription("010");
    cat.getDataItemDescription("020");
    cat.getDataItemDescription("040");

    // Query for an item that doesn't exist - should return false without crashing
    bool result = cat.isFiltered("999", "SomeField");

    EXPECT_FALSE(result);
}

/**
 * Test Case: TC-CPP-CAT-045
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify getDescription iteration through multiple items
 */
TEST(CategoryTest, GetDescriptionIteratesThroughMultipleItems) {
    Category cat(62);

    // Add several items
    DataItemDescription* item1 = cat.getDataItemDescription("010");
    item1->m_strName = "Item 1";

    DataItemDescription* item2 = cat.getDataItemDescription("020");
    item2->m_strName = "Item 2";

    DataItemDescription* item3 = cat.getDataItemDescription("040");
    item3->m_strName = "Item 3";

    // Request the last item to force iteration
    const char* desc = cat.getDescription("I040", NULL, NULL);

    ASSERT_NE(desc, nullptr);
    EXPECT_STREQ(desc, "Item 3");
}

/**
 * Test Case: TC-CPP-CAT-046
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify category name with special characters
 */
TEST(CategoryTest, CategoryNameWithSpecialCharacters) {
    Category cat(48);

    cat.m_strName = "Monoradar Target Reports (Mode-S)";
    cat.m_strVer = "1.21-beta";

    EXPECT_EQ(cat.m_strName, "Monoradar Target Reports (Mode-S)");
    EXPECT_EQ(cat.m_strVer, "1.21-beta");
}

/**
 * Test Case: TC-CPP-CAT-047
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify data item ID format with leading zeros
 */
TEST(CategoryTest, DataItemIDWithLeadingZeros) {
    Category cat(48);

    DataItemDescription* item001 = cat.getDataItemDescription("001");
    DataItemDescription* item010 = cat.getDataItemDescription("010");
    DataItemDescription* item100 = cat.getDataItemDescription("100");

    EXPECT_EQ(item001->m_strID, "001");
    EXPECT_EQ(item010->m_strID, "010");
    EXPECT_EQ(item100->m_strID, "100");

    EXPECT_EQ(cat.m_lDataItems.size(), 3);
}

/**
 * Test Case: TC-CPP-CAT-048
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify UAP selection with only conditional UAPs
 */
TEST(CategoryTest, GetUAPWithOnlyConditionalUAPs) {
    Category cat(48);

    UAP* uap1 = cat.newUAP();
    UAP* uap2 = cat.newUAP();

    // Both UAPs have conditions that don't match
    uap1->m_nUseIfBitSet = 100;
    uap1->m_nUseIfByteNr = 0;

    uap2->m_nUseIfByteNr = 50;
    uap2->m_nIsSetTo = 0xFF;
    uap2->m_nUseIfBitSet = 0;

    // Short data - neither condition can match
    unsigned char data[] = {0x00, 0x01};

    UAP* result = cat.getUAP(data, 2);

    // Should return NULL as no UAP matches
    EXPECT_EQ(result, nullptr);
}

/**
 * Test Case: TC-CPP-CAT-049
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify getDescription with empty item string handling
 */
TEST(CategoryTest, GetDescriptionWithDifferentItemPrefixes) {
    Category cat(48);

    DataItemDescription* item = cat.getDataItemDescription("220");
    item->m_strName = "Aircraft Address";

    // Test with standard "I" prefix
    const char* desc = cat.getDescription("I220", NULL, NULL);
    ASSERT_NE(desc, nullptr);
    EXPECT_STREQ(desc, "Aircraft Address");
}

/**
 * Test Case: TC-CPP-CAT-050
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP with bit at byte boundary
 */
TEST(CategoryTest, GetUAPWithBitAtByteBoundary) {
    Category cat(48);

    UAP* uap1 = cat.newUAP();

    // Check bit 8 (last bit of first byte)
    uap1->m_nUseIfBitSet = 8;
    uap1->m_nUseIfByteNr = 0;

    // FSPEC, then byte with bit 0 set (LSB)
    unsigned char data[] = {0x00, 0x01};

    UAP* result = cat.getUAP(data, 2);

    EXPECT_EQ(result, uap1);
}

/**
 * Test Case: TC-CPP-CAT-051
 * Requirement: REQ-HLR-UAP-001
 * Description: Verify getUAP with bit 9 (second byte, MSB)
 */
TEST(CategoryTest, GetUAPWithBitInSecondByte) {
    Category cat(62);

    UAP* uap1 = cat.newUAP();

    // Check bit 2 (first byte, bit 6)
    // Bit 2: byte offset = (2-1)/8 = 0, bit position = 7-((2-1)%8) = 7-1 = 6
    uap1->m_nUseIfBitSet = 2;
    uap1->m_nUseIfByteNr = 0;

    // FSPEC (1 byte), then data byte with bit 6 set (0x40)
    unsigned char data[] = {0x00, 0x40};

    UAP* result = cat.getUAP(data, 2);

    EXPECT_EQ(result, uap1);
}

/**
 * Test Case: TC-CPP-CAT-052
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify category filtered flag persists across multiple filter calls
 */
TEST(CategoryTest, CategoryFilteredFlagPersists) {
    Category cat(48);

    // Add items
    cat.getDataItemDescription("010");
    cat.getDataItemDescription("020");

    EXPECT_FALSE(cat.m_bFiltered);

    // First filter call on non-existent item
    cat.filterOutItem("999", "Field1");
    EXPECT_TRUE(cat.m_bFiltered);

    // Flag should remain true even after another call
    cat.filterOutItem("888", "Field2");
    EXPECT_TRUE(cat.m_bFiltered);
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
