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


class TestCAT021UnsupportedFRN(unittest.TestCase):
    """Test handling of unsupported FRNs."""

    def setUp(self):
        """Set up decoder for tests."""
        self.decoder = CAT021Decoder()

    def test_unsupported_frn(self):
        """Test that unsupported FRN raises error."""
        with self.assertRaises(DecoderError):
            self.decoder._decode_data_item(99, b'\x00\x00', 0)


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


if __name__ == '__main__':
    unittest.main()
