#!/usr/bin/env python3
"""
Real-Time ASTERIX Streaming Example

Demonstrates real-time generation and streaming of ASTERIX surveillance data,
simulating a live radar system with:
- Continuous radar rotation (4-second cycle)
- North Marker messages (CAT034)
- Sector Crossing messages (CAT034)
- Radar plot updates (CAT048)
- MLAT system status (CAT019)
- System track updates (CAT062)

This simulates a complete airport surveillance system operating in real-time.

Usage:
    python3 realtime_streaming.py [--duration SECONDS] [--udp] [--port PORT]

Options:
    --duration: Run for N seconds (default: 60)
    --udp: Stream to UDP multicast (239.1.1.1:9000)
    --port: UDP port (default: 9000)
    --display: Show ASCII radar display updates

License: GPL-3.0
"""

import sys
import time
import argparse
import socket
import os

# Add project root to path (assumes examples/radar_integration/ directory structure)
project_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, project_root)

from asterix.radar_integration import MockRadar
from asterix.radar_integration.encoder import (
    encode_cat019_status,
    encode_cat034_north_marker,
    encode_cat034_sector_crossing,
    encode_cat048,
    encode_cat062
)
from asterix.radar_integration.visualization import plot_radar_ascii


class RealtimeRadarSimulator:
    """Simulates real-time radar system with ASTERIX encoding."""

    def __init__(self, sac=0, sic=1):
        """Initialize simulator."""
        self.sac = sac
        self.sic = sic
        self.radar = MockRadar(lat=52.5597, lon=13.2877, alt=100.0)
        self.rotation_period = 4.0  # 4 second rotation (15 RPM)
        self.num_sectors = 8
        self.sector_period = self.rotation_period / self.num_sectors

        self.current_sector = 0
        self.rotation_count = 0
        self.udp_sock = None

    def setup_udp(self, multicast_group='239.1.1.1', port=9000):
        """Setup UDP multicast streaming."""
        self.udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp_sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)
        self.multicast_addr = (multicast_group, port)
        print(f"✅ UDP streaming to {multicast_group}:{port}")

    def send_message(self, asterix_data, msg_type="DATA"):
        """Send ASTERIX message via UDP if configured."""
        if self.udp_sock:
            self.udp_sock.sendto(asterix_data, self.multicast_addr)
            print(f"  → Sent {len(asterix_data)} bytes via UDP ({msg_type})")
        else:
            print(f"  Generated {len(asterix_data)} bytes ({msg_type})")

    def run(self, duration=60.0, display=False):
        """
        Run real-time simulation.

        Args:
            duration: Simulation duration in seconds
            display: Show ASCII radar display
        """
        print("=" * 80)
        print("Real-Time ASTERIX Streaming Simulation")
        print("=" * 80)
        print(f"Duration: {duration:.0f} seconds")
        print(f"Rotation: {self.rotation_period:.1f}s ({60/self.rotation_period:.1f} RPM)")
        print(f"Sectors: {self.num_sectors}")
        print()

        start_time = time.time()
        last_sector_time = start_time
        last_mlat_status = start_time
        message_count = 0

        # Initial MLAT status
        mlat_status = encode_cat019_status(
            sac=self.sac, sic=self.sic,
            message_type=1,  # Start
            mlat_lat=52.5597, mlat_lon=13.2877, mlat_height_m=100.0,
            operational=True, num_sensors=4
        )
        self.send_message(mlat_status, "CAT019 Start")
        message_count += 1

        while True:
            current_time = time.time()
            elapsed = current_time - start_time

            # Check duration
            if elapsed >= duration:
                break

            # Check for sector crossing
            if (current_time - last_sector_time) >= self.sector_period:
                # Sector crossing
                sector_msg = encode_cat034_sector_crossing(
                    sac=self.sac, sic=self.sic,
                    sector_number=self.current_sector,
                    timestamp=current_time
                )
                self.send_message(sector_msg, f"CAT034 Sector {self.current_sector}")
                message_count += 1

                # If sector 0, send North Marker
                if self.current_sector == 0:
                    nm_msg = encode_cat034_north_marker(
                        sac=self.sac, sic=self.sic,
                        sector_number=0,
                        timestamp=current_time,
                        antenna_rotation_speed=self.rotation_period
                    )
                    self.send_message(nm_msg, "CAT034 North Marker")
                    message_count += 1
                    self.rotation_count += 1

                    # Generate radar plots for this rotation
                    plots = self.radar.generate_plots(
                        num_targets=15,
                        timestamp=current_time,
                        add_noise=True,
                        add_clutter=True
                    )

                    # Encode to CAT048
                    cat048_data = encode_cat048(plots, sac=self.sac, sic=self.sic)
                    self.send_message(cat048_data, f"CAT048 ({len(plots)} plots)")
                    message_count += 1

                    # Display if requested
                    if display:
                        print(f"\nRotation {self.rotation_count}:")
                        print(plot_radar_ascii(plots[:min(len(plots), 20)]))

                # Next sector
                self.current_sector = (self.current_sector + 1) % self.num_sectors
                last_sector_time = current_time

            # MLAT status every 10 seconds
            if (current_time - last_mlat_status) >= 10.0:
                mlat_status = encode_cat019_status(
                    sac=self.sac, sic=self.sic,
                    message_type=2,  # Periodic
                    operational=True, num_sensors=4
                )
                self.send_message(mlat_status, "CAT019 Status")
                message_count += 1
                last_mlat_status = current_time

            # Small delay
            time.sleep(0.01)

        print()
        print("=" * 80)
        print(f"✅ Simulation Complete")
        print("=" * 80)
        print(f"Runtime: {elapsed:.1f} seconds")
        print(f"Rotations: {self.rotation_count}")
        print(f"Messages: {message_count}")
        print(f"Rate: {message_count / elapsed:.1f} messages/second")


def main():
    """Run real-time streaming example."""
    parser = argparse.ArgumentParser(description='Real-time ASTERIX streaming')
    parser.add_argument('--duration', type=float, default=60.0, help='Duration in seconds')
    parser.add_argument('--udp', action='store_true', help='Stream to UDP multicast')
    parser.add_argument('--port', type=int, default=9000, help='UDP port')
    parser.add_argument('--display', action='store_true', help='Show ASCII display')

    args = parser.parse_args()

    simulator = RealtimeRadarSimulator(sac=0, sic=1)

    if args.udp:
        simulator.setup_udp(port=args.port)

    simulator.run(duration=args.duration, display=args.display)


if __name__ == "__main__":
    main()
