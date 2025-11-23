#!/usr/bin/env python3
"""
Unit tests for radar visualization utilities

Tests ASCII radar display and optional matplotlib visualizations.

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import unittest
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from asterix.radar_integration import MockRadar
from asterix.radar_integration.visualization import (
    plot_radar_ascii,
    check_matplotlib_available,
)


class TestASCIIVisualization(unittest.TestCase):
    """Test ASCII radar visualization (no dependencies)."""

    def setUp(self):
        """Set up test radar and plots."""
        self.radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        self.plots = self.radar.generate_plots(num_targets=10, add_noise=False)

    def test_basic_ascii_plot(self):
        """Test basic ASCII radar plot generation."""
        output = plot_radar_ascii(self.plots, radar_pos=(52.5, 13.4))
        self.assertIsInstance(output, str)
        self.assertGreater(len(output), 0)

    def test_ascii_contains_cardinal_directions(self):
        """Test ASCII plot contains cardinal directions."""
        output = plot_radar_ascii(self.plots, radar_pos=(52.5, 13.4))
        self.assertIn('N', output)  # North
        self.assertIn('S', output)  # South
        self.assertIn('E', output)  # East
        self.assertIn('W', output)  # West
        self.assertIn('R', output)  # Radar center

    def test_ascii_with_legend(self):
        """Test ASCII plot includes legend."""
        output = plot_radar_ascii(self.plots, show_legend=True)
        self.assertIn('Legend', output)
        self.assertIn('*', output)  # High SNR symbol

    def test_ascii_with_stats(self):
        """Test ASCII plot includes statistics."""
        output = plot_radar_ascii(self.plots, show_stats=True)
        self.assertIn('Statistics', output)
        self.assertIn('range', output.lower())

    def test_ascii_empty_plots(self):
        """Test ASCII plot with empty plot list."""
        output = plot_radar_ascii([])
        self.assertIn('No radar plots', output)

    def test_ascii_custom_dimensions(self):
        """Test ASCII plot with custom dimensions."""
        output = plot_radar_ascii(self.plots, width=60, height=30)
        lines = output.split('\n')
        # Should have approximately the specified height (plus headers/footers)
        self.assertGreater(len(lines), 30)

    def test_ascii_max_range(self):
        """Test ASCII plot with specified max range."""
        output = plot_radar_ascii(self.plots, max_range_km=100.0)
        self.assertIn('100 km', output)


class TestMatplotlibVisualization(unittest.TestCase):
    """Test matplotlib-based visualizations (if available)."""

    def setUp(self):
        """Set up test radar and plots."""
        self.radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        self.plots = self.radar.generate_plots(num_targets=20, add_noise=True)
        self.has_matplotlib = check_matplotlib_available()

    def test_matplotlib_availability_check(self):
        """Test matplotlib availability detection."""
        result = check_matplotlib_available()
        self.assertIsInstance(result, bool)

    def test_plot_range_azimuth(self):
        """Test polar radar plot (requires matplotlib)."""
        if not self.has_matplotlib:
            self.skipTest("Matplotlib not available")

        from asterix.radar_integration.visualization import plot_range_azimuth

        # Should not raise exception
        try:
            import tempfile
            with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
                filename = f.name

            plot_range_azimuth(self.plots, filename=filename)
            self.assertTrue(os.path.exists(filename))

            # Clean up
            os.unlink(filename)
        except Exception as e:
            self.fail(f"plot_range_azimuth raised exception: {e}")

    def test_plot_tracks(self):
        """Test track plotting (requires matplotlib)."""
        if not self.has_matplotlib:
            self.skipTest("Matplotlib not available")

        from asterix.radar_integration.visualization import plot_tracks

        # Generate test tracks
        track1 = self.radar.generate_track(50e3, 45.0, 200.0, 90.0, duration_s=60.0)
        track2 = self.radar.generate_track(80e3, 180.0, 180.0, 270.0, duration_s=60.0)

        try:
            import tempfile
            with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
                filename = f.name

            plot_tracks([track1, track2], filename=filename)
            self.assertTrue(os.path.exists(filename))

            # Clean up
            os.unlink(filename)
        except Exception as e:
            self.fail(f"plot_tracks raised exception: {e}")

    def test_plot_snr_heatmap(self):
        """Test SNR heatmap (requires matplotlib)."""
        if not self.has_matplotlib:
            self.skipTest("Matplotlib not available")

        from asterix.radar_integration.visualization import plot_snr_heatmap

        # Generate more plots for better heatmap
        plots = self.radar.generate_plots(num_targets=100, add_noise=True)

        try:
            import tempfile
            with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
                filename = f.name

            plot_snr_heatmap(plots, filename=filename)
            self.assertTrue(os.path.exists(filename))

            # Clean up
            os.unlink(filename)
        except Exception as e:
            self.fail(f"plot_snr_heatmap raised exception: {e}")

    def test_plot_elevation_profile(self):
        """Test elevation profile plot (requires matplotlib)."""
        if not self.has_matplotlib:
            self.skipTest("Matplotlib not available")

        from asterix.radar_integration.visualization import plot_elevation_profile

        try:
            import tempfile
            with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
                filename = f.name

            plot_elevation_profile(self.plots, filename=filename)
            self.assertTrue(os.path.exists(filename))

            # Clean up
            os.unlink(filename)
        except Exception as e:
            self.fail(f"plot_elevation_profile raised exception: {e}")

    def test_matplotlib_import_error_handling(self):
        """Test graceful handling when matplotlib not available."""
        if self.has_matplotlib:
            self.skipTest("Matplotlib is available")

        from asterix.radar_integration.visualization import plot_range_azimuth

        with self.assertRaises(ImportError):
            plot_range_azimuth(self.plots)


class TestVisualizationInfo(unittest.TestCase):
    """Test visualization information functions."""

    def test_print_visualization_info(self):
        """Test printing visualization capabilities."""
        from asterix.radar_integration.visualization import print_visualization_info

        # Should not raise exception
        try:
            print_visualization_info()
        except Exception as e:
            self.fail(f"print_visualization_info raised exception: {e}")


if __name__ == '__main__':
    unittest.main()
