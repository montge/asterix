#!/usr/bin/env python3
"""
Basic Mock Radar Example

Demonstrates simple radar plot generation and ASTERIX encoding.
This is the simplest starting point for radar-to-ASTERIX integration.

Usage:
    python3 basic_mock_radar.py

Requirements:
    - Python 3.10+
    - asterix_decoder package installed (pip install asterix_decoder)
    - Mock radar generator (from .local/integration/)
"""

import sys
import os

# Add parent directory to path for asterix import
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from asterix.radar_integration import MockRadar
from asterix.radar_integration.encoder import encode_cat048


def main():
    print("=" * 60)
    print("Basic Mock Radar Example")
    print("=" * 60)
    print()

    # Step 1: Create radar sensor
    print("Step 1: Create mock radar sensor")
    print("-" * 60)

    radar = MockRadar(
        lat=52.5,              # Berlin, Germany
        lon=13.4,
        alt=100.0,             # 100 meters altitude
        max_range=200e3,       # 200 km max range
        name="RADAR_01"
    )

    print(f"Radar: {radar.position.name}")
    print(f"Position: ({radar.position.lat}°, {radar.position.lon}°)")
    print(f"Altitude: {radar.position.alt} m")
    print(f"Max range: {radar.max_range / 1000:.0f} km")
    print()

    # Step 2: Generate radar plots
    print("Step 2: Generate synthetic radar plots")
    print("-" * 60)

    num_targets = 5
    plots = radar.generate_plots(
        num_targets=num_targets,
        add_noise=True,        # Add realistic measurement noise
        add_clutter=False      # No clutter for simplicity
    )

    print(f"Generated {len(plots)} radar plots\n")

    # Display sample plots
    for i, plot in enumerate(plots[:3]):  # Show first 3
        print(f"Plot {i}:")
        print(f"  Range:    {plot.range:.0f} m ({plot.range / 1852:.1f} NM)")
        print(f"  Azimuth:  {plot.azimuth:.2f}°")
        print(f"  Elevation: {plot.elevation:.2f}°")
        print(f"  SNR:      {plot.snr:.1f} dB")
        print(f"  RCS:      {plot.rcs:.1f} dBsm")
        print(f"  Doppler:  {plot.doppler:.1f} Hz")
        print()

    # Step 3: Encode to ASTERIX CAT048
    print("Step 3: Encode to ASTERIX CAT048")
    print("-" * 60)

    asterix_data = encode_cat048(
        plots,
        radar_position=(radar.position.lat, radar.position.lon, radar.position.alt),
        sac=0,                 # System Area Code
        sic=1                  # System ID Code
    )

    print(f"Encoded {len(plots)} plots → {len(asterix_data)} bytes")
    print(f"Average: {len(asterix_data) / len(plots):.1f} bytes/plot")
    print(f"Data block: CAT={asterix_data[0]}")
    print(f"Hex preview (first 32 bytes):")
    print(f"  {asterix_data[:32].hex()}")
    print()

    # Step 4: Save to file (optional)
    print("Step 4: Save ASTERIX data to file")
    print("-" * 60)

    output_file = "basic_radar_output.ast"
    with open(output_file, 'wb') as f:
        f.write(asterix_data)

    print(f"Saved to: {output_file}")
    print(f"File size: {len(asterix_data)} bytes")
    print()

    # Step 5: Decode (if ASTERIX decoder available)
    print("Step 5: Decode ASTERIX data (verification)")
    print("-" * 60)

    try:
        import asterix
        decoded = asterix.parse(asterix_data)

        print(f"✅ Successfully decoded {len(decoded)} records\n")

        # Show first decoded record
        if decoded:
            rec = decoded[0]
            print("Sample decoded record:")
            print(f"  Category: {rec.get('category')}")
            print(f"  SAC/SIC: {rec.get('I010', {}).get('SAC')}/{rec.get('I010', {}).get('SIC')}")

            if 'I040' in rec:
                i040 = rec['I040']
                print(f"  Range: {i040.get('RHO')} NM = {i040.get('RHO') * 1852:.0f} m")
                print(f"  Azimuth: {i040.get('THETA')}°")

    except ImportError:
        print("⚠️  ASTERIX decoder not installed")
        print("   Install with: pip install asterix_decoder")
        print("   However, encoding worked correctly!")

    except Exception as e:
        print(f"❌ Decode error: {e}")

    print()
    print("=" * 60)
    print("✅ Example complete!")
    print("=" * 60)


if __name__ == "__main__":
    main()
