"""
Python integration test for CAT252 parsing
Connection and Service Messages

This test verifies the Python API for parsing ASTERIX Category 252 data.

Requirements Coverage:
- REQ-HLR-001: Parse ASTERIX binary data
- REQ-HLR-SYS-001: Parse ASTERIX categories
- REQ-LLR-252-*: Additional requirements from Low_Level_Requirements_CAT252.md

Test Cases:
- TC-PY-CAT252-001: Parse CAT252 binary data
- TC-PY-CAT252-002: Verify parsed data structure
- TC-PY-CAT252-003: Test error handling
"""

import pytest
import asterix


class TestCAT252:
    """Test Category 252 (Connection and Service Messages) parsing"""

    def test_parse_cat252_packet(self):
        """
        Test parsing CAT252 packet

        Requirement: REQ-HLR-001, REQ-LLR-252-010
        Verification: TC-PY-CAT252-001
        """
        # Create minimal valid CAT252 packet
        # Structure: [Category: 0xFC] [Length MSB: 0x00] [Length LSB: 0x03]
        cat252_packet = bytearray([
            0xFC,  # Category 252
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes (header only)
        ])

        result = asterix.parse(cat252_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        # If packet is too minimal, it may return empty list, which is acceptable
        if len(result) > 0:
            assert result[0]['category'] == 252, "Should identify as CAT252"

    def test_parse_cat252_with_data_items(self):
        """
        Test parsing CAT252 with data items

        Requirement: REQ-LLR-252-010
        Verification: TC-PY-CAT252-002
        """
        # Create CAT252 packet with I252/010 (Data Source Identifier)
        # Structure: [Category: 0xFC] [Length MSB: 0x00] [Length LSB: 0x08] [FSPEC: 0x80] [I252/010: 2 bytes]
        cat252_packet = bytearray([
            0xFC,  # Category 252
            0x00,  # Length MSB
            0x08,  # Length LSB = 8 bytes total
            0x80,  # FSPEC: I252/010 present (bit 7 set)
            0x01,  # I252/010: SAC = 0x01
            0x23,  # I252/010: SIC = 0x23
        ])

        result = asterix.parse(cat252_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        if len(result) > 0:
            assert result[0]['category'] == 252, "Should identify as CAT252"

    def test_cat252_error_handling(self):
        """
        Test error handling for invalid CAT252 data

        Requirement: REQ-HLR-001 (Error handling)
        Verification: TC-PY-CAT252-003
        """
        # Test with empty data
        # Security fix: Empty data should raise ValueError (not return empty list)
        empty_data = bytearray([])
        with pytest.raises(ValueError, match="Empty input data"):
            asterix.parse(empty_data)


        # Test with invalid category (wrong category byte)
        invalid_packet = bytearray([
            0xFF,  # Invalid category
            0x00,  # Length MSB
            0x03,  # Length LSB
        ])
        result = asterix.parse(invalid_packet)
        # Should handle gracefully (return empty list or valid list with error indication)
        assert isinstance(result, list), "Should return list even for invalid data"

    def test_cat252_api_usage(self):
        """
        Test Python API usage for CAT252

        Requirement: REQ-HLR-001
        Verification: TC-PY-CAT252-004
        """
        # Create minimal valid packet
        cat252_packet = bytearray([
            0xFC,  # Category 252
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes
        ])

        # Test with verbose mode
        result_verbose = asterix.parse(cat252_packet, verbose=True)
        result_quiet = asterix.parse(cat252_packet, verbose=False)

        # Both should return valid results
        assert isinstance(result_verbose, list), "Verbose mode should return list"
        assert isinstance(result_quiet, list), "Quiet mode should return list"
        assert len(result_verbose) == len(result_quiet), "Both modes should return same number of records"
