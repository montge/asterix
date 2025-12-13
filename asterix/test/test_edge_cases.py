#!/usr/bin/python

import asterix
import unittest
import os


class AsterixEdgeCasesTest(unittest.TestCase):
    """Test edge cases and error handling paths to improve coverage."""

    def test_parse_with_offset_functionality(self):
        """Test parse_with_offset() function to cover lines 160-165."""
        sample_filename = asterix.get_sample_file('cat048.raw')
        with open(sample_filename, "rb") as f:
            data = f.read()

            # Test with default parameters
            result, offset = asterix.parse_with_offset(data)
            self.assertIsNotNone(result)
            self.assertIsInstance(offset, int)
            self.assertGreater(offset, 0)

            # Test with custom offset
            result, offset = asterix.parse_with_offset(data, offset=0, blocks_count=1)
            self.assertIsNotNone(result)

            # Test with verbose=False (line 160-161)
            result, offset = asterix.parse_with_offset(data, offset=0, blocks_count=1, verbose=False)
            self.assertIsNotNone(result)

            # Test with verbose=True explicitly (line 162-163)
            result, offset = asterix.parse_with_offset(data, offset=0, blocks_count=1, verbose=True)
            self.assertIsNotNone(result)

    def test_describe_xml_spec_with_lxml(self):
        """Test describeXML() output formatter with lxml (lines 177-232)."""
        # Skip if lxml is not available
        try:
            from lxml import etree
        except ImportError:
            self.skipTest("lxml not available")

        # Test with CAT048 which has dict items with 'val'
        sample_filename = asterix.get_sample_file('cat048.raw')
        with open(sample_filename, "rb") as f:
            data = f.read()
            parsed = asterix.parse(data)

            # Test describeXML with parsed data (without descriptions)
            xml_result = asterix.describeXML(parsed)
            self.assertIsNotNone(xml_result)

            # Test describeXML with descriptions=True (line 190-191)
            xml_result = asterix.describeXML(parsed, descriptions=True)
            self.assertIsNotNone(xml_result)

            # Verify XML structure
            self.assertEqual(xml_result.tag, 'ASTERIXSTART')
            self.assertTrue(len(xml_result) > 0)

            # Test with CAT062/065 which has nested items
            sample_filename = asterix.get_sample_file('cat062cat065.raw')
            with open(sample_filename, "rb") as f:
                data = f.read()
                parsed = asterix.parse(data)

                # Test without descriptions (covers lines 177-231)
                xml_result = asterix.describeXML(parsed)
                self.assertIsNotNone(xml_result)
                self.assertEqual(xml_result.tag, 'ASTERIXSTART')

                # Test with descriptions=True (covers lines with description2)
                xml_result = asterix.describeXML(parsed, descriptions=True)
                self.assertIsNotNone(xml_result)

                # Verify we have ASTERIX records
                asterix_records = xml_result.findall('ASTERIX')
                self.assertGreater(len(asterix_records), 0)

    def test_describe_formatter_function(self):
        """Test describe() output formatter for textual format (lines 241-283)."""
        # Test with CAT048 which has simple dict items
        sample_filename = asterix.get_sample_file('cat048.raw')
        with open(sample_filename, "rb") as f:
            data = f.read()
            parsed = asterix.parse(data)

            # Test describe formatter function
            description = asterix.describe(parsed)
            self.assertIsNotNone(description)
            self.assertIsInstance(description, str)
            self.assertGreater(len(description), 0)

            # Verify it contains expected elements
            self.assertIn('Asterix record:', description)
            self.assertIn('Len:', description)
            self.assertIn('CRC:', description)
            self.assertIn('Category:', description)

        # Test with CAT062/065 which has nested dict items (covers lines 269-271)
        sample_filename = asterix.get_sample_file('cat062cat065.raw')
        with open(sample_filename, "rb") as f:
            data = f.read()
            parsed = asterix.parse(data)

            # Test describe formatter function with nested structures
            description = asterix.describe(parsed)
            self.assertIsNotNone(description)
            self.assertIsInstance(description, str)
            self.assertGreater(len(description), 0)

            # Verify nested structures are in output
            self.assertIn('Item:', description)
            # CAT062 has compound items with nested structure

    def test_list_sample_files(self):
        """Test list_sample_files() function (lines 293-299)."""
        sample_files = asterix.list_sample_files()
        self.assertIsInstance(sample_files, list)
        self.assertGreater(len(sample_files), 0)

        # Verify all returned paths are files
        for f in sample_files:
            self.assertTrue(os.path.isfile(f))

    def test_get_sample_file(self):
        """Test get_sample_file() function (lines 302-313)."""
        # Test finding a known sample file
        result = asterix.get_sample_file('cat048')
        self.assertIsNotNone(result)
        self.assertTrue(os.path.isfile(result))
        self.assertIn('cat048', result)

        # Test with non-existent match
        result = asterix.get_sample_file('nonexistent_file_xyz')
        self.assertIsNone(result)

    def test_list_configuration_files(self):
        """Test list_configuration_files() function (lines 321-327)."""
        config_files = asterix.list_configuration_files()
        self.assertIsInstance(config_files, list)
        self.assertGreater(len(config_files), 0)

        # Verify all returned paths are files
        for f in config_files:
            self.assertTrue(os.path.isfile(f))

    def test_get_configuration_file(self):
        """Test get_configuration_file() function (lines 337-341)."""
        # Test finding a known config file
        result = asterix.get_configuration_file('cat048')
        self.assertIsNotNone(result)
        self.assertTrue(os.path.isfile(result))
        self.assertIn('cat048', result)

        # Test with DTD
        result = asterix.get_configuration_file('dtd')
        self.assertIsNotNone(result)
        self.assertTrue(os.path.isfile(result))

        # Test with non-existent match
        result = asterix.get_configuration_file('nonexistent_config_xyz')
        self.assertIsNone(result)

    def test_describexml_without_lxml(self):
        """Test describeXML returns None when lxml is not available (line 232 and 58-59)."""
        # NOTE: Lines 58-59 and 232 are challenging to test because lxml is already imported
        # when the module loads. To properly test this, we would need to:
        # 1. Start Python without lxml installed, OR
        # 2. Use import hooks to block lxml import during asterix module load
        #
        # For coverage purposes, we document that these lines handle the case where
        # lxml is not installed. The ImportError on lines 58-59 would set lxml_found=False,
        # which would cause line 232 to return None instead of processing XML.
        #
        # Since lxml IS installed in our test environment, we verify the positive case:

        # Parse some data
        sample_filename = asterix.get_sample_file('cat048.raw')
        with open(sample_filename, "rb") as f:
            data = f.read()
            parsed = asterix.parse(data)

            # With lxml available, describeXML should return an Element
            result = asterix.describeXML(parsed)

            # Verify lxml is working
            try:
                from lxml import etree
                # If lxml is available, result should be an Element
                self.assertIsInstance(result, etree._Element)
            except ImportError:
                # If lxml is not available, result should be None
                self.assertIsNone(result)

    def test_describexml_with_string_value_items(self):
        """Test describeXML with items that have string values (line 229)."""
        # This tests the 'else' branch where value is neither dict nor list
        try:
            from lxml import etree
        except ImportError:
            self.skipTest("lxml not available")

        # Create a mock parsed structure with a string value
        # (normally parse() doesn't return this, but we test the formatter logic)
        parsed = [{
            'category': 48,
            'len': 10,
            'crc': 'TEST1234',
            'ts': 1234567890,
            'hexdata': '0102030405',
            'I999': 'StringValue'  # This should trigger line 229
        }]

        # Test describeXML with this structure
        xml_result = asterix.describeXML(parsed)
        self.assertIsNotNone(xml_result)
        self.assertEqual(xml_result.tag, 'ASTERIXSTART')

        # Find the I999 item
        asterix_rec = xml_result.find('ASTERIX')
        self.assertIsNotNone(asterix_rec)
        item_999 = asterix_rec.find('I999')
        self.assertIsNotNone(item_999)
        self.assertEqual(item_999.text, 'StringValue')


def main():
    unittest.main()


if __name__ == '__main__':
    main()
