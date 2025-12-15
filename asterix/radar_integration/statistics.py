#!/usr/bin/env python3
"""
Radar Data Statistics and Analysis Utilities

Provides statistical analysis tools for radar detection data, including
scenario characterization, coverage analysis, and performance metrics.

Usage:
    from asterix_encoder.statistics import (
        compute_scenario_stats,
        compute_coverage_map,
        analyze_detection_performance
    )
    from mock_radar import MockRadar

    # Generate radar data
    radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
    plots = radar.generate_plots(num_targets=100)

    # Compute statistics
    stats = compute_scenario_stats(plots)
    print(f"Detection rate: {stats['detection_rate']:.2f} detections/second")

    # Analyze coverage
    coverage = compute_coverage_map(plots, grid_size=50)
    print(f"Coverage: {coverage['percent_covered']:.1f}%")

    # Performance analysis
    perf = analyze_detection_performance(plots)
    print(f"Mean SNR: {perf['mean_snr']:.1f} dB")

---

License: GPL-3.0 (same as ASTERIX project)
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import math
from typing import List, Dict, Optional

# Constants to avoid string duplication (SonarCloud S1192)
ERR_NO_PLOTS = 'No plots provided'


def compute_scenario_stats(plots: List, time_window: Optional[float] = None) -> Dict:
    """
    Compute comprehensive statistics for a radar scenario.

    Args:
        plots: List of RadarPlot objects
        time_window: Time window in seconds for rate calculations (auto if None)

    Returns:
        Dictionary containing scenario statistics:
            - total_detections: Total number of detections
            - detection_rate: Detections per second
            - range_stats: Min, max, mean, std of range
            - azimuth_coverage: Azimuth coverage in degrees
            - elevation_stats: Min, max, mean of elevation
            - snr_stats: SNR statistics
            - doppler_stats: Doppler statistics
            - spatial_density: Detections per square km

    Example:
        >>> plots = radar.generate_plots(num_targets=50)
        >>> stats = compute_scenario_stats(plots)
        >>> print(f"Range: {stats['range_stats']['mean']:.1f} m")
    """
    if not plots:
        return {
            'total_detections': 0,
            'error': ERR_NO_PLOTS
        }

    # Time statistics
    timestamps = [p.timestamp for p in plots]
    time_span = max(timestamps) - min(timestamps) if len(timestamps) > 1 else 0

    # If all timestamps are the same, use 1 second as default window
    if time_span == 0:
        time_span = 1.0

    if time_window is None:
        time_window = time_span

    detection_rate = len(plots) / time_window if time_window > 0 else 0

    # Range statistics
    ranges = [p.range for p in plots]
    range_mean = sum(ranges) / len(ranges)
    range_variance = sum((r - range_mean) ** 2 for r in ranges) / len(ranges)
    range_std = math.sqrt(range_variance)

    # Azimuth coverage
    azimuths = sorted([p.azimuth for p in plots])
    azimuth_gaps = []
    for i in range(len(azimuths)):
        gap = (azimuths[(i + 1) % len(azimuths)] - azimuths[i]) % 360
        azimuth_gaps.append(gap)

    max_gap = max(azimuth_gaps) if azimuth_gaps else 360
    azimuth_coverage = 360 - max_gap

    # Elevation statistics
    elevations = [p.elevation for p in plots]

    # SNR statistics
    snrs = [p.snr for p in plots]
    snr_mean = sum(snrs) / len(snrs)

    # Doppler statistics
    dopplers = [p.doppler for p in plots]
    doppler_mean = sum(dopplers) / len(dopplers)
    doppler_abs_mean = sum(abs(d) for d in dopplers) / len(dopplers)

    # Spatial density (detections per square km)
    max_range = max(ranges)
    coverage_area_km2 = math.pi * (max_range / 1000.0) ** 2 * (azimuth_coverage / 360.0)
    spatial_density = len(plots) / coverage_area_km2 if coverage_area_km2 > 0 else 0

    return {
        'total_detections': len(plots),
        'detection_rate': detection_rate,
        'time_span': time_span,
        'range_stats': {
            'min': min(ranges),
            'max': max(ranges),
            'mean': range_mean,
            'std': range_std,
            'min_km': min(ranges) / 1000.0,
            'max_km': max(ranges) / 1000.0,
            'mean_km': range_mean / 1000.0,
        },
        'azimuth_stats': {
            'min': min(azimuths),
            'max': max(azimuths),
            'coverage_deg': azimuth_coverage,
            'max_gap_deg': max_gap,
            'full_coverage': max_gap < 30  # Consider full coverage if max gap < 30°
        },
        'elevation_stats': {
            'min': min(elevations),
            'max': max(elevations),
            'mean': sum(elevations) / len(elevations),
        },
        'snr_stats': {
            'min': min(snrs),
            'max': max(snrs),
            'mean': snr_mean,
            'high_quality_count': sum(1 for s in snrs if s > 30),
            'low_quality_count': sum(1 for s in snrs if s < 15),
        },
        'doppler_stats': {
            'min': min(dopplers),
            'max': max(dopplers),
            'mean': doppler_mean,
            'abs_mean': doppler_abs_mean,
            'stationary_count': sum(1 for d in dopplers if abs(d) < 10),
            'approaching_count': sum(1 for d in dopplers if d > 10),
            'receding_count': sum(1 for d in dopplers if d < -10),
        },
        'spatial_density': spatial_density,
        'coverage_area_km2': coverage_area_km2,
    }


def compute_coverage_map(
    plots: List,
    grid_size: int = 50,
    max_range_km: Optional[float] = None,
    min_snr_threshold: float = 10.0
) -> Dict:
    """
    Compute radar coverage map using grid-based analysis.

    Divides the surveillance area into a grid and determines which cells
    have adequate radar coverage (at least one detection above SNR threshold).

    Args:
        plots: List of RadarPlot objects
        grid_size: Grid resolution (cells per dimension)
        max_range_km: Maximum range for analysis (auto if None)
        min_snr_threshold: Minimum SNR for valid detection (dB)

    Returns:
        Dictionary containing coverage analysis:
            - grid_size: Grid resolution used
            - max_range_km: Maximum range analyzed
            - total_cells: Total grid cells
            - covered_cells: Cells with valid detections
            - percent_covered: Coverage percentage
            - coverage_by_range: Coverage vs range bins
            - coverage_by_azimuth: Coverage vs azimuth bins

    Example:
        >>> plots = radar.generate_plots(num_targets=200)
        >>> coverage = compute_coverage_map(plots, grid_size=40)
        >>> print(f"Coverage: {coverage['percent_covered']:.1f}%")
    """
    if not plots:
        return {
            'grid_size': grid_size,
            'total_cells': 0,
            'error': ERR_NO_PLOTS
        }

    # Determine range extent
    if max_range_km is None:
        max_range_km = max(p.range for p in plots) / 1000.0

    max_range = max_range_km * 1000.0

    # Initialize coverage grid (range bins x azimuth bins)
    range_bins = grid_size
    azimuth_bins = grid_size
    coverage_grid = [[False for _ in range(azimuth_bins)] for _ in range(range_bins)]

    # Mark covered cells
    covered_count = 0
    for plot in plots:
        if plot.range > max_range or plot.snr < min_snr_threshold:
            continue

        # Compute grid indices
        range_idx = min(int((plot.range / max_range) * range_bins), range_bins - 1)
        azimuth_idx = min(int((plot.azimuth / 360.0) * azimuth_bins), azimuth_bins - 1)

        if not coverage_grid[range_idx][azimuth_idx]:
            coverage_grid[range_idx][azimuth_idx] = True
            covered_count += 1

    total_cells = range_bins * azimuth_bins
    percent_covered = (covered_count / total_cells * 100.0) if total_cells > 0 else 0

    # Coverage by range bins
    coverage_by_range = []
    for r_idx in range(range_bins):
        covered_in_range = sum(1 for a_idx in range(azimuth_bins) if coverage_grid[r_idx][a_idx])
        range_start = (r_idx / range_bins) * max_range_km
        range_end = ((r_idx + 1) / range_bins) * max_range_km
        coverage_by_range.append({
            'range_start_km': range_start,
            'range_end_km': range_end,
            'covered_cells': covered_in_range,
            'total_cells': azimuth_bins,
            'percent': (covered_in_range / azimuth_bins * 100.0) if azimuth_bins > 0 else 0
        })

    # Coverage by azimuth sectors (8 sectors: N, NE, E, SE, S, SW, W, NW)
    sector_size = azimuth_bins // 8
    coverage_by_azimuth = []
    sector_names = ['N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW']

    for sector_idx in range(8):
        az_start = sector_idx * sector_size
        az_end = min((sector_idx + 1) * sector_size, azimuth_bins)
        covered_in_sector = sum(
            1 for r_idx in range(range_bins)
            for a_idx in range(az_start, az_end)
            if coverage_grid[r_idx][a_idx]
        )
        total_in_sector = range_bins * (az_end - az_start)
        coverage_by_azimuth.append({
            'sector': sector_names[sector_idx],
            'azimuth_start': sector_idx * 45.0,
            'azimuth_end': (sector_idx + 1) * 45.0,
            'covered_cells': covered_in_sector,
            'total_cells': total_in_sector,
            'percent': (covered_in_sector / total_in_sector * 100.0) if total_in_sector > 0 else 0
        })

    return {
        'grid_size': grid_size,
        'max_range_km': max_range_km,
        'min_snr_threshold': min_snr_threshold,
        'total_cells': total_cells,
        'covered_cells': covered_count,
        'percent_covered': percent_covered,
        'coverage_by_range': coverage_by_range,
        'coverage_by_azimuth': coverage_by_azimuth,
    }


def analyze_detection_performance(
    plots: List,
    range_bins: int = 10,
    min_detections_per_bin: int = 3
) -> Dict:
    """
    Analyze radar detection performance vs range.

    Computes SNR degradation, detection density, and quality metrics as
    a function of range. Useful for radar equation validation and
    performance characterization.

    Args:
        plots: List of RadarPlot objects
        range_bins: Number of range bins for analysis
        min_detections_per_bin: Minimum detections required for valid statistics

    Returns:
        Dictionary containing performance metrics:
            - overall_performance: Overall SNR and quality statistics
            - performance_by_range: Per-range-bin analysis
            - range_equation_fit: Radar equation parameters (if applicable)
            - detection_quality_distribution: Quality histogram

    Example:
        >>> plots = radar.generate_plots(num_targets=100)
        >>> perf = analyze_detection_performance(plots)
        >>> print(f"SNR at 50km: {perf['performance_by_range'][5]['mean_snr']:.1f} dB")
    """
    if not plots:
        return {
            'overall_performance': {},
            'error': ERR_NO_PLOTS
        }

    # Overall statistics
    snrs = [p.snr for p in plots]
    qualities = [p.quality for p in plots]
    rcss = [p.rcs for p in plots]

    overall = {
        'total_detections': len(plots),
        'mean_snr': sum(snrs) / len(snrs),
        'min_snr': min(snrs),
        'max_snr': max(snrs),
        'mean_quality': sum(qualities) / len(qualities),
        'mean_rcs': sum(rcss) / len(rcss),
        'high_quality_percent': sum(1 for q in qualities if q > 0.8) / len(qualities) * 100.0,
    }

    # Performance by range bins
    max_range = max(p.range for p in plots)
    range_bin_size = max_range / range_bins

    performance_by_range = []
    for bin_idx in range(range_bins):
        range_start = bin_idx * range_bin_size
        range_end = (bin_idx + 1) * range_bin_size
        range_center = (range_start + range_end) / 2.0

        # Filter plots in this range bin
        bin_plots = [p for p in plots if range_start <= p.range < range_end]

        if len(bin_plots) >= min_detections_per_bin:
            bin_snrs = [p.snr for p in bin_plots]
            bin_qualities = [p.quality for p in bin_plots]

            bin_stats = {
                'range_start_km': range_start / 1000.0,
                'range_end_km': range_end / 1000.0,
                'range_center_km': range_center / 1000.0,
                'detection_count': len(bin_plots),
                'mean_snr': sum(bin_snrs) / len(bin_snrs),
                'std_snr': math.sqrt(sum((s - sum(bin_snrs) / len(bin_snrs)) ** 2 for s in bin_snrs) / len(bin_snrs)),
                'min_snr': min(bin_snrs),
                'max_snr': max(bin_snrs),
                'mean_quality': sum(bin_qualities) / len(bin_qualities),
            }
        else:
            bin_stats = {
                'range_start_km': range_start / 1000.0,
                'range_end_km': range_end / 1000.0,
                'range_center_km': range_center / 1000.0,
                'detection_count': len(bin_plots),
                'mean_snr': None,
                'insufficient_data': True
            }

        performance_by_range.append(bin_stats)

    # Radar equation fit (simplified)
    # SNR ∝ 1/R^4 for monostatic radar
    # Try to estimate reference SNR at 10km
    ref_range_km = 10.0
    snr_at_ref = None

    # Find bins near reference range
    ref_bins = [b for b in performance_by_range
                if b.get('mean_snr') is not None
                and abs(b['range_center_km'] - ref_range_km) < ref_range_km * 0.5]

    if ref_bins:
        # Weight by detection count
        total_weight = sum(b['detection_count'] for b in ref_bins)
        snr_at_ref = sum(b['mean_snr'] * b['detection_count'] for b in ref_bins) / total_weight

        # Predict SNR at other ranges
        for bin_stats in performance_by_range:
            if bin_stats.get('mean_snr') is not None:
                predicted_snr = snr_at_ref - 40 * math.log10(bin_stats['range_center_km'] / ref_range_km)
                bin_stats['predicted_snr'] = predicted_snr
                bin_stats['snr_error'] = bin_stats['mean_snr'] - predicted_snr

    # Quality distribution
    quality_bins = [0.0, 0.2, 0.4, 0.6, 0.8, 1.0]
    quality_distribution = {}
    for i in range(len(quality_bins) - 1):
        low = quality_bins[i]
        high = quality_bins[i + 1]
        count = sum(1 for q in qualities if low <= q < high)
        quality_distribution[f'{low:.1f}-{high:.1f}'] = {
            'count': count,
            'percent': count / len(qualities) * 100.0
        }

    return {
        'overall_performance': overall,
        'performance_by_range': performance_by_range,
        'radar_equation_fit': {
            'reference_range_km': ref_range_km,
            'snr_at_reference': snr_at_ref,
            'equation': 'SNR(R) = SNR_ref - 40*log10(R/R_ref)' if snr_at_ref else None
        },
        'quality_distribution': quality_distribution,
    }


def analyze_track_statistics(track: List) -> Dict:
    """
    Analyze statistics for a single radar track.

    Args:
        track: List of RadarPlot objects forming a track

    Returns:
        Dictionary containing track statistics:
            - duration: Track duration in seconds
            - num_updates: Number of radar updates
            - range_variation: Range change statistics
            - velocity_estimate: Estimated target velocity
            - heading_estimate: Estimated target heading
            - snr_variation: SNR variation over track

    Example:
        >>> track = radar.generate_track(50e3, 45.0, 200.0, 90.0, 120.0)
        >>> stats = analyze_track_statistics(track)
        >>> print(f"Velocity: {stats['velocity_estimate']:.1f} m/s")
    """
    if len(track) < 2:
        return {
            'error': 'Track must have at least 2 points'
        }

    # Time span
    timestamps = [p.timestamp for p in track]
    duration = max(timestamps) - min(timestamps)

    # Range variation
    ranges = [p.range for p in track]
    range_change = ranges[-1] - ranges[0]
    range_rate = range_change / duration if duration > 0 else 0

    # Azimuth change
    azimuths = [p.azimuth for p in track]
    # Handle azimuth wraparound
    azimuth_diffs = []
    for i in range(1, len(azimuths)):
        diff = azimuths[i] - azimuths[i - 1]
        # Normalize to -180 to +180
        if diff > 180:
            diff -= 360
        elif diff < -180:
            diff += 360
        azimuth_diffs.append(diff)

    azimuth_change = sum(azimuth_diffs)

    # Velocity estimate (from Doppler or range rate)
    dopplers = [p.doppler for p in track]
    mean_doppler = sum(dopplers) / len(dopplers)

    # Convert Doppler to velocity (for L-band: v ≈ Doppler / 8.67)
    velocity_from_doppler = mean_doppler / 8.67

    # Velocity from range rate (radial component only)
    velocity_from_range = -range_rate  # Negative because decreasing range = approaching

    # SNR variation
    snrs = [p.snr for p in track]
    snr_mean = sum(snrs) / len(snrs)
    snr_std = math.sqrt(sum((s - snr_mean) ** 2 for s in snrs) / len(snrs))

    # Heading estimate (very rough approximation)
    # This is simplified and assumes flat earth
    if len(track) > 1:
        start_x = track[0].range * math.sin(math.radians(track[0].azimuth))
        start_y = track[0].range * math.cos(math.radians(track[0].azimuth))
        end_x = track[-1].range * math.sin(math.radians(track[-1].azimuth))
        end_y = track[-1].range * math.cos(math.radians(track[-1].azimuth))

        dx = end_x - start_x
        dy = end_y - start_y
        heading = math.degrees(math.atan2(dx, dy)) % 360
        distance_traveled = math.sqrt(dx**2 + dy**2)
        velocity_estimate = distance_traveled / duration if duration > 0 else 0
    else:
        heading = None
        velocity_estimate = None

    return {
        'duration': duration,
        'num_updates': len(track),
        'update_rate': len(track) / duration if duration > 0 else 0,
        'range_stats': {
            'start': ranges[0],
            'end': ranges[-1],
            'min': min(ranges),
            'max': max(ranges),
            'change': range_change,
            'rate': range_rate,
        },
        'azimuth_stats': {
            'start': azimuths[0],
            'end': azimuths[-1],
            'change': azimuth_change,
        },
        'velocity_estimate': velocity_estimate,
        'velocity_from_doppler': velocity_from_doppler,
        'velocity_from_range': velocity_from_range,
        'heading_estimate': heading,
        'snr_stats': {
            'mean': snr_mean,
            'std': snr_std,
            'min': min(snrs),
            'max': max(snrs),
        },
        'distance_traveled': distance_traveled if len(track) > 1 else 0,
    }


def compare_scenarios(scenarios: Dict[str, List]) -> Dict:
    """
    Compare multiple radar scenarios.

    Args:
        scenarios: Dictionary mapping scenario names to lists of RadarPlot objects

    Returns:
        Dictionary containing comparative statistics

    Example:
        >>> scenario1 = radar.generate_plots(num_targets=50)
        >>> scenario2 = generate_aircraft_scenario(num_aircraft=5, duration=60)
        >>> comparison = compare_scenarios({'Static': scenario1, 'Dynamic': scenario2})
        >>> print(comparison['summary'])
    """
    if not scenarios:
        return {'error': 'No scenarios provided'}

    comparison = {}

    for name, plots in scenarios.items():
        if plots:
            stats = compute_scenario_stats(plots)
            comparison[name] = {
                'detection_count': stats['total_detections'],
                'mean_range_km': stats['range_stats']['mean_km'],
                'mean_snr': stats['snr_stats']['mean'],
                'azimuth_coverage': stats['azimuth_stats']['coverage_deg'],
                'spatial_density': stats['spatial_density'],
            }

    # Summary comparison
    summary = []
    for name in comparison:
        summary.append(f"{name}: {comparison[name]['detection_count']} detections, "
                      f"mean range {comparison[name]['mean_range_km']:.1f} km, "
                      f"mean SNR {comparison[name]['mean_snr']:.1f} dB")

    return {
        'scenarios': comparison,
        'summary': '\n'.join(summary)
    }


if __name__ == "__main__":
    # Simple test/demo
    print("=" * 60)
    print("Radar Statistics Module - Test")
    print("=" * 60)
    print()

    try:
        import sys
        import os
        sys.path.insert(0, os.path.dirname(__file__))
        from mock_radar import MockRadar

        print("Generating test radar data...")
        radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        plots = radar.generate_plots(num_targets=100, add_noise=True)

        print("\n1. Scenario Statistics:")
        print("-" * 60)
        stats = compute_scenario_stats(plots)
        print(f"Total detections: {stats['total_detections']}")
        print(f"Range: {stats['range_stats']['min_km']:.1f} - {stats['range_stats']['max_km']:.1f} km "
              f"(mean: {stats['range_stats']['mean_km']:.1f} km)")
        print(f"SNR: {stats['snr_stats']['min']:.1f} - {stats['snr_stats']['max']:.1f} dB "
              f"(mean: {stats['snr_stats']['mean']:.1f} dB)")
        print(f"Azimuth coverage: {stats['azimuth_stats']['coverage_deg']:.0f}°")
        print(f"Spatial density: {stats['spatial_density']:.2f} detections/km²")

        print("\n2. Coverage Analysis:")
        print("-" * 60)
        coverage = compute_coverage_map(plots, grid_size=30)
        print(f"Grid size: {coverage['grid_size']}x{coverage['grid_size']}")
        print(f"Coverage: {coverage['percent_covered']:.1f}% ({coverage['covered_cells']}/{coverage['total_cells']} cells)")

        print("\n3. Detection Performance:")
        print("-" * 60)
        perf = analyze_detection_performance(plots, range_bins=5)
        print(f"Overall mean SNR: {perf['overall_performance']['mean_snr']:.1f} dB")
        print(f"High quality detections: {perf['overall_performance']['high_quality_percent']:.1f}%")

        if perf['radar_equation_fit']['snr_at_reference']:
            print(f"Reference SNR (10km): {perf['radar_equation_fit']['snr_at_reference']:.1f} dB")

        print("\n✓ Statistics test complete!")

    except ImportError as e:
        print(f"Test failed: {e}")
        print("This is expected if mock_radar.py is not in the same directory")
