#!/usr/bin/env python3
"""
FFI Security Validation Tests

Comprehensive tests for FFI boundary security fixes from Issue #29.
Tests input validation, exception handling, and error propagation across
the Python/C++ FFI boundary.

Test Coverage:
- MEDIUM-003: Category validation
- MEDIUM-004: Filename/path validation
- MEDIUM-005: Exception handling
- CRITICAL: Input validation (empty data, oversized data, null pointers)
"""

import asterix
import unittest
import os
import sys
import tempfile


class AsterixFFISecurityTest(unittest.TestCase):
    """Test FFI boundary security validations"""

    @classmethod
    def setUpClass(cls):
        """Initialize ASTERIX with valid config before tests"""
        sample_init = os.path.join(os.path.dirname(__file__), '../config/asterix_cat062_1_19.xml')
        asterix.init(sample_init)

    # ========================================================================
    # MEDIUM-003: Category Validation Tests
    # ========================================================================

    def test_describe_invalid_category_zero(self):
        """MEDIUM-003: Test that category 0 is rejected"""
        with self.assertRaises(ValueError) as cm:
            asterix._asterix.describe(0)

        self.assertIn("category", str(cm.exception).lower())
        self.assertIn("0", str(cm.exception))

    def test_describe_invalid_category_negative(self):
        """Test that negative categories are rejected"""
        # Python will convert negative to unsigned, but test the behavior
        with self.assertRaises((ValueError, OverflowError)):
            asterix._asterix.describe(-1)

    def test_describe_valid_category_boundary(self):
        """Test valid category boundaries (1 and 255)"""
        # Category 1 - valid
        result = asterix._asterix.describe(1)
        self.assertIsNotNone(result)

        # Category 255 - valid
        result = asterix._asterix.describe(255)
        self.assertIsNotNone(result)

    # ========================================================================
    # MEDIUM-004: Filename Validation Tests
    # ========================================================================

    def test_init_empty_filename(self):
        """MEDIUM-004: Test that empty filename is rejected"""
        with self.assertRaises(ValueError) as cm:
            asterix.init("")

        self.assertIn("empty", str(cm.exception).lower())

    def test_init_path_traversal_leading_dotdot(self):
        """MEDIUM-004: Test that path traversal (../) is blocked"""
        with self.assertRaises(ValueError) as cm:
            asterix.init("../../../etc/passwd")

        self.assertIn("traversal", str(cm.exception).lower())

    def test_init_path_traversal_windows(self):
        """MEDIUM-004: Test that Windows path traversal (..\\) is blocked"""
        with self.assertRaises(ValueError) as cm:
            asterix.init("..\\..\\..\\windows\\system32\\config\\sam")

        self.assertIn("traversal", str(cm.exception).lower())

    def test_init_path_traversal_exact_dotdot(self):
        """MEDIUM-004: Test that exact '..' is blocked"""
        with self.assertRaises(ValueError) as cm:
            asterix.init("..")

        self.assertIn("traversal", str(cm.exception).lower())

    def test_init_filename_too_long(self):
        """MEDIUM-004: Test that filenames longer than 4096 characters are rejected"""
        long_filename = "a" * 4097
        with self.assertRaises(ValueError) as cm:
            asterix.init(long_filename)

        self.assertIn("too long", str(cm.exception).lower())

    def test_init_valid_path_with_dotdot_middle(self):
        """MEDIUM-004: Test that paths with /../ in the middle are allowed"""
        # This should NOT be blocked - only leading ".." is blocked
        # The path may not exist, but it should pass validation
        try:
            with tempfile.TemporaryDirectory() as tmpdir:
                # Create a valid XML file
                valid_xml = os.path.join(tmpdir, "test.xml")
                subdir = os.path.join(tmpdir, "subdir")
                os.makedirs(subdir, exist_ok=True)

                # Write a simple valid XML
                with open(valid_xml, 'w') as f:
                    f.write('<?xml version="1.0"?>\n<Category id="1" ver="1.0"></Category>')

                # Create path with /../ in middle
                path_with_middle_dotdot = os.path.join(subdir, "..", "test.xml")

                # This should be allowed (validation passes, may fail on XML parse)
                result = asterix.init(path_with_middle_dotdot)
                # If it returns without ValueError, validation passed
                self.assertIsNotNone(result)
        except (IOError, SyntaxError):
            # These are OK - means validation passed but file/XML invalid
            pass

    # ========================================================================
    # MEDIUM-005: Exception Handling Tests
    # ========================================================================

    def test_parse_exception_handling_empty_data(self):
        """MEDIUM-005: Test that empty data raises proper Python exception"""
        with self.assertRaises(ValueError) as cm:
            asterix.parse(b"")

        # Should get ValueError from input validation
        self.assertIn("empty", str(cm.exception).lower())

    def test_init_xml_syntax_error_exception(self):
        """MEDIUM-005: Test that XML syntax errors are caught and converted to Python exceptions"""
        malformed_xml = os.path.join(os.path.dirname(__file__), './parsing_error_1.xml')

        with self.assertRaises(SyntaxError):
            asterix.init(malformed_xml)

    # ========================================================================
    # CRITICAL: Input Validation Tests
    # ========================================================================

    def test_parse_oversized_data(self):
        """Test that oversized data (>64KB) is rejected"""
        # Create data larger than MAX_ASTERIX_MESSAGE_SIZE (65536 bytes)
        oversized_data = b'\x30' * 70000

        with self.assertRaises(ValueError) as cm:
            asterix.parse(oversized_data)

        self.assertIn("too large", str(cm.exception).lower())

    def test_parse_with_offset_invalid_offset(self):
        """Test that invalid offset is rejected"""
        valid_data = b'\x30\x00\x03'

        # Offset beyond data length
        with self.assertRaises(ValueError) as cm:
            asterix.parse_with_offset(valid_data, offset=1000, blocks_count=1)

        self.assertIn("offset", str(cm.exception).lower())

    def test_parse_with_offset_excessive_blocks(self):
        """Test that excessive blocks_count is rejected"""
        valid_data = b'\x30\x00\x03'

        # More than MAX_BLOCKS_PER_CALL (10000)
        with self.assertRaises(ValueError) as cm:
            asterix.parse_with_offset(valid_data, offset=0, blocks_count=10001)

        self.assertIn("blocks_count", str(cm.exception).lower())

    def test_parse_with_offset_negative_values(self):
        """Test that negative offset/blocks are handled"""
        valid_data = b'\x30\x00\x03'

        # Python will convert negatives to large unsigned values
        # which should fail validation
        with self.assertRaises((ValueError, OverflowError)):
            asterix.parse_with_offset(valid_data, offset=-1, blocks_count=1)

    def test_describe_with_invalid_strings(self):
        """Test describe() with various invalid string inputs"""
        # Test with valid category but invalid item/field strings
        # Should not crash, may return empty or error
        try:
            result = asterix._asterix.describe(62, None, None, None)
            self.assertIsInstance(result, str)
        except Exception as e:
            # Any exception is OK as long as it doesn't crash
            self.assertIsInstance(e, (ValueError, RuntimeError, TypeError))

    # ========================================================================
    # Boundary Condition Tests
    # ========================================================================

    def test_parse_minimum_valid_asterix_block(self):
        """Test parsing minimum valid ASTERIX block (3 bytes)"""
        # Minimum ASTERIX block: CAT + LEN (2 bytes) = 3 bytes minimum
        min_block = bytes([0x30, 0x00, 0x03])  # CAT 48, length 3

        try:
            result = asterix.parse(min_block)
            # Should either parse or raise appropriate error
            self.assertIsInstance(result, list)
        except (RuntimeError, ValueError):
            # Expected - invalid data but caught properly
            pass

    def test_parse_maximum_category(self):
        """Test parsing with maximum category value (255)"""
        # ASTERIX block with category 255
        max_cat_block = bytes([0xFF, 0x00, 0x03])

        try:
            result = asterix.parse(max_cat_block)
            self.assertIsInstance(result, list)
        except RuntimeError:
            # Expected if category 255 not loaded
            pass

    def test_init_multiple_times(self):
        """Test that multiple init() calls don't cause crashes"""
        sample_init = os.path.join(os.path.dirname(__file__), '../config/asterix_cat062_1_19.xml')

        # First init
        result1 = asterix.init(sample_init)
        self.assertEqual(result1, 0)

        # Second init (should succeed)
        result2 = asterix.init(sample_init)
        self.assertEqual(result2, 0)


def main():
    unittest.main()


if __name__ == '__main__':
    main()
