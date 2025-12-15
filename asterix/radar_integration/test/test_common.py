"""
Test suite for ASTERIX encoder common utilities.

Tests the shared encoding functions in asterix/radar_integration/encoder/common.py
with comprehensive coverage including boundary values, edge cases, and precision
verification.

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-12-14
"""

import struct
import sys
import time
import unittest
from pathlib import Path

# Add project root to path
project_root = Path(__file__).parent.parent.parent.parent
sys.path.insert(0, str(project_root))

from asterix.radar_integration.encoder.common import (
    ASTERIXDataItem,
    encode_fspec,
    encode_i010,
    encode_time_of_day,
    encode_wgs84_position,
    encode_aircraft_address,
    encode_callsign,
    encode_track_number,
    encode_flight_level,
    encode_cartesian_velocity,
)


class TestASTERIXDataItem(unittest.TestCase):
    """Tests for ASTERIXDataItem dataclass."""

    def test_creation(self):
        """Test basic dataclass creation."""
        item = ASTERIXDataItem(frn=1, data=b'\x00\x01')
        self.assertEqual(item.frn, 1)
        self.assertEqual(item.data, b'\x00\x01')

    def test_equality(self):
        """Test dataclass equality."""
        item1 = ASTERIXDataItem(frn=1, data=b'\x00\x01')
        item2 = ASTERIXDataItem(frn=1, data=b'\x00\x01')
        self.assertEqual(item1, item2)

    def test_inequality(self):
        """Test dataclass inequality."""
        item1 = ASTERIXDataItem(frn=1, data=b'\x00\x01')
        item2 = ASTERIXDataItem(frn=2, data=b'\x00\x01')
        self.assertNotEqual(item1, item2)


class TestEncodeFspec(unittest.TestCase):
    """Tests for encode_fspec function."""

    def test_empty_list(self):
        """Test FSPEC with no items."""
        result = encode_fspec([])
        self.assertEqual(result, b'')

    def test_single_item_frn1(self):
        """Test FSPEC with FRN 1 only."""
        result = encode_fspec([1])
        # FRN 1 is bit 8 (0x80), no extension
        self.assertEqual(result, b'\x80')

    def test_single_item_frn7(self):
        """Test FSPEC with FRN 7 only."""
        result = encode_fspec([7])
        # FRN 7 is bit 2 (0x02), no extension
        self.assertEqual(result, b'\x02')

    def test_first_seven_items(self):
        """Test FSPEC with FRN 1-7 (all bits in first octet)."""
        result = encode_fspec([1, 2, 3, 4, 5, 6, 7])
        # All bits 8-2 set: 11111110 = 0xFE
        self.assertEqual(result, b'\xfe')

    def test_frn8_requires_extension(self):
        """Test FSPEC with FRN 8 requires extension bit."""
        result = encode_fspec([8])
        # First octet: extension bit only (0x01)
        # Second octet: FRN 8 is bit 8 (0x80)
        self.assertEqual(result, b'\x01\x80')

    def test_frn1_and_frn8(self):
        """Test FSPEC with FRN 1 and FRN 8."""
        result = encode_fspec([1, 8])
        # First octet: FRN 1 (0x80) + extension (0x01) = 0x81
        # Second octet: FRN 8 (0x80)
        self.assertEqual(result, b'\x81\x80')

    def test_frn14_end_of_second_octet(self):
        """Test FSPEC with FRN 14 (last in second octet)."""
        result = encode_fspec([14])
        # First octet: extension only (0x01)
        # Second octet: FRN 14 is bit 2 (0x02)
        self.assertEqual(result, b'\x01\x02')

    def test_frn15_requires_third_octet(self):
        """Test FSPEC with FRN 15 requires third octet."""
        result = encode_fspec([15])
        # First octet: extension (0x01)
        # Second octet: extension (0x01)
        # Third octet: FRN 15 is bit 8 (0x80)
        self.assertEqual(result, b'\x01\x01\x80')

    def test_frn21_end_of_third_octet(self):
        """Test FSPEC with FRN 21."""
        result = encode_fspec([21])
        self.assertEqual(len(result), 3)
        # FRN 21 should be bit 2 of third octet
        self.assertEqual(result[2] & 0x02, 0x02)

    def test_frn27_fourth_octet(self):
        """Test FSPEC with FRN 27 (used in CAT062)."""
        result = encode_fspec([27])
        self.assertEqual(len(result), 4)
        # Extension bits in first 3 octets
        self.assertEqual(result[0] & 0x01, 0x01)
        self.assertEqual(result[1] & 0x01, 0x01)
        self.assertEqual(result[2] & 0x01, 0x01)

    def test_multiple_frns_scattered(self):
        """Test FSPEC with scattered FRNs."""
        result = encode_fspec([1, 3, 4, 6, 12])
        self.assertEqual(len(result), 2)
        # First octet: FRN 1,3,4,6 + extension
        # FRN 1=0x80, FRN 3=0x20, FRN 4=0x10, FRN 6=0x04, ext=0x01
        self.assertEqual(result[0], 0x80 | 0x20 | 0x10 | 0x04 | 0x01)
        # Second octet: FRN 12 (bit 4 = 0x08)
        self.assertEqual(result[1], 0x08)

    def test_unsorted_frns(self):
        """Test FSPEC with unsorted FRN list."""
        result1 = encode_fspec([3, 1, 2])
        result2 = encode_fspec([1, 2, 3])
        self.assertEqual(result1, result2)


class TestEncodeI010(unittest.TestCase):
    """Tests for encode_i010 (SAC/SIC) function."""

    def test_zero_values(self):
        """Test SAC=0, SIC=0."""
        result = encode_i010(0, 0)
        self.assertEqual(result, b'\x00\x00')

    def test_typical_values(self):
        """Test typical SAC/SIC values."""
        result = encode_i010(0, 1)
        self.assertEqual(result, b'\x00\x01')

    def test_max_values(self):
        """Test maximum SAC=255, SIC=255."""
        result = encode_i010(255, 255)
        self.assertEqual(result, b'\xff\xff')

    def test_different_sac_sic(self):
        """Test different SAC and SIC values."""
        result = encode_i010(10, 20)
        self.assertEqual(result, b'\x0a\x14')

    def test_length(self):
        """Test that result is always 2 bytes."""
        result = encode_i010(0, 0)
        self.assertEqual(len(result), 2)


class TestEncodeTimeOfDay(unittest.TestCase):
    """Tests for encode_time_of_day function."""

    def test_midnight(self):
        """Test encoding at midnight (0 seconds)."""
        # Midnight = 0 seconds since midnight
        result = encode_time_of_day(0.0)
        self.assertEqual(result, b'\x00\x00\x00')

    def test_one_second(self):
        """Test encoding at 1 second past midnight."""
        result = encode_time_of_day(1.0)
        # 1 second = 128 units = 0x000080
        self.assertEqual(result, b'\x00\x00\x80')

    def test_noon(self):
        """Test encoding at noon (43200 seconds)."""
        result = encode_time_of_day(43200.0)
        # 43200 * 128 = 5529600 = 0x546000
        self.assertEqual(result, b'\x54\x60\x00')

    def test_end_of_day(self):
        """Test encoding at end of day (86399 seconds)."""
        result = encode_time_of_day(86399.0)
        # 86399 * 128 = 11059072 = 0xA8BF80
        expected = struct.pack('>I', 11059072)[1:]
        self.assertEqual(result, expected)

    def test_wraps_at_midnight(self):
        """Test that times wrap at midnight."""
        result1 = encode_time_of_day(0.0)
        result2 = encode_time_of_day(86400.0)  # Next day
        self.assertEqual(result1, result2)

    def test_fractional_seconds(self):
        """Test fractional second precision."""
        # 0.5 seconds = 64 units
        result = encode_time_of_day(0.5)
        time_units = struct.unpack('>I', b'\x00' + result)[0]
        self.assertEqual(time_units, 64)

    def test_resolution_128th_second(self):
        """Test 1/128 second resolution."""
        # 1/128 second = 1 unit
        result = encode_time_of_day(1.0 / 128.0)
        time_units = struct.unpack('>I', b'\x00' + result)[0]
        self.assertEqual(time_units, 1)

    def test_length(self):
        """Test that result is always 3 bytes."""
        result = encode_time_of_day(12345.0)
        self.assertEqual(len(result), 3)


class TestEncodeWGS84Position(unittest.TestCase):
    """Tests for encode_wgs84_position function."""

    def test_origin(self):
        """Test encoding at origin (0, 0)."""
        result = encode_wgs84_position(0.0, 0.0)
        self.assertEqual(result, b'\x00\x00\x00\x00\x00\x00')

    def test_positive_coordinates(self):
        """Test positive lat/lon (Eiffel Tower)."""
        result = encode_wgs84_position(48.8584, 2.2945)
        self.assertEqual(len(result), 6)
        # Verify non-zero encoding
        self.assertNotEqual(result, b'\x00\x00\x00\x00\x00\x00')

    def test_negative_latitude(self):
        """Test negative latitude (Southern Hemisphere)."""
        result = encode_wgs84_position(-33.8688, 151.2093)  # Sydney
        self.assertEqual(len(result), 6)
        # First 3 bytes (latitude) should be negative (high bit set in 24-bit)
        lat_bytes = result[:3]
        lat_value = struct.unpack('>i', b'\xff' + lat_bytes if lat_bytes[0] & 0x80 else b'\x00' + lat_bytes)[0]
        self.assertLess(lat_value, 0)

    def test_negative_longitude(self):
        """Test negative longitude (Western Hemisphere)."""
        result = encode_wgs84_position(40.7128, -74.0060)  # New York
        self.assertEqual(len(result), 6)
        # Last 3 bytes (longitude) should encode the negative value
        lon_bytes = result[3:]
        # Sign extend 24-bit to 32-bit
        if lon_bytes[0] & 0x80:  # Negative (MSB set)
            lon_value = struct.unpack('>i', b'\xff' + lon_bytes)[0]
        else:
            lon_value = struct.unpack('>i', b'\x00' + lon_bytes)[0]
        # -74 degrees should give negative encoded value
        self.assertLess(lon_value, 0)

    def test_north_pole(self):
        """Test encoding at North Pole (+90, 0)."""
        result = encode_wgs84_position(90.0, 0.0)
        self.assertEqual(len(result), 6)

    def test_south_pole(self):
        """Test encoding at South Pole (-90, 0)."""
        result = encode_wgs84_position(-90.0, 0.0)
        self.assertEqual(len(result), 6)

    def test_international_date_line_positive(self):
        """Test encoding at +180 longitude."""
        result = encode_wgs84_position(0.0, 180.0)
        self.assertEqual(len(result), 6)

    def test_international_date_line_negative(self):
        """Test encoding at -180 longitude."""
        result = encode_wgs84_position(0.0, -180.0)
        self.assertEqual(len(result), 6)

    def test_high_precision_mode(self):
        """Test high precision encoding mode."""
        result_std = encode_wgs84_position(48.8584, 2.2945, high_precision=False)
        result_hp = encode_wgs84_position(48.8584, 2.2945, high_precision=True)
        # Different precision should give different results
        self.assertNotEqual(result_std, result_hp)

    def test_standard_precision_scale(self):
        """Test standard precision uses 180/2^23 scale (fits full ±180° range)."""
        lat = 45.0  # 45 degrees
        result = encode_wgs84_position(lat, 0.0, high_precision=False)
        lat_bytes = result[:3]
        lat_value = struct.unpack('>i', b'\x00' + lat_bytes)[0]
        # Expected: 45 * (2^23 / 180) = 45 * 46603.38... ≈ 2097152
        expected = int(45.0 * (2**23 / 180.0))
        self.assertEqual(lat_value, expected)

    def test_high_precision_scale(self):
        """Test high precision uses 180/2^25 scale (higher resolution, limited range)."""
        lat = 45.0
        result = encode_wgs84_position(lat, 0.0, high_precision=True)
        lat_bytes = result[:3]
        lat_value = struct.unpack('>i', b'\x00' + lat_bytes)[0]
        # Expected: 45 * (2^25 / 180) = 45 * 186413.51... ≈ 8388608
        # This is exactly at the 24-bit boundary (clamped to max 8388607)
        expected = min(int(45.0 * (2**25 / 180.0)), 8388607)
        self.assertEqual(lat_value, expected)

    def test_length(self):
        """Test that result is always 6 bytes."""
        result = encode_wgs84_position(48.8584, 2.2945)
        self.assertEqual(len(result), 6)


class TestEncodeAircraftAddress(unittest.TestCase):
    """Tests for encode_aircraft_address function."""

    def test_zero_address(self):
        """Test zero ICAO address."""
        result = encode_aircraft_address(0x000000)
        self.assertEqual(result, b'\x00\x00\x00')

    def test_max_address(self):
        """Test maximum ICAO address (0xFFFFFF)."""
        result = encode_aircraft_address(0xFFFFFF)
        self.assertEqual(result, b'\xff\xff\xff')

    def test_typical_address(self):
        """Test typical ICAO address."""
        result = encode_aircraft_address(0x3C6544)  # German registration
        self.assertEqual(result, b'\x3c\x65\x44')

    def test_us_address_range(self):
        """Test US ICAO address range (0xA00000-0xAFFFFF)."""
        result = encode_aircraft_address(0xA12345)
        self.assertEqual(result, b'\xa1\x23\x45')

    def test_truncates_to_24_bits(self):
        """Test that addresses are truncated to 24 bits."""
        result = encode_aircraft_address(0xFF123456)  # 32-bit value
        self.assertEqual(result, b'\x12\x34\x56')  # Only last 24 bits

    def test_length(self):
        """Test that result is always 3 bytes."""
        result = encode_aircraft_address(0x123456)
        self.assertEqual(len(result), 3)


class TestEncodeCallsign(unittest.TestCase):
    """Tests for encode_callsign function."""

    def test_typical_callsign(self):
        """Test typical airline callsign."""
        result = encode_callsign("DLH123")
        self.assertEqual(len(result), 6)
        # Verify non-zero encoding
        self.assertNotEqual(result, b'\x00\x00\x00\x00\x00\x00')

    def test_8_character_callsign(self):
        """Test maximum 8 character callsign."""
        result = encode_callsign("DLH12345")
        self.assertEqual(len(result), 6)

    def test_short_callsign_padded(self):
        """Test short callsign is padded with spaces."""
        result1 = encode_callsign("ABC")
        result2 = encode_callsign("ABC     ")  # Explicitly padded
        self.assertEqual(result1, result2)

    def test_long_callsign_truncated(self):
        """Test long callsign is truncated to 8 characters."""
        result = encode_callsign("DLH123456789")  # 12 chars
        # Should only use first 8 characters
        result2 = encode_callsign("DLH12345")
        self.assertEqual(result, result2)

    def test_lowercase_converted(self):
        """Test lowercase is converted to uppercase."""
        result1 = encode_callsign("dlh123")
        result2 = encode_callsign("DLH123")
        self.assertEqual(result1, result2)

    def test_all_spaces(self):
        """Test all spaces callsign."""
        result = encode_callsign("        ")
        self.assertEqual(len(result), 6)

    def test_numeric_callsign(self):
        """Test numeric-only callsign."""
        result = encode_callsign("12345678")
        self.assertEqual(len(result), 6)

    def test_special_characters_become_space(self):
        """Test special characters are converted to space."""
        result1 = encode_callsign("ABC-123")
        result2 = encode_callsign("ABC 123")
        self.assertEqual(result1, result2)

    def test_length(self):
        """Test that result is always 6 bytes."""
        result = encode_callsign("TEST")
        self.assertEqual(len(result), 6)


class TestEncodeTrackNumber(unittest.TestCase):
    """Tests for encode_track_number function."""

    def test_zero(self):
        """Test track number 0."""
        result = encode_track_number(0)
        self.assertEqual(result, b'\x00\x00')

    def test_typical_value(self):
        """Test typical track number."""
        result = encode_track_number(1234)
        self.assertEqual(result, b'\x04\xd2')

    def test_max_value(self):
        """Test maximum track number (65535)."""
        result = encode_track_number(65535)
        self.assertEqual(result, b'\xff\xff')

    def test_truncates_to_16_bits(self):
        """Test that values are truncated to 16 bits."""
        result = encode_track_number(0x10000)  # 65536
        self.assertEqual(result, b'\x00\x00')  # Wraps to 0

    def test_large_value_mask(self):
        """Test large value is masked to 16 bits."""
        result = encode_track_number(0x12345678)
        self.assertEqual(result, b'\x56\x78')

    def test_length(self):
        """Test that result is always 2 bytes."""
        result = encode_track_number(12345)
        self.assertEqual(len(result), 2)


class TestEncodeFlightLevel(unittest.TestCase):
    """Tests for encode_flight_level function."""

    def test_fl0(self):
        """Test flight level 0."""
        result = encode_flight_level(0)
        self.assertEqual(result, b'\x00\x00')

    def test_fl350(self):
        """Test typical cruise FL350."""
        result = encode_flight_level(350)
        # 350 * 4 = 1400 = 0x0578
        self.assertEqual(result, b'\x05\x78')

    def test_fl100(self):
        """Test FL100."""
        result = encode_flight_level(100)
        # 100 * 4 = 400 = 0x0190
        self.assertEqual(result, b'\x01\x90')

    def test_negative_flight_level(self):
        """Test negative flight level (below sea level)."""
        result = encode_flight_level(-10)
        # -10 * 4 = -40 in two's complement
        value = struct.unpack('>h', result)[0]
        self.assertEqual(value, -40)

    def test_max_positive(self):
        """Test maximum positive flight level."""
        result = encode_flight_level(8191)  # Max for 16-bit signed / 4
        value = struct.unpack('>h', result)[0]
        self.assertGreater(value, 0)

    def test_quarter_fl_resolution(self):
        """Test 1/4 FL resolution."""
        result = encode_flight_level(1)
        # 1 FL = 4 units
        value = struct.unpack('>h', result)[0]
        self.assertEqual(value, 4)

    def test_length(self):
        """Test that result is always 2 bytes."""
        result = encode_flight_level(350)
        self.assertEqual(len(result), 2)


class TestEncodeCartesianVelocity(unittest.TestCase):
    """Tests for encode_cartesian_velocity function."""

    def test_zero_velocity(self):
        """Test zero velocity."""
        result = encode_cartesian_velocity(0.0, 0.0)
        self.assertEqual(result, b'\x00\x00\x00\x00')

    def test_positive_velocities(self):
        """Test positive velocities (NE direction)."""
        result = encode_cartesian_velocity(100.0, 200.0)
        # vx=100*4=400, vy=200*4=800
        vx, vy = struct.unpack('>hh', result)
        self.assertEqual(vx, 400)
        self.assertEqual(vy, 800)

    def test_negative_velocities(self):
        """Test negative velocities (SW direction)."""
        result = encode_cartesian_velocity(-100.0, -200.0)
        vx, vy = struct.unpack('>hh', result)
        self.assertEqual(vx, -400)
        self.assertEqual(vy, -800)

    def test_mixed_velocities(self):
        """Test mixed positive/negative velocities."""
        result = encode_cartesian_velocity(50.0, -75.0)
        vx, vy = struct.unpack('>hh', result)
        self.assertEqual(vx, 200)
        self.assertEqual(vy, -300)

    def test_resolution_025_ms(self):
        """Test 0.25 m/s resolution."""
        result = encode_cartesian_velocity(0.25, 0.25)
        vx, vy = struct.unpack('>hh', result)
        self.assertEqual(vx, 1)
        self.assertEqual(vy, 1)

    def test_typical_aircraft_speed(self):
        """Test typical aircraft cruise speed (~250 m/s)."""
        result = encode_cartesian_velocity(250.0, 0.0)
        vx, vy = struct.unpack('>hh', result)
        self.assertEqual(vx, 1000)
        self.assertEqual(vy, 0)

    def test_length(self):
        """Test that result is always 4 bytes."""
        result = encode_cartesian_velocity(100.0, 200.0)
        self.assertEqual(len(result), 4)


class TestEncodingRoundTrip(unittest.TestCase):
    """Tests for encoding consistency and round-trip verification."""

    def test_i010_decode_consistency(self):
        """Test that I010 encoding is consistent."""
        for sac in [0, 1, 127, 255]:
            for sic in [0, 1, 127, 255]:
                result = encode_i010(sac, sic)
                decoded_sac, decoded_sic = struct.unpack('BB', result)
                self.assertEqual(decoded_sac, sac)
                self.assertEqual(decoded_sic, sic)

    def test_track_number_decode_consistency(self):
        """Test that track number encoding is consistent."""
        for track in [0, 1, 1000, 65535]:
            result = encode_track_number(track)
            decoded = struct.unpack('>H', result)[0]
            self.assertEqual(decoded, track)

    def test_flight_level_decode_consistency(self):
        """Test that flight level encoding is consistent."""
        for fl in [0, 100, 350, 410, -10]:
            result = encode_flight_level(fl)
            decoded_units = struct.unpack('>h', result)[0]
            decoded_fl = decoded_units // 4
            self.assertEqual(decoded_fl, fl)

    def test_velocity_decode_consistency(self):
        """Test that velocity encoding is consistent."""
        test_cases = [(0.0, 0.0), (100.0, 200.0), (-50.0, 75.0)]
        for vx, vy in test_cases:
            result = encode_cartesian_velocity(vx, vy)
            decoded_vx, decoded_vy = struct.unpack('>hh', result)
            # Decode back to m/s
            self.assertEqual(decoded_vx / 4, vx)
            self.assertEqual(decoded_vy / 4, vy)


class TestEdgeCases(unittest.TestCase):
    """Tests for edge cases and boundary conditions."""

    def test_fspec_large_frn(self):
        """Test FSPEC with very large FRN."""
        result = encode_fspec([50])
        # FRN 50 needs 8 octets (50 / 7 = 7.14, round up)
        self.assertGreaterEqual(len(result), 7)

    def test_time_maximum_precision(self):
        """Test time encoding at maximum precision boundary."""
        # 86399.9921875 seconds (max before next day)
        result = encode_time_of_day(86399.9921875)
        time_units = struct.unpack('>I', b'\x00' + result)[0]
        # Should be less than 24-bit max
        self.assertLess(time_units, 0x1000000)

    def test_position_near_boundary(self):
        """Test position encoding near boundary values."""
        # Just under +90 latitude
        result = encode_wgs84_position(89.999999, 0.0)
        self.assertEqual(len(result), 6)

        # Just under +180 longitude
        result = encode_wgs84_position(0.0, 179.999999)
        self.assertEqual(len(result), 6)

    def test_callsign_unicode_handling(self):
        """Test callsign with non-ASCII characters."""
        # Non-ASCII should be converted to space
        result = encode_callsign("ABC€123")  # Euro sign
        self.assertEqual(len(result), 6)

    def test_velocity_near_limits(self):
        """Test velocity encoding near 16-bit limits."""
        # Near max positive
        result = encode_cartesian_velocity(8000.0, 0.0)
        vx, vy = struct.unpack('>hh', result)
        self.assertEqual(vx, 32000)

        # Near max negative
        result = encode_cartesian_velocity(-8000.0, 0.0)
        vx, vy = struct.unpack('>hh', result)
        self.assertEqual(vx, -32000)


if __name__ == '__main__':
    unittest.main()
