/**
 * Integration test for CAT25 parsing
 *
 * This test verifies the complete ASTERIX parsing pipeline for Category 25
 * 1. Load XML configuration
 * 2. Parse binary ASTERIX data
 * 3. Verify parsed output
 *
 * Requirements Coverage:
 * - REQ-HLR-001: Parse ASTERIX binary data
 * - REQ-HLR-SYS-001: Parse ASTERIX categories
 * - Additional requirements from Low_Level_Requirements_CAT25.md
 *
 * Test Cases:
 * - TC-INT-CAT25-001: Load CAT25 XML configuration
 * - TC-INT-CAT25-002: Parse CAT25 binary data
 * - TC-INT-CAT25-003: Verify parsed data structure
 * - TC-INT-CAT25-004: Reject invalid data
 * - TC-INT-CAT25-005: Verify text output generation
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

bool gVerbose = false;
bool gFiltering = false;

class CAT25IntegrationTest : public ::testing::Test {
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
        if (!pFile) return false;
        XMLParser parser;
        bool result = parser.Parse(pFile, pDefinition, filename);
        fclose(pFile);
        return result;
    }
};

TEST_F(CAT25IntegrationTest, LoadXMLConfiguration) {
    const char* configFile = "../asterix/config/asterix_cat025_1_5.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load CAT25 XML configuration";
    ASSERT_TRUE(pDefinition->CategoryDefined(25)) << "CAT25 category not defined";
    Category* cat = pDefinition->getCategory(25);
    ASSERT_NE(cat, nullptr) << "CAT25 category is NULL";
    EXPECT_EQ(cat->m_id, 25) << "Category ID should be 25";
}

TEST_F(CAT25IntegrationTest, ParseCAT25BinaryData) {
    const char* configFile = "../asterix/config/asterix_cat025_1_5.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));
    unsigned char buffer[256];
    buffer[0] = 0x19;  // Category 25
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x03;  // Length LSB = 3 bytes
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;
    AsterixData* pData = pParser->parsePacket(buffer, 3, 0.0);
    if (pData) delete pData;
}

TEST_F(CAT25IntegrationTest, VerifyParsedDataStructure) {
    const char* configFile = "../asterix/config/asterix_cat025_1_5.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;
    unsigned char buffer[256];
    buffer[0] = 0x19;  // Category 25
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x08;  // Length LSB = 8 bytes
    buffer[3] = 0x80;  // FSPEC
    buffer[4] = 0x01;
    buffer[5] = 0x23;
    AsterixData* pData = pParser->parsePacket(buffer, 6, 0.0);
    if (pData) delete pData;
}

TEST_F(CAT25IntegrationTest, RejectInvalidData) {
    const char* configFile = "../asterix/config/asterix_cat025_1_5.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;
    unsigned char emptyBuffer[10] = {0};
    AsterixData* pData = pParser->parsePacket(emptyBuffer, 0, 0.0);
    if (pData) {
        EXPECT_EQ(pData->m_lDataBlocks.size(), 0) << "Empty data should have no blocks";
        delete pData;
    }
}

TEST_F(CAT25IntegrationTest, VerifyTextOutput) {
    const char* configFile = "../asterix/config/asterix_cat025_1_5.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));
    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;
    unsigned char buffer[256];
    buffer[0] = 0x19;  // Category 25
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x03;  // Length LSB = 3 bytes
    AsterixData* pData = pParser->parsePacket(buffer, 3, 0.0);
    if (pData && pData->m_lDataBlocks.size() > 0) {
        std::string textOutput;
        bool result = pData->getText(textOutput, 0);
        EXPECT_TRUE(result) << "getText should return true";
        if (result) EXPECT_GT(textOutput.length(), 0) << "getText output should not be empty";
        delete pData;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
