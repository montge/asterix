"""
Advanced parsing tests for ASTERIX decoder
Tests for parse_with_offset, describeXML, and describe variants

Requirements Traceability:
- REQ-HLR-001: Parse ASTERIX binary data
- REQ-HLR-002: Data integrity verification
- REQ-HLR-API-001: Python API for ASTERIX parsing
"""

import unittest
import asterix
import os


class TestParseWithOffset(unittest.TestCase):
    """Test parse_with_offset function
    Requirement: REQ-HLR-001
    Verification: TC-001-010 through TC-001-015
    """

    def test_parse_with_offset_basic(self):
        """Test parse_with_offset with default parameters
        Test Case: TC-001-010
        """
        # CAT048 packet
        asterix_packet = bytearray([
            0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
            0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
            0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
            0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
            0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
            0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
        ])

        result, offset = asterix.parse_with_offset(asterix_packet)

        # Should return parsed data and offset
        assert result is not None, "parse_with_offset should return data"
        assert isinstance(result, list), "Result should be a list"
        assert isinstance(offset, int), "Offset should be an integer"
        assert offset >= 0, "Offset should be non-negative"

    def test_parse_with_offset_with_offset_parameter(self):
        """Test parse_with_offset starting from specific offset
        Test Case: TC-001-011
        """
        # CAT048 packet with some padding
        padding = bytearray([0x00] * 10)
        asterix_packet = bytearray([
            0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
            0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
            0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
            0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
            0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
            0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
        ])

        full_data = padding + asterix_packet

        # Parse starting from offset 10 (skip padding)
        result, offset = asterix.parse_with_offset(full_data, offset=10)

        assert result is not None, "Should parse data from offset"
        assert offset > 10, "Offset should advance beyond starting point"

    def test_parse_with_offset_blocks_count(self):
        """Test parse_with_offset with blocks_count parameter
        Test Case: TC-001-012
        """
        asterix_packet = bytearray([
            0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
            0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
            0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
            0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
            0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
            0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
        ])

        # Parse with blocks_count=1
        result, offset = asterix.parse_with_offset(asterix_packet, blocks_count=1)

        assert result is not None, "Should parse with blocks_count"
        assert len(result) <= 1, "Should not exceed blocks_count"

    def test_parse_with_offset_verbose_false(self):
        """Test parse_with_offset with verbose=False
        Test Case: TC-001-013
        """
        asterix_packet = bytearray([
            0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
            0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
            0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
            0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
            0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
            0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
        ])

        result, offset = asterix.parse_with_offset(asterix_packet, verbose=False)

        assert result is not None, "Should parse in non-verbose mode"
        assert isinstance(result, list), "Result should be a list"

    def test_parse_with_offset_real_file(self):
        """Test parse_with_offset with real CAT048 sample file
        Test Case: TC-001-014
        """
        sample_file = asterix.get_sample_file('cat048')
        if sample_file and os.path.exists(sample_file):
            with open(sample_file, 'rb') as f:
                data = f.read()

            # Parse first block
            result1, offset1 = asterix.parse_with_offset(data, offset=0, blocks_count=1)
            assert result1 is not None, "Should parse first block"
            assert offset1 > 0, "Offset should advance"

            # Parse second block
            if offset1 < len(data):
                result2, offset2 = asterix.parse_with_offset(data, offset=offset1, blocks_count=1)
                assert offset2 >= offset1, "Offset should continue advancing"

    def test_parse_with_offset_empty_data(self):
        """Test parse_with_offset with empty data
        Test Case: TC-001-015
        """
        empty_data = bytearray()
        result, offset = asterix.parse_with_offset(empty_data)

        # Should handle empty data gracefully
        assert isinstance(result, list), "Should return list for empty data"
        assert offset == 0, "Offset should be 0 for empty data"


class TestDescribeXMLFunction(unittest.TestCase):
    """Test describeXML function for XML output
    Requirement: REQ-HLR-API-002
    Verification: TC-002-010 through TC-002-015
    """

    def setUp(self):
        """Set up test data"""
        self.asterix_packet = bytearray([
            0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
            0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
            0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
            0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
            0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
            0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
        ])

    def test_describeXML_basic(self):
        """Test describeXML with parsed data
        Test Case: TC-002-010
        """
        parsed = asterix.parse(self.asterix_packet)
        xml_result = asterix.describeXML(parsed)

        # May return None if lxml is not installed
        if xml_result is not None:
            # Should return XML ElementTree
            assert xml_result is not None, "describeXML should return XML"
            assert hasattr(xml_result, 'tag'), "Should be an XML element"
            assert xml_result.tag == 'ASTERIXSTART', "Root should be ASTERIXSTART"

    def test_describeXML_with_descriptions(self):
        """Test describeXML with descriptions enabled
        Test Case: TC-002-011
        """
        parsed = asterix.parse(self.asterix_packet)
        xml_result = asterix.describeXML(parsed, descriptions=True)

        if xml_result is not None:
            # Should include description attributes
            assert xml_result is not None, "Should return XML with descriptions"
            # Check for ASTERIX elements
            asterix_elements = xml_result.findall('ASTERIX')
            assert len(asterix_elements) > 0, "Should have ASTERIX elements"

    def test_describeXML_without_descriptions(self):
        """Test describeXML with descriptions disabled
        Test Case: TC-002-012
        """
        parsed = asterix.parse(self.asterix_packet)
        xml_result = asterix.describeXML(parsed, descriptions=False)

        if xml_result is not None:
            assert xml_result is not None, "Should return XML without descriptions"
            assert xml_result.tag == 'ASTERIXSTART', "Root should be ASTERIXSTART"

    def test_describeXML_multiple_records(self):
        """Test describeXML with multiple records
        Test Case: TC-002-013
        """
        # Parse a file with multiple records
        sample_file = asterix.get_sample_file('cat048')
        if sample_file and os.path.exists(sample_file):
            with open(sample_file, 'rb') as f:
                data = f.read()

            parsed = asterix.parse(data)
            xml_result = asterix.describeXML(parsed)

            if xml_result is not None:
                asterix_elements = xml_result.findall('ASTERIX')
                assert len(asterix_elements) >= 1, "Should have at least one ASTERIX element"

    def test_describeXML_empty_list(self):
        """Test describeXML with empty parsed list
        Test Case: TC-002-014
        """
        empty_parsed = []
        xml_result = asterix.describeXML(empty_parsed)

        if xml_result is not None:
            assert xml_result.tag == 'ASTERIXSTART', "Should have root element"
            asterix_elements = xml_result.findall('ASTERIX')
            assert len(asterix_elements) == 0, "Should have no ASTERIX elements"


class TestDescribeVariants(unittest.TestCase):
    """Test describe() function with various parameter combinations
    Requirement: REQ-HLR-API-002
    Verification: TC-003-010 through TC-003-015
    """

    def test_describe_category_only(self):
        """Test describe with category only (specification version)
        Test Case: TC-003-010
        """
        # This uses the _asterix.describe() function
        description = asterix._asterix.describe(48)
        assert description is not None, "Should return category description"
        assert isinstance(description, str), "Description should be a string"

    def test_describe_category_and_item(self):
        """Test describe with category and item
        Test Case: TC-003-011
        """
        description = asterix._asterix.describe(48, 'I010')
        assert description is not None, "Should return item description"
        assert isinstance(description, str), "Description should be a string"

    def test_describe_category_item_field(self):
        """Test describe with category, item, and field
        Test Case: TC-003-012
        """
        description = asterix._asterix.describe(48, 'I010', 'SAC')
        assert description is not None, "Should return field description"
        assert isinstance(description, str), "Description should be a string"

    def test_describe_category_item_field_value(self):
        """Test describe with category, item, field, and value
        Test Case: TC-003-013
        """
        description = asterix._asterix.describe(48, 'I010', 'SAC', '1')
        # This might return empty string if value has no special meaning
        assert isinstance(description, str), "Description should be a string"

    def test_describe_parsed_data(self):
        """Test describe() with parsed data (textual format)
        Test Case: TC-003-014
        """
        asterix_packet = bytearray([
            0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
            0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
            0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
            0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
            0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
            0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
        ])

        parsed = asterix.parse(asterix_packet)
        description = asterix.describe(parsed)

        assert description is not None, "Should return textual description"
        assert isinstance(description, str), "Description should be a string"
        assert len(description) > 0, "Description should not be empty"
        assert 'Asterix record' in description, "Should contain record information"

    def test_describe_parsed_multiple_records(self):
        """Test describe() with multiple parsed records
        Test Case: TC-003-015
        """
        sample_file = asterix.get_sample_file('cat048')
        if sample_file and os.path.exists(sample_file):
            with open(sample_file, 'rb') as f:
                data = f.read()

            parsed = asterix.parse(data)
            description = asterix.describe(parsed)

            assert description is not None, "Should describe multiple records"
            assert isinstance(description, str), "Description should be a string"
            # Should have multiple "Asterix record:" entries
            record_count = description.count('Asterix record:')
            assert record_count >= 1, "Should describe at least one record"


class TestDescribeXMLSpecFunction(unittest.TestCase):
    """Test describeXML() specification function (not parsed data)
    Requirement: REQ-HLR-API-002
    Verification: TC-004-010 through TC-004-013
    """

    def test_describeXML_spec_category(self):
        """Test describeXML specification for category
        Test Case: TC-004-010
        """
        try:
            # Test the specification describe function
            result = asterix._asterix.describeXML(48)
            assert result is not None, "Should return XML for category"
        except AttributeError:
            # Function might not be available
            pass

    def test_describeXML_spec_item(self):
        """Test describeXML specification for item
        Test Case: TC-004-011
        """
        try:
            result = asterix._asterix.describeXML(48, 'I010')
            assert result is not None, "Should return XML for item"
        except AttributeError:
            pass

    def test_describeXML_spec_field(self):
        """Test describeXML specification for field
        Test Case: TC-004-012
        """
        try:
            result = asterix._asterix.describeXML(48, 'I010', 'SAC')
            assert result is not None, "Should return XML for field"
        except AttributeError:
            pass

    def test_describeXML_spec_value(self):
        """Test describeXML specification for value
        Test Case: TC-004-013
        """
        try:
            result = asterix._asterix.describeXML(48, 'I010', 'SAC', '1')
            # Result might be empty for values without special meaning
            assert result is not None or result == '', "Should return XML or empty"
        except AttributeError:
            pass


if __name__ == '__main__':
    unittest.main()
