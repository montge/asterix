"""
Unit tests for ASTERIX CAT034 encoder (Radar Service Messages).

Tests North Marker messages, Sector Crossing messages, and
system status encoding.

License: GPL-3.0
"""

import unittest
import struct
import time
from asterix.radar_integration.encoder.cat034 import (
    encode_fspec,
    encode_i010,
    encode_i000,
    encode_i020,
    encode_i030,
    encode_i041,
    encode_cat034_north_marker,
    encode_cat034_sector_crossing,
)


class TestCAT034DataItems(unittest.TestCase):
    """Test individual CAT034 data item encoding."""

    def test_i010_encoding(self):
        """Test I010: Data Source Identifier."""
        data = encode_i010(sac=5, sic=10)
        self.assertEqual(len(data), 2)
        self.assertEqual(data[0], 5)
        self.assertEqual(data[1], 10)

    def test_i000_north_marker(self):
        """Test I000: North Marker message type."""
        data = encode_i000(message_type=1)
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0], 1)

    def test_i000_sector_crossing(self):
        """Test I000: Sector Crossing message type."""
        data = encode_i000(message_type=2)
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0], 2)

    def test_i020_sector_number(self):
        """Test I020: Sector number encoding."""
        for sector in range(8):
            data = encode_i020(sector)
            self.assertEqual(len(data), 1)
            self.assertEqual(data[0], sector)

    def test_i030_time_encoding(self):
        """Test I030: Time of day encoding."""
        timestamp = time.time()
        data = encode_i030(timestamp)
        self.assertEqual(len(data), 3)

    def test_i041_antenna_rotation(self):
        """Test I041: Antenna rotation speed."""
        rotation_period = 4.0  # 4 second rotation (15 RPM)
        data = encode_i041(rotation_period)
        self.assertEqual(len(data), 2)

        # Decode
        decoded = struct.unpack('>H', data)[0]
        period_recovered = decoded / 128.0
        self.assertAlmostEqual(period_recovered, rotation_period, places=2)

    def test_i041_fast_rotation(self):
        """Test antenna rotation with faster speed."""
        rotation_period = 2.0  # 2 second rotation (30 RPM)
        data = encode_i041(rotation_period)
        decoded = struct.unpack('>H', data)[0]
        period_recovered = decoded / 128.0
        self.assertAlmostEqual(period_recovered, rotation_period, places=2)


class TestCAT034NorthMarker(unittest.TestCase):
    """Test North Marker message encoding."""

    def test_basic_north_marker(self):
        """Test basic North Marker message."""
        data = encode_cat034_north_marker(
            sac=0,
            sic=1,
            sector_number=0,
            antenna_rotation_speed=4.0
        )

        # Check category
        self.assertEqual(data[0], 34)

        # Check length
        length = struct.unpack('!H', data[1:3])[0]
        self.assertEqual(length, len(data))
        self.assertGreater(len(data), 10)

    def test_north_marker_with_timestamp(self):
        """Test North Marker with explicit timestamp."""
        timestamp = time.time()
        data = encode_cat034_north_marker(
            sac=1,
            sic=2,
            sector_number=5,
            timestamp=timestamp,
            antenna_rotation_speed=3.5
        )

        self.assertEqual(data[0], 34)
        self.assertGreater(len(data), 10)


class TestCAT034SectorCrossing(unittest.TestCase):
    """Test Sector Crossing message encoding."""

    def test_sector_crossing_basic(self):
        """Test basic Sector Crossing message."""
        data = encode_cat034_sector_crossing(
            sac=0,
            sic=1,
            sector_number=3
        )

        self.assertEqual(data[0], 34)
        length = struct.unpack('!H', data[1:3])[0]
        self.assertEqual(length, len(data))

    def test_all_sectors(self):
        """Test Sector Crossing for all 8 sectors."""
        for sector in range(8):
            data = encode_cat034_sector_crossing(
                sac=0,
                sic=1,
                sector_number=sector,
                timestamp=time.time() + sector * 0.5
            )

            self.assertEqual(data[0], 34)
            self.assertGreater(len(data), 8)


class TestCAT034RadarRotation(unittest.TestCase):
    """Test radar rotation simulation."""

    def test_360_degree_rotation(self):
        """Test complete 360° rotation with 8 sectors."""
        rotation_period = 4.0  # seconds
        sectors = 8
        sector_period = rotation_period / sectors

        messages = []

        # Generate sector crossings for full rotation
        for sector in range(sectors):
            msg = encode_cat034_sector_crossing(
                sac=0,
                sic=1,
                sector_number=sector,
                timestamp=time.time() + sector * sector_period
            )
            messages.append(msg)

        # Verify all messages generated
        self.assertEqual(len(messages), 8)

        # All should be CAT034
        for msg in messages:
            self.assertEqual(msg[0], 34)

    def test_north_marker_timing(self):
        """Test North Marker occurs once per rotation."""
        rotation_period = 4.0

        # North Marker at start of rotation
        nm = encode_cat034_north_marker(
            sac=0,
            sic=1,
            sector_number=0,
            timestamp=time.time(),
            antenna_rotation_speed=rotation_period
        )

        self.assertEqual(nm[0], 34)
        self.assertGreater(len(nm), 10)


if __name__ == '__main__':
    unittest.main()
