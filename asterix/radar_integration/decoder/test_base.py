#!/usr/bin/env python3
"""
Comprehensive unit tests for ASTERIX decoder base module.

Tests all functions and classes in base.py including:
- decode_fspec: FSPEC parsing with edge cases
- AsterixDecoder: Abstract base class methods
- Utility functions: decode_uint, decode_int, decode_octal, decode_6bit_ascii
- Error handling and edge cases

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-12-06
"""

import unittest
import sys
import os
import struct

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', '..'))

from asterix.radar_integration.decoder.base import (
    decode_fspec,
    DecoderError,
    AsterixDecoder,
    decode_uint,
    decode_int,
    decode_octal,
    decode_6bit_ascii,
    bytes_consumed_for_fspec,
)


class TestDecodeFspec(unittest.TestCase):
    """Comprehensive tests for decode_fspec function."""

    def test_fspec_single_octet_no_extension(self):
        """Test FSPEC with single octet, no FX bit."""
        # 0xE0 = 0b11100000 (FRNs 1, 2, 3 present, no extension)
        frns, consumed = decode_fspec(b'\xe0\x12\x34')
        self.assertEqual(frns, [1, 2, 3])
        self.assertEqual(consumed, 1)

    def test_fspec_extension_two_octets(self):
        """Test FSPEC with two octets."""
        # 0x81, 0x80 = FRN 1 in first octet, FRN 8 in second octet
        frns, consumed = decode_fspec(b'\x81\x80\xff')
        self.assertIn(1, frns)
        self.assertIn(8, frns)
        self.assertEqual(consumed, 2)

    def test_fspec_extension_three_octets(self):
        """Test FSPEC with three octets."""
        # 0x01, 0x01, 0x80 = Extension in first two, FRN 15 in third
        frns, consumed = decode_fspec(b'\x01\x01\x80')
        self.assertIn(15, frns)
        self.assertEqual(consumed, 3)

    def test_fspec_empty(self):
        """Test FSPEC with no items present."""
        # 0x00 = no items, no extension
        frns, consumed = decode_fspec(b'\x00')
        self.assertEqual(frns, [])
        self.assertEqual(consumed, 1)

    def test_fspec_all_items_first_octet(self):
        """Test FSPEC with all 7 items in first octet."""
        # 0xFE = 0b11111110 (FRNs 1-7, no extension)
        frns, consumed = decode_fspec(b'\xfe')
        self.assertEqual(frns, [1, 2, 3, 4, 5, 6, 7])
        self.assertEqual(consumed, 1)

    def test_fspec_all_items_with_extension(self):
        """Test FSPEC with all items and extension."""
        # 0xFF, 0xFE = All 7 items in first octet + extension, 7 items in second
        frns, consumed = decode_fspec(b'\xff\xfe')
        self.assertEqual(frns, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14])
        self.assertEqual(consumed, 2)

    def test_fspec_offset_valid(self):
        """Test FSPEC parsing with non-zero offset."""
        data = b'\xff\xff\xe0\x12'  # FSPEC starts at offset 2
        frns, consumed = decode_fspec(data, offset=2)
        self.assertEqual(frns, [1, 2, 3])
        self.assertEqual(consumed, 1)

    def test_fspec_offset_at_end(self):
        """Test FSPEC parsing with offset at data end (error case)."""
        data = b'\xe0\x12'
        with self.assertRaises(DecoderError) as cm:
            decode_fspec(data, offset=2)
        self.assertIn("offset", str(cm.exception).lower())

    def test_fspec_offset_beyond_end(self):
        """Test FSPEC parsing with offset beyond data end."""
        data = b'\xe0\x12'
        with self.assertRaises(DecoderError) as cm:
            decode_fspec(data, offset=10)
        self.assertIn("offset", str(cm.exception).lower())

    def test_fspec_truncated_after_extension_bit(self):
        """Test FSPEC with extension bit but no next octet (error case)."""
        # 0x81 = FRN 1 + FX bit, but no following octet
        with self.assertRaises(DecoderError) as cm:
            decode_fspec(b'\x81')
        self.assertIn("truncated", str(cm.exception).lower())

    def test_fspec_too_long(self):
        """Test FSPEC with excessive length (>100 octets protection)."""
        # Create data with 102 octets all with FX bit set
        data = bytes([0x01] * 102)  # All extension bits set
        with self.assertRaises(DecoderError) as cm:
            decode_fspec(data)
        self.assertIn("too long", str(cm.exception).lower())


class ConcreteDecoder(AsterixDecoder):
    """Concrete implementation of AsterixDecoder for testing."""

    def _init_frn_map(self):
        """Initialize FRN map for testing."""
        self.frn_map = {
            1: 'I010',  # SAC/SIC (2 bytes)
            2: 'I140',  # Time of Day (3 bytes)
            3: 'I020',  # Target Report Descriptor (1 byte)
            4: 'I040',  # Position (4 bytes)
        }

    def _decode_data_item(self, frn, data, offset):
        """Decode data item based on FRN."""
        if frn == 1:  # I010 - SAC/SIC (2 bytes)
            if offset + 2 > len(data):
                raise DecoderError("I010 truncated")
            sac = data[offset]
            sic = data[offset + 1]
            return {'SAC': sac, 'SIC': sic}, 2

        elif frn == 2:  # I140 - Time of Day (3 bytes)
            if offset + 3 > len(data):
                raise DecoderError("I140 truncated")
            tod = int.from_bytes(data[offset:offset+3], 'big')
            return {'value': tod}, 3

        elif frn == 3:  # I020 - Target Report Descriptor (1 byte)
            if offset + 1 > len(data):
                raise DecoderError("I020 truncated")
            return {'value': data[offset]}, 1

        elif frn == 4:  # I040 - Position (4 bytes)
            if offset + 4 > len(data):
                raise DecoderError("I040 truncated")
            lat = struct.unpack('!h', data[offset:offset+2])[0]
            lon = struct.unpack('!h', data[offset+2:offset+4])[0]
            return {'latitude': lat, 'longitude': lon}, 4

        else:
            raise DecoderError(f"Unknown FRN {frn}")


class FailingDecoder(AsterixDecoder):
    """Decoder that always fails for testing error handling."""

    def _init_frn_map(self):
        self.frn_map = {1: 'I010'}

    def _decode_data_item(self, frn, data, offset):
        raise ValueError("Intentional decode failure")


class TestAsterixDecoder(unittest.TestCase):
    """Tests for AsterixDecoder base class."""

    def test_init_verbose_true(self):
        """Test decoder initialization with verbose=True."""
        decoder = ConcreteDecoder(category=48, verbose=True)
        self.assertEqual(decoder.category, 48)
        self.assertTrue(decoder.verbose)
        self.assertIn(1, decoder.frn_map)

    def test_init_verbose_false(self):
        """Test decoder initialization with verbose=False."""
        decoder = ConcreteDecoder(category=62, verbose=False)
        self.assertEqual(decoder.category, 62)
        self.assertFalse(decoder.verbose)

    def test_decode_record_simple(self):
        """Test decoding a simple record."""
        decoder = ConcreteDecoder(category=48)

        # Build record: FSPEC (FRN 1, 2) + I010 + I140
        fspec = b'\xC0'  # 0b11000000 = FRN 1, 2
        i010 = b'\x10\x20'  # SAC=16, SIC=32
        i140 = b'\x01\x02\x03'  # TOD value
        data = fspec + i010 + i140

        record, consumed = decoder.decode_record(data)

        self.assertEqual(record['category'], 48)
        self.assertEqual(record['I010'], {'SAC': 16, 'SIC': 32})
        self.assertEqual(record['I140'], {'value': 0x010203})
        self.assertEqual(record['len'], len(data))
        self.assertEqual(consumed, len(data))

    def test_decode_record_with_offset(self):
        """Test decoding record with non-zero offset."""
        decoder = ConcreteDecoder(category=48)

        # Add padding before record
        padding = b'\xff\xff'
        fspec = b'\x80'  # FRN 1 only
        i010 = b'\x10\x20'
        data = padding + fspec + i010

        record, consumed = decoder.decode_record(data, offset=2)
        self.assertEqual(record['I010'], {'SAC': 16, 'SIC': 32})
        self.assertEqual(consumed, 3)  # FSPEC + I010

    def test_decode_record_offset_at_end(self):
        """Test decode_record with offset at data end."""
        decoder = ConcreteDecoder(category=48)
        data = b'\xC0\x10\x20'

        with self.assertRaises(DecoderError) as cm:
            decoder.decode_record(data, offset=3)
        self.assertIn("offset", str(cm.exception).lower())

    def test_decode_record_offset_beyond_end(self):
        """Test decode_record with offset beyond data end."""
        decoder = ConcreteDecoder(category=48)
        data = b'\xC0\x10\x20'

        with self.assertRaises(DecoderError) as cm:
            decoder.decode_record(data, offset=10)
        self.assertIn("offset", str(cm.exception).lower())

    def test_decode_record_item_decode_failure(self):
        """Test record decode when item decode fails."""
        decoder = FailingDecoder(category=48, verbose=True)

        # FSPEC with FRN 1
        fspec = b'\x80'
        i010 = b'\x10\x20'
        data = fspec + i010

        with self.assertRaises(DecoderError) as cm:
            decoder.decode_record(data)

        # Should propagate the error and include item name
        self.assertIn("I010", str(cm.exception))
        self.assertIn("FRN", str(cm.exception))

    def test_decode_datablock_simple(self):
        """Test decoding a simple data block."""
        decoder = ConcreteDecoder(category=48)

        # Build record
        fspec = b'\x80'  # FRN 1 only
        i010 = b'\x10\x20'
        record_data = fspec + i010

        # Build data block
        cat = struct.pack('B', 48)
        length = struct.pack('!H', 3 + len(record_data))
        datablock = cat + length + record_data

        records = decoder.decode_datablock(datablock)

        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]['category'], 48)
        self.assertEqual(records[0]['I010'], {'SAC': 16, 'SIC': 32})

    def test_decode_datablock_too_short(self):
        """Test data block that's too short (< 3 bytes)."""
        decoder = ConcreteDecoder(category=48)

        with self.assertRaises(DecoderError) as cm:
            decoder.decode_datablock(b'\x30\x00')
        self.assertIn("too short", str(cm.exception).lower())

    def test_decode_datablock_category_mismatch(self):
        """Test data block with wrong category."""
        decoder = ConcreteDecoder(category=48)

        # Build block with category 62 instead of 48
        cat = struct.pack('B', 62)
        length = struct.pack('!H', 3)
        datablock = cat + length

        with self.assertRaises(DecoderError) as cm:
            decoder.decode_datablock(datablock)
        self.assertIn("mismatch", str(cm.exception).lower())
        self.assertIn("48", str(cm.exception))
        self.assertIn("62", str(cm.exception))

    def test_decode_datablock_length_exceeds_data(self):
        """Test data block where declared length > actual data."""
        decoder = ConcreteDecoder(category=48)

        cat = struct.pack('B', 48)
        length = struct.pack('!H', 100)  # Claims 100 bytes
        datablock = cat + length + b'\x00'  # But only 4 bytes total

        with self.assertRaises(DecoderError) as cm:
            decoder.decode_datablock(datablock)
        self.assertIn("exceeds", str(cm.exception).lower())

    def test_decode_datablock_record_decode_failure(self):
        """Test data block where record decoding fails."""
        decoder = FailingDecoder(category=48, verbose=True)

        # Build valid block structure but decoder will fail
        fspec = b'\x80'
        i010 = b'\x10\x20'
        record_data = fspec + i010

        cat = struct.pack('B', 48)
        length = struct.pack('!H', 3 + len(record_data))
        datablock = cat + length + record_data

        with self.assertRaises(DecoderError) as cm:
            decoder.decode_datablock(datablock)
        self.assertIn("record", str(cm.exception).lower())

    def test_decode_multiple_blocks_single_block(self):
        """Test decode_multiple_blocks with single block."""
        decoder = ConcreteDecoder(category=48)

        # Build one block
        fspec = b'\x80'
        i010 = b'\x10\x20'
        record_data = fspec + i010

        cat = struct.pack('B', 48)
        length = struct.pack('!H', 3 + len(record_data))
        block1 = cat + length + record_data

        records = decoder.decode_multiple_blocks(block1)
        self.assertEqual(len(records), 1)

    def test_decode_multiple_blocks_two_blocks(self):
        """Test decode_multiple_blocks with two consecutive blocks."""
        decoder = ConcreteDecoder(category=48)

        # Build first block
        fspec1 = b'\x80'
        i010_1 = b'\x10\x20'
        record1 = fspec1 + i010_1
        cat1 = struct.pack('B', 48)
        length1 = struct.pack('!H', 3 + len(record1))
        block1 = cat1 + length1 + record1

        # Build second block
        fspec2 = b'\x80'
        i010_2 = b'\x11\x21'
        record2 = fspec2 + i010_2
        cat2 = struct.pack('B', 48)
        length2 = struct.pack('!H', 3 + len(record2))
        block2 = cat2 + length2 + record2

        data = block1 + block2
        records = decoder.decode_multiple_blocks(data)

        self.assertEqual(len(records), 2)
        self.assertEqual(records[0]['I010'], {'SAC': 16, 'SIC': 32})
        self.assertEqual(records[1]['I010'], {'SAC': 17, 'SIC': 33})

    def test_decode_multiple_blocks_partial_header(self):
        """Test decode_multiple_blocks with incomplete header at end."""
        decoder = ConcreteDecoder(category=48)

        # Build one complete block
        fspec = b'\x80'
        i010 = b'\x10\x20'
        record_data = fspec + i010
        cat = struct.pack('B', 48)
        length = struct.pack('!H', 3 + len(record_data))
        block1 = cat + length + record_data

        # Add partial header (only 2 bytes instead of 3)
        data = block1 + b'\x30\x00'

        # Should decode first block and stop (not raise error)
        records = decoder.decode_multiple_blocks(data)
        self.assertEqual(len(records), 1)

    def test_decode_multiple_blocks_length_extends_beyond(self):
        """Test decode_multiple_blocks where block length extends beyond data."""
        decoder = ConcreteDecoder(category=48)

        # Build block with length that extends beyond actual data
        cat = struct.pack('B', 48)
        length = struct.pack('!H', 100)  # Claims 100 bytes
        data = cat + length + b'\x80\x10\x20'

        with self.assertRaises(DecoderError) as cm:
            decoder.decode_multiple_blocks(data)
        self.assertIn("beyond", str(cm.exception).lower())


class TestUtilityFunctions(unittest.TestCase):
    """Tests for utility decoding functions."""

    def test_decode_uint_single_byte(self):
        """Test decode_uint with 1 byte."""
        data = b'\x00\xFF\x12'
        self.assertEqual(decode_uint(data, 0, 1), 0)
        self.assertEqual(decode_uint(data, 1, 1), 255)
        self.assertEqual(decode_uint(data, 2, 1), 18)

    def test_decode_uint_two_bytes(self):
        """Test decode_uint with 2 bytes."""
        data = b'\x12\x34\x56'
        self.assertEqual(decode_uint(data, 0, 2), 0x1234)
        self.assertEqual(decode_uint(data, 1, 2), 0x3456)

    def test_decode_uint_four_bytes(self):
        """Test decode_uint with 4 bytes."""
        data = b'\x12\x34\x56\x78'
        self.assertEqual(decode_uint(data, 0, 4), 0x12345678)

    def test_decode_uint_offset(self):
        """Test decode_uint with offset."""
        data = b'\xFF\xFF\x12\x34'
        self.assertEqual(decode_uint(data, 2, 2), 0x1234)

    def test_decode_uint_insufficient_data(self):
        """Test decode_uint with insufficient data."""
        data = b'\x12\x34'
        with self.assertRaises(DecoderError) as cm:
            decode_uint(data, 1, 2)
        self.assertIn("cannot read", str(cm.exception).lower())

    def test_decode_int_positive(self):
        """Test decode_int with positive values."""
        data = b'\x00\x7F\x00\x01'
        self.assertEqual(decode_int(data, 0, 1), 0)
        self.assertEqual(decode_int(data, 1, 1), 127)
        self.assertEqual(decode_int(data, 2, 2), 1)

    def test_decode_int_negative(self):
        """Test decode_int with negative values (two's complement)."""
        data = b'\xFF\x80'
        self.assertEqual(decode_int(data, 0, 1), -1)
        self.assertEqual(decode_int(data, 1, 1), -128)

    def test_decode_int_two_bytes_negative(self):
        """Test decode_int with 2-byte negative value."""
        data = b'\xFF\xFF'
        self.assertEqual(decode_int(data, 0, 2), -1)

    def test_decode_int_insufficient_data(self):
        """Test decode_int with insufficient data."""
        data = b'\x12'
        with self.assertRaises(DecoderError) as cm:
            decode_int(data, 0, 2)
        self.assertIn("cannot read", str(cm.exception).lower())

    def test_decode_octal_basic(self):
        """Test decode_octal with basic mask."""
        data = b'\x0F\xFF'
        # 0x0FFF with mask 0x0FFF = 0xFFF
        self.assertEqual(decode_octal(data, 0, 2, 0x0FFF), 0x0FFF)

    def test_decode_octal_mode3a(self):
        """Test decode_octal for Mode 3/A code (common ASTERIX usage)."""
        # Mode 3/A: 12 bits in octal format
        data = b'\x0A\xBC'  # Binary: 0000 1010 1011 1100
        mask = 0x0FFF
        result = decode_octal(data, 0, 2, mask)
        self.assertEqual(result, 0x0ABC)

    def test_decode_octal_insufficient_data(self):
        """Test decode_octal with insufficient data."""
        data = b'\x12'
        with self.assertRaises(DecoderError):
            decode_octal(data, 0, 2, 0xFFFF)

    def test_decode_6bit_ascii_simple(self):
        """Test decode_6bit_ascii with simple characters."""
        # 'A' = 1, 'B' = 2 in 6-bit ASCII
        # 6 bits each: 000001 000010 = 0x04 0x80 (when packed)
        # First byte: 00|0001|00 = 0x04
        # Second byte: |00|10|0000 = 0x20
        data = b'\x04\x20\x00'
        result = decode_6bit_ascii(data, 0, 2)
        self.assertEqual(result, 'AB')

    def test_decode_6bit_ascii_space(self):
        """Test decode_6bit_ascii with space (value 32)."""
        # Space = 32 (0b100000)
        # Packed into bytes: 100000 = 0x80 shifted appropriately
        # First space at bit 0: 10|0000|00 = 0x80
        data = b'\x80\x00'
        result = decode_6bit_ascii(data, 0, 1)
        self.assertEqual(result, ' ')

    def test_decode_6bit_ascii_digits(self):
        """Test decode_6bit_ascii with digits."""
        # '0' = 48 in 6-bit ASCII
        # 6 bits: 110000
        data = b'\xC0\x00'
        result = decode_6bit_ascii(data, 0, 1)
        self.assertEqual(result, '0')

    def test_decode_6bit_ascii_unknown_char(self):
        """Test decode_6bit_ascii with unknown value (should return '?')."""
        # Value 63 (0b111111) is not mapped
        data = b'\xFC\x00'
        result = decode_6bit_ascii(data, 0, 1)
        self.assertEqual(result, '?')

    def test_decode_6bit_ascii_insufficient_data(self):
        """Test decode_6bit_ascii with insufficient data."""
        data = b'\x00'
        with self.assertRaises(DecoderError) as cm:
            decode_6bit_ascii(data, 0, 10)  # Needs more bytes
        self.assertIn("cannot read", str(cm.exception).lower())

    def test_bytes_consumed_for_fspec_empty(self):
        """Test bytes_consumed_for_fspec with empty FRN list."""
        self.assertEqual(bytes_consumed_for_fspec([]), 0)

    def test_bytes_consumed_for_fspec_first_octet(self):
        """Test bytes_consumed_for_fspec with FRNs in first octet."""
        self.assertEqual(bytes_consumed_for_fspec([1, 2, 3]), 1)
        self.assertEqual(bytes_consumed_for_fspec([7]), 1)

    def test_bytes_consumed_for_fspec_second_octet(self):
        """Test bytes_consumed_for_fspec with FRNs in second octet."""
        self.assertEqual(bytes_consumed_for_fspec([8]), 2)
        self.assertEqual(bytes_consumed_for_fspec([8, 9, 10]), 2)
        self.assertEqual(bytes_consumed_for_fspec([14]), 2)

    def test_bytes_consumed_for_fspec_third_octet(self):
        """Test bytes_consumed_for_fspec with FRNs in third octet."""
        self.assertEqual(bytes_consumed_for_fspec([15]), 3)
        self.assertEqual(bytes_consumed_for_fspec([21]), 3)

    def test_bytes_consumed_for_fspec_mixed(self):
        """Test bytes_consumed_for_fspec with mixed FRNs."""
        # Max FRN is 10, so needs 2 octets
        self.assertEqual(bytes_consumed_for_fspec([1, 5, 10]), 2)
        # Max FRN is 20, so needs 3 octets
        self.assertEqual(bytes_consumed_for_fspec([1, 7, 14, 20]), 3)


class TestDecoderError(unittest.TestCase):
    """Tests for DecoderError exception class."""

    def test_decoder_error_creation(self):
        """Test creating DecoderError."""
        error = DecoderError("Test error message")
        self.assertIsInstance(error, Exception)
        self.assertEqual(str(error), "Test error message")

    def test_decoder_error_raise(self):
        """Test raising DecoderError."""
        with self.assertRaises(DecoderError) as cm:
            raise DecoderError("Test error")
        self.assertEqual(str(cm.exception), "Test error")


if __name__ == '__main__':
    unittest.main()
