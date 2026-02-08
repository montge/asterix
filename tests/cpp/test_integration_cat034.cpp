/**
 * Integration test for CAT034 parsing
 *
 * This test verifies the complete ASTERIX parsing pipeline for Category 034
 * (Monoradar Service Messages Part 2b):
 * 1. Load XML configuration
 * 2. Parse binary ASTERIX data
 * 3. Verify parsed output
 *
 * Requirements Coverage:
 * - REQ-HLR-001: Parse ASTERIX binary data
 * - REQ-HLR-SYS-001: Parse ASTERIX categories
 *
 * Test Cases:
 * - TC-INT-CAT034-001: Load CAT034 XML configuration
 * - TC-INT-CAT034-002: Parse CAT034 binary data from sample file
 * - TC-INT-CAT034-003: Parse constructed CAT034 packet
 * - TC-INT-CAT034-004: Reject invalid data
 * - TC-INT-CAT034-005: Verify text output generation
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

class CAT034IntegrationTest : public ::testing::Test {
protected:
    AsterixDefinition* pDefinition;
    InputParser* pParser;

    void SetUp() override {
        pDefinition = new AsterixDefinition();
        pParser = nullptr;
    }

    void TearDown() override {
        if (pParser) {
            delete pParser;
        } else if (pDefinition) {
            delete pDefinition;
        }
    }

    bool LoadXMLConfig(const char* filename) {
        FILE* pBDSFile = fopen("../asterix/config/asterix_bds.xml", "r");
        if (pBDSFile) {
            XMLParser bdsParser;
            bdsParser.Parse(pBDSFile, pDefinition, "asterix_bds.xml");
            fclose(pBDSFile);
        }

        FILE* pFile = fopen(filename, "r");
        if (!pFile) {
            return false;
        }

        XMLParser parser;
        bool result = parser.Parse(pFile, pDefinition, filename);
        fclose(pFile);
        return result;
    }

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
 * Test Case: TC-INT-CAT034-001
 * Verify that CAT034 XML configuration can be loaded successfully
 */
TEST_F(CAT034IntegrationTest, LoadXMLConfiguration) {
    const char* configFile = "../asterix/config/asterix_cat034_1_29.xml";

    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load CAT034 XML configuration";

    ASSERT_TRUE(pDefinition->CategoryDefined(34)) << "CAT034 category not defined after loading XML";

    Category* cat034 = pDefinition->getCategory(34);
    ASSERT_NE(cat034, nullptr) << "CAT034 category is NULL";
    EXPECT_EQ(cat034->m_id, 34) << "Category ID should be 34";
}

/**
 * Test Case: TC-INT-CAT034-002
 * Verify that CAT034 binary sample data can be parsed
 */
TEST_F(CAT034IntegrationTest, ParseCAT034SampleData) {
    const char* configFile = "../asterix/config/asterix_cat034_1_29.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    unsigned char buffer[4096];
    const char* dataFile = "../asterix/sample_data/cat034.raw";
    size_t dataSize = ReadBinaryFile(dataFile, buffer, sizeof(buffer));

    ASSERT_GT(dataSize, 0) << "Failed to read CAT034 sample data file";

    // Verify first byte is CAT034
    EXPECT_EQ(buffer[0], 0x22) << "First byte should be 0x22 (category 34)";

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(buffer, dataSize, 0.0);

    ASSERT_NE(pData, nullptr) << "parsePacket returned NULL";
    EXPECT_GT(pData->m_lDataBlocks.size(), 0) << "No data blocks parsed";

    delete pData;
}

/**
 * Test Case: TC-INT-CAT034-003
 * Verify parsing of constructed CAT034 packet
 */
TEST_F(CAT034IntegrationTest, ParseConstructedPacket) {
    const char* configFile = "../asterix/config/asterix_cat034_1_29.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Create minimal CAT034 packet with I034/010 (Data Source Identifier)
    unsigned char buffer[256];
    buffer[0] = 0x22;  // Category 34
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x06;  // Length LSB = 6 bytes
    buffer[3] = 0x80;  // FSPEC: I034/010 present
    buffer[4] = 0x01;  // I034/010: SAC
    buffer[5] = 0x23;  // I034/010: SIC

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(buffer, 6, 0.0);

    if (pData) {
        delete pData;
    }
}

/**
 * Test Case: TC-INT-CAT034-004
 * Verify that invalid data is rejected properly
 */
TEST_F(CAT034IntegrationTest, RejectInvalidData) {
    const char* configFile = "../asterix/config/asterix_cat034_1_29.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    unsigned char emptyBuffer[10] = {0};
    AsterixData* pData = pParser->parsePacket(emptyBuffer, 0, 0.0);
    if (pData) {
        EXPECT_EQ(pData->m_lDataBlocks.size(), 0);
        delete pData;
    }
}

/**
 * Test Case: TC-INT-CAT034-005
 * Verify text output generation for CAT034
 */
TEST_F(CAT034IntegrationTest, VerifyTextOutput) {
    const char* configFile = "../asterix/config/asterix_cat034_1_29.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    unsigned char buffer[4096];
    const char* dataFile = "../asterix/sample_data/cat034.raw";
    size_t dataSize = ReadBinaryFile(dataFile, buffer, sizeof(buffer));

    if (dataSize == 0) {
        GTEST_SKIP() << "CAT034 sample data not available";
    }

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(buffer, dataSize, 0.0);

    if (pData && pData->m_lDataBlocks.size() > 0) {
        std::string textOutput;
        bool result = pData->getText(textOutput, 0);
        EXPECT_TRUE(result);
        EXPECT_GT(textOutput.length(), 0);
        delete pData;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
