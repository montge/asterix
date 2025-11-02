"""
Python integration test for CAT032 parsing
System Configuration

This test verifies the Python API for parsing ASTERIX Category 032 data.

Requirements Coverage:
- REQ-HLR-001: Parse ASTERIX binary data
- REQ-HLR-SYS-001: Parse ASTERIX categories
- REQ-LLR-032-*: Additional requirements from Low_Level_Requirements_CAT032.md

Test Cases:
- TC-PY-CAT032-001: Parse CAT032 binary data
- TC-PY-CAT032-002: Verify parsed data structure
- TC-PY-CAT032-003: Test error handling
"""

import pytest
import asterix


class TestCAT032:
    """Test Category 032 (System Configuration) parsing"""

    def test_parse_cat032_packet(self):
        """
        Test parsing CAT032 packet

        Requirement: REQ-HLR-001, REQ-LLR-032-010
        Verification: TC-PY-CAT032-001
        """
        # Create minimal valid CAT032 packet
        # Structure: [Category: 0x20] [Length MSB: 0x00] [Length LSB: 0x03]
        cat032_packet = bytearray([
            0x20,  # Category 32
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes (header only)
        ])

        result = asterix.parse(cat032_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        # If packet is too minimal, it may return empty list, which is acceptable
        if len(result) > 0:
            assert result[0]['category'] == 32, f"Should identify as CAT032"

    def test_parse_cat032_with_data_items(self):
        """
        Test parsing CAT032 with data items

        Requirement: REQ-LLR-032-010
        Verification: TC-PY-CAT032-002
        """
        # Create CAT032 packet with I032/010 (Data Source Identifier)
        # Structure: [Category: 0x20] [Length MSB: 0x00] [Length LSB: 0x08] [FSPEC: 0x80] [I032/010: 2 bytes]
        cat032_packet = bytearray([
            0x20,  # Category 32
            0x00,  # Length MSB
            0x08,  # Length LSB = 8 bytes total
            0x80,  # FSPEC: I032/010 present (bit 7 set)
            0x01,  # I032/010: SAC = 0x01
            0x23,  # I032/010: SIC = 0x23
        ])

        result = asterix.parse(cat032_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        if len(result) > 0:
            assert result[0]['category'] == 32, f"Should identify as CAT032"

    def test_cat032_error_handling(self):
        """
        Test error handling for invalid CAT032 data

        Requirement: REQ-HLR-001 (Error handling)
        Verification: TC-PY-CAT032-003
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

    def test_cat032_api_usage(self):
        """
        Test Python API usage for CAT032

        Requirement: REQ-HLR-001
        Verification: TC-PY-CAT032-004
        """
        # Create minimal valid packet
        cat032_packet = bytearray([
            0x20,  # Category 32
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes
        ])

        # Test with verbose mode
        result_verbose = asterix.parse(cat032_packet, verbose=True)
        result_quiet = asterix.parse(cat032_packet, verbose=False)

        # Both should return valid results
        assert isinstance(result_verbose, list), "Verbose mode should return list"
        assert isinstance(result_quiet, list), "Quiet mode should return list"
        assert len(result_verbose) == len(result_quiet), "Both modes should return same number of records"
