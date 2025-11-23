#!/usr/bin/env python3
"""
Radar Data Visualization Demo

Demonstrates visualization capabilities for radar detection data:
- ASCII PPI display (no dependencies)
- Matplotlib-based plots (optional)
- Track visualization
- SNR heatmaps

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(__file__))))

from asterix.radar_integration import MockRadar
from asterix.radar_integration.visualization import (
    plot_radar_ascii,
    check_matplotlib_available,
    print_visualization_info,
)
from asterix.radar_integration.statistics import compute_scenario_stats


def demo_ascii_visualization():
    """Demonstrate ASCII radar visualization (no dependencies)."""
    print("\n" + "=" * 70)
    print("ASCII Radar Visualization Demo (No Dependencies)")
    print("=" * 70)

    # Create mock radar
    radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
    plots = radar.generate_plots(num_targets=30, add_noise=True)

    print(f"\nGenerated {len(plots)} radar detections")
    print("\nPPI Display (Plan Position Indicator):")
    print(plot_radar_ascii(plots, radar_pos=(52.5, 13.4), width=80, height=35))


def demo_statistics():
    """Demonstrate statistics and analysis."""
    print("\n" + "=" * 70)
    print("Radar Statistics Demo")
    print("=" * 70)

    # Create mock radar
    radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
    plots = radar.generate_plots(num_targets=100, add_noise=True)

    # Compute statistics
    stats = compute_scenario_stats(plots)

    print(f"\nScenario Statistics:")
    print(f"  Total detections: {stats['total_detections']}")
    print(f"  Detection rate: {stats['detection_rate']:.2f} detections/sec")


if __name__ == "__main__":
    print("Radar Data Visualization and Analysis Demo")
    demo_ascii_visualization()
    demo_statistics()
