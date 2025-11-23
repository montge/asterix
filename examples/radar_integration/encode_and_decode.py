#!/usr/bin/env python3
"""
Round-Trip Encode/Decode Example

Demonstrates complete encode → decode → validate pipeline.
Shows how to verify ASTERIX encoding accuracy and data preservation.

Usage:
    python3 encode_and_decode.py

Requirements:
    - Python 3.10+
    - asterix_decoder package installed (pip install asterix_decoder)
    - Mock radar generator (from .local/integration/)
"""

import sys
import os
import struct
import math

# Add path to mock radar generator
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../.local/integration'))

from mock_radar import MockRadar
from asterix_encoder.cat048 import encode_cat048


def validate_round_trip(original_plots, decoded_records):
    """
    Validate that encoded→decoded data preserves original values.

    Args:
        original_plots: List of original RadarPlot objects
        decoded_records: List of decoded ASTERIX records

    Returns:
        Tuple (success: bool, stats: dict)
    """
    if len(original_plots) != len(decoded_records):
        return False, {
            'error': f"Count mismatch: {len(original_plots)} plots != {len(decoded_records)} records"
        }

    range_errors = []
    azimuth_errors = []
    missing_items = {'I010': 0, 'I040': 0, 'I140': 0}

    for i, (plot, record) in enumerate(zip(original_plots, decoded_records)):
        # Check required data items
        if 'I010' not in record:
            missing_items['I010'] += 1
        if 'I140' not in record:
            missing_items['I140'] += 1

        # Check I040: Position
        if 'I040' in record:
            i040 = record['I040']

            # Range: NM to meters
            decoded_range = i040.get('RHO', 0) * 1852.0
            range_error = abs(decoded_range - plot.range)
            range_errors.append(range_error)

            # Azimuth: handle wrap-around
            decoded_azimuth = i040.get('THETA', 0)
            azimuth_error = abs(decoded_azimuth - plot.azimuth)
            if azimuth_error > 180:
                azimuth_error = 360 - azimuth_error
            azimuth_errors.append(azimuth_error)
        else:
            missing_items['I040'] += 1

    # Calculate statistics
    stats = {
        'total_records': len(original_plots),
        'range_errors': {
            'mean': sum(range_errors) / len(range_errors) if range_errors else 0,
            'max': max(range_errors) if range_errors else 0,
            'min': min(range_errors) if range_errors else 0,
        },
        'azimuth_errors': {
            'mean': sum(azimuth_errors) / len(azimuth_errors) if azimuth_errors else 0,
            'max': max(azimuth_errors) if azimuth_errors else 0,
            'min': min(azimuth_errors) if azimuth_errors else 0,
        },
        'missing_items': missing_items,
    }

    # Check tolerances
    max_range_error = stats['range_errors']['max']
    max_azimuth_error = stats['azimuth_errors']['max']

    success = (
        max_range_error < 500 and  # 500 meter tolerance
        max_azimuth_error < 0.5 and  # 0.5 degree tolerance
        missing_items['I040'] == 0  # All records must have position
    )

    return success, stats


def print_validation_report(success, stats):
    """Print detailed validation report."""
    print("\nValidation Report:")
    print("=" * 70)

    if success:
        print("✅ VALIDATION PASSED")
    else:
        print("❌ VALIDATION FAILED")

    print()
    print(f"Total records: {stats['total_records']}")
    print()

    print("Range Errors:")
    print(f"  Mean:  {stats['range_errors']['mean']:.2f} m")
    print(f"  Max:   {stats['range_errors']['max']:.2f} m")
    print(f"  Min:   {stats['range_errors']['min']:.2f} m")
    print()

    print("Azimuth Errors:")
    print(f"  Mean:  {stats['azimuth_errors']['mean']:.4f}°")
    print(f"  Max:   {stats['azimuth_errors']['max']:.4f}°")
    print(f"  Min:   {stats['azimuth_errors']['min']:.4f}°")
    print()

    if any(stats['missing_items'].values()):
        print("Missing Data Items:")
        for item, count in stats['missing_items'].items():
            if count > 0:
                print(f"  {item}: {count} records missing")
    else:
        print("✅ All required data items present")

    print("=" * 70)


def analyze_asterix_structure(data):
    """Analyze ASTERIX binary data structure."""
    print("\nASTERIX Data Structure Analysis:")
    print("-" * 70)

    if len(data) < 3:
        print("❌ Data too short")
        return

    # Parse header
    cat = data[0]
    length = struct.unpack('!H', data[1:3])[0]

    print(f"Category:      {cat}")
    print(f"Block length:  {length} bytes")
    print(f"Actual size:   {len(data)} bytes")

    if length != len(data):
        print(f"⚠️  Length mismatch!")

    # Estimate record count (simplified)
    payload = data[3:]
    estimated_records = len(payload) // 15  # Rough estimate (15 bytes/record avg)

    print(f"Payload:       {len(payload)} bytes")
    print(f"Est. records:  ~{estimated_records}")
    print()

    # Show hex dump (first 128 bytes)
    print("Hex dump (first 128 bytes):")
    for i in range(0, min(128, len(data)), 16):
        hex_str = ' '.join(f'{b:02x}' for b in data[i:i+16])
        ascii_str = ''.join(chr(b) if 32 <= b < 127 else '.' for b in data[i:i+16])
        print(f"  {i:04x}: {hex_str:<48} {ascii_str}")


def main():
    print("=" * 70)
    print("Round-Trip Encode/Decode Example")
    print("=" * 70)
    print()

    # Step 1: Generate radar data
    print("Step 1: Generate Radar Data")
    print("-" * 70)

    radar = MockRadar(
        lat=52.5,
        lon=13.4,
        alt=100.0,
        max_range=150e3,  # 150 km
        name="TEST_RADAR"
    )

    num_targets = 20
    plots = radar.generate_plots(
        num_targets=num_targets,
        add_noise=True,
        add_clutter=False
    )

    print(f"Generated {len(plots)} radar plots")
    print(f"Range: {min(p.range for p in plots)/1000:.1f} - {max(p.range for p in plots)/1000:.1f} km")
    print(f"Azimuth: {min(p.azimuth for p in plots):.1f}° - {max(p.azimuth for p in plots):.1f}°")
    print()

    # Step 2: Encode to ASTERIX
    print("Step 2: Encode to ASTERIX CAT048")
    print("-" * 70)

    asterix_data = encode_cat048(
        plots,
        radar_position=(52.5, 13.4, 100.0),
        sac=0,
        sic=1
    )

    print(f"Encoded: {len(plots)} plots → {len(asterix_data)} bytes")
    print(f"Compression: {(1 - len(asterix_data) / (len(plots) * 72)) * 100:.1f}%")
    print(f"  (Python RadarPlot object ≈72 bytes, ASTERIX record ≈{len(asterix_data)/len(plots):.1f} bytes)")

    analyze_asterix_structure(asterix_data)
    print()

    # Step 3: Decode ASTERIX
    print("Step 3: Decode ASTERIX Data")
    print("-" * 70)

    try:
        import asterix

        decoded = asterix.parse(asterix_data)
        print(f"✅ Decoded {len(decoded)} records")

        # Show sample decoded record
        if decoded:
            print("\nSample decoded record (first):")
            rec = decoded[0]
            print(f"  Category: {rec.get('category')}")
            print(f"  I010: SAC={rec.get('I010', {}).get('SAC')}, SIC={rec.get('I010', {}).get('SIC')}")

            if 'I140' in rec:
                print(f"  I140 (Time): {rec['I140']}")

            if 'I040' in rec:
                i040 = rec['I040']
                print(f"  I040 (Position):")
                print(f"    RHO:   {i040.get('RHO')} NM ({i040.get('RHO') * 1852:.0f} m)")
                print(f"    THETA: {i040.get('THETA')}°")

        print()

        # Step 4: Validate round-trip
        print("Step 4: Validate Round-Trip Accuracy")
        print("-" * 70)

        success, stats = validate_round_trip(plots, decoded)
        print_validation_report(success, stats)

        # Step 5: Detailed comparison (first 5 records)
        print("\nDetailed Comparison (first 5 records):")
        print("-" * 70)
        print(f"{'#':<3} {'Original Range':<15} {'Decoded Range':<15} {'Error':<12} {'Az Error':<10}")
        print("-" * 70)

        for i in range(min(5, len(plots))):
            plot = plots[i]
            record = decoded[i]

            if 'I040' in record:
                i040 = record['I040']
                decoded_range = i040.get('RHO', 0) * 1852.0
                decoded_azimuth = i040.get('THETA', 0)

                range_error = abs(decoded_range - plot.range)
                azimuth_error = abs(decoded_azimuth - plot.azimuth)
                if azimuth_error > 180:
                    azimuth_error = 360 - azimuth_error

                print(f"{i:<3} {plot.range/1000:>6.2f} km     "
                      f"{decoded_range/1000:>6.2f} km     "
                      f"{range_error:>7.2f} m    "
                      f"{azimuth_error:>6.4f}°")

    except ImportError:
        print("❌ ASTERIX decoder not installed")
        print("   Install with: pip install asterix_decoder")
        print()
        print("   However, encoding completed successfully!")
        print(f"   Generated {len(asterix_data)} bytes of valid ASTERIX data")

    except Exception as e:
        print(f"❌ Error during decode/validation: {e}")
        import traceback
        traceback.print_exc()

    print()
    print("=" * 70)
    print("Example complete!")
    print("=" * 70)


if __name__ == "__main__":
    main()
