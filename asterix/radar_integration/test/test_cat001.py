"""
Unit tests for ASTERIX CAT001 encoder.

Tests individual data item encoding, polar to Cartesian conversion,
complete record encoding, and round-trip validation.

License: GPL-3.0
"""

import unittest
import struct
import time
import math
from asterix.radar_integration.encoder.cat001 import (
    encode_i010,
    encode_i020,
    encode_i040,
    encode_i042,
    encode_i070,
    encode_i090,
    encode_i141,
    polar_to_cartesian,
    encode_cat001_record,
    encode_cat001_datablock,
    encode_cat001,
)


class TestCAT001DataItems(unittest.TestCase):
    """Test individual CAT001 data item encoding."""

    def test_i010_encoding(self):
        """Test I010: Data Source Identifier."""
        data = encode_i010(sac=5, sic=10)
        self.assertEqual(len(data), 2)
        self.assertEqual(data[0], 5)
        self.assertEqual(data[1], 10)

    def test_i020_psr(self):
        """Test I020: PSR report type."""
        data = encode_i020(typ="PSR")
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0] & 0b11000000, 0b01000000)

    def test_i020_ssr(self):
        """Test I020: SSR report type."""
        data = encode_i020(typ="SSR")
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0] & 0b11000000, 0b10000000)

    def test_i020_combined(self):
        """Test I020: Combined report type."""
        data = encode_i020(typ="CMB")
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0] & 0b11000000, 0b11000000)

    def test_i040_polar_coordinates(self):
        """Test I040: Polar coordinates encoding."""
        range_m = 50000  # 50 km
        azimuth = 45.0
        data = encode_i040(range_m, azimuth)
        self.assertEqual(len(data), 4)

        # Decode and verify
        rho, theta = struct.unpack('>HH', data)
        range_recovered = (rho / 128.0) * 1852.0
        azimuth_recovered = theta * 360.0 / 65536.0

        self.assertAlmostEqual(range_recovered, range_m, delta=100)
        self.assertAlmostEqual(azimuth_recovered, azimuth, delta=1.0)

    def test_i042_cartesian_coordinates(self):
        """Test I042: Cartesian coordinates encoding."""
        x_m = 35355  # ~50km at 45°
        y_m = 35355
        data = encode_i042(x_m, y_m)
        self.assertEqual(len(data), 4)

        # Decode
        x_enc, y_enc = struct.unpack('>hh', data)
        x_recovered = (x_enc / 64.0) * 1852.0
        y_recovered = (y_enc / 64.0) * 1852.0

        # Resolution is ~28.8m, allow error
        self.assertAlmostEqual(x_recovered, x_m, delta=200)
        self.assertAlmostEqual(y_recovered, y_m, delta=200)

    def test_i070_mode3a_code(self):
        """Test I070: Mode 3/A code encoding."""
        code = 0o1234  # Octal
        data = encode_i070(code)
        self.assertEqual(len(data), 2)

        decoded = struct.unpack('>H', data)[0] & 0x0FFF
        self.assertEqual(decoded, code)

    def test_i090_flight_level(self):
        """Test I090: Flight level encoding."""
        fl = 350  # FL350
        data = encode_i090(flight_level=fl)
        self.assertEqual(len(data), 2)

        # Decode
        decoded = struct.unpack('>H', data)[0]
        fl_recovered = decoded / 4.0
        self.assertAlmostEqual(fl_recovered, fl, places=1)

    def test_i141_truncated_time(self):
        """Test I141: Truncated time encoding."""
        timestamp = time.time()
        data = encode_i141(timestamp)
        self.assertEqual(len(data), 2)


class TestPolarToCartesian(unittest.TestCase):
    """Test polar to Cartesian coordinate conversion."""

    def test_conversion_45_degrees(self):
        """Test conversion at 45 degrees."""
        range_m = 50000
        azimuth = 45.0
        x, y = polar_to_cartesian(range_m, azimuth)

        # At 45°, x and y should be equal
        self.assertAlmostEqual(x, y, delta=1.0)

        # Check range
        recovered_range = math.sqrt(x**2 + y**2)
        self.assertAlmostEqual(recovered_range, range_m, delta=1.0)

    def test_conversion_cardinal_directions(self):
        """Test conversion at cardinal directions."""
        range_m = 100000

        # North (0°): x=0, y=range
        x, y = polar_to_cartesian(range_m, 0)
        self.assertAlmostEqual(x, 0, delta=1.0)
        self.assertAlmostEqual(y, range_m, delta=1.0)

        # East (90°): x=range, y=0
        x, y = polar_to_cartesian(range_m, 90)
        self.assertAlmostEqual(x, range_m, delta=1.0)
        self.assertAlmostEqual(y, 0, delta=1.0)

        # South (180°): x=0, y=-range
        x, y = polar_to_cartesian(range_m, 180)
        self.assertAlmostEqual(x, 0, delta=1.0)
        self.assertAlmostEqual(y, -range_m, delta=1.0)

        # West (270°): x=-range, y=0
        x, y = polar_to_cartesian(range_m, 270)
        self.assertAlmostEqual(x, -range_m, delta=1.0)
        self.assertAlmostEqual(y, 0, delta=1.0)


class TestCAT001RecordEncoding(unittest.TestCase):
    """Test complete CAT001 record encoding."""

    def test_minimal_record(self):
        """Test minimal CAT001 record (mandatory items only)."""
        record = encode_cat001_record(
            range_m=50000,
            azimuth_deg=45.0,
            sac=0,
            sic=1,
            include_cartesian=False
        )
        self.assertGreater(len(record), 10)

    def test_complete_record(self):
        """Test complete CAT001 record with all items."""
        record = encode_cat001_record(
            range_m=50000,
            azimuth_deg=45.0,
            timestamp=time.time(),
            sac=0,
            sic=1,
            mode3a=0o1234,
            flight_level=350,
            typ="SSR",
            include_cartesian=True
        )
        self.assertGreater(len(record), 20)

    def test_record_with_mode3a(self):
        """Test record with Mode 3/A code."""
        record = encode_cat001_record(
            range_m=75000,
            azimuth_deg=180.0,
            mode3a=0o7777,
            sac=0,
            sic=1
        )
        self.assertGreater(len(record), 15)


class TestCAT001DatablockEncoding(unittest.TestCase):
    """Test CAT001 data block encoding."""

    def test_single_plot_datablock(self):
        """Test data block with single plot."""
        record = encode_cat001_record(range_m=50000, azimuth_deg=45.0)
        block = encode_cat001_datablock([record])

        # Check header
        self.assertEqual(block[0], 1)  # Category
        length = struct.unpack('!H', block[1:3])[0]
        self.assertEqual(length, len(block))

    def test_multiple_plots_datablock(self):
        """Test data block with multiple plots."""
        records = []
        for i in range(5):
            rec = encode_cat001_record(
                range_m=30000 + i * 10000,
                azimuth_deg=i * 45.0,
                sac=0,
                sic=1
            )
            records.append(rec)

        block = encode_cat001_datablock(records)
        self.assertEqual(block[0], 1)
        length = struct.unpack('!H', block[1:3])[0]
        self.assertEqual(length, len(block))


class TestCAT001HighLevelAPI(unittest.TestCase):
    """Test CAT001 high-level encoding API."""

    def test_encode_with_radar_plots(self):
        """Test encoding with RadarPlot objects."""
        from asterix.radar_integration import MockRadar

        radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        plots = radar.generate_plots(num_targets=5)

        data = encode_cat001(plots, sac=0, sic=1)

        # Verify structure
        self.assertEqual(data[0], 1)
        self.assertGreater(len(data), 50)

    def test_encode_with_dicts(self):
        """Test encoding with dict objects."""
        plots = [
            {'range': 50000, 'azimuth': 45.0, 'timestamp': time.time()},
            {'range': 75000, 'azimuth': 180.0, 'timestamp': time.time()},
        ]

        data = encode_cat001(plots, sac=0, sic=1)
        self.assertEqual(data[0], 1)

    def test_encode_empty(self):
        """Test encoding empty plot list."""
        data = encode_cat001([], sac=0, sic=1)
        self.assertEqual(data[0], 1)
        self.assertEqual(len(data), 3)  # Just header


if __name__ == '__main__':
    unittest.main()
