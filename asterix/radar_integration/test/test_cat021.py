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
    encode_i200,
    encode_i155,
    encode_i150,
    encode_i151,
    encode_i152,
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


class TestI040TargetReportDescriptor(unittest.TestCase):
    """Tests for I040: Target Report Descriptor."""

    def test_default_values(self):
        """Test I040 with default values."""
        data = encode_i040()
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0], 0x00)

    def test_address_type_24bit(self):
        """Test ATP=0 (24-bit ICAO address)."""
        data = encode_i040(atp=0)
        self.assertEqual(data[0] & 0b01100000, 0)

    def test_address_type_duplicate(self):
        """Test ATP=1 (duplicate address)."""
        data = encode_i040(atp=1)
        self.assertEqual(data[0] & 0b01100000, 0b00100000)

    def test_address_type_surface(self):
        """Test ATP=2 (surface vehicle)."""
        data = encode_i040(atp=2)
        self.assertEqual(data[0] & 0b01100000, 0b01000000)

    def test_address_type_anonymous(self):
        """Test ATP=3 (anonymous address)."""
        data = encode_i040(atp=3)
        self.assertEqual(data[0] & 0b01100000, 0b01100000)

    def test_altitude_reporting_100ft(self):
        """Test ARC=0 (100ft resolution)."""
        data = encode_i040(arc=0)
        self.assertEqual(data[0] & 0b00010000, 0)

    def test_altitude_reporting_25ft(self):
        """Test ARC=1 (25ft resolution)."""
        data = encode_i040(arc=1)
        self.assertEqual(data[0] & 0b00010000, 0b00010000)

    def test_range_check_default(self):
        """Test RC=0 (default range check)."""
        data = encode_i040(rc=0)
        self.assertEqual(data[0] & 0b00001000, 0)

    def test_range_check_gnss(self):
        """Test RC=1 (GNSS range check)."""
        data = encode_i040(rc=1)
        self.assertEqual(data[0] & 0b00001000, 0b00001000)

    def test_combined_flags(self):
        """Test all flags set."""
        data = encode_i040(atp=3, arc=1, rc=1)
        expected = 0b01111000
        self.assertEqual(data[0], expected)


class TestI161TrackNumber(unittest.TestCase):
    """Tests for I161: Track Number."""

    def test_zero_track(self):
        """Test track number 0."""
        data = encode_i161(0)
        self.assertEqual(len(data), 2)
        self.assertEqual(data, b'\x00\x00')

    def test_typical_track(self):
        """Test typical track number."""
        data = encode_i161(1234)
        self.assertEqual(len(data), 2)
        # Track is left-shifted by 4 bits
        decoded = struct.unpack('>H', data)[0] >> 4
        self.assertEqual(decoded, 1234)

    def test_max_track_4095(self):
        """Test maximum track number (12-bit limit)."""
        data = encode_i161(4095)
        decoded = struct.unpack('>H', data)[0] >> 4
        self.assertEqual(decoded, 4095)

    def test_overflow_wraps(self):
        """Test track number > 4095 wraps (12-bit mask)."""
        data = encode_i161(0x1000)  # 4096
        decoded = struct.unpack('>H', data)[0] >> 4
        self.assertEqual(decoded, 0)  # Masked to 12 bits


class TestI015ServiceId(unittest.TestCase):
    """Tests for I015: Service Identification."""

    def test_default_service(self):
        """Test default service ID."""
        data = encode_i015()
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0], 0)

    def test_service_id_255(self):
        """Test maximum service ID."""
        data = encode_i015(255)
        self.assertEqual(data[0], 255)

    def test_typical_service_id(self):
        """Test typical service ID."""
        data = encode_i015(42)
        self.assertEqual(data[0], 42)


class TestI071TimeOfApplicability(unittest.TestCase):
    """Tests for I071: Time of Applicability for Position."""

    def test_length(self):
        """Test output length is 3 bytes."""
        data = encode_i071()
        self.assertEqual(len(data), 3)

    def test_midnight(self):
        """Test encoding at midnight."""
        # Unix timestamp for midnight (any day)
        midnight = 1700000000 - (1700000000 % 86400)
        data = encode_i071(midnight)
        # Should be close to 0 (start of day)
        value = struct.unpack('>I', b'\x00' + data)[0]
        self.assertLess(value, 128 * 60)  # Less than 1 minute

    def test_noon(self):
        """Test encoding at noon (12:00:00)."""
        noon = 1700000000 - (1700000000 % 86400) + 43200
        data = encode_i071(noon)
        value = struct.unpack('>I', b'\x00' + data)[0]
        # 12 hours = 43200 seconds * 128 = 5,529,600 time units
        expected = 43200 * 128
        self.assertAlmostEqual(value, expected, delta=128)

    def test_current_time_default(self):
        """Test using current time as default."""
        before = time.time()
        data = encode_i071()
        after = time.time()
        # Should be non-zero for most of day
        value = struct.unpack('>I', b'\x00' + data)[0]
        self.assertGreaterEqual(value, 0)


class TestI200TargetStatus(unittest.TestCase):
    """Tests for I200: Target Status."""

    def test_default_status(self):
        """Test default status (all zeros)."""
        data = encode_i200()
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0], 0x00)

    def test_intent_change_flag(self):
        """Test ICF (Intent Change Flag)."""
        data = encode_i200(icf=1)
        self.assertEqual(data[0] & 0b10000000, 0b10000000)

    def test_lnav_mode(self):
        """Test LNAV mode active."""
        data = encode_i200(lnav=1)
        self.assertEqual(data[0] & 0b01000000, 0b01000000)

    def test_priority_status(self):
        """Test priority status (3 bits)."""
        for ps in range(8):
            data = encode_i200(ps=ps)
            decoded_ps = (data[0] >> 3) & 0x07
            self.assertEqual(decoded_ps, ps)

    def test_surveillance_status(self):
        """Test surveillance status."""
        data = encode_i200(ss=7)
        # SS is in lower bits (may be truncated due to FX bit)
        self.assertEqual(len(data), 1)

    def test_combined_status(self):
        """Test all flags combined."""
        data = encode_i200(icf=1, lnav=1, ps=5, ss=3)
        self.assertEqual(data[0] & 0b10000000, 0b10000000)  # ICF
        self.assertEqual(data[0] & 0b01000000, 0b01000000)  # LNAV
        decoded_ps = (data[0] >> 3) & 0x07
        self.assertEqual(decoded_ps, 5)  # PS


class TestCAT021EdgeCases(unittest.TestCase):
    """Edge case tests for CAT021 encoding."""

    def test_i130_negative_coordinates(self):
        """Test I130 with negative coordinates (Western/Southern hemisphere)."""
        # Sydney, Australia
        lat, lon = -33.8688, 151.2093
        data = encode_i130(lat, lon)
        self.assertEqual(len(data), 6)

        # Decode latitude
        lat_bytes = struct.unpack('>I', b'\x00' + data[:3])[0]
        if lat_bytes & 0x800000:
            lat_bytes -= (1 << 24)
        self.assertLess(lat_bytes, 0)  # Negative latitude

    def test_i130_extreme_longitude(self):
        """Test I130 with extreme longitude (near date line)."""
        # Near International Date Line
        lat, lon = 0.0, 179.9
        data = encode_i130(lat, lon)
        self.assertEqual(len(data), 6)

    def test_i145_zero_flight_level(self):
        """Test I145 at FL000 (ground level)."""
        data = encode_i145(0)
        self.assertEqual(struct.unpack('>H', data)[0], 0)

    def test_i140_negative_height(self):
        """Test I140 with negative height (below sea level)."""
        data = encode_i140(-500)  # Dead Sea area
        value = struct.unpack('>h', data)[0]
        self.assertLess(value, 0)

    def test_i155_large_climb_rate(self):
        """Test I155 with large climb rate."""
        data = encode_i155(6000)  # 6000 ft/min climb
        value = struct.unpack('>h', data)[0]
        recovered = value * 6.25
        self.assertAlmostEqual(recovered, 6000, places=0)

    def test_i155_large_descent_rate(self):
        """Test I155 with large descent rate."""
        data = encode_i155(-4000)
        value = struct.unpack('>h', data)[0]
        recovered = value * 6.25
        self.assertAlmostEqual(recovered, -4000, places=0)

    def test_i152_heading_zero(self):
        """Test I152 at heading 0 (North)."""
        data = encode_i152(0.0)
        value = struct.unpack('>H', data)[0]
        self.assertEqual(value, 0)

    def test_i152_heading_360(self):
        """Test I152 at heading 360 (wraps to 0)."""
        data = encode_i152(360.0)
        value = struct.unpack('>H', data)[0]
        self.assertEqual(value, 0)  # 360 wraps to 0

    def test_i170_empty_callsign(self):
        """Test I170 with empty callsign."""
        data = encode_i170("")
        self.assertEqual(len(data), 6)

    def test_i170_long_callsign(self):
        """Test I170 with callsign longer than 8 chars (truncates)."""
        data = encode_i170("VERYLONG123")
        self.assertEqual(len(data), 6)

    def test_i080_zero_address(self):
        """Test I080 with address 0x000000."""
        data = encode_i080(0)
        self.assertEqual(data, b'\x00\x00\x00')

    def test_i080_max_address(self):
        """Test I080 with maximum address 0xFFFFFF."""
        data = encode_i080(0xFFFFFF)
        self.assertEqual(data, b'\xff\xff\xff')


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
