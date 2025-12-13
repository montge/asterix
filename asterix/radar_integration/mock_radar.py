#!/usr/bin/env python3
"""
Mock Radar Data Generator for ASTERIX Integration Testing

This module generates synthetic radar plot data compatible with what a real
radar simulator (like RadarSimPy) would output. Used for testing ASTERIX
encoder/decoder without requiring proprietary radar simulation software.

Usage:
    from mock_radar import MockRadar, generate_aircraft_scenario

    # Generate simple scenario
    radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
    plots = radar.generate_plots(num_targets=10)

    # Generate realistic aircraft scenario
    plots = generate_aircraft_scenario(num_aircraft=5, duration=60.0)

---

License: GPL-3.0 (same as ASTERIX project)
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import numpy as np
from dataclasses import dataclass, field
from typing import List, Optional
import time


@dataclass
class RadarPlot:
    """
    Represents a single radar detection plot.

    Attributes:
        range: Slant range in meters (distance from radar to target)
        azimuth: Azimuth angle in degrees (0-360, 0=North, clockwise)
        elevation: Elevation angle in degrees (-90 to +90, optional)
        rcs: Radar Cross Section in dBsm (decibels per square meter)
        snr: Signal-to-Noise Ratio in dB
        doppler: Doppler frequency shift in Hz (positive = approaching)
        timestamp: Unix timestamp (seconds since epoch)
        amplitude: Received signal amplitude (arbitrary units, optional)
        quality: Detection quality indicator (0.0-1.0, optional)
    """
    range: float
    azimuth: float
    elevation: float = 0.0
    rcs: float = 0.0
    snr: float = 20.0
    doppler: float = 0.0
    timestamp: float = field(default_factory=time.time)
    amplitude: float = 1.0
    quality: float = 1.0

    def to_dict(self):
        """Convert to dictionary for JSON serialization."""
        return {
            'range': self.range,
            'azimuth': self.azimuth,
            'elevation': self.elevation,
            'rcs': self.rcs,
            'snr': self.snr,
            'doppler': self.doppler,
            'timestamp': self.timestamp,
            'amplitude': self.amplitude,
            'quality': self.quality,
        }


@dataclass
class RadarPosition:
    """
    Radar sensor position in WGS84 coordinates.

    Attributes:
        lat: Latitude in degrees (-90 to +90)
        lon: Longitude in degrees (-180 to +180)
        alt: Altitude in meters above sea level
        name: Radar site identifier
    """
    lat: float
    lon: float
    alt: float
    name: str = "RADAR_01"


class MockRadar:
    """
    Mock radar sensor for generating synthetic detection data.

    Simulates a rotating surveillance radar (e.g., primary or secondary radar)
    with configurable parameters and realistic noise characteristics.

    Attributes:
        position: Radar geographic location (WGS84)
        max_range: Maximum detection range in meters
        min_range: Minimum detection range in meters
        azimuth_resolution: Azimuth resolution in degrees
        range_resolution: Range resolution in meters
        update_rate: Radar update rate in Hz
        noise_std: Standard deviation for measurement noise
    """

    def __init__(
        self,
        lat: float,
        lon: float,
        alt: float,
        max_range: float = 200e3,  # 200 km
        min_range: float = 1e3,     # 1 km
        azimuth_resolution: float = 1.0,  # 1 degree
        range_resolution: float = 100.0,  # 100 meters
        update_rate: float = 0.25,  # 4 second rotation
        noise_std: float = 0.05,
        name: str = "RADAR_01"
    ):
        """
        Initialize mock radar sensor.

        Args:
            lat: Radar latitude in degrees
            lon: Radar longitude in degrees
            alt: Radar altitude in meters
            max_range: Maximum detection range (meters)
            min_range: Minimum detection range (meters)
            azimuth_resolution: Azimuth measurement resolution (degrees)
            range_resolution: Range measurement resolution (meters)
            update_rate: Radar scan rate (Hz)
            noise_std: Measurement noise standard deviation (fraction)
            name: Radar identifier
        """
        self.position = RadarPosition(lat, lon, alt, name)
        self.max_range = max_range
        self.min_range = min_range
        self.azimuth_resolution = azimuth_resolution
        self.range_resolution = range_resolution
        self.update_rate = update_rate
        self.noise_std = noise_std
        self._rng = np.random.default_rng()

    def generate_plots(
        self,
        num_targets: int = 10,
        timestamp: Optional[float] = None,
        add_noise: bool = True,
        add_clutter: bool = False
    ) -> List[RadarPlot]:
        """
        Generate synthetic radar plots for multiple targets.

        Args:
            num_targets: Number of target detections to generate
            timestamp: Unix timestamp (default: current time)
            add_noise: Add realistic measurement noise
            add_clutter: Add clutter/false alarms

        Returns:
            List of RadarPlot objects
        """
        if timestamp is None:
            timestamp = time.time()

        plots = []

        for i in range(num_targets):
            # Generate random target parameters
            range_m = self._rng.uniform(self.min_range, self.max_range)
            azimuth_deg = self._rng.uniform(0, 360)
            elevation_deg = self._rng.uniform(-2, 15)  # Typical aircraft elevations

            # RCS varies by target type (small aircraft ~0 dBsm, large ~20 dBsm)
            rcs_dBsm = self._rng.uniform(-5, 25)

            # SNR decreases with range (simplified radar equation)
            # SNR ∝ 1/R^4 for monostatic radar
            range_km = range_m / 1000.0
            snr_base = 40.0  # SNR at 10km reference
            snr_dB = snr_base - 40 * np.log10(range_km / 10.0) + self._rng.normal(0, 3)
            snr_dB = max(10.0, min(50.0, snr_dB))  # Clip to realistic range

            # Doppler: typical aircraft speeds -500 to +500 Hz @ 1.3 GHz (L-band)
            doppler_Hz = self._rng.uniform(-500, 500)

            # Add measurement noise if requested
            if add_noise:
                range_m += self._rng.normal(0, self.range_resolution * self.noise_std)
                azimuth_deg += self._rng.normal(0, self.azimuth_resolution * self.noise_std)
                elevation_deg += self._rng.normal(0, 0.1 * self.noise_std)

            # Quantize to sensor resolution
            range_m = round(range_m / self.range_resolution) * self.range_resolution
            azimuth_deg = round(azimuth_deg / self.azimuth_resolution) * self.azimuth_resolution
            azimuth_deg = azimuth_deg % 360.0  # Wrap to 0-360

            # Quality indicator (0.0-1.0, higher SNR = higher quality)
            quality = min(1.0, snr_dB / 40.0)

            plot = RadarPlot(
                range=range_m,
                azimuth=azimuth_deg,
                elevation=elevation_deg,
                rcs=rcs_dBsm,
                snr=snr_dB,
                doppler=doppler_Hz,
                timestamp=timestamp,
                amplitude=10.0 ** (snr_dB / 20.0),  # Convert dB to linear
                quality=quality
            )
            plots.append(plot)

        # Add clutter if requested (false alarms, ground clutter)
        if add_clutter:
            num_clutter = self._rng.integers(0, num_targets // 3 + 1)
            for _ in range(num_clutter):
                clutter_plot = RadarPlot(
                    range=self._rng.uniform(self.min_range, self.max_range),
                    azimuth=self._rng.uniform(0, 360),
                    elevation=self._rng.uniform(-5, 2),  # Low elevation (ground clutter)
                    rcs=self._rng.uniform(-15, 5),  # Lower RCS
                    snr=self._rng.uniform(10, 20),  # Lower SNR
                    doppler=self._rng.uniform(-50, 50),  # Near-zero Doppler
                    timestamp=timestamp,
                    quality=self._rng.uniform(0.1, 0.5)  # Low quality
                )
                plots.append(clutter_plot)

        return plots

    def generate_track(
        self,
        start_range: float,
        start_azimuth: float,
        velocity_ms: float,
        heading_deg: float,
        duration_s: float,
        dt: float = 4.0
    ) -> List[RadarPlot]:
        """
        Generate a series of radar plots for a single moving target (track).

        Args:
            start_range: Initial range in meters
            start_azimuth: Initial azimuth in degrees
            velocity_ms: Target velocity in m/s
            heading_deg: Target heading in degrees (0=North)
            duration_s: Tracking duration in seconds
            dt: Time between radar updates in seconds

        Returns:
            List of RadarPlot objects forming a track
        """
        plots = []
        timestamp = time.time()

        # Initial position in local Cartesian coordinates
        x = start_range * np.sin(np.radians(start_azimuth))
        y = start_range * np.cos(np.radians(start_azimuth))

        # Velocity components
        vx = velocity_ms * np.sin(np.radians(heading_deg))
        vy = velocity_ms * np.cos(np.radians(heading_deg))

        num_updates = int(duration_s / dt)

        for i in range(num_updates):
            # Update position
            x += vx * dt
            y += vy * dt

            # Convert back to polar
            range_m = np.sqrt(x**2 + y**2)
            azimuth_deg = np.degrees(np.arctan2(x, y)) % 360

            # Check if still in detection range
            if range_m < self.min_range or range_m > self.max_range:
                break

            # Calculate Doppler (radial velocity component)
            # Doppler = (2 * v_radial * f_carrier) / c
            # For L-band (1.3 GHz): Doppler ≈ v_radial * 8.67 Hz/(m/s)
            range_rate = (vx * x + vy * y) / range_m  # Radial velocity
            doppler_Hz = range_rate * 8.67

            plot = RadarPlot(
                range=range_m,
                azimuth=azimuth_deg,
                elevation=self._rng.uniform(2, 10),  # Cruising altitude
                rcs=15.0,  # Typical commercial aircraft
                snr=40.0 - 40 * np.log10(range_m / 10000.0),
                doppler=doppler_Hz,
                timestamp=timestamp + i * dt,
                quality=0.95
            )
            plots.append(plot)

        return plots


def generate_aircraft_scenario(
    num_aircraft: int = 5,
    duration: float = 60.0,
    radar_lat: float = 52.5,
    radar_lon: float = 13.4,
    radar_alt: float = 100.0
) -> List[RadarPlot]:
    """
    Generate a realistic aircraft surveillance scenario.

    Simulates multiple aircraft at different ranges, speeds, and headings
    over a specified duration.

    Args:
        num_aircraft: Number of aircraft to simulate
        duration: Scenario duration in seconds
        radar_lat: Radar latitude (degrees)
        radar_lon: Radar longitude (degrees)
        radar_alt: Radar altitude (meters)

    Returns:
        List of radar plots from all aircraft
    """
    radar = MockRadar(radar_lat, radar_lon, radar_alt)
    all_plots = []

    for i in range(num_aircraft):
        # Random initial conditions
        start_range = np.random.uniform(20e3, 150e3)  # 20-150 km
        start_azimuth = np.random.uniform(0, 360)
        velocity = np.random.uniform(150, 250)  # 150-250 m/s (typical cruise)
        heading = np.random.uniform(0, 360)

        # Generate track
        track = radar.generate_track(
            start_range=start_range,
            start_azimuth=start_azimuth,
            velocity_ms=velocity,
            heading_deg=heading,
            duration_s=duration,
            dt=4.0  # 4 second radar rotation
        )
        all_plots.extend(track)

    return all_plots


def generate_approach_scenario(
    num_aircraft: int = 3,
    radar_lat: float = 52.5,
    radar_lon: float = 13.4,
    radar_alt: float = 100.0
) -> List[RadarPlot]:
    """
    Generate aircraft approach scenario (landing pattern).

    Simulates aircraft approaching the radar location (airport) from
    different directions at decreasing altitudes.

    Args:
        num_aircraft: Number of approaching aircraft
        radar_lat: Radar latitude (degrees)
        radar_lon: Radar longitude (degrees)
        radar_alt: Radar altitude (meters)

    Returns:
        List of radar plots showing approach trajectories
    """
    radar = MockRadar(radar_lat, radar_lon, radar_alt)
    all_plots = []

    for i in range(num_aircraft):
        # Start at different distances/azimuths
        start_range = 50e3 + i * 20e3  # Staggered distances
        start_azimuth = i * (360 / num_aircraft)  # Evenly distributed
        velocity = 120  # m/s (approach speed)
        heading = (start_azimuth + 180) % 360  # Heading toward radar

        track = radar.generate_track(
            start_range=start_range,
            start_azimuth=start_azimuth,
            velocity_ms=velocity,
            heading_deg=heading,
            duration_s=300.0,  # 5 minutes
            dt=4.0
        )
        all_plots.extend(track)

    return all_plots


if __name__ == "__main__":
    # Example usage and testing
    print("Mock Radar Data Generator - Examples\n")

    # Example 1: Simple random plots
    print("1. Generating 10 random radar plots...")
    radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
    plots = radar.generate_plots(num_targets=10, add_noise=True, add_clutter=True)
    print(f"   Generated {len(plots)} plots")
    print(f"   Sample plot: Range={plots[0].range:.0f}m, "
          f"Az={plots[0].azimuth:.1f}°, SNR={plots[0].snr:.1f}dB\n")

    # Example 2: Single aircraft track
    print("2. Generating single aircraft track (60 seconds)...")
    track = radar.generate_track(
        start_range=50e3,
        start_azimuth=45.0,
        velocity_ms=200.0,
        heading_deg=90.0,
        duration_s=60.0
    )
    print(f"   Generated {len(track)} plot updates")
    print(f"   Start: Range={track[0].range:.0f}m, Az={track[0].azimuth:.1f}°")
    print(f"   End:   Range={track[-1].range:.0f}m, Az={track[-1].azimuth:.1f}°\n")

    # Example 3: Multi-aircraft scenario
    print("3. Generating multi-aircraft scenario (5 aircraft, 60 seconds)...")
    scenario_plots = generate_aircraft_scenario(num_aircraft=5, duration=60.0)
    print(f"   Generated {len(scenario_plots)} total plots")
    print(f"   Average: {len(scenario_plots) / 5:.1f} plots per aircraft\n")

    # Example 4: Approach scenario
    print("4. Generating approach scenario (3 aircraft landing)...")
    approach_plots = generate_approach_scenario(num_aircraft=3)
    print(f"   Generated {len(approach_plots)} total plots")
    print(f"   First aircraft: {len([p for p in approach_plots if p.azimuth < 120])} plots")

    print("\n✅ Mock radar generator working correctly!")
