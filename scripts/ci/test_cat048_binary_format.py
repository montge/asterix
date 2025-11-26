#!/usr/bin/env python3
"""
CAT048 Binary Format Validation

Verifies that encoded ASTERIX CAT048 data follows the binary format specification.
"""

import sys


def main():
    try:
        from asterix.radar_integration.encoder.cat048 import (
            encode_cat048_record,
            encode_cat048_datablock
        )
    except ImportError as e:
        print(f"Import error: {e}")
        print("Make sure to run: python setup.py install")
        return 1

    # Create a test record
    record = encode_cat048_record(
        range_m=10000.0,
        azimuth_deg=45.0,
        sac=1,
        sic=2,
        mode3a=0o1234
    )

    # Create data block
    data = encode_cat048_datablock([record])

    # Verify ASTERIX data block structure
    print("Validating CAT048 binary format structure...")

    # Check category byte
    assert data[0] == 48, f"Category must be 48, got {data[0]}"
    print(f"✓ Category: {data[0]}")

    # Length is in bytes 1-2 (big-endian 16-bit)
    length = (data[1] << 8) | data[2]
    print(f"✓ Length field: {length} bytes")
    assert length == len(data), f"Length mismatch: field={length}, actual={len(data)}"

    # FSPEC starts at byte 3
    fspec_byte = data[3]
    print(f"✓ FSPEC: 0x{fspec_byte:02x} (binary: {bin(fspec_byte)})")

    # Verify minimum size
    assert len(data) >= 4, f"Data block too small: {len(data)} bytes"

    print("✅ CAT048 binary format validation passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
