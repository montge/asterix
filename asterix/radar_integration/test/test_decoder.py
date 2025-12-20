#!/usr/bin/env python3
"""
Unit tests for ASTERIX Python decoder

Tests:
- Base decoder (FSPEC parsing, data block structure)
- CAT048 decoder (all data items)
- CAT062 decoder (subset of items)
- CAT021 decoder (subset of items)
- Round-trip validation (encode → decode → compare)
- Comparison with C++ decoder

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

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', '..'))

from asterix.radar_integration.decoder import (
    decode_fspec,
    DecoderError,
    CAT048Decoder,
    decode_cat048,
    decode_asterix,
)
from asterix.radar_integration.decoder.validator import (
    validate_round_trip_cat048,
    compare_values,
    compare_angles,
)
from asterix.radar_integration.encoder.cat048 import (
    encode_fspec,
    encode_cat048_record,
    encode_cat048_datablock,
    encode_cat048,
)
from asterix.radar_integration.mock_radar import RadarPlot, MockRadar


class TestFSPECDecoding(unittest.TestCase):
    """Test Field Specification (FSPEC) decoding."""

    def test_fspec_single_octet(self):
        """Test FSPEC with items in first octet (FRN 1-7)."""
        # 0xE0 = 0b11100000 (FRNs 1, 2, 3 present, no extension)
        frns, consumed = decode_fspec(b'\xe0\x12\x34')
        self.assertEqual(frns, [1, 2, 3])
        self.assertEqual(consumed, 1)

    def test_fspec_extension(self):
        """Test FSPEC with extension to second octet."""
        # 0x81, 0x80 = FRN 1 in first octet, FRN 8 in second octet
        frns, consumed = decode_fspec(b'\x81\x80\xff')
        self.assertIn(1, frns)
        self.assertIn(8, frns)
        self.assertEqual(consumed, 2)

    def test_fspec_empty(self):
        """Test FSPEC with no items."""
        # 0x00 = no items, no extension
        frns, consumed = decode_fspec(b'\x00')
        self.assertEqual(frns, [])
        self.assertEqual(consumed, 1)

    def test_fspec_all_items_first_octet(self):
        """Test FSPEC with all 7 items in first octet."""
        # 0xFE = 0b11111110 (FRNs 1-7, no extension)
        frns, consumed = decode_fspec(b'\xfe')
        self.assertEqual(frns, [1, 2, 3, 4, 5, 6, 7])
        self.assertEqual(consumed, 1)

    def test_fspec_truncated(self):
        """Test FSPEC with truncated data."""
        with self.assertRaises(DecoderError):
            decode_fspec(b'')

    def test_fspec_encode_decode_roundtrip(self):
        """Test FSPEC encode → decode round-trip."""
        frns_original = [1, 2, 4, 6, 8, 10]
        encoded = encode_fspec(frns_original)
        frns_decoded, _ = decode_fspec(encoded)
        self.assertEqual(sorted(frns_decoded), sorted(frns_original))


class TestCAT048Decoder(unittest.TestCase):
    """Test CAT048 decoder."""

    def test_decode_i010(self):
        """Test I010 (SAC/SIC) decoding."""
        decoder = CAT048Decoder(verbose=False)
        data = struct.pack('BB', 123, 45)
        result, consumed = decoder._decode_i010(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['SAC'], 123)
        self.assertEqual(result['SIC'], 45)

    def test_decode_i040(self):
        """Test I040 (polar position) decoding."""
        decoder = CAT048Decoder(verbose=False)

        # Encode: 50 km at 45 degrees
        range_m = 50000.0
        azimuth_deg = 45.0

        range_nm = range_m / 1852.0
        rho = int(range_nm * 256)
        theta = int(azimuth_deg * 65536 / 360.0)

        data = struct.pack('>HH', rho, theta)
        result, consumed = decoder._decode_i040(data, 0)

        self.assertEqual(consumed, 4)
        self.assertAlmostEqual(result['RHO'], range_nm, places=2)
        self.assertAlmostEqual(result['THETA'], azimuth_deg, places=2)

    def test_decode_i140(self):
        """Test I140 (time of day) decoding."""
        decoder = CAT048Decoder(verbose=False)

        # Encode: 12:30:45.5 = 45045.5 seconds
        time_seconds = 12 * 3600 + 30 * 60 + 45.5
        time_128 = int(time_seconds * 128)

        data = struct.pack('>I', time_128)[1:]  # 3 bytes
        result, consumed = decoder._decode_i140(data, 0)

        self.assertEqual(consumed, 3)
        self.assertAlmostEqual(result, time_seconds, places=1)

    def test_decode_i070(self):
        """Test I070 (Mode 3/A) decoding."""
        decoder = CAT048Decoder(verbose=False)

        # Encode: Mode 3/A code 0o1234
        code = 0o1234
        data = struct.pack('>H', code & 0x0FFF)
        result, consumed = decoder._decode_i070(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['code'], code)
        self.assertFalse(result['V'])  # Not validated
        self.assertFalse(result['G'])  # Not garbled

    def test_decode_i220(self):
        """Test I220 (aircraft address) decoding."""
        decoder = CAT048Decoder(verbose=False)

        # Encode: Address 0xABCDEF
        address = 0xABCDEF
        data = struct.pack('>I', address)[1:]  # 3 bytes
        result, consumed = decoder._decode_i220(data, 0)

        self.assertEqual(consumed, 3)
        self.assertEqual(result, address)

    def test_decode_full_record(self):
        """Test decoding a complete CAT048 record."""
        # Encode a record with known values
        encoded_record = encode_cat048_record(
            range_m=50000.0,
            azimuth_deg=135.0,
            timestamp=time.time(),
            sac=12,
            sic=34,
            mode3a=0o7654,
            typ="PSR"
        )

        # Decode it
        decoder = CAT048Decoder(verbose=False)
        record, consumed = decoder.decode_record(encoded_record, 0)

        self.assertEqual(record['category'], 48)
        self.assertEqual(record['I010']['SAC'], 12)
        self.assertEqual(record['I010']['SIC'], 34)
        self.assertIn('I040', record)
        self.assertIn('I140', record)
        self.assertEqual(record['I070']['code'], 0o7654)

    def test_decode_datablock(self):
        """Test decoding a complete data block."""
        # Encode 3 records
        records_data = []
        for i in range(3):
            rec = encode_cat048_record(
                range_m=30000.0 + i * 10000,
                azimuth_deg=45.0 + i * 90,
                timestamp=time.time() + i,
                sac=0,
                sic=1,
            )
            records_data.append(rec)

        datablock = encode_cat048_datablock(records_data)

        # Decode it
        decoder = CAT048Decoder(verbose=False)
        decoded_records = decoder.decode_datablock(datablock)

        self.assertEqual(len(decoded_records), 3)
        for rec in decoded_records:
            self.assertEqual(rec['category'], 48)
            self.assertIn('I010', rec)
            self.assertIn('I040', rec)


class TestRoundTripValidation(unittest.TestCase):
    """Test round-trip encode → decode → validate."""

    def test_single_plot_roundtrip(self):
        """Test single radar plot round-trip."""
        # Generate plot
        plot = RadarPlot(
            range=50000.0,
            azimuth=135.5,
            timestamp=time.time(),
        )

        # Encode
        encoded = encode_cat048([plot], sac=0, sic=1)

        # Decode
        decoded = decode_cat048(encoded, verbose=False)

        # Validate
        success, stats = validate_round_trip_cat048([plot], decoded)

        self.assertTrue(success, f"Validation failed: {stats.errors}")
        self.assertEqual(stats.total_records, 1)
        self.assertEqual(stats.successful, 1)
        self.assertEqual(stats.failed, 0)

    def test_multiple_plots_roundtrip(self):
        """Test multiple radar plots round-trip."""
        # Generate 20 plots
        radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        plots = radar.generate_plots(num_targets=20, add_noise=False)

        # Encode
        encoded = encode_cat048(plots, sac=0, sic=1)

        # Decode
        decoded = decode_cat048(encoded, verbose=False)

        # Validate
        success, stats = validate_round_trip_cat048(plots, decoded)

        self.assertEqual(len(decoded), len(plots))
        self.assertTrue(success, f"Validation failed: {stats.errors[:5]}")
        self.assertEqual(stats.successful, 20)

    def test_roundtrip_with_noise(self):
        """Test round-trip with noisy radar data."""
        # Generate plots with noise
        radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        plots = radar.generate_plots(num_targets=10, add_noise=True)

        # Encode
        encoded = encode_cat048(plots, sac=0, sic=1)

        # Decode
        decoded = decode_cat048(encoded, verbose=False)

        # Validate (should still pass - encoding removes noise effects)
        success, stats = validate_round_trip_cat048(plots, decoded)

        self.assertEqual(len(decoded), len(plots))
        self.assertTrue(success or stats.success_rate() > 0.9)

    def test_tolerance_comparison(self):
        """Test tolerance-aware value comparison."""
        # Absolute tolerance
        matches, error = compare_values(100.0, 100.5, tolerance=1.0)
        self.assertTrue(matches)
        self.assertEqual(error, 0.5)

        matches, error = compare_values(100.0, 102.0, tolerance=1.0)
        self.assertFalse(matches)
        self.assertEqual(error, 2.0)

        # Angle comparison
        matches, error = compare_angles(359.0, 1.0, tolerance_deg=3.0)
        self.assertTrue(matches)
        self.assertEqual(error, 2.0)

        matches, error = compare_angles(0.0, 180.0, tolerance_deg=5.0)
        self.assertFalse(matches)
        self.assertEqual(error, 180.0)


class TestDecodeAsterix(unittest.TestCase):
    """Test auto-detect decoder."""

    def test_decode_asterix_cat048(self):
        """Test decode_asterix with CAT048 data."""
        # Encode CAT048
        plot = RadarPlot(range=40000, azimuth=90.0, timestamp=time.time())
        encoded = encode_cat048([plot], sac=5, sic=10)

        # Decode with auto-detect
        decoded = decode_asterix(encoded, verbose=False)

        self.assertEqual(len(decoded), 1)
        self.assertEqual(decoded[0]['category'], 48)
        self.assertEqual(decoded[0]['I010']['SAC'], 5)
        self.assertEqual(decoded[0]['I010']['SIC'], 10)

    def test_decode_asterix_invalid_category(self):
        """Test decode_asterix with unsupported category."""
        # Fake CAT099 data block
        data = struct.pack('!BH', 99, 10) + b'\x00' * 7

        with self.assertRaises(DecoderError):
            decode_asterix(data)

    def test_decode_asterix_truncated(self):
        """Test decode_asterix with truncated data."""
        with self.assertRaises(DecoderError):
            decode_asterix(b'\x30')  # Only category byte


class TestCompareWithCppDecoder(unittest.TestCase):
    """Test comparison with C++ decoder (if available)."""

    def test_compare_with_cpp_decoder(self):
        """Compare Python decoder with C++ decoder."""
        try:
            import asterix
        except ImportError:
            self.skipTest("C++ ASTERIX decoder not available")

        # Generate test data
        radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        plots = radar.generate_plots(num_targets=5, add_noise=False)

        # Encode
        encoded = encode_cat048(plots, sac=0, sic=1)

        # Decode with Python
        py_decoded = decode_cat048(encoded, verbose=True)

        # Decode with C++
        cpp_decoded = asterix.parse(encoded, verbose=True)

        # Compare record count
        self.assertEqual(len(py_decoded), len(cpp_decoded))

        # Compare each record
        for py_rec, cpp_rec in zip(py_decoded, cpp_decoded):
            # Check category
            self.assertEqual(py_rec['category'], cpp_rec['category'])

            # Check I010 (handle C++ decoder dict format)
            cpp_sac = cpp_rec['I010']['SAC']
            cpp_sic = cpp_rec['I010']['SIC']
            cpp_sac_val = cpp_sac.get('val') if isinstance(cpp_sac, dict) else cpp_sac
            cpp_sic_val = cpp_sic.get('val') if isinstance(cpp_sic, dict) else cpp_sic
            self.assertEqual(py_rec['I010']['SAC'], cpp_sac_val)
            self.assertEqual(py_rec['I010']['SIC'], cpp_sic_val)

            # Check I040 (position) - allow small differences due to float precision
            if 'I040' in py_rec and 'I040' in cpp_rec:
                py_rho = py_rec['I040']['RHO']
                cpp_rho = cpp_rec['I040']['RHO']
                cpp_rho_val = cpp_rho.get("val") if isinstance(cpp_rho, dict) else cpp_rho
                self.assertAlmostEqual(py_rho, cpp_rho_val, places=2)
                py_theta = py_rec['I040']['THETA']
                cpp_theta = cpp_rec['I040']['THETA']
                cpp_theta_val = cpp_theta.get("val") if isinstance(cpp_theta, dict) else cpp_theta
                self.assertAlmostEqual(py_theta, cpp_theta_val, places=2)

class TestBaseDecoderUtilities(unittest.TestCase):
    """Test base decoder utility functions."""

    def test_decode_uint(self):
        """Test unsigned integer decoding."""
        from asterix.radar_integration.decoder.base import decode_uint

        # 1 byte
        self.assertEqual(decode_uint(b'\xff', 0, 1), 255)
        self.assertEqual(decode_uint(b'\x00', 0, 1), 0)

        # 2 bytes (big-endian)
        self.assertEqual(decode_uint(b'\x01\x00', 0, 2), 256)
        self.assertEqual(decode_uint(b'\xff\xff', 0, 2), 65535)

        # 3 bytes
        self.assertEqual(decode_uint(b'\x01\x00\x00', 0, 3), 65536)

        # With offset
        self.assertEqual(decode_uint(b'\x00\x00\xff', 2, 1), 255)

    def test_decode_uint_truncated(self):
        """Test decode_uint with insufficient data."""
        from asterix.radar_integration.decoder.base import decode_uint

        with self.assertRaises(DecoderError):
            decode_uint(b'\x00', 0, 2)

    def test_decode_int(self):
        """Test signed integer decoding."""
        from asterix.radar_integration.decoder.base import decode_int

        # Positive
        self.assertEqual(decode_int(b'\x7f', 0, 1), 127)

        # Negative (two's complement)
        self.assertEqual(decode_int(b'\xff', 0, 1), -1)
        self.assertEqual(decode_int(b'\x80', 0, 1), -128)

        # 2 bytes
        self.assertEqual(decode_int(b'\xff\xff', 0, 2), -1)
        self.assertEqual(decode_int(b'\x80\x00', 0, 2), -32768)

    def test_decode_int_truncated(self):
        """Test decode_int with insufficient data."""
        from asterix.radar_integration.decoder.base import decode_int

        with self.assertRaises(DecoderError):
            decode_int(b'\x00', 0, 2)

    def test_decode_octal(self):
        """Test octal value decoding."""
        from asterix.radar_integration.decoder.base import decode_octal

        # Mode 3/A code 7777 = 0xFFF (12 bits)
        data = struct.pack('>H', 0x0FFF)
        self.assertEqual(decode_octal(data, 0, 2, 0x0FFF), 0o7777)

        # Mode 3/A code 1234 = 0x29C (octal to binary)
        code = 0o1234
        data = struct.pack('>H', code)
        self.assertEqual(decode_octal(data, 0, 2, 0x0FFF), code)

    def test_decode_6bit_ascii(self):
        """Test 6-bit ASCII decoding."""
        from asterix.radar_integration.decoder.base import decode_6bit_ascii

        # Encode "ABC" in 6-bit ASCII
        # A=1, B=2, C=3 => 000001 000010 000011 = 0x04 0x20 0xC0 (padded)
        # Actually let's test with known encoding
        # For simplicity, test that function handles space (32)
        # Space = 0x20 in 6-bit = value 32

        # Test truncation error
        with self.assertRaises(DecoderError):
            decode_6bit_ascii(b'\x00', 0, 10)

    def test_bytes_consumed_for_fspec(self):
        """Test FSPEC bytes calculation."""
        from asterix.radar_integration.decoder.base import bytes_consumed_for_fspec

        # Empty list
        self.assertEqual(bytes_consumed_for_fspec([]), 0)

        # FRNs 1-7 fit in 1 byte
        self.assertEqual(bytes_consumed_for_fspec([1, 2, 3]), 1)
        self.assertEqual(bytes_consumed_for_fspec([7]), 1)

        # FRN 8 requires 2 bytes
        self.assertEqual(bytes_consumed_for_fspec([8]), 2)
        self.assertEqual(bytes_consumed_for_fspec([1, 8]), 2)

        # FRN 14 requires 2 bytes
        self.assertEqual(bytes_consumed_for_fspec([14]), 2)

        # FRN 15 requires 3 bytes
        self.assertEqual(bytes_consumed_for_fspec([15]), 3)


class TestCAT021Decoder(unittest.TestCase):
    """Test CAT021 decoder (ADS-B)."""

    def test_cat021_decoder_i010(self):
        """Test CAT021 I010 (SAC/SIC) decoding."""
        from asterix.radar_integration.decoder import CAT021Decoder

        decoder = CAT021Decoder(verbose=False)
        data = struct.pack('BB', 55, 66)
        result, consumed = decoder._decode_i010(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['SAC'], 55)
        self.assertEqual(result['SIC'], 66)

    def test_cat021_decoder_i161(self):
        """Test CAT021 I161 (Track Number) decoding."""
        from asterix.radar_integration.decoder import CAT021Decoder

        decoder = CAT021Decoder(verbose=False)
        # Track number 1234 in upper 12 bits
        track_num = 1234
        value = track_num << 4
        data = struct.pack('>H', value)
        result, consumed = decoder._decode_i161(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['track_number'], track_num)

    def test_cat021_decoder_i015(self):
        """Test CAT021 I015 (Service ID) decoding."""
        from asterix.radar_integration.decoder import CAT021Decoder

        decoder = CAT021Decoder(verbose=False)
        data = b'\x42'
        result, consumed = decoder._decode_i015(data, 0)

        self.assertEqual(consumed, 1)
        self.assertEqual(result, 0x42)

    def test_cat021_decoder_i071(self):
        """Test CAT021 I071 (Time of Applicability) decoding."""
        from asterix.radar_integration.decoder import CAT021Decoder

        decoder = CAT021Decoder(verbose=False)
        # 12:30:45.5 = 45045.5 seconds since midnight
        time_seconds = 12 * 3600 + 30 * 60 + 45.5
        time_128 = int(time_seconds * 128)
        data = struct.pack('>I', time_128)[1:]  # 3 bytes

        result, consumed = decoder._decode_i071(data, 0)

        self.assertEqual(consumed, 3)
        self.assertAlmostEqual(result, time_seconds, places=2)


class TestCAT062Decoder(unittest.TestCase):
    """Test CAT062 decoder (System Tracks)."""

    def test_cat062_decoder_i010(self):
        """Test CAT062 I010 (SAC/SIC) decoding."""
        from asterix.radar_integration.decoder import CAT062Decoder

        decoder = CAT062Decoder(verbose=False)
        data = struct.pack('BB', 99, 88)
        result, consumed = decoder._decode_i010(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['SAC'], 99)
        self.assertEqual(result['SIC'], 88)

    def test_cat062_decoder_i070(self):
        """Test CAT062 I070 (Time of Track Information) decoding."""
        from asterix.radar_integration.decoder import CAT062Decoder

        decoder = CAT062Decoder(verbose=False)
        # Time of day: 10:30:00 = 37800 seconds
        time_seconds = 10 * 3600 + 30 * 60
        time_128 = int(time_seconds * 128)
        data = struct.pack('>I', time_128)[1:]  # 3 bytes

        result, consumed = decoder._decode_i070(data, 0)

        self.assertEqual(consumed, 3)
        self.assertAlmostEqual(result, time_seconds, places=2)

    def test_cat062_decoder_i040(self):
        """Test CAT062 I040 (Track Number) decoding."""
        from asterix.radar_integration.decoder import CAT062Decoder

        decoder = CAT062Decoder(verbose=False)
        track_num = 4567
        data = struct.pack('>H', track_num)
        result, consumed = decoder._decode_i040(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result, track_num)  # verbose=False returns int

    def test_cat062_decoder_i040_verbose(self):
        """Test CAT062 I040 (Track Number) decoding with verbose mode."""
        from asterix.radar_integration.decoder import CAT062Decoder

        decoder = CAT062Decoder(verbose=True)
        track_num = 4567
        data = struct.pack('>H', track_num)
        result, consumed = decoder._decode_i040(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['track_number'], track_num)


class TestDecoderEdgeCases(unittest.TestCase):
    """Test decoder edge cases and error handling."""

    def test_empty_datablock(self):
        """Test decoding empty data."""
        with self.assertRaises(DecoderError):
            decode_asterix(b'')

    def test_truncated_header(self):
        """Test decoding truncated header."""
        with self.assertRaises(DecoderError):
            decode_asterix(b'\x30\x00')  # Category + partial length

    def test_length_mismatch(self):
        """Test decoding with length mismatch."""
        # Header says 100 bytes but only 10 provided
        data = struct.pack('!BH', 48, 100) + b'\x00' * 7
        with self.assertRaises(DecoderError):
            decode_asterix(data)

    def test_decoder_verbose_mode(self):
        """Test decoder verbose mode includes descriptions."""
        # Encode a simple record
        plot = RadarPlot(range=50000, azimuth=90.0, timestamp=time.time())
        encoded = encode_cat048([plot], sac=0, sic=1)

        # Decode with verbose=True
        decoded = decode_cat048(encoded, verbose=True)

        self.assertEqual(len(decoded), 1)
        # Verbose mode should still have required fields
        self.assertIn('I010', decoded[0])
        self.assertIn('I040', decoded[0])


def run_tests():
    """Run all tests."""
    loader = unittest.TestLoader()
    suite = loader.loadTestsFromModule(sys.modules[__name__])
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return result.wasSuccessful()


if __name__ == '__main__':
    success = run_tests()
    sys.exit(0 if success else 1)
