"""Tests for strict parsing mode.

This module tests the strict parameter functionality which controls
whether the parser raises exceptions on errors or silently ignores them.
"""

import os
import unittest
import asterix


class TestStrictMode(unittest.TestCase):
    """Test cases for strict parsing mode."""

    def setUp(self):
        """Set up test fixtures."""
        # Valid CAT048 data
        self.valid_data = bytes([
            0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9,
            0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
            0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10,
            0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
            0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb,
            0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
        ])
        # Corrupted data (invalid category + garbage)
        self.corrupted_data = bytes([0xFF, 0x00, 0x10, 0xFF, 0xFF, 0xFF, 0xFF])

    def test_parse_valid_data_strict_false(self):
        """Test parsing valid data with strict=False."""
        records = asterix.parse(self.valid_data, strict=False)
        self.assertIsInstance(records, list)
        self.assertGreater(len(records), 0)
        self.assertEqual(records[0]['category'], 48)

    def test_parse_valid_data_strict_true(self):
        """Test parsing valid data with strict=True."""
        records = asterix.parse(self.valid_data, strict=True)
        self.assertIsInstance(records, list)
        self.assertGreater(len(records), 0)
        self.assertEqual(records[0]['category'], 48)

    def test_parse_valid_data_default_strict(self):
        """Test parsing valid data with default strict value."""
        records = asterix.parse(self.valid_data)
        self.assertIsInstance(records, list)
        self.assertGreater(len(records), 0)

    def test_strict_mode_with_environment_variable(self):
        """Test that ASTERIX_STRICT environment variable is respected."""
        # Save original value
        original = os.environ.get('ASTERIX_STRICT')

        try:
            # Test with env var set to '1'
            os.environ['ASTERIX_STRICT'] = '1'
            # Force reload of the default
            self.assertTrue(asterix._get_strict_default())

            # Test with env var set to 'true'
            os.environ['ASTERIX_STRICT'] = 'true'
            self.assertTrue(asterix._get_strict_default())

            # Test with env var set to '0'
            os.environ['ASTERIX_STRICT'] = '0'
            self.assertFalse(asterix._get_strict_default())

            # Test with env var set to 'false'
            os.environ['ASTERIX_STRICT'] = 'false'
            self.assertFalse(asterix._get_strict_default())

        finally:
            # Restore original value
            if original is not None:
                os.environ['ASTERIX_STRICT'] = original
            elif 'ASTERIX_STRICT' in os.environ:
                del os.environ['ASTERIX_STRICT']

    def test_strict_parameter_overrides_env(self):
        """Test that explicit strict parameter overrides environment variable."""
        original = os.environ.get('ASTERIX_STRICT')

        try:
            # Set env to strict
            os.environ['ASTERIX_STRICT'] = '1'

            # Explicit strict=False should override env
            records = asterix.parse(self.valid_data, strict=False)
            self.assertIsInstance(records, list)

            # Explicit strict=True should also work
            records = asterix.parse(self.valid_data, strict=True)
            self.assertIsInstance(records, list)

        finally:
            if original is not None:
                os.environ['ASTERIX_STRICT'] = original
            elif 'ASTERIX_STRICT' in os.environ:
                del os.environ['ASTERIX_STRICT']

    def test_parse_with_offset_strict_false(self):
        """Test parse_with_offset with strict=False."""
        records, offset = asterix.parse_with_offset(
            self.valid_data, offset=0, blocks_count=10, strict=False
        )
        self.assertIsInstance(records, list)
        self.assertIsInstance(offset, int)

    def test_parse_with_offset_strict_true(self):
        """Test parse_with_offset with strict=True on valid data."""
        records, offset = asterix.parse_with_offset(
            self.valid_data, offset=0, blocks_count=10, strict=True
        )
        self.assertIsInstance(records, list)
        self.assertIsInstance(offset, int)

    def test_verbose_and_strict_combination(self):
        """Test combining verbose and strict parameters."""
        # verbose=True, strict=True
        records = asterix.parse(self.valid_data, verbose=True, strict=True)
        self.assertIsInstance(records, list)
        self.assertGreater(len(records), 0)

        # verbose=False, strict=True
        records = asterix.parse(self.valid_data, verbose=False, strict=True)
        self.assertIsInstance(records, list)
        self.assertGreater(len(records), 0)

        # verbose=True, strict=False
        records = asterix.parse(self.valid_data, verbose=True, strict=False)
        self.assertIsInstance(records, list)
        self.assertGreater(len(records), 0)

        # verbose=False, strict=False
        records = asterix.parse(self.valid_data, verbose=False, strict=False)
        self.assertIsInstance(records, list)
        self.assertGreater(len(records), 0)


class TestStrictModeEnvVariableValues(unittest.TestCase):
    """Test various environment variable values for strict mode."""

    def test_env_values_true(self):
        """Test values that should enable strict mode."""
        original = os.environ.get('ASTERIX_STRICT')
        try:
            for value in ['1', 'true', 'TRUE', 'True', 'yes', 'YES', 'on', 'ON']:
                os.environ['ASTERIX_STRICT'] = value
                self.assertTrue(
                    asterix._get_strict_default(),
                    f"Expected strict=True for ASTERIX_STRICT={value}"
                )
        finally:
            if original is not None:
                os.environ['ASTERIX_STRICT'] = original
            elif 'ASTERIX_STRICT' in os.environ:
                del os.environ['ASTERIX_STRICT']

    def test_env_values_false(self):
        """Test values that should disable strict mode."""
        original = os.environ.get('ASTERIX_STRICT')
        try:
            for value in ['0', 'false', 'FALSE', 'no', 'off', '', 'invalid']:
                os.environ['ASTERIX_STRICT'] = value
                self.assertFalse(
                    asterix._get_strict_default(),
                    f"Expected strict=False for ASTERIX_STRICT={value}"
                )
        finally:
            if original is not None:
                os.environ['ASTERIX_STRICT'] = original
            elif 'ASTERIX_STRICT' in os.environ:
                del os.environ['ASTERIX_STRICT']

    def test_env_not_set(self):
        """Test default when environment variable is not set."""
        original = os.environ.get('ASTERIX_STRICT')
        try:
            if 'ASTERIX_STRICT' in os.environ:
                del os.environ['ASTERIX_STRICT']
            self.assertFalse(asterix._get_strict_default())
        finally:
            if original is not None:
                os.environ['ASTERIX_STRICT'] = original


if __name__ == '__main__':
    unittest.main()
