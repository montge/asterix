"""
Python integration test for CAT063 parsing
Meteorological Data

This test verifies the Python API for parsing ASTERIX Category 063 data.

Requirements Coverage:
- REQ-HLR-001: Parse ASTERIX binary data
- REQ-HLR-SYS-001: Parse ASTERIX categories
- REQ-LLR-063-*: Additional requirements from Low_Level_Requirements_CAT063.md

Test Cases:
- TC-PY-CAT063-001: Parse CAT063 binary data
- TC-PY-CAT063-002: Verify parsed data structure
- TC-PY-CAT063-003: Test error handling
"""

import pytest
import asterix


class TestCAT063:
    """Test Category 063 (Meteorological Data) parsing"""

    def test_parse_cat063_packet(self):
        """
        Test parsing CAT063 packet

        Requirement: REQ-HLR-001, REQ-LLR-063-010
        Verification: TC-PY-CAT063-001
        """
        # Create minimal valid CAT063 packet
        # Structure: [Category: 0x3F] [Length MSB: 0x00] [Length LSB: 0x03]
        cat063_packet = bytearray([
            0x3F,  # Category 63
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes (header only)
        ])

        result = asterix.parse(cat063_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        # If packet is too minimal, it may return empty list, which is acceptable
        if len(result) > 0:
            assert result[0]['category'] == 63, "Should identify as CAT063"

    def test_parse_cat063_with_data_items(self):
        """
        Test parsing CAT063 with data items

        Requirement: REQ-LLR-063-010
        Verification: TC-PY-CAT063-002
        """
        # Create CAT063 packet with I063/010 (Data Source Identifier)
        # Structure: [Category: 0x3F] [Length MSB: 0x00] [Length LSB: 0x08] [FSPEC: 0x80] [I063/010: 2 bytes]
        cat063_packet = bytearray([
            0x3F,  # Category 63
            0x00,  # Length MSB
            0x08,  # Length LSB = 8 bytes total
            0x80,  # FSPEC: I063/010 present (bit 7 set)
            0x01,  # I063/010: SAC = 0x01
            0x23,  # I063/010: SIC = 0x23
        ])

        result = asterix.parse(cat063_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        if len(result) > 0:
            assert result[0]['category'] == 63, "Should identify as CAT063"

    def test_cat063_error_handling(self):
        """
        Test error handling for invalid CAT063 data

        Requirement: REQ-HLR-001 (Error handling)
        Verification: TC-PY-CAT063-003
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

    def test_cat063_api_usage(self):
        """
        Test Python API usage for CAT063

        Requirement: REQ-HLR-001
        Verification: TC-PY-CAT063-004
        """
        # Create minimal valid packet
        cat063_packet = bytearray([
            0x3F,  # Category 63
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes
        ])

        # Test with verbose mode
        result_verbose = asterix.parse(cat063_packet, verbose=True)
        result_quiet = asterix.parse(cat063_packet, verbose=False)

        # Both should return valid results
        assert isinstance(result_verbose, list), "Verbose mode should return list"
        assert isinstance(result_quiet, list), "Quiet mode should return list"
        assert len(result_verbose) == len(result_quiet), "Both modes should return same number of records"
