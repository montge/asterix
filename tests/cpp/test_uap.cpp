/**
 * Unit tests for UAP (User Application Profile) class
 *
 * UAP Background:
 * - UAP defines field mapping for ASTERIX categories
 * - Each category can have multiple UAPs (different editions/versions)
 * - FRN (Field Reference Number) maps to Data Item IDs
 * - Critical for correct parsing - errors cause misinterpreted surveillance data
 *
 * Coverage Goals:
 * - UAP.cpp: 0% -> 80%+ coverage
 * - Test all public methods and edge cases
 *
 * Requirements Traceability:
 * - REQ-HLR-002: Parse ASTERIX UAP definitions
 * - REQ-LLR-UAP-001: UAP construction and initialization
 * - REQ-LLR-UAP-002: UAP item management
 * - REQ-LLR-UAP-003: FRN to Data Item ID mapping
 * - REQ-LLR-UAP-004: Multiple UAP support
 * - REQ-LLR-UAP-005: Error handling for invalid FRNs
 *
 * DO-278A AL-3 Compliance Testing
 */

#include <gtest/gtest.h>
#include "../../src/asterix/UAP.h"
#include "../../src/asterix/UAPItem.h"
#include <cstring>

/**
 * Test Fixture for UAP tests
 * Provides helper methods for creating test UAP configurations
 */
class UAPTest : public ::testing::Test {
protected:
    UAP* pUAP;

    void SetUp() override {
        pUAP = nullptr;
    }

    void TearDown() override {
        if (pUAP) {
            delete pUAP;
            pUAP = nullptr;
        }
    }

    /**
     * Helper: Create UAP item with specific FRN and Item ID
     */
    void addUAPItem(UAP* uap, int frn, const char* itemId, bool fx = false) {
        UAPItem* item = uap->newUAPItem();
        item->m_nFRN = frn;
        item->m_strItemID = itemId;
        item->m_bFX = fx;
    }
};

/**
 * Test Case: TC-CPP-UAP-001
 * Requirement: REQ-LLR-UAP-001
 * Description: Verify UAP constructor initializes all fields correctly
 */
TEST_F(UAPTest, ConstructorInitialization) {
    pUAP = new UAP();

    // Verify all member variables are initialized to zero/default
    EXPECT_EQ(pUAP->m_nUseIfBitSet, 0);
    EXPECT_EQ(pUAP->m_nUseIfByteNr, 0);
    EXPECT_EQ(pUAP->m_nIsSetTo, 0);
    EXPECT_TRUE(pUAP->m_lUAPItems.empty());
}

/**
 * Test Case: TC-CPP-UAP-002
 * Requirement: REQ-LLR-UAP-002
 * Description: Verify newUAPItem creates and adds UAP item to list
 */
TEST_F(UAPTest, NewUAPItemCreatesAndAdds) {
    pUAP = new UAP();

    UAPItem* item = pUAP->newUAPItem();

    EXPECT_NE(item, nullptr);
    EXPECT_EQ(pUAP->m_lUAPItems.size(), 1);

    // Verify the item is in the list
    EXPECT_EQ(pUAP->m_lUAPItems.front(), item);
}

/**
 * Test Case: TC-CPP-UAP-003
 * Requirement: REQ-LLR-UAP-002
 * Description: Verify multiple UAP items can be added
 */
TEST_F(UAPTest, MultipleUAPItemsCanBeAdded) {
    pUAP = new UAP();

    UAPItem* item1 = pUAP->newUAPItem();
    UAPItem* item2 = pUAP->newUAPItem();
    UAPItem* item3 = pUAP->newUAPItem();

    EXPECT_EQ(pUAP->m_lUAPItems.size(), 3);
    EXPECT_NE(item1, item2);
    EXPECT_NE(item2, item3);
    EXPECT_NE(item1, item3);
}

/**
 * Test Case: TC-CPP-UAP-004
 * Requirement: REQ-LLR-UAP-003
 * Description: Verify getDataItemIDByUAPfrn returns correct item ID for valid FRN
 */
TEST_F(UAPTest, GetDataItemIDByValidFRN) {
    pUAP = new UAP();

    addUAPItem(pUAP, 1, "010");
    addUAPItem(pUAP, 2, "020");
    addUAPItem(pUAP, 3, "040");

    std::string result = pUAP->getDataItemIDByUAPfrn(1);
    EXPECT_EQ(result, "010");

    result = pUAP->getDataItemIDByUAPfrn(2);
    EXPECT_EQ(result, "020");

    result = pUAP->getDataItemIDByUAPfrn(3);
    EXPECT_EQ(result, "040");
}

/**
 * Test Case: TC-CPP-UAP-005
 * Requirement: REQ-LLR-UAP-005
 * Description: Verify getDataItemIDByUAPfrn returns empty string for invalid FRN
 */
TEST_F(UAPTest, GetDataItemIDByInvalidFRN) {
    pUAP = new UAP();

    addUAPItem(pUAP, 1, "010");
    addUAPItem(pUAP, 2, "020");

    // Request non-existent FRN
    std::string result = pUAP->getDataItemIDByUAPfrn(99);
    EXPECT_EQ(result, "");
}

/**
 * Test Case: TC-CPP-UAP-006
 * Requirement: REQ-LLR-UAP-005
 * Description: Verify getDataItemIDByUAPfrn handles empty UAP
 */
TEST_F(UAPTest, GetDataItemIDByFRNEmptyUAP) {
    pUAP = new UAP();

    // UAP has no items
    std::string result = pUAP->getDataItemIDByUAPfrn(1);
    EXPECT_EQ(result, "");
}

/**
 * Test Case: TC-CPP-UAP-007
 * Requirement: REQ-LLR-UAP-003
 * Description: Verify getDataItemIDByUAPfrn with FRN = 0
 */
TEST_F(UAPTest, GetDataItemIDByFRNZero) {
    pUAP = new UAP();

    addUAPItem(pUAP, 0, "000");
    addUAPItem(pUAP, 1, "010");

    std::string result = pUAP->getDataItemIDByUAPfrn(0);
    EXPECT_EQ(result, "000");
}

/**
 * Test Case: TC-CPP-UAP-008
 * Requirement: REQ-LLR-UAP-003
 * Description: Verify getDataItemIDByUAPfrn returns first match for duplicate FRNs
 */
TEST_F(UAPTest, GetDataItemIDByFRNDuplicateFRNs) {
    pUAP = new UAP();

    // Add duplicate FRNs (should not happen in practice, but test robustness)
    addUAPItem(pUAP, 1, "010");
    addUAPItem(pUAP, 1, "020");  // Duplicate FRN

    // Should return the first match
    std::string result = pUAP->getDataItemIDByUAPfrn(1);
    EXPECT_EQ(result, "010");
}

/**
 * Test Case: TC-CPP-UAP-009
 * Requirement: REQ-LLR-UAP-001
 * Description: Verify destructor properly cleans up UAP items
 */
TEST_F(UAPTest, DestructorCleansUpItems) {
    pUAP = new UAP();

    // Add several items
    addUAPItem(pUAP, 1, "010");
    addUAPItem(pUAP, 2, "020");
    addUAPItem(pUAP, 3, "040");

    EXPECT_EQ(pUAP->m_lUAPItems.size(), 3);

    // Delete UAP - should clean up all items
    delete pUAP;
    pUAP = nullptr;

    // If we get here without crash, destructor worked correctly
    SUCCEED();
}

/**
 * Test Case: TC-CPP-UAP-010
 * Requirement: REQ-LLR-UAP-004
 * Description: Verify UAP with conditional selection fields (UseIfBitSet)
 */
TEST_F(UAPTest, ConditionalUAPUseIfBitSet) {
    pUAP = new UAP();

    // Set conditional use fields
    pUAP->m_nUseIfBitSet = 5;    // Bit 5
    pUAP->m_nUseIfByteNr = 2;    // Byte 2
    pUAP->m_nIsSetTo = 1;        // Set to 1

    EXPECT_EQ(pUAP->m_nUseIfBitSet, 5);
    EXPECT_EQ(pUAP->m_nUseIfByteNr, 2);
    EXPECT_EQ(pUAP->m_nIsSetTo, 1);
}

/**
 * Test Case: TC-CPP-UAP-011
 * Requirement: REQ-LLR-UAP-002
 * Description: Verify UAP item list order is preserved
 */
TEST_F(UAPTest, UAPItemListOrderPreserved) {
    pUAP = new UAP();

    addUAPItem(pUAP, 1, "010");
    addUAPItem(pUAP, 2, "020");
    addUAPItem(pUAP, 3, "040");
    addUAPItem(pUAP, 4, "070");

    EXPECT_EQ(pUAP->m_lUAPItems.size(), 4);

    // Verify order by iterating through list
    std::list<UAPItem*>::iterator it = pUAP->m_lUAPItems.begin();
    EXPECT_EQ((*it)->m_nFRN, 1);
    ++it;
    EXPECT_EQ((*it)->m_nFRN, 2);
    ++it;
    EXPECT_EQ((*it)->m_nFRN, 3);
    ++it;
    EXPECT_EQ((*it)->m_nFRN, 4);
}

/**
 * Test Case: TC-CPP-UAP-012
 * Requirement: REQ-LLR-UAP-003
 * Description: Verify getDataItemIDByUAPfrn with CAT048 typical UAP
 */
TEST_F(UAPTest, CAT048TypicalUAP) {
    pUAP = new UAP();

    // CAT048 typical UAP items (first 7 FRNs)
    addUAPItem(pUAP, 1, "010");  // Data Source Identifier
    addUAPItem(pUAP, 2, "020");  // Target Report Descriptor
    addUAPItem(pUAP, 3, "040");  // Measured Position in Slant Polar Coordinates
    addUAPItem(pUAP, 4, "070");  // Mode-3/A Code in Octal Representation
    addUAPItem(pUAP, 5, "090");  // Flight Level in Binary Representation
    addUAPItem(pUAP, 6, "130");  // Radar Plot Characteristics
    addUAPItem(pUAP, 7, "220");  // Aircraft Address

    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(1), "010");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(2), "020");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(3), "040");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(4), "070");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(5), "090");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(6), "130");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(7), "220");
}

/**
 * Test Case: TC-CPP-UAP-013
 * Requirement: REQ-LLR-UAP-003
 * Description: Verify getDataItemIDByUAPfrn with CAT062 typical UAP
 */
TEST_F(UAPTest, CAT062TypicalUAP) {
    pUAP = new UAP();

    // CAT062 typical UAP items (first 7 FRNs)
    addUAPItem(pUAP, 1, "010");  // Data Source Identifier
    addUAPItem(pUAP, 2, "015");  // Service Identification
    addUAPItem(pUAP, 3, "070");  // Time Of Track Information
    addUAPItem(pUAP, 4, "105");  // Calculated Track Position (WGS-84)
    addUAPItem(pUAP, 5, "100");  // Calculated Track Position (Cartesian)
    addUAPItem(pUAP, 6, "185");  // Calculated Track Velocity (Cartesian)
    addUAPItem(pUAP, 7, "210");  // Calculated Acceleration (Cartesian)

    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(1), "010");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(2), "015");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(3), "070");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(4), "105");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(5), "100");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(6), "185");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(7), "210");
}

/**
 * Test Case: TC-CPP-UAP-014
 * Requirement: REQ-LLR-UAP-002
 * Description: Verify UAP with FX (Field Extension) items
 */
TEST_F(UAPTest, UAPWithFXItems) {
    pUAP = new UAP();

    // Add items with FX flags (Field Extension indicators)
    addUAPItem(pUAP, 1, "010", false);
    addUAPItem(pUAP, 2, "020", false);
    addUAPItem(pUAP, 3, "040", false);
    addUAPItem(pUAP, 4, "070", false);
    addUAPItem(pUAP, 5, "090", false);
    addUAPItem(pUAP, 6, "130", false);
    addUAPItem(pUAP, 7, "", true);  // FX bit - no item ID

    EXPECT_EQ(pUAP->m_lUAPItems.size(), 7);

    // FX item should return empty string
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(7), "");
}

/**
 * Test Case: TC-CPP-UAP-015
 * Requirement: REQ-LLR-UAP-003
 * Description: Verify getDataItemIDByUAPfrn with large FRN values
 */
TEST_F(UAPTest, GetDataItemIDByLargeFRN) {
    pUAP = new UAP();

    // Add items with large FRN values (extended FSPEC)
    addUAPItem(pUAP, 14, "250");  // FRN 14
    addUAPItem(pUAP, 21, "260");  // FRN 21
    addUAPItem(pUAP, 28, "270");  // FRN 28

    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(14), "250");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(21), "260");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(28), "270");
}

/**
 * Test Case: TC-CPP-UAP-016
 * Requirement: REQ-LLR-UAP-005
 * Description: Verify getDataItemIDByUAPfrn with negative FRN
 */
TEST_F(UAPTest, GetDataItemIDByNegativeFRN) {
    pUAP = new UAP();

    addUAPItem(pUAP, 1, "010");
    addUAPItem(pUAP, 2, "020");

    // Negative FRN should not match
    std::string result = pUAP->getDataItemIDByUAPfrn(-1);
    EXPECT_EQ(result, "");
}

/**
 * Test Case: TC-CPP-UAP-017
 * Requirement: REQ-LLR-UAP-001
 * Description: Verify UAP can be created and destroyed multiple times
 */
TEST_F(UAPTest, MultipleCreateDestroyCycles) {
    for (int i = 0; i < 10; i++) {
        pUAP = new UAP();
        addUAPItem(pUAP, 1, "010");
        addUAPItem(pUAP, 2, "020");

        EXPECT_EQ(pUAP->m_lUAPItems.size(), 2);

        delete pUAP;
        pUAP = nullptr;
    }

    SUCCEED();
}

/**
 * Test Case: TC-CPP-UAP-018
 * Requirement: REQ-LLR-UAP-002
 * Description: Verify UAP with maximum typical FRN count (3 FSPEC octets = 21 FRNs)
 */
TEST_F(UAPTest, UAPWithMaximumTypicalFRNs) {
    pUAP = new UAP();

    // Add 21 items (3 FSPEC octets worth)
    for (int i = 1; i <= 21; i++) {
        char itemId[16];
        snprintf(itemId, sizeof(itemId), "%03d", i * 10);
        addUAPItem(pUAP, i, itemId);
    }

    EXPECT_EQ(pUAP->m_lUAPItems.size(), 21);

    // Verify we can retrieve all items
    for (int i = 1; i <= 21; i++) {
        std::string result = pUAP->getDataItemIDByUAPfrn(i);
        EXPECT_FALSE(result.empty());
    }
}

/**
 * Test Case: TC-CPP-UAP-019
 * Requirement: REQ-LLR-UAP-004
 * Description: Verify UAP with all conditional fields set to zero (default UAP)
 */
TEST_F(UAPTest, DefaultUAPConditionals) {
    pUAP = new UAP();

    // Default UAP (UseIfBitSet = 0 means always use)
    pUAP->m_nUseIfBitSet = 0;
    pUAP->m_nUseIfByteNr = 0;
    pUAP->m_nIsSetTo = 0;

    addUAPItem(pUAP, 1, "010");

    EXPECT_EQ(pUAP->m_nUseIfBitSet, 0);
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(1), "010");
}

/**
 * Test Case: TC-CPP-UAP-020
 * Requirement: REQ-LLR-UAP-003
 * Description: Verify getDataItemIDByUAPfrn with sparse FRN mapping
 */
TEST_F(UAPTest, SparseUAPMapping) {
    pUAP = new UAP();

    // Non-contiguous FRNs
    addUAPItem(pUAP, 1, "010");
    addUAPItem(pUAP, 5, "050");
    addUAPItem(pUAP, 10, "100");
    addUAPItem(pUAP, 20, "200");

    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(1), "010");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(5), "050");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(10), "100");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(20), "200");

    // Non-existent FRNs should return empty
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(2), "");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(3), "");
    EXPECT_EQ(pUAP->getDataItemIDByUAPfrn(15), "");
}

/**
 * Test Case: TC-CPP-UAP-021
 * Requirement: REQ-LLR-UAP-002
 * Description: Verify UAP item list can be iterated
 */
TEST_F(UAPTest, UAPItemListIteration) {
    pUAP = new UAP();

    addUAPItem(pUAP, 1, "010");
    addUAPItem(pUAP, 2, "020");
    addUAPItem(pUAP, 3, "040");

    int count = 0;
    for (std::list<UAPItem*>::iterator it = pUAP->m_lUAPItems.begin();
         it != pUAP->m_lUAPItems.end(); ++it) {
        EXPECT_NE(*it, nullptr);
        count++;
    }

    EXPECT_EQ(count, 3);
}

/**
 * Test Case: TC-CPP-UAP-022
 * Requirement: REQ-LLR-UAP-001
 * Description: Verify empty UAP destructor doesn't crash
 */
TEST_F(UAPTest, EmptyUAPDestruction) {
    pUAP = new UAP();

    EXPECT_TRUE(pUAP->m_lUAPItems.empty());

    delete pUAP;
    pUAP = nullptr;

    SUCCEED();
}
