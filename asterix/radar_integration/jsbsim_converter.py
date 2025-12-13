"""
JSBSim to ASTERIX Converter

Converts JSBSim flight dynamics simulation data into ASTERIX surveillance
formats (CAT021 ADS-B, CAT062 system tracks).

JSBSim is an open-source flight dynamics model supporting realistic aircraft
simulation with 6-DOF motion, aerodynamics, engines, and environmental effects.

Enables integration with flight simulation for:
- Realistic flight trajectories
- Aircraft performance testing
- Scenario generation (takeoff, landing, maneuvers)
- High-fidelity surveillance data

Usage:
    from asterix.radar_integration.jsbsim_converter import (
        JSBSimToAsterixConverter,
        run_scenario_to_asterix
    )

    # Run JSBSim scenario
    converter = JSBSimToAsterixConverter()
    converter.load_aircraft('c172p')
    converter.set_initial_condition(lat=52.5, lon=13.4, alt_ft=5000)
    converter.run_scenario(duration=300)

    # Convert to ASTERIX
    adsb_reports = converter.get_adsb_reports()
    asterix_data = encode_cat021(adsb_reports)

Requirements:
    pip install jsbsim

References:
    - JSBSim Documentation: https://jsbsim-team.github.io/jsbsim/
    - JSBSim Python API: https://jsbsim-team.github.io/jsbsim/python/
    - JSBSim GitHub: https://github.com/JSBSim-Team/jsbsim
    - PyPI Package: https://pypi.org/project/jsbsim/

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import time
from typing import Dict, List, Optional
from dataclasses import dataclass


@dataclass
class FlightState:
    """Stores flight state from JSBSim."""
    lat: float = 0.0  # degrees
    lon: float = 0.0  # degrees
    alt_msl_ft: float = 0.0  # feet MSL
    alt_agl_ft: float = 0.0  # feet AGL
    heading_deg: float = 0.0  # degrees true
    pitch_deg: float = 0.0  # degrees
    roll_deg: float = 0.0  # degrees
    airspeed_kt: float = 0.0  # knots
    groundspeed_kt: float = 0.0  # knots
    vertical_speed_fps: float = 0.0  # feet/second
    velocity_north_fps: float = 0.0  # feet/second
    velocity_east_fps: float = 0.0  # feet/second
    velocity_down_fps: float = 0.0  # feet/second
    timestamp: float = 0.0


class JSBSimToAsterixConverter:
    """
    Converts JSBSim flight simulation data to ASTERIX formats.

    Supports:
    - CAT021: ADS-B Target Reports (high precision)
    - CAT062: System Track Data (fused surveillance)

    Example:
        >>> converter = JSBSimToAsterixConverter()
        >>> converter.load_aircraft('c172p')
        >>> converter.run_scenario(duration=300)
        >>> reports = converter.get_adsb_reports()
        >>> asterix_data = encode_cat021(reports)
    """

    def __init__(self, sac: int = 0, sic: int = 1):
        """
        Initialize converter.

        Args:
            sac: System Area Code for ASTERIX
            sic: System Identification Code for ASTERIX
        """
        self.sac = sac
        self.sic = sic
        self.fdm = None
        self.flight_states = []
        self.track_number_base = 3000

    def load_aircraft(self, aircraft_name: str = 'c172p'):
        """
        Load aircraft model.

        Args:
            aircraft_name: Aircraft model name (c172p, f16, 747, etc.)

        Raises:
            ImportError: If jsbsim not installed
            RuntimeError: If aircraft load fails
        """
        try:
            import jsbsim
        except ImportError:
            raise ImportError(
                "jsbsim not installed. Install with: pip install jsbsim"
            )

        try:
            self.fdm = jsbsim.FGFDMExec(None)
            self.fdm.load_model(aircraft_name)
            print(f"✅ Loaded aircraft: {aircraft_name}")
        except Exception as e:
            raise RuntimeError(f"Failed to load aircraft: {e}")

    def set_initial_condition(
        self,
        lat: float = 37.62,  # Default: Moffett Field, CA
        lon: float = -122.38,
        alt_ft: float = 5000,
        heading_deg: float = 0,
        airspeed_kt: float = 100
    ):
        """
        Set initial flight conditions.

        Args:
            lat: Initial latitude (degrees)
            lon: Initial longitude (degrees)
            alt_ft: Initial altitude MSL (feet)
            heading_deg: Initial heading (degrees)
            airspeed_kt: Initial airspeed (knots)
        """
        if not self.fdm:
            raise RuntimeError("Aircraft not loaded. Call load_aircraft() first.")

        # Set position
        self.fdm['ic/lat-geod-deg'] = lat
        self.fdm['ic/long-gc-deg'] = lon
        self.fdm['ic/h-sl-ft'] = alt_ft
        self.fdm['ic/psi-true-deg'] = heading_deg
        self.fdm['ic/vc-kts'] = airspeed_kt

        # Initialize
        self.fdm.run_ic()

        print(f"✅ Initial conditions set: {lat:.4f}°, {lon:.4f}°, {alt_ft:.0f} ft")

    def run_scenario(
        self,
        duration: float = 60.0,
        dt: float = 1.0,
        throttle: float = 0.7,
        auto_pilot: bool = False
    ):
        """
        Run flight simulation scenario.

        Args:
            duration: Simulation duration (seconds)
            dt: Time step (seconds) for data recording
            throttle: Throttle setting (0.0-1.0)
            auto_pilot: Enable simple autopilot (hold heading/altitude)
        """
        if not self.fdm:
            raise RuntimeError("Aircraft not loaded.")

        print(f"Running simulation for {duration:.0f} seconds...")

        # Set throttle
        self.fdm['fcs/throttle-cmd-norm'] = throttle

        sim_time = 0
        next_record = 0

        while sim_time < duration:
            # Run simulation step
            self.fdm.run()

            # Record state at intervals
            if sim_time >= next_record:
                state = self._capture_state()
                self.flight_states.append(state)
                next_record += dt

            sim_time = self.fdm['simulation/sim-time-sec']

        print(f"✅ Simulation complete: {len(self.flight_states)} states recorded")

    def _capture_state(self) -> FlightState:
        """Capture current flight state from JSBSim."""
        return FlightState(
            lat=self.fdm['position/lat-geod-deg'],
            lon=self.fdm['position/long-gc-deg'],
            alt_msl_ft=self.fdm['position/h-sl-ft'],
            alt_agl_ft=self.fdm['position/h-agl-ft'],
            heading_deg=self.fdm['attitude/psi-deg'],
            pitch_deg=self.fdm['attitude/theta-deg'],
            roll_deg=self.fdm['attitude/phi-deg'],
            airspeed_kt=self.fdm['velocities/vc-kts'],
            groundspeed_kt=self.fdm['velocities/vg-kts'],
            vertical_speed_fps=self.fdm['velocities/h-dot-fps'],
            velocity_north_fps=self.fdm['velocities/v-north-fps'],
            velocity_east_fps=self.fdm['velocities/v-east-fps'],
            velocity_down_fps=self.fdm['velocities/v-down-fps'],
            timestamp=time.time()
        )

    def get_adsb_reports(self, track_number: Optional[int] = None) -> List[Dict]:
        """
        Convert all recorded states to ADS-B reports (CAT021 format).

        Args:
            track_number: Track number (auto-assigned if None)

        Returns:
            List of ADS-B report dictionaries
        """
        if track_number is None:
            track_number = self.track_number_base

        reports = []

        for i, state in enumerate(self.flight_states):
            # Generate ICAO address (mock)
            aircraft_address = 0x500000 + track_number

            # Calculate vertical rate in ft/min
            vertical_rate_fpm = state.vertical_speed_fps * 60.0

            # Calculate flight level
            flight_level = state.alt_msl_ft / 100.0

            # Convert velocities to m/s
            state.velocity_north_fps * 0.3048
            state.velocity_east_fps * 0.3048

            report = {
                'lat': state.lat,
                'lon': state.lon,
                'aircraft_address': aircraft_address,
                'callsign': f'JSB{track_number:04d}',
                'flight_level': flight_level,
                'gnss_height_ft': state.alt_msl_ft,
                'airspeed_kt': state.airspeed_kt,
                'true_airspeed_kt': state.airspeed_kt,  # Simplified
                'magnetic_heading_deg': state.heading_deg,
                'vertical_rate_fpm': vertical_rate_fpm,
                'track_number': track_number,
                'timestamp': state.timestamp + i  # Increment timestamp
            }
            reports.append(report)

        return reports

    def get_system_tracks(self) -> List[Dict]:
        """
        Convert recorded states to system tracks (CAT062 format).

        Returns:
            List of system track dictionaries
        """
        tracks = []

        for i, state in enumerate(self.flight_states):
            # Convert velocities to m/s
            vx_ms = state.velocity_north_fps * 0.3048
            vy_ms = state.velocity_east_fps * 0.3048

            track = {
                'lat': state.lat,
                'lon': state.lon,
                'altitude_ft': state.alt_msl_ft,
                'vx': vy_ms,  # East
                'vy': vx_ms,  # North
                'callsign': f'JSB{self.track_number_base:04d}',
                'adep': 'JSIM',  # Mock
                'ades': 'JSIM',
                'timestamp': state.timestamp + i
            }
            tracks.append(track)

        return tracks


def run_scenario_to_asterix(
    aircraft: str = 'c172p',
    initial_lat: float = 52.5,
    initial_lon: float = 13.4,
    initial_alt_ft: float = 5000,
    duration: float = 300.0,
    output_file: str = 'jsbsim_flight.ast',
    category: str = 'CAT021',
    sac: int = 0,
    sic: int = 1
):
    """
    Run JSBSim scenario and save to ASTERIX file.

    Args:
        aircraft: Aircraft model name
        initial_lat: Starting latitude
        initial_lon: Starting longitude
        initial_alt_ft: Starting altitude (feet MSL)
        duration: Simulation duration (seconds)
        output_file: Output ASTERIX file
        category: 'CAT021' or 'CAT062'
        sac: System Area Code
        sic: System Identification Code

    Example:
        >>> run_scenario_to_asterix(
        ...     aircraft='c172p',
        ...     initial_lat=52.5,
        ...     initial_lon=13.4,
        ...     duration=300.0,
        ...     output_file='cessna_flight.ast'
        ... )
    """
    from asterix.radar_integration.encoder import encode_cat021, encode_cat062

    converter = JSBSimToAsterixConverter(sac=sac, sic=sic)
    converter.load_aircraft(aircraft)
    converter.set_initial_condition(
        lat=initial_lat,
        lon=initial_lon,
        alt_ft=initial_alt_ft
    )
    converter.run_scenario(duration=duration)

    # Get reports
    if category == 'CAT021':
        reports = converter.get_adsb_reports()
        asterix_data = encode_cat021(reports, sac=sac, sic=sic)
    elif category == 'CAT062':
        tracks = converter.get_system_tracks()
        asterix_data = encode_cat062(tracks, sac=sac, sic=sic)
    else:
        raise ValueError(f"Unsupported category: {category}")

    # Save to file
    with open(output_file, 'wb') as f:
        f.write(asterix_data)

    print(f"✅ Saved {len(reports if category == 'CAT021' else tracks)} records to {output_file}")
    print(f"   {len(asterix_data)} bytes")


if __name__ == "__main__":
    print("JSBSim to ASTERIX Converter\n")

    print("This module requires JSBSim Python package.")
    print("\nInstall with:")
    print("  pip install jsbsim\n")

    print("Supported aircraft:")
    print("  - c172p (Cessna 172)")
    print("  - f16 (F-16 Fighting Falcon)")
    print("  - 737 (Boeing 737)")
    print("  - 747 (Boeing 747)")
    print("  - A320 (Airbus A320)")
    print("  - and many more...\n")

    print("Example usage:")
    print()
    print("  from jsbsim_converter import run_scenario_to_asterix")
    print()
    print("  # Simulate Cessna 172 flight")
    print("  run_scenario_to_asterix(")
    print("      aircraft='c172p',")
    print("      initial_lat=52.5,")
    print("      initial_lon=13.4,")
    print("      initial_alt_ft=5000,")
    print("      duration=300.0,")
    print("      output_file='cessna_flight.ast',")
    print("      category='CAT021'")
    print("  )")
    print()
    print("Benefits of JSBSim integration:")
    print("  ✅ Realistic flight dynamics (6-DOF)")
    print("  ✅ Aircraft-specific models (C172, 747, F-16, etc.)")
    print("  ✅ Aerodynamic effects (lift, drag, thrust)")
    print("  ✅ Environmental simulation (wind, turbulence)")
    print("  ✅ LGPLv2.1 license (compatible with ASTERIX GPL-3.0)")
