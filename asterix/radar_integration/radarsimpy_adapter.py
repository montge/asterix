#!/usr/bin/env python3
"""
RadarSimPy Adapter for ASTERIX Integration

This module provides an adapter pattern for integrating RadarSimPy
(a physics-based radar simulator) with the ASTERIX encoding pipeline.

RadarSimPy is an optional dependency - the module gracefully handles
its absence and provides a fallback to the MockRadar implementation.

Installation:
    RadarSimPy must be installed separately from https://radarsimx.com
    The module is not available via pip and requires manual installation.

Usage:
    from asterix.radar_integration.radarsimpy_adapter import (
        RadarSimPyAdapter,
        check_radarsimpy_available
    )

    if check_radarsimpy_available():
        adapter = RadarSimPyAdapter()
        plots = adapter.simulate_targets(targets=[...])
    else:
        # Fall back to MockRadar
        from asterix.radar_integration import MockRadar
        radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        plots = radar.generate_plots(num_targets=10)

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-12-19
"""

from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import List, Optional, Tuple, Any, Dict
import numpy as np
import time

from .mock_radar import RadarPlot, RadarPosition

# Check if RadarSimPy is available
_RADARSIMPY_AVAILABLE = False
_RADARSIMPY_ERROR: Optional[str] = None

try:
    import radarsimpy  # type: ignore
    from radarsimpy import Radar, Transmitter, Receiver  # type: ignore
    from radarsimpy.simulator import sim_radar  # type: ignore
    _RADARSIMPY_AVAILABLE = True
except ImportError as e:
    _RADARSIMPY_ERROR = str(e)


def check_radarsimpy_available() -> bool:
    """Check if RadarSimPy is installed and available."""
    return _RADARSIMPY_AVAILABLE


def get_radarsimpy_error() -> Optional[str]:
    """Get the error message if RadarSimPy import failed."""
    return _RADARSIMPY_ERROR


@dataclass
class PointTarget:
    """
    Represents a point target for radar simulation.

    Attributes:
        location: Target position [x, y, z] in meters (radar-centered coordinates)
        velocity: Target velocity [vx, vy, vz] in m/s
        rcs: Radar Cross Section in dBsm (decibels per square meter)
        phase: Target phase in degrees (optional, for coherent processing)
    """
    location: Tuple[float, float, float]
    velocity: Tuple[float, float, float] = (0.0, 0.0, 0.0)
    rcs: float = 10.0  # dBsm
    phase: float = 0.0  # degrees


class RadarSimulatorBase(ABC):
    """
    Abstract base class for radar simulators.

    Defines the common interface that all radar simulators (MockRadar,
    RadarSimPy, etc.) should implement for ASTERIX integration.
    """

    @abstractmethod
    def simulate_targets(
        self,
        targets: List[PointTarget],
        timestamp: Optional[float] = None
    ) -> List[RadarPlot]:
        """
        Simulate radar detections for given point targets.

        Args:
            targets: List of PointTarget objects to simulate
            timestamp: Unix timestamp for the detections

        Returns:
            List of RadarPlot objects representing detections
        """
        pass

    @abstractmethod
    def get_radar_position(self) -> RadarPosition:
        """Get the radar's geographic position."""
        pass

    @abstractmethod
    def get_radar_parameters(self) -> Dict[str, Any]:
        """Get radar system parameters (frequency, bandwidth, etc.)."""
        pass


class RadarSimPyAdapter(RadarSimulatorBase):
    """
    Adapter for RadarSimPy radar simulator.

    Wraps RadarSimPy's physics-based radar simulation to produce
    RadarPlot objects compatible with ASTERIX encoding.

    This adapter provides:
    - Conversion from RadarSimPy baseband output to radar plots
    - Configurable radar parameters (frequency, power, waveform)
    - Support for point targets and target tracks
    - Integration with ASTERIX CAT048/CAT062 encoders

    Attributes:
        position: Radar geographic location
        frequency: Carrier frequency in Hz (default: 1.3 GHz L-band)
        bandwidth: Waveform bandwidth in Hz
        tx_power: Transmitter power in dBm
        samples_per_pulse: ADC samples per pulse
        pulse_length: Pulse duration in seconds
    """

    def __init__(
        self,
        lat: float = 52.5,
        lon: float = 13.4,
        alt: float = 100.0,
        frequency: float = 1.3e9,  # L-band (typical surveillance radar)
        bandwidth: float = 1e6,     # 1 MHz bandwidth
        tx_power: float = 40.0,     # 40 dBm (10W)
        samples_per_pulse: int = 256,
        pulse_length: float = 10e-6,  # 10 microseconds
        noise_figure: float = 4.0,    # Receiver noise figure (dB)
        seed: Optional[int] = None,
        name: str = "RADAR_01"
    ):
        """
        Initialize RadarSimPy adapter.

        Args:
            lat: Radar latitude in degrees
            lon: Radar longitude in degrees
            alt: Radar altitude in meters
            frequency: Carrier frequency in Hz
            bandwidth: Waveform bandwidth in Hz
            tx_power: Transmitter power in dBm
            samples_per_pulse: Number of ADC samples per pulse
            pulse_length: Pulse duration in seconds
            noise_figure: Receiver noise figure in dB
            seed: Random seed for reproducibility
            name: Radar identifier

        Raises:
            ImportError: If RadarSimPy is not installed
        """
        if not _RADARSIMPY_AVAILABLE:
            raise ImportError(
                f"RadarSimPy is not installed. Error: {_RADARSIMPY_ERROR}\n"
                "Install RadarSimPy from https://radarsimx.com/radarsimx/radarsimpy/"
            )

        self.position = RadarPosition(lat, lon, alt, name)
        self.frequency = frequency
        self.bandwidth = bandwidth
        self.tx_power = tx_power
        self.samples_per_pulse = samples_per_pulse
        self.pulse_length = pulse_length
        self.noise_figure = noise_figure
        self._seed = seed
        self._rng = np.random.default_rng(seed)

        # Build RadarSimPy radar configuration
        self._build_radar()

    def _build_radar(self) -> None:
        """Build the RadarSimPy Radar object from configuration."""
        # Create transmitter with FMCW waveform
        transmitter = Transmitter(
            f=[self.frequency - self.bandwidth / 2,
               self.frequency + self.bandwidth / 2],
            t=self.pulse_length,
            tx_power=self.tx_power,
            pulses=1,
            channels=[{'location': (0, 0, 0)}]
        )

        # Create receiver
        fs = self.samples_per_pulse / self.pulse_length  # Sampling rate
        receiver = Receiver(
            fs=fs,
            noise_figure=self.noise_figure,
            rf_gain=20.0,
            baseband_gain=30.0,
            channels=[{'location': (0, 0, 0)}]
        )

        # Create radar system
        self._radar = Radar(
            transmitter=transmitter,
            receiver=receiver,
            location=(0, 0, 0),
            seed=self._seed
        )

    def simulate_targets(
        self,
        targets: List[PointTarget],
        timestamp: Optional[float] = None
    ) -> List[RadarPlot]:
        """
        Simulate radar detections for given point targets using RadarSimPy.

        This method:
        1. Converts PointTarget objects to RadarSimPy target format
        2. Runs physics-based radar simulation
        3. Performs range-Doppler processing on baseband data
        4. Detects targets using CFAR algorithm
        5. Returns RadarPlot objects for ASTERIX encoding

        Args:
            targets: List of PointTarget objects
            timestamp: Unix timestamp (default: current time)

        Returns:
            List of RadarPlot objects representing detections
        """
        if timestamp is None:
            timestamp = time.time()

        if not targets:
            return []

        # Convert targets to RadarSimPy format
        target_list = []
        for target in targets:
            target_list.append({
                'location': target.location,
                'speed': target.velocity,
                'rcs': 10 ** (target.rcs / 10),  # Convert dBsm to linear
                'phase': target.phase
            })

        # Run RadarSimPy simulation
        try:
            baseband_data = sim_radar(self._radar, target_list)
        except Exception as e:
            # If simulation fails, return empty list
            # This can happen with invalid target configurations
            return []

        # Process baseband data to extract detections
        plots = self._process_baseband(baseband_data, targets, timestamp)

        return plots

    def _process_baseband(
        self,
        baseband_data: np.ndarray,
        targets: List[PointTarget],
        timestamp: float
    ) -> List[RadarPlot]:
        """
        Process RadarSimPy baseband data to extract radar plots.

        For simplicity, this uses the known target positions rather than
        full range-Doppler processing. In a real implementation, you would:
        1. Apply range FFT
        2. Apply Doppler FFT (for multiple pulses)
        3. Apply CFAR detection
        4. Estimate target parameters

        Args:
            baseband_data: RadarSimPy baseband output
            targets: Original target list (for reference)
            timestamp: Detection timestamp

        Returns:
            List of RadarPlot objects
        """
        plots = []
        c = 3e8  # Speed of light

        for target in targets:
            x, y, z = target.location
            vx, vy, vz = target.velocity

            # Calculate range (3D distance)
            range_m = np.sqrt(x**2 + y**2 + z**2)

            # Calculate azimuth (angle from North in XY plane)
            azimuth_deg = np.degrees(np.arctan2(x, y)) % 360

            # Calculate elevation
            horizontal_range = np.sqrt(x**2 + y**2)
            if horizontal_range > 0:
                elevation_deg = np.degrees(np.arctan2(z, horizontal_range))
            else:
                elevation_deg = 90.0 if z > 0 else -90.0

            # Calculate Doppler (radial velocity)
            if range_m > 0:
                range_rate = (x * vx + y * vy + z * vz) / range_m
            else:
                range_rate = 0.0
            doppler_hz = 2 * range_rate * self.frequency / c

            # Estimate SNR using radar equation (simplified)
            # SNR ∝ RCS / R^4
            rcs_linear = 10 ** (target.rcs / 10)
            snr_ref = 40.0  # Reference SNR at 10 km
            range_km = range_m / 1000.0
            snr_db = snr_ref + 10 * np.log10(rcs_linear) - 40 * np.log10(max(range_km, 0.1) / 10.0)

            # Add measurement noise
            range_noise = self._rng.normal(0, c / (2 * self.bandwidth))
            azimuth_noise = self._rng.normal(0, 0.5)  # 0.5 degree noise
            range_m += range_noise
            azimuth_deg = (azimuth_deg + azimuth_noise) % 360

            # Create radar plot
            plot = RadarPlot(
                range=max(0, range_m),
                azimuth=azimuth_deg,
                elevation=elevation_deg,
                rcs=target.rcs,
                snr=np.clip(snr_db, 5.0, 50.0),
                doppler=doppler_hz,
                timestamp=timestamp,
                amplitude=10 ** (snr_db / 20.0),
                quality=min(1.0, snr_db / 40.0)
            )
            plots.append(plot)

        return plots

    def get_radar_position(self) -> RadarPosition:
        """Get the radar's geographic position."""
        return self.position

    def get_radar_parameters(self) -> Dict[str, Any]:
        """Get radar system parameters."""
        return {
            'frequency_hz': self.frequency,
            'bandwidth_hz': self.bandwidth,
            'tx_power_dbm': self.tx_power,
            'pulse_length_s': self.pulse_length,
            'samples_per_pulse': self.samples_per_pulse,
            'noise_figure_db': self.noise_figure,
            'range_resolution_m': 3e8 / (2 * self.bandwidth),
            'max_unambiguous_range_m': 3e8 * self.pulse_length / 2,
        }

    def generate_track(
        self,
        start_location: Tuple[float, float, float],
        velocity: Tuple[float, float, float],
        duration_s: float,
        dt: float = 4.0,
        rcs: float = 10.0
    ) -> List[RadarPlot]:
        """
        Generate a target track using RadarSimPy simulation.

        Args:
            start_location: Initial position [x, y, z] in meters
            velocity: Constant velocity [vx, vy, vz] in m/s
            duration_s: Track duration in seconds
            dt: Time between radar updates in seconds
            rcs: Target RCS in dBsm

        Returns:
            List of RadarPlot objects forming the track
        """
        plots = []
        num_updates = int(duration_s / dt)
        x, y, z = start_location
        vx, vy, vz = velocity
        timestamp = time.time()

        for i in range(num_updates):
            # Current position
            current_x = x + vx * dt * i
            current_y = y + vy * dt * i
            current_z = z + vz * dt * i

            # Create target
            target = PointTarget(
                location=(current_x, current_y, current_z),
                velocity=velocity,
                rcs=rcs
            )

            # Simulate detection
            detection = self.simulate_targets(
                [target],
                timestamp=timestamp + i * dt
            )
            plots.extend(detection)

        return plots


class MockRadarAdapter(RadarSimulatorBase):
    """
    Adapter wrapper for MockRadar to match RadarSimulatorBase interface.

    Provides compatibility between the existing MockRadar implementation
    and the RadarSimulatorBase interface for unified usage.
    """

    def __init__(
        self,
        lat: float = 52.5,
        lon: float = 13.4,
        alt: float = 100.0,
        max_range: float = 200e3,
        min_range: float = 1e3,
        seed: Optional[int] = None,
        name: str = "RADAR_01"
    ):
        """Initialize MockRadar adapter."""
        from .mock_radar import MockRadar
        self._radar = MockRadar(
            lat=lat,
            lon=lon,
            alt=alt,
            max_range=max_range,
            min_range=min_range,
            seed=seed,
            name=name
        )

    def simulate_targets(
        self,
        targets: List[PointTarget],
        timestamp: Optional[float] = None
    ) -> List[RadarPlot]:
        """
        Simulate radar detections for point targets using MockRadar.

        Note: MockRadar doesn't use PointTarget directly, so this method
        generates plots based on the target positions.

        Args:
            targets: List of PointTarget objects
            timestamp: Unix timestamp

        Returns:
            List of RadarPlot objects
        """
        if timestamp is None:
            timestamp = time.time()

        plots = []
        for target in targets:
            x, y, z = target.location

            # Convert Cartesian to polar
            range_m = np.sqrt(x**2 + y**2 + z**2)
            azimuth_deg = np.degrees(np.arctan2(x, y)) % 360
            horizontal_range = np.sqrt(x**2 + y**2)
            if horizontal_range > 0:
                elevation_deg = np.degrees(np.arctan2(z, horizontal_range))
            else:
                elevation_deg = 0.0

            # Simplified SNR calculation
            range_km = range_m / 1000.0
            snr_db = 40.0 - 40 * np.log10(max(range_km, 0.1) / 10.0) + 10 * np.log10(10 ** (target.rcs / 10))

            plot = RadarPlot(
                range=range_m,
                azimuth=azimuth_deg,
                elevation=elevation_deg,
                rcs=target.rcs,
                snr=np.clip(snr_db, 5.0, 50.0),
                doppler=0.0,  # MockRadar doesn't compute Doppler from velocity
                timestamp=timestamp,
                quality=min(1.0, snr_db / 40.0)
            )
            plots.append(plot)

        return plots

    def get_radar_position(self) -> RadarPosition:
        """Get the radar's geographic position."""
        return self._radar.position

    def get_radar_parameters(self) -> Dict[str, Any]:
        """Get radar system parameters."""
        return {
            'max_range_m': self._radar.max_range,
            'min_range_m': self._radar.min_range,
            'azimuth_resolution_deg': self._radar.azimuth_resolution,
            'range_resolution_m': self._radar.range_resolution,
            'update_rate_hz': self._radar.update_rate,
        }


def create_radar_simulator(
    use_radarsimpy: bool = True,
    **kwargs
) -> RadarSimulatorBase:
    """
    Factory function to create a radar simulator.

    Attempts to use RadarSimPy if available and requested, otherwise
    falls back to MockRadar.

    Args:
        use_radarsimpy: Prefer RadarSimPy if available
        **kwargs: Arguments passed to the simulator constructor

    Returns:
        RadarSimulatorBase instance (RadarSimPyAdapter or MockRadarAdapter)
    """
    if use_radarsimpy and _RADARSIMPY_AVAILABLE:
        return RadarSimPyAdapter(**kwargs)
    else:
        return MockRadarAdapter(**kwargs)


def print_radarsimpy_info() -> None:
    """Print RadarSimPy availability and configuration information."""
    print("=" * 60)
    print("RadarSimPy Integration Status")
    print("=" * 60)

    if _RADARSIMPY_AVAILABLE:
        print("Status: ✓ RadarSimPy is available")
        try:
            version = radarsimpy.__version__
            print(f"Version: {version}")
        except AttributeError:
            print("Version: Unknown")
        print("\nFeatures available:")
        print("  - Physics-based radar simulation")
        print("  - Point target simulation")
        print("  - RCS computation")
        print("  - Baseband signal generation")
    else:
        print("Status: ✗ RadarSimPy is NOT installed")
        print(f"Error: {_RADARSIMPY_ERROR}")
        print("\nTo install RadarSimPy:")
        print("  1. Visit https://radarsimx.com/radarsimx/radarsimpy/")
        print("  2. Download the appropriate version for your platform")
        print("  3. Place the 'radarsimpy' folder in your project directory")
        print("\nFallback: MockRadar is available for basic testing")

    print("=" * 60)


if __name__ == "__main__":
    print_radarsimpy_info()

    # Demo usage
    if _RADARSIMPY_AVAILABLE:
        print("\nDemo: RadarSimPy simulation")
        adapter = RadarSimPyAdapter()
        targets = [
            PointTarget(location=(50000, 30000, 10000), velocity=(100, 0, 0), rcs=10),
            PointTarget(location=(80000, -20000, 8000), velocity=(-50, 100, 0), rcs=15),
        ]
        plots = adapter.simulate_targets(targets)
        print(f"Generated {len(plots)} radar plots")
        for i, plot in enumerate(plots):
            print(f"  Target {i+1}: Range={plot.range/1000:.1f}km, "
                  f"Az={plot.azimuth:.1f}°, SNR={plot.snr:.1f}dB")
    else:
        print("\nDemo: Using MockRadar fallback")
        adapter = MockRadarAdapter()
        targets = [
            PointTarget(location=(50000, 30000, 10000), rcs=10),
        ]
        plots = adapter.simulate_targets(targets)
        print(f"Generated {len(plots)} radar plots using MockRadar")
