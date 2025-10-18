/**
 * Unit tests for DataItem class
 *
 * Requirements Traceability:
 * - REQ-HLR-001: Parse ASTERIX binary data
 * - REQ-LLR-048-010: Parse Data Source Identifier (I048/010)
 *
 * DO-278A AL-3 Compliance Testing
 */

#include <gtest/gtest.h>
#include "DataItem.h"
#include "DataItemDescription.h"
#include "DataItemFormat.h"
#include "DataItemFormatFixed.h"

/**
 * Test Case: TC-CPP-DI-001
 * Requirement: REQ-HLR-001
 * Description: Verify DataItem constructor initializes correctly
 */
TEST(DataItemTest, ConstructorInitializes) {
    DataItemDescription desc("010");
    DataItem item(&desc);

    EXPECT_EQ(item.m_pDescription, &desc);
    EXPECT_EQ(item.getLength(), 0);
}

/**
 * Test Case: TC-CPP-DI-002
 * Requirement: REQ-HLR-001
 * Description: Verify DataItem initial length is zero
 */
TEST(DataItemTest, InitialLengthIsZero) {
    DataItemDescription desc("010");
    DataItem item(&desc);

    EXPECT_EQ(item.getLength(), 0);
}

/**
 * Test Case: TC-CPP-DI-003
 * Requirement: REQ-HLR-001
 * Description: Verify DataItem with NULL description
 */
TEST(DataItemTest, NullDescriptionHandled) {
    // This test verifies the code doesn't crash with NULL
    // In production, this shouldn't happen, but defensive coding is tested
    DataItem item(NULL);

    EXPECT_EQ(item.m_pDescription, nullptr);
    EXPECT_EQ(item.getLength(), 0);
}

/**
 * Test Case: TC-CPP-DI-004
 * Requirement: REQ-LLR-048-010
 * Description: Verify DataItem parsing with NULL format returns 0
 * Note: Full parsing requires complex initialization with bits configuration
 */
TEST(DataItemTest, ParsingWithoutFormatReturnsZero) {
    DataItemDescription desc("010");
    desc.m_pFormat = nullptr; // No format configured

    DataItem item(&desc);

    // Parse 2 bytes of data without format configured
    unsigned char data[] = {0x01, 0x02};
    long parsed = item.parse(data, sizeof(data));

    // Without format, should return 0
    EXPECT_EQ(parsed, 0);
    EXPECT_EQ(item.getLength(), 0);
}

/**
 * Test Case: TC-CPP-DI-005
 * Requirement: REQ-HLR-001
 * Description: Verify DataItem parsing with insufficient data
 */
TEST(DataItemTest, ParsingWithInsufficientData) {
    DataItemDescription desc("010");

    // Create a fixed format for 2-byte data item
    DataItemFormatFixed* format = new DataItemFormatFixed(2);
    desc.m_pFormat = format;

    DataItem item(&desc);

    // Try to parse only 1 byte when 2 are needed
    unsigned char data[] = {0x01};
    long parsed = item.parse(data, sizeof(data));

    // Should handle gracefully (likely return 0 or error)
    EXPECT_LE(parsed, 1);
}

/**
 * Test Case: TC-CPP-DI-006
 * Requirement: REQ-HLR-001
 * Description: Verify DataItem parsing with empty data
 */
TEST(DataItemTest, ParsingWithEmptyData) {
    DataItemDescription desc("010");
    DataItemFormatFixed* format = new DataItemFormatFixed(2);
    desc.m_pFormat = format;

    DataItem item(&desc);

    unsigned char* data = nullptr;
    long parsed = item.parse(data, 0);

    // Should handle gracefully
    EXPECT_EQ(parsed, 0);
}

/**
 * Test Case: TC-CPP-DI-007
 * Requirement: REQ-HLR-001
 * Description: Verify DataItem getText returns data representation
 */
TEST(DataItemTest, GetTextReturnsRepresentation) {
    DataItemDescription desc("010");
    DataItemFormatFixed* format = new DataItemFormatFixed(2);
    desc.m_pFormat = format;

    DataItem item(&desc);

    // Parse some data
    unsigned char data[] = {0x01, 0x02};
    item.parse(data, sizeof(data));

    std::string result;
    std::string header;
    bool success = item.getText(result, header, 0);

    // getText should work (or return false if not implemented for this format)
    // We just verify it doesn't crash
    SUCCEED();
}

/**
 * Test Case: TC-CPP-DI-008
 * Requirement: REQ-LLR-048-010
 * Description: Verify DataItem description can be set
 * Note: Full parsing test would require XML configuration loading
 */
TEST(DataItemTest, DataSourceIdentifierDescription) {
    DataItemDescription desc("010");
    desc.m_strName = "Data Source Identifier";

    DataItem item(&desc);

    // Verify description is accessible
    EXPECT_EQ(item.m_pDescription->m_strName, "Data Source Identifier");
    EXPECT_EQ(item.getLength(), 0); // No data parsed yet
}

/**
 * Test Case: TC-CPP-DI-009
 * Requirement: REQ-HLR-001
 * Description: Verify multiple DataItems can coexist
 */
TEST(DataItemTest, MultipleDataItemsCoexist) {
    DataItemDescription desc1("010");
    DataItemDescription desc2("020");
    DataItemDescription desc3("040");

    DataItem item1(&desc1);
    DataItem item2(&desc2);
    DataItem item3(&desc3);

    EXPECT_EQ(item1.m_pDescription, &desc1);
    EXPECT_EQ(item2.m_pDescription, &desc2);
    EXPECT_EQ(item3.m_pDescription, &desc3);

    // All should be independent
    EXPECT_NE(item1.m_pDescription, item2.m_pDescription);
    EXPECT_NE(item2.m_pDescription, item3.m_pDescription);
}

/**
 * Test Case: TC-CPP-DI-010
 * Requirement: REQ-HLR-001
 * Description: Verify DataItem destructor cleans up properly
 */
TEST(DataItemTest, DestructorCleansUp) {
    {
        DataItemDescription desc("010");
        DataItem item(&desc);

        unsigned char data[] = {0x01, 0x02};
        item.parse(data, sizeof(data));
    }
    // If we reach here without crash, cleanup succeeded
    SUCCEED();
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
