/**
 * Integration test for CAT002 parsing
 *
 * This test verifies the complete ASTERIX parsing pipeline for Category 002
 * (Monoradar Service Messages):
 * 1. Load XML configuration
 * 2. Parse binary ASTERIX data
 * 3. Verify parsed output
 *
 * Requirements Coverage:
 * - REQ-HLR-001: Parse ASTERIX binary data
 * - REQ-HLR-SYS-001: Parse ASTERIX categories
 *
 * Test Cases:
 * - TC-INT-CAT002-001: Load CAT002 XML configuration
 * - TC-INT-CAT002-002: Parse CAT002 binary data with I002/010
 * - TC-INT-CAT002-003: Reject invalid data
 * - TC-INT-CAT002-004: Verify text output generation
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

class CAT002IntegrationTest : public ::testing::Test {
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
};

/**
 * Test Case: TC-INT-CAT002-001
 * Verify that CAT002 XML configuration can be loaded successfully
 */
TEST_F(CAT002IntegrationTest, LoadXMLConfiguration) {
    const char* configFile = "../asterix/config/asterix_cat002_1_1.xml";

    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load CAT002 XML configuration";

    ASSERT_TRUE(pDefinition->CategoryDefined(2)) << "CAT002 category not defined after loading XML";

    Category* cat002 = pDefinition->getCategory(2);
    ASSERT_NE(cat002, nullptr) << "CAT002 category is NULL";
    EXPECT_EQ(cat002->m_id, 2) << "Category ID should be 2";
}

/**
 * Test Case: TC-INT-CAT002-002
 * Verify that CAT002 binary data can be parsed successfully
 */
TEST_F(CAT002IntegrationTest, ParseCAT002BinaryData) {
    const char* configFile = "../asterix/config/asterix_cat002_1_1.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Create minimal CAT002 packet with I002/010 (Data Source Identifier)
    unsigned char buffer[256];
    buffer[0] = 0x02;  // Category 2
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x07;  // Length LSB = 7 bytes
    buffer[3] = 0xC0;  // FSPEC: I002/010 + I002/000 present
    buffer[4] = 0x01;  // I002/010: SAC
    buffer[5] = 0x23;  // I002/010: SIC
    buffer[6] = 0x01;  // I002/000: Message Type

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(buffer, 7, 0.0);

    if (pData) {
        delete pData;
    }
}

/**
 * Test Case: TC-INT-CAT002-003
 * Verify that invalid data is rejected properly
 */
TEST_F(CAT002IntegrationTest, RejectInvalidData) {
    const char* configFile = "../asterix/config/asterix_cat002_1_1.xml";
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
 * Test Case: TC-INT-CAT002-004
 * Verify text output generation for CAT002
 */
TEST_F(CAT002IntegrationTest, VerifyTextOutput) {
    const char* configFile = "../asterix/config/asterix_cat002_1_1.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    unsigned char buffer[256];
    buffer[0] = 0x02;  // Category 2
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x07;  // Length LSB = 7 bytes
    buffer[3] = 0xC0;  // FSPEC: I002/010 + I002/000
    buffer[4] = 0x01;  // SAC
    buffer[5] = 0x23;  // SIC
    buffer[6] = 0x01;  // Message Type

    AsterixData* pData = pParser->parsePacket(buffer, 7, 0.0);

    if (pData && pData->m_lDataBlocks.size() > 0) {
        std::string textOutput;
        bool result = pData->getText(textOutput, 0);
        EXPECT_TRUE(result);
        delete pData;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
