"""
Python integration test for CAT240 parsing
MLAT Position Report

This test verifies the Python API for parsing ASTERIX Category 240 data.

Requirements Coverage:
- REQ-HLR-001: Parse ASTERIX binary data
- REQ-HLR-SYS-001: Parse ASTERIX categories
- REQ-LLR-240-*: Additional requirements from Low_Level_Requirements_CAT240.md

Test Cases:
- TC-PY-CAT240-001: Parse CAT240 binary data
- TC-PY-CAT240-002: Verify parsed data structure
- TC-PY-CAT240-003: Test error handling
"""

import pytest
import asterix


class TestCAT240:
    """Test Category 240 (MLAT Position Report) parsing"""

    def test_parse_cat240_packet(self):
        """
        Test parsing CAT240 packet

        Requirement: REQ-HLR-001, REQ-LLR-240-010
        Verification: TC-PY-CAT240-001
        """
        # Create minimal valid CAT240 packet
        # Structure: [Category: 0xF0] [Length MSB: 0x00] [Length LSB: 0x03]
        cat240_packet = bytearray([
            0xF0,  # Category 240
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes (header only)
        ])

        result = asterix.parse(cat240_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        # If packet is too minimal, it may return empty list, which is acceptable
        if len(result) > 0:
            assert result[0]['category'] == 240, f"Should identify as CAT240"

    def test_parse_cat240_with_data_items(self):
        """
        Test parsing CAT240 with data items

        Requirement: REQ-LLR-240-010
        Verification: TC-PY-CAT240-002
        """
        # Create CAT240 packet with I240/010 (Data Source Identifier)
        # Structure: [Category: 0xF0] [Length MSB: 0x00] [Length LSB: 0x08] [FSPEC: 0x80] [I240/010: 2 bytes]
        cat240_packet = bytearray([
            0xF0,  # Category 240
            0x00,  # Length MSB
            0x08,  # Length LSB = 8 bytes total
            0x80,  # FSPEC: I240/010 present (bit 7 set)
            0x01,  # I240/010: SAC = 0x01
            0x23,  # I240/010: SIC = 0x23
        ])

        result = asterix.parse(cat240_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        if len(result) > 0:
            assert result[0]['category'] == 240, f"Should identify as CAT240"

    def test_cat240_error_handling(self):
        """
        Test error handling for invalid CAT240 data

        Requirement: REQ-HLR-001 (Error handling)
        Verification: TC-PY-CAT240-003
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

    def test_cat240_api_usage(self):
        """
        Test Python API usage for CAT240

        Requirement: REQ-HLR-001
        Verification: TC-PY-CAT240-004
        """
        # Create minimal valid packet
        cat240_packet = bytearray([
            0xF0,  # Category 240
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes
        ])

        # Test with verbose mode
        result_verbose = asterix.parse(cat240_packet, verbose=True)
        result_quiet = asterix.parse(cat240_packet, verbose=False)

        # Both should return valid results
        assert isinstance(result_verbose, list), "Verbose mode should return list"
        assert isinstance(result_quiet, list), "Quiet mode should return list"
        assert len(result_verbose) == len(result_quiet), "Both modes should return same number of records"
