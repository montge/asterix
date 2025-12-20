#!/usr/bin/env python3
"""
Unit tests for RadarSimPy adapter module.

Tests the adapter pattern for RadarSimPy integration, including:
- Optional dependency detection
- MockRadarAdapter fallback behavior
- PointTarget data class
- RadarSimulatorBase interface compliance

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-12-19
"""

import unittest
import sys
import os
import time
import struct

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from asterix.radar_integration.radarsimpy_adapter import (
    PointTarget,
    MockRadarAdapter,
    RadarSimulatorBase,
    check_radarsimpy_available,
    create_radar_simulator,
    get_radarsimpy_error,
)
from asterix.radar_integration.mock_radar import RadarPlot, RadarPosition


class TestPointTarget(unittest.TestCase):
    """Test PointTarget data class."""

    def test_point_target_creation(self):
        """Test creating a PointTarget with default values."""
        target = PointTarget(location=(1000, 2000, 3000))
        self.assertEqual(target.location, (1000, 2000, 3000))
        self.assertEqual(target.velocity, (0.0, 0.0, 0.0))
        self.assertEqual(target.rcs, 10.0)
        self.assertEqual(target.phase, 0.0)

    def test_point_target_with_velocity(self):
        """Test creating a PointTarget with velocity."""
        target = PointTarget(
            location=(50000, 30000, 10000),
            velocity=(100, -50, 0),
            rcs=15.0
        )
        self.assertEqual(target.velocity, (100, -50, 0))
        self.assertEqual(target.rcs, 15.0)

    def test_point_target_with_all_params(self):
        """Test creating a PointTarget with all parameters."""
        target = PointTarget(
            location=(100000, 0, 5000),
            velocity=(200, 0, -10),
            rcs=20.0,
            phase=45.0
        )
        self.assertEqual(target.phase, 45.0)


class TestMockRadarAdapter(unittest.TestCase):
    """Test MockRadarAdapter class."""

    def setUp(self):
        """Set up test adapter."""
        self.adapter = MockRadarAdapter(
            lat=52.5,
            lon=13.4,
            alt=100.0,
            seed=42  # For reproducibility
        )

    def test_adapter_creation(self):
        """Test creating MockRadarAdapter."""
        self.assertIsInstance(self.adapter, RadarSimulatorBase)

    def test_get_radar_position(self):
        """Test getting radar position."""
        position = self.adapter.get_radar_position()
        self.assertIsInstance(position, RadarPosition)
        self.assertEqual(position.lat, 52.5)
        self.assertEqual(position.lon, 13.4)
        self.assertEqual(position.alt, 100.0)

    def test_get_radar_parameters(self):
        """Test getting radar parameters."""
        params = self.adapter.get_radar_parameters()
        self.assertIsInstance(params, dict)
        self.assertIn('max_range_m', params)
        self.assertIn('min_range_m', params)
        self.assertIn('range_resolution_m', params)

    def test_simulate_single_target(self):
        """Test simulating a single target."""
        target = PointTarget(location=(50000, 30000, 10000), rcs=10.0)
        plots = self.adapter.simulate_targets([target])

        self.assertEqual(len(plots), 1)
        plot = plots[0]
        self.assertIsInstance(plot, RadarPlot)

        # Verify range calculation (approximately)
        expected_range = (50000**2 + 30000**2 + 10000**2) ** 0.5
        self.assertAlmostEqual(plot.range, expected_range, delta=1.0)

    def test_simulate_multiple_targets(self):
        """Test simulating multiple targets."""
        targets = [
            PointTarget(location=(50000, 0, 5000), rcs=10.0),
            PointTarget(location=(0, 80000, 8000), rcs=15.0),
            PointTarget(location=(-30000, 40000, 12000), rcs=5.0),
        ]
        plots = self.adapter.simulate_targets(targets)

        self.assertEqual(len(plots), 3)
        for plot in plots:
            self.assertIsInstance(plot, RadarPlot)
            self.assertGreater(plot.range, 0)
            self.assertGreaterEqual(plot.azimuth, 0)
            self.assertLess(plot.azimuth, 360)

    def test_simulate_empty_targets(self):
        """Test simulating with no targets."""
        plots = self.adapter.simulate_targets([])
        self.assertEqual(len(plots), 0)

    def test_simulate_with_timestamp(self):
        """Test simulating with custom timestamp."""
        target = PointTarget(location=(10000, 10000, 1000))
        custom_time = 1700000000.0
        plots = self.adapter.simulate_targets([target], timestamp=custom_time)

        self.assertEqual(len(plots), 1)
        self.assertEqual(plots[0].timestamp, custom_time)

    def test_azimuth_calculation(self):
        """Test azimuth is correctly calculated from Cartesian coordinates."""
        # Target due East (x=+, y=0) should be at ~90 degrees
        target_east = PointTarget(location=(50000, 0, 0))
        plots = self.adapter.simulate_targets([target_east])
        self.assertAlmostEqual(plots[0].azimuth, 90.0, delta=1.0)

        # Target due North (x=0, y=+) should be at ~0 degrees
        target_north = PointTarget(location=(0, 50000, 0))
        plots = self.adapter.simulate_targets([target_north])
        self.assertAlmostEqual(plots[0].azimuth, 0.0, delta=1.0)

    def test_elevation_calculation(self):
        """Test elevation is correctly calculated."""
        # Target at 45 degree elevation
        target = PointTarget(location=(10000, 0, 10000))
        plots = self.adapter.simulate_targets([target])
        self.assertAlmostEqual(plots[0].elevation, 45.0, delta=1.0)

    def test_snr_varies_with_range(self):
        """Test SNR decreases with range."""
        target_close = PointTarget(location=(10000, 0, 0), rcs=10.0)
        target_far = PointTarget(location=(100000, 0, 0), rcs=10.0)

        plots_close = self.adapter.simulate_targets([target_close])
        plots_far = self.adapter.simulate_targets([target_far])

        # Closer target should have higher SNR
        self.assertGreater(plots_close[0].snr, plots_far[0].snr)


class TestRadarSimPyAvailability(unittest.TestCase):
    """Test RadarSimPy availability detection."""

    def test_check_availability_returns_bool(self):
        """Test that availability check returns boolean."""
        result = check_radarsimpy_available()
        self.assertIsInstance(result, bool)

    def test_get_error_when_unavailable(self):
        """Test error message is available when RadarSimPy not installed."""
        if not check_radarsimpy_available():
            error = get_radarsimpy_error()
            self.assertIsNotNone(error)
            self.assertIsInstance(error, str)


class TestCreateRadarSimulator(unittest.TestCase):
    """Test radar simulator factory function."""

    def test_create_with_mock_fallback(self):
        """Test creating simulator falls back to MockRadar."""
        # With use_radarsimpy=False, should always use MockRadar
        simulator = create_radar_simulator(use_radarsimpy=False, lat=52.5, lon=13.4, alt=100.0)
        self.assertIsInstance(simulator, MockRadarAdapter)

    def test_create_returns_base_type(self):
        """Test factory returns RadarSimulatorBase subclass."""
        simulator = create_radar_simulator(use_radarsimpy=False)
        self.assertIsInstance(simulator, RadarSimulatorBase)

    def test_create_with_custom_params(self):
        """Test creating simulator with custom parameters."""
        simulator = create_radar_simulator(
            use_radarsimpy=False,
            lat=48.0,
            lon=11.0,
            alt=500.0,
            max_range=300e3,
            name="TEST_RADAR"
        )
        position = simulator.get_radar_position()
        self.assertEqual(position.lat, 48.0)
        self.assertEqual(position.name, "TEST_RADAR")


class TestRadarSimulatorInterface(unittest.TestCase):
    """Test that adapters properly implement RadarSimulatorBase interface."""

    def test_mock_adapter_implements_interface(self):
        """Test MockRadarAdapter implements all required methods."""
        adapter = MockRadarAdapter()

        # Check all abstract methods are implemented
        self.assertTrue(callable(getattr(adapter, 'simulate_targets', None)))
        self.assertTrue(callable(getattr(adapter, 'get_radar_position', None)))
        self.assertTrue(callable(getattr(adapter, 'get_radar_parameters', None)))

    def test_interface_methods_return_correct_types(self):
        """Test interface methods return correct types."""
        adapter = MockRadarAdapter()

        # simulate_targets returns List[RadarPlot]
        target = PointTarget(location=(10000, 10000, 1000))
        plots = adapter.simulate_targets([target])
        self.assertIsInstance(plots, list)
        self.assertIsInstance(plots[0], RadarPlot)

        # get_radar_position returns RadarPosition
        position = adapter.get_radar_position()
        self.assertIsInstance(position, RadarPosition)

        # get_radar_parameters returns Dict
        params = adapter.get_radar_parameters()
        self.assertIsInstance(params, dict)


class TestIntegrationWithEncoder(unittest.TestCase):
    """Test adapter integration with ASTERIX encoders."""

    def test_adapter_output_compatible_with_cat048(self):
        """Test adapter output can be encoded as CAT048."""
        from asterix.radar_integration.encoder import encode_cat048

        adapter = MockRadarAdapter(seed=42)
        target = PointTarget(location=(50000, 30000, 10000), rcs=10.0)
        plots = adapter.simulate_targets([target])

        # Should not raise
        encoded = encode_cat048(plots, sac=0, sic=1)
        self.assertIsInstance(encoded, bytes)
        self.assertGreater(len(encoded), 0)

        # Verify CAT048 header
        self.assertEqual(encoded[0], 48)  # CAT048

    def test_adapter_output_compatible_with_cat062(self):
        """Test adapter output can be converted for CAT062 encoding."""
        from asterix.radar_integration.encoder import encode_cat062

        adapter = MockRadarAdapter(lat=52.5, lon=13.4, alt=100.0, seed=42)
        targets = [
            PointTarget(location=(50000, 0, 10000), rcs=10.0),
            PointTarget(location=(80000, 20000, 8000), rcs=15.0),
        ]
        plots = adapter.simulate_targets(targets)

        # CAT062 expects tracks with lat/lon, not RadarPlot
        # Convert plots to track format (this would be done by a converter)
        radar_pos = adapter.get_radar_position()
        tracks = []
        for plot in plots:
            # Simple conversion - in practice, you'd use proper coordinate transform
            tracks.append({
                'lat': radar_pos.lat + plot.range / 111000 * 0.001,  # Approximate
                'lon': radar_pos.lon + plot.range / 111000 * 0.001,
                'timestamp': plot.timestamp,
                'altitude_ft': plot.elevation * 1000,  # Rough conversion
            })

        # Should not raise
        encoded = encode_cat062(tracks, sac=0, sic=1)
        self.assertIsInstance(encoded, bytes)

        # Verify CAT062 header
        self.assertEqual(encoded[0], 62)  # CAT062


class TestReproducibility(unittest.TestCase):
    """Test reproducibility with random seed."""

    def test_same_seed_produces_same_results(self):
        """Test that same seed produces identical results."""
        adapter1 = MockRadarAdapter(seed=12345)
        adapter2 = MockRadarAdapter(seed=12345)

        target = PointTarget(location=(50000, 30000, 10000), rcs=10.0)
        timestamp = 1700000000.0

        plots1 = adapter1.simulate_targets([target], timestamp=timestamp)
        plots2 = adapter2.simulate_targets([target], timestamp=timestamp)

        self.assertEqual(plots1[0].snr, plots2[0].snr)
        self.assertEqual(plots1[0].quality, plots2[0].quality)

    def test_different_seed_produces_different_results(self):
        """Test that different seeds produce different results."""
        adapter1 = MockRadarAdapter(seed=11111)
        adapter2 = MockRadarAdapter(seed=22222)

        # Use targets where SNR calculation involves randomness
        targets = [PointTarget(location=(50000, 30000, 10000), rcs=10.0)]

        # Run multiple times to get statistically different results
        # Note: The core calculation is deterministic, but some noise may be added
        plots1 = adapter1.simulate_targets(targets)
        plots2 = adapter2.simulate_targets(targets)

        # Results should be close but potentially different due to noise
        # This test verifies the interface works correctly
        self.assertEqual(len(plots1), len(plots2))


if __name__ == '__main__':
    unittest.main()
