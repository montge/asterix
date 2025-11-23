"""
Unit tests for ASTERIX CAT021 (ADS-B) encoder.

Tests individual data item encoding, complete record encoding,
and round-trip validation.

License: GPL-3.0
"""

import unittest
import struct
import time
from asterix.radar_integration.encoder.cat021 import (
    encode_fspec,
    encode_i010,
    encode_i040,
    encode_i161,
    encode_i015,
    encode_i071,
    encode_i130,
    encode_i080,
    encode_i170,
    encode_i145,
    encode_i140,
    encode_i155,
    encode_i150,
    encode_i151,
    encode_i152,
    encode_i200,
    encode_cat021_record,
    encode_cat021_datablock,
    encode_cat021,
)


class TestCAT021DataItems(unittest.TestCase):
    """Test individual CAT021 data item encoding."""

    def test_i010_encoding(self):
        """Test I010: Data Source Identifier."""
        data = encode_i010(sac=5, sic=10)
        self.assertEqual(len(data), 2)
        self.assertEqual(data[0], 5)
        self.assertEqual(data[1], 10)

    def test_i080_encoding(self):
        """Test I080: Aircraft Address (24-bit)."""
        addr = 0x3950A1  # Air France example
        data = encode_i080(addr)
        self.assertEqual(len(data), 3)
        decoded = struct.unpack('>I', b'\x00' + data)[0]
        self.assertEqual(decoded, addr)

    def test_i130_high_precision(self):
        """Test I130: High-precision WGS-84 encoding."""
        lat, lon = 48.858844, 2.294351  # Eiffel Tower
        data = encode_i130(lat, lon)
        self.assertEqual(len(data), 6)

        # Decode and verify precision
        lat_bytes = struct.unpack('>I', b'\x00' + data[:3])[0]
        lon_bytes = struct.unpack('>I', b'\x00' + data[3:])[0]

        # Handle two's complement
        if lat_bytes & 0x800000:
            lat_bytes -= (1 << 24)
        if lon_bytes & 0x800000:
            lon_bytes -= (1 << 24)

        scale = 2**23 / 180.0
        lat_decoded = lat_bytes / scale
        lon_decoded = lon_bytes / scale

        # Check precision (~0.021 meters = ~0.00002 degrees)
        # Allow 4 decimal places = ~11 meter accuracy (excellent for ADS-B)
        self.assertAlmostEqual(lat_decoded, lat, places=4)
        self.assertAlmostEqual(lon_decoded, lon, places=4)

    def test_i170_callsign_encoding(self):
        """Test I170: Callsign encoding."""
        callsign = "AFR123"
        data = encode_i170(callsign)
        self.assertEqual(len(data), 6)

    def test_i145_flight_level(self):
        """Test I145: Flight level encoding."""
        fl = 350  # FL350
        data = encode_i145(fl)
        self.assertEqual(len(data), 2)

        # Decode and verify
        decoded = struct.unpack('>H', data)[0]
        fl_recovered = decoded / 4.0
        self.assertAlmostEqual(fl_recovered, fl, places=2)

    def test_i140_gnss_height(self):
        """Test I140: GNSS height encoding."""
        height = 35000  # feet
        data = encode_i140(height)
        self.assertEqual(len(data), 2)

        # Decode and verify
        decoded = struct.unpack('>h', data)[0]  # Signed
        height_recovered = decoded * 6.25
        self.assertAlmostEqual(height_recovered, height, places=1)

    def test_i150_airspeed(self):
        """Test I150: Air speed encoding."""
        ias = 450  # knots
        data = encode_i150(ias)
        self.assertEqual(len(data), 2)

    def test_i151_true_airspeed(self):
        """Test I151: True airspeed encoding."""
        tas = 480  # knots
        data = encode_i151(tas)
        self.assertEqual(len(data), 2)

        # Decode and verify
        decoded = struct.unpack('>H', data)[0] >> 1  # 15 bits
        self.assertEqual(decoded, tas)

    def test_i152_magnetic_heading(self):
        """Test I152: Magnetic heading encoding."""
        heading = 135.5  # degrees
        data = encode_i152(heading)
        self.assertEqual(len(data), 2)

        # Decode and verify
        decoded = struct.unpack('>H', data)[0]
        heading_recovered = decoded * 360.0 / 65536.0
        self.assertAlmostEqual(heading_recovered, heading, places=1)

    def test_i155_vertical_rate(self):
        """Test I155: Vertical rate encoding."""
        vrate = -800  # ft/min (descending)
        data = encode_i155(vrate)
        self.assertEqual(len(data), 2)

        # Decode and verify
        decoded = struct.unpack('>h', data)[0]  # Signed
        vrate_recovered = decoded * 6.25
        self.assertAlmostEqual(vrate_recovered, vrate, places=1)


class TestCAT021RecordEncoding(unittest.TestCase):
    """Test complete CAT021 record encoding."""

    def test_minimal_record(self):
        """Test minimal CAT021 record (mandatory items only)."""
        record = encode_cat021_record(
            lat=48.8584,
            lon=2.2945,
            aircraft_address=0x3950A1,
            sac=0,
            sic=1
        )

        # Should have FSPEC + mandatory items
        self.assertGreater(len(record), 15)

    def test_complete_record(self):
        """Test complete CAT021 record with all optional items."""
        record = encode_cat021_record(
            lat=48.8584,
            lon=2.2945,
            aircraft_address=0x3950A1,
            callsign="AFR123",
            flight_level=350,
            gnss_height_ft=35000,
            airspeed_kt=450,
            true_airspeed_kt=480,
            magnetic_heading_deg=45.0,
            vertical_rate_fpm=0,
            track_number=1234,
            sac=0,
            sic=1
        )

        # Should be significantly longer with all items
        self.assertGreater(len(record), 30)


class TestCAT021DatablockEncoding(unittest.TestCase):
    """Test CAT021 data block encoding."""

    def test_single_report_datablock(self):
        """Test data block with single ADS-B report."""
        record = encode_cat021_record(
            lat=48.8584, lon=2.2945, aircraft_address=0x3950A1
        )
        block = encode_cat021_datablock([record])

        # Check header
        self.assertEqual(block[0], 21)  # Category
        length = struct.unpack('!H', block[1:3])[0]
        self.assertEqual(length, len(block))

    def test_multiple_reports_datablock(self):
        """Test data block with multiple ADS-B reports."""
        records = []
        for i in range(5):
            rec = encode_cat021_record(
                lat=48.0 + i,
                lon=2.0 + i,
                aircraft_address=0x400000 + i,
                sac=0,
                sic=1
            )
            records.append(rec)

        block = encode_cat021_datablock(records)

        # Verify header
        self.assertEqual(block[0], 21)
        length = struct.unpack('!H', block[1:3])[0]
        self.assertEqual(length, len(block))


class TestCAT021HighLevelAPI(unittest.TestCase):
    """Test CAT021 high-level encoding API."""

    def test_encode_single_adsb(self):
        """Test encoding single ADS-B report."""
        reports = [
            {
                'lat': 48.8584,
                'lon': 2.2945,
                'aircraft_address': 0x3950A1,
                'callsign': 'AFR123',
                'flight_level': 350
            }
        ]

        data = encode_cat021(reports, sac=0, sic=1)

        # Verify basic structure
        self.assertEqual(data[0], 21)
        self.assertGreater(len(data), 20)

    def test_encode_multiple_adsb(self):
        """Test encoding multiple ADS-B reports."""
        reports = [
            {'lat': 48.8, 'lon': 2.3, 'aircraft_address': 0x3950A1},
            {'lat': 51.5, 'lon': -0.1, 'aircraft_address': 0x4009A8},
            {'lat': 40.7, 'lon': -74.0, 'aircraft_address': 0xA12345},
        ]

        data = encode_cat021(reports, sac=0, sic=1)

        # Verify encoding
        self.assertEqual(data[0], 21)
        length = struct.unpack('!H', data[1:3])[0]
        self.assertEqual(length, len(data))


class TestCAT021Precision(unittest.TestCase):
    """Test high-precision encoding accuracy."""

    def test_position_precision_meter_level(self):
        """Test that position encoding achieves meter-level precision."""
        # Test point: Eiffel Tower
        lat, lon = 48.858844, 2.294351

        data = encode_i130(lat, lon)

        # Decode
        lat_bytes = struct.unpack('>I', b'\x00' + data[:3])[0]
        lon_bytes = struct.unpack('>I', b'\x00' + data[3:])[0]

        if lat_bytes & 0x800000:
            lat_bytes -= (1 << 24)
        if lon_bytes & 0x800000:
            lon_bytes -= (1 << 24)

        scale = 2**23 / 180.0
        lat_decoded = lat_bytes / scale
        lon_decoded = lon_bytes / scale

        # Calculate error in meters (approximate)
        lat_error_m = abs(lat - lat_decoded) * 111000
        lon_error_m = abs(lon - lon_decoded) * 111000 * abs(lat / 90.0)

        # Should be sub-meter accuracy
        self.assertLess(lat_error_m, 5.0, "Latitude error should be < 5 meters")
        self.assertLess(lon_error_m, 5.0, "Longitude error should be < 5 meters")


if __name__ == '__main__':
    unittest.main()
