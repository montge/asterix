"""
Unit tests for ASTERIX CAT020 (MLAT) encoder.

Tests multilateration position encoding, surface surveillance,
and TDOA-based target reports.

License: GPL-3.0
"""

import unittest
import struct
import time
from asterix.radar_integration.encoder.cat020 import (
    encode_fspec,
    encode_i010,
    encode_i020,
    encode_i140,
    encode_i041,
    encode_i042,
    encode_i161,
    encode_i202,
    encode_i220,
    encode_i245,
    encode_cat020_record,
    encode_cat020_datablock,
    encode_cat020,
)


class TestCAT020DataItems(unittest.TestCase):
    """Test individual CAT020 data item encoding."""

    def test_i010_encoding(self):
        """Test I010: Data Source Identifier."""
        data = encode_i010(sac=5, sic=10)
        self.assertEqual(len(data), 2)
        self.assertEqual(data[0], 5)
        self.assertEqual(data[1], 10)

    def test_i020_mode_s(self):
        """Test I020: Mode S detection."""
        data = encode_i020(ms=True)
        self.assertEqual(len(data), 1)
        self.assertTrue(data[0] & 0b01000000)

    def test_i041_wgs84_position(self):
        """Test I041: High-precision WGS-84 position."""
        lat, lon = 52.5597, 13.2877  # Berlin Airport
        data = encode_i041(lat, lon)
        self.assertEqual(len(data), 8)

        # Decode and verify
        lat_enc, lon_enc = struct.unpack('>ii', data)
        scale = 2**25 / 180.0
        lat_recovered = lat_enc / scale
        lon_recovered = lon_enc / scale

        self.assertAlmostEqual(lat_recovered, lat, places=5)
        self.assertAlmostEqual(lon_recovered, lon, places=5)

    def test_i042_cartesian_position(self):
        """Test I042: Cartesian position (1 meter resolution)."""
        x_m, y_m = 1200, 800  # Airport surface coordinates
        data = encode_i042(x_m, y_m)
        self.assertEqual(len(data), 6)

        # Decode
        x_bytes = struct.unpack('>I', b'\x00' + data[:3])[0]
        y_bytes = struct.unpack('>I', b'\x00' + data[3:])[0]

        # Handle two's complement
        if x_bytes & 0x800000:
            x_bytes -= (1 << 24)
        if y_bytes & 0x800000:
            y_bytes -= (1 << 24)

        self.assertEqual(x_bytes, x_m)
        self.assertEqual(y_bytes, y_m)

    def test_i161_track_number(self):
        """Test I161: Track number."""
        track = 1234
        data = encode_i161(track)
        self.assertEqual(len(data), 2)

        decoded = struct.unpack('>H', data)[0]
        self.assertEqual(decoded, track)

    def test_i202_velocity(self):
        """Test I202: Track velocity."""
        vx, vy = 5.0, 3.0  # m/s (taxiing)
        data = encode_i202(vx, vy)
        self.assertEqual(len(data), 4)

        # Decode
        vx_enc, vy_enc = struct.unpack('>hh', data)
        vx_recovered = vx_enc * 0.25
        vy_recovered = vy_enc * 0.25

        self.assertAlmostEqual(vx_recovered, vx, places=2)
        self.assertAlmostEqual(vy_recovered, vy, places=2)

    def test_i220_aircraft_address(self):
        """Test I220: Aircraft address."""
        addr = 0x3C6544
        data = encode_i220(addr)
        self.assertEqual(len(data), 3)

        decoded = struct.unpack('>I', b'\x00' + data)[0]
        self.assertEqual(decoded, addr)

    def test_i245_callsign(self):
        """Test I245: Callsign encoding."""
        callsign = "DLH123"
        data = encode_i245(callsign)
        self.assertEqual(len(data), 7)


class TestCAT020RecordEncoding(unittest.TestCase):
    """Test complete CAT020 record encoding."""

    def test_minimal_mlat_record(self):
        """Test minimal MLAT record (position only)."""
        record = encode_cat020_record(
            lat=52.5597,
            lon=13.2877,
            sac=0,
            sic=1
        )
        self.assertGreaterEqual(len(record), 15)

    def test_complete_mlat_record(self):
        """Test complete MLAT record with all items."""
        record = encode_cat020_record(
            lat=52.5597,
            lon=13.2877,
            aircraft_address=0x3C6544,
            callsign="DLH123",
            track_number=5001,
            vx=5.0,
            vy=3.0,
            x_m=1200,
            y_m=800,
            timestamp=time.time(),
            sac=0,
            sic=1
        )
        self.assertGreater(len(record), 30)

    def test_surface_target(self):
        """Test surface surveillance target (low speed)."""
        record = encode_cat020_record(
            lat=52.5597,
            lon=13.2877,
            vx=2.0,  # Slow taxiing
            vy=1.5,
            sac=0,
            sic=1
        )
        self.assertGreater(len(record), 15)


class TestCAT020DatablockEncoding(unittest.TestCase):
    """Test CAT020 data block encoding."""

    def test_single_target_datablock(self):
        """Test data block with single MLAT target."""
        record = encode_cat020_record(lat=52.5, lon=13.3, sac=0, sic=1)
        block = encode_cat020_datablock([record])

        self.assertEqual(block[0], 20)
        length = struct.unpack('!H', block[1:3])[0]
        self.assertEqual(length, len(block))

    def test_multiple_targets_datablock(self):
        """Test data block with multiple surface targets."""
        records = []
        for i in range(3):
            rec = encode_cat020_record(
                lat=52.5 + i * 0.001,
                lon=13.3 + i * 0.001,
                track_number=5000 + i,
                sac=0,
                sic=1
            )
            records.append(rec)

        block = encode_cat020_datablock(records)
        self.assertEqual(block[0], 20)


class TestCAT020HighLevelAPI(unittest.TestCase):
    """Test CAT020 high-level API."""

    def test_encode_mlat_targets(self):
        """Test encoding MLAT surface targets."""
        targets = [
            {
                'lat': 52.5597, 'lon': 13.2877,
                'aircraft_address': 0x3C6544,
                'callsign': 'DLH123',
                'track_number': 5001,
                'vx': 5.0, 'vy': 3.0
            },
            {
                'lat': 52.5601, 'lon': 13.2885,
                'aircraft_address': 0x4009A8,
                'callsign': 'BAW456',
                'track_number': 5002
            }
        ]

        data = encode_cat020(targets, sac=0, sic=1)
        self.assertEqual(data[0], 20)
        self.assertGreater(len(data), 40)

    def test_encode_empty_targets(self):
        """Test encoding empty target list."""
        data = encode_cat020([], sac=0, sic=1)
        self.assertEqual(data[0], 20)
        self.assertEqual(len(data), 3)


if __name__ == '__main__':
    unittest.main()
