"""
Unit tests for ASTERIX CAT019 (MLAT System Status) encoder.

Tests multilateration system status messages, health monitoring,
and service status encoding.

License: GPL-3.0
"""

import unittest
import struct
import time
from asterix.radar_integration.encoder.cat019 import (
    encode_fspec,
    encode_i010,
    encode_i000,
    encode_i140,
    encode_i550,
    encode_i600,
    encode_i610,
    encode_cat019_status,
    encode_cat019_update_cycle,
)


class TestCAT019DataItems(unittest.TestCase):
    """Test individual CAT019 data item encoding."""

    def test_i010_encoding(self):
        """Test I010: Data Source Identifier."""
        data = encode_i010(sac=5, sic=10)
        self.assertEqual(len(data), 2)
        self.assertEqual(data[0], 5)
        self.assertEqual(data[1], 10)

    def test_i000_start_of_update(self):
        """Test I000: Start of Update message type."""
        data = encode_i000(message_type=1)
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0], 1)

    def test_i000_periodic_status(self):
        """Test I000: Periodic Status message type."""
        data = encode_i000(message_type=2)
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0], 2)

    def test_i000_event_triggered(self):
        """Test I000: Event-Triggered message type."""
        data = encode_i000(message_type=3)
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0], 3)

    def test_i140_time_encoding(self):
        """Test I140: Time of day encoding."""
        timestamp = time.time()
        data = encode_i140(timestamp)
        self.assertEqual(len(data), 3)

    def test_i550_system_operational(self):
        """Test I550: System operational status."""
        data = encode_i550(operational=True, num_sensors=4)
        self.assertGreaterEqual(len(data), 2)

    def test_i550_system_not_operational(self):
        """Test I550: System not operational status."""
        data = encode_i550(operational=False, num_sensors=0)
        self.assertGreaterEqual(len(data), 1)
        # NOGO bit should be set
        self.assertTrue(data[0] & 0b10000000)

    def test_i600_mlat_position(self):
        """Test I600: MLAT reference position."""
        lat, lon = 52.5597, 13.2877  # Berlin Airport
        data = encode_i600(lat, lon)
        self.assertEqual(len(data), 8)

        # Decode and verify
        lat_enc, lon_enc = struct.unpack('>ii', data)
        scale = 2**25 / 180.0
        lat_recovered = lat_enc / scale
        lon_recovered = lon_enc / scale

        self.assertAlmostEqual(lat_recovered, lat, places=5)
        self.assertAlmostEqual(lon_recovered, lon, places=5)

    def test_i610_mlat_height(self):
        """Test I610: MLAT reference height."""
        height_m = 100.0
        data = encode_i610(height_m)
        self.assertEqual(len(data), 2)

        # Decode
        decoded = struct.unpack('>h', data)[0]
        self.assertEqual(decoded, int(height_m))

    def test_i610_negative_height(self):
        """Test I610: Negative height (below WGS-84 ellipsoid)."""
        height_m = -50.0
        data = encode_i610(height_m)
        decoded = struct.unpack('>h', data)[0]
        self.assertEqual(decoded, int(height_m))


class TestCAT019StatusMessages(unittest.TestCase):
    """Test complete CAT019 status message encoding."""

    def test_basic_status_message(self):
        """Test basic status message."""
        msg = encode_cat019_status(
            sac=0,
            sic=1,
            message_type=2,  # Periodic
            operational=True,
            num_sensors=4
        )

        # Check category
        self.assertEqual(msg[0], 19)

        # Check length
        length = struct.unpack('!H', msg[1:3])[0]
        self.assertEqual(length, len(msg))
        self.assertGreater(len(msg), 10)

    def test_status_with_mlat_position(self):
        """Test status message with MLAT reference position."""
        msg = encode_cat019_status(
            sac=0,
            sic=1,
            mlat_lat=52.5597,
            mlat_lon=13.2877,
            mlat_height_m=100.0,
            operational=True,
            num_sensors=4
        )

        # Should be longer with position data
        self.assertGreater(len(msg), 20)

    def test_update_cycle_message(self):
        """Test start of update cycle message."""
        msg = encode_cat019_update_cycle(sac=0, sic=1)

        self.assertEqual(msg[0], 19)
        self.assertGreater(len(msg), 8)

    def test_degraded_operation(self):
        """Test degraded operational status."""
        msg = encode_cat019_status(
            sac=0,
            sic=1,
            operational=True,  # Still operational
            num_sensors=2  # But fewer sensors
        )

        self.assertEqual(msg[0], 19)

    def test_system_failure(self):
        """Test system not operational."""
        msg = encode_cat019_status(
            sac=0,
            sic=1,
            operational=False,
            num_sensors=0
        )

        self.assertEqual(msg[0], 19)


class TestCAT019MLATMonitoring(unittest.TestCase):
    """Test MLAT system health monitoring scenarios."""

    def test_continuous_status_updates(self):
        """Test continuous status message generation."""
        messages = []

        for i in range(10):
            msg = encode_cat019_status(
                sac=0,
                sic=1,
                timestamp=time.time() + i,
                operational=True,
                num_sensors=4
            )
            messages.append(msg)

        # All should be CAT019
        for msg in messages:
            self.assertEqual(msg[0], 19)

        # Should have 10 messages
        self.assertEqual(len(messages), 10)

    def test_sensor_degradation_scenario(self):
        """Test scenario where sensors fail progressively."""
        # Start with 4 sensors
        msg1 = encode_cat019_status(operational=True, num_sensors=4)
        self.assertEqual(msg1[0], 19)

        # 1 sensor fails
        msg2 = encode_cat019_status(operational=True, num_sensors=3)
        self.assertEqual(msg2[0], 19)

        # Critical failure - system down
        msg3 = encode_cat019_status(operational=False, num_sensors=0)
        self.assertEqual(msg3[0], 19)


class TestCAT019Integration(unittest.TestCase):
    """Test CAT019 integration with CAT020."""

    def test_mlat_system_initialization(self):
        """Test MLAT system initialization sequence."""
        # Start of update
        init_msg = encode_cat019_update_cycle(sac=0, sic=1)

        # Status message with position
        status_msg = encode_cat019_status(
            sac=0, sic=1,
            mlat_lat=52.5597,
            mlat_lon=13.2877,
            mlat_height_m=100.0,
            operational=True,
            num_sensors=4
        )

        # Both should be CAT019
        self.assertEqual(init_msg[0], 19)
        self.assertEqual(status_msg[0], 19)

        # Status should be longer (has position)
        self.assertGreater(len(status_msg), len(init_msg))


if __name__ == '__main__':
    unittest.main()
