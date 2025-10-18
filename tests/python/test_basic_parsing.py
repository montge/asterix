"""
Basic parsing tests for ASTERIX decoder
DO-278A AL-3 Compliance Test Suite

Test Coverage Goals:
- Overall: 90-95%
- This module: ≥90%

Requirements Traceability:
- REQ-HLR-001: Parse ASTERIX data blocks
- REQ-HLR-048: Parse Category 048 data
"""

import pytest
import asterix
import os


class TestBasicParsing:
    """Test basic ASTERIX parsing functionality"""

    def test_parse_simple_packet(self):
        """
        Test parsing a simple ASTERIX packet

        Requirement: REQ-HLR-001
        Verification: TC-001-001
        """
        # Sample CAT048 packet (simplified)
        asterix_packet = bytearray([
            0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
            0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
            0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
            0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
            0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
            0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
        ])

        result = asterix.parse(asterix_packet)

        # Verify basic parsing
        assert result is not None, "Parse result should not be None"
        assert len(result) > 0, "Should parse at least one record"
        assert 'category' in result[0], "Record should have category field"

    def test_parse_returns_list(self):
        """
        Test that parse returns a list

        Requirement: REQ-HLR-001
        Verification: TC-001-002
        """
        data = bytearray([0x30, 0x00, 0x03])  # Minimal packet
        result = asterix.parse(data)
        assert isinstance(result, list), "Parse should return a list"

    def test_parse_empty_data(self):
        """
        Test parsing empty data

        Requirement: REQ-LLR-ERR-001 (Error handling)
        Verification: TC-ERR-001
        """
        empty_data = bytearray([])
        result = asterix.parse(empty_data)
        # Should handle gracefully (return empty list or raise appropriate error)
        assert isinstance(result, list), "Should return list even for empty data"

    def test_parse_verbose_mode(self):
        """
        Test parse with verbose mode enabled/disabled

        Requirement: REQ-HLR-002 (Output formatting)
        Verification: TC-002-001
        """
        asterix_packet = bytearray([
            0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
            0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
            0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
            0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
            0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
            0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
        ])

        result_verbose = asterix.parse(asterix_packet, verbose=True)
        result_quiet = asterix.parse(asterix_packet, verbose=False)

        assert len(result_verbose) == len(result_quiet), \
            "Verbose mode should not change number of records"


class TestCategoryparsing:
    """Test ASTERIX category-specific parsing"""

    def test_category_048_identification(self):
        """
        Test that CAT048 packets are correctly identified

        Requirement: REQ-HLR-048
        Verification: TC-048-001
        """
        # CAT048 packet starts with 0x30 (category 48)
        cat048_packet = bytearray([
            0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
            0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
            0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
            0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
            0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
            0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
        ])

        result = asterix.parse(cat048_packet)
        assert result[0]['category'] == 48, "Should identify as CAT048"

    @pytest.mark.skipif(
        not os.path.exists('install/sample_data/cat_034_048.pcap'),
        reason="Sample data file not found"
    )
    def test_parse_real_cat048_file(self):
        """
        Test parsing real CAT048 PCAP file

        Requirement: REQ-HLR-048
        Verification: TC-048-002
        """
        # This test requires the sample data file
        # It will be skipped if file doesn't exist
        pass  # TODO: Implement when sample data parsing is set up


class TestConfigurationFiles:
    """Test configuration file handling"""

    def test_list_configuration_files(self):
        """
        Test listing available configuration files

        Requirement: REQ-HLR-003 (Configuration management)
        Verification: TC-003-001
        """
        config_files = asterix.list_configuration_files()
        assert isinstance(config_files, list), "Should return list"
        assert len(config_files) > 0, "Should have configuration files"

    def test_get_configuration_file(self):
        """
        Test retrieving specific configuration file

        Requirement: REQ-HLR-003
        Verification: TC-003-002
        """
        config_file = asterix.get_configuration_file('cat048')
        assert config_file is not None, "Should find CAT048 config"
        assert 'cat048' in config_file.lower(), "Config file should contain 'cat048'"
        assert os.path.exists(config_file), "Config file should exist"

    def test_get_dtd_file(self):
        """
        Test retrieving DTD configuration file

        Requirement: REQ-HLR-003
        Verification: TC-003-003
        """
        dtd_file = asterix.get_configuration_file('dtd')
        assert dtd_file is not None, "Should find DTD file"
        assert os.path.exists(dtd_file), "DTD file should exist"


class TestSampleFiles:
    """Test sample file handling"""

    def test_list_sample_files(self):
        """
        Test listing sample files

        Requirement: REQ-HLR-004 (Test data)
        Verification: TC-004-001
        """
        sample_files = asterix.list_sample_files()
        assert isinstance(sample_files, list), "Should return list"
        # May be empty, that's OK

    def test_get_sample_file(self):
        """
        Test retrieving specific sample file

        Requirement: REQ-HLR-004
        Verification: TC-004-002
        """
        sample_file = asterix.get_sample_file('cat048')
        # May be None if no sample exists, that's OK
        if sample_file:
            assert os.path.exists(sample_file), "Sample file should exist if returned"


class TestDescribeFunction:
    """Test description/documentation functions"""

    def test_describe_category(self):
        """
        Test describing a category

        Requirement: REQ-HLR-005 (Documentation)
        Verification: TC-005-001
        """
        # Note: Use _asterix.describe for specification descriptions
        # The describe() function is overloaded and conflicts
        description = asterix._asterix.describe(48)
        assert description is not None, "Should return description"
        assert isinstance(description, str), "Description should be string"

    def test_describe_item(self):
        """
        Test describing a data item

        Requirement: REQ-HLR-005
        Verification: TC-005-002
        """
        description = asterix._asterix.describe(48, 'I048/010')
        assert description is not None, "Should return description"
        assert isinstance(description, str), "Description should be string"

    def test_describe_parsed_data(self):
        """
        Test describing parsed data (the other describe function)

        Requirement: REQ-HLR-005
        Verification: TC-005-003
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
        description = asterix.describe(parsed)  # This uses the overloaded function
        assert description is not None, "Should return description"
        assert isinstance(description, str), "Description should be string"
        assert 'Asterix record' in description, "Should contain record information"


# Coverage improvement tests - add more as needed to reach 90%+

class TestErrorHandling:
    """Test error handling and edge cases"""

    def test_parse_invalid_category(self):
        """
        Test parsing data with invalid/unknown category

        Requirement: REQ-LLR-ERR-002
        Verification: TC-ERR-002
        """
        # Category 255 may not be defined
        invalid_packet = bytearray([0xFF, 0x00, 0x03])
        result = asterix.parse(invalid_packet)
        # Should handle gracefully
        assert isinstance(result, list), "Should return list even for invalid category"

    def test_parse_truncated_data(self):
        """
        Test parsing truncated packet

        Requirement: REQ-LLR-ERR-003
        Verification: TC-ERR-003
        """
        # Packet claims length but is truncated
        truncated = bytearray([0x30, 0x00, 0xFF])  # Says 255 bytes but only 3
        result = asterix.parse(truncated)
        # Should handle gracefully
        assert isinstance(result, list), "Should handle truncated data"


# Pytest configuration
def pytest_configure(config):
    """Configure pytest with custom markers"""
    config.addinivalue_line(
        "markers", "slow: marks tests as slow (deselect with '-m \"not slow\"')"
    )
    config.addinivalue_line(
        "markers", "integration: marks tests as integration tests"
    )


if __name__ == '__main__':
    # Run tests with coverage
    pytest.main([
        __file__,
        '-v',
        '--cov=asterix',
        '--cov-report=term',
        '--cov-report=html'
    ])
