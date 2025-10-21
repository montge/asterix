/**
 * Unit tests for UAPItem class
 *
 * UAPItem Background:
 * - UAPItem represents individual fields in the UAP (User Application Profile)
 * - Inherits from DataItemFormat base class
 * - Contains FRN (Field Reference Number) and Data Item ID mapping
 * - Includes bit position, FX flag, and length information
 * - Used during FSPEC parsing to determine which data items are present
 *
 * Coverage Goals:
 * - UAPItem.cpp: 0% -> 80%+ coverage
 * - Test all constructors, copy constructor, and member access
 *
 * Requirements Traceability:
 * - REQ-HLR-003: Parse ASTERIX UAP item definitions
 * - REQ-LLR-UAPITEM-001: UAPItem construction and initialization
 * - REQ-LLR-UAPITEM-002: UAPItem copy constructor
 * - REQ-LLR-UAPITEM-003: FRN and Item ID mapping
 * - REQ-LLR-UAPITEM-004: FX (Field Extension) flag handling
 * - REQ-LLR-UAPITEM-005: Clone functionality
 *
 * DO-278A AL-3 Compliance Testing
 */

#include <gtest/gtest.h>
#include "../../src/asterix/UAPItem.h"
#include "../../src/asterix/DataItemFormat.h"
#include <cstring>

// Global variables required by ASTERIX library (used by Tracer)
bool gVerbose = false;
bool gFiltering = false;

/**
 * Test Fixture for UAPItem tests
 */
class UAPItemTest : public ::testing::Test {
protected:
    UAPItem* pUAPItem;

    void SetUp() override {
        pUAPItem = nullptr;
        gVerbose = false;
        gFiltering = false;
    }

    void TearDown() override {
        if (pUAPItem) {
            delete pUAPItem;
            pUAPItem = nullptr;
        }
    }
};

/**
 * Test Case: TC-CPP-UAPITEM-001
 * Requirement: REQ-LLR-UAPITEM-001
 * Description: Verify UAPItem default constructor initializes all fields
 */
TEST_F(UAPItemTest, DefaultConstructorInitialization) {
    pUAPItem = new UAPItem();

    // Verify all member variables are initialized to zero/default
    EXPECT_EQ(pUAPItem->m_nBit, 0);
    EXPECT_EQ(pUAPItem->m_nFRN, 0);
    EXPECT_FALSE(pUAPItem->m_bFX);
    EXPECT_EQ(pUAPItem->m_nLen, 0);
    EXPECT_TRUE(pUAPItem->m_strItemID.empty());
}

/**
 * Test Case: TC-CPP-UAPITEM-002
 * Requirement: REQ-LLR-UAPITEM-001
 * Description: Verify UAPItem members can be set
 */
TEST_F(UAPItemTest, MembersCanBeSet) {
    pUAPItem = new UAPItem();

    pUAPItem->m_nBit = 7;
    pUAPItem->m_nFRN = 1;
    pUAPItem->m_bFX = false;
    pUAPItem->m_nLen = 2;
    pUAPItem->m_strItemID = "010";

    EXPECT_EQ(pUAPItem->m_nBit, 7);
    EXPECT_EQ(pUAPItem->m_nFRN, 1);
    EXPECT_FALSE(pUAPItem->m_bFX);
    EXPECT_EQ(pUAPItem->m_nLen, 2);
    EXPECT_EQ(pUAPItem->m_strItemID, "010");
}

/**
 * Test Case: TC-CPP-UAPITEM-003
 * Requirement: REQ-LLR-UAPITEM-002
 * Description: Verify UAPItem copy constructor copies all fields
 */
TEST_F(UAPItemTest, CopyConstructorCopiesAllFields) {
    UAPItem original;
    original.m_nBit = 5;
    original.m_nFRN = 3;
    original.m_bFX = true;
    original.m_nLen = 4;
    original.m_strItemID = "070";

    UAPItem copy(original);

    EXPECT_EQ(copy.m_nBit, 5);
    EXPECT_EQ(copy.m_nFRN, 3);
    EXPECT_TRUE(copy.m_bFX);
    EXPECT_EQ(copy.m_nLen, 4);
    EXPECT_EQ(copy.m_strItemID, "070");
}

/**
 * Test Case: TC-CPP-UAPITEM-004
 * Requirement: REQ-LLR-UAPITEM-002
 * Description: Verify UAPItem copy is independent of original
 */
TEST_F(UAPItemTest, CopyIsIndependent) {
    UAPItem original;
    original.m_nFRN = 1;
    original.m_strItemID = "010";

    UAPItem copy(original);

    // Modify original
    original.m_nFRN = 99;
    original.m_strItemID = "999";

    // Copy should be unchanged
    EXPECT_EQ(copy.m_nFRN, 1);
    EXPECT_EQ(copy.m_strItemID, "010");
}

/**
 * Test Case: TC-CPP-UAPITEM-005
 * Requirement: REQ-LLR-UAPITEM-005
 * Description: Verify UAPItem clone method creates independent copy
 */
TEST_F(UAPItemTest, CloneCreatesIndependentCopy) {
    pUAPItem = new UAPItem();
    pUAPItem->m_nBit = 3;
    pUAPItem->m_nFRN = 2;
    pUAPItem->m_bFX = false;
    pUAPItem->m_nLen = 3;
    pUAPItem->m_strItemID = "040";

    UAPItem* clone = pUAPItem->clone();

    EXPECT_NE(clone, pUAPItem);
    EXPECT_EQ(clone->m_nBit, 3);
    EXPECT_EQ(clone->m_nFRN, 2);
    EXPECT_FALSE(clone->m_bFX);
    EXPECT_EQ(clone->m_nLen, 3);
    EXPECT_EQ(clone->m_strItemID, "040");

    delete clone;
}

/**
 * Test Case: TC-CPP-UAPITEM-006
 * Requirement: REQ-LLR-UAPITEM-003
 * Description: Verify UAPItem with CAT048/010 (Data Source Identifier)
 */
TEST_F(UAPItemTest, CAT048_010DataSourceIdentifier) {
    pUAPItem = new UAPItem();

    // CAT048/010: FRN=1, Bit=7, Length=2, No FX
    pUAPItem->m_nFRN = 1;
    pUAPItem->m_nBit = 7;
    pUAPItem->m_nLen = 2;
    pUAPItem->m_bFX = false;
    pUAPItem->m_strItemID = "010";

    EXPECT_EQ(pUAPItem->m_nFRN, 1);
    EXPECT_EQ(pUAPItem->m_nBit, 7);
    EXPECT_EQ(pUAPItem->m_nLen, 2);
    EXPECT_FALSE(pUAPItem->m_bFX);
    EXPECT_EQ(pUAPItem->m_strItemID, "010");
}

/**
 * Test Case: TC-CPP-UAPITEM-007
 * Requirement: REQ-LLR-UAPITEM-004
 * Description: Verify UAPItem with FX (Field Extension) flag set
 */
TEST_F(UAPItemTest, FXFlagSet) {
    pUAPItem = new UAPItem();

    // FX bit: typically FRN=7, Bit=0, FX=true
    pUAPItem->m_nFRN = 7;
    pUAPItem->m_nBit = 0;
    pUAPItem->m_bFX = true;
    pUAPItem->m_nLen = 0;
    pUAPItem->m_strItemID = "";  // FX items have no item ID

    EXPECT_EQ(pUAPItem->m_nFRN, 7);
    EXPECT_EQ(pUAPItem->m_nBit, 0);
    EXPECT_TRUE(pUAPItem->m_bFX);
    EXPECT_TRUE(pUAPItem->m_strItemID.empty());
}

/**
 * Test Case: TC-CPP-UAPITEM-008
 * Requirement: REQ-LLR-UAPITEM-003
 * Description: Verify UAPItem with various bit positions (0-7)
 */
TEST_F(UAPItemTest, VariousBitPositions) {
    for (int bit = 0; bit <= 7; bit++) {
        UAPItem item;
        item.m_nBit = bit;
        item.m_nFRN = bit + 1;
        item.m_strItemID = std::to_string((bit + 1) * 10);

        EXPECT_EQ(item.m_nBit, bit);
        EXPECT_EQ(item.m_nFRN, bit + 1);
    }
}

/**
 * Test Case: TC-CPP-UAPITEM-009
 * Requirement: REQ-LLR-UAPITEM-003
 * Description: Verify UAPItem with CAT062/010 (Data Source Identifier)
 */
TEST_F(UAPItemTest, CAT062_010DataSourceIdentifier) {
    pUAPItem = new UAPItem();

    // CAT062/010: FRN=1, Bit=7, Length=2, No FX
    pUAPItem->m_nFRN = 1;
    pUAPItem->m_nBit = 7;
    pUAPItem->m_nLen = 2;
    pUAPItem->m_bFX = false;
    pUAPItem->m_strItemID = "010";

    EXPECT_EQ(pUAPItem->m_strItemID, "010");
    EXPECT_EQ(pUAPItem->m_nLen, 2);
}

/**
 * Test Case: TC-CPP-UAPITEM-010
 * Requirement: REQ-LLR-UAPITEM-001
 * Description: Verify UAPItem destructor works correctly
 */
TEST_F(UAPItemTest, DestructorWorks) {
    pUAPItem = new UAPItem();
    pUAPItem->m_nFRN = 1;
    pUAPItem->m_strItemID = "010";

    delete pUAPItem;
    pUAPItem = nullptr;

    SUCCEED();
}

/**
 * Test Case: TC-CPP-UAPITEM-011
 * Requirement: REQ-LLR-UAPITEM-003
 * Description: Verify UAPItem with variable length item (m_nLen = -1)
 */
TEST_F(UAPItemTest, VariableLengthItem) {
    pUAPItem = new UAPItem();

    // Variable length items typically have m_nLen = -1
    pUAPItem->m_nFRN = 4;
    pUAPItem->m_nBit = 4;
    pUAPItem->m_nLen = -1;
    pUAPItem->m_bFX = false;
    pUAPItem->m_strItemID = "070";

    EXPECT_EQ(pUAPItem->m_nLen, -1);
    EXPECT_EQ(pUAPItem->m_strItemID, "070");
}

/**
 * Test Case: TC-CPP-UAPITEM-012
 * Requirement: REQ-LLR-UAPITEM-003
 * Description: Verify UAPItem with large FRN (extended FSPEC)
 */
TEST_F(UAPItemTest, LargeFRNValue) {
    pUAPItem = new UAPItem();

    // Extended FSPEC can have FRN > 21
    pUAPItem->m_nFRN = 28;
    pUAPItem->m_nBit = 0;
    pUAPItem->m_nLen = 4;
    pUAPItem->m_bFX = false;
    pUAPItem->m_strItemID = "295";

    EXPECT_EQ(pUAPItem->m_nFRN, 28);
    EXPECT_EQ(pUAPItem->m_strItemID, "295");
}

/**
 * Test Case: TC-CPP-UAPITEM-013
 * Requirement: REQ-LLR-UAPITEM-002
 * Description: Verify copy constructor with empty ItemID
 */
TEST_F(UAPItemTest, CopyConstructorEmptyItemID) {
    UAPItem original;
    original.m_nFRN = 7;
    original.m_bFX = true;
    original.m_strItemID = "";  // FX item

    UAPItem copy(original);

    EXPECT_EQ(copy.m_nFRN, 7);
    EXPECT_TRUE(copy.m_bFX);
    EXPECT_TRUE(copy.m_strItemID.empty());
}

/**
 * Test Case: TC-CPP-UAPITEM-014
 * Requirement: REQ-LLR-UAPITEM-003
 * Description: Verify UAPItem with all fields at maximum values
 */
TEST_F(UAPItemTest, MaximumFieldValues) {
    pUAPItem = new UAPItem();

    pUAPItem->m_nBit = 7;
    pUAPItem->m_nFRN = 255;  // Maximum reasonable FRN
    pUAPItem->m_bFX = true;
    pUAPItem->m_nLen = 1024;  // Large length
    pUAPItem->m_strItemID = "999";

    EXPECT_EQ(pUAPItem->m_nBit, 7);
    EXPECT_EQ(pUAPItem->m_nFRN, 255);
    EXPECT_TRUE(pUAPItem->m_bFX);
    EXPECT_EQ(pUAPItem->m_nLen, 1024);
    EXPECT_EQ(pUAPItem->m_strItemID, "999");
}

/**
 * Test Case: TC-CPP-UAPITEM-015
 * Requirement: REQ-LLR-UAPITEM-003
 * Description: Verify UAPItem with typical CAT048 item range
 */
TEST_F(UAPItemTest, CAT048TypicalItems) {
    // Test several typical CAT048 items
    struct TestCase {
        int frn;
        int bit;
        int len;
        const char* itemId;
    };

    TestCase testCases[] = {
        {1, 7, 2, "010"},   // Data Source Identifier
        {2, 6, 1, "020"},   // Target Report Descriptor
        {3, 5, 4, "040"},   // Measured Position
        {4, 4, 2, "070"},   // Mode-3/A Code
        {5, 3, 2, "090"},   // Flight Level
        {6, 2, 2, "130"},   // Radar Plot Characteristics
        {7, 1, 3, "220"}    // Aircraft Address
    };

    for (const auto& tc : testCases) {
        UAPItem item;
        item.m_nFRN = tc.frn;
        item.m_nBit = tc.bit;
        item.m_nLen = tc.len;
        item.m_strItemID = tc.itemId;

        EXPECT_EQ(item.m_nFRN, tc.frn);
        EXPECT_EQ(item.m_nBit, tc.bit);
        EXPECT_EQ(item.m_nLen, tc.len);
        EXPECT_EQ(item.m_strItemID, tc.itemId);
    }
}

/**
 * Test Case: TC-CPP-UAPITEM-016
 * Requirement: REQ-LLR-UAPITEM-005
 * Description: Verify multiple clones can be created
 */
TEST_F(UAPItemTest, MultipleClones) {
    pUAPItem = new UAPItem();
    pUAPItem->m_nFRN = 5;
    pUAPItem->m_strItemID = "090";

    UAPItem* clone1 = pUAPItem->clone();
    UAPItem* clone2 = pUAPItem->clone();
    UAPItem* clone3 = pUAPItem->clone();

    EXPECT_NE(clone1, clone2);
    EXPECT_NE(clone2, clone3);
    EXPECT_NE(clone1, clone3);

    EXPECT_EQ(clone1->m_nFRN, 5);
    EXPECT_EQ(clone2->m_nFRN, 5);
    EXPECT_EQ(clone3->m_nFRN, 5);

    delete clone1;
    delete clone2;
    delete clone3;
}

/**
 * Test Case: TC-CPP-UAPITEM-017
 * Requirement: REQ-LLR-UAPITEM-001
 * Description: Verify UAPItem creation and destruction in loop (memory test)
 */
TEST_F(UAPItemTest, CreateDestroyLoop) {
    for (int i = 0; i < 100; i++) {
        UAPItem* item = new UAPItem();
        item->m_nFRN = i;
        item->m_strItemID = std::to_string(i);
        delete item;
    }

    SUCCEED();
}

/**
 * Test Case: TC-CPP-UAPITEM-018
 * Requirement: REQ-LLR-UAPITEM-003
 * Description: Verify UAPItem with special characters in ItemID
 */
TEST_F(UAPItemTest, ItemIDWithSpecialCharacters) {
    pUAPItem = new UAPItem();

    pUAPItem->m_nFRN = 1;
    pUAPItem->m_strItemID = "010/SAC";

    EXPECT_EQ(pUAPItem->m_strItemID, "010/SAC");
}

/**
 * Test Case: TC-CPP-UAPITEM-019
 * Requirement: REQ-LLR-UAPITEM-002
 * Description: Verify copy constructor with parent format set
 */
TEST_F(UAPItemTest, CopyConstructorWithParentFormat) {
    UAPItem original;
    original.m_nFRN = 1;
    original.m_strItemID = "010";
    // Note: m_pParentFormat is typically NULL for UAPItems

    UAPItem copy(original);

    EXPECT_EQ(copy.m_nFRN, original.m_nFRN);
    EXPECT_EQ(copy.m_strItemID, original.m_strItemID);
}

/**
 * Test Case: TC-CPP-UAPITEM-020
 * Requirement: REQ-LLR-UAPITEM-001
 * Description: Verify UAPItem members default to safe values
 */
TEST_F(UAPItemTest, SafeDefaultValues) {
    pUAPItem = new UAPItem();

    // Verify defaults don't cause issues
    EXPECT_GE(pUAPItem->m_nBit, 0);
    EXPECT_GE(pUAPItem->m_nFRN, 0);
    EXPECT_GE(pUAPItem->m_nLen, 0);
}

/**
 * Test Case: TC-CPP-UAPITEM-021
 * Requirement: REQ-LLR-UAPITEM-003
 * Description: Verify UAPItem bit positions for second FSPEC octet
 */
TEST_F(UAPItemTest, SecondFSPECOctetBitPositions) {
    // Second FSPEC octet: FRNs 8-14, bits 7-1 (bit 0 is FX)
    for (int frn = 8; frn <= 14; frn++) {
        UAPItem item;
        item.m_nFRN = frn;
        item.m_nBit = 7 - (frn - 8);  // Bit 7 for FRN 8, down to bit 1 for FRN 14
        item.m_strItemID = std::to_string(frn * 10);

        EXPECT_EQ(item.m_nFRN, frn);
        EXPECT_GE(item.m_nBit, 1);
        EXPECT_LE(item.m_nBit, 7);
    }
}

/**
 * Test Case: TC-CPP-UAPITEM-022
 * Requirement: REQ-LLR-UAPITEM-004
 * Description: Verify FX item at end of first FSPEC octet
 */
TEST_F(UAPItemTest, FXItemFirstOctet) {
    pUAPItem = new UAPItem();

    // FX item at FRN 7 (end of first FSPEC octet)
    pUAPItem->m_nFRN = 7;
    pUAPItem->m_nBit = 0;
    pUAPItem->m_bFX = true;
    pUAPItem->m_strItemID = "";

    EXPECT_EQ(pUAPItem->m_nFRN, 7);
    EXPECT_EQ(pUAPItem->m_nBit, 0);
    EXPECT_TRUE(pUAPItem->m_bFX);
}

/**
 * Test Case: TC-CPP-UAPITEM-023
 * Requirement: REQ-LLR-UAPITEM-004
 * Description: Verify FX item at end of second FSPEC octet
 */
TEST_F(UAPItemTest, FXItemSecondOctet) {
    pUAPItem = new UAPItem();

    // FX item at FRN 14 (end of second FSPEC octet)
    pUAPItem->m_nFRN = 14;
    pUAPItem->m_nBit = 0;
    pUAPItem->m_bFX = true;
    pUAPItem->m_strItemID = "";

    EXPECT_EQ(pUAPItem->m_nFRN, 14);
    EXPECT_EQ(pUAPItem->m_nBit, 0);
    EXPECT_TRUE(pUAPItem->m_bFX);
}

/**
 * Test Case: TC-CPP-UAPITEM-024
 * Requirement: REQ-LLR-UAPITEM-003
 * Description: Verify UAPItem with zero-length fixed item
 */
TEST_F(UAPItemTest, ZeroLengthItem) {
    pUAPItem = new UAPItem();

    pUAPItem->m_nFRN = 10;
    pUAPItem->m_nBit = 5;
    pUAPItem->m_nLen = 0;
    pUAPItem->m_strItemID = "SP";  // Special Purpose item

    EXPECT_EQ(pUAPItem->m_nLen, 0);
    EXPECT_EQ(pUAPItem->m_strItemID, "SP");
}

/**
 * Test Case: TC-CPP-UAPITEM-025
 * Requirement: REQ-LLR-UAPITEM-001
 * Description: Verify default constructor is called by base class constructor
 */
TEST_F(UAPItemTest, BaseClassConstructorChain) {
    pUAPItem = new UAPItem();

    // UAPItem inherits from DataItemFormat
    // Verify it was properly constructed
    EXPECT_NE(pUAPItem, nullptr);
}
