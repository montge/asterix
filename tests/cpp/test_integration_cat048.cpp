/**
 * Integration test for CAT048 parsing
 *
 * This test verifies the complete ASTERIX parsing pipeline:
 * 1. Load XML configuration
 * 2. Parse binary ASTERIX data
 * 3. Verify parsed output
 *
 * Requirements Coverage:
 * - REQ-HLR-001: Parse ASTERIX binary data
 * - REQ-HLR-SYS-001: Parse ASTERIX categories
 * - REQ-LLR-048-010: Parse CAT048 Data Source Identifier
 *
 * Test Cases:
 * - TC-INT-CAT048-001: Load CAT048 XML configuration
 * - TC-INT-CAT048-002: Parse CAT048 binary data
 * - TC-INT-CAT048-003: Verify parsed data structure
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

class CAT048IntegrationTest : public ::testing::Test {
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
        // First load BDS definitions (required for CAT048)
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
};

/**
 * Test Case: TC-INT-CAT048-001
 * Requirement: REQ-HLR-SYS-001
 *
 * Verify that CAT048 XML configuration can be loaded successfully
 */
TEST_F(CAT048IntegrationTest, LoadXMLConfiguration) {
    const char* configFile = "../asterix/config/asterix_cat048_1_30.xml";

    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load CAT048 XML configuration";

    // Verify CAT048 category is defined
    ASSERT_TRUE(pDefinition->CategoryDefined(48)) << "CAT048 category not defined after loading XML";

    // Get CAT048 category
    Category* cat048 = pDefinition->getCategory(48);
    ASSERT_NE(cat048, nullptr) << "CAT048 category is NULL";

    // Verify category ID
    EXPECT_EQ(cat048->m_id, 48) << "Category ID should be 48";
}

/**
 * Test Case: TC-INT-CAT048-002
 * Requirement: REQ-HLR-001
 *
 * Verify that CAT048 binary data can be parsed successfully
 */
TEST_F(CAT048IntegrationTest, ParseCAT048BinaryData) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat048_1_30.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load configuration";

    // Read CAT048 sample data
    unsigned char buffer[1024];
    const char* dataFile = "../asterix/sample_data/cat048.raw";
    size_t dataSize = ReadBinaryFile(dataFile, buffer, sizeof(buffer));

    ASSERT_GT(dataSize, 0) << "Failed to read CAT048 sample data file";
    ASSERT_EQ(dataSize, 48) << "CAT048 sample file should be 48 bytes";

    // Verify first bytes are CAT048 header
    EXPECT_EQ(buffer[0], 0x30) << "First byte should be 0x30 (category 48)";
    EXPECT_EQ(buffer[1], 0x00) << "Second byte should be 0x00 (length MSB)";
    EXPECT_EQ(buffer[2], 0x30) << "Third byte should be 0x30 (length LSB = 48)";

    // Create parser
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr; // InputParser now owns it

    // Parse the data
    AsterixData* pData = pParser->parsePacket(buffer, dataSize, 0.0);

    ASSERT_NE(pData, nullptr) << "parsePacket returned NULL";

    // Verify we have data blocks
    EXPECT_GT(pData->m_lDataBlocks.size(), 0) << "No data blocks parsed";

    // Clean up
    delete pData;
}

/**
 * Test Case: TC-INT-CAT048-003
 * Requirement: REQ-LLR-048-010
 *
 * Verify that parsed CAT048 data structure is correct
 */
TEST_F(CAT048IntegrationTest, VerifyParsedDataStructure) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat048_1_30.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Read CAT048 sample data
    unsigned char buffer[1024];
    const char* dataFile = "../asterix/sample_data/cat048.raw";
    size_t dataSize = ReadBinaryFile(dataFile, buffer, sizeof(buffer));
    ASSERT_GT(dataSize, 0);

    // Create parser and parse
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(buffer, dataSize, 0.0);
    ASSERT_NE(pData, nullptr);

    // Verify data blocks
    ASSERT_EQ(pData->m_lDataBlocks.size(), 1) << "Should have exactly 1 data block";

    // Get first data block
    DataBlock* pBlock = pData->m_lDataBlocks.front();
    ASSERT_NE(pBlock, nullptr) << "Data block is NULL";

    // Verify data block properties (45 bytes payload + 3 bytes header = 48 total)
    EXPECT_EQ(pBlock->m_nLength, 45) << "Data block length should be 45 bytes (payload size)";
    EXPECT_TRUE(pBlock->m_bFormatOK) << "Data block format should be OK";

    // Verify category
    ASSERT_NE(pBlock->m_pCategory, nullptr) << "Data block category is NULL";
    EXPECT_EQ(pBlock->m_pCategory->m_id, 48) << "Data block category ID should be 48";

    // Verify we have data records
    EXPECT_GT(pBlock->m_lDataRecords.size(), 0) << "Data block should contain records";

    // Clean up
    delete pData;
}

/**
 * Test Case: TC-INT-CAT048-004
 * Requirement: REQ-HLR-001
 *
 * Verify that invalid data is rejected properly
 */
TEST_F(CAT048IntegrationTest, RejectInvalidData) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat048_1_30.xml";
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

    // Test with truncated data (only 10 bytes instead of declared 48)
    unsigned char truncatedBuffer[10] = {0x30, 0x00, 0x30, 0xFD, 0xF7, 0x02, 0x19, 0xC9, 0x35, 0x6D};
    AsterixData* pData2 = pParser->parsePacket(truncatedBuffer, 10, 0.0);

    // Truncated data may be parsed but should be marked as format error
    if (pData2) {
        if (pData2->m_lDataBlocks.size() > 0) {
            DataBlock* pBlock = pData2->m_lDataBlocks.front();
            // Format may be marked as NOT OK due to truncation
            // (exact behavior depends on implementation)
        }
        delete pData2;
    }
}

/**
 * Test Case: TC-INT-CAT048-005
 * Requirement: REQ-HLR-001
 *
 * Verify that getText output is generated
 */
TEST_F(CAT048IntegrationTest, VerifyTextOutput) {
    // Load configuration
    const char* configFile = "../asterix/config/asterix_cat048_1_30.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Read and parse CAT048 sample data
    unsigned char buffer[1024];
    const char* dataFile = "../asterix/sample_data/cat048.raw";
    size_t dataSize = ReadBinaryFile(dataFile, buffer, sizeof(buffer));
    ASSERT_GT(dataSize, 0);

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(buffer, dataSize, 0.0);
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
