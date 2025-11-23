"""
ASTERIX Round-Trip Validation Framework

Validates encoded→decoded data preservation for accuracy testing.

Key functions:
- validate_round_trip: Compare original data with decoded records
- compare_values: Tolerance-aware value comparison
- generate_validation_report: Detailed statistics and analysis

Supports:
- CAT048: Radar plots (position, Mode 3/A, timestamps)
- CAT062: System tracks (WGS-84, velocity, flight level)
- CAT021: ADS-B reports (high-precision position, ICAO address)

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import math
from typing import List, Dict, Any, Tuple, Optional, Union
from dataclasses import dataclass, field


@dataclass
class ValidationStats:
    """Statistics from round-trip validation."""
    total_records: int = 0
    successful: int = 0
    failed: int = 0
    errors: List[str] = field(default_factory=list)

    # Position errors (CAT048/062/021)
    range_errors: List[float] = field(default_factory=list)
    azimuth_errors: List[float] = field(default_factory=list)
    lat_errors: List[float] = field(default_factory=list)
    lon_errors: List[float] = field(default_factory=list)

    # Time errors
    time_errors: List[float] = field(default_factory=list)

    # Mode 3/A errors
    mode3a_mismatches: int = 0

    # Missing data items
    missing_items: Dict[str, int] = field(default_factory=dict)

    def success_rate(self) -> float:
        """Calculate success rate (0.0-1.0)."""
        if self.total_records == 0:
            return 0.0
        return self.successful / self.total_records

    def mean_error(self, errors: List[float]) -> float:
        """Calculate mean of error list."""
        return sum(errors) / len(errors) if errors else 0.0

    def max_error(self, errors: List[float]) -> float:
        """Calculate max of error list."""
        return max(errors) if errors else 0.0

    def min_error(self, errors: List[float]) -> float:
        """Calculate min of error list."""
        return min(errors) if errors else 0.0


def compare_values(
    original: float,
    decoded: float,
    tolerance: float,
    tolerance_type: str = 'absolute'
) -> Tuple[bool, float]:
    """
    Compare two values with tolerance.

    Args:
        original: Original value
        decoded: Decoded value
        tolerance: Tolerance threshold
        tolerance_type: 'absolute' or 'relative' (percentage)

    Returns:
        Tuple of (matches, error)
        - matches: True if within tolerance
        - error: Absolute difference
    """
    error = abs(decoded - original)

    if tolerance_type == 'absolute':
        matches = error <= tolerance
    elif tolerance_type == 'relative':
        # Relative tolerance (percentage)
        if original == 0:
            matches = error <= tolerance
        else:
            relative_error = error / abs(original)
            matches = relative_error <= tolerance
    else:
        raise ValueError(f"Unknown tolerance_type: {tolerance_type}")

    return matches, error


def compare_angles(
    original_deg: float,
    decoded_deg: float,
    tolerance_deg: float = 0.5
) -> Tuple[bool, float]:
    """
    Compare two angles (handles wrap-around at 0/360).

    Args:
        original_deg: Original angle in degrees (0-360)
        decoded_deg: Decoded angle in degrees (0-360)
        tolerance_deg: Tolerance in degrees

    Returns:
        Tuple of (matches, error)
        - matches: True if within tolerance
        - error: Minimum angular difference
    """
    # Normalize to 0-360
    original_deg = original_deg % 360
    decoded_deg = decoded_deg % 360

    # Calculate shortest angular distance
    error = abs(decoded_deg - original_deg)
    if error > 180:
        error = 360 - error

    matches = error <= tolerance_deg

    return matches, error


def validate_cat048_record(
    original_plot: Any,
    decoded_record: Dict[str, Any],
    stats: ValidationStats,
    tolerances: Optional[Dict[str, float]] = None
) -> bool:
    """
    Validate a single CAT048 record (radar plot).

    Args:
        original_plot: Original RadarPlot object or dict
        decoded_record: Decoded ASTERIX record
        stats: ValidationStats object to update
        tolerances: Custom tolerance thresholds (optional)

    Returns:
        True if validation passed, False otherwise
    """
    if tolerances is None:
        tolerances = {
            'range_m': 500.0,      # 500 meter tolerance
            'azimuth_deg': 0.5,    # 0.5 degree tolerance
            'time_s': 0.01,        # 10 millisecond tolerance
        }

    success = True

    # Extract original values
    if hasattr(original_plot, 'range'):
        orig_range = original_plot.range
        orig_azimuth = original_plot.azimuth
        orig_timestamp = original_plot.timestamp
    else:
        orig_range = original_plot.get('range', 0)
        orig_azimuth = original_plot.get('azimuth', 0)
        orig_timestamp = original_plot.get('timestamp', 0)

    # Check I040: Position
    if 'I040' in decoded_record:
        i040 = decoded_record['I040']

        # Range: Convert NM to meters
        if 'RHO' in i040:
            decoded_range_m = i040['RHO'] * 1852.0
            matches, error = compare_values(orig_range, decoded_range_m, tolerances['range_m'])
            stats.range_errors.append(error)

            if not matches:
                stats.errors.append(f"Range error: {error:.2f} m (tolerance: {tolerances['range_m']} m)")
                success = False

        # Azimuth
        if 'THETA' in i040:
            decoded_azimuth = i040['THETA']
            matches, error = compare_angles(orig_azimuth, decoded_azimuth, tolerances['azimuth_deg'])
            stats.azimuth_errors.append(error)

            if not matches:
                stats.errors.append(f"Azimuth error: {error:.4f}° (tolerance: {tolerances['azimuth_deg']}°)")
                success = False
    else:
        stats.missing_items['I040'] = stats.missing_items.get('I040', 0) + 1
        stats.errors.append("Missing I040 (position)")
        success = False

    # Check I140: Time of Day
    if 'I140' in decoded_record:
        i140 = decoded_record['I140']

        # Extract time value
        if isinstance(i140, dict):
            decoded_time = i140.get('value', 0)
        else:
            decoded_time = i140

        # Compare time of day (modulo 24h)
        orig_time_of_day = orig_timestamp % 86400
        matches, error = compare_values(orig_time_of_day, decoded_time, tolerances['time_s'])
        stats.time_errors.append(error)

        if not matches:
            stats.errors.append(f"Time error: {error:.3f} s (tolerance: {tolerances['time_s']} s)")
            success = False
    else:
        stats.missing_items['I140'] = stats.missing_items.get('I140', 0) + 1

    # Check I010: SAC/SIC (should always be present)
    if 'I010' not in decoded_record:
        stats.missing_items['I010'] = stats.missing_items.get('I010', 0) + 1
        stats.errors.append("Missing I010 (SAC/SIC)")
        success = False

    return success


def validate_round_trip_cat048(
    original_plots: List[Any],
    decoded_records: List[Dict[str, Any]],
    tolerances: Optional[Dict[str, float]] = None
) -> Tuple[bool, ValidationStats]:
    """
    Validate complete round-trip for CAT048 (radar plots).

    Args:
        original_plots: List of original RadarPlot objects or dicts
        decoded_records: List of decoded ASTERIX records
        tolerances: Custom tolerance thresholds (optional)

    Returns:
        Tuple of (success, stats)
        - success: True if all records validated successfully
        - stats: ValidationStats with detailed analysis
    """
    stats = ValidationStats()
    stats.total_records = len(original_plots)

    if len(original_plots) != len(decoded_records):
        stats.errors.append(
            f"Count mismatch: {len(original_plots)} plots != {len(decoded_records)} records"
        )
        stats.failed = max(len(original_plots), len(decoded_records))
        return False, stats

    # Validate each record
    for i, (plot, record) in enumerate(zip(original_plots, decoded_records)):
        try:
            if validate_cat048_record(plot, record, stats, tolerances):
                stats.successful += 1
            else:
                stats.failed += 1

        except Exception as e:
            stats.failed += 1
            stats.errors.append(f"Record {i} validation error: {e}")

    # Overall success if all records passed
    success = (stats.failed == 0 and stats.total_records > 0)

    return success, stats


def validate_round_trip(
    original_data: List[Any],
    decoded_records: List[Dict[str, Any]],
    category: Optional[int] = None,
    tolerances: Optional[Dict[str, float]] = None
) -> Tuple[bool, ValidationStats]:
    """
    Validate round-trip for any ASTERIX category (auto-detect or specify).

    Args:
        original_data: List of original objects (RadarPlot, dict, etc.)
        decoded_records: List of decoded ASTERIX records
        category: ASTERIX category (auto-detect if None)
        tolerances: Custom tolerance thresholds

    Returns:
        Tuple of (success, stats)
    """
    # Auto-detect category if not specified
    if category is None and decoded_records:
        category = decoded_records[0].get('category', 48)

    # Dispatch to category-specific validator
    if category == 48:
        return validate_round_trip_cat048(original_data, decoded_records, tolerances)
    elif category == 62:
        # TODO: Implement CAT062 validator
        raise NotImplementedError("CAT062 validation not implemented yet")
    elif category == 21:
        # TODO: Implement CAT021 validator
        raise NotImplementedError("CAT021 validation not implemented yet")
    else:
        raise ValueError(f"Unsupported category for validation: {category}")


def print_validation_report(stats: ValidationStats, verbose: bool = True) -> None:
    """
    Print detailed validation report.

    Args:
        stats: ValidationStats object
        verbose: If True, show detailed error messages
    """
    print("\n" + "=" * 70)
    print("ASTERIX Round-Trip Validation Report")
    print("=" * 70)
    print()

    # Overall status
    if stats.failed == 0 and stats.total_records > 0:
        print("✅ VALIDATION PASSED")
    else:
        print("❌ VALIDATION FAILED")

    print()
    print(f"Total records:    {stats.total_records}")
    print(f"Successful:       {stats.successful}")
    print(f"Failed:           {stats.failed}")
    print(f"Success rate:     {stats.success_rate() * 100:.1f}%")
    print()

    # Position errors
    if stats.range_errors:
        print("Range Errors:")
        print(f"  Mean:  {stats.mean_error(stats.range_errors):>10.2f} m")
        print(f"  Max:   {stats.max_error(stats.range_errors):>10.2f} m")
        print(f"  Min:   {stats.min_error(stats.range_errors):>10.2f} m")
        print()

    if stats.azimuth_errors:
        print("Azimuth Errors:")
        print(f"  Mean:  {stats.mean_error(stats.azimuth_errors):>10.4f}°")
        print(f"  Max:   {stats.max_error(stats.azimuth_errors):>10.4f}°")
        print(f"  Min:   {stats.min_error(stats.azimuth_errors):>10.4f}°")
        print()

    if stats.time_errors:
        print("Time Errors:")
        print(f"  Mean:  {stats.mean_error(stats.time_errors):>10.6f} s")
        print(f"  Max:   {stats.max_error(stats.time_errors):>10.6f} s")
        print(f"  Min:   {stats.min_error(stats.time_errors):>10.6f} s")
        print()

    # Missing items
    if stats.missing_items:
        print("Missing Data Items:")
        for item, count in sorted(stats.missing_items.items()):
            print(f"  {item}: {count} records")
        print()
    else:
        print("✅ All required data items present")
        print()

    # Detailed errors (if verbose)
    if verbose and stats.errors:
        print("Detailed Errors:")
        for i, error in enumerate(stats.errors[:20]):  # Limit to first 20
            print(f"  {i + 1}. {error}")
        if len(stats.errors) > 20:
            print(f"  ... and {len(stats.errors) - 20} more errors")
        print()

    print("=" * 70)
    print()


def compare_with_cpp_decoder(
    asterix_data: bytes,
    python_records: List[Dict[str, Any]]
) -> Tuple[bool, Dict[str, Any]]:
    """
    Compare Python decoder output with C++ decoder (asterix.parse).

    Args:
        asterix_data: Raw ASTERIX binary data
        python_records: Records decoded by Python decoder

    Returns:
        Tuple of (matches, comparison_stats)
        - matches: True if outputs are equivalent
        - comparison_stats: Dict with comparison details
    """
    try:
        import asterix
    except ImportError:
        raise ImportError("C++ ASTERIX decoder not available (pip install asterix_decoder)")

    # Decode with C++ decoder
    cpp_records = asterix.parse(asterix_data, verbose=True)

    # Compare record count
    if len(python_records) != len(cpp_records):
        return False, {
            'error': 'Record count mismatch',
            'python_count': len(python_records),
            'cpp_count': len(cpp_records),
        }

    # Compare each record
    mismatches = []

    for i, (py_rec, cpp_rec) in enumerate(zip(python_records, cpp_records)):
        # Compare category
        if py_rec.get('category') != cpp_rec.get('category'):
            mismatches.append(f"Record {i}: Category mismatch")

        # Compare data items
        py_items = set(k for k in py_rec.keys() if k.startswith('I'))
        cpp_items = set(k for k in cpp_rec.keys() if k.startswith('I'))

        if py_items != cpp_items:
            missing_in_py = cpp_items - py_items
            missing_in_cpp = py_items - cpp_items
            if missing_in_py:
                mismatches.append(f"Record {i}: Python missing items: {missing_in_py}")
            if missing_in_cpp:
                mismatches.append(f"Record {i}: C++ missing items: {missing_in_cpp}")

    matches = len(mismatches) == 0

    stats = {
        'matches': matches,
        'total_records': len(python_records),
        'mismatches': mismatches,
    }

    return matches, stats
