#!/usr/bin/env python3
"""
Radar Data Visualization Utilities

Provides visualization capabilities for radar detection data, supporting both
simple ASCII-based displays (no dependencies) and advanced matplotlib-based
plots when available.

Usage:
    from asterix_encoder.visualization import plot_radar_ascii, plot_range_azimuth
    from mock_radar import MockRadar

    # Generate some radar data
    radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
    plots = radar.generate_plots(num_targets=20)

    # ASCII visualization (always available)
    print(plot_radar_ascii(plots, radar_pos=(52.5, 13.4)))

    # Matplotlib visualization (if available)
    try:
        plot_range_azimuth(plots, filename="radar_display.png")
        plot_tracks([track1, track2], filename="tracks.png")
    except ImportError:
        print("Matplotlib not available")

---

License: GPL-3.0 (same as ASTERIX project)
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import math
from typing import List, Optional, Tuple
import sys

# Try to import matplotlib - optional dependency
try:
    import matplotlib.pyplot as plt
    import matplotlib.patches as patches
    from matplotlib.collections import LineCollection
    import numpy as np
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False
    np = None  # Will need numpy for some calculations


def _require_matplotlib(func):
    """Decorator to check matplotlib availability."""
    def wrapper(*args, **kwargs):
        if not HAS_MATPLOTLIB:
            raise ImportError(
                f"{func.__name__} requires matplotlib. "
                "Install with: pip install matplotlib"
            )
        return func(*args, **kwargs)
    return wrapper


def plot_radar_ascii(
    plots: List,
    radar_pos: Optional[Tuple[float, float]] = None,
    width: int = 80,
    height: int = 40,
    max_range_km: Optional[float] = None,
    show_legend: bool = True,
    show_stats: bool = True
) -> str:
    """
    Create ASCII art PPI (Plan Position Indicator) radar display.

    This function requires no external dependencies and creates a text-based
    representation of radar detections in polar coordinates. Perfect for
    terminal-based monitoring and debugging.

    Args:
        plots: List of RadarPlot objects to display
        radar_pos: Radar position (lat, lon) for header info, optional
        width: Display width in characters (default: 80)
        height: Display height in characters (default: 40)
        max_range_km: Maximum range to display in km (auto-detected if None)
        show_legend: Include legend for symbols
        show_stats: Show detection statistics

    Returns:
        String containing the ASCII radar display

    Example:
        >>> radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        >>> plots = radar.generate_plots(num_targets=10)
        >>> print(plot_radar_ascii(plots, (52.5, 13.4)))

                           N (0°)
                             |
                             *
                         *   |
                      *      |      *
                   *         |         *
                W (270°) ----R---- E (90°)
                             |
                             |
                           S (180°)

        Range rings: 50km, 100km, 150km
        * = Primary target (10 detections)
    """
    if not plots:
        return "No radar plots to display"

    # Calculate automatic range if not specified
    if max_range_km is None:
        max_range = max(p.range for p in plots)
        # Round up to nearest 50km
        max_range_km = math.ceil(max_range / 50000.0) * 50.0
    else:
        max_range = max_range_km * 1000.0

    # Initialize display grid
    grid = [[' ' for _ in range(width)] for _ in range(height)]

    # Calculate center position
    center_x = width // 2
    center_y = height // 2

    # Draw radar center
    grid[center_y][center_x] = 'R'

    # Draw cardinal directions and axes
    if center_y > 2:
        grid[2][center_x] = 'N'
        grid[1][center_x] = '|'
    if center_y < height - 3:
        grid[height - 3][center_x] = 'S'
        grid[height - 2][center_x] = '|'
    if center_x > 5:
        grid[center_y][5] = 'W'
        for x in range(6, center_x):
            grid[center_y][x] = '-'
    if center_x < width - 6:
        grid[center_y][width - 6] = 'E'
        for x in range(center_x + 1, width - 6):
            grid[center_y][x] = '-'

    # Draw range rings (circles)
    num_rings = 3
    ring_ranges = [max_range_km * (i + 1) / (num_rings + 1) for i in range(num_rings)]

    for ring_range_km in ring_ranges:
        ring_range = ring_range_km * 1000.0
        radius_chars = int((ring_range / max_range) * min(center_x - 10, center_y - 5))

        if radius_chars > 0:
            # Draw approximate circle with dots
            for angle_deg in range(0, 360, 15):
                angle_rad = math.radians(angle_deg)
                dx = int(radius_chars * math.sin(angle_rad))
                dy = int(radius_chars * math.cos(angle_rad))
                x = center_x + dx
                y = center_y - dy  # Invert Y for screen coordinates

                if 0 <= x < width and 0 <= y < height:
                    if grid[y][x] == ' ':
                        grid[y][x] = '·'

    # Plot radar detections
    high_snr_count = 0
    low_snr_count = 0

    for plot in plots:
        # Convert polar to Cartesian (screen coordinates)
        angle_rad = math.radians(plot.azimuth)
        norm_range = plot.range / max_range

        # Scale to screen
        radius_chars = norm_range * min(center_x - 10, center_y - 5)
        dx = int(radius_chars * math.sin(angle_rad))
        dy = int(radius_chars * math.cos(angle_rad))

        x = center_x + dx
        y = center_y - dy  # Invert Y

        # Check bounds
        if 0 <= x < width and 0 <= y < height:
            # Symbol based on SNR
            if plot.snr > 30:
                symbol = '*'
                high_snr_count += 1
            else:
                symbol = '+'
                low_snr_count += 1

            grid[y][x] = symbol

    # Convert grid to string
    lines = [''.join(row) for row in grid]
    output = '\n'.join(lines)

    # Add header
    header_lines = []
    header_lines.append("=" * width)
    if radar_pos:
        header_lines.append(f"Radar Position: {radar_pos[0]:.2f}°N, {radar_pos[1]:.2f}°E")
    header_lines.append(f"Max Range: {max_range_km:.0f} km ({len(plots)} detections)")
    header_lines.append("=" * width)

    # Add legend
    footer_lines = []
    if show_legend:
        footer_lines.append("")
        footer_lines.append("Legend:")
        footer_lines.append(f"  R = Radar position")
        footer_lines.append(f"  * = High SNR target (SNR > 30 dB): {high_snr_count}")
        footer_lines.append(f"  + = Low SNR target (SNR ≤ 30 dB): {low_snr_count}")
        footer_lines.append(f"  · = Range ring ({', '.join(f'{r:.0f}km' for r in ring_ranges)})")

    # Add statistics
    if show_stats and plots:
        footer_lines.append("")
        footer_lines.append("Statistics:")
        avg_range = sum(p.range for p in plots) / len(plots)
        avg_snr = sum(p.snr for p in plots) / len(plots)
        max_snr = max(p.snr for p in plots)
        min_snr = min(p.snr for p in plots)

        footer_lines.append(f"  Average range: {avg_range / 1000:.1f} km ({avg_range / 1852:.1f} NM)")
        footer_lines.append(f"  SNR: avg={avg_snr:.1f} dB, min={min_snr:.1f} dB, max={max_snr:.1f} dB")
        footer_lines.append(f"  Azimuth coverage: {min(p.azimuth for p in plots):.0f}° - {max(p.azimuth for p in plots):.0f}°")

    return '\n'.join(header_lines + [output] + footer_lines)


@_require_matplotlib
def plot_range_azimuth(
    plots: List,
    filename: Optional[str] = None,
    title: str = "Radar PPI Display",
    max_range_km: Optional[float] = None,
    show_snr: bool = True,
    figsize: Tuple[float, float] = (10, 10)
):
    """
    Create polar plot of radar detections (PPI display).

    Requires matplotlib. Creates a professional-looking radar display with
    color-coded SNR values and range rings.

    Args:
        plots: List of RadarPlot objects
        filename: Output filename (PNG, PDF, SVG). If None, display interactively.
        title: Plot title
        max_range_km: Maximum range to display (auto if None)
        show_snr: Color-code detections by SNR
        figsize: Figure size in inches (width, height)

    Example:
        >>> plots = radar.generate_plots(num_targets=50)
        >>> plot_range_azimuth(plots, "radar_ppi.png", max_range_km=200)
    """
    if not plots:
        print("No plots to display")
        return

    fig = plt.figure(figsize=figsize)
    ax = fig.add_subplot(111, projection='polar')

    # Convert to numpy arrays
    azimuths = np.array([np.radians(p.azimuth) for p in plots])
    ranges = np.array([p.range / 1000.0 for p in plots])  # Convert to km
    snrs = np.array([p.snr for p in plots])

    # Set north at top, clockwise
    ax.set_theta_zero_location('N')
    ax.set_theta_direction(-1)

    # Plot detections
    if show_snr:
        scatter = ax.scatter(
            azimuths, ranges, c=snrs, s=50,
            cmap='viridis', alpha=0.7, edgecolors='black', linewidths=0.5
        )
        cbar = plt.colorbar(scatter, ax=ax, pad=0.1)
        cbar.set_label('SNR (dB)', rotation=270, labelpad=15)
    else:
        ax.scatter(azimuths, ranges, s=50, alpha=0.7, color='blue', edgecolors='black', linewidths=0.5)

    # Configure range axis
    if max_range_km:
        ax.set_ylim(0, max_range_km)

    ax.set_xlabel('Range (km)')
    ax.set_title(title, pad=20)
    ax.grid(True, alpha=0.3)

    plt.tight_layout()

    if filename:
        plt.savefig(filename, dpi=150, bbox_inches='tight')
        print(f"Saved: {filename}")
    else:
        plt.show()

    plt.close(fig)


@_require_matplotlib
def plot_tracks(
    tracks: List[List],
    filename: Optional[str] = None,
    title: str = "Radar Tracks",
    max_range_km: Optional[float] = None,
    show_time_evolution: bool = True,
    figsize: Tuple[float, float] = (10, 10)
):
    """
    Plot multiple target trajectories over time.

    Requires matplotlib. Displays track history with optional time-based
    color coding to show temporal evolution.

    Args:
        tracks: List of tracks, where each track is a list of RadarPlot objects
        filename: Output filename (PNG, PDF, SVG). If None, display interactively.
        title: Plot title
        max_range_km: Maximum range to display (auto if None)
        show_time_evolution: Color-code tracks by time
        figsize: Figure size in inches (width, height)

    Example:
        >>> track1 = radar.generate_track(50e3, 45.0, 200.0, 90.0, 120.0)
        >>> track2 = radar.generate_track(80e3, 180.0, 180.0, 270.0, 120.0)
        >>> plot_tracks([track1, track2], "tracks.png")
    """
    if not tracks or all(len(t) == 0 for t in tracks):
        print("No tracks to display")
        return

    fig = plt.figure(figsize=figsize)
    ax = fig.add_subplot(111, projection='polar')

    # Set north at top, clockwise
    ax.set_theta_zero_location('N')
    ax.set_theta_direction(-1)

    # Plot each track
    colors = plt.cm.tab10(np.linspace(0, 1, len(tracks)))

    for track_idx, track in enumerate(tracks):
        if not track:
            continue

        azimuths = np.array([np.radians(p.azimuth) for p in track])
        ranges = np.array([p.range / 1000.0 for p in track])

        if show_time_evolution:
            # Create color gradient along track
            points = np.array([azimuths, ranges]).T.reshape(-1, 1, 2)
            segments = np.concatenate([points[:-1], points[1:]], axis=1)

            # Time-based coloring
            time_norm = np.linspace(0, 1, len(track) - 1)
            lc = LineCollection(segments, cmap='coolwarm', alpha=0.7)
            lc.set_array(time_norm)
            lc.set_linewidth(2)
            ax.add_collection(lc)

            # Mark start and end
            ax.plot(azimuths[0], ranges[0], 'go', markersize=8, label=f'Track {track_idx + 1} start')
            ax.plot(azimuths[-1], ranges[-1], 'ro', markersize=8, label=f'Track {track_idx + 1} end')
        else:
            ax.plot(azimuths, ranges, '-o', color=colors[track_idx],
                   alpha=0.7, linewidth=2, markersize=4, label=f'Track {track_idx + 1}')

    # Configure range axis
    if max_range_km:
        ax.set_ylim(0, max_range_km)

    ax.set_xlabel('Range (km)')
    ax.set_title(title, pad=20)
    ax.grid(True, alpha=0.3)
    ax.legend(loc='upper right', bbox_to_anchor=(1.3, 1.0))

    plt.tight_layout()

    if filename:
        plt.savefig(filename, dpi=150, bbox_inches='tight')
        print(f"Saved: {filename}")
    else:
        plt.show()

    plt.close(fig)


@_require_matplotlib
def plot_snr_heatmap(
    plots: List,
    filename: Optional[str] = None,
    title: str = "SNR Heatmap (Range-Azimuth)",
    max_range_km: Optional[float] = None,
    grid_size: Tuple[int, int] = (50, 72),  # (range bins, azimuth bins)
    figsize: Tuple[float, float] = (12, 8)
):
    """
    Create range-azimuth SNR heatmap.

    Requires matplotlib. Displays average SNR values across range and azimuth
    bins, useful for analyzing radar coverage and detection performance.

    Args:
        plots: List of RadarPlot objects
        filename: Output filename (PNG, PDF, SVG). If None, display interactively.
        title: Plot title
        max_range_km: Maximum range to display (auto if None)
        grid_size: (range_bins, azimuth_bins) for binning
        figsize: Figure size in inches (width, height)

    Example:
        >>> plots = radar.generate_plots(num_targets=200)
        >>> plot_snr_heatmap(plots, "snr_heatmap.png", grid_size=(40, 72))
    """
    if not plots:
        print("No plots to display")
        return

    # Determine range extent
    if max_range_km is None:
        max_range_km = max(p.range for p in plots) / 1000.0

    range_bins, azimuth_bins = grid_size

    # Create grid for SNR accumulation
    snr_grid = np.zeros((range_bins, azimuth_bins))
    count_grid = np.zeros((range_bins, azimuth_bins))

    # Bin the data
    for plot in plots:
        range_km = plot.range / 1000.0
        if range_km > max_range_km:
            continue

        range_idx = int((range_km / max_range_km) * (range_bins - 1))
        azimuth_idx = int((plot.azimuth / 360.0) * (azimuth_bins - 1))

        snr_grid[range_idx, azimuth_idx] += plot.snr
        count_grid[range_idx, azimuth_idx] += 1

    # Compute average SNR (avoid division by zero)
    with np.errstate(divide='ignore', invalid='ignore'):
        avg_snr = np.divide(snr_grid, count_grid)
        avg_snr[~np.isfinite(avg_snr)] = 0  # Replace NaN/Inf with 0

    # Create polar heatmap
    fig = plt.figure(figsize=figsize)
    ax = fig.add_subplot(111, projection='polar')

    # Set north at top, clockwise
    ax.set_theta_zero_location('N')
    ax.set_theta_direction(-1)

    # Create mesh grid
    azimuth_edges = np.linspace(0, 2 * np.pi, azimuth_bins + 1)
    range_edges = np.linspace(0, max_range_km, range_bins + 1)

    azimuth_centers = (azimuth_edges[:-1] + azimuth_edges[1:]) / 2
    range_centers = (range_edges[:-1] + range_edges[1:]) / 2

    # Create 2D mesh
    THETA, R = np.meshgrid(azimuth_centers, range_centers)

    # Plot heatmap
    pcm = ax.pcolormesh(THETA, R, avg_snr, cmap='jet', shading='auto', vmin=0, vmax=50)
    cbar = plt.colorbar(pcm, ax=ax, pad=0.1)
    cbar.set_label('Average SNR (dB)', rotation=270, labelpad=15)

    ax.set_ylim(0, max_range_km)
    ax.set_xlabel('Range (km)')
    ax.set_title(title, pad=20)
    ax.grid(True, alpha=0.3, color='white')

    plt.tight_layout()

    if filename:
        plt.savefig(filename, dpi=150, bbox_inches='tight')
        print(f"Saved: {filename}")
    else:
        plt.show()

    plt.close(fig)


@_require_matplotlib
def plot_elevation_profile(
    plots: List,
    filename: Optional[str] = None,
    title: str = "Elevation vs Range Profile",
    max_range_km: Optional[float] = None,
    figsize: Tuple[float, float] = (12, 6)
):
    """
    Plot elevation angle vs range scatter plot.

    Requires matplotlib. Useful for analyzing radar coverage in vertical plane
    and identifying ground clutter.

    Args:
        plots: List of RadarPlot objects
        filename: Output filename (PNG, PDF, SVG). If None, display interactively.
        title: Plot title
        max_range_km: Maximum range to display (auto if None)
        figsize: Figure size in inches (width, height)

    Example:
        >>> plots = radar.generate_plots(num_targets=100)
        >>> plot_elevation_profile(plots, "elevation.png")
    """
    if not plots:
        print("No plots to display")
        return

    fig, ax = plt.subplots(figsize=figsize)

    ranges = np.array([p.range / 1000.0 for p in plots])
    elevations = np.array([p.elevation for p in plots])
    snrs = np.array([p.snr for p in plots])

    scatter = ax.scatter(ranges, elevations, c=snrs, s=50, cmap='viridis',
                        alpha=0.6, edgecolors='black', linewidths=0.5)

    cbar = plt.colorbar(scatter, ax=ax)
    cbar.set_label('SNR (dB)', rotation=270, labelpad=15)

    if max_range_km:
        ax.set_xlim(0, max_range_km)

    ax.set_xlabel('Range (km)')
    ax.set_ylabel('Elevation (degrees)')
    ax.set_title(title)
    ax.grid(True, alpha=0.3)
    ax.axhline(0, color='red', linestyle='--', linewidth=1, alpha=0.5, label='Horizon')
    ax.legend()

    plt.tight_layout()

    if filename:
        plt.savefig(filename, dpi=150, bbox_inches='tight')
        print(f"Saved: {filename}")
    else:
        plt.show()

    plt.close(fig)


def check_matplotlib_available() -> bool:
    """
    Check if matplotlib is available for advanced visualizations.

    Returns:
        True if matplotlib is installed, False otherwise
    """
    return HAS_MATPLOTLIB


def print_visualization_info():
    """
    Print information about available visualization capabilities.
    """
    print("=" * 60)
    print("Radar Visualization Capabilities")
    print("=" * 60)
    print()
    print("Core visualizations (no dependencies):")
    print("  ✓ plot_radar_ascii() - ASCII PPI display")
    print()

    if HAS_MATPLOTLIB:
        print("Advanced visualizations (matplotlib available):")
        print("  ✓ plot_range_azimuth() - Polar radar display")
        print("  ✓ plot_tracks() - Track trajectories")
        print("  ✓ plot_snr_heatmap() - SNR coverage heatmap")
        print("  ✓ plot_elevation_profile() - Elevation vs range")
    else:
        print("Advanced visualizations (matplotlib not available):")
        print("  ✗ Install matplotlib for additional visualizations")
        print("    pip install matplotlib")

    print()
    print("=" * 60)


if __name__ == "__main__":
    # Simple test/demo
    print_visualization_info()
    print()

    # Create minimal test data
    try:
        # Try to import mock radar
        import sys
        import os
        sys.path.insert(0, os.path.dirname(__file__))
        from mock_radar import MockRadar

        print("Generating test radar data...")
        radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        plots = radar.generate_plots(num_targets=20, add_noise=True)

        print()
        print(plot_radar_ascii(plots, (52.5, 13.4), width=80, height=35))

        if HAS_MATPLOTLIB:
            print("\nGenerating matplotlib plots...")
            plot_range_azimuth(plots, "test_ppi.png")
            print("\n✓ Visualization test complete!")
        else:
            print("\n✓ ASCII visualization test complete!")

    except ImportError as e:
        print(f"Test data generation failed: {e}")
        print("This is expected if mock_radar.py is not in the same directory")
