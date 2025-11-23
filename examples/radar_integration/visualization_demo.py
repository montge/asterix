#!/usr/bin/env python3
"""
Radar Visualization Demonstration

Comprehensive demonstration of all radar data visualization and analysis
capabilities. Shows both ASCII-based (no dependencies) and matplotlib-based
visualizations.

This example demonstrates:
    1. ASCII radar PPI display (always available)
    2. Matplotlib polar plots (if matplotlib available)
    3. Track visualization
    4. SNR heatmaps
    5. Statistical analysis
    6. Coverage maps

Usage:
    python3 visualization_demo.py

    # Save matplotlib plots to files
    python3 visualization_demo.py --save

    # Skip matplotlib plots
    python3 visualization_demo.py --ascii-only

Requirements:
    - Python 3.10+
    - numpy (for mock radar generator)
    - matplotlib (optional, for advanced visualizations)

---

License: GPL-3.0 (same as ASTERIX project)
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import sys
import os
import argparse

# Add path to integration modules
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../.local/integration'))

from mock_radar import MockRadar, generate_aircraft_scenario, generate_approach_scenario
from asterix_encoder.visualization import (
    plot_radar_ascii,
    plot_range_azimuth,
    plot_tracks,
    plot_snr_heatmap,
    plot_elevation_profile,
    check_matplotlib_available,
    print_visualization_info
)
from asterix_encoder.statistics import (
    compute_scenario_stats,
    compute_coverage_map,
    analyze_detection_performance,
    analyze_track_statistics,
    compare_scenarios
)


def demo_ascii_visualization(radar, save_mode=False):
    """Demonstrate ASCII-based visualization (no dependencies)."""
    print("\n" + "=" * 80)
    print("DEMONSTRATION 1: ASCII Radar Display (No Dependencies)")
    print("=" * 80)
    print("\nGenerating radar scenario (20 targets)...")

    plots = radar.generate_plots(num_targets=20, add_noise=True, add_clutter=False)

    print(f"Generated {len(plots)} radar plots\n")
    print(plot_radar_ascii(
        plots,
        radar_pos=(radar.position.lat, radar.position.lon),
        width=80,
        height=35,
        show_legend=True,
        show_stats=True
    ))

    if save_mode:
        # Save to text file
        output_file = "radar_ascii_display.txt"
        with open(output_file, 'w') as f:
            f.write(plot_radar_ascii(
                plots,
                radar_pos=(radar.position.lat, radar.position.lon),
                width=80,
                height=35
            ))
        print(f"\n✓ Saved ASCII display to: {output_file}")


def demo_matplotlib_visualizations(radar, save_mode=False):
    """Demonstrate matplotlib-based visualizations."""
    if not check_matplotlib_available():
        print("\n⚠️  Matplotlib not available. Skipping advanced visualizations.")
        print("   Install with: pip install matplotlib")
        return

    print("\n" + "=" * 80)
    print("DEMONSTRATION 2: Matplotlib Visualizations")
    print("=" * 80)

    # Generate larger scenario for better visualization
    print("\nGenerating radar scenario (100 targets)...")
    plots = radar.generate_plots(num_targets=100, add_noise=True, add_clutter=True)
    print(f"Generated {len(plots)} radar plots")

    # 1. Range-Azimuth PPI Display
    print("\n1. Creating Range-Azimuth PPI Display...")
    filename = "radar_ppi.png" if save_mode else None
    plot_range_azimuth(
        plots,
        filename=filename,
        title="Radar PPI Display - 100 Targets",
        show_snr=True,
        max_range_km=None  # Auto-detect
    )

    # 2. SNR Heatmap
    print("2. Creating SNR Heatmap...")
    filename = "snr_heatmap.png" if save_mode else None
    plot_snr_heatmap(
        plots,
        filename=filename,
        title="SNR Coverage Heatmap",
        grid_size=(40, 72)  # 40 range bins, 72 azimuth bins (5° resolution)
    )

    # 3. Elevation Profile
    print("3. Creating Elevation Profile...")
    filename = "elevation_profile.png" if save_mode else None
    plot_elevation_profile(
        plots,
        filename=filename,
        title="Elevation vs Range Profile"
    )


def demo_track_visualization(radar, save_mode=False):
    """Demonstrate track visualization."""
    if not check_matplotlib_available():
        return

    print("\n" + "=" * 80)
    print("DEMONSTRATION 3: Track Visualization")
    print("=" * 80)

    print("\nGenerating 5 aircraft tracks (120 seconds each)...")

    tracks = []
    for i in range(5):
        track = radar.generate_track(
            start_range=50e3 + i * 20e3,
            start_azimuth=i * 72.0,  # Evenly distributed
            velocity_ms=180.0 + i * 20.0,  # Different speeds
            heading_deg=(i * 72.0 + 90.0) % 360,  # Tangential motion
            duration_s=120.0,
            dt=4.0
        )
        tracks.append(track)
        print(f"  Track {i + 1}: {len(track)} updates, "
              f"velocity {180.0 + i * 20.0:.0f} m/s")

    # Visualize tracks
    print("\nPlotting tracks with time evolution...")
    filename = "radar_tracks.png" if save_mode else None
    plot_tracks(
        tracks,
        filename=filename,
        title="Multi-Target Tracking (5 Aircraft)",
        show_time_evolution=True
    )


def demo_statistics(radar):
    """Demonstrate statistical analysis."""
    print("\n" + "=" * 80)
    print("DEMONSTRATION 4: Statistical Analysis")
    print("=" * 80)

    print("\nGenerating test scenario (150 targets)...")
    plots = radar.generate_plots(num_targets=150, add_noise=True, add_clutter=True)

    # 1. Scenario Statistics
    print("\n1. Scenario Statistics:")
    print("-" * 80)
    stats = compute_scenario_stats(plots)

    print(f"Total detections: {stats['total_detections']}")
    print(f"Detection rate: {stats['detection_rate']:.2f} detections/second")
    print(f"\nRange statistics:")
    print(f"  Min: {stats['range_stats']['min_km']:.1f} km")
    print(f"  Max: {stats['range_stats']['max_km']:.1f} km")
    print(f"  Mean: {stats['range_stats']['mean_km']:.1f} km ± {stats['range_stats']['std'] / 1000.0:.1f} km")
    print(f"\nSNR statistics:")
    print(f"  Min: {stats['snr_stats']['min']:.1f} dB")
    print(f"  Max: {stats['snr_stats']['max']:.1f} dB")
    print(f"  Mean: {stats['snr_stats']['mean']:.1f} dB")
    print(f"  High quality (>30dB): {stats['snr_stats']['high_quality_count']} detections")
    print(f"  Low quality (<15dB): {stats['snr_stats']['low_quality_count']} detections")
    print(f"\nAzimuth coverage: {stats['azimuth_stats']['coverage_deg']:.0f}°")
    print(f"Full 360° coverage: {'Yes' if stats['azimuth_stats']['full_coverage'] else 'No'}")
    print(f"\nSpatial density: {stats['spatial_density']:.2f} detections/km²")
    print(f"Coverage area: {stats['coverage_area_km2']:.1f} km²")

    # 2. Coverage Map
    print("\n2. Coverage Map Analysis:")
    print("-" * 80)
    coverage = compute_coverage_map(plots, grid_size=40, min_snr_threshold=10.0)

    print(f"Grid resolution: {coverage['grid_size']}x{coverage['grid_size']}")
    print(f"Total cells: {coverage['total_cells']}")
    print(f"Covered cells: {coverage['covered_cells']}")
    print(f"Coverage: {coverage['percent_covered']:.1f}%")
    print(f"\nCoverage by azimuth sector:")
    for sector in coverage['coverage_by_azimuth']:
        print(f"  {sector['sector']:3s} ({sector['azimuth_start']:3.0f}°-{sector['azimuth_end']:3.0f}°): "
              f"{sector['percent']:5.1f}% ({sector['covered_cells']}/{sector['total_cells']})")

    # 3. Detection Performance
    print("\n3. Detection Performance Analysis:")
    print("-" * 80)
    perf = analyze_detection_performance(plots, range_bins=8)

    print(f"Overall mean SNR: {perf['overall_performance']['mean_snr']:.1f} dB")
    print(f"High quality detections: {perf['overall_performance']['high_quality_percent']:.1f}%")

    if perf['radar_equation_fit']['snr_at_reference']:
        print(f"\nRadar equation fit:")
        print(f"  Reference SNR at 10km: {perf['radar_equation_fit']['snr_at_reference']:.1f} dB")
        print(f"  Equation: {perf['radar_equation_fit']['equation']}")

    print(f"\nPerformance by range:")
    for bin_data in perf['performance_by_range']:
        if bin_data.get('mean_snr') is not None:
            error_str = ""
            if 'snr_error' in bin_data:
                error_str = f", error: {bin_data['snr_error']:+.1f} dB"

            print(f"  {bin_data['range_start_km']:5.1f}-{bin_data['range_end_km']:5.1f} km: "
                  f"SNR={bin_data['mean_snr']:5.1f} dB ± {bin_data['std_snr']:.1f} dB "
                  f"({bin_data['detection_count']} detections){error_str}")


def demo_track_statistics(radar):
    """Demonstrate track statistics."""
    print("\n" + "=" * 80)
    print("DEMONSTRATION 5: Track Statistics")
    print("=" * 80)

    print("\nGenerating aircraft track (180 seconds, 200 m/s, heading 90°)...")
    track = radar.generate_track(
        start_range=60e3,
        start_azimuth=45.0,
        velocity_ms=200.0,
        heading_deg=90.0,
        duration_s=180.0,
        dt=4.0
    )

    print(f"Generated track with {len(track)} updates\n")

    stats = analyze_track_statistics(track)

    print("Track Statistics:")
    print("-" * 80)
    print(f"Duration: {stats['duration']:.1f} seconds")
    print(f"Updates: {stats['num_updates']} (rate: {stats['update_rate']:.2f} Hz)")
    print(f"\nRange:")
    print(f"  Start: {stats['range_stats']['start'] / 1000.0:.1f} km")
    print(f"  End: {stats['range_stats']['end'] / 1000.0:.1f} km")
    print(f"  Change: {stats['range_stats']['change'] / 1000.0:.1f} km")
    print(f"  Rate: {stats['range_stats']['rate']:.1f} m/s")
    print(f"\nAzimuth:")
    print(f"  Start: {stats['azimuth_stats']['start']:.1f}°")
    print(f"  End: {stats['azimuth_stats']['end']:.1f}°")
    print(f"  Change: {stats['azimuth_stats']['change']:.1f}°")
    print(f"\nVelocity estimates:")
    print(f"  From position: {stats['velocity_estimate']:.1f} m/s ({stats['velocity_estimate'] * 1.944:.1f} knots)")
    print(f"  From Doppler: {stats['velocity_from_doppler']:.1f} m/s")
    print(f"  From range rate: {stats['velocity_from_range']:.1f} m/s")
    print(f"  Heading: {stats['heading_estimate']:.1f}°")
    print(f"\nDistance traveled: {stats['distance_traveled'] / 1000.0:.1f} km")
    print(f"\nSNR:")
    print(f"  Mean: {stats['snr_stats']['mean']:.1f} dB ± {stats['snr_stats']['std']:.1f} dB")
    print(f"  Min: {stats['snr_stats']['min']:.1f} dB")
    print(f"  Max: {stats['snr_stats']['max']:.1f} dB")


def demo_scenario_comparison(radar):
    """Demonstrate scenario comparison."""
    print("\n" + "=" * 80)
    print("DEMONSTRATION 6: Scenario Comparison")
    print("=" * 80)

    print("\nGenerating multiple scenarios...")

    # Scenario 1: Random static plots
    print("  1. Random static scenario (50 targets)")
    scenario1 = radar.generate_plots(num_targets=50, add_noise=True, add_clutter=False)

    # Scenario 2: Aircraft approach
    print("  2. Aircraft approach scenario (3 aircraft)")
    scenario2 = generate_approach_scenario(
        num_aircraft=3,
        radar_lat=radar.position.lat,
        radar_lon=radar.position.lon,
        radar_alt=radar.position.alt
    )

    # Scenario 3: Multi-aircraft cruise
    print("  3. Multi-aircraft cruise scenario (5 aircraft)")
    scenario3 = generate_aircraft_scenario(
        num_aircraft=5,
        duration=120.0,
        radar_lat=radar.position.lat,
        radar_lon=radar.position.lon,
        radar_alt=radar.position.alt
    )

    # Compare scenarios
    print("\nScenario Comparison:")
    print("-" * 80)
    comparison = compare_scenarios({
        'Random Static': scenario1,
        'Aircraft Approach': scenario2,
        'Multi-Aircraft Cruise': scenario3
    })

    print(comparison['summary'])

    print("\nDetailed comparison:")
    for name, stats in comparison['scenarios'].items():
        print(f"\n{name}:")
        print(f"  Detections: {stats['detection_count']}")
        print(f"  Mean range: {stats['mean_range_km']:.1f} km")
        print(f"  Mean SNR: {stats['mean_snr']:.1f} dB")
        print(f"  Azimuth coverage: {stats['azimuth_coverage']:.0f}°")
        print(f"  Spatial density: {stats['spatial_density']:.2f} detections/km²")


def main():
    parser = argparse.ArgumentParser(
        description='Radar Visualization Demonstration',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 visualization_demo.py              # Interactive mode
  python3 visualization_demo.py --save       # Save plots to files
  python3 visualization_demo.py --ascii-only # Skip matplotlib plots
        """
    )
    parser.add_argument(
        '--save', action='store_true',
        help='Save matplotlib plots to files instead of displaying'
    )
    parser.add_argument(
        '--ascii-only', action='store_true',
        help='Only run ASCII visualization (skip matplotlib)'
    )

    args = parser.parse_args()

    # Print header
    print("=" * 80)
    print("Radar Data Visualization & Analysis Demonstration")
    print("=" * 80)
    print()
    print("This demo showcases all visualization and analysis capabilities")
    print("for radar detection data in the ASTERIX integration project.")
    print()

    # Show available capabilities
    print_visualization_info()

    # Create radar sensor
    print("\nInitializing radar sensor...")
    radar = MockRadar(
        lat=52.5,              # Berlin, Germany
        lon=13.4,
        alt=100.0,             # 100 meters altitude
        max_range=250e3,       # 250 km max range
        name="DEMO_RADAR"
    )
    print(f"Radar: {radar.position.name}")
    print(f"Position: ({radar.position.lat}°N, {radar.position.lon}°E, {radar.position.alt}m)")
    print(f"Max range: {radar.max_range / 1000:.0f} km")

    # Run demonstrations
    try:
        # Always run ASCII demo
        demo_ascii_visualization(radar, save_mode=args.save)

        if not args.ascii_only:
            # Run matplotlib demos if available
            demo_matplotlib_visualizations(radar, save_mode=args.save)
            demo_track_visualization(radar, save_mode=args.save)

        # Run statistics demos
        demo_statistics(radar)
        demo_track_statistics(radar)
        demo_scenario_comparison(radar)

        # Final summary
        print("\n" + "=" * 80)
        print("DEMONSTRATION COMPLETE")
        print("=" * 80)
        print()
        print("Summary:")
        print("  ✓ ASCII visualization (always available)")
        if not args.ascii_only and check_matplotlib_available():
            print("  ✓ Matplotlib visualizations (polar plots, heatmaps, tracks)")
        print("  ✓ Statistical analysis (scenario stats, coverage, performance)")
        print("  ✓ Track analysis (velocity, heading, SNR variation)")
        print("  ✓ Scenario comparison")
        print()

        if args.save:
            print("Generated files:")
            files = [
                "radar_ascii_display.txt",
                "radar_ppi.png",
                "snr_heatmap.png",
                "elevation_profile.png",
                "radar_tracks.png"
            ]
            for f in files:
                if os.path.exists(f):
                    print(f"  ✓ {f}")

        print("\n✅ All demonstrations completed successfully!")

    except KeyboardInterrupt:
        print("\n\n⚠️  Demo interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n\n❌ Error during demonstration: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
