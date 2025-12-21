"""
Unit tests for ASTERIX CAT021 (ADS-B) decoder.

Tests decoding of individual data items. Tests focus on data item
decoding rather than full record round-trips (since encoder/decoder
FRN support may differ).

License: GPL-3.0
"""

import unittest
import struct

from asterix.radar_integration.decoder.cat021 import (
    CAT021Decoder,
    decode_cat021,
)
from asterix.radar_integration.decoder.base import DecoderError


class TestCAT021DecoderInit(unittest.TestCase):
    """Test CAT021 decoder initialization."""

    def test_init_default(self):
        """Test default initialization."""
        decoder = CAT021Decoder()
        self.assertEqual(decoder.category, 21)
        self.assertTrue(decoder.verbose)

    def test_init_non_verbose(self):
        """Test non-verbose initialization."""
        decoder = CAT021Decoder(verbose=False)
        self.assertFalse(decoder.verbose)

    def test_frn_map_initialization(self):
        """Test that FRN map is properly initialized."""
        decoder = CAT021Decoder()
        self.assertIn(1, decoder.frn_map)
        self.assertEqual(decoder.frn_map[1], 'I010')
        self.assertEqual(decoder.frn_map[6], 'I130')
        self.assertEqual(decoder.frn_map[11], 'I080')


class TestCAT021DataItemDecoding(unittest.TestCase):
    """Test individual CAT021 data item decoding."""

    def setUp(self):
        """Set up decoder for tests."""
        self.decoder = CAT021Decoder(verbose=True)
        self.decoder_nonverbose = CAT021Decoder(verbose=False)

    def test_decode_i010(self):
        """Test I010: Data Source Identifier decoding."""
        # Direct binary: SAC=5, SIC=10
        data = bytes([5, 10])
        result, consumed = self.decoder._decode_i010(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['SAC'], 5)
        self.assertEqual(result['SIC'], 10)

    def test_decode_i010_truncated(self):
        """Test I010 with truncated data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i010(b'\x05', 0)

    def test_decode_i040(self):
        """Test I040: Target Report Descriptor decoding."""
        # Binary: ATP=1 (bits 8-6=001), ARC=1 (bits 5-4=01), RC=1 (bit 3=1), RAB=0, FX=0
        # 001 01 1 0 0 = 0x2C (44 decimal)
        data = bytes([0x2C])
        result, consumed = self.decoder._decode_i040(data, 0)

        self.assertGreaterEqual(consumed, 1)
        self.assertIn('ATP', result)
        self.assertIn('ARC', result)
        self.assertIn('RC', result)
        self.assertEqual(result['ATP'], 1)
        self.assertEqual(result['ARC'], 1)
        self.assertTrue(result['RC'])

    def test_decode_i040_truncated(self):
        """Test I040 with empty data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i040(b'', 0)

    def test_decode_i040_with_extension(self):
        """Test I040 with extension bit set and extension byte present."""
        # Create data with FX bit set (0x01) and extension byte
        data = bytes([0x21, 0x80])  # ATP=1, FX=1, then extension byte
        result, consumed = self.decoder._decode_i040(data, 0)
        self.assertIn('ATP', result)
        self.assertEqual(consumed, 2)  # Should consume both bytes

    def test_decode_i040_all_bits_set(self):
        """Test I040 with all bits set in first byte."""
        data = bytes([0xFF])  # All bits set including FX
        result, consumed = self.decoder._decode_i040(data, 0)

        self.assertIn('ATP', result)
        self.assertIn('ARC', result)
        self.assertIn('RC', result)
        self.assertIn('RAB', result)
        # ATP should be 7 (bits 8-6 = 111)
        self.assertEqual(result['ATP'], 7)
        # ARC should be 3 (bits 5-4 = 11)
        self.assertEqual(result['ARC'], 3)
        self.assertTrue(result['RC'])
        self.assertTrue(result['RAB'])

    def test_decode_i040_no_bits_set(self):
        """Test I040 with no bits set (all zeros, no extension)."""
        data = bytes([0x00])  # All bits clear, FX=0
        result, consumed = self.decoder._decode_i040(data, 0)

        self.assertEqual(consumed, 1)
        self.assertEqual(result['ATP'], 0)
        self.assertEqual(result['ARC'], 0)
        self.assertFalse(result['RC'])
        self.assertFalse(result['RAB'])

    def test_decode_i040_extension_no_data(self):
        """Test I040 with FX=1 but truncated (no extension byte)."""
        data = bytes([0x01])  # FX=1 but no second byte
        result, consumed = self.decoder._decode_i040(data, 0)

        # Should handle gracefully - only consume 1 byte
        self.assertEqual(consumed, 1)
        self.assertIn('ATP', result)

    def test_decode_i040_at_offset(self):
        """Test I040 decoding at non-zero offset."""
        # 0x40 = 0b01000000 = ATP=2 (bits 8-6=010), ARC=0, RC=0, RAB=0, FX=0
        data = b'\xFF\xFF' + bytes([0x40])
        result, consumed = self.decoder._decode_i040(data, 2)

        self.assertEqual(consumed, 1)
        self.assertEqual(result['ATP'], 2)
        self.assertEqual(result['ARC'], 0)

    def test_decode_i080(self):
        """Test I080: Target Address (ICAO) decoding."""
        addr = 0x3950A1
        # Direct binary: 24-bit address in 3 bytes
        data = struct.pack('>I', addr)[1:]  # Take last 3 bytes
        result, consumed = self.decoder._decode_i080(data, 0)

        self.assertEqual(consumed, 3)
        # Verbose mode returns dict
        self.assertIn('address', result)
        self.assertEqual(result['address'], addr)
        self.assertIn('hex', result)

    def test_decode_i080_nonverbose(self):
        """Test I080 decoding in non-verbose mode."""
        addr = 0x3950A1
        # Direct binary: 24-bit address in 3 bytes
        data = struct.pack('>I', addr)[1:]  # Take last 3 bytes
        result, consumed = self.decoder_nonverbose._decode_i080(data, 0)

        self.assertEqual(consumed, 3)
        self.assertEqual(result, addr)

    def test_decode_i080_truncated(self):
        """Test I080 with truncated data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i080(b'\x39\x50', 0)

    def test_decode_i080_min_address(self):
        """Test I080 with minimum ICAO address (0x000000)."""
        addr = 0x000000
        data = struct.pack('>I', addr)[1:]
        result, consumed = self.decoder._decode_i080(data, 0)

        self.assertEqual(consumed, 3)
        self.assertEqual(result['address'], 0)
        self.assertEqual(result['hex'], '0x000000')

    def test_decode_i080_max_address(self):
        """Test I080 with maximum ICAO address (0xFFFFFF)."""
        addr = 0xFFFFFF
        data = struct.pack('>I', addr)[1:]
        result, consumed = self.decoder._decode_i080(data, 0)

        self.assertEqual(consumed, 3)
        self.assertEqual(result['address'], addr)
        self.assertEqual(result['hex'], '0xFFFFFF')

    def test_decode_i080_real_icao_addresses(self):
        """Test I080 with real-world ICAO addresses."""
        # US registration (starts with A)
        addr = 0xA12345
        data = struct.pack('>I', addr)[1:]
        result, consumed = self.decoder._decode_i080(data, 0)

        self.assertEqual(result['address'], addr)

        # European registration
        addr = 0x3C6666
        data = struct.pack('>I', addr)[1:]
        result, consumed = self.decoder._decode_i080(data, 0)

        self.assertEqual(result['address'], addr)

    def test_decode_i080_at_offset(self):
        """Test I080 decoding at non-zero offset."""
        addr = 0x123456
        data = b'\x00\x00' + struct.pack('>I', addr)[1:]
        result, consumed = self.decoder._decode_i080(data, 2)

        self.assertEqual(consumed, 3)
        self.assertEqual(result['address'], addr)

    def test_decode_i130(self):
        """Test I130: WGS-84 Position decoding."""
        lat, lon = 48.858844, 2.294351  # Eiffel Tower
        # Encode position: 180/2^23 degrees LSB (24-bit signed integers)
        lat_raw = int(lat * (2 ** 23) / 180.0)
        lon_raw = int(lon * (2 ** 23) / 180.0)

        # Convert to unsigned 24-bit
        if lat_raw < 0:
            lat_raw += 0x1000000
        if lon_raw < 0:
            lon_raw += 0x1000000

        # Pack as 3 bytes each
        data = struct.pack('>I', lat_raw)[1:] + struct.pack('>I', lon_raw)[1:]
        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 6)
        self.assertIn('lat', result)
        self.assertIn('lon', result)
        # Check precision (should be close to original)
        self.assertAlmostEqual(result['lat'], lat, places=4)
        self.assertAlmostEqual(result['lon'], lon, places=4)

    def test_decode_i130_negative_coords(self):
        """Test I130 with negative coordinates (Southern/Western)."""
        lat, lon = -33.8688, 151.2093  # Sydney, Australia (negative lat, positive lon)
        # Encode position: 180/2^23 degrees LSB (24-bit signed integers)
        lat_raw = int(lat * (2 ** 23) / 180.0)
        lon_raw = int(lon * (2 ** 23) / 180.0)

        # Convert to unsigned 24-bit
        if lat_raw < 0:
            lat_raw += 0x1000000
        if lon_raw < 0:
            lon_raw += 0x1000000

        # Pack as 3 bytes each
        data = struct.pack('>I', lat_raw)[1:] + struct.pack('>I', lon_raw)[1:]
        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 6)
        self.assertAlmostEqual(result['lat'], lat, places=4)
        self.assertAlmostEqual(result['lon'], lon, places=4)

    def test_decode_i130_western_coords(self):
        """Test I130 with western (negative longitude) coordinates."""
        lat, lon = 40.7128, -74.0060  # New York City (positive lat, negative lon)
        # Encode position: 180/2^23 degrees LSB (24-bit signed integers)
        lat_raw = int(lat * (2 ** 23) / 180.0)
        lon_raw = int(lon * (2 ** 23) / 180.0)

        # Convert to unsigned 24-bit
        if lat_raw < 0:
            lat_raw += 0x1000000
        if lon_raw < 0:
            lon_raw += 0x1000000

        # Pack as 3 bytes each
        data = struct.pack('>I', lat_raw)[1:] + struct.pack('>I', lon_raw)[1:]
        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 6)
        self.assertAlmostEqual(result['lat'], lat, places=4)
        self.assertAlmostEqual(result['lon'], lon, places=4)

    def test_decode_i130_truncated(self):
        """Test I130 with truncated data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i130(b'\x00' * 5, 0)  # Need 6 bytes, only 5 provided

    def test_decode_i130_boundary_lat(self):
        """Test I130 with boundary latitude values (-90 to +90 degrees)."""
        # Test max positive latitude (North Pole)
        lat, lon = 90.0, 0.0
        lat_raw = int(lat * (2 ** 23) / 180.0)
        lon_raw = int(lon * (2 ** 23) / 180.0)

        if lat_raw < 0:
            lat_raw += 0x1000000
        if lon_raw < 0:
            lon_raw += 0x1000000

        data = struct.pack('>I', lat_raw)[1:] + struct.pack('>I', lon_raw)[1:]
        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 6)
        self.assertAlmostEqual(result['lat'], lat, places=4)

        # Test max negative latitude (South Pole)
        lat, lon = -90.0, 0.0
        lat_raw = int(lat * (2 ** 23) / 180.0)
        lon_raw = int(lon * (2 ** 23) / 180.0)

        if lat_raw < 0:
            lat_raw += 0x1000000
        if lon_raw < 0:
            lon_raw += 0x1000000

        data = struct.pack('>I', lat_raw)[1:] + struct.pack('>I', lon_raw)[1:]
        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 6)
        self.assertAlmostEqual(result['lat'], lat, places=4)

    def test_decode_i130_boundary_lon(self):
        """Test I130 with boundary longitude values (-180 to +180 degrees)."""
        # Test max positive longitude (near International Date Line)
        # Note: Due to 24-bit precision, exact ±180 may wrap around
        lat, lon = 0.0, 179.9
        lat_raw = int(lat * (2 ** 23) / 180.0)
        lon_raw = int(lon * (2 ** 23) / 180.0)

        if lat_raw < 0:
            lat_raw += 0x1000000
        if lon_raw < 0:
            lon_raw += 0x1000000

        data = struct.pack('>I', lat_raw)[1:] + struct.pack('>I', lon_raw)[1:]
        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 6)
        self.assertAlmostEqual(result['lon'], lon, places=1)

        # Test max negative longitude
        lat, lon = 0.0, -179.9
        lat_raw = int(lat * (2 ** 23) / 180.0)
        lon_raw = int(lon * (2 ** 23) / 180.0)

        if lat_raw < 0:
            lat_raw += 0x1000000
        if lon_raw < 0:
            lon_raw += 0x1000000

        data = struct.pack('>I', lat_raw)[1:] + struct.pack('>I', lon_raw)[1:]
        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertEqual(consumed, 6)
        self.assertAlmostEqual(result['lon'], lon, places=1)

    def test_decode_i130_verbose_description(self):
        """Test I130 verbose mode includes description field."""
        lat, lon = 51.5074, -0.1278  # London
        lat_raw = int(lat * (2 ** 23) / 180.0)
        lon_raw = int(lon * (2 ** 23) / 180.0)

        if lat_raw < 0:
            lat_raw += 0x1000000
        if lon_raw < 0:
            lon_raw += 0x1000000

        data = struct.pack('>I', lat_raw)[1:] + struct.pack('>I', lon_raw)[1:]
        result, consumed = self.decoder._decode_i130(data, 0)

        self.assertIn('description', result)
        self.assertIn('WGS-84', result['description'])

    def test_decode_i130_at_offset(self):
        """Test I130 decoding at non-zero offset."""
        lat, lon = 0.0, 0.0  # Equator, Prime Meridian
        lat_raw = int(lat * (2 ** 23) / 180.0)
        lon_raw = int(lon * (2 ** 23) / 180.0)

        # Add padding before actual data
        data = b'\xFF\xFF' + struct.pack('>I', lat_raw)[1:] + struct.pack('>I', lon_raw)[1:]
        result, consumed = self.decoder._decode_i130(data, 2)

        self.assertEqual(consumed, 6)
        self.assertAlmostEqual(result['lat'], lat, places=4)
        self.assertAlmostEqual(result['lon'], lon, places=4)

    def test_decode_i145(self):
        """Test I145: Flight Level decoding."""
        fl = 350  # FL350
        # Encode as 2-byte signed integer (1/4 FL LSB)
        fl_raw = int(fl * 4)
        data = struct.pack('>h', fl_raw)  # signed short
        result, consumed = self.decoder._decode_i145(data, 0)

        self.assertEqual(consumed, 2)
        # Verbose mode returns dict
        self.assertIn('FL', result)
        self.assertAlmostEqual(result['FL'], fl, places=2)

    def test_decode_i145_nonverbose(self):
        """Test I145 decoding in non-verbose mode."""
        fl = 350
        # Encode as 2-byte signed integer (1/4 FL LSB)
        fl_raw = int(fl * 4)
        data = struct.pack('>h', fl_raw)  # signed short
        result, consumed = self.decoder_nonverbose._decode_i145(data, 0)

        self.assertEqual(consumed, 2)
        self.assertAlmostEqual(result, fl, places=2)

    def test_decode_i145_truncated(self):
        """Test I145 with truncated data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i145(b'\x05', 0)

    def test_decode_i145_negative_fl(self):
        """Test I145 with negative flight level (below sea level)."""
        fl = -10  # Below sea level
        fl_raw = int(fl * 4)
        data = struct.pack('>h', fl_raw)
        result, consumed = self.decoder._decode_i145(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('FL', result)
        self.assertAlmostEqual(result['FL'], fl, places=2)

    def test_decode_i145_zero_fl(self):
        """Test I145 with FL0 (sea level)."""
        fl = 0
        fl_raw = int(fl * 4)
        data = struct.pack('>h', fl_raw)
        result, consumed = self.decoder._decode_i145(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('FL', result)
        self.assertEqual(result['FL'], 0.0)

    def test_decode_i145_max_fl(self):
        """Test I145 with maximum flight level."""
        fl = 600  # Very high altitude
        fl_raw = int(fl * 4)
        data = struct.pack('>h', fl_raw)
        result, consumed = self.decoder._decode_i145(data, 0)

        self.assertEqual(consumed, 2)
        self.assertAlmostEqual(result['FL'], fl, places=2)

    def test_decode_i145_fractional_fl(self):
        """Test I145 with fractional flight level (0.25 resolution)."""
        fl = 350.75  # FL350.75
        fl_raw = int(fl * 4)
        data = struct.pack('>h', fl_raw)
        result, consumed = self.decoder._decode_i145(data, 0)

        self.assertEqual(consumed, 2)
        self.assertAlmostEqual(result['FL'], fl, places=2)

    def test_decode_i145_at_offset(self):
        """Test I145 decoding at non-zero offset."""
        fl = 250
        fl_raw = int(fl * 4)
        data = b'\x00' + struct.pack('>h', fl_raw)
        result, consumed = self.decoder._decode_i145(data, 1)

        self.assertEqual(consumed, 2)
        self.assertAlmostEqual(result['FL'], fl, places=2)

    def test_decode_i140(self):
        """Test I140: Geometric Height decoding."""
        height_ft = 35000
        # Encode as 2-byte signed integer (6.25 ft LSB)
        height_raw = int(height_ft / 6.25)
        data = struct.pack('>h', height_raw)  # signed short
        result, consumed = self.decoder._decode_i140(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('height_ft', result)
        self.assertAlmostEqual(result['height_ft'], height_ft, places=0)
        self.assertIn('height_m', result)

    def test_decode_i140_nonverbose(self):
        """Test I140 decoding in non-verbose mode."""
        height_ft = 35000
        # Encode as 2-byte signed integer (6.25 ft LSB)
        height_raw = int(height_ft / 6.25)
        data = struct.pack('>h', height_raw)  # signed short
        result, consumed = self.decoder_nonverbose._decode_i140(data, 0)

        self.assertEqual(consumed, 2)
        self.assertAlmostEqual(result, height_ft, places=0)

    def test_decode_i140_truncated(self):
        """Test I140 with truncated data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i140(b'\x10', 0)

    def test_decode_i140_negative_height(self):
        """Test I140 with negative height (below sea level)."""
        height_ft = -1000  # Below sea level
        height_raw = int(height_ft / 6.25)
        data = struct.pack('>h', height_raw)
        result, consumed = self.decoder._decode_i140(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('height_ft', result)
        self.assertAlmostEqual(result['height_ft'], height_ft, places=0)
        # Check negative meters conversion
        self.assertLess(result['height_m'], 0)

    def test_decode_i140_zero_height(self):
        """Test I140 with sea level (0 ft)."""
        height_ft = 0
        height_raw = int(height_ft / 6.25)
        data = struct.pack('>h', height_raw)
        result, consumed = self.decoder._decode_i140(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['height_ft'], 0.0)
        self.assertEqual(result['height_m'], 0.0)

    def test_decode_i140_max_height(self):
        """Test I140 with maximum height."""
        # Max signed 16-bit value
        height_raw = 32767
        height_ft = height_raw * 6.25
        data = struct.pack('>h', height_raw)
        result, consumed = self.decoder._decode_i140(data, 0)

        self.assertEqual(consumed, 2)
        self.assertAlmostEqual(result['height_ft'], height_ft, places=0)

    def test_decode_i140_min_height(self):
        """Test I140 with minimum height."""
        # Min signed 16-bit value
        height_raw = -32768
        height_ft = height_raw * 6.25
        data = struct.pack('>h', height_raw)
        result, consumed = self.decoder._decode_i140(data, 0)

        self.assertEqual(consumed, 2)
        self.assertAlmostEqual(result['height_ft'], height_ft, places=0)

    def test_decode_i140_at_offset(self):
        """Test I140 decoding at non-zero offset."""
        height_ft = 10000
        height_raw = int(height_ft / 6.25)
        data = b'\xFF\xFF' + struct.pack('>h', height_raw)
        result, consumed = self.decoder._decode_i140(data, 2)

        self.assertEqual(consumed, 2)
        self.assertAlmostEqual(result['height_ft'], height_ft, places=0)

    def test_decode_i070(self):
        """Test I070: Mode 3/A Code decoding."""
        # Encode Mode 3/A code (octal 7700 = emergency)
        code = 0o7700  # 4032 decimal
        data = struct.pack('>H', code)
        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('code', result)
        self.assertEqual(result['code'], code)
        self.assertIn('octal', result)

    def test_decode_i070_truncated(self):
        """Test I070 with truncated data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i070(b'\x77', 0)

    def test_decode_i070_mode_a_codes(self):
        """Test I070 with various standard Mode 3/A codes."""
        # Test VFR code (1200 octal)
        code = 0o1200
        data = struct.pack('>H', code)
        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['code'], code)
        self.assertEqual(result['octal'], '1200')

        # Test IFR code (2000 octal)
        code = 0o2000
        data = struct.pack('>H', code)
        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertEqual(result['code'], code)
        self.assertEqual(result['octal'], '2000')

    def test_decode_i070_special_codes(self):
        """Test I070 with special emergency/priority codes."""
        # Hijack code (7500 octal)
        code = 0o7500
        data = struct.pack('>H', code)
        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['code'], code)
        self.assertEqual(result['octal'], '7500')

        # Radio failure (7600 octal)
        code = 0o7600
        data = struct.pack('>H', code)
        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertEqual(result['code'], code)
        self.assertEqual(result['octal'], '7600')

    def test_decode_i070_zero_code(self):
        """Test I070 with code 0000."""
        code = 0o0000
        data = struct.pack('>H', code)
        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['code'], 0)
        self.assertEqual(result['octal'], '0000')

    def test_decode_i070_max_code(self):
        """Test I070 with maximum 12-bit octal code (7777)."""
        code = 0o7777  # 4095 decimal
        data = struct.pack('>H', code)
        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['code'], code)
        self.assertEqual(result['octal'], '7777')

    def test_decode_i070_at_offset(self):
        """Test I070 decoding at non-zero offset."""
        code = 0o3456
        data = b'\x00\x00\x00' + struct.pack('>H', code)
        result, consumed = self.decoder._decode_i070(data, 3)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['code'], code)

    def test_decode_i090(self):
        """Test I090: Quality Indicators decoding."""
        data = bytes([0x80])  # No extension
        result, consumed = self.decoder._decode_i090(data, 0)

        self.assertGreaterEqual(consumed, 1)
        self.assertIn('quality_byte1', result)

    def test_decode_i090_with_extension(self):
        """Test I090 with extension."""
        data = bytes([0x01, 0x80])  # FX=1, extension byte
        result, consumed = self.decoder._decode_i090(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('quality_byte1', result)
        self.assertIn('quality_byte2', result)

    def test_decode_i090_truncated(self):
        """Test I090 with empty data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i090(b'', 0)

    def test_decode_i090_no_extension(self):
        """Test I090 without extension (FX=0)."""
        data = bytes([0x80])  # FX=0 (bit 1 clear)
        result, consumed = self.decoder._decode_i090(data, 0)

        self.assertEqual(consumed, 1)
        self.assertIn('quality_byte1', result)
        self.assertNotIn('quality_byte2', result)

    def test_decode_i090_extension_truncated(self):
        """Test I090 with FX=1 but no extension byte available."""
        data = bytes([0x01])  # FX=1 but no second byte
        result, consumed = self.decoder._decode_i090(data, 0)

        # Should still consume 1 byte (no crash on truncated extension)
        self.assertEqual(consumed, 1)
        self.assertIn('quality_byte1', result)

    def test_decode_i090_various_quality_values(self):
        """Test I090 with various quality indicator values."""
        # Test all bits set in first byte
        data = bytes([0xFF])
        result, consumed = self.decoder._decode_i090(data, 0)

        self.assertIn('quality_byte1', result)
        self.assertEqual(result['quality_byte1'], 0xFF)

        # Test with extension
        data = bytes([0x01, 0xAA])  # FX=1, extension byte=0xAA
        result, consumed = self.decoder._decode_i090(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['quality_byte1'], 0x01)
        self.assertEqual(result['quality_byte2'], 0xAA)

    def test_decode_i090_at_offset(self):
        """Test I090 decoding at non-zero offset."""
        data = b'\xFF\xFF' + bytes([0x42, 0x84])
        result, consumed = self.decoder._decode_i090(data, 2)

        self.assertEqual(consumed, 1)  # No FX bit, only 1 byte consumed
        self.assertEqual(result['quality_byte1'], 0x42)

    def test_decode_i073(self):
        """Test I073: Time of Message Reception decoding."""
        # Encode time as 1/128 seconds since midnight
        time_128 = 12345678
        data = struct.pack('>I', time_128)[1:]  # 3 bytes
        result, consumed = self.decoder._decode_i073(data, 0)

        self.assertEqual(consumed, 3)
        # Verbose mode returns dict
        self.assertIn('value', result)
        self.assertIn('time_128', result)
        self.assertEqual(result['time_128'], time_128)

    def test_decode_i073_nonverbose(self):
        """Test I073 decoding in non-verbose mode."""
        time_128 = 12345678
        data = struct.pack('>I', time_128)[1:]
        result, consumed = self.decoder_nonverbose._decode_i073(data, 0)

        self.assertEqual(consumed, 3)
        # Should return time in seconds
        expected_seconds = time_128 / 128.0
        self.assertAlmostEqual(result, expected_seconds, places=3)

    def test_decode_i073_truncated(self):
        """Test I073 with truncated data."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_i073(b'\x00\x00', 0)

    def test_decode_i073_midnight(self):
        """Test I073 with midnight time (0 seconds)."""
        time_128 = 0
        data = struct.pack('>I', time_128)[1:]
        result, consumed = self.decoder._decode_i073(data, 0)

        self.assertEqual(consumed, 3)
        self.assertEqual(result['value'], 0.0)
        self.assertEqual(result['time_128'], 0)

    def test_decode_i073_max_time(self):
        """Test I073 with maximum 24-bit time value."""
        time_128 = 0xFFFFFF  # Max 24-bit
        data = struct.pack('>I', time_128)[1:]
        result, consumed = self.decoder._decode_i073(data, 0)

        self.assertEqual(consumed, 3)
        expected_seconds = time_128 / 128.0
        self.assertAlmostEqual(result['value'], expected_seconds, places=3)

    def test_decode_i073_end_of_day(self):
        """Test I073 with time near end of day (86400 seconds)."""
        # 86400 seconds = 1 day
        time_seconds = 86399.0  # One second before midnight
        time_128 = int(time_seconds * 128)
        data = struct.pack('>I', time_128)[1:]
        result, consumed = self.decoder._decode_i073(data, 0)

        self.assertEqual(consumed, 3)
        self.assertAlmostEqual(result['value'], time_seconds, places=1)

    def test_decode_i073_at_offset(self):
        """Test I073 decoding at non-zero offset."""
        time_128 = 5000000
        data = b'\xFF' + struct.pack('>I', time_128)[1:]
        result, consumed = self.decoder._decode_i073(data, 1)

        self.assertEqual(consumed, 3)
        expected_seconds = time_128 / 128.0
        self.assertAlmostEqual(result['value'], expected_seconds, places=3)


class TestCAT021UnsupportedFRN(unittest.TestCase):
    """Test handling of unsupported FRNs."""

    def setUp(self):
        """Set up decoder for tests."""
        self.decoder = CAT021Decoder()

    def test_unsupported_frn(self):
        """Test that unsupported FRN raises error."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_data_item(99, b'\x00\x00', 0)

    def test_unsupported_frn_8(self):
        """Test FRN 8 (I072) is not supported."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_data_item(8, b'\x00\x00\x00', 0)

    def test_unsupported_frn_9(self):
        """Test FRN 9 (I150) is not supported."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_data_item(9, b'\x00\x00', 0)

    def test_unsupported_frn_10(self):
        """Test FRN 10 (I151) is not supported."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_data_item(10, b'\x00\x00', 0)


class TestCAT021DecodeDataItemRouting(unittest.TestCase):
    """Test _decode_data_item routing for all supported FRNs."""

    def setUp(self):
        """Set up decoder for tests."""
        self.decoder = CAT021Decoder()

    def test_decode_data_item_frn1_i010(self):
        """Test FRN 1 routes to I010."""
        data = bytes([10, 20])
        result, consumed = self.decoder._decode_data_item(1, data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('SAC', result)
        self.assertEqual(result['SAC'], 10)
        self.assertEqual(result['SIC'], 20)

    def test_decode_data_item_frn2_i040(self):
        """Test FRN 2 routes to I040."""
        data = bytes([0x44])  # ATP=2, ARC=1
        result, consumed = self.decoder._decode_data_item(2, data, 0)

        self.assertIn('ATP', result)
        self.assertEqual(result['ATP'], 2)

    def test_decode_data_item_frn3_i161(self):
        """Test FRN 3 routes to I161."""
        track_num = 100
        value = track_num << 4
        data = struct.pack('>H', value)
        result, consumed = self.decoder._decode_data_item(3, data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['track_number'], track_num)

    def test_decode_data_item_frn4_i015(self):
        """Test FRN 4 routes to I015."""
        data = bytes([42])
        result, consumed = self.decoder._decode_data_item(4, data, 0)

        self.assertEqual(consumed, 1)
        self.assertEqual(result, 42)

    def test_decode_data_item_frn5_i071(self):
        """Test FRN 5 routes to I071."""
        time_128 = 1000000
        data = struct.pack('>I', time_128)[1:]
        result, consumed = self.decoder._decode_data_item(5, data, 0)

        self.assertEqual(consumed, 3)
        self.assertIn('value', result)

    def test_decode_data_item_frn6_i130(self):
        """Test FRN 6 routes to I130."""
        lat, lon = 10.0, 20.0
        lat_raw = int(lat * (2 ** 23) / 180.0)
        lon_raw = int(lon * (2 ** 23) / 180.0)

        data = struct.pack('>I', lat_raw)[1:] + struct.pack('>I', lon_raw)[1:]
        result, consumed = self.decoder._decode_data_item(6, data, 0)

        self.assertEqual(consumed, 6)
        self.assertIn('lat', result)
        self.assertAlmostEqual(result['lat'], lat, places=4)

    def test_decode_data_item_frn11_i080(self):
        """Test FRN 11 routes to I080."""
        addr = 0xABCDEF
        data = struct.pack('>I', addr)[1:]
        result, consumed = self.decoder._decode_data_item(11, data, 0)

        self.assertEqual(consumed, 3)
        self.assertEqual(result['address'], addr)

    def test_decode_data_item_frn12_i073(self):
        """Test FRN 12 routes to I073."""
        time_128 = 5000000
        data = struct.pack('>I', time_128)[1:]
        result, consumed = self.decoder._decode_data_item(12, data, 0)

        self.assertEqual(consumed, 3)
        self.assertIn('value', result)

    def test_decode_data_item_frn16_i140(self):
        """Test FRN 16 routes to I140."""
        height_ft = 15000
        height_raw = int(height_ft / 6.25)
        data = struct.pack('>h', height_raw)
        result, consumed = self.decoder._decode_data_item(16, data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('height_ft', result)

    def test_decode_data_item_frn17_i090(self):
        """Test FRN 17 routes to I090."""
        data = bytes([0x80])
        result, consumed = self.decoder._decode_data_item(17, data, 0)

        self.assertIn('quality_byte1', result)

    def test_decode_data_item_frn19_i070(self):
        """Test FRN 19 routes to I070."""
        code = 0o7777
        data = struct.pack('>H', code)
        result, consumed = self.decoder._decode_data_item(19, data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['code'], code)

    def test_decode_data_item_frn21_i145(self):
        """Test FRN 21 routes to I145."""
        fl = 400
        fl_raw = int(fl * 4)
        data = struct.pack('>h', fl_raw)
        result, consumed = self.decoder._decode_data_item(21, data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('FL', result)


class TestCAT021RecordDecoding(unittest.TestCase):
    """Test complete CAT021 record decoding."""

    def test_decode_minimal_record(self):
        """Test decoding minimal CAT021 record with I010 and I130."""
        decoder = CAT021Decoder()

        # Build a minimal datablock with CAT021 header + FSPEC + I010 + I130
        # Header: CAT=21, length=19 (3 header + 1 FSPEC + 2 I010 + 6 I130 + 7 extra)
        cat = 21

        # FSPEC: FRN 1 (I010) and FRN 6 (I130) present
        # Bit layout: FRN1=1, FRN2-5=0, FRN6=1, FRN7=0, FX=0
        # Binary: 1000010 0 = 0x84
        fspec = bytes([0x84])

        # I010: SAC=0, SIC=1
        i010 = bytes([0, 1])

        # I130: Position (simplified - zeros for now)
        i130 = bytes([0x00] * 6)

        # Build complete record
        record_data = fspec + i010 + i130
        record_length = 3 + len(record_data)  # 3 byte header + data

        # Build datablock
        header = struct.pack('>BH', cat, record_length)
        block = header + record_data

        # Decode
        records = decoder.decode_datablock(block)

        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]['category'], 21)
        self.assertIn('I010', records[0])
        self.assertEqual(records[0]['I010']['SAC'], 0)
        self.assertEqual(records[0]['I010']['SIC'], 1)

    def test_decode_record_multiple_items(self):
        """Test decoding record with multiple data items."""
        decoder = CAT021Decoder()

        # FSPEC: FRN 1, 11, 19, 21 present (skip 6/I130 to avoid truncation issues)
        # Byte 1: FRN1=1, FRN2-7=0, FX=1
        # Byte 2: FRN8-10=0, FRN11=1, FRN12-14=0, FX=1
        # Byte 3: FRN15-18=0, FRN19=1, FRN20=0, FRN21=1, FX=0
        fspec = bytes([0x81, 0x11, 0x0A])

        # I010: SAC=5, SIC=10
        i010 = bytes([5, 10])

        # I080: ICAO address
        i080 = bytes([0x12, 0x34, 0x56])

        # I070: Mode 3/A code
        i070 = struct.pack('>H', 0o7777)

        # I145: Flight level
        i145 = struct.pack('>h', 1400)  # FL350

        record_data = fspec + i010 + i080 + i070 + i145
        record_length = 3 + len(record_data)

        header = struct.pack('>BH', 21, record_length)
        block = header + record_data

        records = decoder.decode_datablock(block)

        self.assertEqual(len(records), 1)
        record = records[0]
        self.assertIn('I010', record)
        self.assertIn('I080', record)
        self.assertIn('I070', record)
        self.assertIn('I145', record)
        self.assertEqual(record['I010']['SAC'], 5)
        self.assertEqual(record['I010']['SIC'], 10)

    def test_decode_block_wrong_category(self):
        """Test decoding with wrong category number raises error."""
        decoder = CAT021Decoder()

        # Build block with wrong category (62 instead of 21)
        header = struct.pack('>BH', 62, 10)
        block = header + b'\x00' * 7

        with self.assertRaises(DecoderError) as ctx:
            decoder.decode_datablock(block)

        self.assertIn('Category mismatch', str(ctx.exception))

    def test_decode_block_too_short(self):
        """Test decoding block that's too short raises error."""
        decoder = CAT021Decoder()

        # Block with only 2 bytes (need at least 3 for header)
        with self.assertRaises(DecoderError):
            decoder.decode_datablock(b'\x15\x00')

    def test_decode_block_length_exceeds_data(self):
        """Test decoding when declared length exceeds actual data."""
        decoder = CAT021Decoder()

        # Header says 100 bytes but only provide 10
        header = struct.pack('>BH', 21, 100)
        block = header + b'\x00' * 7

        with self.assertRaises(DecoderError) as ctx:
            decoder.decode_datablock(block)

        self.assertIn('exceeds actual data', str(ctx.exception))


class TestCAT021ConvenienceFunction(unittest.TestCase):
    """Test the decode_cat021 convenience function."""

    def test_decode_cat021_basic(self):
        """Test decode_cat021 convenience function."""
        # Build a minimal datablock
        cat = 21
        fspec = bytes([0x84])  # FRN 1 and 6
        i010 = bytes([0, 1])
        i130 = bytes([0x00] * 6)

        record_data = fspec + i010 + i130
        record_length = 3 + len(record_data)
        header = struct.pack('>BH', cat, record_length)
        block = header + record_data

        records = decode_cat021(block, verbose=True)

        self.assertEqual(len(records), 1)
        self.assertEqual(records[0]['category'], 21)

    def test_decode_cat021_nonverbose(self):
        """Test decode_cat021 in non-verbose mode."""
        # Build a minimal datablock
        cat = 21
        fspec = bytes([0x84])  # FRN 1 and 6
        i010 = bytes([0, 1])
        i130 = bytes([0x00] * 6)

        record_data = fspec + i010 + i130
        record_length = 3 + len(record_data)
        header = struct.pack('>BH', cat, record_length)
        block = header + record_data

        records = decode_cat021(block, verbose=False)

        self.assertEqual(len(records), 1)


class TestCAT021AdditionalDataItems(unittest.TestCase):
    """Test additional CAT021 data items."""

    def test_decode_i161(self):
        """Test I161: Track Number decoding."""
        decoder = CAT021Decoder()
        # Track number 4095 (max 12-bit value), shifted left 4 bits
        track_num = 4095
        value = track_num << 4
        data = struct.pack('>H', value)
        result, consumed = decoder._decode_i161(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIn('track_number', result)
        self.assertEqual(result['track_number'], track_num)

    def test_decode_i161_truncated(self):
        """Test I161 with truncated data."""
        decoder = CAT021Decoder()
        with self.assertRaises(DecoderError):
            decoder._decode_i161(b'\x10', 0)

    def test_decode_i161_min_value(self):
        """Test I161 with minimum track number (0)."""
        decoder = CAT021Decoder()
        track_num = 0
        value = track_num << 4
        data = struct.pack('>H', value)
        result, consumed = decoder._decode_i161(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['track_number'], 0)

    def test_decode_i161_at_offset(self):
        """Test I161 decoding at non-zero offset."""
        decoder = CAT021Decoder()
        track_num = 2048
        value = track_num << 4
        data = b'\x00\x00' + struct.pack('>H', value)
        result, consumed = decoder._decode_i161(data, 2)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['track_number'], track_num)

    def test_decode_i015(self):
        """Test I015: Service Identification decoding."""
        decoder = CAT021Decoder()
        service_id = 42
        data = bytes([service_id])
        result, consumed = decoder._decode_i015(data, 0)

        self.assertEqual(consumed, 1)
        self.assertEqual(result, service_id)

    def test_decode_i015_truncated(self):
        """Test I015 with empty data."""
        decoder = CAT021Decoder()
        with self.assertRaises(DecoderError):
            decoder._decode_i015(b'', 0)

    def test_decode_i015_boundary_values(self):
        """Test I015 with min/max values."""
        decoder = CAT021Decoder()
        # Test min value (0)
        result, _ = decoder._decode_i015(bytes([0]), 0)
        self.assertEqual(result, 0)

        # Test max value (255)
        result, _ = decoder._decode_i015(bytes([255]), 0)
        self.assertEqual(result, 255)

    def test_decode_i071(self):
        """Test I071: Time of Applicability decoding."""
        decoder = CAT021Decoder(verbose=True)
        # Time in 1/128 seconds
        time_128 = 12345678
        data = struct.pack('>I', time_128)[1:]  # 3 bytes
        result, consumed = decoder._decode_i071(data, 0)

        self.assertEqual(consumed, 3)
        self.assertIn('value', result)
        self.assertIn('time_128', result)
        self.assertEqual(result['time_128'], time_128)

    def test_decode_i071_nonverbose(self):
        """Test I071 in non-verbose mode."""
        decoder = CAT021Decoder(verbose=False)
        time_128 = 12345678
        data = struct.pack('>I', time_128)[1:]
        result, consumed = decoder._decode_i071(data, 0)

        self.assertEqual(consumed, 3)
        expected_seconds = time_128 / 128.0
        self.assertAlmostEqual(result, expected_seconds, places=3)

    def test_decode_i071_truncated(self):
        """Test I071 with truncated data."""
        decoder = CAT021Decoder()
        with self.assertRaises(DecoderError):
            decoder._decode_i071(b'\x00\x00', 0)

    def test_decode_i071_midnight(self):
        """Test I071 with time at midnight (0 seconds)."""
        decoder = CAT021Decoder(verbose=True)
        time_128 = 0
        data = struct.pack('>I', time_128)[1:]
        result, consumed = decoder._decode_i071(data, 0)

        self.assertEqual(consumed, 3)
        self.assertEqual(result['value'], 0.0)
        self.assertEqual(result['time_128'], 0)

    def test_decode_i071_max_value(self):
        """Test I071 with maximum 24-bit value."""
        decoder = CAT021Decoder(verbose=False)
        time_128 = 0xFFFFFF  # Max 24-bit value
        data = struct.pack('>I', time_128)[1:]
        result, consumed = decoder._decode_i071(data, 0)

        self.assertEqual(consumed, 3)
        expected_seconds = time_128 / 128.0
        self.assertAlmostEqual(result, expected_seconds, places=3)


if __name__ == '__main__':
    unittest.main()
