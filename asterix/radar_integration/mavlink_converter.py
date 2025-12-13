"""
MAVLink to ASTERIX Converter

Converts MAVLink telemetry from ArduPilot SITL (or real autopilots) into
ASTERIX surveillance data formats (CAT021 ADS-B, CAT062 system tracks).

Enables integration with flight simulators and autopilot systems for:
- Realistic test data generation
- Flight trajectory validation
- Autopilot telemetry encoding
- Surveillance system testing

Usage:
    from asterix.radar_integration.mavlink_converter import (
        MAVLinkToAsterixConverter,
        listen_mavlink_stream
    )

    # Connect to ArduPilot SITL
    converter = MAVLinkToAsterixConverter()
    converter.connect('udpin:localhost:14550')

    # Convert telemetry to ASTERIX
    for adsb_report in converter.stream_adsb_reports():
        asterix_data = encode_cat021([adsb_report])
        # Process or save ASTERIX data

Requirements:
    pip install pymavlink

References:
    - ArduPilot SITL: https://ardupilot.org/dev/docs/using-sitl-for-ardupilot-testing.html
    - MAVLink Protocol: https://mavlink.io/
    - pymavlink: https://github.com/ArduPilot/pymavlink

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import time
from typing import Dict, Optional, Iterator
from dataclasses import dataclass


@dataclass
class MAVLinkState:
    """Stores latest state from MAVLink messages."""
    lat: float = 0.0  # degrees
    lon: float = 0.0  # degrees
    alt_msl: float = 0.0  # meters MSL
    alt_relative: float = 0.0  # meters AGL
    vx: float = 0.0  # m/s North
    vy: float = 0.0  # m/s East
    vz: float = 0.0  # m/s Down
    heading: float = 0.0  # degrees
    groundspeed: float = 0.0  # m/s
    airspeed: float = 0.0  # m/s
    climb_rate: float = 0.0  # m/s
    timestamp: float = 0.0
    system_id: int = 1
    armed: bool = False
    mode: str = "UNKNOWN"


class MAVLinkToAsterixConverter:
    """
    Converts MAVLink telemetry to ASTERIX surveillance formats.

    Supports:
    - CAT021: ADS-B Target Reports (high precision)
    - CAT062: System Track Data (fused surveillance)

    Example:
        >>> converter = MAVLinkToAsterixConverter()
        >>> converter.connect('udpin:localhost:14550')
        >>> for report in converter.stream_adsb_reports(duration=60):
        ...     asterix_data = encode_cat021([report])
    """

    def __init__(self, sac: int = 0, sic: int = 1):
        """
        Initialize converter.

        Args:
            sac: System Area Code for ASTERIX encoding
            sic: System Identification Code for ASTERIX encoding
        """
        self.sac = sac
        self.sic = sic
        self.state = MAVLinkState()
        self.connection = None
        self.track_number = 1000

    def connect(self, connection_string: str = 'udpin:localhost:14550'):
        """
        Connect to MAVLink telemetry source.

        Args:
            connection_string: pymavlink connection string
                - 'udpin:localhost:14550' - ArduPilot SITL
                - 'udp:127.0.0.1:14550' - Alternative UDP
                - '/dev/ttyUSB0' - Serial port
                - 'tcp:127.0.0.1:5760' - TCP connection

        Raises:
            ImportError: If pymavlink not installed
            ConnectionError: If connection fails
        """
        try:
            from pymavlink import mavutil
        except ImportError:
            raise ImportError(
                "pymavlink not installed. Install with: pip install pymavlink"
            )

        try:
            self.connection = mavutil.mavlink_connection(connection_string)
            # Wait for heartbeat
            msg = self.connection.wait_heartbeat(timeout=10)
            if msg:
                print(f"✅ Connected to system {msg.get_srcSystem()}")
                self.state.system_id = msg.get_srcSystem()
            else:
                raise ConnectionError("No heartbeat received")
        except Exception as e:
            raise ConnectionError(f"Failed to connect to MAVLink: {e}")

    def update_from_global_position(self, msg):
        """
        Update state from GLOBAL_POSITION_INT message.

        Args:
            msg: MAVLink GLOBAL_POSITION_INT message
        """
        self.state.lat = msg.lat / 1e7  # Convert to degrees
        self.state.lon = msg.lon / 1e7
        self.state.alt_msl = msg.alt / 1000.0  # mm to meters
        self.state.alt_relative = msg.relative_alt / 1000.0

        # Velocities in cm/s to m/s
        self.state.vx = msg.vx / 100.0  # North
        self.state.vy = msg.vy / 100.0  # East
        self.state.vz = msg.vz / 100.0  # Down

        self.state.heading = msg.hdg / 100.0  # centidegrees to degrees
        self.state.timestamp = time.time()

    def update_from_vfr_hud(self, msg):
        """
        Update state from VFR_HUD message.

        Args:
            msg: MAVLink VFR_HUD message
        """
        self.state.airspeed = msg.airspeed  # m/s
        self.state.groundspeed = msg.groundspeed  # m/s
        self.state.heading = msg.heading  # degrees
        self.state.climb_rate = msg.climb  # m/s

    def update_from_heartbeat(self, msg):
        """
        Update state from HEARTBEAT message.

        Args:
            msg: MAVLink HEARTBEAT message
        """
        self.state.armed = (msg.base_mode & 128) != 0  # MAV_MODE_FLAG_SAFETY_ARMED
        # Store mode as string (would need mode mapping table for actual names)
        self.state.mode = f"MODE_{msg.custom_mode}"

    def to_adsb_report(self, track_number: Optional[int] = None) -> Dict:
        """
        Convert current MAVLink state to ADS-B report (CAT021 format).

        Args:
            track_number: Optional track number (auto-increment if None)

        Returns:
            Dictionary compatible with encode_cat021()
        """
        if track_number is None:
            track_number = self.track_number
            self.track_number += 1

        # Generate ICAO address from system ID (mock)
        aircraft_address = 0x400000 + self.state.system_id

        # Convert altitude to feet
        altitude_ft = self.state.alt_msl * 3.28084
        flight_level = altitude_ft / 100.0

        # Convert climb rate to ft/min
        vertical_rate_fpm = self.state.climb_rate * 196.85  # m/s to ft/min

        # Convert speeds to knots
        airspeed_kt = self.state.airspeed * 1.94384
        groundspeed_kt = self.state.groundspeed * 1.94384

        return {
            'lat': self.state.lat,
            'lon': self.state.lon,
            'aircraft_address': aircraft_address,
            'callsign': f'SIM{self.state.system_id:03d}',  # Mock callsign
            'flight_level': flight_level,
            'gnss_height_ft': altitude_ft,
            'airspeed_kt': airspeed_kt,
            'true_airspeed_kt': airspeed_kt,  # Approximate
            'magnetic_heading_deg': self.state.heading,
            'vertical_rate_fpm': vertical_rate_fpm,
            'track_number': track_number,
            'timestamp': self.state.timestamp
        }

    def to_system_track(self) -> Dict:
        """
        Convert current MAVLink state to system track (CAT062 format).

        Returns:
            Dictionary compatible with encode_cat062()
        """
        # Convert altitude to feet
        altitude_ft = self.state.alt_msl * 3.28084

        return {
            'lat': self.state.lat,
            'lon': self.state.lon,
            'altitude_ft': altitude_ft,
            'vx': self.state.vy,  # MAVLink vy = East
            'vy': self.state.vx,  # MAVLink vx = North
            'callsign': f'SIM{self.state.system_id:03d}',
            'adep': 'SITL',  # Mock departure
            'ades': 'SITL',  # Mock destination
            'timestamp': self.state.timestamp
        }

    def stream_adsb_reports(
        self,
        duration: Optional[float] = None,
        update_rate: float = 1.0
    ) -> Iterator[Dict]:
        """
        Stream ADS-B reports from MAVLink telemetry.

        Args:
            duration: Duration in seconds (None = infinite)
            update_rate: Target update rate in Hz

        Yields:
            ADS-B report dictionaries
        """
        if self.connection is None:
            raise RuntimeError("Not connected. Call connect() first.")

        start_time = time.time()
        last_update = 0

        while True:
            # Check duration
            if duration and (time.time() - start_time) > duration:
                break

            # Receive MAVLink messages
            msg = self.connection.recv_match(
                type=['GLOBAL_POSITION_INT', 'VFR_HUD', 'HEARTBEAT'],
                blocking=False,
                timeout=1.0
            )

            if msg:
                msg_type = msg.get_type()

                if msg_type == 'GLOBAL_POSITION_INT':
                    self.update_from_global_position(msg)

                    # Generate report at target rate
                    now = time.time()
                    if (now - last_update) >= (1.0 / update_rate):
                        yield self.to_adsb_report()
                        last_update = now

                elif msg_type == 'VFR_HUD':
                    self.update_from_vfr_hud(msg)

                elif msg_type == 'HEARTBEAT':
                    self.update_from_heartbeat(msg)

            time.sleep(0.01)  # Small delay to prevent CPU spinning


def record_flight_to_asterix(
    connection_string: str = 'udpin:localhost:14550',
    output_file: str = 'flight.ast',
    duration: float = 60.0,
    category: str = 'CAT021',
    sac: int = 0,
    sic: int = 1
):
    """
    Record ArduPilot SITL flight to ASTERIX binary file.

    Args:
        connection_string: MAVLink connection string
        output_file: Output ASTERIX file path
        duration: Recording duration in seconds
        category: ASTERIX category ('CAT021' or 'CAT062')
        sac: System Area Code
        sic: System Identification Code

    Example:
        # Start ArduPilot SITL first:
        # cd ardupilot && ./Tools/autotest/sim_vehicle.py -v ArduPlane

        # Then record:
        >>> record_flight_to_asterix(
        ...     output_file='takeoff.ast',
        ...     duration=120.0,
        ...     category='CAT021'
        ... )
    """
    from asterix.radar_integration.encoder import encode_cat021, encode_cat062

    converter = MAVLinkToAsterixConverter(sac=sac, sic=sic)
    converter.connect(connection_string)

    reports = []

    print(f"Recording {category} data for {duration:.0f} seconds...")
    print(f"Output: {output_file}")

    for report in converter.stream_adsb_reports(duration=duration, update_rate=1.0):
        reports.append(report)
        print(f"\r  Recorded {len(reports)} reports", end='', flush=True)

    print(f"\n\nEncoding {len(reports)} reports to {category}...")

    # Encode to ASTERIX
    if category == 'CAT021':
        asterix_data = encode_cat021(reports, sac=sac, sic=sic)
    elif category == 'CAT062':
        tracks = [converter.to_system_track() for _ in reports]
        asterix_data = encode_cat062(tracks, sac=sac, sic=sic)
    else:
        raise ValueError(f"Unsupported category: {category}")

    # Save to file
    with open(output_file, 'wb') as f:
        f.write(asterix_data)

    print(f"✅ Saved {len(asterix_data)} bytes to {output_file}")
    print(f"   {len(reports)} reports")
    print(f"   {len(asterix_data) / len(reports):.1f} bytes/report")


if __name__ == "__main__":
    print("MAVLink to ASTERIX Converter\n")

    print("This module requires ArduPilot SITL running.")
    print("\nTo start SITL:")
    print("  cd /home/e/Development/ardupilot")
    print("  ./Tools/autotest/sim_vehicle.py -v ArduPlane --console\n")

    print("Example usage:")
    print()
    print("  from mavlink_converter import MAVLinkToAsterixConverter")
    print("  from asterix.radar_integration.encoder import encode_cat021")
    print()
    print("  converter = MAVLinkToAsterixConverter()")
    print("  converter.connect('udpin:localhost:14550')")
    print()
    print("  for report in converter.stream_adsb_reports(duration=60):")
    print("      asterix_data = encode_cat021([report])")
    print("      # Save or process ASTERIX data")
    print()
    print("Or use the convenience function:")
    print()
    print("  record_flight_to_asterix(")
    print("      output_file='takeoff.ast',")
    print("      duration=120.0,")
    print("      category='CAT021'")
    print("  )")
