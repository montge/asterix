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

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
