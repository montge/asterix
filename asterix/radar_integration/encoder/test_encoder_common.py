"""
Tests for ASTERIX Encoder Common Utilities

Tests the shared encoding functions used across multiple ASTERIX categories.
"""

import unittest
import struct
import time

from asterix.radar_integration.encoder.common import (
    ASTERIXDataItem,
    encode_fspec,
    encode_i010,
    encode_i140,
    encode_wgs84_position,
    encode_altitude,
    encode_velocity,
    encode_track_number,
    encode_time_of_day,
    build_data_block,
)


class TestASTERIXDataItem(unittest.TestCase):
    """Tests for ASTERIXDataItem dataclass."""

    def test_creation(self):
        """Test creating an ASTERIXDataItem."""
        item = ASTERIXDataItem(frn=1, data=b'\x01\x02')

        self.assertEqual(item.frn, 1)
        self.assertEqual(item.data, b'\x01\x02')

    def test_empty_data(self):
        """Test creating an item with empty data."""
        item = ASTERIXDataItem(frn=5, data=b'')

        self.assertEqual(item.frn, 5)
        self.assertEqual(item.data, b'')


class TestEncodeFspec(unittest.TestCase):
    """Tests for encode_fspec function."""

    def test_empty_items(self):
        """Test FSPEC with no items."""
        fspec = encode_fspec([])
        self.assertEqual(fspec, b'\x00')

    def test_single_item_frn1(self):
        """Test FSPEC with only FRN 1."""
        fspec = encode_fspec([1])
        # FRN 1 is bit 8 (MSB) of first octet
        self.assertEqual(fspec[0] & 0x80, 0x80)

    def test_single_item_frn7(self):
        """Test FSPEC with only FRN 7."""
        fspec = encode_fspec([7])
        # FRN 7 is bit 2 of first octet
        self.assertEqual(fspec[0] & 0x02, 0x02)

    def test_first_three_items(self):
        """Test FSPEC with FRN 1, 2, 3."""
        fspec = encode_fspec([1, 2, 3])
        # FRN 1,2,3 = bits 8,7,6 = 0xE0
        self.assertEqual(fspec[0] & 0xE0, 0xE0)

    def test_extension_bit_set(self):
        """Test that extension bit is set when FRN > 7."""
        fspec = encode_fspec([1, 8])
        # First octet should have extension bit (bit 1) set
        self.assertEqual(fspec[0] & 0x01, 0x01)

    def test_multiple_octets(self):
        """Test FSPEC spanning multiple octets."""
        fspec = encode_fspec([1, 8, 15])
        # Should have 3 octets
        self.assertEqual(len(fspec), 3)
        # First two octets should have extension bit set
        self.assertEqual(fspec[0] & 0x01, 0x01)
        self.assertEqual(fspec[1] & 0x01, 0x01)
        # Last octet should NOT have extension bit set
        self.assertEqual(fspec[2] & 0x01, 0x00)

    def test_all_first_seven_items(self):
        """Test FSPEC with all first 7 FRNs."""
        fspec = encode_fspec([1, 2, 3, 4, 5, 6, 7])
        # All bits 8-2 set, no extension
        self.assertEqual(fspec[0], 0xFE)


class TestEncodeI010(unittest.TestCase):
    """Tests for encode_i010 (SAC/SIC) function."""

    def test_basic_encoding(self):
        """Test basic SAC/SIC encoding."""
        data = encode_i010(sac=10, sic=20)

        self.assertEqual(len(data), 2)
        self.assertEqual(data[0], 10)
        self.assertEqual(data[1], 20)

    def test_zero_values(self):
        """Test encoding with zero SAC/SIC."""
        data = encode_i010(sac=0, sic=0)

        self.assertEqual(data, b'\x00\x00')

    def test_max_values(self):
        """Test encoding with maximum SAC/SIC (255)."""
        data = encode_i010(sac=255, sic=255)

        self.assertEqual(data, b'\xff\xff')

    def test_mixed_values(self):
        """Test encoding with different SAC/SIC values."""
        data = encode_i010(sac=100, sic=50)

        self.assertEqual(data[0], 100)
        self.assertEqual(data[1], 50)


class TestEncodeI140(unittest.TestCase):
    """Tests for encode_i140 (Time of Day) function."""

    def test_midnight(self):
        """Test encoding midnight (00:00:00)."""
        # Time of Day is seconds since midnight * 128
        data = encode_i140(0)

        self.assertEqual(len(data), 3)
        # 0 seconds = 0 * 128 = 0
        self.assertEqual(data, b'\x00\x00\x00')

    def test_one_second(self):
        """Test encoding 1 second after midnight."""
        data = encode_i140(1.0)

        # 1 second = 1 * 128 = 128 = 0x000080
        value = struct.unpack('>I', b'\x00' + data)[0]
        self.assertEqual(value, 128)

    def test_noon(self):
        """Test encoding noon (12:00:00)."""
        noon_seconds = 12 * 3600  # 43200 seconds
        data = encode_i140(noon_seconds)

        value = struct.unpack('>I', b'\x00' + data)[0]
        expected = noon_seconds * 128
        self.assertEqual(value, expected)


class TestEncodeWGS84Position(unittest.TestCase):
    """Tests for encode_wgs84_position function."""

    def test_zero_position(self):
        """Test encoding position at 0,0."""
        data = encode_wgs84_position(lat=0.0, lon=0.0)

        self.assertEqual(len(data), 8)  # 4 bytes each for lat/lon

    def test_positive_coordinates(self):
        """Test encoding positive lat/lon."""
        data = encode_wgs84_position(lat=47.5, lon=8.5)

        self.assertEqual(len(data), 8)
        # Verify data is non-zero
        self.assertNotEqual(data, b'\x00' * 8)

    def test_negative_coordinates(self):
        """Test encoding negative lat/lon."""
        data = encode_wgs84_position(lat=-33.8, lon=-122.4)

        self.assertEqual(len(data), 8)

    def test_max_coordinates(self):
        """Test encoding maximum lat/lon values."""
        data = encode_wgs84_position(lat=90.0, lon=180.0)

        self.assertEqual(len(data), 8)

    def test_min_coordinates(self):
        """Test encoding minimum lat/lon values."""
        data = encode_wgs84_position(lat=-90.0, lon=-180.0)

        self.assertEqual(len(data), 8)


class TestEncodeAltitude(unittest.TestCase):
    """Tests for encode_altitude function."""

    def test_zero_altitude(self):
        """Test encoding zero altitude."""
        data = encode_altitude(0)

        self.assertEqual(len(data), 2)

    def test_positive_altitude(self):
        """Test encoding positive altitude (feet)."""
        data = encode_altitude(35000)

        self.assertEqual(len(data), 2)

    def test_negative_altitude(self):
        """Test encoding negative altitude (below sea level)."""
        data = encode_altitude(-100)

        self.assertEqual(len(data), 2)


class TestEncodeVelocity(unittest.TestCase):
    """Tests for encode_velocity function."""

    def test_zero_velocity(self):
        """Test encoding zero velocity."""
        data = encode_velocity(vx=0.0, vy=0.0)

        self.assertEqual(len(data), 4)

    def test_positive_velocity(self):
        """Test encoding positive velocity components."""
        data = encode_velocity(vx=100.0, vy=50.0)

        self.assertEqual(len(data), 4)

    def test_negative_velocity(self):
        """Test encoding negative velocity components."""
        data = encode_velocity(vx=-50.0, vy=-25.0)

        self.assertEqual(len(data), 4)

    def test_mixed_velocity(self):
        """Test encoding mixed velocity components."""
        data = encode_velocity(vx=100.0, vy=-50.0)

        self.assertEqual(len(data), 4)


class TestEncodeTrackNumber(unittest.TestCase):
    """Tests for encode_track_number function."""

    def test_zero_track(self):
        """Test encoding track number 0."""
        data = encode_track_number(0)

        self.assertEqual(len(data), 2)
        self.assertEqual(data, b'\x00\x00')

    def test_basic_track(self):
        """Test encoding basic track number."""
        data = encode_track_number(1234)

        self.assertEqual(len(data), 2)
        value = struct.unpack('>H', data)[0]
        self.assertEqual(value, 1234)

    def test_max_track(self):
        """Test encoding maximum track number (65535)."""
        data = encode_track_number(65535)

        self.assertEqual(len(data), 2)
        value = struct.unpack('>H', data)[0]
        self.assertEqual(value, 65535)


class TestEncodeTimeOfDay(unittest.TestCase):
    """Tests for encode_time_of_day function."""

    def test_encoding(self):
        """Test time of day encoding."""
        data = encode_time_of_day(3600.0)  # 1 hour

        self.assertEqual(len(data), 3)

    def test_current_time(self):
        """Test encoding current time."""
        current = time.time() % 86400  # Seconds since midnight
        data = encode_time_of_day(current)

        self.assertEqual(len(data), 3)


class TestBuildDataBlock(unittest.TestCase):
    """Tests for build_data_block function."""

    def test_single_record(self):
        """Test building data block with single record."""
        records = [b'\x01\x02\x03']
        block = build_data_block(category=48, records=records)

        # Category (1) + Length (2) + records
        self.assertGreaterEqual(len(block), 4)
        self.assertEqual(block[0], 48)

    def test_empty_records(self):
        """Test building data block with no records."""
        records = []
        block = build_data_block(category=62, records=records)

        # Should still have header
        self.assertEqual(len(block), 3)
        self.assertEqual(block[0], 62)

    def test_multiple_records(self):
        """Test building data block with multiple records."""
        records = [b'\x01\x02', b'\x03\x04', b'\x05\x06']
        block = build_data_block(category=21, records=records)

        self.assertEqual(block[0], 21)
        # Length should include header + all records
        length = struct.unpack('>H', block[1:3])[0]
        self.assertEqual(length, 3 + 6)  # header + 3 records of 2 bytes each


if __name__ == '__main__':
    unittest.main()
