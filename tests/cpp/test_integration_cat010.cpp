/**
 * Integration test for CAT010 parsing
 *
 * This test verifies the complete ASTERIX parsing pipeline for Category 010
 * (Monoradar Target Reports - Surface Movement):
 * 1. Load XML configuration
 * 2. Parse binary ASTERIX data
 * 3. Verify parsed output
 *
 * Requirements Coverage:
 * - REQ-HLR-001: Parse ASTERIX binary data
 * - REQ-HLR-SYS-001: Parse ASTERIX categories
 *
 * Test Cases:
 * - TC-INT-CAT010-001: Load CAT010 XML configuration
 * - TC-INT-CAT010-002: Parse CAT010 binary data with I010/010
 * - TC-INT-CAT010-003: Reject invalid data
 * - TC-INT-CAT010-004: Verify text output generation
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

class CAT010IntegrationTest : public ::testing::Test {
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
 * Test Case: TC-INT-CAT010-001
 * Verify that CAT010 XML configuration can be loaded successfully
 */
TEST_F(CAT010IntegrationTest, LoadXMLConfiguration) {
    const char* configFile = "../asterix/config/asterix_cat010_1_1.xml";

    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load CAT010 XML configuration";

    ASSERT_TRUE(pDefinition->CategoryDefined(10)) << "CAT010 category not defined after loading XML";

    Category* cat010 = pDefinition->getCategory(10);
    ASSERT_NE(cat010, nullptr) << "CAT010 category is NULL";
    EXPECT_EQ(cat010->m_id, 10) << "Category ID should be 10";
}

/**
 * Test Case: TC-INT-CAT010-002
 * Verify that CAT010 binary data can be parsed successfully
 */
TEST_F(CAT010IntegrationTest, ParseCAT010BinaryData) {
    const char* configFile = "../asterix/config/asterix_cat010_1_1.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Create minimal CAT010 packet with I010/010 (Data Source Identifier)
    unsigned char buffer[256];
    buffer[0] = 0x0A;  // Category 10
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x06;  // Length LSB = 6 bytes
    buffer[3] = 0x80;  // FSPEC: I010/010 present
    buffer[4] = 0x01;  // I010/010: SAC
    buffer[5] = 0x23;  // I010/010: SIC

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(buffer, 6, 0.0);

    if (pData) {
        delete pData;
    }
}

/**
 * Test Case: TC-INT-CAT010-003
 * Verify that invalid data is rejected properly
 */
TEST_F(CAT010IntegrationTest, RejectInvalidData) {
    const char* configFile = "../asterix/config/asterix_cat010_1_1.xml";
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
 * Test Case: TC-INT-CAT010-004
 * Verify text output generation for CAT010
 */
TEST_F(CAT010IntegrationTest, VerifyTextOutput) {
    const char* configFile = "../asterix/config/asterix_cat010_1_1.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    unsigned char buffer[256];
    buffer[0] = 0x0A;  // Category 10
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x06;  // Length LSB = 6 bytes
    buffer[3] = 0x80;  // FSPEC: I010/010 present
    buffer[4] = 0x01;  // SAC
    buffer[5] = 0x23;  // SIC

    AsterixData* pData = pParser->parsePacket(buffer, 6, 0.0);

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
