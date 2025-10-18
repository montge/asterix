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

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
