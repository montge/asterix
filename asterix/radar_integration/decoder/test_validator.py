#!/usr/bin/env python3
"""
Unit tests for ASTERIX validator module

Tests:
- ValidationStats class and methods
- compare_values (absolute/relative tolerance)
- compare_angles (wrap-around handling)
- validate_cat048_record
- validate_round_trip_cat048
- validate_round_trip (dispatcher)
- print_validation_report
- compare_with_cpp_decoder

Coverage target: >80%

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-12-06
"""

import unittest
import sys
import os
from io import StringIO
from unittest.mock import patch, MagicMock

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', '..'))

from asterix.radar_integration.decoder.validator import (
    ValidationStats,
    compare_values,
    compare_angles,
    validate_cat048_record,
    validate_round_trip_cat048,
    validate_round_trip,
    print_validation_report,
    compare_with_cpp_decoder,
)


class TestValidationStats(unittest.TestCase):
    """Test ValidationStats dataclass."""

    def test_initialization_defaults(self):
        """Test ValidationStats initialization with defaults."""
        stats = ValidationStats()

        self.assertEqual(stats.total_records, 0)
        self.assertEqual(stats.successful, 0)
        self.assertEqual(stats.failed, 0)
        self.assertEqual(stats.errors, [])
        self.assertEqual(stats.range_errors, [])
        self.assertEqual(stats.azimuth_errors, [])
        self.assertEqual(stats.lat_errors, [])
        self.assertEqual(stats.lon_errors, [])
        self.assertEqual(stats.time_errors, [])
        self.assertEqual(stats.mode3a_mismatches, 0)
        self.assertEqual(stats.missing_items, {})

    def test_initialization_with_values(self):
        """Test ValidationStats initialization with custom values."""
        stats = ValidationStats(
            total_records=10,
            successful=8,
            failed=2,
            errors=['error1', 'error2']
        )

        self.assertEqual(stats.total_records, 10)
        self.assertEqual(stats.successful, 8)
        self.assertEqual(stats.failed, 2)
        self.assertEqual(len(stats.errors), 2)

    def test_success_rate_all_successful(self):
        """Test success_rate with all records successful."""
        stats = ValidationStats(total_records=10, successful=10, failed=0)
        self.assertAlmostEqual(stats.success_rate(), 1.0)

    def test_success_rate_partial_success(self):
        """Test success_rate with partial success."""
        stats = ValidationStats(total_records=10, successful=7, failed=3)
        self.assertAlmostEqual(stats.success_rate(), 0.7)

    def test_success_rate_zero_records(self):
        """Test success_rate with zero records."""
        stats = ValidationStats()
        self.assertEqual(stats.success_rate(), 0.0)

    def test_mean_error_with_data(self):
        """Test mean_error calculation."""
        stats = ValidationStats()
        errors = [1.0, 2.0, 3.0, 4.0, 5.0]
        mean = stats.mean_error(errors)
        self.assertAlmostEqual(mean, 3.0)

    def test_mean_error_empty_list(self):
        """Test mean_error with empty list."""
        stats = ValidationStats()
        mean = stats.mean_error([])
        self.assertEqual(mean, 0.0)

    def test_max_error_with_data(self):
        """Test max_error calculation."""
        stats = ValidationStats()
        errors = [1.0, 5.0, 3.0, 2.0]
        max_err = stats.max_error(errors)
        self.assertEqual(max_err, 5.0)

    def test_max_error_empty_list(self):
        """Test max_error with empty list."""
        stats = ValidationStats()
        max_err = stats.max_error([])
        self.assertEqual(max_err, 0.0)

    def test_min_error_with_data(self):
        """Test min_error calculation."""
        stats = ValidationStats()
        errors = [1.0, 5.0, 3.0, 2.0]
        min_err = stats.min_error(errors)
        self.assertEqual(min_err, 1.0)

    def test_min_error_empty_list(self):
        """Test min_error with empty list."""
        stats = ValidationStats()
        min_err = stats.min_error([])
        self.assertEqual(min_err, 0.0)


class TestCompareValues(unittest.TestCase):
    """Test compare_values function."""

    def test_absolute_tolerance_within(self):
        """Test absolute tolerance comparison within bounds."""
        matches, error = compare_values(10.0, 10.5, 1.0, 'absolute')
        self.assertTrue(matches)
        self.assertAlmostEqual(error, 0.5)

    def test_absolute_tolerance_exact_match(self):
        """Test absolute tolerance with exact match."""
        matches, error = compare_values(10.0, 10.0, 0.1, 'absolute')
        self.assertTrue(matches)
        self.assertEqual(error, 0.0)

    def test_absolute_tolerance_outside(self):
        """Test absolute tolerance comparison outside bounds."""
        matches, error = compare_values(10.0, 12.0, 1.0, 'absolute')
        self.assertFalse(matches)
        self.assertAlmostEqual(error, 2.0)

    def test_absolute_tolerance_boundary(self):
        """Test absolute tolerance at exact boundary."""
        matches, error = compare_values(10.0, 11.0, 1.0, 'absolute')
        self.assertTrue(matches)
        self.assertAlmostEqual(error, 1.0)

    def test_relative_tolerance_within(self):
        """Test relative tolerance within bounds."""
        # 5% tolerance, error is 1/100 = 1%
        matches, error = compare_values(100.0, 101.0, 0.05, 'relative')
        self.assertTrue(matches)
        self.assertAlmostEqual(error, 1.0)

    def test_relative_tolerance_outside(self):
        """Test relative tolerance outside bounds."""
        # 1% tolerance, error is 10/100 = 10%
        matches, error = compare_values(100.0, 110.0, 0.01, 'relative')
        self.assertFalse(matches)
        self.assertAlmostEqual(error, 10.0)

    def test_relative_tolerance_zero_original(self):
        """Test relative tolerance with zero original value."""
        # Falls back to absolute comparison
        matches, error = compare_values(0.0, 0.5, 1.0, 'relative')
        self.assertTrue(matches)
        self.assertAlmostEqual(error, 0.5)

    def test_relative_tolerance_negative_values(self):
        """Test relative tolerance with negative values."""
        matches, error = compare_values(-100.0, -105.0, 0.1, 'relative')
        self.assertTrue(matches)
        self.assertAlmostEqual(error, 5.0)

    def test_invalid_tolerance_type(self):
        """Test invalid tolerance type raises ValueError."""
        with self.assertRaises(ValueError) as context:
            compare_values(10.0, 10.5, 1.0, 'invalid')
        self.assertIn("Unknown tolerance_type", str(context.exception))

    def test_negative_to_positive_comparison(self):
        """Test comparison with sign change."""
        matches, error = compare_values(-10.0, 10.0, 15.0, 'absolute')
        self.assertFalse(matches)
        self.assertAlmostEqual(error, 20.0)


class TestCompareAngles(unittest.TestCase):
    """Test compare_angles function."""

    def test_angles_within_tolerance(self):
        """Test angles within tolerance."""
        matches, error = compare_angles(45.0, 45.3, 0.5)
        self.assertTrue(matches)
        self.assertAlmostEqual(error, 0.3)

    def test_angles_exact_match(self):
        """Test angles with exact match."""
        matches, error = compare_angles(90.0, 90.0, 0.1)
        self.assertTrue(matches)
        self.assertEqual(error, 0.0)

    def test_angles_outside_tolerance(self):
        """Test angles outside tolerance."""
        matches, error = compare_angles(45.0, 46.0, 0.5)
        self.assertFalse(matches)
        self.assertAlmostEqual(error, 1.0)

    def test_angles_wrap_around_0_360(self):
        """Test angle wrap-around at 0/360 boundary."""
        # 359° to 1° = 2° difference (not 358°)
        matches, error = compare_angles(359.0, 1.0, 3.0)
        self.assertTrue(matches)
        self.assertAlmostEqual(error, 2.0)

    def test_angles_wrap_around_360_0(self):
        """Test angle wrap-around from 360 to 0."""
        # 1° to 359° = 2° difference
        matches, error = compare_angles(1.0, 359.0, 3.0)
        self.assertTrue(matches)
        self.assertAlmostEqual(error, 2.0)

    def test_angles_180_degree_difference(self):
        """Test angles at maximum difference (180°)."""
        matches, error = compare_angles(0.0, 180.0, 1.0)
        self.assertFalse(matches)
        self.assertAlmostEqual(error, 180.0)

    def test_angles_normalization_above_360(self):
        """Test angle normalization for values > 360."""
        # 370° normalizes to 10°
        matches, error = compare_angles(370.0, 10.0, 0.5)
        self.assertTrue(matches)
        self.assertAlmostEqual(error, 0.0)

    def test_angles_normalization_negative(self):
        """Test angle normalization for negative values."""
        # -10° normalizes to 350°
        matches, error = compare_angles(-10.0, 350.0, 0.5)
        self.assertTrue(matches)
        self.assertAlmostEqual(error, 0.0)

    def test_angles_default_tolerance(self):
        """Test default tolerance (0.5 degrees)."""
        matches, error = compare_angles(45.0, 45.4)
        self.assertTrue(matches)

        matches, error = compare_angles(45.0, 45.6)
        self.assertFalse(matches)


class TestValidateCat048Record(unittest.TestCase):
    """Test validate_cat048_record function."""

    def setUp(self):
        """Set up common test data."""
        self.stats = ValidationStats()

    def test_validate_successful_record_with_dict(self):
        """Test successful validation with dict original data."""
        original = {
            'range': 10000.0,  # meters
            'azimuth': 45.0,   # degrees
            'timestamp': 3600.0  # seconds
        }

        decoded = {
            'I010': {'SAC': 1, 'SIC': 2},
            'I040': {
                'RHO': 10000.0 / 1852.0,  # Convert to NM
                'THETA': 45.0
            },
            'I140': {'value': 3600.0}
        }

        result = validate_cat048_record(original, decoded, self.stats)

        self.assertTrue(result)
        self.assertEqual(len(self.stats.range_errors), 1)
        self.assertEqual(len(self.stats.azimuth_errors), 1)
        self.assertEqual(len(self.stats.time_errors), 1)

    def test_validate_successful_record_with_object(self):
        """Test successful validation with object original data."""
        class MockPlot:
            def __init__(self):
                self.range = 5000.0
                self.azimuth = 90.0
                self.timestamp = 7200.0

        original = MockPlot()

        decoded = {
            'I010': {'SAC': 1, 'SIC': 2},
            'I040': {
                'RHO': 5000.0 / 1852.0,
                'THETA': 90.0
            },
            'I140': {'value': 7200.0}
        }

        result = validate_cat048_record(original, decoded, self.stats)

        self.assertTrue(result)

    def test_validate_range_error_exceeds_tolerance(self):
        """Test validation fails when range error exceeds tolerance."""
        original = {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0}

        decoded = {
            'I010': {'SAC': 1, 'SIC': 2},
            'I040': {
                'RHO': 11000.0 / 1852.0,  # 1000m error
                'THETA': 45.0
            },
            'I140': {'value': 3600.0}
        }

        tolerances = {'range_m': 500.0, 'azimuth_deg': 0.5, 'time_s': 0.01}
        result = validate_cat048_record(original, decoded, self.stats, tolerances)

        self.assertFalse(result)
        self.assertGreater(len(self.stats.errors), 0)
        self.assertIn('Range error', self.stats.errors[0])

    def test_validate_azimuth_error_exceeds_tolerance(self):
        """Test validation fails when azimuth error exceeds tolerance."""
        original = {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0}

        decoded = {
            'I010': {'SAC': 1, 'SIC': 2},
            'I040': {
                'RHO': 10000.0 / 1852.0,
                'THETA': 46.0  # 1 degree error
            },
            'I140': {'value': 3600.0}
        }

        tolerances = {'range_m': 500.0, 'azimuth_deg': 0.5, 'time_s': 0.01}
        result = validate_cat048_record(original, decoded, self.stats, tolerances)

        self.assertFalse(result)
        self.assertIn('Azimuth error', self.stats.errors[0])

    def test_validate_time_error_exceeds_tolerance(self):
        """Test validation fails when time error exceeds tolerance."""
        original = {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0}

        decoded = {
            'I010': {'SAC': 1, 'SIC': 2},
            'I040': {
                'RHO': 10000.0 / 1852.0,
                'THETA': 45.0
            },
            'I140': {'value': 3600.5}  # 0.5s error
        }

        tolerances = {'range_m': 500.0, 'azimuth_deg': 0.5, 'time_s': 0.01}
        result = validate_cat048_record(original, decoded, self.stats, tolerances)

        self.assertFalse(result)
        self.assertIn('Time error', self.stats.errors[0])

    def test_validate_missing_i040(self):
        """Test validation fails when I040 (position) is missing."""
        original = {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0}

        decoded = {
            'I010': {'SAC': 1, 'SIC': 2},
            'I140': {'value': 3600.0}
        }

        result = validate_cat048_record(original, decoded, self.stats)

        self.assertFalse(result)
        self.assertEqual(self.stats.missing_items.get('I040', 0), 1)
        self.assertIn('Missing I040', self.stats.errors[0])

    def test_validate_missing_i010(self):
        """Test validation fails when I010 (SAC/SIC) is missing."""
        original = {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0}

        decoded = {
            'I040': {
                'RHO': 10000.0 / 1852.0,
                'THETA': 45.0
            },
            'I140': {'value': 3600.0}
        }

        result = validate_cat048_record(original, decoded, self.stats)

        self.assertFalse(result)
        self.assertEqual(self.stats.missing_items.get('I010', 0), 1)

    def test_validate_missing_i140(self):
        """Test validation with missing I140 (time) - not critical."""
        original = {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0}

        decoded = {
            'I010': {'SAC': 1, 'SIC': 2},
            'I040': {
                'RHO': 10000.0 / 1852.0,
                'THETA': 45.0
            }
        }

        result = validate_cat048_record(original, decoded, self.stats)

        # Missing I140 is tracked but doesn't fail validation if I010 and I040 are present
        self.assertEqual(self.stats.missing_items.get('I140', 0), 1)

    def test_validate_i140_as_plain_value(self):
        """Test I140 validation when it's a plain value instead of dict."""
        original = {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0}

        decoded = {
            'I010': {'SAC': 1, 'SIC': 2},
            'I040': {
                'RHO': 10000.0 / 1852.0,
                'THETA': 45.0
            },
            'I140': 3600.0  # Plain value instead of dict
        }

        result = validate_cat048_record(original, decoded, self.stats)

        self.assertTrue(result)
        self.assertEqual(len(self.stats.time_errors), 1)

    def test_validate_custom_tolerances(self):
        """Test validation with custom tolerance values."""
        original = {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0}

        decoded = {
            'I010': {'SAC': 1, 'SIC': 2},
            'I040': {
                'RHO': 10100.0 / 1852.0,  # 100m error
                'THETA': 45.05  # 0.05 degree error
            },
            'I140': {'value': 3600.001}  # 1ms error
        }

        # Tight tolerances
        tolerances = {'range_m': 50.0, 'azimuth_deg': 0.01, 'time_s': 0.0001}
        result = validate_cat048_record(original, decoded, self.stats, tolerances)

        self.assertFalse(result)

    def test_validate_time_modulo_24h(self):
        """Test time validation with modulo 24h (86400s)."""
        original = {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 90000.0}  # > 24h

        decoded = {
            'I010': {'SAC': 1, 'SIC': 2},
            'I040': {
                'RHO': 10000.0 / 1852.0,
                'THETA': 45.0
            },
            'I140': {'value': 3600.0}  # 90000 % 86400 = 3600
        }

        result = validate_cat048_record(original, decoded, self.stats)

        self.assertTrue(result)


class TestValidateRoundTripCat048(unittest.TestCase):
    """Test validate_round_trip_cat048 function."""

    def test_successful_round_trip(self):
        """Test successful round-trip validation."""
        original = [
            {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0},
            {'range': 15000.0, 'azimuth': 90.0, 'timestamp': 3601.0},
        ]

        decoded = [
            {
                'I010': {'SAC': 1, 'SIC': 2},
                'I040': {'RHO': 10000.0 / 1852.0, 'THETA': 45.0},
                'I140': {'value': 3600.0}
            },
            {
                'I010': {'SAC': 1, 'SIC': 2},
                'I040': {'RHO': 15000.0 / 1852.0, 'THETA': 90.0},
                'I140': {'value': 3601.0}
            }
        ]

        success, stats = validate_round_trip_cat048(original, decoded)

        self.assertTrue(success)
        self.assertEqual(stats.total_records, 2)
        self.assertEqual(stats.successful, 2)
        self.assertEqual(stats.failed, 0)

    def test_count_mismatch(self):
        """Test validation with mismatched record counts."""
        original = [
            {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0},
        ]

        decoded = [
            {
                'I010': {'SAC': 1, 'SIC': 2},
                'I040': {'RHO': 10000.0 / 1852.0, 'THETA': 45.0},
                'I140': {'value': 3600.0}
            },
            {
                'I010': {'SAC': 1, 'SIC': 2},
                'I040': {'RHO': 15000.0 / 1852.0, 'THETA': 90.0},
                'I140': {'value': 3601.0}
            }
        ]

        success, stats = validate_round_trip_cat048(original, decoded)

        self.assertFalse(success)
        self.assertIn('Count mismatch', stats.errors[0])
        self.assertEqual(stats.failed, 2)  # max(1, 2)

    def test_partial_failures(self):
        """Test validation with some records failing."""
        original = [
            {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0},
            {'range': 15000.0, 'azimuth': 90.0, 'timestamp': 3601.0},
        ]

        decoded = [
            {
                'I010': {'SAC': 1, 'SIC': 2},
                'I040': {'RHO': 10000.0 / 1852.0, 'THETA': 45.0},
                'I140': {'value': 3600.0}
            },
            {
                'I010': {'SAC': 1, 'SIC': 2},
                # Missing I040 - will fail
                'I140': {'value': 3601.0}
            }
        ]

        success, stats = validate_round_trip_cat048(original, decoded)

        self.assertFalse(success)
        self.assertEqual(stats.successful, 1)
        self.assertEqual(stats.failed, 1)

    def test_exception_handling(self):
        """Test exception handling during validation."""
        original = [
            {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0},
        ]

        # Malformed decoded record (will cause exception)
        decoded = [None]

        success, stats = validate_round_trip_cat048(original, decoded)

        self.assertFalse(success)
        self.assertEqual(stats.failed, 1)
        self.assertGreater(len(stats.errors), 0)

    def test_custom_tolerances(self):
        """Test round-trip validation with custom tolerances."""
        original = [
            {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0},
        ]

        decoded = [
            {
                'I010': {'SAC': 1, 'SIC': 2},
                'I040': {'RHO': 10100.0 / 1852.0, 'THETA': 45.1},  # Small errors
                'I140': {'value': 3600.0}
            }
        ]

        # Very tight tolerances
        tolerances = {'range_m': 50.0, 'azimuth_deg': 0.05, 'time_s': 0.001}
        success, stats = validate_round_trip_cat048(original, decoded, tolerances)

        self.assertFalse(success)

    def test_empty_lists(self):
        """Test validation with empty lists."""
        success, stats = validate_round_trip_cat048([], [])

        # Empty lists should fail (no records to validate)
        self.assertFalse(success)
        self.assertEqual(stats.total_records, 0)


class TestValidateRoundTrip(unittest.TestCase):
    """Test validate_round_trip dispatcher function."""

    def test_auto_detect_cat048(self):
        """Test auto-detection of CAT048."""
        original = [
            {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0},
        ]

        decoded = [
            {
                'category': 48,
                'I010': {'SAC': 1, 'SIC': 2},
                'I040': {'RHO': 10000.0 / 1852.0, 'THETA': 45.0},
                'I140': {'value': 3600.0}
            }
        ]

        success, stats = validate_round_trip(original, decoded)

        self.assertTrue(success)

    def test_explicit_cat048(self):
        """Test explicit CAT048 specification."""
        original = [
            {'range': 10000.0, 'azimuth': 45.0, 'timestamp': 3600.0},
        ]

        decoded = [
            {
                'I010': {'SAC': 1, 'SIC': 2},
                'I040': {'RHO': 10000.0 / 1852.0, 'THETA': 45.0},
                'I140': {'value': 3600.0}
            }
        ]

        success, stats = validate_round_trip(original, decoded, category=48)

        self.assertTrue(success)

    def test_unsupported_category(self):
        """Test unsupported category raises ValueError."""
        original = [{}]
        decoded = [{'category': 99}]

        with self.assertRaises(ValueError) as context:
            validate_round_trip(original, decoded)

        self.assertIn("Unsupported category", str(context.exception))

    def test_cat062_not_implemented(self):
        """Test CAT062 raises NotImplementedError."""
        original = [{}]
        decoded = [{'category': 62}]

        with self.assertRaises(NotImplementedError) as context:
            validate_round_trip(original, decoded)

        self.assertIn("CAT062", str(context.exception))

    def test_cat021_not_implemented(self):
        """Test CAT021 raises NotImplementedError."""
        original = [{}]
        decoded = [{'category': 21}]

        with self.assertRaises(NotImplementedError) as context:
            validate_round_trip(original, decoded)

        self.assertIn("CAT021", str(context.exception))

    def test_empty_decoded_records(self):
        """Test with empty decoded records list."""
        original = [{}]
        decoded = []

        # Should default to category 48 but fail due to count mismatch
        success, stats = validate_round_trip(original, decoded, category=48)

        self.assertFalse(success)


class TestPrintValidationReport(unittest.TestCase):
    """Test print_validation_report function."""

    def test_successful_report(self):
        """Test report printing for successful validation."""
        stats = ValidationStats(
            total_records=10,
            successful=10,
            failed=0,
            range_errors=[1.0, 2.0, 3.0],
            azimuth_errors=[0.1, 0.2, 0.3],
            time_errors=[0.001, 0.002, 0.003]
        )

        # Capture output
        captured_output = StringIO()
        with patch('sys.stdout', captured_output):
            print_validation_report(stats, verbose=True)

        output = captured_output.getvalue()

        self.assertIn("VALIDATION PASSED", output)
        self.assertIn("Total records:    10", output)
        self.assertIn("Successful:       10", output)
        self.assertIn("Success rate:     100.0%", output)
        self.assertIn("Range Errors:", output)
        self.assertIn("Azimuth Errors:", output)
        self.assertIn("Time Errors:", output)

    def test_failed_report(self):
        """Test report printing for failed validation."""
        stats = ValidationStats(
            total_records=10,
            successful=7,
            failed=3,
            errors=['Error 1', 'Error 2', 'Error 3']
        )

        captured_output = StringIO()
        with patch('sys.stdout', captured_output):
            print_validation_report(stats, verbose=True)

        output = captured_output.getvalue()

        self.assertIn("VALIDATION FAILED", output)
        self.assertIn("Failed:           3", output)
        self.assertIn("Success rate:     70.0%", output)
        self.assertIn("Detailed Errors:", output)
        self.assertIn("Error 1", output)

    def test_missing_items_report(self):
        """Test report with missing items."""
        stats = ValidationStats(
            total_records=5,
            successful=3,
            failed=2,
            missing_items={'I040': 2, 'I010': 1}
        )

        captured_output = StringIO()
        with patch('sys.stdout', captured_output):
            print_validation_report(stats, verbose=False)

        output = captured_output.getvalue()

        self.assertIn("Missing Data Items:", output)
        self.assertIn("I040: 2 records", output)
        self.assertIn("I010: 1 records", output)

    def test_no_missing_items_report(self):
        """Test report with no missing items."""
        stats = ValidationStats(
            total_records=5,
            successful=5,
            failed=0
        )

        captured_output = StringIO()
        with patch('sys.stdout', captured_output):
            print_validation_report(stats, verbose=False)

        output = captured_output.getvalue()

        self.assertIn("All required data items present", output)

    def test_verbose_false_no_detailed_errors(self):
        """Test report with verbose=False hides detailed errors."""
        stats = ValidationStats(
            total_records=5,
            successful=3,
            failed=2,
            errors=['Error 1', 'Error 2']
        )

        captured_output = StringIO()
        with patch('sys.stdout', captured_output):
            print_validation_report(stats, verbose=False)

        output = captured_output.getvalue()

        self.assertNotIn("Detailed Errors:", output)

    def test_many_errors_truncation(self):
        """Test that many errors are truncated to first 20."""
        errors = [f"Error {i}" for i in range(30)]
        stats = ValidationStats(
            total_records=30,
            successful=0,
            failed=30,
            errors=errors
        )

        captured_output = StringIO()
        with patch('sys.stdout', captured_output):
            print_validation_report(stats, verbose=True)

        output = captured_output.getvalue()

        self.assertIn("... and 10 more errors", output)

    def test_empty_stats(self):
        """Test report with empty stats."""
        stats = ValidationStats()

        captured_output = StringIO()
        with patch('sys.stdout', captured_output):
            print_validation_report(stats, verbose=True)

        output = captured_output.getvalue()

        self.assertIn("Total records:    0", output)
        self.assertIn("VALIDATION FAILED", output)


class TestCompareWithCppDecoder(unittest.TestCase):
    """Test compare_with_cpp_decoder function."""

    def test_successful_comparison(self):
        """Test successful comparison with C++ decoder."""
        asterix_data = b'\x30\x00\x30'
        python_records = [
            {'category': 48, 'I010': {'SAC': 1, 'SIC': 2}}
        ]

        # Mock the asterix module
        mock_asterix = MagicMock()
        mock_asterix.parse.return_value = [
            {'category': 48, 'I010': {'SAC': 1, 'SIC': 2}}
        ]

        with patch.dict('sys.modules', {'asterix': mock_asterix}):
            matches, stats = compare_with_cpp_decoder(asterix_data, python_records)

        self.assertTrue(matches)
        self.assertTrue(stats['matches'])
        self.assertEqual(stats['total_records'], 1)
        self.assertEqual(len(stats['mismatches']), 0)

    def test_record_count_mismatch(self):
        """Test comparison with different record counts."""
        asterix_data = b'\x30\x00\x30'
        python_records = [
            {'category': 48, 'I010': {'SAC': 1, 'SIC': 2}}
        ]

        # Mock asterix module with 2 records
        mock_asterix = MagicMock()
        mock_asterix.parse.return_value = [
            {'category': 48, 'I010': {'SAC': 1, 'SIC': 2}},
            {'category': 48, 'I010': {'SAC': 1, 'SIC': 3}}
        ]

        with patch.dict('sys.modules', {'asterix': mock_asterix}):
            matches, stats = compare_with_cpp_decoder(asterix_data, python_records)

        self.assertFalse(matches)
        self.assertEqual(stats['error'], 'Record count mismatch')
        self.assertEqual(stats['python_count'], 1)
        self.assertEqual(stats['cpp_count'], 2)

    def test_category_mismatch(self):
        """Test comparison with category mismatch."""
        asterix_data = b'\x30\x00\x30'
        python_records = [
            {'category': 48, 'I010': {'SAC': 1, 'SIC': 2}}
        ]

        mock_asterix = MagicMock()
        mock_asterix.parse.return_value = [
            {'category': 62, 'I010': {'SAC': 1, 'SIC': 2}}
        ]

        with patch.dict('sys.modules', {'asterix': mock_asterix}):
            matches, stats = compare_with_cpp_decoder(asterix_data, python_records)

        self.assertFalse(matches)
        self.assertIn('Category mismatch', stats['mismatches'][0])

    def test_missing_items_in_python(self):
        """Test comparison with items missing in Python output."""
        asterix_data = b'\x30\x00\x30'
        python_records = [
            {'category': 48, 'I010': {'SAC': 1, 'SIC': 2}}
        ]

        mock_asterix = MagicMock()
        mock_asterix.parse.return_value = [
            {'category': 48, 'I010': {'SAC': 1, 'SIC': 2}, 'I040': {'RHO': 10.0}}
        ]

        with patch.dict('sys.modules', {'asterix': mock_asterix}):
            matches, stats = compare_with_cpp_decoder(asterix_data, python_records)

        self.assertFalse(matches)
        self.assertIn('Python missing items', stats['mismatches'][0])

    def test_missing_items_in_cpp(self):
        """Test comparison with items missing in C++ output."""
        asterix_data = b'\x30\x00\x30'
        python_records = [
            {'category': 48, 'I010': {'SAC': 1, 'SIC': 2}, 'I040': {'RHO': 10.0}}
        ]

        mock_asterix = MagicMock()
        mock_asterix.parse.return_value = [
            {'category': 48, 'I010': {'SAC': 1, 'SIC': 2}}
        ]

        with patch.dict('sys.modules', {'asterix': mock_asterix}):
            matches, stats = compare_with_cpp_decoder(asterix_data, python_records)

        self.assertFalse(matches)
        self.assertIn('C++ missing items', stats['mismatches'][0])

    def test_asterix_module_not_available(self):
        """Test error when C++ decoder is not available."""
        asterix_data = b'\x30\x00\x30'
        python_records = []

        # Temporarily hide asterix module
        with patch.dict('sys.modules', {'asterix': None}):
            with self.assertRaises(ImportError) as context:
                compare_with_cpp_decoder(asterix_data, python_records)

            self.assertIn("C++ ASTERIX decoder not available", str(context.exception))


if __name__ == '__main__':
    unittest.main()
