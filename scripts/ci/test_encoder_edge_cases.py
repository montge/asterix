#!/usr/bin/env python3
"""
Encoder Edge Case Tests

Tests encoder behavior with minimum, maximum, and boundary values.
"""

import sys


def test_minimum_values():
    """Test encoding with minimum valid values."""
    from asterix.radar_integration.encoder.cat048 import (
        encode_cat048_record,
        encode_cat048_datablock
    )

    record = encode_cat048_record(
        range_m=0.0,
        azimuth_deg=0.0,
        sac=0,
        sic=0
    )
    assert len(record) > 0, "Encoding failed for minimum values"

    datablock = encode_cat048_datablock([record])
    assert datablock[0] == 48, "Category must be 48"
    print(f"✓ Minimum values: {len(datablock)} bytes")
    return True


def test_maximum_values():
    """Test encoding with maximum valid values."""
    from asterix.radar_integration.encoder.cat048 import (
        encode_cat048_record,
        encode_cat048_datablock
    )

    record = encode_cat048_record(
        range_m=256000.0,  # ~256 km max range
        azimuth_deg=359.9,
        sac=255,
        sic=255,
        mode3a=0o7777  # Max Mode-3/A code
    )
    assert len(record) > 0, "Encoding failed for maximum values"

    datablock = encode_cat048_datablock([record])
    assert datablock[0] == 48, "Category must be 48"
    print(f"✓ Maximum values: {len(datablock)} bytes")
    return True


def test_multiple_records():
    """Test encoding multiple records in one data block."""
    from asterix.radar_integration.encoder.cat048 import (
        encode_cat048_record,
        encode_cat048_datablock
    )

    records = []
    for i in range(10):
        record = encode_cat048_record(
            range_m=10000.0 + i * 1000,
            azimuth_deg=i * 36.0,
            sac=i % 256,
            sic=(i + 1) % 256
        )
        records.append(record)

    datablock = encode_cat048_datablock(records)
    assert datablock[0] == 48, "Category must be 48"

    # Verify length field
    length = (datablock[1] << 8) | datablock[2]
    assert length == len(datablock), "Length mismatch"

    print(f"✓ Multiple records (10): {len(datablock)} bytes")
    return True


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

    print("Running encoder edge case tests...")

    tests = [
        ("Minimum values", test_minimum_values),
        ("Maximum values", test_maximum_values),
        ("Multiple records", test_multiple_records),
    ]

    passed = 0
    failed = 0

    for name, test_func in tests:
        try:
            if test_func():
                passed += 1
        except Exception as e:
            print(f"✗ {name}: {e}")
            failed += 1

    print()
    print(f"Results: {passed} passed, {failed} failed")

    if failed == 0:
        print("✅ All encoder edge case tests passed")
        return 0
    else:
        print("❌ Some tests failed")
        return 1


if __name__ == "__main__":
    sys.exit(main())
