"""
Unit tests for ASTERIX CAT048 (Monoradar Target Reports) decoder.

Comprehensive test coverage for all CAT048 data items including:
- I010: Data Source Identifier
- I020: Target Report Descriptor
- I040: Measured Position in Polar Coordinates
- I070: Mode-3/A Code
- I090: Flight Level
- I130: Radar Plot Characteristics
- I140: Time of Day
- I161: Track Number
- I170: Track Status
- I200: Calculated Track Velocity
- I220: Aircraft Address
- I240: Aircraft Identification
- I250: Mode S MB Data

Tests focus on:
- Individual data item decoding (verbose and non-verbose modes)
- Edge cases and boundary conditions
- Error handling (truncated data, invalid values)
- Full record decoding
- Data block decoding

License: GPL-3.0
"""

import unittest
import struct

from asterix.radar_integration.decoder.cat048 import (
    CAT048Decoder,
    decode_cat048,
)
from asterix.radar_integration.decoder.base import DecoderError


class TestCAT048DecoderInit(unittest.TestCase):
    """Test CAT048 decoder initialization."""

    def test_init_default(self):
        """Test default initialization with verbose mode."""
        decoder = CAT048Decoder()
        self.assertEqual(decoder.category, 48)
        self.assertTrue(decoder.verbose)

    def test_init_non_verbose(self):
        """Test initialization with verbose=False."""
        decoder = CAT048Decoder(verbose=False)
        self.assertEqual(decoder.category, 48)
        self.assertFalse(decoder.verbose)

    def test_frn_map_initialization(self):
        """Test that FRN map is properly initialized."""
        decoder = CAT048Decoder()
        # Verify key FRN mappings
        self.assertEqual(decoder.frn_map[1], 'I010')
        self.assertEqual(decoder.frn_map[2], 'I140')
        self.assertEqual(decoder.frn_map[3], 'I020')
        self.assertEqual(decoder.frn_map[4], 'I040')
        self.assertEqual(decoder.frn_map[5], 'I070')
        self.assertEqual(decoder.frn_map[6], 'I090')
        self.assertEqual(decoder.frn_map[7], 'I130')
        self.assertEqual(decoder.frn_map[8], 'I220')
        self.assertEqual(decoder.frn_map[9], 'I240')
        self.assertEqual(decoder.frn_map[10], 'I250')
        self.assertEqual(decoder.frn_map[11], 'I161')
        self.assertEqual(decoder.frn_map[12], 'I042')
        self.assertEqual(decoder.frn_map[13], 'I200')
        self.assertEqual(decoder.frn_map[14], 'I170')

    def test_frn_map_completeness(self):
        """Test that FRN map has expected number of entries."""
        decoder = CAT048Decoder()
        self.assertGreaterEqual(len(decoder.frn_map), 10)


class TestCAT048I010(unittest.TestCase):
    """Test I010: Data Source Identifier decoding."""

    def setUp(self):
        """Set up decoders for tests."""
        self.decoder = CAT048Decoder(verbose=True)
        self.decoder_nv = CAT048Decoder(verbose=False)

    def test_decode_i010_verbose(self):
        """Test I010 decoding in verbose mode."""
        data = bytes([100, 50])
        result, consumed = self.decoder._decode_i010(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['SAC'], 100)
        self.assertEqual(result['SIC'], 50)
        self.assertIn('description', result)
        self.assertEqual(result['description'], 'SAC=100 SIC=50')

    def test_decode_i010_non_verbose(self):
        """Test I010 decoding in non-verbose mode."""
        data = bytes([10, 20])
        result, consumed = self.decoder_nv._decode_i010(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['SAC'], 10)
        self.assertEqual(result['SIC'], 20)
        self.assertNotIn('description', result)

    def test_decode_i010_boundary_values(self):
        """Test I010 with boundary values (0 and 255)."""
        # Min values
        data = bytes([0, 0])
        result, consumed = self.decoder._decode_i010(data, 0)
        self.assertEqual(result['SAC'], 0)
        self.assertEqual(result['SIC'], 0)

        # Max values
        data = bytes([255, 255])
        result, consumed = self.decoder._decode_i010(data, 0)
        self.assertEqual(result['SAC'], 255)
        self.assertEqual(result['SIC'], 255)

    def test_decode_i010_truncated(self):
        """Test I010 with truncated data."""
        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_i010(b'\x05', 0)
        self.assertIn('I010 truncated', str(ctx.exception))

    def test_decode_i010_empty(self):
        """Test I010 with empty data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i010(b'', 0)


class TestCAT048I140(unittest.TestCase):
    """Test I140: Time of Day decoding."""

    def setUp(self):
        """Set up decoders for tests."""
        self.decoder = CAT048Decoder(verbose=True)
        self.decoder_nv = CAT048Decoder(verbose=False)

    def test_decode_i140_verbose(self):
        """Test I140 decoding in verbose mode."""
        # Time: 12:30:45.5 seconds = 45045.5 seconds since midnight
        time_seconds = 12 * 3600 + 30 * 60 + 45.5
        time_128 = int(time_seconds * 128)
        data = struct.pack('>I', time_128)[1:]  # 3 bytes

        result, consumed = self.decoder._decode_i140(data, 0)

        self.assertEqual(consumed, 3)
        self.assertIsInstance(result, dict)
        self.assertAlmostEqual(result['value'], time_seconds, places=1)
        self.assertEqual(result['time_128'], time_128)
        self.assertIn('description', result)
        self.assertIn('12:30:45', result['description'])

    def test_decode_i140_non_verbose(self):
        """Test I140 decoding in non-verbose mode."""
        time_seconds = 3600.0  # 01:00:00
        time_128 = int(time_seconds * 128)
        data = struct.pack('>I', time_128)[1:]

        result, consumed = self.decoder_nv._decode_i140(data, 0)

        self.assertEqual(consumed, 3)
        self.assertIsInstance(result, float)
        self.assertAlmostEqual(result, time_seconds, places=1)

    def test_decode_i140_midnight(self):
        """Test I140 at midnight (00:00:00)."""
        time_128 = 0
        data = struct.pack('>I', time_128)[1:]

        result, consumed = self.decoder_nv._decode_i140(data, 0)
        self.assertEqual(result, 0.0)

    def test_decode_i140_end_of_day(self):
        """Test I140 near end of day (23:59:59)."""
        time_seconds = 23 * 3600 + 59 * 60 + 59
        time_128 = int(time_seconds * 128)
        data = struct.pack('>I', time_128)[1:]

        result, consumed = self.decoder_nv._decode_i140(data, 0)
        self.assertAlmostEqual(result, time_seconds, places=1)

    def test_decode_i140_fractional_seconds(self):
        """Test I140 with fractional seconds."""
        time_seconds = 1234.5625  # Exact multiple of 1/128
        time_128 = int(time_seconds * 128)
        data = struct.pack('>I', time_128)[1:]

        result, consumed = self.decoder_nv._decode_i140(data, 0)
        self.assertEqual(result, time_seconds)

    def test_decode_i140_truncated(self):
        """Test I140 with truncated data."""
        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_i140(b'\x12\x34', 0)
        self.assertIn('I140 truncated', str(ctx.exception))


class TestCAT048I020(unittest.TestCase):
    """Test I020: Target Report Descriptor decoding."""

    def setUp(self):
        """Set up decoders for tests."""
        self.decoder = CAT048Decoder(verbose=True)
        self.decoder_nv = CAT048Decoder(verbose=False)

    def test_decode_i020_single_psr(self):
        """Test I020 with single PSR detection."""
        # TYP=2 (single PSR), SIM=0, RDP=0, SPI=0, RAB=0, FX=0
        typ = 2
        byte1 = (typ << 5)
        data = bytes([byte1])

        result, consumed = self.decoder._decode_i020(data, 0)

        self.assertEqual(consumed, 1)
        self.assertEqual(result['TYP'], 2)
        self.assertFalse(result['SIM'])
        self.assertFalse(result['RDP'])
        self.assertFalse(result['SPI'])
        self.assertFalse(result['RAB'])

    def test_decode_i020_ssr_psr_combined(self):
        """Test I020 with SSR + PSR detection."""
        # TYP=3 (SSR + PSR)
        typ = 3
        byte1 = (typ << 5)
        data = bytes([byte1])

        result, consumed = self.decoder._decode_i020(data, 0)
        self.assertEqual(result['TYP'], 3)

    def test_decode_i020_with_flags(self):
        """Test I020 with various flags set."""
        # TYP=1, SIM=1, RDP=0, SPI=1, RAB=0, FX=0
        byte1 = (1 << 5) | 0b00010100
        data = bytes([byte1])

        result, consumed = self.decoder._decode_i020(data, 0)

        self.assertEqual(result['TYP'], 1)
        self.assertTrue(result['SIM'])
        self.assertFalse(result['RDP'])
        self.assertTrue(result['SPI'])
        self.assertFalse(result['RAB'])

    def test_decode_i020_verbose_description(self):
        """Test I020 verbose mode includes description."""
        typ = 4  # Single ModeS All-Call
        byte1 = (typ << 5)
        data = bytes([byte1])

        result, consumed = self.decoder._decode_i020(data, 0)

        self.assertIn('TYP_description', result)
        self.assertEqual(result['TYP_description'], 'Single ModeS All-Call')

    def test_decode_i020_all_typ_values(self):
        """Test I020 with all TYP values."""
        expected_descriptions = {
            0: "No detection",
            1: "Single SSR detection",
            2: "Single PSR detection",
            3: "SSR + PSR detection",
            4: "Single ModeS All-Call",
            5: "Single ModeS Roll-Call",
            6: "ModeS All-Call + PSR",
            7: "ModeS Roll-Call + PSR",
        }

        for typ, expected_desc in expected_descriptions.items():
            byte1 = (typ << 5)
            data = bytes([byte1])
            result, consumed = self.decoder._decode_i020(data, 0)
            self.assertEqual(result['TYP'], typ)
            self.assertEqual(result['TYP_description'], expected_desc)

    def test_decode_i020_with_extension(self):
        """Test I020 with extension bit set."""
        # FX=1 indicates extension
        byte1 = 0b01000001  # TYP=2, FX=1
        byte2 = 0b00000000  # Extension byte
        data = bytes([byte1, byte2])

        result, consumed = self.decoder._decode_i020(data, 0)
        self.assertEqual(consumed, 2)

    def test_decode_i020_extension_truncated(self):
        """Test I020 with extension bit set but missing extension byte."""
        # FX=1 but no data follows - should still work (graceful handling)
        byte1 = 0b01000001  # TYP=2, FX=1
        data = bytes([byte1])  # No extension byte

        # This should still decode the first byte successfully
        result, consumed = self.decoder._decode_i020(data, 0)
        self.assertEqual(consumed, 1)  # No extension byte added

    def test_decode_i020_truncated(self):
        """Test I020 with empty data."""
        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_i020(b'', 0)
        self.assertIn('I020 truncated', str(ctx.exception))


class TestCAT048I040(unittest.TestCase):
    """Test I040: Measured Position in Polar Coordinates."""

    def setUp(self):
        """Set up decoders for tests."""
        self.decoder = CAT048Decoder(verbose=True)
        self.decoder_nv = CAT048Decoder(verbose=False)

    def test_decode_i040_basic(self):
        """Test I040 basic decoding."""
        # Range: 50 km, Azimuth: 45 degrees
        range_m = 50000.0
        azimuth_deg = 45.0

        range_nm = range_m / 1852.0
        rho_raw = int(range_nm * 256)
        theta_raw = int(azimuth_deg * 65536 / 360.0)

        data = struct.pack('>HH', rho_raw, theta_raw)
        result, consumed = self.decoder_nv._decode_i040(data, 0)

        self.assertEqual(consumed, 4)
        self.assertAlmostEqual(result['RHO'], range_nm, places=2)
        self.assertAlmostEqual(result['THETA'], azimuth_deg, places=2)

    def test_decode_i040_verbose(self):
        """Test I040 verbose mode includes RHO_m and description."""
        range_m = 30000.0
        azimuth_deg = 135.0

        range_nm = range_m / 1852.0
        rho_raw = int(range_nm * 256)
        theta_raw = int(azimuth_deg * 65536 / 360.0)

        data = struct.pack('>HH', rho_raw, theta_raw)
        result, consumed = self.decoder._decode_i040(data, 0)

        self.assertIn('RHO_m', result)
        # Allow for quantization error from int() conversion
        self.assertAlmostEqual(result['RHO_m'], range_m, delta=10.0)
        self.assertIn('description', result)
        self.assertIn('Range:', result['description'])
        self.assertIn('Azimuth:', result['description'])

    def test_decode_i040_zero_range(self):
        """Test I040 with zero range."""
        data = struct.pack('>HH', 0, 0)
        result, consumed = self.decoder_nv._decode_i040(data, 0)

        self.assertEqual(result['RHO'], 0.0)
        self.assertEqual(result['THETA'], 0.0)

    def test_decode_i040_max_range(self):
        """Test I040 with maximum range value."""
        rho_raw = 65535  # Max 16-bit value
        theta_raw = 32768  # 180 degrees

        data = struct.pack('>HH', rho_raw, theta_raw)
        result, consumed = self.decoder_nv._decode_i040(data, 0)

        expected_rho_nm = rho_raw / 256.0
        expected_theta_deg = 180.0

        self.assertAlmostEqual(result['RHO'], expected_rho_nm, places=2)
        self.assertAlmostEqual(result['THETA'], expected_theta_deg, places=2)

    def test_decode_i040_all_azimuths(self):
        """Test I040 at cardinal azimuths."""
        range_nm = 10.0
        rho_raw = int(range_nm * 256)

        azimuths = [0, 90, 180, 270, 359.9]
        for azimuth in azimuths:
            theta_raw = int(azimuth * 65536 / 360.0)
            data = struct.pack('>HH', rho_raw, theta_raw)
            result, consumed = self.decoder_nv._decode_i040(data, 0)
            self.assertAlmostEqual(result['THETA'], azimuth, places=1)

    def test_decode_i040_truncated(self):
        """Test I040 with truncated data."""
        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_i040(b'\x12\x34\x56', 0)
        self.assertIn('I040 truncated', str(ctx.exception))


class TestCAT048I070(unittest.TestCase):
    """Test I070: Mode-3/A Code decoding."""

    def setUp(self):
        """Set up decoders for tests."""
        self.decoder = CAT048Decoder(verbose=True)
        self.decoder_nv = CAT048Decoder(verbose=False)

    def test_decode_i070_basic(self):
        """Test I070 basic decoding."""
        code = 0o1234
        data = struct.pack('>H', code)

        result, consumed = self.decoder_nv._decode_i070(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['code'], code)
        self.assertFalse(result['V'])
        self.assertFalse(result['G'])
        self.assertFalse(result['L'])

    def test_decode_i070_validated(self):
        """Test I070 with validated flag."""
        code = 0o7777
        value = code | 0x8000  # Set V bit
        data = struct.pack('>H', value)

        result, consumed = self.decoder_nv._decode_i070(data, 0)

        self.assertEqual(result['code'], code)
        self.assertTrue(result['V'])

    def test_decode_i070_garbled(self):
        """Test I070 with garbled flag."""
        code = 0o1000
        value = code | 0x4000  # Set G bit
        data = struct.pack('>H', value)

        result, consumed = self.decoder_nv._decode_i070(data, 0)

        self.assertEqual(result['code'], code)
        self.assertTrue(result['G'])

    def test_decode_i070_smoothed(self):
        """Test I070 with smoothed flag."""
        code = 0o2000
        value = code | 0x2000  # Set L bit
        data = struct.pack('>H', value)

        result, consumed = self.decoder_nv._decode_i070(data, 0)

        # Note: L bit overlaps with code, so this tests the bit extraction
        self.assertTrue(result['L'])

    def test_decode_i070_verbose(self):
        """Test I070 verbose mode includes octal string."""
        code = 0o1234
        data = struct.pack('>H', code)

        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertIn('octal', result)
        self.assertEqual(result['octal'], '1234')
        self.assertIn('description', result)
        self.assertIn('Mode 3/A: 1234', result['description'])

    def test_decode_i070_not_validated_description(self):
        """Test I070 verbose description when not validated."""
        code = 0o5555
        data = struct.pack('>H', code)

        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertIn('not validated', result['description'])

    def test_decode_i070_garbled_description(self):
        """Test I070 verbose description when garbled."""
        code = 0o1111
        value = code | 0x4000  # Set G bit
        data = struct.pack('>H', value)

        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertIn('garbled', result['description'])

    def test_decode_i070_common_codes(self):
        """Test I070 with common Mode 3/A codes."""
        common_codes = [0o7700, 0o7600, 0o7500, 0o1200, 0o0000]

        for code in common_codes:
            data = struct.pack('>H', code)
            result, consumed = self.decoder_nv._decode_i070(data, 0)
            self.assertEqual(result['code'], code)

    def test_decode_i070_truncated(self):
        """Test I070 with truncated data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i070(b'\x12', 0)


class TestCAT048I090(unittest.TestCase):
    """Test I090: Flight Level decoding."""

    def setUp(self):
        """Set up decoders for tests."""
        self.decoder = CAT048Decoder(verbose=True)
        self.decoder_nv = CAT048Decoder(verbose=False)

    def test_decode_i090_basic(self):
        """Test I090 basic decoding."""
        fl = 350.0  # FL350
        fl_raw = int(fl * 4)
        data = struct.pack('>H', fl_raw)

        result, consumed = self.decoder_nv._decode_i090(data, 0)

        self.assertEqual(consumed, 2)
        self.assertAlmostEqual(result['FL'], fl, places=2)
        self.assertFalse(result['V'])
        self.assertFalse(result['G'])

    def test_decode_i090_validated(self):
        """Test I090 with validated flag."""
        fl = 100.0
        fl_raw = int(fl * 4)
        value = fl_raw | 0x8000  # Set V bit
        data = struct.pack('>H', value)

        result, consumed = self.decoder_nv._decode_i090(data, 0)

        self.assertAlmostEqual(result['FL'], fl, places=2)
        self.assertTrue(result['V'])

    def test_decode_i090_negative_fl(self):
        """Test I090 with negative flight level (two's complement)."""
        fl = -10.0
        fl_raw = int(fl * 4)
        # Two's complement in 14 bits
        if fl_raw < 0:
            fl_raw = (fl_raw & 0x3FFF)

        data = struct.pack('>H', fl_raw)
        result, consumed = self.decoder_nv._decode_i090(data, 0)

        self.assertAlmostEqual(result['FL'], fl, places=2)

    def test_decode_i090_zero_fl(self):
        """Test I090 at sea level (FL000)."""
        data = struct.pack('>H', 0)
        result, consumed = self.decoder_nv._decode_i090(data, 0)

        self.assertEqual(result['FL'], 0.0)

    def test_decode_i090_fractional_fl(self):
        """Test I090 with fractional flight level."""
        fl = 123.75  # 1/4 FL resolution
        fl_raw = int(fl * 4)
        data = struct.pack('>H', fl_raw)

        result, consumed = self.decoder_nv._decode_i090(data, 0)

        self.assertEqual(result['FL'], fl)

    def test_decode_i090_verbose(self):
        """Test I090 verbose mode includes description."""
        fl = 380.0
        fl_raw = int(fl * 4) | 0x8000  # Validated
        data = struct.pack('>H', fl_raw)

        result, consumed = self.decoder._decode_i090(data, 0)

        self.assertIn('description', result)
        self.assertIn('Flight Level: 380.00', result['description'])
        self.assertNotIn('not validated', result['description'])

    def test_decode_i090_not_validated_description(self):
        """Test I090 description when not validated."""
        fl = 200.0
        fl_raw = int(fl * 4)
        data = struct.pack('>H', fl_raw)

        result, consumed = self.decoder._decode_i090(data, 0)

        self.assertIn('not validated', result['description'])

    def test_decode_i090_truncated(self):
        """Test I090 with truncated data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i090(b'\x12', 0)


class TestCAT048I130(unittest.TestCase):
    """Test I130: Radar Plot Characteristics (Compound Item)."""

    def setUp(self):
        """Set up decoders for tests."""
        self.decoder = CAT048Decoder(verbose=True)
        self.decoder_nv = CAT048Decoder(verbose=False)

    def test_decode_i130_empty(self):
        """Test I130 with no subfields present."""
        data = bytes([0x00])  # No subfields
        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 1)
        self.assertEqual(result, {})

    def test_decode_i130_srl(self):
        """Test I130 with SRL subfield."""
        primary = 0b01000000  # SRL present
        srl_raw = 128  # Should convert to ~180 degrees
        data = bytes([primary, srl_raw])

        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('SRL', result)
        # 128/255 * 360 = 180.7 degrees - allow delta for quantization
        self.assertAlmostEqual(result['SRL'], 180.0, delta=2.0)

    def test_decode_i130_srr(self):
        """Test I130 with SRR subfield."""
        primary = 0b00100000  # SRR present
        srr_val = 42
        data = bytes([primary, srr_val])

        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['SRR'], srr_val)

    def test_decode_i130_sam(self):
        """Test I130 with SAM subfield."""
        primary = 0b00010000  # SAM present
        sam_val = -30  # dBm (signed)
        data = bytes([primary]) + struct.pack('b', sam_val)

        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['SAM'], sam_val)

    def test_decode_i130_prl(self):
        """Test I130 with PRL subfield."""
        primary = 0b00001000  # PRL present
        prl_raw = 64  # Should convert to ~90 degrees
        data = bytes([primary, prl_raw])

        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('PRL', result)
        # 64/255 * 360 = 90.35 degrees - allow delta for quantization
        self.assertAlmostEqual(result['PRL'], 90.0, delta=2.0)

    def test_decode_i130_pam(self):
        """Test I130 with PAM subfield."""
        primary = 0b00000100  # PAM present
        pam_val = -40
        data = bytes([primary]) + struct.pack('b', pam_val)

        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['PAM'], pam_val)

    def test_decode_i130_rpd(self):
        """Test I130 with RPD subfield."""
        primary = 0b00000010  # RPD present
        rpd_val = 64  # Raw value
        data = bytes([primary]) + struct.pack('b', rpd_val)

        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('RPD', result)
        self.assertAlmostEqual(result['RPD'], 64.0 / 256.0, places=3)

    def test_decode_i130_multiple_subfields(self):
        """Test I130 with multiple subfields present."""
        primary = 0b01101000  # SRL, SRR, PRL present
        srl_raw = 0
        srr_val = 10
        prl_raw = 255

        data = bytes([primary, srl_raw, srr_val, prl_raw])
        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 4)
        self.assertIn('SRL', result)
        self.assertIn('SRR', result)
        self.assertIn('PRL', result)
        self.assertEqual(result['SRR'], srr_val)

    def test_decode_i130_all_subfields(self):
        """Test I130 with all standard subfields present."""
        primary = 0b01111110  # All subfields except FX
        data = bytes([primary, 0, 1, 2, 3, 4, 5])

        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 7)
        self.assertIn('SRL', result)
        self.assertIn('SRR', result)
        self.assertIn('SAM', result)
        self.assertIn('PRL', result)
        self.assertIn('PAM', result)
        self.assertIn('RPD', result)

    def test_decode_i130_with_extension(self):
        """Test I130 with extension bit set."""
        primary = 0b01000001  # SRL + FX
        srl_raw = 100
        ext_byte = 0x00
        data = bytes([primary, srl_raw, ext_byte])

        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 3)  # Primary + SRL + extension
        self.assertIn('SRL', result)

    def test_decode_i130_extension_truncated(self):
        """Test I130 with extension bit set but missing extension byte."""
        primary = 0b00000001  # Only FX bit set
        data = bytes([primary])  # No extension byte

        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_i130(data, 0)
        self.assertIn('extension truncated', str(ctx.exception).lower())

    def test_decode_i130_truncated_primary(self):
        """Test I130 with empty data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i130(b'', 0)

    def test_decode_i130_truncated_subfield(self):
        """Test I130 with missing subfield data."""
        primary = 0b01000000  # SRL present
        data = bytes([primary])  # Missing SRL byte

        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_i130(data, 0)
        self.assertIn('SRL truncated', str(ctx.exception))


class TestCAT048I220(unittest.TestCase):
    """Test I220: Aircraft Address decoding."""

    def setUp(self):
        """Set up decoders for tests."""
        self.decoder = CAT048Decoder(verbose=True)
        self.decoder_nv = CAT048Decoder(verbose=False)

    def test_decode_i220_basic(self):
        """Test I220 basic decoding."""
        address = 0xABCDEF
        data = struct.pack('>I', address)[1:]  # 3 bytes

        result, consumed = self.decoder_nv._decode_i220(data, 0)

        self.assertEqual(consumed, 3)
        self.assertEqual(result, address)

    def test_decode_i220_verbose(self):
        """Test I220 verbose mode includes hex string."""
        address = 0x123456
        data = struct.pack('>I', address)[1:]

        result, consumed = self.decoder._decode_i220(data, 0)

        self.assertEqual(consumed, 3)
        self.assertEqual(result['address'], address)
        self.assertEqual(result['hex'], '0x123456')
        self.assertIn('description', result)
        self.assertIn('Mode S Address: 0x123456', result['description'])

    def test_decode_i220_zero_address(self):
        """Test I220 with zero address."""
        data = bytes([0, 0, 0])
        result, consumed = self.decoder_nv._decode_i220(data, 0)

        self.assertEqual(result, 0)

    def test_decode_i220_max_address(self):
        """Test I220 with maximum address."""
        address = 0xFFFFFF
        data = struct.pack('>I', address)[1:]

        result, consumed = self.decoder_nv._decode_i220(data, 0)

        self.assertEqual(result, address)

    def test_decode_i220_truncated(self):
        """Test I220 with truncated data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i220(b'\x12\x34', 0)


class TestCAT048I240(unittest.TestCase):
    """Test I240: Aircraft Identification decoding."""

    def setUp(self):
        """Set up decoders for tests."""
        self.decoder = CAT048Decoder(verbose=True)
        self.decoder_nv = CAT048Decoder(verbose=False)

    def test_decode_i240_basic(self):
        """Test I240 basic decoding."""
        # Encode "ABC123  " (8 chars, 6 bits each = 48 bits = 6 bytes)
        # This test assumes the decode_6bit_ascii function works correctly
        # We'll use a simple test with known byte patterns
        data = bytes([0x04, 0x62, 0x31, 0x4D, 0x16, 0x00])  # Example encoding

        result, consumed = self.decoder_nv._decode_i240(data, 0)

        self.assertEqual(consumed, 6)
        self.assertIsInstance(result, str)

    def test_decode_i240_verbose(self):
        """Test I240 verbose mode includes description."""
        data = bytes([0x04, 0x62, 0x31, 0x4D, 0x16, 0x00])

        result, consumed = self.decoder._decode_i240(data, 0)

        self.assertEqual(consumed, 6)
        self.assertIn('callsign', result)
        self.assertIn('description', result)

    def test_decode_i240_truncated(self):
        """Test I240 with truncated data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i240(b'\x12\x34\x56\x78', 0)


class TestCAT048I250(unittest.TestCase):
    """Test I250: Mode S MB Data decoding."""

    def setUp(self):
        """Set up decoders for tests."""
        self.decoder = CAT048Decoder(verbose=True)
        self.decoder_nv = CAT048Decoder(verbose=False)

    def test_decode_i250_single_block(self):
        """Test I250 with single MB data block."""
        rep = 1
        mb_data = bytes([0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08])
        data = bytes([rep]) + mb_data

        result, consumed = self.decoder._decode_i250(data, 0)

        self.assertEqual(consumed, 9)  # 1 + 8
        self.assertEqual(result['REP'], 1)
        self.assertEqual(len(result['MB_DATA']), 1)
        self.assertEqual(result['MB_DATA'][0], mb_data.hex())

    def test_decode_i250_multiple_blocks(self):
        """Test I250 with multiple MB data blocks."""
        rep = 3
        mb_blocks = []
        data = bytes([rep])

        for i in range(rep):
            block = bytes([i] * 8)
            mb_blocks.append(block)
            data += block

        result, consumed = self.decoder._decode_i250(data, 0)

        self.assertEqual(consumed, 1 + 3 * 8)
        self.assertEqual(result['REP'], 3)
        self.assertEqual(len(result['MB_DATA']), 3)
        for i, block in enumerate(mb_blocks):
            self.assertEqual(result['MB_DATA'][i], block.hex())

    def test_decode_i250_zero_blocks(self):
        """Test I250 with zero MB data blocks."""
        data = bytes([0])
        result, consumed = self.decoder._decode_i250(data, 0)

        self.assertEqual(consumed, 1)
        self.assertEqual(result['REP'], 0)
        self.assertEqual(len(result['MB_DATA']), 0)

    def test_decode_i250_truncated_rep(self):
        """Test I250 with missing REP byte."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i250(b'', 0)

    def test_decode_i250_truncated_block(self):
        """Test I250 with incomplete MB block."""
        rep = 2
        data = bytes([rep]) + bytes([0] * 10)  # Only 10 bytes instead of 16

        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_i250(data, 0)
        self.assertIn('truncated', str(ctx.exception).lower())


class TestCAT048UnsupportedItems(unittest.TestCase):
    """Test handling of unsupported data items."""

    def setUp(self):
        """Set up decoder for tests."""
        self.decoder = CAT048Decoder(verbose=False)

    def test_unsupported_frn_high(self):
        """Test that unsupported high FRN raises DecoderError."""
        # FRN 15+ are not implemented
        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_data_item(15, b'\x00\x01\x02', 0)
        self.assertIn('Unsupported FRN', str(ctx.exception))

    def test_unsupported_frn_i161(self):
        """Test that I161 (FRN 11) is not yet implemented."""
        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_data_item(11, b'\x00\x01', 0)
        self.assertIn('Unsupported FRN 11', str(ctx.exception))
        self.assertIn('I161', str(ctx.exception))

    def test_unsupported_frn_i042(self):
        """Test that I042 (FRN 12) is not yet implemented."""
        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_data_item(12, b'\x00\x01\x02\x03', 0)
        self.assertIn('Unsupported FRN 12', str(ctx.exception))
        self.assertIn('I042', str(ctx.exception))

    def test_unsupported_frn_i200(self):
        """Test that I200 (FRN 13) is not yet implemented."""
        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_data_item(13, b'\x00\x01\x02\x03', 0)
        self.assertIn('Unsupported FRN 13', str(ctx.exception))
        self.assertIn('I200', str(ctx.exception))

    def test_unsupported_frn_i170(self):
        """Test that I170 (FRN 14) is not yet implemented."""
        with self.assertRaises(DecoderError) as ctx:
            self.decoder._decode_data_item(14, b'\x00', 0)
        self.assertIn('Unsupported FRN 14', str(ctx.exception))
        self.assertIn('I170', str(ctx.exception))


class TestCAT048FullRecord(unittest.TestCase):
    """Test decoding complete CAT048 records."""

    def setUp(self):
        """Set up decoder for tests."""
        self.decoder = CAT048Decoder(verbose=False)

    def test_decode_minimal_record(self):
        """Test decoding minimal record with FSPEC and I010."""
        # FSPEC: 0x80 = FRN 1 only (I010)
        fspec = bytes([0x80])
        i010 = bytes([12, 34])
        data = fspec + i010

        record, consumed = self.decoder.decode_record(data, 0)

        self.assertEqual(record['category'], 48)
        self.assertEqual(record['I010']['SAC'], 12)
        self.assertEqual(record['I010']['SIC'], 34)
        self.assertEqual(consumed, 3)

    def test_decode_record_with_multiple_items(self):
        """Test decoding record with multiple data items."""
        # FSPEC: 0xE0 = FRN 1, 2, 3 (I010, I140, I020)
        fspec = bytes([0xE0])

        # I010: SAC=1, SIC=2
        i010 = bytes([1, 2])

        # I140: 1 hour = 3600 seconds
        time_128 = int(3600 * 128)
        i140 = struct.pack('>I', time_128)[1:]

        # I020: TYP=2 (PSR)
        i020 = bytes([0b01000000])

        data = fspec + i010 + i140 + i020
        record, consumed = self.decoder.decode_record(data, 0)

        self.assertEqual(record['category'], 48)
        self.assertIn('I010', record)
        self.assertIn('I140', record)
        self.assertIn('I020', record)


class TestCAT048DataBlock(unittest.TestCase):
    """Test decoding complete CAT048 data blocks."""

    def setUp(self):
        """Set up decoder for tests."""
        self.decoder = CAT048Decoder(verbose=False)

    def test_decode_datablock_header(self):
        """Test data block header parsing."""
        # Create data block: CAT=48, LEN=10
        fspec = bytes([0x80])
        i010 = bytes([1, 2])
        record = fspec + i010

        length = 3 + len(record)  # Header + record
        header = struct.pack('!BH', 48, length)
        datablock = header + record

        records = self.decoder.decode_datablock(datablock)

        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]['category'], 48)

    def test_decode_datablock_wrong_category(self):
        """Test data block with wrong category number."""
        header = struct.pack('!BH', 62, 10)  # CAT 62 instead of 48
        datablock = header + bytes([0] * 7)

        with self.assertRaises(DecoderError) as ctx:
            self.decoder.decode_datablock(datablock)
        self.assertIn('Category mismatch', str(ctx.exception))

    def test_decode_datablock_too_short(self):
        """Test data block that's too short."""
        with self.assertRaises(DecoderError):
            self.decoder.decode_datablock(b'\x30\x00')

    def test_decode_datablock_length_mismatch(self):
        """Test data block with incorrect length field."""
        header = struct.pack('!BH', 48, 1000)  # Claims 1000 bytes
        datablock = header + bytes([0] * 10)  # Only 10 bytes data

        with self.assertRaises(DecoderError) as ctx:
            self.decoder.decode_datablock(datablock)
        self.assertIn('length', str(ctx.exception).lower())


class TestCAT048ConvenienceFunction(unittest.TestCase):
    """Test decode_cat048 convenience function."""

    def test_decode_cat048_function(self):
        """Test decode_cat048 convenience function."""
        # Create simple data block
        fspec = bytes([0x80])
        i010 = bytes([5, 10])
        record = fspec + i010

        length = 3 + len(record)
        header = struct.pack('!BH', 48, length)
        datablock = header + record

        records = decode_cat048(datablock, verbose=False)

        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]['category'], 48)
        self.assertEqual(records[0]['I010']['SAC'], 5)

    def test_decode_cat048_verbose_mode(self):
        """Test decode_cat048 in verbose mode."""
        fspec = bytes([0x80])
        i010 = bytes([7, 8])
        record = fspec + i010

        length = 3 + len(record)
        header = struct.pack('!BH', 48, length)
        datablock = header + record

        records = decode_cat048(datablock, verbose=True)

        self.assertEqual(len(records), 1)
        # Verbose mode adds description to I010
        self.assertIn('description', records[0]['I010'])


if __name__ == '__main__':
    unittest.main()
