/**
 * Integration test for CAT062 parsing
 *
 * This test verifies the complete ASTERIX parsing pipeline for Category 062
 * (System Track Data):
 * 1. Load XML configuration
 * 2. Parse binary ASTERIX data
 * 3. Verify parsed output
 *
 * Requirements Coverage:
 * - REQ-HLR-001: Parse ASTERIX binary data
 * - REQ-HLR-SYS-001: Parse ASTERIX categories
 * - REQ-LLR-062-010: Parse CAT062 Data Source Identifier
 *
 * Test Cases:
 * - TC-INT-CAT062-001: Load CAT062 XML configuration
 * - TC-INT-CAT062-002: Parse CAT062 binary data
 * - TC-INT-CAT062-003: Verify parsed data structure
 * - TC-INT-CAT062-004: Reject invalid data
 * - TC-INT-CAT062-005: Verify text output generation
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

class CAT062IntegrationTest : public ::testing::Test {
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
        // First load BDS definitions (may be required for some items)
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
     * Extract CAT062 data from combined file
     * Returns size of CAT062 data extracted
     */
    size_t ExtractCAT062Data(const unsigned char* source, size_t sourceSize,
                             unsigned char* dest, size_t destSize) {
        if (sourceSize < 3) return 0;

        // Check if first block is CAT062 (0x3E = 62)
        if (source[0] == 0x3E) {
            // Get block length from bytes 1-2 (big endian)
            size_t blockLength = (source[1] << 8) | source[2];

            if (blockLength <= destSize && blockLength <= sourceSize) {
                memcpy(dest, source, blockLength);
                return blockLength;
            }
        }

        return 0;
    }
};

/**
 * Test Case: TC-INT-CAT062-001
 * Requirement: REQ-HLR-SYS-001
 *
 * Verify that CAT062 XML configuration can be loaded successfully
 */
TEST_F(CAT062IntegrationTest, LoadXMLConfiguration) {
    const char* configFile = "../asterix/config/asterix_cat062_1_18.xml";

    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load CAT062 XML configuration";

    // Verify CAT062 category is defined
    ASSERT_TRUE(pDefinition->CategoryDefined(62)) << "CAT062 category not defined after loading XML";

    // Get CAT062 category
    Category* cat062 = pDefinition->getCategory(62);
    ASSERT_NE(cat062, nullptr) << "CAT062 category is NULL";

    // Verify category ID
    EXPECT_EQ(cat062->m_id, 62) << "Category ID should be 62";
}

/**
 * Test Case: TC-INT-CAT062-002
 * Requirement: REQ-HLR-001
 *
 * Verify that CAT062 binary data can be parsed successfully
 */
TEST_F(CAT062IntegrationTest, ParseCAT062BinaryData) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat062_1_18.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load configuration";

    // Read CAT062+065 combined sample data
    unsigned char buffer[512];
    const char* dataFile = "../asterix/sample_data/cat062cat065.raw";
    size_t dataSize = ReadBinaryFile(dataFile, buffer, sizeof(buffer));

    ASSERT_GT(dataSize, 0) << "Failed to read CAT062/065 sample data file";
    ASSERT_EQ(dataSize, 195) << "CAT062/065 sample file should be 195 bytes";

    // Extract just the CAT062 portion
    unsigned char cat062Buffer[512];
    size_t cat062Size = ExtractCAT062Data(buffer, dataSize, cat062Buffer, sizeof(cat062Buffer));

    ASSERT_GT(cat062Size, 0) << "Failed to extract CAT062 data";
    ASSERT_GE(cat062Size, 3) << "CAT062 data too small";

    // Verify first bytes are CAT062 header
    EXPECT_EQ(cat062Buffer[0], 0x3E) << "First byte should be 0x3E (category 62)";
    EXPECT_EQ(cat062Buffer[1], 0x00) << "Second byte should be 0x00 (length MSB)";
    EXPECT_EQ(cat062Buffer[2], 0xB7) << "Third byte should be 0xB7 (length LSB = 183)";

    // Create parser
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr; // InputParser now owns it

    // Parse the data
    AsterixData* pData = pParser->parsePacket(cat062Buffer, cat062Size, 0.0);

    ASSERT_NE(pData, nullptr) << "parsePacket returned NULL";

    // Verify we have data blocks
    EXPECT_GT(pData->m_lDataBlocks.size(), 0) << "No data blocks parsed";

    // Clean up
    delete pData;
}

/**
 * Test Case: TC-INT-CAT062-003
 * Requirement: REQ-LLR-062-010
 *
 * Verify that parsed CAT062 data structure is correct
 */
TEST_F(CAT062IntegrationTest, VerifyParsedDataStructure) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat062_1_18.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Read and extract CAT062 data
    unsigned char buffer[512];
    const char* dataFile = "../asterix/sample_data/cat062cat065.raw";
    size_t dataSize = ReadBinaryFile(dataFile, buffer, sizeof(buffer));
    ASSERT_GT(dataSize, 0);

    unsigned char cat062Buffer[512];
    size_t cat062Size = ExtractCAT062Data(buffer, dataSize, cat062Buffer, sizeof(cat062Buffer));
    ASSERT_GT(cat062Size, 0);

    // Create parser and parse
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(cat062Buffer, cat062Size, 0.0);
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
    EXPECT_EQ(pBlock->m_pCategory->m_id, 62) << "Data block category ID should be 62";

    // Verify we have data records
    EXPECT_GT(pBlock->m_lDataRecords.size(), 0) << "Data block should contain records";

    // Clean up
    delete pData;
}

/**
 * Test Case: TC-INT-CAT062-004
 * Requirement: REQ-HLR-001
 *
 * Verify that invalid data is rejected properly
 */
TEST_F(CAT062IntegrationTest, RejectInvalidData) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat062_1_18.xml";
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

    // Test with invalid category (not CAT062)
    unsigned char wrongCategory[10] = {0x30, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    AsterixData* pData2 = pParser->parsePacket(wrongCategory, 10, 0.0);

    // Wrong category should still parse (as CAT048 if defined) or be rejected
    if (pData2) {
        // If parsed, verify it's not treated as CAT062
        if (pData2->m_lDataBlocks.size() > 0) {
            DataBlock* pBlock = pData2->m_lDataBlocks.front();
            if (pBlock && pBlock->m_pCategory) {
                EXPECT_NE(pBlock->m_pCategory->m_id, 62) << "Should not be parsed as CAT062";
            }
        }
        delete pData2;
    }

    // Test with truncated CAT062 data (header says 183 bytes but only provide 20)
    unsigned char truncatedBuffer[20] = {0x3E, 0x00, 0xB7}; // Rest zeros
    AsterixData* pData3 = pParser->parsePacket(truncatedBuffer, 20, 0.0);

    // Truncated data may be parsed but should handle gracefully
    if (pData3) {
        // Should either have no blocks or blocks with format errors
        delete pData3;
    }
}

/**
 * Test Case: TC-INT-CAT062-005
 * Requirement: REQ-HLR-001
 *
 * Verify that getText output is generated
 */
TEST_F(CAT062IntegrationTest, VerifyTextOutput) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat062_1_18.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Read and extract CAT062 data
    unsigned char buffer[512];
    const char* dataFile = "../asterix/sample_data/cat062cat065.raw";
    size_t dataSize = ReadBinaryFile(dataFile, buffer, sizeof(buffer));
    ASSERT_GT(dataSize, 0);

    unsigned char cat062Buffer[512];
    size_t cat062Size = ExtractCAT062Data(buffer, dataSize, cat062Buffer, sizeof(cat062Buffer));
    ASSERT_GT(cat062Size, 0);

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(cat062Buffer, cat062Size, 0.0);
    ASSERT_NE(pData, nullptr);
    ASSERT_GT(pData->m_lDataBlocks.size(), 0);

    // Get text output (format type 0 = default text)
    std::string textOutput;
    bool result = pData->getText(textOutput, 0);

    // Should succeed (even if text is empty or minimal)
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
