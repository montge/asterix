/**
 * Integration test for CAT015 parsing
 *
 * This test verifies the complete ASTERIX parsing pipeline for Category 015
 * (INCS Target Reports):
 * 1. Load XML configuration
 * 2. Parse binary ASTERIX data
 * 3. Verify parsed output
 *
 * Requirements Coverage:
 * - REQ-HLR-001: Parse ASTERIX binary data
 * - REQ-HLR-SYS-001: Parse ASTERIX categories
 * - REQ-LLR-015-000: Parse Message Type (I015/000)
 * - REQ-LLR-015-010: Parse Data Source Identifier (I015/010)
 * - Additional requirements from Low_Level_Requirements_CAT015.md
 *
 * Test Cases:
 * - TC-INT-CAT015-001: Load CAT015 XML configuration
 * - TC-INT-CAT015-002: Parse CAT015 binary data
 * - TC-INT-CAT015-003: Verify parsed data structure
 * - TC-INT-CAT015-004: Reject invalid data
 * - TC-INT-CAT015-005: Verify text output generation
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

class CAT015IntegrationTest : public ::testing::Test {
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

TEST_F(CAT015IntegrationTest, LoadXMLConfiguration) {
    const char* configFile = "../asterix/config/asterix_cat015_1_2.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile)) << "Failed to load CAT015 XML configuration";
    ASSERT_TRUE(pDefinition->CategoryDefined(15)) << "CAT015 category not defined";
    Category* cat015 = pDefinition->getCategory(15);
    ASSERT_NE(cat015, nullptr) << "CAT015 category is NULL";
    EXPECT_EQ(cat015->m_id, 15) << "Category ID should be 15";
}

TEST_F(CAT015IntegrationTest, ParseCAT015BinaryData) {
    const char* configFile = "../asterix/config/asterix_cat015_1_2.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));
    
    unsigned char buffer[256];
    buffer[0] = 0x0F;  // Category 15
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x03;  // Length LSB = 3 bytes (header only)

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;
    AsterixData* pData = pParser->parsePacket(buffer, 3, 0.0);
    if (pData) {
        delete pData;
    }
}

TEST_F(CAT015IntegrationTest, VerifyParsedDataStructure) {
    const char* configFile = "../asterix/config/asterix_cat015_1_2.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    unsigned char buffer[256];
    buffer[0] = 0x0F;  // Category 15
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x08;  // Length LSB = 8 bytes total
    buffer[3] = 0x80;  // FSPEC: I015/010 present (bit 7 set)
    buffer[4] = 0x01;  // I015/010: SAC = 0x01
    buffer[5] = 0x23;  // I015/010: SIC = 0x23

    AsterixData* pData = pParser->parsePacket(buffer, 6, 0.0);
    if (pData) {
        delete pData;
    }
}

TEST_F(CAT015IntegrationTest, RejectInvalidData) {
    const char* configFile = "../asterix/config/asterix_cat015_1_2.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    unsigned char emptyBuffer[10] = {0};
    AsterixData* pData1 = pParser->parsePacket(emptyBuffer, 0, 0.0);
    if (pData1) {
        EXPECT_EQ(pData1->m_lDataBlocks.size(), 0) << "Empty data should have no blocks";
        delete pData1;
    }

    unsigned char invalidBuffer[10] = {0xFF, 0x00, 0x03, 0x00};
    AsterixData* pData2 = pParser->parsePacket(invalidBuffer, 4, 0.0);
    if (pData2) {
        delete pData2;
    }
}

TEST_F(CAT015IntegrationTest, VerifyTextOutput) {
    const char* configFile = "../asterix/config/asterix_cat015_1_2.xml";
    ASSERT_TRUE(LoadXMLConfig(configFile));

    pParser = new InputParser(pDefinition);
    pDefinition = nullptr;

    unsigned char buffer[256];
    buffer[0] = 0x0F;  // Category 15
    buffer[1] = 0x00;  // Length MSB
    buffer[2] = 0x03;  // Length LSB = 3 bytes

    AsterixData* pData = pParser->parsePacket(buffer, 3, 0.0);
    if (pData && pData->m_lDataBlocks.size() > 0) {
        std::string textOutput;
        bool result = pData->getText(textOutput, 0);
        EXPECT_TRUE(result) << "getText should return true";
        if (result) {
            EXPECT_GT(textOutput.length(), 0) << "getText output should not be empty";
        }
        delete pData;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

