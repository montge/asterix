#!/usr/bin/env python3
"""
Unit tests for radar statistics and analysis utilities

Tests scenario statistics, coverage analysis, and performance metrics.

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
from asterix.radar_integration.statistics import (
    compute_scenario_stats,
    compute_coverage_map,
    analyze_detection_performance,
    analyze_track_statistics,
    compare_scenarios,
)


class TestScenarioStatistics(unittest.TestCase):
    """Test scenario statistics computation."""

    def setUp(self):
        """Set up test radar and plots."""
        self.radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        self.plots = self.radar.generate_plots(num_targets=50, add_noise=True)

    def test_basic_scenario_stats(self):
        """Test basic scenario statistics."""
        stats = compute_scenario_stats(self.plots)

        self.assertIn('total_detections', stats)
        self.assertEqual(stats['total_detections'], 50)

        self.assertIn('detection_rate', stats)
        self.assertGreater(stats['detection_rate'], 0)

        self.assertIn('range_stats', stats)
        self.assertIn('mean', stats['range_stats'])
        self.assertIn('std', stats['range_stats'])

    def test_range_statistics(self):
        """Test range statistics."""
        stats = compute_scenario_stats(self.plots)

        range_stats = stats['range_stats']
        self.assertGreater(range_stats['max'], range_stats['min'])
        self.assertGreaterEqual(range_stats['mean'], range_stats['min'])
        self.assertLessEqual(range_stats['mean'], range_stats['max'])
        self.assertGreaterEqual(range_stats['std'], 0)

    def test_azimuth_coverage(self):
        """Test azimuth coverage statistics."""
        stats = compute_scenario_stats(self.plots)

        az_stats = stats['azimuth_stats']
        self.assertIn('coverage_deg', az_stats)
        self.assertGreaterEqual(az_stats['coverage_deg'], 0)
        self.assertLessEqual(az_stats['coverage_deg'], 360)

    def test_snr_statistics(self):
        """Test SNR statistics."""
        stats = compute_scenario_stats(self.plots)

        snr_stats = stats['snr_stats']
        self.assertIn('mean', snr_stats)
        self.assertIn('min', snr_stats)
        self.assertIn('max', snr_stats)
        self.assertGreater(snr_stats['max'], snr_stats['min'])

    def test_doppler_statistics(self):
        """Test Doppler statistics."""
        stats = compute_scenario_stats(self.plots)

        doppler_stats = stats['doppler_stats']
        self.assertIn('mean', doppler_stats)
        self.assertIn('abs_mean', doppler_stats)
        self.assertIn('stationary_count', doppler_stats)

    def test_spatial_density(self):
        """Test spatial density calculation."""
        stats = compute_scenario_stats(self.plots)

        self.assertIn('spatial_density', stats)
        self.assertGreater(stats['spatial_density'], 0)

        self.assertIn('coverage_area_km2', stats)
        self.assertGreater(stats['coverage_area_km2'], 0)

    def test_empty_plots(self):
        """Test statistics with empty plot list."""
        stats = compute_scenario_stats([])
        self.assertEqual(stats['total_detections'], 0)
        self.assertIn('error', stats)


class TestCoverageAnalysis(unittest.TestCase):
    """Test radar coverage map computation."""

    def setUp(self):
        """Set up test radar and plots."""
        self.radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        self.plots = self.radar.generate_plots(num_targets=100, add_noise=True)

    def test_basic_coverage_map(self):
        """Test basic coverage map computation."""
        coverage = compute_coverage_map(self.plots, grid_size=30)

        self.assertIn('grid_size', coverage)
        self.assertEqual(coverage['grid_size'], 30)

        self.assertIn('total_cells', coverage)
        self.assertIn('covered_cells', coverage)
        self.assertIn('percent_covered', coverage)

    def test_coverage_percentage(self):
        """Test coverage percentage calculation."""
        coverage = compute_coverage_map(self.plots, grid_size=20)

        percent = coverage['percent_covered']
        self.assertGreaterEqual(percent, 0)
        self.assertLessEqual(percent, 100)

        # With 100 plots, we should have some coverage
        self.assertGreater(percent, 0)

    def test_coverage_by_range(self):
        """Test coverage analysis by range bins."""
        coverage = compute_coverage_map(self.plots, grid_size=30)

        self.assertIn('coverage_by_range', coverage)
        range_bins = coverage['coverage_by_range']
        self.assertGreater(len(range_bins), 0)

        for bin_data in range_bins:
            self.assertIn('range_start_km', bin_data)
            self.assertIn('range_end_km', bin_data)
            self.assertIn('percent', bin_data)

    def test_coverage_by_azimuth(self):
        """Test coverage analysis by azimuth sectors."""
        coverage = compute_coverage_map(self.plots, grid_size=30)

        self.assertIn('coverage_by_azimuth', coverage)
        sectors = coverage['coverage_by_azimuth']
        self.assertEqual(len(sectors), 8)  # 8 sectors (N, NE, E, SE, S, SW, W, NW)

        for sector in sectors:
            self.assertIn('sector', sector)
            self.assertIn('percent', sector)

    def test_snr_threshold(self):
        """Test coverage map with SNR threshold."""
        coverage_high = compute_coverage_map(self.plots, min_snr_threshold=30.0)
        coverage_low = compute_coverage_map(self.plots, min_snr_threshold=10.0)

        # Lower threshold should give more coverage
        self.assertGreaterEqual(
            coverage_low['percent_covered'],
            coverage_high['percent_covered']
        )

    def test_empty_plots(self):
        """Test coverage map with empty plot list."""
        coverage = compute_coverage_map([])
        self.assertEqual(coverage['total_cells'], 0)
        self.assertIn('error', coverage)


class TestDetectionPerformance(unittest.TestCase):
    """Test detection performance analysis."""

    def setUp(self):
        """Set up test radar and plots."""
        self.radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        self.plots = self.radar.generate_plots(num_targets=100, add_noise=True)

    def test_basic_performance_analysis(self):
        """Test basic performance analysis."""
        perf = analyze_detection_performance(self.plots, range_bins=10)

        self.assertIn('overall_performance', perf)
        overall = perf['overall_performance']
        self.assertIn('mean_snr', overall)
        self.assertIn('total_detections', overall)

    def test_performance_by_range(self):
        """Test performance analysis by range bins."""
        perf = analyze_detection_performance(self.plots, range_bins=5)

        self.assertIn('performance_by_range', perf)
        range_bins = perf['performance_by_range']
        self.assertEqual(len(range_bins), 5)

        for bin_data in range_bins:
            self.assertIn('range_center_km', bin_data)
            self.assertIn('detection_count', bin_data)

    def test_radar_equation_fit(self):
        """Test radar equation fitting."""
        perf = analyze_detection_performance(self.plots, range_bins=10)

        self.assertIn('radar_equation_fit', perf)
        fit = perf['radar_equation_fit']
        self.assertIn('reference_range_km', fit)

    def test_quality_distribution(self):
        """Test quality distribution histogram."""
        perf = analyze_detection_performance(self.plots)

        self.assertIn('quality_distribution', perf)
        dist = perf['quality_distribution']

        # Should have multiple quality bins
        self.assertGreater(len(dist), 0)

        for bin_name, bin_data in dist.items():
            self.assertIn('count', bin_data)
            self.assertIn('percent', bin_data)

    def test_empty_plots(self):
        """Test performance analysis with empty plot list."""
        perf = analyze_detection_performance([])
        self.assertIn('error', perf)


class TestTrackStatistics(unittest.TestCase):
    """Test track statistics analysis."""

    def setUp(self):
        """Set up test radar and track."""
        self.radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        self.track = self.radar.generate_track(
            start_range=50e3,
            start_azimuth=45.0,
            velocity_ms=200.0,
            heading_deg=90.0,
            duration_s=60.0
        )

    def test_basic_track_stats(self):
        """Test basic track statistics."""
        stats = analyze_track_statistics(self.track)

        self.assertIn('duration', stats)
        self.assertIn('num_updates', stats)
        self.assertIn('update_rate', stats)

    def test_range_statistics(self):
        """Test track range statistics."""
        stats = analyze_track_statistics(self.track)

        self.assertIn('range_stats', stats)
        range_stats = stats['range_stats']
        self.assertIn('start', range_stats)
        self.assertIn('end', range_stats)
        self.assertIn('change', range_stats)
        self.assertIn('rate', range_stats)

    def test_velocity_estimation(self):
        """Test velocity estimation from track."""
        stats = analyze_track_statistics(self.track)

        self.assertIn('velocity_estimate', stats)
        self.assertIsNotNone(stats['velocity_estimate'])

        self.assertIn('velocity_from_doppler', stats)
        self.assertIn('velocity_from_range', stats)

    def test_heading_estimation(self):
        """Test heading estimation from track."""
        stats = analyze_track_statistics(self.track)

        self.assertIn('heading_estimate', stats)
        heading = stats['heading_estimate']

        if heading is not None:
            self.assertGreaterEqual(heading, 0)
            self.assertLess(heading, 360)

    def test_snr_variation(self):
        """Test SNR variation in track."""
        stats = analyze_track_statistics(self.track)

        self.assertIn('snr_stats', stats)
        snr_stats = stats['snr_stats']
        self.assertIn('mean', snr_stats)
        self.assertIn('std', snr_stats)

    def test_insufficient_track_points(self):
        """Test track with insufficient points."""
        single_point = [self.track[0]]
        stats = analyze_track_statistics(single_point)
        self.assertIn('error', stats)


class TestScenarioComparison(unittest.TestCase):
    """Test scenario comparison functionality."""

    def setUp(self):
        """Set up test scenarios."""
        self.radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        self.scenario1 = self.radar.generate_plots(num_targets=30)
        self.scenario2 = self.radar.generate_plots(num_targets=50)

    def test_basic_comparison(self):
        """Test basic scenario comparison."""
        scenarios = {
            'Scenario A': self.scenario1,
            'Scenario B': self.scenario2,
        }
        comparison = compare_scenarios(scenarios)

        self.assertIn('scenarios', comparison)
        self.assertIn('summary', comparison)

    def test_comparison_metrics(self):
        """Test comparison includes relevant metrics."""
        scenarios = {
            'Low Density': self.scenario1,
            'High Density': self.scenario2,
        }
        comparison = compare_scenarios(scenarios)

        for name in scenarios:
            self.assertIn(name, comparison['scenarios'])
            scenario_data = comparison['scenarios'][name]

            self.assertIn('detection_count', scenario_data)
            self.assertIn('mean_range_km', scenario_data)
            self.assertIn('mean_snr', scenario_data)

    def test_empty_scenarios(self):
        """Test comparison with empty scenarios dict."""
        comparison = compare_scenarios({})
        self.assertIn('error', comparison)


if __name__ == '__main__':
    unittest.main()
