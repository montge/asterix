#!/usr/bin/env python3
"""
Multi-Aircraft Scenario Example

Demonstrates realistic aircraft simulation with multiple moving targets.
Generates aircraft at various ranges, speeds, and headings.

Usage:
    python3 aircraft_scenario.py

Requirements:
    - Python 3.10+
    - asterix_decoder package installed (pip install asterix_decoder)
    - Mock radar generator (from .local/integration/)
"""

import sys
import os
import time

# Add path to mock radar generator
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../.local/integration'))

from mock_radar import MockRadar, generate_aircraft_scenario
from asterix_encoder.cat048 import encode_cat048


def print_scenario_stats(plots):
    """Print statistics about the generated scenario."""
    if not plots:
        return

    ranges = [p.range for p in plots]
    azimuths = [p.azimuth for p in plots]
    snrs = [p.snr for p in plots]

    print("\nScenario Statistics:")
    print(f"  Total plots:      {len(plots)}")
    print(f"  Range:            {min(ranges)/1000:.1f} - {max(ranges)/1000:.1f} km")
    print(f"  Azimuth:          {min(azimuths):.1f}° - {max(azimuths):.1f}°")
    print(f"  SNR:              {min(snrs):.1f} - {max(snrs):.1f} dB")
    print(f"  Time span:        {plots[-1].timestamp - plots[0].timestamp:.1f} seconds")


def analyze_tracks(plots):
    """Group plots by likely aircraft (simple time-based grouping)."""
    # Sort by timestamp
    sorted_plots = sorted(plots, key=lambda p: p.timestamp)

    # Simple grouping by time windows (4 second radar rotation)
    time_windows = {}
    for plot in sorted_plots:
        window = int(plot.timestamp / 4.0) * 4  # Round to 4s window
        if window not in time_windows:
            time_windows[window] = []
        time_windows[window].append(plot)

    print("\nTime-Based Analysis:")
    print(f"  Total time windows: {len(time_windows)}")
    for window, window_plots in sorted(time_windows.items())[:5]:  # Show first 5
        print(f"  t={window}s: {len(window_plots)} plots")


def main():
    print("=" * 70)
    print("Multi-Aircraft Scenario Example")
    print("=" * 70)
    print()

    # Configuration
    num_aircraft = 5
    duration = 60.0  # 60 seconds
    radar_lat = 52.5  # Berlin
    radar_lon = 13.4
    radar_alt = 100.0

    # Step 1: Generate aircraft scenario
    print("Step 1: Generate Multi-Aircraft Scenario")
    print("-" * 70)
    print(f"Parameters:")
    print(f"  Number of aircraft: {num_aircraft}")
    print(f"  Duration:           {duration} seconds")
    print(f"  Radar position:     ({radar_lat}°, {radar_lon}°, {radar_alt}m)")
    print()

    print("Generating aircraft trajectories...")
    start_time = time.time()
    plots = generate_aircraft_scenario(
        num_aircraft=num_aircraft,
        duration=duration,
        radar_lat=radar_lat,
        radar_lon=radar_lon,
        radar_alt=radar_alt
    )
    gen_time = time.time() - start_time

    print(f"✅ Generated {len(plots)} plots in {gen_time*1000:.1f} ms")
    print(f"   Average: {len(plots) / num_aircraft:.1f} plots/aircraft")
    print_scenario_stats(plots)
    analyze_tracks(plots)
    print()

    # Step 2: Display sample trajectories
    print("Step 2: Sample Aircraft Trajectories")
    print("-" * 70)

    # Group plots by aircraft (approximate via timestamp clusters)
    sorted_plots = sorted(plots, key=lambda p: p.timestamp)

    # Show first few plots of first 3 "aircraft"
    plots_per_rotation = len(plots) // (duration / 4.0) // num_aircraft
    approx_plots_per_aircraft = int(plots_per_rotation * (duration / 4.0))

    for aircraft_id in range(min(3, num_aircraft)):
        start_idx = aircraft_id * approx_plots_per_aircraft
        end_idx = start_idx + 3  # Show first 3 updates

        if end_idx > len(sorted_plots):
            break

        print(f"\nAircraft {aircraft_id + 1} (sample trajectory):")
        for i in range(start_idx, min(end_idx, len(sorted_plots))):
            p = sorted_plots[i]
            print(f"  t={p.timestamp - sorted_plots[0].timestamp:5.1f}s: "
                  f"Range={p.range/1000:6.1f}km, "
                  f"Az={p.azimuth:6.1f}°, "
                  f"Doppler={p.doppler:+6.1f}Hz")

    print()

    # Step 3: Encode to ASTERIX CAT048
    print("Step 3: Encode to ASTERIX CAT048")
    print("-" * 70)

    start_time = time.time()
    asterix_data = encode_cat048(
        plots,
        radar_position=(radar_lat, radar_lon, radar_alt),
        sac=0,
        sic=1,
        include_mode3a=False,
        include_aircraft_address=False
    )
    encode_time = time.time() - start_time

    print(f"Encoded {len(plots)} plots → {len(asterix_data)} bytes")
    print(f"Encoding time: {encode_time*1000:.1f} ms")
    print(f"Throughput: {len(plots) / encode_time:.0f} plots/second")
    print(f"Average: {len(asterix_data) / len(plots):.1f} bytes/plot")
    print()

    # Step 4: Save to file
    print("Step 4: Save ASTERIX Data")
    print("-" * 70)

    output_file = "aircraft_scenario.ast"
    with open(output_file, 'wb') as f:
        f.write(asterix_data)

    print(f"Saved to: {output_file}")
    print(f"File size: {len(asterix_data)} bytes")
    print()

    # Step 5: Decode and verify
    print("Step 5: Decode and Verify")
    print("-" * 70)

    try:
        import asterix

        start_time = time.time()
        decoded = asterix.parse(asterix_data)
        decode_time = time.time() - start_time

        print(f"✅ Successfully decoded {len(decoded)} records")
        print(f"Decoding time: {decode_time*1000:.1f} ms")
        print(f"Throughput: {len(decoded) / decode_time:.0f} records/second")
        print()

        # Verify data preservation
        print("Data Preservation Check:")
        errors = 0
        max_range_error = 0.0
        max_azimuth_error = 0.0

        for plot, record in zip(plots, decoded):
            if 'I040' in record:
                i040 = record['I040']
                decoded_range = i040['RHO'] * 1852.0  # NM to meters
                decoded_azimuth = i040['THETA']

                range_error = abs(decoded_range - plot.range)
                azimuth_error = abs(decoded_azimuth - plot.azimuth)

                max_range_error = max(max_range_error, range_error)
                max_azimuth_error = max(max_azimuth_error, azimuth_error)

                if range_error > 500 or azimuth_error > 0.5:
                    errors += 1

        if errors == 0:
            print(f"  ✅ All records within tolerance")
            print(f"  Max range error:   {max_range_error:.2f} m")
            print(f"  Max azimuth error: {max_azimuth_error:.4f}°")
        else:
            print(f"  ⚠️  {errors} records exceeded tolerance")

    except ImportError:
        print("⚠️  ASTERIX decoder not installed")
        print("   Install with: pip install asterix_decoder")

    except Exception as e:
        print(f"❌ Decode error: {e}")

    print()
    print("=" * 70)
    print("✅ Multi-aircraft scenario complete!")
    print("=" * 70)
    print()
    print("Next steps:")
    print("  - View generated file: hexdump -C aircraft_scenario.ast")
    print("  - Parse with CLI:      asterix -f aircraft_scenario.ast -j")
    print("  - Modify parameters:   Edit num_aircraft, duration variables")


if __name__ == "__main__":
    main()
