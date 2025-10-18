/**
 * Unit tests for Utils functions
 *
 * Requirements Traceability:
 * - REQ-HLR-002: Data integrity verification
 * - REQ-HLR-ERR-001: Validate data integrity
 *
 * DO-278A AL-3 Compliance Testing
 */

#include <gtest/gtest.h>
#include "Utils.h"
#include <cstring>

/**
 * Test Case: TC-CPP-UTILS-001
 * Requirement: REQ-HLR-ERR-001
 * Description: Verify CRC32 computation for empty data
 */
TEST(UtilsTest, CRC32EmptyData) {
    const char* data = "";
    uint32_t crc = crc32(data, 0);

    // CRC32 of empty data should be 0
    EXPECT_EQ(crc, 0);
}

/**
 * Test Case: TC-CPP-UTILS-002
 * Requirement: REQ-HLR-002
 * Description: Verify CRC32 computation for known data
 */
TEST(UtilsTest, CRC32KnownData) {
    const char* data = "123456789";
    uint32_t crc = crc32(data, strlen(data));

    // CRC32 of "123456789" should be 0xCBF43926 (standard test vector)
    EXPECT_EQ(crc, 0xCBF43926);
}

/**
 * Test Case: TC-CPP-UTILS-003
 * Requirement: REQ-HLR-002
 * Description: Verify CRC32 for ASTERIX-like binary data
 */
TEST(UtilsTest, CRC32BinaryData) {
    unsigned char data[] = {0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9};
    uint32_t crc = crc32(data, sizeof(data));

    // CRC should be deterministic
    EXPECT_NE(crc, 0);

    // Computing again should give same result
    uint32_t crc2 = crc32(data, sizeof(data));
    EXPECT_EQ(crc, crc2);
}

/**
 * Test Case: TC-CPP-UTILS-004
 * Requirement: REQ-HLR-002
 * Description: Verify CRC32 for different data produces different results
 */
TEST(UtilsTest, CRC32DifferentDataDifferentCRC) {
    const char* data1 = "test data 1";
    const char* data2 = "test data 2";

    uint32_t crc1 = crc32(data1, strlen(data1));
    uint32_t crc2 = crc32(data2, strlen(data2));

    EXPECT_NE(crc1, crc2);
}

/**
 * Test Case: TC-CPP-UTILS-005
 * Requirement: REQ-HLR-002
 * Description: Verify CRC32 with previous CRC (cumulative)
 */
TEST(UtilsTest, CRC32WithPreviousCRC) {
    const char* data1 = "part1";
    const char* data2 = "part2";

    // Compute CRC in two parts
    uint32_t crc1 = crc32(data1, strlen(data1));
    uint32_t crc_cumulative = crc32(data2, strlen(data2), crc1);

    // Compute CRC of combined data
    const char* combined = "part1part2";
    uint32_t crc_combined = crc32(combined, strlen(combined));

    // Should be the same
    EXPECT_EQ(crc_cumulative, crc_combined);
}

/**
 * Test Case: TC-CPP-UTILS-006
 * Requirement: REQ-HLR-ERR-001
 * Description: Verify CRC32 detects data corruption
 */
TEST(UtilsTest, CRC32DetectsCorruption) {
    unsigned char data[] = {0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9};
    uint32_t original_crc = crc32(data, sizeof(data));

    // Corrupt one byte
    data[4] ^= 0x01;
    uint32_t corrupted_crc = crc32(data, sizeof(data));

    // CRC should be different
    EXPECT_NE(original_crc, corrupted_crc);
}

/**
 * Test Case: TC-CPP-UTILS-007
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() function with simple string
 */
TEST(UtilsTest, FormatSimpleString) {
    std::string result = format("Hello, World!");
    EXPECT_EQ(result, "Hello, World!");
}

/**
 * Test Case: TC-CPP-UTILS-008
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() function with integer
 */
TEST(UtilsTest, FormatWithInteger) {
    std::string result = format("Category: %d", 48);
    EXPECT_EQ(result, "Category: 48");
}

/**
 * Test Case: TC-CPP-UTILS-009
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() function with multiple parameters
 */
TEST(UtilsTest, FormatWithMultipleParameters) {
    std::string result = format("Cat %d, Item %s, Value: %d", 48, "I010", 255);
    EXPECT_EQ(result, "Cat 48, Item I010, Value: 255");
}

/**
 * Test Case: TC-CPP-UTILS-010
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() function with hex values
 */
TEST(UtilsTest, FormatWithHexValues) {
    std::string result = format("Hex: 0x%02X", 255);
    EXPECT_EQ(result, "Hex: 0xFF");
}

/**
 * Test Case: TC-CPP-UTILS-011
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() function with floating point
 */
TEST(UtilsTest, FormatWithFloat) {
    std::string result = format("Latitude: %.2f", 45.5);
    EXPECT_EQ(result, "Latitude: 45.50");
}

/**
 * Test Case: TC-CPP-UTILS-012
 * Requirement: REQ-HLR-002
 * Description: Verify CRC32 with large data block
 */
TEST(UtilsTest, CRC32LargeData) {
    // Create 1KB of test data
    unsigned char data[1024];
    for (int i = 0; i < 1024; i++) {
        data[i] = i & 0xFF;
    }

    uint32_t crc = crc32(data, sizeof(data));

    // Should compute without error
    EXPECT_NE(crc, 0);

    // Verify consistency
    uint32_t crc2 = crc32(data, sizeof(data));
    EXPECT_EQ(crc, crc2);
}

/**
 * Test Case: TC-CPP-UTILS-013
 * Requirement: REQ-HLR-002
 * Description: Verify CRC32 with typical ASTERIX packet
 */
TEST(UtilsTest, CRC32ASTERIXPacket) {
    // Typical CAT048 packet
    unsigned char packet[] = {
        0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
        0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
        0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
        0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
        0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
        0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
    };

    uint32_t crc = crc32(packet, sizeof(packet));

    // Should be deterministic
    EXPECT_NE(crc, 0);

    // Recompute
    uint32_t crc2 = crc32(packet, sizeof(packet));
    EXPECT_EQ(crc, crc2);
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
