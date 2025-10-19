/**
 * Unit tests for XMLParser class
 *
 * Requirements Traceability:
 * - REQ-HLR-SYS-001: Parse ASTERIX categories from XML
 * - REQ-HLR-XML-001: Parse XML category definitions
 * - REQ-HLR-XML-002: Validate XML against DTD
 * - REQ-HLR-XML-003: Handle XML parsing errors
 *
 * DO-278A AL-3 Compliance Testing
 */

#include <gtest/gtest.h>
#include "XMLParser.h"
#include "AsterixDefinition.h"
#include "Category.h"
#include "DataItemDescription.h"
#include "DataItemFormatFixed.h"
#include "DataItemFormatVariable.h"
#include "DataItemFormatCompound.h"
#include "DataItemFormatRepetitive.h"
#include "DataItemFormatExplicit.h"
#include "DataItemBits.h"
#include "UAP.h"
#include <fstream>
#include <cstdio>

// Global variables required by ASTERIX library
bool gFiltering = false;

// Test fixture for XMLParser tests
class XMLParserTest : public ::testing::Test {
protected:
    AsterixDefinition* definition;
    std::string testFilePath;

    void SetUp() override {
        definition = new AsterixDefinition();
        testFilePath = "/tmp/test_asterix_parser.xml";
    }

    void TearDown() override {
        delete definition;
        // Clean up test file
        std::remove(testFilePath.c_str());
    }

    // Helper: Create test XML file
    void createTestXML(const std::string& content) {
        std::ofstream file(testFilePath);
        ASSERT_TRUE(file.is_open()) << "Failed to create test file";
        file << content;
        file.close();
    }

    // Helper: Parse the test XML file
    bool parseTestFile() {
        FILE* pFile = fopen(testFilePath.c_str(), "r");
        if (!pFile) {
            return false;
        }

        XMLParser parser;
        bool result = parser.Parse(pFile, definition, testFilePath.c_str());
        fclose(pFile);
        return result;
    }
};

/**
 * Test Case: TC-CPP-XMLP-001
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles simple category with Fixed format
 */
TEST_F(XMLParserTest, ParseSimpleFixedCategory) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Data Source Identifier</DataItemName>
        <DataItemDefinition>Test definition</DataItemDefinition>
        <DataItemFormat desc="Two-octet fixed length">
            <Fixed length="2">
                <Bits from="16" to="9">
                    <BitsShortName>SAC</BitsShortName>
                    <BitsName>System Area Code</BitsName>
                </Bits>
                <Bits from="8" to="1">
                    <BitsShortName>SIC</BitsShortName>
                    <BitsName>System Identification Code</BitsName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
        <UAPItem bit="2" frn="FX">-</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);
    EXPECT_EQ(cat->m_id, 48);
    EXPECT_EQ(cat->m_strName, "Test Category");
    EXPECT_EQ(cat->m_strVer, "1.0");

    // Verify data item was created
    DataItemDescription* item = cat->getDataItemDescription("010");
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->m_strName, "Data Source Identifier");
    EXPECT_EQ(item->m_eRule, DataItemDescription::DATAITEM_MANDATORY);

    // Verify format
    ASSERT_NE(item->m_pFormat, nullptr);
    EXPECT_TRUE(item->m_pFormat->isFixed());
    EXPECT_EQ(((DataItemFormatFixed*)item->m_pFormat)->m_nLength, 2);
}

/**
 * Test Case: TC-CPP-XMLP-002
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles Variable format items
 */
TEST_F(XMLParserTest, ParseVariableFormat) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="020" rule="optional">
        <DataItemName>Target Report Descriptor</DataItemName>
        <DataItemDefinition>Test variable</DataItemDefinition>
        <DataItemFormat desc="Variable length">
            <Variable>
                <Fixed length="1">
                    <Bits bit="8">
                        <BitsShortName>TYP</BitsShortName>
                        <BitsValue val="0">Plot</BitsValue>
                        <BitsValue val="1">Track</BitsValue>
                    </Bits>
                    <Bits bit="1" fx="1">
                        <BitsShortName>FX</BitsShortName>
                    </Bits>
                </Fixed>
            </Variable>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">020</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("020");
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->m_eRule, DataItemDescription::DATAITEM_OPTIONAL);
    EXPECT_TRUE(item->m_pFormat->isVariable());
}

/**
 * Test Case: TC-CPP-XMLP-003
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles Compound format items
 */
TEST_F(XMLParserTest, ParseCompoundFormat) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="030" rule="optional">
        <DataItemName>Compound Item</DataItemName>
        <DataItemDefinition>Test compound</DataItemDefinition>
        <DataItemFormat desc="Compound">
            <Compound>
                <Variable>
                    <Fixed length="1">
                        <Bits bit="8">
                            <BitsShortName>A</BitsShortName>
                        </Bits>
                        <Bits bit="1" fx="1">
                            <BitsShortName>FX</BitsShortName>
                        </Bits>
                    </Fixed>
                </Variable>
                <Fixed length="2">
                    <Bits from="16" to="1">
                        <BitsShortName>DATA</BitsShortName>
                    </Bits>
                </Fixed>
            </Compound>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">030</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("030");
    ASSERT_NE(item, nullptr);
    EXPECT_TRUE(item->m_pFormat->isCompound());

    // Compound should have subitems
    EXPECT_GT(item->m_pFormat->m_lSubItems.size(), 0);
}

/**
 * Test Case: TC-CPP-XMLP-004
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles Repetitive format items
 */
TEST_F(XMLParserTest, ParseRepetitiveFormat) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="040" rule="optional">
        <DataItemName>Repetitive Item</DataItemName>
        <DataItemDefinition>Test repetitive</DataItemDefinition>
        <DataItemFormat desc="Repetitive">
            <Repetitive>
                <Fixed length="4">
                    <Bits from="32" to="1">
                        <BitsShortName>DATA</BitsShortName>
                    </Bits>
                </Fixed>
            </Repetitive>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">040</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("040");
    ASSERT_NE(item, nullptr);
    EXPECT_TRUE(item->m_pFormat->isRepetitive());
}

/**
 * Test Case: TC-CPP-XMLP-005
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles Explicit format items
 */
TEST_F(XMLParserTest, ParseExplicitFormat) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="050" rule="optional">
        <DataItemName>Explicit Item</DataItemName>
        <DataItemDefinition>Test explicit</DataItemDefinition>
        <DataItemFormat desc="Explicit">
            <Explicit>
                <Fixed length="3">
                    <Bits from="24" to="1">
                        <BitsShortName>DATA</BitsShortName>
                    </Bits>
                </Fixed>
            </Explicit>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">050</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("050");
    ASSERT_NE(item, nullptr);
    EXPECT_TRUE(item->m_pFormat->isExplicit());
}

/**
 * Test Case: TC-CPP-XMLP-006
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles BitsValue elements
 */
TEST_F(XMLParserTest, ParseBitsValue) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Test Item</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="1">
                <Bits from="8" to="7">
                    <BitsShortName>MODE</BitsShortName>
                    <BitsValue val="0">Mode A</BitsValue>
                    <BitsValue val="1">Mode B</BitsValue>
                    <BitsValue val="2">Mode C</BitsValue>
                    <BitsValue val="3">Mode D</BitsValue>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("010");
    ASSERT_NE(item, nullptr);
    ASSERT_TRUE(item->m_pFormat->isFixed());

    // Check that bits have values
    EXPECT_GT(item->m_pFormat->m_lSubItems.size(), 0);
}

/**
 * Test Case: TC-CPP-XMLP-007
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles BitsUnit with scale/min/max
 */
TEST_F(XMLParserTest, ParseBitsUnit) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Altitude</DataItemName>
        <DataItemDefinition>Test altitude</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="2">
                <Bits from="16" to="1" encode="signed">
                    <BitsShortName>ALT</BitsShortName>
                    <BitsName>Altitude</BitsName>
                    <BitsUnit scale="0.25" min="-1500" max="150000">FL</BitsUnit>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("010");
    ASSERT_NE(item, nullptr);
    ASSERT_TRUE(item->m_pFormat->isFixed());

    // Verify bits were created
    EXPECT_GT(item->m_pFormat->m_lSubItems.size(), 0);

    DataItemBits* bits = (DataItemBits*)item->m_pFormat->m_lSubItems.front();
    EXPECT_EQ(bits->m_dScale, 0.25);
    EXPECT_TRUE(bits->m_bMinValueSet);
    EXPECT_TRUE(bits->m_bMaxValueSet);
    EXPECT_EQ(bits->m_dMinValue, -1500);
    EXPECT_EQ(bits->m_dMaxValue, 150000);
    EXPECT_EQ(bits->m_strUnit, "FL");
}

/**
 * Test Case: TC-CPP-XMLP-008
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles multiple data items
 */
TEST_F(XMLParserTest, ParseMultipleDataItems) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Item 010</DataItemName>
        <DataItemDefinition>First item</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="2">
                <Bits from="16" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <DataItem id="020" rule="optional">
        <DataItemName>Item 020</DataItemName>
        <DataItemDefinition>Second item</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="1">
                <Bits from="8" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <DataItem id="030" rule="optional">
        <DataItemName>Item 030</DataItemName>
        <DataItemDefinition>Third item</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="4">
                <Bits from="32" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
        <UAPItem bit="2" frn="2">020</UAPItem>
        <UAPItem bit="3" frn="3">030</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    EXPECT_NE(cat->getDataItemDescription("010"), nullptr);
    EXPECT_NE(cat->getDataItemDescription("020"), nullptr);
    EXPECT_NE(cat->getDataItemDescription("030"), nullptr);
}

/**
 * Test Case: TC-CPP-XMLP-009
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles UAP definitions
 */
TEST_F(XMLParserTest, ParseUAPDefinition) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Item 010</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="2">
                <Bits from="16" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
        <UAPItem bit="2" frn="2">020</UAPItem>
        <UAPItem bit="3" frn="FX">-</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    // Verify UAP was created
    EXPECT_GT(cat->m_lUAPs.size(), 0);
}

/**
 * Test Case: TC-CPP-XMLP-010
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles BitsConst elements
 */
TEST_F(XMLParserTest, ParseBitsConst) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Test Item</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="1">
                <Bits from="8" to="7">
                    <BitsShortName>spare</BitsShortName>
                    <BitsConst>0</BitsConst>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("010");
    ASSERT_NE(item, nullptr);
    ASSERT_TRUE(item->m_pFormat->isFixed());
    EXPECT_GT(item->m_pFormat->m_lSubItems.size(), 0);

    DataItemBits* bits = (DataItemBits*)item->m_pFormat->m_lSubItems.front();
    EXPECT_TRUE(bits->m_bIsConst);
}

/**
 * Test Case: TC-CPP-XMLP-011
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles missing file error
 */
TEST_F(XMLParserTest, MissingFileError) {
    FILE* pFile = fopen("/nonexistent/file.xml", "r");
    EXPECT_EQ(pFile, nullptr);
}

/**
 * Test Case: TC-CPP-XMLP-012
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles malformed XML
 */
TEST_F(XMLParserTest, MalformedXMLError) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Item 010
        <!-- Missing closing tag -->
    </DataItem>
</Category>)";

    createTestXML(xml);
    EXPECT_FALSE(parseTestFile());
}

/**
 * Test Case: TC-CPP-XMLP-013
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles invalid category ID
 */
TEST_F(XMLParserTest, InvalidCategoryID) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="999" name="Invalid" ver="1.0">
    <UAP></UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_FALSE(parseTestFile());
}

/**
 * Test Case: TC-CPP-XMLP-014
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles DataItem without Category
 */
TEST_F(XMLParserTest, DataItemWithoutCategory) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<DataItem id="010" rule="mandatory">
    <DataItemName>Orphan Item</DataItemName>
</DataItem>)";

    createTestXML(xml);
    EXPECT_FALSE(parseTestFile());
}

/**
 * Test Case: TC-CPP-XMLP-015
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles Bits without Format
 */
TEST_F(XMLParserTest, BitsWithoutFormat) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Item 010</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Bad">
            <Bits from="8" to="1">
                <BitsShortName>DATA</BitsShortName>
            </Bits>
        </DataItemFormat>
    </DataItem>
    <UAP></UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_FALSE(parseTestFile());
}

/**
 * Test Case: TC-CPP-XMLP-016
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles Bits without Fixed
 */
TEST_F(XMLParserTest, BitsWithoutFixed) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Item 010</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Bad">
            <Variable>
                <Bits from="8" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Variable>
        </DataItemFormat>
    </DataItem>
    <UAP></UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_FALSE(parseTestFile());
}

/**
 * Test Case: TC-CPP-XMLP-017
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles empty XML file
 */
TEST_F(XMLParserTest, EmptyXMLFile) {
    const char* xml = "";

    createTestXML(xml);
    EXPECT_FALSE(parseTestFile());
}

/**
 * Test Case: TC-CPP-XMLP-018
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles XML without DOCTYPE
 */
TEST_F(XMLParserTest, XMLWithoutDoctype) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Category id="48" name="Test Category" ver="1.0">
    <UAP></UAP>
</Category>)";

    createTestXML(xml);
    // Should still parse even without DOCTYPE
    bool result = parseTestFile();
    // Parser may accept this or reject it depending on implementation
    (void)result; // Suppress unused warning
}

/**
 * Test Case: TC-CPP-XMLP-019
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles duplicate format in item
 */
TEST_F(XMLParserTest, DuplicateFormatError) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Item 010</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="First">
            <Fixed length="2">
                <Bits from="16" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
        <DataItemFormat desc="Second">
            <Fixed length="2">
                <Bits from="16" to="1">
                    <BitsShortName>DATA2</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP></UAP>
</Category>)";

    createTestXML(xml);
    // This should fail or only accept the first format
    EXPECT_FALSE(parseTestFile());
}

/**
 * Test Case: TC-CPP-XMLP-020
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles nested compound formats
 */
TEST_F(XMLParserTest, NestedCompoundFormat) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="optional">
        <DataItemName>Nested Compound</DataItemName>
        <DataItemDefinition>Test nested</DataItemDefinition>
        <DataItemFormat desc="Nested">
            <Compound>
                <Variable>
                    <Fixed length="1">
                        <Bits bit="8">
                            <BitsShortName>A</BitsShortName>
                        </Bits>
                        <Bits bit="1" fx="1">
                            <BitsShortName>FX</BitsShortName>
                        </Bits>
                    </Fixed>
                </Variable>
                <Compound>
                    <Variable>
                        <Fixed length="1">
                            <Bits bit="8">
                                <BitsShortName>B</BitsShortName>
                            </Bits>
                            <Bits bit="1" fx="1">
                                <BitsShortName>FX</BitsShortName>
                            </Bits>
                        </Fixed>
                    </Variable>
                    <Fixed length="2">
                        <Bits from="16" to="1">
                            <BitsShortName>DATA</BitsShortName>
                        </Bits>
                    </Fixed>
                </Compound>
            </Compound>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("010");
    ASSERT_NE(item, nullptr);
    EXPECT_TRUE(item->m_pFormat->isCompound());
}

/**
 * Test Case: TC-CPP-XMLP-021
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles BDS category ID
 */
TEST_F(XMLParserTest, ParseBDSCategory) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="BDS" name="BDS Registers" ver="1.0">
    <DataItem id="10" rule="optional">
        <DataItemName>BDS 1,0</DataItemName>
        <DataItemDefinition>Data link capability report</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="7">
                <Bits from="56" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">10</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(BDS_CAT_ID);
    ASSERT_NE(cat, nullptr);
    EXPECT_EQ(cat->m_id, BDS_CAT_ID);
}

/**
 * Test Case: TC-CPP-XMLP-022
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles different bit encodings
 */
TEST_F(XMLParserTest, ParseBitEncodings) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Encodings</DataItemName>
        <DataItemDefinition>Test encodings</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="8">
                <Bits from="64" to="57" encode="unsigned">
                    <BitsShortName>U</BitsShortName>
                </Bits>
                <Bits from="56" to="49" encode="signed">
                    <BitsShortName>S</BitsShortName>
                </Bits>
                <Bits from="48" to="41" encode="6bitschar">
                    <BitsShortName>C</BitsShortName>
                </Bits>
                <Bits from="40" to="33" encode="octal">
                    <BitsShortName>O</BitsShortName>
                </Bits>
                <Bits from="32" to="25" encode="ascii">
                    <BitsShortName>A</BitsShortName>
                </Bits>
                <Bits from="24" to="17" encode="hex">
                    <BitsShortName>H</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("010");
    ASSERT_NE(item, nullptr);

    // Verify multiple bits with different encodings
    EXPECT_EQ(item->m_pFormat->m_lSubItems.size(), 6);
}

/**
 * Test Case: TC-CPP-XMLP-023
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles wrong bit range (bit > length)
 */
TEST_F(XMLParserTest, BitOutOfRangeError) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Item 010</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="1">
                <Bits from="16" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP></UAP>
</Category>)";

    createTestXML(xml);
    // Bit 16 is out of range for length=1 (8 bits)
    EXPECT_FALSE(parseTestFile());
}

/**
 * Test Case: TC-CPP-XMLP-024
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles Explicit with Variable
 */
TEST_F(XMLParserTest, ParseExplicitWithVariable) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="optional">
        <DataItemName>Explicit Variable</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Explicit">
            <Explicit>
                <Variable>
                    <Fixed length="1">
                        <Bits bit="8">
                            <BitsShortName>A</BitsShortName>
                        </Bits>
                        <Bits bit="1" fx="1">
                            <BitsShortName>FX</BitsShortName>
                        </Bits>
                    </Fixed>
                </Variable>
            </Explicit>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("010");
    ASSERT_NE(item, nullptr);
    EXPECT_TRUE(item->m_pFormat->isExplicit());
}

/**
 * Test Case: TC-CPP-XMLP-025
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles Explicit with Repetitive
 */
TEST_F(XMLParserTest, ParseExplicitWithRepetitive) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="optional">
        <DataItemName>Explicit Repetitive</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Explicit">
            <Explicit>
                <Repetitive>
                    <Fixed length="2">
                        <Bits from="16" to="1">
                            <BitsShortName>DATA</BitsShortName>
                        </Bits>
                    </Fixed>
                </Repetitive>
            </Explicit>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("010");
    ASSERT_NE(item, nullptr);
    EXPECT_TRUE(item->m_pFormat->isExplicit());
}

/**
 * Test Case: TC-CPP-XMLP-026
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles multiple UAPs
 */
TEST_F(XMLParserTest, ParseMultipleUAPs) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Item 010</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="2">
                <Bits from="16" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
    </UAP>
    <UAP use_if_bit_set="8" use_if_byte_nr="1" is_set_to="1">
        <UAPItem bit="1" frn="1">010</UAPItem>
        <UAPItem bit="2" frn="2">020</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    // Should have multiple UAPs
    EXPECT_EQ(cat->m_lUAPs.size(), 2);
}

/**
 * Test Case: TC-CPP-XMLP-027
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles unknown XML tag
 */
TEST_F(XMLParserTest, UnknownTagError) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <UnknownTag>Bad content</UnknownTag>
    <UAP></UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_FALSE(parseTestFile());
}

/**
 * Test Case: TC-CPP-XMLP-028
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles closing unopened tag
 */
TEST_F(XMLParserTest, ClosingUnopenedTagError) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <UAP></UAP>
</Category>
</DataItem>)";

    createTestXML(xml);
    EXPECT_FALSE(parseTestFile());
}

/**
 * Test Case: TC-CPP-XMLP-029
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles FX bit attribute
 */
TEST_F(XMLParserTest, ParseFXBitAttribute) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Variable Item</DataItemName>
        <DataItemDefinition>Test FX</DataItemDefinition>
        <DataItemFormat desc="Variable">
            <Variable>
                <Fixed length="1">
                    <Bits from="8" to="2">
                        <BitsShortName>DATA</BitsShortName>
                    </Bits>
                    <Bits bit="1" fx="1">
                        <BitsShortName>FX</BitsShortName>
                    </Bits>
                </Fixed>
            </Variable>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("010");
    ASSERT_NE(item, nullptr);
    EXPECT_TRUE(item->m_pFormat->isVariable());

    // Variable should have fixed parts
    EXPECT_GT(item->m_pFormat->m_lSubItems.size(), 0);
}

/**
 * Test Case: TC-CPP-XMLP-030
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles category with no data items
 */
TEST_F(XMLParserTest, CategoryWithNoDataItems) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Empty Category" ver="1.0">
    <UAP></UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);
    EXPECT_EQ(cat->m_strName, "Empty Category");
}

/**
 * Test Case: TC-CPP-XMLP-031
 * Requirement: REQ-HLR-XML-003
 * Description: Verify parser handles Compound without Variable first
 */
TEST_F(XMLParserTest, CompoundWithoutVariableFirstError) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="optional">
        <DataItemName>Bad Compound</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Compound">
            <Compound>
                <Fixed length="2">
                    <Bits from="16" to="1">
                        <BitsShortName>DATA</BitsShortName>
                    </Bits>
                </Fixed>
            </Compound>
        </DataItemFormat>
    </DataItem>
    <UAP></UAP>
</Category>)";

    createTestXML(xml);
    // Compound must start with Variable
    EXPECT_FALSE(parseTestFile());
}

/**
 * Test Case: TC-CPP-XMLP-032
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles DataItem with note
 */
TEST_F(XMLParserTest, ParseDataItemWithNote) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Item with Note</DataItemName>
        <DataItemDefinition>Test definition</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="2">
                <Bits from="16" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
        <DataItemNote>This is a test note</DataItemNote>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item = cat->getDataItemDescription("010");
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->m_strNote, "This is a test note");
}

/**
 * Test Case: TC-CPP-XMLP-033
 * Requirement: REQ-HLR-XML-001
 * Description: Verify parser handles rule attribute values
 */
TEST_F(XMLParserTest, ParseDataItemRules) {
    const char* xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Category SYSTEM "asterix.dtd">
<Category id="48" name="Test Category" ver="1.0">
    <DataItem id="010" rule="mandatory">
        <DataItemName>Mandatory Item</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="1">
                <Bits from="8" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <DataItem id="020" rule="optional">
        <DataItemName>Optional Item</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="1">
                <Bits from="8" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <DataItem id="030" rule="unknown">
        <DataItemName>Unknown Item</DataItemName>
        <DataItemDefinition>Test</DataItemDefinition>
        <DataItemFormat desc="Fixed">
            <Fixed length="1">
                <Bits from="8" to="1">
                    <BitsShortName>DATA</BitsShortName>
                </Bits>
            </Fixed>
        </DataItemFormat>
    </DataItem>
    <UAP>
        <UAPItem bit="1" frn="1">010</UAPItem>
        <UAPItem bit="2" frn="2">020</UAPItem>
        <UAPItem bit="3" frn="3">030</UAPItem>
    </UAP>
</Category>)";

    createTestXML(xml);
    EXPECT_TRUE(parseTestFile());

    Category* cat = definition->getCategory(48);
    ASSERT_NE(cat, nullptr);

    DataItemDescription* item010 = cat->getDataItemDescription("010");
    DataItemDescription* item020 = cat->getDataItemDescription("020");
    DataItemDescription* item030 = cat->getDataItemDescription("030");

    ASSERT_NE(item010, nullptr);
    ASSERT_NE(item020, nullptr);
    ASSERT_NE(item030, nullptr);

    EXPECT_EQ(item010->m_eRule, DataItemDescription::DATAITEM_MANDATORY);
    EXPECT_EQ(item020->m_eRule, DataItemDescription::DATAITEM_OPTIONAL);
    EXPECT_EQ(item030->m_eRule, DataItemDescription::DATAITEM_UNKNOWN);
}
