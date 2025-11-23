#!/usr/bin/env python3
"""
ASTERIX CAT062 Encoding Example

Demonstrates encoding system track data into ASTERIX CAT062 format.

CAT062: Transmission of System Track Data (fused radar tracks)

This example shows how to:
1. Create system track data (fused from multiple sensors)
2. Encode to ASTERIX CAT062 binary format
3. Decode and verify with ASTERIX parser
4. Visualize encoded tracks

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import time
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(__file__))))

from asterix.radar_integration.encoder import encode_cat062
from asterix.radar_integration.visualization import plot_radar_ascii


def create_sample_tracks():
    """
    Create sample system tracks for encoding.

    Returns:
        List of track dictionaries with position, velocity, and flight plan data
    """
    tracks = [
        {
            # Track 1: Lufthansa flight from Frankfurt to New York
            'lat': 48.8584,  # Paris area
            'lon': 2.2945,
            'altitude_ft': 35000.0,
            'vx': 100.0,  # m/s (East component)
            'vy': 200.0,  # m/s (North component)
            'callsign': 'DLH123',
            'adep': 'EDDF',  # Frankfurt
            'ades': 'KJFK',  # New York JFK
            'timestamp': time.time()
        },
        {
            # Track 2: British Airways flight from London to New York
            'lat': 51.5074,  # London area
            'lon': -0.1278,
            'altitude_ft': 38000.0,
            'vx': 120.0,
            'vy': 180.0,
            'callsign': 'BAW456',
            'adep': 'EGLL',  # London Heathrow
            'ades': 'KJFK',  # New York JFK
            'timestamp': time.time() + 0.5
        },
        {
            # Track 3: Air France flight approaching New York
            'lat': 40.6413,  # New York JFK area
            'lon': -73.7781,
            'altitude_ft': 5000.0,
            'vx': -50.0,  # Decelerating
            'vy': -80.0,
            'callsign': 'AFR789',
            'adep': 'LFPG',  # Paris CDG
            'ades': 'KJFK',  # New York JFK
            'timestamp': time.time() + 1.0
        },
        {
            # Track 4: Cargo flight without full flight plan
            'lat': 49.0, 'lon': 2.5,
            'altitude_ft': 30000.0,
            'vx': 150.0,
            'vy': 100.0,
            'timestamp': time.time() + 1.5
        },
        {
            # Track 5: Regional flight
            'lat': 52.0, 'lon': 4.5,
            'altitude_ft': 15000.0,
            'vx': 80.0,
            'vy': 120.0,
            'callsign': 'KLM890',
            'adep': 'EHAM',  # Amsterdam
            'ades': 'EDDF',  # Frankfurt
            'timestamp': time.time() + 2.0
        },
    ]

    return tracks


def main():
    """Main example function."""
    print("=" * 70)
    print("ASTERIX CAT062 Encoding Example")
    print("=" * 70)
    print()

    # Create sample tracks
    print("1. Creating sample system tracks...")
    print("-" * 70)
    tracks = create_sample_tracks()
    print(f"   Created {len(tracks)} system tracks")
    print()

    # Display track details
    print("2. Track details:")
    print("-" * 70)
    for i, track in enumerate(tracks):
        print(f"   Track {i+1}:")
        print(f"     Position: {track['lat']:.4f}°N, {track['lon']:.4f}°E")
        print(f"     Altitude: {track.get('altitude_ft', 'N/A')} ft")
        if 'vx' in track and 'vy' in track:
            speed_ms = (track['vx']**2 + track['vy']**2)**0.5
            speed_kt = speed_ms * 1.94384
            print(f"     Velocity: {speed_ms:.1f} m/s ({speed_kt:.1f} kt)")
        if 'callsign' in track:
            cs = track['callsign']
            dep = track.get('adep', 'N/A')
            des = track.get('ades', 'N/A')
            print(f"     Flight Plan: {cs} ({dep} -> {des})")
        print()

    # Encode to ASTERIX CAT062
    print("3. Encoding to ASTERIX CAT062 binary format...")
    print("-" * 70)
    asterix_data = encode_cat062(tracks, sac=0, sic=1)
    print(f"   Encoded data length: {len(asterix_data)} bytes")
    print(f"   Category: {asterix_data[0]}")
    print(f"   First 48 bytes (hex): {asterix_data[:48].hex()}")
    print()

    # Decode with ASTERIX parser (if available)
    print("4. Decoding with ASTERIX parser (round-trip verification)...")
    print("-" * 70)
    try:
        import asterix
        decoded = asterix.parse(asterix_data)
        print(f"   ✓ Successfully decoded {len(decoded)} records")
        print()

        if decoded:
            # Display first decoded record
            print("   Sample decoded record (Record 0):")
            rec = decoded[0]
            print(f"     Category: {rec.get('category')}")

            if 'I010' in rec:
                sac = rec['I010'].get('SAC')
                sic = rec['I010'].get('SIC')
                print(f"     SAC/SIC: {sac}/{sic}")

            if 'I105' in rec:
                lat = rec['I105'].get('LAT')
                lon = rec['I105'].get('LON')
                print(f"     Position: {lat:.6f}°, {lon:.6f}°")

            if 'I135' in rec:
                alt_fl = rec['I135'].get('CTB')
                alt_ft = alt_fl * 100 if alt_fl else None
                print(f"     Altitude: FL{alt_fl:.2f} ({alt_ft:.0f} ft)")

            if 'I185' in rec:
                vx = rec['I185'].get('VX')
                vy = rec['I185'].get('VY')
                print(f"     Velocity: Vx={vx:.2f} m/s, Vy={vy:.2f} m/s")

            if 'I390' in rec:
                i390 = rec['I390']
                cs = i390.get('CS', {}).get('CS', 'N/A')
                dep = i390.get('DEP', {}).get('DEP', 'N/A')
                dst = i390.get('DST', {}).get('DST', 'N/A')
                print(f"     Flight Plan: {cs} ({dep} -> {dst})")

    except ImportError:
        print("   ⚠ ASTERIX parser not available (install asterix package)")
    except Exception as e:
        print(f"   ❌ Decoding failed: {e}")

    print()

    # Data item breakdown
    print("5. Data item breakdown:")
    print("-" * 70)
    print("   CAT062 Data Items Implemented:")
    print("     • I010: Data Source Identifier (SAC/SIC)")
    print("     • I070: Time of Track Information (1/128 second resolution)")
    print("     • I105: Calculated Position in WGS-84 (lat/lon, 0.6m resolution)")
    print("     • I135: Calculated Track Barometric Altitude (25 ft resolution)")
    print("     • I185: Calculated Track Velocity Cartesian (0.25 m/s resolution)")
    print("     • I390: Flight Plan Related Data (callsign, ADEP, ADES)")
    print()

    # Use cases
    print("6. Use cases:")
    print("-" * 70)
    print("   • Air Traffic Management (ATM) systems")
    print("   • Radar data fusion and track correlation")
    print("   • Flight data recording and playback")
    print("   • ATC simulator data exchange")
    print("   • Performance monitoring and analysis")
    print()

    print("=" * 70)
    print("✓ Example complete!")
    print()
    print("Next steps:")
    print("  • Modify track data for your use case")
    print("  • Add more data items (see CAT062 specification)")
    print("  • Integrate with real tracking systems")
    print("  • Use for testing and validation")
    print("=" * 70)


if __name__ == "__main__":
    main()
