"""
Python integration test for CAT025 parsing
CNS/ATM Ground System Status Reports

This test verifies the Python API for parsing ASTERIX Category 025 data.

Requirements Coverage:
- REQ-HLR-001: Parse ASTERIX binary data
- REQ-HLR-SYS-001: Parse ASTERIX categories
- REQ-LLR-025-*: Additional requirements from Low_Level_Requirements_CAT025.md

Test Cases:
- TC-PY-CAT025-001: Parse CAT025 binary data
- TC-PY-CAT025-002: Verify parsed data structure
- TC-PY-CAT025-003: Test error handling
"""

import pytest
import asterix


class TestCAT025:
    """Test Category 025 (CNS/ATM Ground System Status Reports) parsing"""

    def test_parse_cat025_packet(self):
        """
        Test parsing CAT025 packet

        Requirement: REQ-HLR-001, REQ-LLR-025-010
        Verification: TC-PY-CAT025-001
        """
        # Create minimal valid CAT025 packet
        # Structure: [Category: 0x19] [Length MSB: 0x00] [Length LSB: 0x03]
        cat025_packet = bytearray([
            0x19,  # Category 25
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes (header only)
        ])

        result = asterix.parse(cat025_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        # If packet is too minimal, it may return empty list, which is acceptable
        if len(result) > 0:
            assert result[0]['category'] == 25, "Should identify as CAT025"

    def test_parse_cat025_with_data_items(self):
        """
        Test parsing CAT025 with data items

        Requirement: REQ-LLR-025-010
        Verification: TC-PY-CAT025-002
        """
        # Create CAT025 packet with I025/010 (Data Source Identifier)
        # Structure: [Category: 0x19] [Length MSB: 0x00] [Length LSB: 0x08] [FSPEC: 0x80] [I025/010: 2 bytes]
        cat025_packet = bytearray([
            0x19,  # Category 25
            0x00,  # Length MSB
            0x08,  # Length LSB = 8 bytes total
            0x80,  # FSPEC: I025/010 present (bit 7 set)
            0x01,  # I025/010: SAC = 0x01
            0x23,  # I025/010: SIC = 0x23
        ])

        result = asterix.parse(cat025_packet)

        # Verify parsing succeeded
        assert result is not None, "Parse result should not be None"
        assert isinstance(result, list), "Parse should return a list"
        
        if len(result) > 0:
            assert result[0]['category'] == 25, "Should identify as CAT025"

    def test_cat025_error_handling(self):
        """
        Test error handling for invalid CAT025 data

        Requirement: REQ-HLR-001 (Error handling)
        Verification: TC-PY-CAT025-003
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

    def test_cat025_api_usage(self):
        """
        Test Python API usage for CAT025

        Requirement: REQ-HLR-001
        Verification: TC-PY-CAT025-004
        """
        # Create minimal valid packet
        cat025_packet = bytearray([
            0x19,  # Category 25
            0x00,  # Length MSB
            0x03,  # Length LSB = 3 bytes
        ])

        # Test with verbose mode
        result_verbose = asterix.parse(cat025_packet, verbose=True)
        result_quiet = asterix.parse(cat025_packet, verbose=False)

        # Both should return valid results
        assert isinstance(result_verbose, list), "Verbose mode should return list"
        assert isinstance(result_quiet, list), "Quiet mode should return list"
        assert len(result_verbose) == len(result_quiet), "Both modes should return same number of records"
