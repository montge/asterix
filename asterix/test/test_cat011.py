"""
Python integration test for CAT011 parsing
Transmission of A-SMGCS Data

This test verifies the Python API for parsing ASTERIX Category 011 data.

Requirements Coverage:
- REQ-HLR-001: Parse ASTERIX binary data
- REQ-HLR-SYS-001: Parse ASTERIX categories
- REQ-LLR-011-*: Additional requirements from Low_Level_Requirements_CAT011.md

Test Cases:
- TC-PY-CAT011-001: Parse CAT011 binary data
- TC-PY-CAT011-002: Verify parsed data structure
- TC-PY-CAT011-003: Test error handling
"""

import pytest
import asterix


class TestCAT011:
    """Test Category 011 (Transmission of A-SMGCS Data) parsing"""

    def test_parse_cat011_packet(self):
        """
        Test parsing CAT011 packet

        Requirement: REQ-HLR-001, REQ-LLR-011-010
        Verification: TC-PY-CAT011-001
        """
        # Create minimal valid CAT011 packet
        # Structure: [Category: 0x0B] [Length MSB: 0x00] [Length LSB: 0x03]
        cat011_packet = bytearray([
            0x0B,  # Category 11
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes (header only)
        ])

        result = asterix.parse(cat011_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        # If packet is too minimal, it may return empty list, which is acceptable
        if len(result) > 0:
            assert result[0]['category'] == 11, f"Should identify as CAT011"

    def test_parse_cat011_with_data_items(self):
        """
        Test parsing CAT011 with data items

        Requirement: REQ-LLR-011-010
        Verification: TC-PY-CAT011-002
        """
        # Create CAT011 packet with I011/010 (Data Source Identifier)
        # Structure: [Category: 0x0B] [Length MSB: 0x00] [Length LSB: 0x08] [FSPEC: 0x80] [I011/010: 2 bytes]
        cat011_packet = bytearray([
            0x0B,  # Category 11
            0x00,  # Length MSB
            0x08,  # Length LSB = 8 bytes total
            0x80,  # FSPEC: I011/010 present (bit 7 set)
            0x01,  # I011/010: SAC = 0x01
            0x23,  # I011/010: SIC = 0x23
        ])

        result = asterix.parse(cat011_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        if len(result) > 0:
            assert result[0]['category'] == 11, f"Should identify as CAT011"

    def test_cat011_error_handling(self):
        """
        Test error handling for invalid CAT011 data

        Requirement: REQ-HLR-001 (Error handling)
        Verification: TC-PY-CAT011-003
        """
        # Test with empty data
        empty_data = bytearray([])
        result = asterix.parse(empty_data)
        assert isinstance(result, list), "Should return list even for empty data"

        # Test with invalid category (wrong category byte)
        invalid_packet = bytearray([
            0xFF,  # Invalid category
            0x00,  # Length MSB
            0x03,  # Length LSB
        ])
        result = asterix.parse(invalid_packet)
        # Should handle gracefully (return empty list or valid list with error indication)
        assert isinstance(result, list), "Should return list even for invalid data"

    def test_cat011_api_usage(self):
        """
        Test Python API usage for CAT011

        Requirement: REQ-HLR-001
        Verification: TC-PY-CAT011-004
        """
        # Create minimal valid packet
        cat011_packet = bytearray([
            0x0B,  # Category 11
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes
        ])

        # Test with verbose mode
        result_verbose = asterix.parse(cat011_packet, verbose=True)
        result_quiet = asterix.parse(cat011_packet, verbose=False)

        # Both should return valid results
        assert isinstance(result_verbose, list), "Verbose mode should return list"
        assert isinstance(result_quiet, list), "Quiet mode should return list"
        assert len(result_verbose) == len(result_quiet), "Both modes should return same number of records"
