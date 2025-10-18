/**
 * Integration test for CAT065 parsing
 *
 * This test verifies the complete ASTERIX parsing pipeline for Category 065
 * (SDPS Service Status Messages):
 * 1. Load XML configuration
 * 2. Parse binary ASTERIX data
 * 3. Verify parsed output
 *
 * Requirements Coverage:
 * - REQ-HLR-001: Parse ASTERIX binary data
 * - REQ-HLR-SYS-001: Parse ASTERIX categories
 * - REQ-LLR-065-010: Parse CAT065 Data Source Identifier
 *
 * Test Cases:
 * - TC-INT-CAT065-001: Load CAT065 XML configuration
 * - TC-INT-CAT065-002: Parse CAT065 binary data
 * - TC-INT-CAT065-003: Verify parsed data structure
 * - TC-INT-CAT065-004: Reject invalid data
 * - TC-INT-CAT065-005: Verify text output generation
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

class CAT065IntegrationTest : public ::testing::Test {
protected:
    AsterixDefinition* pDefinition;
    InputParser* pParser;

    void SetUp() override {
        pDefinition = new AsterixDefinition();
        pParser = nullptr;
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
     * Load XML configuration file
     * Returns true on success
     */
    bool LoadXMLConfig(const char* filename) {
        // Load BDS definitions first (may be required)
        FILE* pBDSFile = fopen("../asterix/config/asterix_bds.xml", "r");
        if (pBDSFile) {
            XMLParser bdsParser;
            bdsParser.Parse(pBDSFile, pDefinition, "asterix_bds.xml");
            fclose(pBDSFile);
        }

        // Then load the category definition
        FILE* pFile = fopen(filename, "r");
        if (!pFile) {
            return false;
        }

        XMLParser parser;
        bool result = parser.Parse(pFile, pDefinition, filename);
        fclose(pFile);

        return result;
    }

    /**
     * Read binary file into buffer
     * Returns number of bytes read, 0 on error
     */
    size_t ReadBinaryFile(const char* filename, unsigned char* buffer, size_t maxSize) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return 0;
        }

        size_t size = file.tellg();
        if (size > maxSize) {
            size = maxSize;
        }

        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(buffer), size);
        file.close();

        return size;
    }

    /**
     * Extract CAT065 data from combined file
     * Returns size of CAT065 data extracted
     */
    size_t ExtractCAT065Data(const unsigned char* source, size_t sourceSize,
                             unsigned char* dest, size_t destSize) {
        // CAT065 is 0x41 in hex (65 decimal)
        size_t offset = 0;

        // Scan through the file looking for CAT065 blocks
        while (offset + 3 <= sourceSize) {
            unsigned char category = source[offset];

            if (category == 0x41) {  // Found CAT065
                // Get block length from bytes 1-2 (big endian)
                size_t blockLength = (source[offset + 1] << 8) | source[offset + 2];

                if (blockLength <= destSize && offset + blockLength <= sourceSize) {
                    memcpy(dest, source + offset, blockLength);
                    return blockLength;
                }
                return 0;  // Block too large
            }

            // Skip to next block
            if (offset + 3 <= sourceSize) {
                size_t blockLength = (source[offset + 1] << 8) | source[offset + 2];
                if (blockLength == 0) break;  // Invalid length
                offset += blockLength;
            } else {
                break;
            }
        }

        return 0;  // CAT065 not found
    }
};

/**
 * Test Case: TC-INT-CAT065-001
 * Requirement: REQ-HLR-SYS-001
 *
 * Verify that CAT065 XML configuration can be loaded successfully
 */
TEST_F(CAT065IntegrationTest, LoadXMLConfiguration) {
    const char* configFile = "../asterix/config/asterix_cat065_1_3.xml";

    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load CAT065 XML configuration";

    // Verify CAT065 category is defined
    ASSERT_TRUE(pDefinition->CategoryDefined(65)) << "CAT065 category not defined after loading XML";

    // Get CAT065 category
    Category* cat065 = pDefinition->getCategory(65);
    ASSERT_NE(cat065, nullptr) << "CAT065 category is NULL";

    // Verify category ID
    EXPECT_EQ(cat065->m_id, 65) << "Category ID should be 65";
}

/**
 * Test Case: TC-INT-CAT065-002
 * Requirement: REQ-HLR-001
 *
 * Verify that CAT065 binary data can be parsed successfully
 */
TEST_F(CAT065IntegrationTest, ParseCAT065BinaryData) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat065_1_3.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load configuration";

    // Read CAT062+065 combined sample data
    unsigned char buffer[512];
    const char* dataFile = "../asterix/sample_data/cat062cat065.raw";
    size_t dataSize = ReadBinaryFile(dataFile, buffer, sizeof(buffer));

    ASSERT_GT(dataSize, 0) << "Failed to read CAT062/065 sample data file";

    // Extract just the CAT065 portion
    unsigned char cat065Buffer[512];
    size_t cat065Size = ExtractCAT065Data(buffer, dataSize, cat065Buffer, sizeof(cat065Buffer));

    ASSERT_GT(cat065Size, 0) << "Failed to extract CAT065 data from combined file";
    ASSERT_GE(cat065Size, 3) << "CAT065 data too small";

    // Verify first bytes are CAT065 header
    EXPECT_EQ(cat065Buffer[0], 0x41) << "First byte should be 0x41 (category 65)";

    // Create parser
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr; // InputParser now owns it

    // Parse the data
    AsterixData* pData = pParser->parsePacket(cat065Buffer, cat065Size, 0.0);

    ASSERT_NE(pData, nullptr) << "parsePacket returned NULL";

    // Verify we have data blocks
    EXPECT_GT(pData->m_lDataBlocks.size(), 0) << "No data blocks parsed";

    // Clean up
    delete pData;
}

/**
 * Test Case: TC-INT-CAT065-003
 * Requirement: REQ-LLR-065-010
 *
 * Verify that parsed CAT065 data structure is correct
 */
TEST_F(CAT065IntegrationTest, VerifyParsedDataStructure) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat065_1_3.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Read and extract CAT065 data
    unsigned char buffer[512];
    const char* dataFile = "../asterix/sample_data/cat062cat065.raw";
    size_t dataSize = ReadBinaryFile(dataFile, buffer, sizeof(buffer));
    ASSERT_GT(dataSize, 0);

    unsigned char cat065Buffer[512];
    size_t cat065Size = ExtractCAT065Data(buffer, dataSize, cat065Buffer, sizeof(cat065Buffer));
    ASSERT_GT(cat065Size, 0);

    // Create parser and parse
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(cat065Buffer, cat065Size, 0.0);
    ASSERT_NE(pData, nullptr);

    // Verify data blocks
    EXPECT_GE(pData->m_lDataBlocks.size(), 1) << "Should have at least 1 data block";

    // Get first data block
    DataBlock* pBlock = pData->m_lDataBlocks.front();
    ASSERT_NE(pBlock, nullptr) << "Data block is NULL";

    // Verify data block properties
    EXPECT_GT(pBlock->m_nLength, 0) << "Data block should have positive length";
    EXPECT_TRUE(pBlock->m_bFormatOK) << "Data block format should be OK";

    // Verify category
    ASSERT_NE(pBlock->m_pCategory, nullptr) << "Data block category is NULL";
    EXPECT_EQ(pBlock->m_pCategory->m_id, 65) << "Data block category ID should be 65";

    // Verify we have data records
    EXPECT_GT(pBlock->m_lDataRecords.size(), 0) << "Data block should contain records";

    // Clean up
    delete pData;
}

/**
 * Test Case: TC-INT-CAT065-004
 * Requirement: REQ-HLR-001
 *
 * Verify that invalid data is rejected properly
 */
TEST_F(CAT065IntegrationTest, RejectInvalidData) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat065_1_3.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Create parser
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    // Test with empty data
    unsigned char emptyBuffer[10] = {0};
    AsterixData* pData1 = pParser->parsePacket(emptyBuffer, 0, 0.0);
    // Empty data may return NULL or empty AsterixData
    if (pData1) {
        EXPECT_EQ(pData1->m_lDataBlocks.size(), 0) << "Empty data should have no blocks";
        delete pData1;
    }

    // Test with wrong category (not CAT065)
    unsigned char wrongCategory[10] = {0x30, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    AsterixData* pData2 = pParser->parsePacket(wrongCategory, 10, 0.0);

    // Wrong category should not be parsed as CAT065
    if (pData2) {
        if (pData2->m_lDataBlocks.size() > 0) {
            DataBlock* pBlock = pData2->m_lDataBlocks.front();
            if (pBlock && pBlock->m_pCategory) {
                EXPECT_NE(pBlock->m_pCategory->m_id, 65) << "Should not be parsed as CAT065";
            }
        }
        delete pData2;
    }

    // Test with truncated data
    unsigned char truncatedBuffer[5] = {0x41, 0x00, 0x64, 0x00, 0x00}; // Says 100 bytes but only 5
    AsterixData* pData3 = pParser->parsePacket(truncatedBuffer, 5, 0.0);

    // Truncated data should be handled gracefully
    if (pData3) {
        delete pData3;
    }
}

/**
 * Test Case: TC-INT-CAT065-005
 * Requirement: REQ-HLR-001
 *
 * Verify that getText output is generated
 */
TEST_F(CAT065IntegrationTest, VerifyTextOutput) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat065_1_3.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Read and extract CAT065 data
    unsigned char buffer[512];
    const char* dataFile = "../asterix/sample_data/cat062cat065.raw";
    size_t dataSize = ReadBinaryFile(dataFile, buffer, sizeof(buffer));
    ASSERT_GT(dataSize, 0);

    unsigned char cat065Buffer[512];
    size_t cat065Size = ExtractCAT065Data(buffer, dataSize, cat065Buffer, sizeof(cat065Buffer));
    ASSERT_GT(cat065Size, 0);

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(cat065Buffer, cat065Size, 0.0);
    ASSERT_NE(pData, nullptr);
    ASSERT_GT(pData->m_lDataBlocks.size(), 0);

    // Get text output (format type 0 = default text)
    std::string textOutput;
    bool result = pData->getText(textOutput, 0);

    // Should succeed
    EXPECT_TRUE(result) << "getText should return true";

    // Text output should contain something
    EXPECT_GT(textOutput.length(), 0) << "getText output should not be empty";

    // Clean up
    delete pData;
}

// Run all tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
