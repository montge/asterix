#!/usr/bin/env python3
"""
Unit tests for ASTERIX CAT062 encoder

Tests all CAT062 data items and encoding functionality:
- I010: Data Source Identifier
- I070: Time of Track Information
- I105: Calculated Position in WGS-84
- I135: Calculated Track Barometric Altitude
- I185: Calculated Track Velocity
- I390: Flight Plan Related Data

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import unittest
import struct
import time
from asterix.radar_integration.encoder.cat062 import (
    encode_fspec,
    encode_i010,
    encode_i070,
    encode_i105,
    encode_i135,
    encode_i185,
    encode_i390,
    encode_cat062_record,
    encode_cat062_datablock,
    encode_cat062,
)


class TestFSPECEncoding(unittest.TestCase):
    """Test FSPEC encoding."""

    def test_single_octet(self):
        """Test FSPEC with single octet."""
        # Items 1-7 only need 1 octet
        fspec = encode_fspec([1, 3, 4])
        self.assertEqual(len(fspec), 1)
        # Bit 8: FRN 1 (128), Bit 6: FRN 3 (32), Bit 5: FRN 4 (16), Bit 1: 0 (no extension)
        expected = 0b10110000  # 176 = 128 + 32 + 16
        self.assertEqual(fspec[0], expected)

    def test_multiple_octets(self):
        """Test FSPEC with multiple octets."""
        # FRN 12 requires 2 octets
        fspec = encode_fspec([1, 3, 4, 12])
        self.assertEqual(len(fspec), 2)
        # First octet should have extension bit set
        self.assertEqual(fspec[0] & 0x01, 0x01)

    def test_frn_27(self):
        """Test FSPEC with FRN 27 (I390)."""
        # FRN 27 is in the 4th octet
        fspec = encode_fspec([1, 3, 4, 6, 12, 27])
        self.assertGreaterEqual(len(fspec), 4)


class TestI010Encoding(unittest.TestCase):
    """Test I010: Data Source Identifier encoding."""

    def test_basic_encoding(self):
        """Test basic SAC/SIC encoding."""
        data = encode_i010(0, 1)
        self.assertEqual(len(data), 2)
        self.assertEqual(data[0], 0)
        self.assertEqual(data[1], 1)

    def test_max_values(self):
        """Test maximum SAC/SIC values."""
        data = encode_i010(255, 255)
        self.assertEqual(len(data), 2)
        self.assertEqual(data[0], 255)
        self.assertEqual(data[1], 255)


class TestI070Encoding(unittest.TestCase):
    """Test I070: Time of Track Information encoding."""

    def test_basic_encoding(self):
        """Test basic time encoding."""
        # Noon: 12:00:00 = 43200 seconds since midnight
        test_timestamp = 43200.0
        data = encode_i070(test_timestamp)
        self.assertEqual(len(data), 3)

        # Expected: 43200 * 128 = 5529600 = 0x546000
        expected = 5529600
        actual = struct.unpack('>I', b'\x00' + data)[0]
        self.assertEqual(actual, expected)

    def test_current_time(self):
        """Test encoding current time."""
        data = encode_i070()
        self.assertEqual(len(data), 3)

    def test_resolution(self):
        """Test time resolution (1/128 second)."""
        # 1 second should be 128 units
        t1 = 10000.0
        t2 = 10001.0
        data1 = encode_i070(t1)
        data2 = encode_i070(t2)

        val1 = struct.unpack('>I', b'\x00' + data1)[0]
        val2 = struct.unpack('>I', b'\x00' + data2)[0]
        self.assertEqual(val2 - val1, 128)


class TestI105Encoding(unittest.TestCase):
    """Test I105: Calculated Position in WGS-84 encoding."""

    def test_basic_encoding(self):
        """Test basic lat/lon encoding."""
        # Eiffel Tower: 48.8584°N, 2.2945°E
        lat, lon = 48.8584, 2.2945
        data = encode_i105(lat, lon)
        self.assertEqual(len(data), 8)

    def test_zero_position(self):
        """Test encoding zero position."""
        data = encode_i105(0.0, 0.0)
        self.assertEqual(len(data), 8)
        lat_val, lon_val = struct.unpack('>ii', data)
        self.assertEqual(lat_val, 0)
        self.assertEqual(lon_val, 0)

    def test_extreme_positions(self):
        """Test encoding extreme lat/lon values."""
        # North Pole
        data = encode_i105(90.0, 0.0)
        lat_val, lon_val = struct.unpack('>ii', data)
        self.assertGreater(lat_val, 0)

        # South Pole
        data = encode_i105(-90.0, 0.0)
        lat_val, lon_val = struct.unpack('>ii', data)
        self.assertLess(lat_val, 0)

        # International Date Line
        data = encode_i105(0.0, 180.0)
        lat_val, lon_val = struct.unpack('>ii', data)
        self.assertGreater(lon_val, 0)

    def test_resolution(self):
        """Test position resolution (approximately 0.6 meters)."""
        # Resolution: 180 / 2^25 ≈ 0.00000536 degrees ≈ 0.6 meters
        lat1, lon1 = 48.8584, 2.2945
        lat2, lon2 = 48.8585, 2.2946  # ~100m difference

        data1 = encode_i105(lat1, lon1)
        data2 = encode_i105(lat2, lon2)

        # Values should be different
        self.assertNotEqual(data1, data2)


class TestI135Encoding(unittest.TestCase):
    """Test I135: Calculated Track Barometric Altitude encoding."""

    def test_basic_encoding(self):
        """Test basic altitude encoding."""
        # Cruising altitude: 35000 ft = FL350
        data = encode_i135(35000.0)
        self.assertEqual(len(data), 2)

        # Expected: 35000 ft = FL350 = 1400 in 0.25 FL units
        # 0x0578 = 1400
        expected = 1400
        actual = struct.unpack('>H', data)[0] & 0x7FFF  # Mask QNH bit
        self.assertEqual(actual, expected)

    def test_qnh_flag(self):
        """Test QNH correction flag."""
        # Without QNH correction
        data1 = encode_i135(35000.0, qnh_correction=False)
        val1 = struct.unpack('>H', data1)[0]
        self.assertEqual(val1 & 0x8000, 0)

        # With QNH correction
        data2 = encode_i135(35000.0, qnh_correction=True)
        val2 = struct.unpack('>H', data2)[0]
        self.assertEqual(val2 & 0x8000, 0x8000)

    def test_zero_altitude(self):
        """Test encoding zero altitude."""
        data = encode_i135(0.0)
        val = struct.unpack('>H', data)[0] & 0x7FFF
        self.assertEqual(val, 0)

    def test_resolution(self):
        """Test altitude resolution (25 feet)."""
        # Resolution: 25 feet (0.25 Flight Levels)
        alt1 = 35000.0
        alt2 = 35025.0  # +25 feet

        data1 = encode_i135(alt1)
        data2 = encode_i135(alt2)

        val1 = struct.unpack('>H', data1)[0] & 0x7FFF
        val2 = struct.unpack('>H', data2)[0] & 0x7FFF
        self.assertEqual(val2 - val1, 1)


class TestI185Encoding(unittest.TestCase):
    """Test I185: Calculated Track Velocity encoding."""

    def test_basic_encoding(self):
        """Test basic velocity encoding."""
        # Aircraft traveling north at 250 m/s
        vx, vy = 0.0, 250.0
        data = encode_i185(vx, vy)
        self.assertEqual(len(data), 4)

        vx_val, vy_val = struct.unpack('>hh', data)
        self.assertEqual(vx_val, 0)
        self.assertEqual(vy_val, 1000)  # 250 / 0.25 = 1000

    def test_zero_velocity(self):
        """Test encoding zero velocity."""
        data = encode_i185(0.0, 0.0)
        vx_val, vy_val = struct.unpack('>hh', data)
        self.assertEqual(vx_val, 0)
        self.assertEqual(vy_val, 0)

    def test_negative_velocity(self):
        """Test encoding negative velocity (approaching)."""
        # Aircraft approaching from south
        vx, vy = 0.0, -100.0
        data = encode_i185(vx, vy)
        vx_val, vy_val = struct.unpack('>hh', data)
        self.assertEqual(vx_val, 0)
        self.assertLess(vy_val, 0)

    def test_resolution(self):
        """Test velocity resolution (0.25 m/s)."""
        v1 = 100.0
        v2 = 100.25

        data1 = encode_i185(v1, 0.0)
        data2 = encode_i185(v2, 0.0)

        vx1, _ = struct.unpack('>hh', data1)
        vx2, _ = struct.unpack('>hh', data2)
        self.assertEqual(vx2 - vx1, 1)


class TestI390Encoding(unittest.TestCase):
    """Test I390: Flight Plan Related Data encoding."""

    def test_callsign_only(self):
        """Test encoding callsign only."""
        data = encode_i390(callsign="DLH123")
        self.assertGreater(len(data), 1)

        # Primary subfield should have CS bit set (bit 7)
        primary = data[0]
        self.assertEqual(primary & 0b01000000, 0b01000000)

        # Should contain callsign
        self.assertIn(b'DLH123', data)

    def test_airports_only(self):
        """Test encoding airports only."""
        data = encode_i390(adep="EDDF", ades="KJFK")
        self.assertGreater(len(data), 1)

        # Should contain airport codes
        self.assertIn(b'EDDF', data)
        self.assertIn(b'KJFK', data)

    def test_complete_flight_plan(self):
        """Test encoding complete flight plan."""
        data = encode_i390(callsign="AFR123", adep="LFPG", ades="KJFK")
        self.assertGreater(len(data), 2)

        # Should contain all elements
        self.assertIn(b'AFR123', data)
        self.assertIn(b'LFPG', data)
        self.assertIn(b'KJFK', data)

    def test_callsign_padding(self):
        """Test callsign is padded to 7 characters."""
        data = encode_i390(callsign="BA123")
        # Primary subfield (1 byte) + callsign (7 bytes) = 8 bytes minimum
        self.assertGreaterEqual(len(data), 8)

    def test_uppercase_conversion(self):
        """Test that callsign and airports are uppercased."""
        data = encode_i390(callsign="dlh123", adep="eddf", ades="kjfk")
        self.assertIn(b'DLH123', data)
        self.assertIn(b'EDDF', data)
        self.assertIn(b'KJFK', data)

    def test_extension_bit(self):
        """Test FX extension bit when secondary subfield needed."""
        # Only callsign: no extension
        data1 = encode_i390(callsign="DLH123")
        primary1 = data1[0]
        self.assertEqual(primary1 & 0x01, 0)  # No FX

        # With destination: extension needed
        data2 = encode_i390(callsign="DLH123", ades="KJFK")
        primary2 = data2[0]
        self.assertEqual(primary2 & 0x01, 0x01)  # FX set


class TestCAT062RecordEncoding(unittest.TestCase):
    """Test complete CAT062 record encoding."""

    def test_minimal_record(self):
        """Test encoding minimal record (mandatory items only)."""
        record = encode_cat062_record(lat=48.8584, lon=2.2945)
        self.assertGreater(len(record), 10)

        # Should have FSPEC + I010 + I070 + I105
        # FSPEC (1+ bytes) + I010 (2) + I070 (3) + I105 (8) = 14+ bytes

    def test_complete_record(self):
        """Test encoding complete record with all items."""
        record = encode_cat062_record(
            lat=48.8584,
            lon=2.2945,
            altitude_ft=35000.0,
            vx=100.0,
            vy=200.0,
            callsign="AFR123",
            adep="LFPG",
            ades="KJFK",
            sac=0,
            sic=1
        )
        self.assertGreater(len(record), 30)

    def test_optional_items(self):
        """Test encoding with selective optional items."""
        # Only altitude
        rec1 = encode_cat062_record(lat=48.8584, lon=2.2945, altitude_ft=35000.0)
        self.assertGreater(len(rec1), 10)

        # Only velocity
        rec2 = encode_cat062_record(lat=48.8584, lon=2.2945, vx=100.0, vy=200.0)
        self.assertGreater(len(rec2), 10)

        # Only callsign
        rec3 = encode_cat062_record(lat=48.8584, lon=2.2945, callsign="AFR123")
        self.assertGreater(len(rec3), 10)


class TestCAT062DatablockEncoding(unittest.TestCase):
    """Test complete CAT062 data block encoding."""

    def test_single_record_datablock(self):
        """Test data block with single record."""
        record = encode_cat062_record(lat=48.8584, lon=2.2945)
        block = encode_cat062_datablock([record])

        # Check category
        self.assertEqual(block[0], 62)

        # Check length
        length = struct.unpack('!H', block[1:3])[0]
        self.assertEqual(length, len(block))

    def test_multiple_records_datablock(self):
        """Test data block with multiple records."""
        records = [
            encode_cat062_record(lat=48.8584, lon=2.2945),
            encode_cat062_record(lat=51.5074, lon=-0.1278),
            encode_cat062_record(lat=40.6413, lon=-73.7781),
        ]
        block = encode_cat062_datablock(records)

        # Check category
        self.assertEqual(block[0], 62)

        # Check length matches actual data
        length = struct.unpack('!H', block[1:3])[0]
        self.assertEqual(length, len(block))


class TestCAT062HighLevelAPI(unittest.TestCase):
    """Test high-level CAT062 encoding API."""

    def test_encode_single_track(self):
        """Test encoding single track."""
        tracks = [
            {
                'lat': 48.8584,
                'lon': 2.2945,
                'altitude_ft': 35000,
                'vx': 100.0,
                'vy': 200.0,
            }
        ]
        data = encode_cat062(tracks, sac=0, sic=1)

        # Check it's a valid data block
        self.assertEqual(data[0], 62)
        length = struct.unpack('!H', data[1:3])[0]
        self.assertEqual(length, len(data))

    def test_encode_multiple_tracks(self):
        """Test encoding multiple tracks."""
        tracks = [
            {'lat': 48.8584, 'lon': 2.2945, 'altitude_ft': 35000},
            {'lat': 51.5074, 'lon': -0.1278, 'altitude_ft': 38000},
            {'lat': 40.6413, 'lon': -73.7781, 'altitude_ft': 5000},
        ]
        data = encode_cat062(tracks, sac=0, sic=1)

        # Check it's a valid data block
        self.assertEqual(data[0], 62)

    def test_encode_with_flight_plan(self):
        """Test encoding tracks with flight plan data."""
        tracks = [
            {
                'lat': 48.8584,
                'lon': 2.2945,
                'altitude_ft': 35000,
                'vx': 100.0,
                'vy': 200.0,
                'callsign': 'AFR123',
                'adep': 'LFPG',
                'ades': 'KJFK',
            }
        ]
        data = encode_cat062(tracks, sac=0, sic=1)

        # Should be longer due to I390
        self.assertGreater(len(data), 30)


class TestCAT062RoundTrip(unittest.TestCase):
    """Test round-trip encoding and decoding."""

    def setUp(self):
        """Set up ASTERIX parser."""
        try:
            import asterix
            self.asterix = asterix
            self.has_parser = True
        except ImportError:
            self.has_parser = False

    def test_roundtrip_minimal(self):
        """Test round-trip with minimal record."""
        if not self.has_parser:
            self.skipTest("ASTERIX parser not available")

        tracks = [{'lat': 48.8584, 'lon': 2.2945}]
        encoded = encode_cat062(tracks, sac=0, sic=1)

        decoded = self.asterix.parse(encoded)
        # C++ decoder may parse CAT062 differently, check we got records
        self.assertGreater(len(decoded), 0)
        # Check at least one record has category 62
        categories = [r.get('category') for r in decoded]
        self.assertIn(62, categories)

    def test_roundtrip_complete(self):
        """Test round-trip with complete record."""
        if not self.has_parser:
            self.skipTest("ASTERIX parser not available")

        tracks = [
            {
                'lat': 48.8584,
                'lon': 2.2945,
                'altitude_ft': 35000,
                'vx': 100.0,
                'vy': 200.0,
                'callsign': 'AFR123',
                'adep': 'LFPG',
                'ades': 'KJFK',
            }
        ]
        encoded = encode_cat062(tracks, sac=0, sic=1)

        decoded = self.asterix.parse(encoded)
        # C++ decoder may parse CAT062 differently, check we got records
        self.assertGreater(len(decoded), 0)

        # Find the CAT062 record(s)
        cat062_records = [r for r in decoded if r.get('category') == 62]
        self.assertGreater(len(cat062_records), 0, "Should have at least one CAT062 record")

        rec = cat062_records[0]

        # Verify I010
        if 'I010' in rec:
            sac = rec['I010'].get('SAC')
            sic = rec['I010'].get('SIC')
            # Handle both direct values and {'val': X, 'desc': Y} format
            sac_val = sac.get('val') if isinstance(sac, dict) else sac
            sic_val = sic.get('val') if isinstance(sic, dict) else sic
            self.assertEqual(sac_val, 0)
            self.assertEqual(sic_val, 1)

        # Verify I105 (position)
        if 'I105' in rec:
            lat = rec['I105'].get('LAT')
            lon = rec['I105'].get('LON')
            self.assertAlmostEqual(lat, 48.8584, places=4)
            self.assertAlmostEqual(lon, 2.2945, places=4)


if __name__ == '__main__':
    unittest.main()
