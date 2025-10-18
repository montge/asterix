/**
 * Unit tests for DataItemBits class - PUBLIC methods only
 *
 * These tests target the 4 UNCOVERED PUBLIC functions in DataItemBits.cpp
 * to improve coverage from 43.4% toward 60% milestone.
 *
 * Uncovered PUBLIC functions tested:
 * 1. getLength() - line 89
 * 2. printDescriptors() - line 831
 * 3. filterOutItem() - line 851
 * 4. getDescription() - line 859
 *
 * Note: getHexBitStringFullByte() and getHexBitStringMask() are PRIVATE
 * and cannot be tested directly (they must be tested through public API).
 *
 * Requirements Coverage:
 * - REQ-LLR-BITS-001: Bit extraction and manipulation
 * - REQ-LLR-BITS-003: Descriptor management
 *
 * Test Cases:
 * - TC-CPP-BITS-001: Test getLength() (error path)
 * - TC-CPP-BITS-002: Test printDescriptors() (descriptor formatting)
 * - TC-CPP-BITS-003: Test printDescriptors() with filtering
 * - TC-CPP-BITS-004: Test filterOutItem() (name matching)
 * - TC-CPP-BITS-005: Test filterOutItem() (prefix matching)
 * - TC-CPP-BITS-006: Test getDescription() (field lookup)
 * - TC-CPP-BITS-007: Test getDescription() with value lookup
 */

#include <gtest/gtest.h>
#include "../../src/asterix/DataItemFormat.h"
#include "../../src/asterix/DataItemBits.h"
#include "../../src/asterix/Tracer.h"
#include <cstring>

// Global variables required by ASTERIX library
bool gVerbose = false;
bool gFiltering = false;

class DataItemBitsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure clean state
        gVerbose = false;
        gFiltering = false;
    }

    void TearDown() override {
        gFiltering = false;
    }
};

/**
 * Test Case: TC-CPP-BITS-001
 * Requirement: REQ-LLR-BITS-001
 *
 * Test getLength() - This function should not be called directly
 * (it logs an error and returns 0), but we test it for coverage.
 */
TEST_F(DataItemBitsTest, GetLengthReturnsZero) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_strName = "System Area Code";

    unsigned char data[10] = {0x12, 0x34, 0x56, 0x78, 0x9A};

    // getLength() should return 0 and log an error
    // (This is an error path - the function shouldn't be called)
    long length = bits.getLength(data);

    EXPECT_EQ(length, 0) << "getLength() should return 0 (error path)";
}

/**
 * Test Case: TC-CPP-BITS-002
 * Requirement: REQ-LLR-BITS-003
 *
 * Test printDescriptors() - Formats descriptor string with name
 */
TEST_F(DataItemBitsTest, PrintDescriptors) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_strName = "System Area Code";
    bits.m_bFiltered = false;

    // Print descriptors with header
    std::string result = bits.printDescriptors("  ");

    EXPECT_GT(result.length(), 0) << "Descriptor should not be empty";
    EXPECT_NE(result.find("SAC"), std::string::npos) << "Should contain short name 'SAC'";
    EXPECT_NE(result.find("System Area Code"), std::string::npos) << "Should contain full name";
    EXPECT_NE(result.find("\n"), std::string::npos) << "Should end with newline";
}

/**
 * Test Case: TC-CPP-BITS-003
 * Requirement: REQ-LLR-BITS-003
 *
 * Test printDescriptors() with filtering enabled
 */
TEST_F(DataItemBitsTest, PrintDescriptorsWithFiltering) {
    // Test with filtering enabled but item not filtered (should prefix with '#')
    {
        DataItemBits bits(8);
        bits.m_strShortName = "SIC";
        bits.m_strName = "System Identification Code";
        bits.m_bFiltered = false;

        gFiltering = true;

        std::string result = bits.printDescriptors("  ");
        EXPECT_GT(result.length(), 0) << "Result should not be empty";
        EXPECT_NE(result.find("#"), std::string::npos)
            << "Should contain '#' when filtering enabled but item not filtered";

        gFiltering = false;
    }

    // Test with filtering enabled and item filtered (should NOT prefix with '#')
    {
        DataItemBits bits2(8);
        bits2.m_strShortName = "SIC";
        bits2.m_strName = "System Identification Code";
        bits2.m_bFiltered = true;

        gFiltering = true;

        std::string result2 = bits2.printDescriptors("  ");
        EXPECT_GT(result2.length(), 0) << "Result should not be empty";
        // When m_bFiltered is true, no '#' prefix
        size_t hashPos = result2.find("#");
        bool hasHashAtStart = (hashPos == 0 || (hashPos == 2)); // After "  " header
        EXPECT_FALSE(hasHashAtStart || result2.find("#  ") != std::string::npos)
            << "Should NOT contain '#' prefix when item is filtered";

        gFiltering = false;
    }
}

/**
 * Test Case: TC-CPP-BITS-004
 * Requirement: REQ-LLR-BITS-003
 *
 * Test filterOutItem() - Filters items by exact name match
 */
TEST_F(DataItemBitsTest, FilterOutItemExactMatch) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_bFiltered = false;

    // Should match when names match exactly
    bool filtered = bits.filterOutItem("SAC");

    EXPECT_TRUE(filtered) << "Should return true for matching name";
    EXPECT_TRUE(bits.m_bFiltered) << "Filtered flag should be set to true";
}

/**
 * Test Case: TC-CPP-BITS-005
 * Requirement: REQ-LLR-BITS-003
 *
 * Test filterOutItem() - No match for different name
 */
TEST_F(DataItemBitsTest, FilterOutItemNoMatch) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_bFiltered = false;

    // Should not match different name
    bool filtered = bits.filterOutItem("SIC");

    EXPECT_FALSE(filtered) << "Should return false for non-matching name";
    EXPECT_FALSE(bits.m_bFiltered) << "Filtered flag should remain false";
}

/**
 * Test Case: TC-CPP-BITS-006
 * Requirement: REQ-LLR-BITS-003
 *
 * Test filterOutItem() - Prefix match (uses strncmp)
 */
TEST_F(DataItemBitsTest, FilterOutItemPrefixMatch) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_bFiltered = false;

    // Test prefix match (strncmp compares up to length of m_strShortName)
    // "SAC/SIC" starts with "SAC" so should match
    bool filtered = bits.filterOutItem("SAC/SIC");

    EXPECT_TRUE(filtered) << "Should match prefix 'SAC' in 'SAC/SIC'";
    EXPECT_TRUE(bits.m_bFiltered) << "Filtered flag should be set";
}

/**
 * Test Case: TC-CPP-BITS-007
 * Requirement: REQ-LLR-BITS-003
 *
 * Test getDescription() - Returns description for matching field (no value)
 */
TEST_F(DataItemBitsTest, GetDescriptionFieldOnly) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_strName = "System Area Code";

    // Get description for matching field (NULL value means return field description)
    const char* desc = bits.getDescription("SAC", NULL);

    ASSERT_NE(desc, nullptr) << "Description should not be NULL for matching field";
    EXPECT_STREQ(desc, "System Area Code") << "Should return full name as description";
}

/**
 * Test Case: TC-CPP-BITS-008
 * Requirement: REQ-LLR-BITS-003
 *
 * Test getDescription() - Returns NULL for non-matching field
 */
TEST_F(DataItemBitsTest, GetDescriptionNoMatch) {
    DataItemBits bits(8);
    bits.m_strShortName = "SAC";
    bits.m_strName = "System Area Code";

    // Get description for non-matching field
    const char* desc = bits.getDescription("SIC", NULL);

    EXPECT_EQ(desc, nullptr) << "Should return NULL for non-matching field";
}

/**
 * Test Case: TC-CPP-BITS-009
 * Requirement: REQ-LLR-BITS-003
 *
 * Test getDescription() - With empty names (copies short to full or vice versa)
 */
TEST_F(DataItemBitsTest, GetDescriptionEmptyNames) {
    // Test with short name only (full name empty)
    {
        DataItemBits bits(8);
        bits.m_strShortName = "SAC";
        bits.m_strName = "";

        const char* desc = bits.getDescription("SAC", NULL);
        ASSERT_NE(desc, nullptr);
        // After getDescription is called, m_strName should be copied from m_strShortName
        EXPECT_STREQ(desc, "SAC") << "Should use short name when full name empty";
    }

    // Test with full name only (short name empty)
    {
        DataItemBits bits2(8);
        bits2.m_strShortName = "";
        bits2.m_strName = "System Area Code";

        const char* desc2 = bits2.getDescription("System Area Code", NULL);
        ASSERT_NE(desc2, nullptr);
        EXPECT_STREQ(desc2, "System Area Code");
    }
}

/**
 * Test Case: TC-CPP-BITS-010
 * Requirement: REQ-LLR-BITS-003
 *
 * Test getDescription() - With value parameter (looks up BitsValue descriptions)
 */
TEST_F(DataItemBitsTest, GetDescriptionWithValue) {
    DataItemBits bits(8);
    bits.m_strShortName = "TYP";
    bits.m_strName = "Target Type";

    // Add a BitsValue for testing (value=1, description="Primary Target")
    BitsValue *bv = new BitsValue(1, "Primary Target");
    bits.m_lValue.push_back(bv);

    // Get description for field with specific value
    // Note: getDescription compares string value "1" against m_nVal converted to string
    const char* desc = bits.getDescription("TYP", "1");

    ASSERT_NE(desc, nullptr) << "Should find description for value '1'";
    EXPECT_STREQ(desc, "Primary Target") << "Should return value description";
}

/**
 * Test Case: TC-CPP-BITS-011
 * Requirement: REQ-LLR-BITS-003
 *
 * Test getDescription() - Value not found returns NULL
 */
TEST_F(DataItemBitsTest, GetDescriptionValueNotFound) {
    DataItemBits bits(8);
    bits.m_strShortName = "TYP";
    bits.m_strName = "Target Type";

    // Add a BitsValue for testing (value=1)
    BitsValue *bv = new BitsValue(1, "Primary Target");
    bits.m_lValue.push_back(bv);

    // Get description for non-existent value (999)
    const char* desc = bits.getDescription("TYP", "999");

    EXPECT_EQ(desc, nullptr) << "Should return NULL for non-existent value";
}

// Run all tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
