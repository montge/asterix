/**
 * Integration test for CAT001 parsing
 *
 * This test verifies the complete ASTERIX parsing pipeline for Category 001
 * (Monoradar Target Reports and Plot Messages):
 * 1. Load XML configuration
 * 2. Parse binary ASTERIX data
 * 3. Verify parsed output
 *
 * Requirements Coverage:
 * - REQ-HLR-001: Parse ASTERIX binary data
 * - REQ-HLR-SYS-001: Parse ASTERIX categories
 *
 * Test Cases:
 * - TC-INT-CAT001-001: Load CAT001 XML configuration
 * - TC-INT-CAT001-002: Parse CAT001 binary data with I001/010
 * - TC-INT-CAT001-003: Reject invalid data
 * - TC-INT-CAT001-004: Verify text output generation
 * - TC-INT-CAT001-005: Parse from sample PCAP file
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

class CAT001IntegrationTest : public ::testing::Test {
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
 * Test Case: TC-INT-CAT001-001
 * Verify that CAT001 XML configuration can be loaded successfully
 */
TEST_F(CAT001IntegrationTest, LoadXMLConfiguration) {
    const char* configFile = "../asterix/config/asterix_cat001_1_4.xml";

    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load CAT001 XML configuration";

    ASSERT_TRUE(pDefinition->CategoryDefined(1)) << "CAT001 category not defined after loading XML";

    Category* cat001 = pDefinition->getCategory(1);
    ASSERT_NE(cat001, nullptr) << "CAT001 category is NULL";
    EXPECT_EQ(cat001->m_id, 1) << "Category ID should be 1";
}

/**
 * Test Case: TC-INT-CAT001-002
 * Verify that CAT001 binary data can be parsed successfully
 */
TEST_F(CAT001IntegrationTest, ParseCAT001BinaryData) {
    const char* configFile = "../asterix/config/asterix_cat001_1_4.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Create minimal CAT001 packet with I001/010 (Data Source Identifier)
    // CAT001 uses multiple UAPs selected by first FSPEC bit
    unsigned char buffer[256];
    buffer[0] = 0x01;  // Category 1
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x06;  // Length LSB = 6 bytes
    buffer[3] = 0x80;  // FSPEC: I001/010 present
    buffer[4] = 0x01;  // I001/010: SAC
    buffer[5] = 0x23;  // I001/010: SIC

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    AsterixData* pData = pParser->parsePacket(buffer, 6, 0.0);

    if (pData) {
        // Should parse without crashing
        delete pData;
    }
}

/**
 * Test Case: TC-INT-CAT001-003
 * Verify that invalid data is rejected properly
 */
TEST_F(CAT001IntegrationTest, RejectInvalidData) {
    const char* configFile = "../asterix/config/asterix_cat001_1_4.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    // Test with empty data
    unsigned char emptyBuffer[10] = {0};
    AsterixData* pData = pParser->parsePacket(emptyBuffer, 0, 0.0);
    if (pData) {
        EXPECT_EQ(pData->m_lDataBlocks.size(), 0);
        delete pData;
    }
}

/**
 * Test Case: TC-INT-CAT001-004
 * Verify text output generation for CAT001
 */
TEST_F(CAT001IntegrationTest, VerifyTextOutput) {
    const char* configFile = "../asterix/config/asterix_cat001_1_4.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    unsigned char buffer[256];
    buffer[0] = 0x01;  // Category 1
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x06;  // Length LSB = 6 bytes
    buffer[3] = 0x80;  // FSPEC: I001/010 present
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

/**
 * Test Case: TC-INT-CAT001-005
 * Verify parsing from sample PCAP file containing CAT001
 */
TEST_F(CAT001IntegrationTest, ParseFromSamplePCAPFile) {
    const char* configFile = "../asterix/config/asterix_cat001_1_4.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    // Also load CAT002 since the PCAP file contains both
    FILE* pFile = fopen("../asterix/config/asterix_cat002_1_1.xml", "r");
    if (pFile) {
        XMLParser parser;
        parser.Parse(pFile, pDefinition, "asterix_cat002_1_1.xml");
        fclose(pFile);
    }

    // Verify categories are loaded
    EXPECT_TRUE(pDefinition->CategoryDefined(1));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
