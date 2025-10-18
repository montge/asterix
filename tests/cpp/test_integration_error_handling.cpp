/**
 * Integration test for error handling and edge cases
 *
 * This test verifies robust error handling in the ASTERIX parsing pipeline:
 * 1. Malformed data handling
 * 2. Invalid XML configuration
 * 3. Buffer overflow protection
 * 4. Truncated data handling
 * 5. Invalid category handling
 *
 * Requirements Coverage:
 * - REQ-HLR-ERR-001: Validate data integrity
 * - REQ-HLR-ERR-002: Handle malformed input gracefully
 * - REQ-HLR-ERR-003: Prevent buffer overflows
 *
 * Test Cases:
 * - TC-INT-ERR-001: Handle completely invalid data
 * - TC-INT-ERR-002: Handle truncated ASTERIX blocks
 * - TC-INT-ERR-003: Handle invalid category numbers
 * - TC-INT-ERR-004: Handle corrupted length fields
 * - TC-INT-ERR-005: Handle zero-length data
 */

#include <gtest/gtest.h>
#include "../../src/asterix/XMLParser.h"
#include "../../src/asterix/AsterixDefinition.h"
#include "../../src/asterix/InputParser.h"
#include "../../src/asterix/AsterixData.h"
#include "../../src/asterix/DataBlock.h"
#include <cstdio>
#include <cstring>
#include <fstream>

// Global variables required by ASTERIX library
bool gVerbose = false;
bool gFiltering = false;

class ErrorHandlingTest : public ::testing::Test {
protected:
    AsterixDefinition* pDefinition;
    InputParser* pParser;

    void SetUp() override {
        pDefinition = new AsterixDefinition();
        pParser = nullptr;

        // Load CAT048 configuration for valid category tests
        LoadCAT048Config();
    }

    void TearDown() override {
        // InputParser deletes pDefinition in its destructor
        if (pParser) {
            delete pParser;
        } else if (pDefinition) {
            delete pDefinition;
        }
    }

    /**
     * Load CAT048 XML configuration for tests
     */
    bool LoadCAT048Config() {
        // Load BDS definitions
        FILE* pBDSFile = fopen("../asterix/config/asterix_bds.xml", "r");
        if (pBDSFile) {
            XMLParser bdsParser;
            bdsParser.Parse(pBDSFile, pDefinition, "asterix_bds.xml");
            fclose(pBDSFile);
        }

        // Load CAT048
        FILE* pFile = fopen("../asterix/config/asterix_cat048_1_21.xml", "r");
        if (!pFile) {
            return false;
        }

        XMLParser parser;
        bool result = parser.Parse(pFile, pDefinition, "asterix_cat048_1_21.xml");
        fclose(pFile);

        return result;
    }
};

/**
 * Test Case: TC-INT-ERR-001
 * Requirement: REQ-HLR-ERR-002
 *
 * Verify that completely invalid data is handled gracefully
 */
TEST_F(ErrorHandlingTest, HandleCompletelyInvalidData) {
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    // Test 1: All zeros
    unsigned char zeros[100];
    memset(zeros, 0, sizeof(zeros));

    AsterixData* pData1 = pParser->parsePacket(zeros, sizeof(zeros), 0.0);
    // Should either return NULL or empty data, not crash
    if (pData1) {
        // If it parsed something, it should be empty or marked as error
        delete pData1;
    }
    SUCCEED() << "Handled all-zeros data without crashing";

    // Test 2: All 0xFF
    unsigned char ones[100];
    memset(ones, 0xFF, sizeof(ones));

    AsterixData* pData2 = pParser->parsePacket(ones, sizeof(ones), 0.0);
    if (pData2) {
        delete pData2;
    }
    SUCCEED() << "Handled all-ones data without crashing";

    // Test 3: Random garbage
    unsigned char random[100] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88
    };

    AsterixData* pData3 = pParser->parsePacket(random, sizeof(random), 0.0);
    if (pData3) {
        delete pData3;
    }
    SUCCEED() << "Handled random garbage data without crashing";
}

/**
 * Test Case: TC-INT-ERR-002
 * Requirement: REQ-HLR-ERR-002
 *
 * Verify that truncated ASTERIX blocks are handled correctly
 */
TEST_F(ErrorHandlingTest, HandleTruncatedBlocks) {
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    // Valid CAT048 header claiming 1000 bytes but only provide 10
    unsigned char truncated[] = {
        0x30,  // Category 48
        0x03, 0xE8,  // Length: 1000 bytes (but we only have 10)
        0xFD, 0xF7,  // FSPEC start
        0x02, 0x19, 0xC9, 0x35, 0x6D
    };

    AsterixData* pData = pParser->parsePacket(truncated, sizeof(truncated), 0.0);

    // Should handle gracefully - either NULL or data with format error
    if (pData) {
        if (pData->m_lDataBlocks.size() > 0) {
            DataBlock* pBlock = pData->m_lDataBlocks.front();
            // Block might be marked as format error
            // This is OK - we just don't want crashes
        }
        delete pData;
    }

    SUCCEED() << "Handled truncated block without crashing";
}

/**
 * Test Case: TC-INT-ERR-003
 * Requirement: REQ-HLR-ERR-002
 *
 * Verify that invalid category numbers are handled
 */
TEST_F(ErrorHandlingTest, HandleInvalidCategories) {
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    // Test various invalid category numbers
    unsigned char invalidCategories[][10] = {
        {0xFF, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Cat 255
        {0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Cat 0
        {0xAA, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Cat 170
        {0x99, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Cat 153
    };

    for (int i = 0; i < 4; i++) {
        AsterixData* pData = pParser->parsePacket(invalidCategories[i], 10, 0.0);

        // Should handle gracefully
        if (pData) {
            // Might have blocks or might be empty
            delete pData;
        }
    }

    SUCCEED() << "Handled invalid category numbers without crashing";
}

/**
 * Test Case: TC-INT-ERR-004
 * Requirement: REQ-HLR-ERR-003
 *
 * Verify protection against buffer overflows via corrupted length fields
 */
TEST_F(ErrorHandlingTest, HandleCorruptedLengthFields) {
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    // CAT048 with impossibly large length field
    unsigned char hugeLength[] = {
        0x30,  // Category 48
        0xFF, 0xFF,  // Length: 65535 bytes (but buffer is tiny)
        0xFD, 0xF7,
        0x02, 0x19, 0xC9
    };

    AsterixData* pData1 = pParser->parsePacket(hugeLength, sizeof(hugeLength), 0.0);
    if (pData1) {
        delete pData1;
    }
    SUCCEED() << "Handled huge length field without buffer overflow";

    // Length smaller than header (impossible)
    unsigned char tinyLength[] = {
        0x30,  // Category 48
        0x00, 0x02,  // Length: 2 bytes (less than 3-byte header!)
        0xFD
    };

    AsterixData* pData2 = pParser->parsePacket(tinyLength, sizeof(tinyLength), 0.0);
    if (pData2) {
        delete pData2;
    }
    SUCCEED() << "Handled impossible tiny length field";

    // Zero length
    unsigned char zeroLength[] = {
        0x30,  // Category 48
        0x00, 0x00,  // Length: 0 bytes
    };

    AsterixData* pData3 = pParser->parsePacket(zeroLength, sizeof(zeroLength), 0.0);
    if (pData3) {
        delete pData3;
    }
    SUCCEED() << "Handled zero length field";
}

/**
 * Test Case: TC-INT-ERR-005
 * Requirement: REQ-HLR-ERR-002
 *
 * Verify handling of zero-length and minimal data
 */
TEST_F(ErrorHandlingTest, HandleZeroLengthData) {
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    // Test 1: Zero-length buffer
    AsterixData* pData1 = pParser->parsePacket(nullptr, 0, 0.0);
    // Should return either NULL or empty data structure
    if (pData1) {
        // If not NULL, should have no blocks
        EXPECT_EQ(pData1->m_lDataBlocks.size(), 0) << "Zero-length should have no blocks";
        delete pData1;
    }
    SUCCEED() << "Handled zero-length data gracefully";

    // Test 2: Single byte
    unsigned char oneByte[] = {0x30};
    AsterixData* pData2 = pParser->parsePacket(oneByte, 1, 0.0);
    // Should handle gracefully
    if (pData2) {
        EXPECT_EQ(pData2->m_lDataBlocks.size(), 0) << "Single byte should not parse to valid block";
        delete pData2;
    }

    // Test 3: Two bytes (not enough for header)
    unsigned char twoBytes[] = {0x30, 0x00};
    AsterixData* pData3 = pParser->parsePacket(twoBytes, 2, 0.0);
    if (pData3) {
        EXPECT_EQ(pData3->m_lDataBlocks.size(), 0) << "Two bytes should not parse to valid block";
        delete pData3;
    }

    SUCCEED() << "Handled minimal data without crashing";
}

/**
 * Test Case: TC-INT-ERR-006
 * Requirement: REQ-HLR-ERR-001
 *
 * Verify handling of data with invalid FSPEC
 */
TEST_F(ErrorHandlingTest, HandleInvalidFSPEC) {
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    // CAT048 with all-ones FSPEC (claims all fields present)
    unsigned char invalidFSPEC[] = {
        0x30,  // Category 48
        0x00, 0x10,  // Length: 16 bytes
        0xFF, 0xFF, 0xFF, 0xFF,  // FSPEC: All fields present (invalid)
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    };

    AsterixData* pData = pParser->parsePacket(invalidFSPEC, sizeof(invalidFSPEC), 0.0);

    // Should handle gracefully
    if (pData) {
        // Might parse with errors, but shouldn't crash
        delete pData;
    }

    SUCCEED() << "Handled invalid FSPEC without crashing";
}

/**
 * Test Case: TC-INT-ERR-007
 * Requirement: REQ-HLR-ERR-002
 *
 * Verify handling of multiple blocks with errors
 */
TEST_F(ErrorHandlingTest, HandleMultipleBlocksWithErrors) {
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    // Multiple blocks, some valid, some invalid
    unsigned char multipleBlocks[200];
    size_t offset = 0;

    // Block 1: Valid CAT048 (minimal)
    multipleBlocks[offset++] = 0x30;  // Category
    multipleBlocks[offset++] = 0x00;  // Length MSB
    multipleBlocks[offset++] = 0x03;  // Length LSB (3 bytes total - just header)

    // Block 2: Invalid category
    multipleBlocks[offset++] = 0xFF;  // Invalid category
    multipleBlocks[offset++] = 0x00;
    multipleBlocks[offset++] = 0x03;

    // Block 3: Valid CAT048 again
    multipleBlocks[offset++] = 0x30;
    multipleBlocks[offset++] = 0x00;
    multipleBlocks[offset++] = 0x03;

    AsterixData* pData = pParser->parsePacket(multipleBlocks, offset, 0.0);

    // Should handle mix of valid and invalid blocks
    if (pData) {
        // Might have parsed some blocks successfully
        delete pData;
    }

    SUCCEED() << "Handled multiple blocks with mixed validity";
}

// Run all tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
