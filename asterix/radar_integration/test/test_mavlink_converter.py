"""
Tests for MAVLink to ASTERIX Converter

Tests the MAVLinkToAsterixConverter class and its methods for converting
MAVLink telemetry to ASTERIX surveillance formats.
"""

import unittest
from unittest.mock import Mock, patch, MagicMock
import time

from asterix.radar_integration.mavlink_converter import (
    MAVLinkState,
    MAVLinkToAsterixConverter,
)


class TestMAVLinkState(unittest.TestCase):
    """Tests for MAVLinkState dataclass."""

    def test_default_values(self):
        """Test MAVLinkState has correct default values."""
        state = MAVLinkState()

        self.assertEqual(state.lat, 0.0)
        self.assertEqual(state.lon, 0.0)
        self.assertEqual(state.alt_msl, 0.0)
        self.assertEqual(state.alt_relative, 0.0)
        self.assertEqual(state.vx, 0.0)
        self.assertEqual(state.vy, 0.0)
        self.assertEqual(state.vz, 0.0)
        self.assertEqual(state.heading, 0.0)
        self.assertEqual(state.groundspeed, 0.0)
        self.assertEqual(state.airspeed, 0.0)
        self.assertEqual(state.climb_rate, 0.0)
        self.assertEqual(state.timestamp, 0.0)
        self.assertEqual(state.system_id, 1)
        self.assertFalse(state.armed)
        self.assertEqual(state.mode, "UNKNOWN")

    def test_custom_values(self):
        """Test MAVLinkState with custom values."""
        state = MAVLinkState(
            lat=47.123456,
            lon=-122.654321,
            alt_msl=1000.0,
            alt_relative=500.0,
            vx=10.0,
            vy=5.0,
            vz=-2.0,
            heading=90.0,
            groundspeed=50.0,
            airspeed=55.0,
            climb_rate=3.0,
            timestamp=1234567890.0,
            system_id=2,
            armed=True,
            mode="AUTO",
        )

        self.assertAlmostEqual(state.lat, 47.123456, places=6)
        self.assertAlmostEqual(state.lon, -122.654321, places=6)
        self.assertEqual(state.alt_msl, 1000.0)
        self.assertEqual(state.alt_relative, 500.0)
        self.assertEqual(state.vx, 10.0)
        self.assertEqual(state.vy, 5.0)
        self.assertEqual(state.vz, -2.0)
        self.assertEqual(state.heading, 90.0)
        self.assertEqual(state.groundspeed, 50.0)
        self.assertEqual(state.airspeed, 55.0)
        self.assertEqual(state.climb_rate, 3.0)
        self.assertEqual(state.timestamp, 1234567890.0)
        self.assertEqual(state.system_id, 2)
        self.assertTrue(state.armed)
        self.assertEqual(state.mode, "AUTO")


class TestMAVLinkToAsterixConverter(unittest.TestCase):
    """Tests for MAVLinkToAsterixConverter class."""

    def test_init_default(self):
        """Test converter initialization with defaults."""
        converter = MAVLinkToAsterixConverter()

        self.assertEqual(converter.sac, 0)
        self.assertEqual(converter.sic, 1)
        self.assertIsNotNone(converter.state)
        self.assertIsNone(converter.connection)
        self.assertEqual(converter.track_number, 1000)

    def test_init_custom_sac_sic(self):
        """Test converter initialization with custom SAC/SIC."""
        converter = MAVLinkToAsterixConverter(sac=10, sic=20)

        self.assertEqual(converter.sac, 10)
        self.assertEqual(converter.sic, 20)

    def test_update_from_global_position(self):
        """Test updating state from GLOBAL_POSITION_INT message."""
        converter = MAVLinkToAsterixConverter()

        # Create mock message
        msg = Mock()
        msg.lat = 471234567  # degE7
        msg.lon = -1226543210  # degE7
        msg.alt = 1000000  # mm MSL
        msg.relative_alt = 500000  # mm AGL
        msg.vx = 100  # cm/s
        msg.vy = 50  # cm/s
        msg.vz = -20  # cm/s
        msg.hdg = 9000  # cdeg (90 degrees)

        converter.update_from_global_position(msg)

        self.assertAlmostEqual(converter.state.lat, 47.1234567, places=6)
        self.assertAlmostEqual(converter.state.lon, -122.6543210, places=6)
        self.assertAlmostEqual(converter.state.alt_msl, 1000.0, places=1)
        self.assertAlmostEqual(converter.state.alt_relative, 500.0, places=1)
        self.assertAlmostEqual(converter.state.vx, 1.0, places=2)
        self.assertAlmostEqual(converter.state.vy, 0.5, places=2)
        self.assertAlmostEqual(converter.state.vz, -0.2, places=2)
        self.assertAlmostEqual(converter.state.heading, 90.0, places=1)

    def test_update_from_vfr_hud(self):
        """Test updating state from VFR_HUD message."""
        converter = MAVLinkToAsterixConverter()

        # Create mock message
        msg = Mock()
        msg.groundspeed = 50.0  # m/s
        msg.airspeed = 55.0  # m/s
        msg.climb = 3.0  # m/s

        converter.update_from_vfr_hud(msg)

        self.assertEqual(converter.state.groundspeed, 50.0)
        self.assertEqual(converter.state.airspeed, 55.0)
        self.assertEqual(converter.state.climb_rate, 3.0)

    def test_update_from_heartbeat(self):
        """Test updating state from HEARTBEAT message."""
        converter = MAVLinkToAsterixConverter()

        # Create mock message for armed state
        msg = Mock()
        msg.base_mode = 128  # MAV_MODE_FLAG_SAFETY_ARMED = 128
        msg.custom_mode = 3  # Some custom mode

        converter.update_from_heartbeat(msg)

        self.assertTrue(converter.state.armed)

    def test_update_from_heartbeat_disarmed(self):
        """Test updating state from HEARTBEAT message (disarmed)."""
        converter = MAVLinkToAsterixConverter()

        # Create mock message for disarmed state
        msg = Mock()
        msg.base_mode = 0  # Not armed
        msg.custom_mode = 0

        converter.update_from_heartbeat(msg)

        self.assertFalse(converter.state.armed)

    def test_to_adsb_report(self):
        """Test conversion to ADS-B report format."""
        converter = MAVLinkToAsterixConverter(sac=5, sic=10)

        # Set up state
        converter.state.lat = 47.5
        converter.state.lon = -122.3
        converter.state.alt_msl = 10000.0
        converter.state.vx = 100.0
        converter.state.vy = 50.0
        converter.state.vz = -5.0
        converter.state.heading = 45.0
        converter.state.groundspeed = 250.0
        converter.state.timestamp = time.time()

        report = converter.to_adsb_report(track_number=1234)

        self.assertEqual(report["SAC"], 5)
        self.assertEqual(report["SIC"], 10)
        self.assertEqual(report["track_number"], 1234)
        self.assertAlmostEqual(report["latitude"], 47.5, places=6)
        self.assertAlmostEqual(report["longitude"], -122.3, places=6)
        self.assertAlmostEqual(report["altitude_ft"], 10000.0 * 3.28084, places=0)
        self.assertEqual(report["heading"], 45.0)
        self.assertEqual(report["groundspeed_kts"], 250.0 * 1.94384)

    def test_to_adsb_report_auto_track_number(self):
        """Test ADS-B report uses auto-incremented track number."""
        converter = MAVLinkToAsterixConverter()
        converter.state.lat = 47.5
        converter.state.lon = -122.3
        converter.state.alt_msl = 5000.0
        converter.state.timestamp = time.time()

        initial_track = converter.track_number

        report1 = converter.to_adsb_report()
        report2 = converter.to_adsb_report()

        self.assertEqual(report1["track_number"], initial_track)
        self.assertEqual(report2["track_number"], initial_track + 1)

    def test_to_system_track(self):
        """Test conversion to CAT062 system track format."""
        converter = MAVLinkToAsterixConverter(sac=5, sic=10)

        # Set up state
        converter.state.lat = 47.5
        converter.state.lon = -122.3
        converter.state.alt_msl = 10000.0
        converter.state.vx = 100.0
        converter.state.vy = 50.0
        converter.state.groundspeed = 250.0
        converter.state.heading = 45.0
        converter.state.timestamp = time.time()
        converter.state.system_id = 5
        converter.track_number = 2000

        track = converter.to_system_track()

        self.assertEqual(track["SAC"], 5)
        self.assertEqual(track["SIC"], 10)
        self.assertEqual(track["track_number"], 2000)
        self.assertAlmostEqual(track["latitude"], 47.5, places=6)
        self.assertAlmostEqual(track["longitude"], -122.3, places=6)
        self.assertIn("altitude_ft", track)
        self.assertIn("heading", track)
        self.assertIn("groundspeed_kts", track)

    def test_connect_without_pymavlink(self):
        """Test connect raises ImportError when pymavlink not available."""
        converter = MAVLinkToAsterixConverter()

        # Mock pymavlink as unavailable
        with patch.dict("sys.modules", {"pymavlink": None}):
            with self.assertRaises((ImportError, ModuleNotFoundError)):
                converter.connect("udpin:localhost:14550")

    @patch("asterix.radar_integration.mavlink_converter.mavutil")
    def test_connect_with_mock_mavutil(self, mock_mavutil):
        """Test connect creates connection with mavutil."""
        mock_connection = Mock()
        mock_mavutil.mavlink_connection.return_value = mock_connection

        converter = MAVLinkToAsterixConverter()

        try:
            converter.connect("udpin:localhost:14550")
            mock_mavutil.mavlink_connection.assert_called_once_with(
                "udpin:localhost:14550"
            )
            self.assertEqual(converter.connection, mock_connection)
        except (ImportError, ModuleNotFoundError):
            # Skip if pymavlink not installed
            self.skipTest("pymavlink not installed")

    def test_state_timestamp_update(self):
        """Test that state timestamp is updated correctly."""
        converter = MAVLinkToAsterixConverter()

        initial_timestamp = converter.state.timestamp

        # Simulate position update
        msg = Mock()
        msg.lat = 470000000
        msg.lon = -1220000000
        msg.alt = 1000000
        msg.relative_alt = 500000
        msg.vx = 0
        msg.vy = 0
        msg.vz = 0
        msg.hdg = 0

        converter.update_from_global_position(msg)

        # Timestamp should be updated
        self.assertGreater(converter.state.timestamp, initial_timestamp)

    def test_velocity_conversion(self):
        """Test velocity is correctly converted from cm/s to m/s."""
        converter = MAVLinkToAsterixConverter()

        msg = Mock()
        msg.lat = 470000000
        msg.lon = -1220000000
        msg.alt = 1000000
        msg.relative_alt = 500000
        msg.vx = 1000  # 1000 cm/s = 10 m/s
        msg.vy = 500  # 500 cm/s = 5 m/s
        msg.vz = -200  # -200 cm/s = -2 m/s
        msg.hdg = 0

        converter.update_from_global_position(msg)

        self.assertAlmostEqual(converter.state.vx, 10.0, places=2)
        self.assertAlmostEqual(converter.state.vy, 5.0, places=2)
        self.assertAlmostEqual(converter.state.vz, -2.0, places=2)

    def test_altitude_conversion(self):
        """Test altitude is correctly converted from mm to m."""
        converter = MAVLinkToAsterixConverter()

        msg = Mock()
        msg.lat = 470000000
        msg.lon = -1220000000
        msg.alt = 5000000  # 5000000 mm = 5000 m
        msg.relative_alt = 2500000  # 2500000 mm = 2500 m
        msg.vx = 0
        msg.vy = 0
        msg.vz = 0
        msg.hdg = 0

        converter.update_from_global_position(msg)

        self.assertAlmostEqual(converter.state.alt_msl, 5000.0, places=1)
        self.assertAlmostEqual(converter.state.alt_relative, 2500.0, places=1)

    def test_heading_conversion(self):
        """Test heading is correctly converted from cdeg to degrees."""
        converter = MAVLinkToAsterixConverter()

        msg = Mock()
        msg.lat = 470000000
        msg.lon = -1220000000
        msg.alt = 1000000
        msg.relative_alt = 500000
        msg.vx = 0
        msg.vy = 0
        msg.vz = 0
        msg.hdg = 18000  # 18000 cdeg = 180 degrees

        converter.update_from_global_position(msg)

        self.assertAlmostEqual(converter.state.heading, 180.0, places=1)

    def test_latitude_longitude_conversion(self):
        """Test lat/lon is correctly converted from degE7 to degrees."""
        converter = MAVLinkToAsterixConverter()

        msg = Mock()
        msg.lat = 474123456  # 47.4123456 degrees
        msg.lon = -1221234567  # -122.1234567 degrees
        msg.alt = 1000000
        msg.relative_alt = 500000
        msg.vx = 0
        msg.vy = 0
        msg.vz = 0
        msg.hdg = 0

        converter.update_from_global_position(msg)

        self.assertAlmostEqual(converter.state.lat, 47.4123456, places=6)
        self.assertAlmostEqual(converter.state.lon, -122.1234567, places=6)


class TestMAVLinkConverterEdgeCases(unittest.TestCase):
    """Edge case tests for MAVLink converter."""

    def test_zero_values(self):
        """Test handling of zero values in all fields."""
        converter = MAVLinkToAsterixConverter()

        msg = Mock()
        msg.lat = 0
        msg.lon = 0
        msg.alt = 0
        msg.relative_alt = 0
        msg.vx = 0
        msg.vy = 0
        msg.vz = 0
        msg.hdg = 0

        converter.update_from_global_position(msg)

        self.assertEqual(converter.state.lat, 0.0)
        self.assertEqual(converter.state.lon, 0.0)
        self.assertEqual(converter.state.alt_msl, 0.0)

    def test_negative_altitude(self):
        """Test handling of negative altitude (below sea level)."""
        converter = MAVLinkToAsterixConverter()

        msg = Mock()
        msg.lat = 470000000
        msg.lon = -1220000000
        msg.alt = -50000  # -50 meters (Dead Sea)
        msg.relative_alt = 500000
        msg.vx = 0
        msg.vy = 0
        msg.vz = 0
        msg.hdg = 0

        converter.update_from_global_position(msg)

        self.assertAlmostEqual(converter.state.alt_msl, -50.0, places=1)

    def test_large_track_number(self):
        """Test handling of large track numbers."""
        converter = MAVLinkToAsterixConverter()
        converter.track_number = 65535
        converter.state.lat = 47.5
        converter.state.lon = -122.3
        converter.state.alt_msl = 1000.0
        converter.state.timestamp = time.time()

        report = converter.to_adsb_report()

        self.assertEqual(report["track_number"], 65535)

    def test_max_sac_sic(self):
        """Test maximum SAC/SIC values."""
        converter = MAVLinkToAsterixConverter(sac=255, sic=255)
        converter.state.lat = 47.5
        converter.state.lon = -122.3
        converter.state.alt_msl = 1000.0
        converter.state.timestamp = time.time()

        report = converter.to_adsb_report()

        self.assertEqual(report["SAC"], 255)
        self.assertEqual(report["SIC"], 255)

    def test_heading_wraparound(self):
        """Test heading values at 0 and 360 degrees."""
        converter = MAVLinkToAsterixConverter()

        # Test 0 degrees
        msg = Mock()
        msg.lat = 470000000
        msg.lon = -1220000000
        msg.alt = 1000000
        msg.relative_alt = 500000
        msg.vx = 0
        msg.vy = 0
        msg.vz = 0
        msg.hdg = 0

        converter.update_from_global_position(msg)
        self.assertAlmostEqual(converter.state.heading, 0.0, places=1)

        # Test 360 degrees (should be 360 or 0)
        msg.hdg = 36000
        converter.update_from_global_position(msg)
        self.assertIn(converter.state.heading, [0.0, 360.0])


if __name__ == "__main__":
    unittest.main()
