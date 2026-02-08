/**
 * Integration test for output format validation
 *
 * This test verifies that all output formats (text, JSON, XML, extensive JSON)
 * produce valid output from parsed ASTERIX data.
 *
 * Requirements Coverage:
 * - REQ-HLR-OUT-001: Support text output format
 * - REQ-HLR-OUT-002: Support JSON output format
 * - REQ-HLR-OUT-003: Support XML output format
 * - REQ-HLR-OUT-004: Support extensive JSON output format
 *
 * Test Cases:
 * - TC-INT-OUT-001: Verify JSON output format
 * - TC-INT-OUT-002: Verify human-readable JSON output format
 * - TC-INT-OUT-003: Verify XML output format
 * - TC-INT-OUT-004: Verify human-readable XML output format
 * - TC-INT-OUT-005: Verify extensive JSON output format
 * - TC-INT-OUT-006: Verify line-per-item output format
 * - TC-INT-OUT-007: Verify all formats produce non-empty output
 */

#include <gtest/gtest.h>
#include "../../src/asterix/XMLParser.h"
#include "../../src/asterix/AsterixDefinition.h"
#include "../../src/asterix/InputParser.h"
#include "../../src/asterix/AsterixData.h"
#include "../../src/asterix/DataBlock.h"
#include "asterixformat.hxx"
#include <cstdio>
#include <cstring>
#include <fstream>

// Global variables required by ASTERIX library
bool gVerbose = false;
bool gFiltering = false;

class OutputFormatIntegrationTest : public ::testing::Test {
protected:
    AsterixDefinition* pDefinition;
    InputParser* pParser;
    AsterixData* pData;

    void SetUp() override {
        pDefinition = new AsterixDefinition();
        pParser = nullptr;
        pData = nullptr;
    }

    void TearDown() override {
        if (pData) {
            delete pData;
        }
        if (pParser) {
            delete pParser;
        } else if (pDefinition) {
            delete pDefinition;
        }
    }

    bool LoadAndParse() {
        // Load CAT048 config
        FILE* pBDSFile = fopen("../asterix/config/asterix_bds.xml", "r");
        if (pBDSFile) {
            XMLParser bdsParser;
            bdsParser.Parse(pBDSFile, pDefinition, "asterix_bds.xml");
            fclose(pBDSFile);
        }

        const char* configFile = "../asterix/config/asterix_cat048_1_30.xml";
        FILE* pFile = fopen(configFile, "r");
        if (!pFile) {
            return false;
        }

        XMLParser parser;
        bool result = parser.Parse(pFile, pDefinition, configFile);
        fclose(pFile);

        if (!result) return false;

        // Read sample data
        std::ifstream dataFile("../asterix/sample_data/cat048.raw", std::ios::binary | std::ios::ate);
        if (!dataFile.is_open()) {
            return false;
        }

        size_t size = dataFile.tellg();
        std::vector<unsigned char> buffer(size);
        dataFile.seekg(0, std::ios::beg);
        dataFile.read(reinterpret_cast<char*>(buffer.data()), size);
        dataFile.close();

        pParser = new InputParser(pDefinition);
        pDefinition = nullptr;

        pData = pParser->parsePacket(buffer.data(), size, 0.0);

        return pData != nullptr && pData->m_lDataBlocks.size() > 0;
    }
};

/**
 * Test Case: TC-INT-OUT-001
 * Verify JSON output format produces valid JSON-like output
 */
TEST_F(OutputFormatIntegrationTest, VerifyJSONOutput) {
    ASSERT_TRUE(LoadAndParse()) << "Failed to load and parse test data";

    std::string jsonOutput;
    bool result = pData->getText(jsonOutput, CAsterixFormat::EJSON);

    EXPECT_TRUE(result) << "getText with EJSON should succeed";
    EXPECT_GT(jsonOutput.length(), 0) << "JSON output should not be empty";

    // JSON output should contain braces
    EXPECT_NE(jsonOutput.find('{'), std::string::npos) << "JSON should contain opening brace";
    EXPECT_NE(jsonOutput.find('}'), std::string::npos) << "JSON should contain closing brace";
}

/**
 * Test Case: TC-INT-OUT-002
 * Verify human-readable JSON output format
 */
TEST_F(OutputFormatIntegrationTest, VerifyJSONHumanReadableOutput) {
    ASSERT_TRUE(LoadAndParse()) << "Failed to load and parse test data";

    std::string jsonhOutput;
    bool result = pData->getText(jsonhOutput, CAsterixFormat::EJSONH);

    EXPECT_TRUE(result) << "getText with EJSONH should succeed";
    EXPECT_GT(jsonhOutput.length(), 0) << "JSONH output should not be empty";

    // Human-readable JSON should contain newlines and indentation
    EXPECT_NE(jsonhOutput.find('\n'), std::string::npos) << "Human-readable JSON should contain newlines";
}

/**
 * Test Case: TC-INT-OUT-003
 * Verify XML output format
 */
TEST_F(OutputFormatIntegrationTest, VerifyXMLOutput) {
    ASSERT_TRUE(LoadAndParse()) << "Failed to load and parse test data";

    std::string xmlOutput;
    bool result = pData->getText(xmlOutput, CAsterixFormat::EXML);

    EXPECT_TRUE(result) << "getText with EXML should succeed";
    EXPECT_GT(xmlOutput.length(), 0) << "XML output should not be empty";

    // XML should contain angle brackets
    EXPECT_NE(xmlOutput.find('<'), std::string::npos) << "XML should contain opening angle bracket";
    EXPECT_NE(xmlOutput.find('>'), std::string::npos) << "XML should contain closing angle bracket";
}

/**
 * Test Case: TC-INT-OUT-004
 * Verify human-readable XML output format
 */
TEST_F(OutputFormatIntegrationTest, VerifyXMLHumanReadableOutput) {
    ASSERT_TRUE(LoadAndParse()) << "Failed to load and parse test data";

    std::string xmlhOutput;
    bool result = pData->getText(xmlhOutput, CAsterixFormat::EXMLH);

    EXPECT_TRUE(result) << "getText with EXMLH should succeed";
    EXPECT_GT(xmlhOutput.length(), 0) << "XMLH output should not be empty";

    // Human-readable XML should contain newlines
    EXPECT_NE(xmlhOutput.find('\n'), std::string::npos) << "Human-readable XML should contain newlines";
}

/**
 * Test Case: TC-INT-OUT-005
 * Verify extensive JSON output format (with descriptions)
 */
TEST_F(OutputFormatIntegrationTest, VerifyExtensiveJSONOutput) {
    ASSERT_TRUE(LoadAndParse()) << "Failed to load and parse test data";

    std::string jsoneOutput;
    bool result = pData->getText(jsoneOutput, CAsterixFormat::EJSONE);

    EXPECT_TRUE(result) << "getText with EJSONE should succeed";
    EXPECT_GT(jsoneOutput.length(), 0) << "Extensive JSON output should not be empty";

    // Extensive JSON should be larger than compact JSON (has descriptions)
    std::string jsonOutput;
    pData->getText(jsonOutput, CAsterixFormat::EJSON);

    EXPECT_GE(jsoneOutput.length(), jsonOutput.length())
        << "Extensive JSON should be at least as large as compact JSON";
}

/**
 * Test Case: TC-INT-OUT-006
 * Verify line-per-item output format
 */
TEST_F(OutputFormatIntegrationTest, VerifyLineOutput) {
    ASSERT_TRUE(LoadAndParse()) << "Failed to load and parse test data";

    std::string lineOutput;
    bool result = pData->getText(lineOutput, CAsterixFormat::ETxt);

    EXPECT_TRUE(result) << "getText with ETxt should succeed";
    EXPECT_GT(lineOutput.length(), 0) << "Line output should not be empty";
}

/**
 * Test Case: TC-INT-OUT-007
 * Verify all output formats produce non-empty output
 */
TEST_F(OutputFormatIntegrationTest, AllFormatsProduceOutput) {
    ASSERT_TRUE(LoadAndParse()) << "Failed to load and parse test data";

    // Test all format types that produce output
    int formats[] = {
        CAsterixFormat::ETxt,
        CAsterixFormat::EXML,
        CAsterixFormat::EXMLH,
        CAsterixFormat::EJSON,
        CAsterixFormat::EJSONH,
        CAsterixFormat::EJSONE,
    };

    for (int fmt : formats) {
        std::string output;
        bool result = pData->getText(output, fmt);
        EXPECT_TRUE(result) << "getText should succeed for format " << fmt;
        EXPECT_GT(output.length(), 0) << "Output should not be empty for format " << fmt;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
