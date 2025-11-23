#!/usr/bin/env python3
"""
Unit tests for mock radar data generation

Tests:
- RadarPlot creation and validation
- RadarPosition creation
- MockRadar initialization
- Plot generation (random targets)
- Track generation (moving targets)
- Scenario generation functions

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import unittest
import sys
import os
import time

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', '..'))

from asterix.radar_integration.mock_radar import (
    RadarPlot,
    RadarPosition,
    MockRadar,
    generate_aircraft_scenario,
    generate_approach_scenario
)


class TestRadarPlot(unittest.TestCase):
    """Test RadarPlot dataclass."""

    def test_radar_plot_creation(self):
        """Test creating a RadarPlot with required fields."""
        plot = RadarPlot(range=50000.0, azimuth=45.0)
        self.assertEqual(plot.range, 50000.0)
        self.assertEqual(plot.azimuth, 45.0)
        self.assertEqual(plot.elevation, 0.0)  # Default value

    def test_radar_plot_all_fields(self):
        """Test creating a RadarPlot with all fields."""
        plot = RadarPlot(
            range=50000.0,
            azimuth=45.0,
            elevation=5.0,
            rcs=15.0,
            snr=30.0,
            doppler=100.0,
            timestamp=1234567890.0,
            amplitude=2.0,
            quality=0.9
        )
        self.assertEqual(plot.rcs, 15.0)
        self.assertEqual(plot.snr, 30.0)
        self.assertEqual(plot.doppler, 100.0)

    def test_radar_plot_to_dict(self):
        """Test converting RadarPlot to dictionary."""
        plot = RadarPlot(range=50000.0, azimuth=45.0)
        d = plot.to_dict()
        self.assertIsInstance(d, dict)
        self.assertEqual(d['range'], 50000.0)
        self.assertEqual(d['azimuth'], 45.0)
        self.assertIn('timestamp', d)


class TestRadarPosition(unittest.TestCase):
    """Test RadarPosition dataclass."""

    def test_radar_position_creation(self):
        """Test creating a RadarPosition."""
        pos = RadarPosition(lat=52.5, lon=13.4, alt=100.0)
        self.assertEqual(pos.lat, 52.5)
        self.assertEqual(pos.lon, 13.4)
        self.assertEqual(pos.alt, 100.0)
        self.assertEqual(pos.name, "RADAR_01")  # Default

    def test_radar_position_with_name(self):
        """Test creating a RadarPosition with custom name."""
        pos = RadarPosition(lat=52.5, lon=13.4, alt=100.0, name="TEST_RADAR")
        self.assertEqual(pos.name, "TEST_RADAR")


class TestMockRadar(unittest.TestCase):
    """Test MockRadar class."""

    def setUp(self):
        """Create a MockRadar instance for testing."""
        self.radar = MockRadar(
            lat=52.5,
            lon=13.4,
            alt=100.0,
            max_range=200e3,
            min_range=1e3,
            name="TEST_RADAR"
        )

    def test_radar_initialization(self):
        """Test MockRadar initialization."""
        self.assertEqual(self.radar.position.lat, 52.5)
        self.assertEqual(self.radar.position.lon, 13.4)
        self.assertEqual(self.radar.max_range, 200e3)
        self.assertEqual(self.radar.min_range, 1e3)

    def test_generate_plots_count(self):
        """Test generating correct number of plots."""
        num_targets = 10
        plots = self.radar.generate_plots(num_targets=num_targets)
        self.assertEqual(len(plots), num_targets)

    def test_generate_plots_range_bounds(self):
        """Test that generated plots are within range bounds."""
        plots = self.radar.generate_plots(num_targets=20)
        for plot in plots:
            self.assertGreaterEqual(plot.range, self.radar.min_range)
            self.assertLessEqual(plot.range, self.radar.max_range)

    def test_generate_plots_azimuth_bounds(self):
        """Test that azimuth values are in 0-360 range."""
        plots = self.radar.generate_plots(num_targets=20)
        for plot in plots:
            self.assertGreaterEqual(plot.azimuth, 0.0)
            self.assertLess(plot.azimuth, 360.0)

    def test_generate_plots_with_noise(self):
        """Test plot generation with noise."""
        plots = self.radar.generate_plots(num_targets=10, add_noise=True)
        self.assertEqual(len(plots), 10)
        # Noise should still keep values in reasonable bounds
        for plot in plots:
            self.assertGreaterEqual(plot.range, 0)
            self.assertGreaterEqual(plot.azimuth, 0)
            self.assertLess(plot.azimuth, 360)

    def test_generate_plots_with_clutter(self):
        """Test plot generation with clutter."""
        num_targets = 10
        plots = self.radar.generate_plots(num_targets=num_targets, add_clutter=True)
        # Should have more plots due to clutter
        self.assertGreaterEqual(len(plots), num_targets)

    def test_generate_plots_timestamp(self):
        """Test that plots have timestamps."""
        timestamp = time.time()
        plots = self.radar.generate_plots(num_targets=5, timestamp=timestamp)
        for plot in plots:
            # All plots in same batch should have same timestamp
            self.assertAlmostEqual(plot.timestamp, timestamp, places=3)

    def test_generate_track_count(self):
        """Test track generation produces plots."""
        track = self.radar.generate_track(
            start_range=50e3,
            start_azimuth=45.0,
            velocity_ms=200.0,
            heading_deg=90.0,
            duration_s=60.0,
            dt=4.0
        )
        expected_updates = int(60.0 / 4.0)
        self.assertGreaterEqual(len(track), 1)
        self.assertLessEqual(len(track), expected_updates)

    def test_generate_track_movement(self):
        """Test that track shows movement over time."""
        track = self.radar.generate_track(
            start_range=50e3,
            start_azimuth=45.0,
            velocity_ms=200.0,
            heading_deg=90.0,
            duration_s=20.0,
            dt=4.0
        )
        if len(track) >= 2:
            # Position should change between updates
            first_plot = track[0]
            last_plot = track[-1]
            # Either range or azimuth should be different
            position_changed = (
                abs(first_plot.range - last_plot.range) > 1.0 or
                abs(first_plot.azimuth - last_plot.azimuth) > 0.1
            )
            self.assertTrue(position_changed, "Track position should change over time")

    def test_generate_track_timestamps(self):
        """Test that track has increasing timestamps."""
        track = self.radar.generate_track(
            start_range=50e3,
            start_azimuth=45.0,
            velocity_ms=200.0,
            heading_deg=90.0,
            duration_s=20.0,
            dt=4.0
        )
        if len(track) >= 2:
            # Timestamps should increase
            for i in range(1, len(track)):
                self.assertGreater(track[i].timestamp, track[i-1].timestamp)


class TestScenarioGeneration(unittest.TestCase):
    """Test scenario generation functions."""

    def test_aircraft_scenario_generation(self):
        """Test generating aircraft scenario."""
        plots = generate_aircraft_scenario(num_aircraft=3, duration=30.0)
        self.assertGreater(len(plots), 0)
        # Should have multiple plots per aircraft
        self.assertGreaterEqual(len(plots), 3)

    def test_aircraft_scenario_bounds(self):
        """Test aircraft scenario produces valid plots."""
        plots = generate_aircraft_scenario(num_aircraft=5, duration=60.0)
        for plot in plots:
            self.assertGreater(plot.range, 0)
            self.assertGreaterEqual(plot.azimuth, 0)
            self.assertLess(plot.azimuth, 360)

    def test_approach_scenario_generation(self):
        """Test generating approach scenario."""
        plots = generate_approach_scenario(num_aircraft=2)
        self.assertGreater(len(plots), 0)

    def test_approach_scenario_multiple_aircraft(self):
        """Test approach scenario with multiple aircraft."""
        num_aircraft = 3
        plots = generate_approach_scenario(num_aircraft=num_aircraft)
        # Each aircraft should have multiple plot updates
        self.assertGreaterEqual(len(plots), num_aircraft)


class TestRadarPhysics(unittest.TestCase):
    """Test that radar physics are realistic."""

    def setUp(self):
        """Create radar for physics tests."""
        self.radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)

    def test_snr_decreases_with_range(self):
        """Test that SNR generally decreases with range (radar equation)."""
        # Generate many plots to get statistical trend
        plots = self.radar.generate_plots(num_targets=100, add_noise=False)

        # Sort by range
        sorted_plots = sorted(plots, key=lambda p: p.range)

        # Divide into near and far groups
        near_half = sorted_plots[:50]
        far_half = sorted_plots[50:]

        avg_snr_near = sum(p.snr for p in near_half) / len(near_half)
        avg_snr_far = sum(p.snr for p in far_half) / len(far_half)

        # Near targets should have higher average SNR
        self.assertGreater(avg_snr_near, avg_snr_far)

    def test_quality_indicator_valid(self):
        """Test that quality indicator is in valid range."""
        plots = self.radar.generate_plots(num_targets=20)
        for plot in plots:
            self.assertGreaterEqual(plot.quality, 0.0)
            self.assertLessEqual(plot.quality, 1.0)


if __name__ == '__main__':
    unittest.main()
