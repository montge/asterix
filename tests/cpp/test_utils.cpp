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

/**
 * Test Case: TC-CPP-UTILS-014
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() with empty string
 */
TEST(UtilsTest, FormatEmptyString) {
    std::string result = format("");
    EXPECT_EQ(result, "");
}

/**
 * Test Case: TC-CPP-UTILS-015
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() heap fallback path with string >512 chars
 */
TEST(UtilsTest, FormatLargeStringHeapFallback) {
    // Create a format string that will produce output > 512 bytes
    // This tests the heap allocation fallback path in format()
    std::string large_pattern(600, 'X');
    std::string result = format("%s", large_pattern.c_str());

    EXPECT_EQ(result.length(), 600);
    EXPECT_EQ(result, large_pattern);
}

/**
 * Test Case: TC-CPP-UTILS-016
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() with exactly 511 chars (fits in stack buffer)
 */
TEST(UtilsTest, FormatExactlyFitsStackBuffer) {
    std::string pattern(511, 'Y');
    std::string result = format("%s", pattern.c_str());

    EXPECT_EQ(result.length(), 511);
    EXPECT_EQ(result, pattern);
}

/**
 * Test Case: TC-CPP-UTILS-017
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() with exactly 512 chars (boundary - needs heap)
 */
TEST(UtilsTest, FormatBoundaryStackToHeap) {
    std::string pattern(512, 'Z');
    std::string result = format("%s", pattern.c_str());

    EXPECT_EQ(result.length(), 512);
    EXPECT_EQ(result, pattern);
}

/**
 * Test Case: TC-CPP-UTILS-018
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() with very large string (2000+ chars)
 */
TEST(UtilsTest, FormatVeryLargeString) {
    std::string pattern(2000, 'W');
    std::string result = format("%s", pattern.c_str());

    EXPECT_EQ(result.length(), 2000);
    EXPECT_EQ(result, pattern);
}

/**
 * Test Case: TC-CPP-UTILS-019
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() with multiple large arguments
 */
TEST(UtilsTest, FormatMultipleLargeArgs) {
    std::string arg1(200, 'A');
    std::string arg2(200, 'B');
    std::string arg3(200, 'C');
    std::string result = format("%s-%s-%s", arg1.c_str(), arg2.c_str(), arg3.c_str());

    // Total length: 200 + 1 + 200 + 1 + 200 = 602, needs heap
    EXPECT_EQ(result.length(), 602);
    EXPECT_NE(result.find(arg1), std::string::npos);
    EXPECT_NE(result.find(arg2), std::string::npos);
    EXPECT_NE(result.find(arg3), std::string::npos);
}

/**
 * Test Case: TC-CPP-UTILS-020
 * Requirement: REQ-HLR-002
 * Description: Verify CRC32 with single byte data
 */
TEST(UtilsTest, CRC32SingleByte) {
    unsigned char data[] = {0x00};
    uint32_t crc = crc32(data, sizeof(data));

    // Should be deterministic
    uint32_t crc2 = crc32(data, sizeof(data));
    EXPECT_EQ(crc, crc2);
}

/**
 * Test Case: TC-CPP-UTILS-021
 * Requirement: REQ-HLR-002
 * Description: Verify CRC32 with all-zero data
 */
TEST(UtilsTest, CRC32AllZeroData) {
    unsigned char data[16] = {0};
    uint32_t crc = crc32(data, sizeof(data));

    // All zeros should produce non-zero CRC
    EXPECT_NE(crc, 0);
}

/**
 * Test Case: TC-CPP-UTILS-022
 * Requirement: REQ-HLR-002
 * Description: Verify CRC32 with all-ones data
 */
TEST(UtilsTest, CRC32AllOnesData) {
    unsigned char data[16];
    memset(data, 0xFF, sizeof(data));
    uint32_t crc = crc32(data, sizeof(data));

    // Should be deterministic
    uint32_t crc2 = crc32(data, sizeof(data));
    EXPECT_EQ(crc, crc2);

    // All zeros and all ones should give different CRCs
    unsigned char zeros[16] = {0};
    uint32_t crc_zeros = crc32(zeros, sizeof(zeros));
    EXPECT_NE(crc, crc_zeros);
}

/**
 * Test Case: TC-CPP-UTILS-023
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() with printf-style padding
 */
TEST(UtilsTest, FormatWithPadding) {
    std::string result = format("%08d", 42);
    EXPECT_EQ(result, "00000042");
}

/**
 * Test Case: TC-CPP-UTILS-024
 * Requirement: REQ-HLR-SYS-001
 * Description: Verify format() with string width specifier
 */
TEST(UtilsTest, FormatWithStringWidth) {
    std::string result = format("%-20s|", "test");
    EXPECT_EQ(result, "test                |");
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
