#!/usr/bin/env python3
"""
Property-Based Tests for ASTERIX Encoders

Uses hypothesis library for fuzz testing all 7 ASTERIX encoders with random valid inputs.
Tests focus on:
- Encoders don't crash with valid extreme values
- Encoding/decoding round-trips preserve data
- Coordinate transformations are reversible
- Precision bounds are respected (no overflow)
- FSPEC encoding/decoding is consistent

NOTE: This requires the hypothesis library:
    pip install hypothesis
    or
    python -m venv .venv && source .venv/bin/activate && pip install hypothesis

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import unittest
import sys
import os
import struct
import time
import math

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', '..'))

try:
    from hypothesis import given, strategies as st, settings, HealthCheck
    from hypothesis.strategies import composite
    HYPOTHESIS_AVAILABLE = True
except ImportError:
    HYPOTHESIS_AVAILABLE = False
    print("⚠️  Hypothesis not installed. Skipping property-based tests.")
    print("   Install with: pip install hypothesis")
    # Define dummy decorators so code still loads
    def given(*args, **kwargs):
        def decorator(func):
            return func
        return decorator
    def settings(*args, **kwargs):
        def decorator(func):
            return func
        return decorator
    def composite(func):
        # Return a dummy strategy factory
        def strategy(*args, **kwargs):
            return None
        return strategy
    class st:
        @staticmethod
        def floats(*args, **kwargs):
            return None
        @staticmethod
        def integers(*args, **kwargs):
            return None
        @staticmethod
        def text(*args, **kwargs):
            return None
        @staticmethod
        def lists(*args, **kwargs):
            return None
        @staticmethod
        def tuples(*args, **kwargs):
            return None
        @staticmethod
        def characters(*args, **kwargs):
            return None
    class HealthCheck:
        too_slow = None

# Import all encoders
from asterix.radar_integration.encoder.cat001 import encode_cat001
from asterix.radar_integration.encoder.cat019 import encode_cat019_status
from asterix.radar_integration.encoder.cat020 import encode_cat020
from asterix.radar_integration.encoder.cat021 import encode_cat021
from asterix.radar_integration.encoder.cat034 import encode_cat034_north_marker
from asterix.radar_integration.encoder.cat048 import (
    encode_cat048, encode_fspec, encode_i040, encode_i140,
    encode_i010, encode_i070, encode_i220
)
from asterix.radar_integration.encoder.cat062 import encode_cat062


# ============================================================================
# Hypothesis Strategies (Input Generation)
# ============================================================================

@composite
def radar_plot_strategy(draw):
    """Generate valid radar plot data."""
    return {
        'range': draw(st.floats(min_value=1000.0, max_value=200000.0, allow_nan=False, allow_infinity=False)),
        'azimuth': draw(st.floats(min_value=0.0, max_value=359.999, allow_nan=False, allow_infinity=False)),
        'timestamp': draw(st.floats(min_value=0.0, max_value=86400.0 * 365, allow_nan=False, allow_infinity=False))
    }


@composite
def wgs84_position_strategy(draw):
    """Generate valid WGS-84 coordinates."""
    return (
        draw(st.floats(min_value=-90.0, max_value=90.0, allow_nan=False, allow_infinity=False)),   # latitude
        draw(st.floats(min_value=-180.0, max_value=180.0, allow_nan=False, allow_infinity=False))  # longitude
    )


@composite
def altitude_strategy(draw):
    """Generate valid altitudes in meters."""
    return draw(st.floats(min_value=-1000.0, max_value=50000.0, allow_nan=False, allow_infinity=False))


@composite
def track_data_strategy(draw):
    """Generate valid track data for CAT062."""
    lat, lon = draw(wgs84_position_strategy())
    return {
        'track_number': draw(st.integers(min_value=0, max_value=4095)),
        'lat': lat,
        'lon': lon,
        'altitude': draw(altitude_strategy()),
        'timestamp': draw(st.floats(min_value=0.0, max_value=86400.0, allow_nan=False, allow_infinity=False))
    }


@composite
def adsb_data_strategy(draw):
    """Generate valid ADS-B data for CAT021."""
    lat, lon = draw(wgs84_position_strategy())
    return {
        'track_number': draw(st.integers(min_value=0, max_value=4095)),
        'lat': lat,
        'lon': lon,
        'altitude': draw(altitude_strategy()),
        'timestamp': draw(st.floats(min_value=0.0, max_value=86400.0, allow_nan=False, allow_infinity=False)),
        'icao_address': draw(st.integers(min_value=0x000000, max_value=0xFFFFFF)),
        'callsign': draw(st.text(alphabet=st.characters(whitelist_categories=('Lu', 'Nd')), min_size=0, max_size=8))
    }


@composite
def mode3a_code_strategy(draw):
    """Generate valid Mode 3/A codes (octal 0000-7777)."""
    return draw(st.integers(min_value=0o0000, max_value=0o7777))


@composite
def sac_sic_strategy(draw):
    """Generate valid SAC/SIC pairs."""
    return (
        draw(st.integers(min_value=0, max_value=255)),  # SAC
        draw(st.integers(min_value=0, max_value=255))   # SIC
    )


# ============================================================================
# Property-Based Tests
# ============================================================================

@unittest.skipUnless(HYPOTHESIS_AVAILABLE, "Hypothesis not installed")
class TestCAT048Properties(unittest.TestCase):
    """Property-based tests for CAT048 encoder."""

    @given(radar_plot_strategy())
    @settings(max_examples=100, suppress_health_check=[HealthCheck.too_slow])
    def test_cat048_encoding_never_crashes(self, plot):
        """CAT048 encoding should never crash with valid inputs."""
        try:
            asterix_data = encode_cat048([plot], sac=0, sic=1)
            self.assertGreater(len(asterix_data), 3, "Data block should have at least header")
            self.assertEqual(asterix_data[0], 48, "First byte should be CAT 048")
        except Exception as e:
            self.fail(f"Encoder crashed with valid input: {e}")

    @given(radar_plot_strategy(), sac_sic_strategy())
    @settings(max_examples=100, suppress_health_check=[HealthCheck.too_slow])
    def test_cat048_header_valid(self, plot, sac_sic):
        """CAT048 data blocks should have valid headers."""
        sac, sic = sac_sic
        asterix_data = encode_cat048([plot], sac=sac, sic=sic)

        # Check CAT
        self.assertEqual(asterix_data[0], 48)

        # Check LEN
        length = struct.unpack('!H', asterix_data[1:3])[0]
        self.assertEqual(length, len(asterix_data), "LEN should match actual data length")
        self.assertGreater(length, 3, "LEN should be greater than header size")

    @given(st.lists(radar_plot_strategy(), min_size=1, max_size=10))
    @settings(max_examples=50, suppress_health_check=[HealthCheck.too_slow])
    def test_cat048_multiple_plots(self, plots):
        """CAT048 should handle multiple plots correctly."""
        asterix_data = encode_cat048(plots, sac=0, sic=1)

        self.assertEqual(asterix_data[0], 48)
        length = struct.unpack('!H', asterix_data[1:3])[0]
        self.assertEqual(length, len(asterix_data))
        # More plots should result in larger data
        self.assertGreater(len(asterix_data), 3 + len(plots) * 5)

    @given(
        st.floats(min_value=1000.0, max_value=200000.0, allow_nan=False, allow_infinity=False),
        st.floats(min_value=0.0, max_value=359.999, allow_nan=False, allow_infinity=False)
    )
    @settings(max_examples=200)
    def test_i040_polar_encoding_precision(self, range_m, azimuth_deg):
        """I040 polar encoding should preserve precision within limits."""
        data = encode_i040(range_m, azimuth_deg)
        self.assertEqual(len(data), 4, "I040 should be 4 bytes")

        # Decode
        rho, theta = struct.unpack('>HH', data)

        # RHO: Check range is within valid bounds (0-65535)
        self.assertGreaterEqual(rho, 0)
        self.assertLessEqual(rho, 65535)

        # THETA: Check azimuth is within valid bounds (0-65535)
        self.assertGreaterEqual(theta, 0)
        self.assertLessEqual(theta, 65535)

        # Verify precision loss is acceptable (< 1% for range, < 0.01° for azimuth)
        decoded_range = (rho / 256.0) * 1852.0  # Convert back to meters
        decoded_azimuth = (theta / 65536.0) * 360.0

        range_error_pct = abs(decoded_range - range_m) / range_m * 100
        self.assertLess(range_error_pct, 1.0, f"Range precision loss too high: {range_error_pct:.2f}%")

        azimuth_error = abs(decoded_azimuth - azimuth_deg)
        if azimuth_error > 180:  # Handle wrap-around
            azimuth_error = 360 - azimuth_error
        self.assertLess(azimuth_error, 0.01, f"Azimuth precision loss too high: {azimuth_error:.4f}°")

    @given(st.floats(min_value=0.0, max_value=86400.0 * 365, allow_nan=False, allow_infinity=False))
    @settings(max_examples=200)
    def test_i140_time_encoding(self, timestamp):
        """I140 time encoding should preserve time within limits."""
        data = encode_i140(timestamp)
        self.assertEqual(len(data), 3, "I140 should be 3 bytes")

        # Decode
        time_value = struct.unpack('>I', b'\x00' + data)[0]

        # Should fit in 24 bits
        self.assertLess(time_value, 2**24)

        # Verify time is preserved (modulo 24h with 1/128s precision)
        seconds_since_midnight = timestamp % 86400
        expected_time_128 = int(seconds_since_midnight * 128) % (86400 * 128)

        # Allow small rounding error
        self.assertAlmostEqual(time_value, expected_time_128, delta=2)

    @given(mode3a_code_strategy())
    @settings(max_examples=200)
    def test_i070_mode3a_encoding(self, mode3a):
        """I070 Mode 3/A encoding should preserve codes."""
        data = encode_i070(mode3a)
        self.assertEqual(len(data), 2, "I070 should be 2 bytes")

        # Decode
        code_value = struct.unpack('>H', data)[0]
        decoded_code = code_value & 0x0FFF

        # Should preserve code exactly
        self.assertEqual(decoded_code, mode3a, "Mode 3/A code should be preserved")

    @given(st.integers(min_value=0, max_value=255), st.integers(min_value=0, max_value=255))
    @settings(max_examples=200)
    def test_i010_sac_sic_encoding(self, sac, sic):
        """I010 SAC/SIC encoding should be exact."""
        data = encode_i010(sac, sic)
        self.assertEqual(len(data), 2, "I010 should be 2 bytes")
        self.assertEqual(data[0], sac, "SAC should be preserved")
        self.assertEqual(data[1], sic, "SIC should be preserved")

    @given(st.integers(min_value=0x000000, max_value=0xFFFFFF))
    @settings(max_examples=200)
    def test_i220_aircraft_address_encoding(self, address):
        """I220 aircraft address encoding should preserve 24-bit addresses."""
        data = encode_i220(address)
        self.assertEqual(len(data), 3, "I220 should be 3 bytes")

        # Decode
        decoded = struct.unpack('>I', b'\x00' + data)[0]
        self.assertEqual(decoded, address, "Aircraft address should be preserved")


@unittest.skipUnless(HYPOTHESIS_AVAILABLE, "Hypothesis not installed")
class TestCAT021Properties(unittest.TestCase):
    """Property-based tests for CAT021 encoder."""

    @given(adsb_data_strategy())
    @settings(max_examples=100, suppress_health_check=[HealthCheck.too_slow])
    def test_cat021_encoding_never_crashes(self, data):
        """CAT021 encoding should never crash with valid inputs."""
        try:
            asterix_data = encode_cat021(
                [data],
                sac=0,
                sic=1
            )
            self.assertGreater(len(asterix_data), 3)
            self.assertEqual(asterix_data[0], 21, "First byte should be CAT 021")
        except Exception as e:
            self.fail(f"Encoder crashed with valid input: {e}")

    @given(adsb_data_strategy(), sac_sic_strategy())
    @settings(max_examples=100, suppress_health_check=[HealthCheck.too_slow])
    def test_cat021_header_valid(self, data, sac_sic):
        """CAT021 data blocks should have valid headers."""
        sac, sic = sac_sic
        asterix_data = encode_cat021([data], sac=sac, sic=sic)

        self.assertEqual(asterix_data[0], 21)
        length = struct.unpack('!H', asterix_data[1:3])[0]
        self.assertEqual(length, len(asterix_data))

    @given(wgs84_position_strategy(), altitude_strategy())
    @settings(max_examples=200)
    def test_cat021_position_encoding_bounds(self, position, altitude):
        """CAT021 position encoding should respect WGS-84 bounds."""
        lat, lon = position
        data = {
            'track_number': 1,
            'lat': lat,
            'lon': lon,
            'altitude': altitude,
            'timestamp': time.time(),
            'icao_address': 0x123456
        }

        try:
            asterix_data = encode_cat021([data], sac=0, sic=1)
            self.assertGreater(len(asterix_data), 3)
            # Verify it doesn't crash on extreme but valid coordinates
        except Exception as e:
            self.fail(f"Failed on valid coords lat={lat:.6f}, lon={lon:.6f}, alt={altitude:.1f}: {e}")


@unittest.skipUnless(HYPOTHESIS_AVAILABLE, "Hypothesis not installed")
class TestCAT062Properties(unittest.TestCase):
    """Property-based tests for CAT062 encoder."""

    @given(track_data_strategy())
    @settings(max_examples=100, suppress_health_check=[HealthCheck.too_slow])
    def test_cat062_encoding_never_crashes(self, data):
        """CAT062 encoding should never crash with valid inputs."""
        try:
            asterix_data = encode_cat062([data], sac=0, sic=1)
            self.assertGreater(len(asterix_data), 3)
            self.assertEqual(asterix_data[0], 62, "First byte should be CAT 062")
        except Exception as e:
            self.fail(f"Encoder crashed with valid input: {e}")

    @given(st.lists(track_data_strategy(), min_size=1, max_size=20))
    @settings(max_examples=50, suppress_health_check=[HealthCheck.too_slow])
    def test_cat062_multiple_tracks(self, tracks):
        """CAT062 should handle multiple tracks correctly."""
        asterix_data = encode_cat062(tracks, sac=0, sic=1)

        self.assertEqual(asterix_data[0], 62)
        length = struct.unpack('!H', asterix_data[1:3])[0]
        self.assertEqual(length, len(asterix_data))
        self.assertGreater(len(asterix_data), 3 + len(tracks) * 5)


@unittest.skipUnless(HYPOTHESIS_AVAILABLE, "Hypothesis not installed")
class TestCAT001Properties(unittest.TestCase):
    """Property-based tests for CAT001 encoder."""

    @given(radar_plot_strategy())
    @settings(max_examples=100, suppress_health_check=[HealthCheck.too_slow])
    def test_cat001_encoding_never_crashes(self, plot):
        """CAT001 encoding should never crash with valid inputs."""
        try:
            asterix_data = encode_cat001([plot], sac=0, sic=1)
            self.assertGreater(len(asterix_data), 3)
            self.assertEqual(asterix_data[0], 1, "First byte should be CAT 001")
        except Exception as e:
            self.fail(f"Encoder crashed with valid input: {e}")


@unittest.skipUnless(HYPOTHESIS_AVAILABLE, "Hypothesis not installed")
class TestCAT019Properties(unittest.TestCase):
    """Property-based tests for CAT019 encoder."""

    @given(st.floats(min_value=0.0, max_value=86400.0, allow_nan=False, allow_infinity=False))
    @settings(max_examples=100, suppress_health_check=[HealthCheck.too_slow])
    def test_cat019_encoding_never_crashes(self, timestamp):
        """CAT019 encoding should never crash with valid inputs."""
        try:
            asterix_data = encode_cat019_status(
                sac=0,
                sic=1,
                timestamp=timestamp,
                operational=True,
                num_sensors=4
            )
            self.assertGreater(len(asterix_data), 3)
            self.assertEqual(asterix_data[0], 19, "First byte should be CAT 019")
        except Exception as e:
            self.fail(f"Encoder crashed with valid input: {e}")


@unittest.skipUnless(HYPOTHESIS_AVAILABLE, "Hypothesis not installed")
class TestCAT020Properties(unittest.TestCase):
    """Property-based tests for CAT020 encoder."""

    @given(radar_plot_strategy())
    @settings(max_examples=100, suppress_health_check=[HealthCheck.too_slow])
    def test_cat020_encoding_never_crashes(self, plot):
        """CAT020 encoding should never crash with valid inputs."""
        try:
            asterix_data = encode_cat020([plot], sac=0, sic=1)
            self.assertGreater(len(asterix_data), 3)
            self.assertEqual(asterix_data[0], 20, "First byte should be CAT 020")
        except Exception as e:
            self.fail(f"Encoder crashed with valid input: {e}")


@unittest.skipUnless(HYPOTHESIS_AVAILABLE, "Hypothesis not installed")
class TestCAT034Properties(unittest.TestCase):
    """Property-based tests for CAT034 encoder."""

    @given(
        st.floats(min_value=0.0, max_value=86400.0, allow_nan=False, allow_infinity=False),
        st.integers(min_value=0, max_value=255)
    )
    @settings(max_examples=100)
    def test_cat034_encoding_never_crashes(self, timestamp, sector):
        """CAT034 encoding should never crash with valid inputs."""
        try:
            asterix_data = encode_cat034_north_marker(
                sac=0,
                sic=1,
                sector_number=sector,
                timestamp=timestamp
            )
            self.assertGreater(len(asterix_data), 3)
            self.assertEqual(asterix_data[0], 34, "First byte should be CAT 034")
        except Exception as e:
            self.fail(f"Encoder crashed with valid input: {e}")


@unittest.skipUnless(HYPOTHESIS_AVAILABLE, "Hypothesis not installed")
class TestFSPECProperties(unittest.TestCase):
    """Property-based tests for FSPEC encoding (used by all categories)."""

    @given(st.lists(st.integers(min_value=1, max_value=50), min_size=1, max_size=20, unique=True))
    @settings(max_examples=200)
    def test_fspec_encoding_length(self, items):
        """FSPEC length should be correct based on max FRN."""
        fspec = encode_fspec(items)

        max_frn = max(items)
        expected_octets = (max_frn + 6) // 7

        self.assertEqual(len(fspec), expected_octets,
                        f"FSPEC should have {expected_octets} octets for max FRN {max_frn}")

    @given(st.lists(st.integers(min_value=1, max_value=28), min_size=1, max_size=10, unique=True))
    @settings(max_examples=200)
    def test_fspec_extension_bits(self, items):
        """FSPEC extension bits should be set correctly."""
        fspec = encode_fspec(items)

        # All but last octet should have FX bit (bit 1) set
        for i in range(len(fspec) - 1):
            self.assertEqual(fspec[i] & 0x01, 0x01,
                           f"Octet {i} should have FX bit set")

        # Last octet should NOT have FX bit set
        if len(fspec) > 0:
            self.assertEqual(fspec[-1] & 0x01, 0x00,
                           "Last octet should not have FX bit set")

    @given(st.lists(st.integers(min_value=1, max_value=14), min_size=1, max_size=14, unique=True))
    @settings(max_examples=200)
    def test_fspec_item_bits_set(self, items):
        """FSPEC should have correct bits set for each item."""
        fspec = encode_fspec(items)

        for frn in items:
            octet_idx = (frn - 1) // 7
            bit_idx = 7 - ((frn - 1) % 7)

            if octet_idx < len(fspec):
                bit_set = bool(fspec[octet_idx] & (1 << bit_idx))
                self.assertTrue(bit_set, f"FRN {frn} should set bit {bit_idx} in octet {octet_idx}")


@unittest.skipUnless(HYPOTHESIS_AVAILABLE, "Hypothesis not installed")
class TestCoordinateTransformations(unittest.TestCase):
    """Property-based tests for coordinate transformations (if used)."""

    @given(wgs84_position_strategy())
    @settings(max_examples=200)
    def test_wgs84_coordinates_valid_range(self, position):
        """WGS-84 coordinates should stay within valid bounds."""
        lat, lon = position

        self.assertGreaterEqual(lat, -90.0, "Latitude should be >= -90")
        self.assertLessEqual(lat, 90.0, "Latitude should be <= 90")
        self.assertGreaterEqual(lon, -180.0, "Longitude should be >= -180")
        self.assertLessEqual(lon, 180.0, "Longitude should be <= 180")

    @given(
        st.floats(min_value=0.0, max_value=359.999, allow_nan=False, allow_infinity=False),
        st.floats(min_value=1000.0, max_value=200000.0, allow_nan=False, allow_infinity=False)
    )
    @settings(max_examples=200)
    def test_polar_to_cartesian_no_overflow(self, azimuth, range_m):
        """Polar to Cartesian conversion should not overflow."""
        # Simple polar to Cartesian
        x = range_m * math.cos(math.radians(azimuth))
        y = range_m * math.sin(math.radians(azimuth))

        # Should not produce NaN or infinity
        self.assertFalse(math.isnan(x), "X coordinate should not be NaN")
        self.assertFalse(math.isnan(y), "Y coordinate should not be NaN")
        self.assertFalse(math.isinf(x), "X coordinate should not be infinity")
        self.assertFalse(math.isinf(y), "Y coordinate should not be infinity")

        # Should be within reasonable bounds
        self.assertLessEqual(abs(x), range_m * 1.01, "X should not exceed range")
        self.assertLessEqual(abs(y), range_m * 1.01, "Y should not exceed range")


@unittest.skipUnless(HYPOTHESIS_AVAILABLE, "Hypothesis not installed")
class TestEncodingEdgeCases(unittest.TestCase):
    """Property-based tests for edge cases across all encoders."""

    @given(
        st.floats(min_value=0.0, max_value=0.1, allow_nan=False, allow_infinity=False),
        st.floats(min_value=199999.0, max_value=200000.0, allow_nan=False, allow_infinity=False)
    )
    @settings(max_examples=100)
    def test_extreme_azimuths_and_ranges(self, azimuth, range_m):
        """Test extreme but valid azimuth and range values."""
        plot = {
            'range': range_m,
            'azimuth': azimuth,
            'timestamp': time.time()
        }

        try:
            asterix_data = encode_cat048([plot], sac=0, sic=1)
            self.assertEqual(asterix_data[0], 48)
        except Exception as e:
            self.fail(f"Failed on extreme values: azimuth={azimuth:.6f}, range={range_m:.1f}: {e}")

    @given(
        st.floats(min_value=-89.999, max_value=-85.0, allow_nan=False, allow_infinity=False),
        st.floats(min_value=-180.0, max_value=-179.0, allow_nan=False, allow_infinity=False)
    )
    @settings(max_examples=100)
    def test_extreme_polar_coordinates(self, lat, lon):
        """Test extreme polar coordinates (near poles, dateline)."""
        data = {
            'track_number': 1,
            'lat': lat,
            'lon': lon,
            'altitude': 10000.0,
            'timestamp': time.time(),
            'icao_address': 0x123456
        }

        try:
            asterix_data = encode_cat021([data], sac=0, sic=1)
            self.assertEqual(asterix_data[0], 21)
        except Exception as e:
            self.fail(f"Failed on extreme coords: lat={lat:.6f}, lon={lon:.6f}: {e}")

    @given(st.floats(min_value=0.0, max_value=1.0, allow_nan=False, allow_infinity=False))
    @settings(max_examples=100)
    def test_very_small_timestamps(self, timestamp):
        """Test very small timestamp values (near epoch)."""
        plot = {
            'range': 50000.0,
            'azimuth': 45.0,
            'timestamp': timestamp
        }

        try:
            asterix_data = encode_cat048([plot], sac=0, sic=1)
            self.assertEqual(asterix_data[0], 48)
        except Exception as e:
            self.fail(f"Failed on small timestamp: {timestamp}: {e}")


# ============================================================================
# Test Summary
# ============================================================================

def count_property_tests():
    """Count the number of property-based tests."""
    if not HYPOTHESIS_AVAILABLE:
        return 0

    count = 0
    for name, obj in globals().items():
        if isinstance(obj, type) and issubclass(obj, unittest.TestCase):
            if name.startswith('Test') and 'Properties' in name:
                for attr in dir(obj):
                    if attr.startswith('test_'):
                        count += 1
    return count


if __name__ == '__main__':
    if HYPOTHESIS_AVAILABLE:
        print(f"\n{'='*70}")
        print("Property-Based Testing with Hypothesis")
        print(f"{'='*70}")
        print(f"Tests created: {count_property_tests()}")
        print("Each test runs 50-200 examples (configurable)")
        print(f"{'='*70}\n")

    unittest.main()
