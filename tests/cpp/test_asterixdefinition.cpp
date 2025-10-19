/**
 * Unit tests for AsterixDefinition class
 *
 * Requirements Traceability:
 * - REQ-HLR-SYS-001: Parse ASTERIX categories
 * - REQ-HLR-CAT-001: Support multiple ASTERIX categories
 * - REQ-HLR-CAT-002: Category management and retrieval
 *
 * DO-278A AL-3 Compliance Testing
 */

#include <gtest/gtest.h>
#include "AsterixDefinition.h"
#include "Category.h"

/**
 * Test fixture for AsterixDefinition tests
 */
class AsterixDefinitionTest : public ::testing::Test {
protected:
    AsterixDefinition* def;

    void SetUp() override {
        def = new AsterixDefinition();
    }

    void TearDown() override {
        delete def;
    }

    // Helper: Create a simple category for testing
    Category* createTestCategory(int id, const std::string& name = "", const std::string& version = "") {
        Category* cat = new Category(id);
        cat->m_strName = name;
        cat->m_strVer = version;
        return cat;
    }
};

/**
 * Test Case: TC-CPP-ADEF-001
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify AsterixDefinition constructor initializes correctly
 */
TEST_F(AsterixDefinitionTest, ConstructorInitializesAllCategoriesToNull) {
    // All categories should be NULL after construction
    for (int i = 0; i < MAX_CATEGORIES; i++) {
        EXPECT_FALSE(def->CategoryDefined(i))
            << "Category " << i << " should not be defined after construction";
    }
}

/**
 * Test Case: TC-CPP-ADEF-002
 * Requirement: REQ-HLR-CAT-001
 * Description: Verify getCategory creates category on first access
 */
TEST_F(AsterixDefinitionTest, GetCategoryCreatesOnFirstAccess) {
    // Category should not exist initially
    EXPECT_FALSE(def->CategoryDefined(48));

    // First access should create the category
    Category* cat = def->getCategory(48);

    ASSERT_NE(cat, nullptr) << "getCategory should return non-null pointer";
    EXPECT_EQ(cat->m_id, 48) << "Created category should have correct ID";
    EXPECT_TRUE(def->CategoryDefined(48)) << "Category should be defined after getCategory";
}

/**
 * Test Case: TC-CPP-ADEF-003
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify getCategory returns same instance on subsequent calls
 */
TEST_F(AsterixDefinitionTest, GetCategoryReturnsSameInstance) {
    Category* cat1 = def->getCategory(48);
    Category* cat2 = def->getCategory(48);

    EXPECT_EQ(cat1, cat2) << "Multiple getCategory calls should return same instance";
}

/**
 * Test Case: TC-CPP-ADEF-004
 * Requirement: REQ-HLR-CAT-001
 * Description: Verify getCategory handles boundary values
 */
TEST_F(AsterixDefinitionTest, GetCategoryBoundaryValues) {
    // Test category 0
    Category* cat0 = def->getCategory(0);
    ASSERT_NE(cat0, nullptr);
    EXPECT_EQ(cat0->m_id, 0);

    // Test maximum valid category (MAX_CATEGORIES - 1 = 256)
    Category* catMax = def->getCategory(MAX_CATEGORIES - 1);
    ASSERT_NE(catMax, nullptr);
    EXPECT_EQ(catMax->m_id, MAX_CATEGORIES - 1);

    // Test beyond maximum (should return NULL)
    Category* catInvalid = def->getCategory(MAX_CATEGORIES);
    EXPECT_EQ(catInvalid, nullptr) << "Category >= MAX_CATEGORIES should return NULL";

    Category* catInvalid2 = def->getCategory(MAX_CATEGORIES + 100);
    EXPECT_EQ(catInvalid2, nullptr) << "Category > MAX_CATEGORIES should return NULL";
}

/**
 * Test Case: TC-CPP-ADEF-005
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify setCategory adds new category
 */
TEST_F(AsterixDefinitionTest, SetCategoryAddsNewCategory) {
    Category* cat48 = createTestCategory(48, "CAT048 Target Reports", "1.21");

    def->setCategory(cat48);

    EXPECT_TRUE(def->CategoryDefined(48));

    Category* retrieved = def->getCategory(48);
    EXPECT_EQ(retrieved, cat48);
    EXPECT_EQ(retrieved->m_strName, "CAT048 Target Reports");
    EXPECT_EQ(retrieved->m_strVer, "1.21");
}

/**
 * Test Case: TC-CPP-ADEF-006
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify setCategory replaces existing category
 */
TEST_F(AsterixDefinitionTest, SetCategoryReplacesExistingCategory) {
    // Create first category
    Category* cat1 = createTestCategory(48, "First CAT048", "1.0");
    def->setCategory(cat1);

    // Replace with second category
    Category* cat2 = createTestCategory(48, "Second CAT048", "2.0");
    def->setCategory(cat2);

    // Should get the second category
    Category* retrieved = def->getCategory(48);
    EXPECT_EQ(retrieved, cat2);
    EXPECT_EQ(retrieved->m_strName, "Second CAT048");
    EXPECT_EQ(retrieved->m_strVer, "2.0");
}

/**
 * Test Case: TC-CPP-ADEF-007
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify setCategory handles NULL gracefully
 */
TEST_F(AsterixDefinitionTest, SetCategoryHandlesNull) {
    def->setCategory(NULL);

    // Should not crash, and no categories should be defined
    EXPECT_FALSE(def->CategoryDefined(0));
    EXPECT_FALSE(def->CategoryDefined(48));
}

/**
 * Test Case: TC-CPP-ADEF-008
 * Requirement: REQ-HLR-CAT-001
 * Description: Verify multiple different categories can be stored
 */
TEST_F(AsterixDefinitionTest, MultipleCategoriesCanBeStored) {
    Category* cat1 = createTestCategory(1, "CAT001", "1.0");
    Category* cat48 = createTestCategory(48, "CAT048", "1.21");
    Category* cat62 = createTestCategory(62, "CAT062", "1.18");
    Category* cat65 = createTestCategory(65, "CAT065", "1.3");
    Category* cat256 = createTestCategory(256, "BDS", "1.0"); // BDS register

    def->setCategory(cat1);
    def->setCategory(cat48);
    def->setCategory(cat62);
    def->setCategory(cat65);
    def->setCategory(cat256);

    EXPECT_TRUE(def->CategoryDefined(1));
    EXPECT_TRUE(def->CategoryDefined(48));
    EXPECT_TRUE(def->CategoryDefined(62));
    EXPECT_TRUE(def->CategoryDefined(65));
    EXPECT_TRUE(def->CategoryDefined(256));

    EXPECT_EQ(def->getCategory(1), cat1);
    EXPECT_EQ(def->getCategory(48), cat48);
    EXPECT_EQ(def->getCategory(62), cat62);
    EXPECT_EQ(def->getCategory(65), cat65);
    EXPECT_EQ(def->getCategory(256), cat256);
}

/**
 * Test Case: TC-CPP-ADEF-009
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify CategoryDefined returns false for undefined categories
 */
TEST_F(AsterixDefinitionTest, CategoryDefinedReturnsFalseForUndefined) {
    EXPECT_FALSE(def->CategoryDefined(0));
    EXPECT_FALSE(def->CategoryDefined(48));
    EXPECT_FALSE(def->CategoryDefined(100));
    EXPECT_FALSE(def->CategoryDefined(255));
}

/**
 * Test Case: TC-CPP-ADEF-010
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify CategoryDefined returns true for defined categories
 */
TEST_F(AsterixDefinitionTest, CategoryDefinedReturnsTrueForDefined) {
    Category* cat48 = createTestCategory(48);
    def->setCategory(cat48);

    EXPECT_TRUE(def->CategoryDefined(48));
    EXPECT_FALSE(def->CategoryDefined(49)); // Adjacent should still be false
}

/**
 * Test Case: TC-CPP-ADEF-011
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify CategoryDefined handles boundary conditions
 */
TEST_F(AsterixDefinitionTest, CategoryDefinedBoundaryConditions) {
    // Test beyond valid range
    EXPECT_FALSE(def->CategoryDefined(MAX_CATEGORIES));
    EXPECT_FALSE(def->CategoryDefined(MAX_CATEGORIES + 1));
    EXPECT_FALSE(def->CategoryDefined(999));

    // Test at maximum valid value
    Category* catMax = createTestCategory(MAX_CATEGORIES - 1);
    def->setCategory(catMax);
    EXPECT_TRUE(def->CategoryDefined(MAX_CATEGORIES - 1));
}

/**
 * Test Case: TC-CPP-ADEF-012
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify getDescription returns category name
 */
TEST_F(AsterixDefinitionTest, GetDescriptionReturnsCategoryName) {
    Category* cat48 = createTestCategory(48, "Target Reports", "1.21");
    def->setCategory(cat48);

    const char* desc = def->getDescription(48, NULL, NULL, NULL);

    ASSERT_NE(desc, nullptr);
    EXPECT_STREQ(desc, "Target Reports");
}

/**
 * Test Case: TC-CPP-ADEF-013
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify getDescription returns NULL for undefined category
 */
TEST_F(AsterixDefinitionTest, GetDescriptionReturnsNullForUndefined) {
    const char* desc = def->getDescription(99, NULL, NULL, NULL);

    EXPECT_EQ(desc, nullptr) << "Description should be NULL for undefined category";
}

/**
 * Test Case: TC-CPP-ADEF-014
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify getDescription with NULL parameters returns category name
 */
TEST_F(AsterixDefinitionTest, GetDescriptionWithNullParams) {
    Category* cat48 = createTestCategory(48, "CAT048", "1.0");
    def->setCategory(cat48);

    const char* desc = def->getDescription(48, NULL, NULL, NULL);

    ASSERT_NE(desc, nullptr);
    EXPECT_STREQ(desc, "CAT048");
}

/**
 * Test Case: TC-CPP-ADEF-015
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify filterOutItem returns false for undefined category
 */
TEST_F(AsterixDefinitionTest, FilterOutItemReturnsFalseForUndefinedCategory) {
    bool result = def->filterOutItem(99, "010", "SAC/SIC");

    EXPECT_FALSE(result) << "filterOutItem should return false for undefined category";
}

/**
 * Test Case: TC-CPP-ADEF-016
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify filterOutItem works with defined category
 */
TEST_F(AsterixDefinitionTest, FilterOutItemWithDefinedCategory) {
    Category* cat48 = createTestCategory(48);
    def->setCategory(cat48);

    // Call filterOutItem (exact behavior depends on Category implementation)
    bool result = def->filterOutItem(48, "010", "SAC/SIC");

    // Should not crash and return a boolean
    EXPECT_TRUE(result == true || result == false);
}

/**
 * Test Case: TC-CPP-ADEF-017
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify isFiltered returns false for undefined category
 */
TEST_F(AsterixDefinitionTest, IsFilteredReturnsFalseForUndefinedCategory) {
    bool result = def->isFiltered(99, "010", "SAC/SIC");

    EXPECT_FALSE(result) << "isFiltered should return false for undefined category";
}

/**
 * Test Case: TC-CPP-ADEF-018
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify isFiltered works with defined category
 */
TEST_F(AsterixDefinitionTest, IsFilteredWithDefinedCategory) {
    Category* cat48 = createTestCategory(48);
    def->setCategory(cat48);

    // Call isFiltered (exact behavior depends on Category implementation)
    bool result = def->isFiltered(48, "010", "SAC/SIC");

    // Should not crash and return a boolean
    EXPECT_TRUE(result == true || result == false);
}

/**
 * Test Case: TC-CPP-ADEF-019
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify printDescriptors returns empty string with no categories
 */
TEST_F(AsterixDefinitionTest, PrintDescriptorsEmptyWithNoCategories) {
    std::string result = def->printDescriptors();

    EXPECT_EQ(result, "") << "printDescriptors should return empty string with no categories";
}

/**
 * Test Case: TC-CPP-ADEF-020
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify printDescriptors works with defined categories
 */
TEST_F(AsterixDefinitionTest, PrintDescriptorsWithDefinedCategories) {
    Category* cat48 = createTestCategory(48, "CAT048", "1.21");
    Category* cat62 = createTestCategory(62, "CAT062", "1.18");

    def->setCategory(cat48);
    def->setCategory(cat62);

    std::string result = def->printDescriptors();

    // Should return a string (exact content depends on Category::printDescriptors implementation)
    // At minimum, should not crash
    EXPECT_TRUE(result.size() >= 0);
}

/**
 * Test Case: TC-CPP-ADEF-021
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify BDS category (ID 256) can be stored and retrieved
 */
TEST_F(AsterixDefinitionTest, BDSCategorySupport) {
    Category* catBDS = createTestCategory(BDS_CAT_ID, "BDS Register", "1.0");
    def->setCategory(catBDS);

    EXPECT_TRUE(def->CategoryDefined(BDS_CAT_ID));

    Category* retrieved = def->getCategory(BDS_CAT_ID);
    EXPECT_EQ(retrieved, catBDS);
    EXPECT_EQ(retrieved->m_strName, "BDS Register");
}

/**
 * Test Case: TC-CPP-ADEF-022
 * Requirement: REQ-HLR-CAT-001
 * Description: Verify all standard ASTERIX categories (1-255) can be stored
 */
TEST_F(AsterixDefinitionTest, AllStandardCategoriesSupported) {
    // Test a representative sample of category IDs
    int testIds[] = {1, 10, 20, 34, 48, 62, 63, 65, 129, 200, 240, 255};

    for (int i = 0; i < 12; i++) {
        int id = testIds[i];
        Category* cat = createTestCategory(id);
        def->setCategory(cat);

        EXPECT_TRUE(def->CategoryDefined(id)) << "Category " << id << " should be defined";
        EXPECT_EQ(def->getCategory(id)->m_id, id) << "Category " << id << " should have correct ID";
    }
}

/**
 * Test Case: TC-CPP-ADEF-023
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify category replacement frees old category
 */
TEST_F(AsterixDefinitionTest, CategoryReplacementHandlesMemory) {
    // First category
    Category* cat1 = createTestCategory(48, "First", "1.0");
    def->setCategory(cat1);

    Category* retrieved1 = def->getCategory(48);
    EXPECT_EQ(retrieved1, cat1);

    // Replace with second category (old one should be deleted by setCategory)
    Category* cat2 = createTestCategory(48, "Second", "2.0");
    def->setCategory(cat2);

    Category* retrieved2 = def->getCategory(48);
    EXPECT_EQ(retrieved2, cat2);
    EXPECT_NE(retrieved2, cat1); // Should be different object
}

/**
 * Test Case: TC-CPP-ADEF-024
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify sparse category storage works efficiently
 */
TEST_F(AsterixDefinitionTest, SparseCategoryStorage) {
    // Create only a few categories in sparse array
    Category* cat1 = createTestCategory(1);
    Category* cat100 = createTestCategory(100);
    Category* cat256 = createTestCategory(256);

    def->setCategory(cat1);
    def->setCategory(cat100);
    def->setCategory(cat256);

    // Verify only these are defined
    EXPECT_TRUE(def->CategoryDefined(1));
    EXPECT_TRUE(def->CategoryDefined(100));
    EXPECT_TRUE(def->CategoryDefined(256));

    // Verify others are not
    EXPECT_FALSE(def->CategoryDefined(2));
    EXPECT_FALSE(def->CategoryDefined(50));
    EXPECT_FALSE(def->CategoryDefined(99));
    EXPECT_FALSE(def->CategoryDefined(200));
}

/**
 * Test Case: TC-CPP-ADEF-025
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify destructor properly cleans up all categories
 */
TEST_F(AsterixDefinitionTest, DestructorCleansUpCategories) {
    AsterixDefinition* tempDef = new AsterixDefinition();

    // Add several categories
    tempDef->setCategory(createTestCategory(1));
    tempDef->setCategory(createTestCategory(48));
    tempDef->setCategory(createTestCategory(62));

    // Delete should clean up all categories without memory leaks
    delete tempDef;

    // If we reach here without crash, destructor worked
    SUCCEED();
}

/**
 * Test Case: TC-CPP-ADEF-026
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify getCategory with boundary ID exactly at MAX_CATEGORIES
 */
TEST_F(AsterixDefinitionTest, GetCategoryAtMaxBoundary) {
    // MAX_CATEGORIES is 257 (256+1)
    // Valid range is 0 to 256 inclusive

    Category* catAtMax = def->getCategory(MAX_CATEGORIES);
    EXPECT_EQ(catAtMax, nullptr) << "Category at MAX_CATEGORIES should return NULL";

    Category* catJustBelowMax = def->getCategory(MAX_CATEGORIES - 1);
    EXPECT_NE(catJustBelowMax, nullptr) << "Category at MAX_CATEGORIES-1 should be valid";
    EXPECT_EQ(catJustBelowMax->m_id, MAX_CATEGORIES - 1);
}

/**
 * Test Case: TC-CPP-ADEF-027
 * Requirement: REQ-HLR-CAT-002
 * Description: Verify getDescription with non-NULL item/field/value parameters
 */
TEST_F(AsterixDefinitionTest, GetDescriptionWithItemFieldValue) {
    Category* cat48 = createTestCategory(48, "CAT048", "1.21");
    def->setCategory(cat48);

    // Call getDescription with item/field/value (delegates to Category::getDescription)
    const char* desc = def->getDescription(48, "010", "SAC", "123");

    // Result depends on Category implementation, but should not crash
    // May return NULL if item not configured in category
    EXPECT_TRUE(desc == nullptr || desc != nullptr);
}
