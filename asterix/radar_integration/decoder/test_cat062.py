#!/usr/bin/env python3
"""
Unit tests for ASTERIX CAT062 (System Track Data) decoder

Tests:
- Individual data item decoding (I010, I040, I060, I070, I105, I185, I200, I220, I390)
- Complete record decoding
- Data block decoding
- Verbose vs non-verbose modes
- Error handling and edge cases
- Round-trip validation

Coverage target: >80%

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-12-06
"""

import unittest
import sys
import os
import struct
import math

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', '..'))

from asterix.radar_integration.decoder import (
    DecoderError,
    CAT062Decoder,
    decode_cat062,
)


class TestCAT062DataItems(unittest.TestCase):
    """Test individual CAT062 data item decoding."""

    def setUp(self):
        """Set up test fixtures."""
        self.decoder_verbose = CAT062Decoder(verbose=True)
        self.decoder_quiet = CAT062Decoder(verbose=False)

    def test_i010_data_source_identifier(self):
        """Test I010: Data Source Identifier (SAC/SIC)."""
        # Test data: SAC=123, SIC=45
        data = struct.pack('BB', 123, 45)

        result, consumed = self.decoder_verbose._decode_i010(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['SAC'], 123)
        self.assertEqual(result['SIC'], 45)

    def test_i010_truncated(self):
        """Test I010 with truncated data."""
        data = b'\x01'  # Only 1 byte

        with self.assertRaises(DecoderError) as cm:
            self.decoder_verbose._decode_i010(data, 0)

        self.assertIn("I010 truncated", str(cm.exception))

    def test_i070_time_of_track_verbose(self):
        """Test I070: Time of Track Information (verbose mode)."""
        # Test: 12:30:45.5 = 45045.5 seconds since midnight
        time_seconds = 12 * 3600 + 30 * 60 + 45.5
        time_128 = int(time_seconds * 128)

        data = struct.pack('>I', time_128)[1:]  # 3 bytes (big-endian)
        result, consumed = self.decoder_verbose._decode_i070(data, 0)

        self.assertEqual(consumed, 3)
        self.assertIsInstance(result, dict)
        self.assertAlmostEqual(result['value'], time_seconds, places=1)
        self.assertEqual(result['time_128'], time_128)

    def test_i070_time_of_track_quiet(self):
        """Test I070: Time of Track Information (quiet mode)."""
        time_seconds = 3661.0  # 1h 1min 1s
        time_128 = int(time_seconds * 128)

        data = struct.pack('>I', time_128)[1:]  # 3 bytes
        result, consumed = self.decoder_quiet._decode_i070(data, 0)

        self.assertEqual(consumed, 3)
        self.assertIsInstance(result, float)
        self.assertAlmostEqual(result, time_seconds, places=1)

    def test_i070_truncated(self):
        """Test I070 with truncated data."""
        data = b'\x01\x02'  # Only 2 bytes

        with self.assertRaises(DecoderError) as cm:
            self.decoder_verbose._decode_i070(data, 0)

        self.assertIn("I070 truncated", str(cm.exception))

    def test_i105_wgs84_position(self):
        """Test I105: Calculated Position in WGS-84 Coordinates."""
        # Test position: 52.5°N, 13.4°E (Berlin)
        lat = 52.5
        lon = 13.4

        # Encode using LSB = 180/2^25
        lsb = 180.0 / (2 ** 25)
        lat_raw = int(lat / lsb)
        lon_raw = int(lon / lsb)

        data = struct.pack('>ii', lat_raw, lon_raw)
        result, consumed = self.decoder_verbose._decode_i105(data, 0)

        self.assertEqual(consumed, 8)
        self.assertAlmostEqual(result['lat'], lat, places=5)
        self.assertAlmostEqual(result['lon'], lon, places=5)
        self.assertIn('description', result)

    def test_i105_negative_coordinates(self):
        """Test I105 with negative coordinates (Southern/Western hemisphere)."""
        # Test: -33.8°S, -151.2°W (Sydney)
        lat = -33.8
        lon = -151.2

        lsb = 180.0 / (2 ** 25)
        lat_raw = int(lat / lsb)
        lon_raw = int(lon / lsb)

        data = struct.pack('>ii', lat_raw, lon_raw)
        result, consumed = self.decoder_verbose._decode_i105(data, 0)

        self.assertEqual(consumed, 8)
        self.assertAlmostEqual(result['lat'], lat, places=5)
        self.assertAlmostEqual(result['lon'], lon, places=5)

    def test_i105_truncated(self):
        """Test I105 with truncated data."""
        data = b'\x01\x02\x03\x04'  # Only 4 bytes

        with self.assertRaises(DecoderError) as cm:
            self.decoder_verbose._decode_i105(data, 0)

        self.assertIn("I105 truncated", str(cm.exception))

    def test_i185_velocity_verbose(self):
        """Test I185: Calculated Track Velocity (verbose mode)."""
        # Test velocity: Vx = 100 m/s, Vy = -50 m/s
        vx = 100.0  # m/s
        vy = -50.0  # m/s

        # LSB = 0.25 m/s
        vx_raw = int(vx / 0.25)
        vy_raw = int(vy / 0.25)

        data = struct.pack('>hh', vx_raw, vy_raw)
        result, consumed = self.decoder_verbose._decode_i185(data, 0)

        self.assertEqual(consumed, 4)
        self.assertAlmostEqual(result['Vx'], vx, places=1)
        self.assertAlmostEqual(result['Vy'], vy, places=1)

        # Check calculated speed and heading
        expected_speed = math.sqrt(vx**2 + vy**2)
        self.assertAlmostEqual(result['speed'], expected_speed, places=1)
        self.assertIn('heading', result)
        self.assertIn('description', result)

    def test_i185_velocity_quiet(self):
        """Test I185: Calculated Track Velocity (quiet mode)."""
        vx = 200.0
        vy = 150.0

        vx_raw = int(vx / 0.25)
        vy_raw = int(vy / 0.25)

        data = struct.pack('>hh', vx_raw, vy_raw)
        result, consumed = self.decoder_quiet._decode_i185(data, 0)

        self.assertEqual(consumed, 4)
        self.assertAlmostEqual(result['Vx'], vx, places=1)
        self.assertAlmostEqual(result['Vy'], vy, places=1)
        # Quiet mode should not have speed/heading
        self.assertNotIn('speed', result)
        self.assertNotIn('heading', result)

    def test_i185_zero_velocity(self):
        """Test I185 with zero velocity (stationary target)."""
        data = struct.pack('>hh', 0, 0)
        result, consumed = self.decoder_verbose._decode_i185(data, 0)

        self.assertEqual(consumed, 4)
        self.assertEqual(result['Vx'], 0.0)
        self.assertEqual(result['Vy'], 0.0)
        self.assertEqual(result['speed'], 0.0)

    def test_i185_truncated(self):
        """Test I185 with truncated data."""
        data = b'\x01\x02'  # Only 2 bytes

        with self.assertRaises(DecoderError) as cm:
            self.decoder_verbose._decode_i185(data, 0)

        self.assertIn("I185 truncated", str(cm.exception))

    def test_i060_mode3a_code_verbose(self):
        """Test I060: Track Mode 3/A Code (verbose mode)."""
        # Test code: 0o1234 (octal)
        code = 0o1234
        data = struct.pack('>H', code & 0x0FFF)

        result, consumed = self.decoder_verbose._decode_i060(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['code'], code)
        self.assertEqual(result['octal'], f"{code:04o}")

    def test_i060_mode3a_code_quiet(self):
        """Test I060: Track Mode 3/A Code (quiet mode)."""
        code = 0o7654
        data = struct.pack('>H', code & 0x0FFF)

        result, consumed = self.decoder_quiet._decode_i060(data, 0)

        self.assertEqual(consumed, 2)
        self.assertEqual(result['code'], code)
        self.assertNotIn('octal', result)

    def test_i060_truncated(self):
        """Test I060 with truncated data."""
        data = b'\x01'  # Only 1 byte

        with self.assertRaises(DecoderError) as cm:
            self.decoder_verbose._decode_i060(data, 0)

        self.assertIn("I060 truncated", str(cm.exception))

    def test_i040_track_number_verbose(self):
        """Test I040: Track Number (verbose mode)."""
        track_num = 1234
        data = struct.pack('>H', track_num)

        result, consumed = self.decoder_verbose._decode_i040(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIsInstance(result, dict)
        self.assertEqual(result['track_number'], track_num)

    def test_i040_track_number_quiet(self):
        """Test I040: Track Number (quiet mode)."""
        track_num = 9999
        data = struct.pack('>H', track_num)

        result, consumed = self.decoder_quiet._decode_i040(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIsInstance(result, int)
        self.assertEqual(result, track_num)

    def test_i040_truncated(self):
        """Test I040 with truncated data."""
        data = b'\x01'  # Only 1 byte

        with self.assertRaises(DecoderError) as cm:
            self.decoder_verbose._decode_i040(data, 0)

        self.assertIn("I040 truncated", str(cm.exception))

    def test_i200_mode_of_movement(self):
        """Test I200: Mode of Movement."""
        # Encode: TRANS=2, LONG=1, VERT=3, ADF=True
        # Bits: 10 01 11 1x
        byte = (2 << 6) | (1 << 4) | (3 << 2) | 0x02
        data = bytes([byte])

        result, consumed = self.decoder_verbose._decode_i200(data, 0)

        self.assertEqual(consumed, 1)
        self.assertEqual(result['TRANS'], 2)
        self.assertEqual(result['LONG'], 1)
        self.assertEqual(result['VERT'], 3)
        self.assertTrue(result['ADF'])

    def test_i200_all_zeros(self):
        """Test I200 with all fields zero."""
        data = bytes([0x00])

        result, consumed = self.decoder_verbose._decode_i200(data, 0)

        self.assertEqual(consumed, 1)
        self.assertEqual(result['TRANS'], 0)
        self.assertEqual(result['LONG'], 0)
        self.assertEqual(result['VERT'], 0)
        self.assertFalse(result['ADF'])

    def test_i200_truncated(self):
        """Test I200 with truncated data."""
        data = b''

        with self.assertRaises(DecoderError) as cm:
            self.decoder_verbose._decode_i200(data, 0)

        self.assertIn("I200 truncated", str(cm.exception))

    def test_i220_rate_of_climb_verbose(self):
        """Test I220: Calculated Rate of Climb/Descent (verbose mode)."""
        # Test: +2000 ft/min (climbing)
        rocd = 2000.0  # ft/min
        rocd_raw = int(rocd / 6.25)

        data = struct.pack('>h', rocd_raw)
        result, consumed = self.decoder_verbose._decode_i220(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIsInstance(result, dict)
        self.assertAlmostEqual(result['ROCD'], rocd, places=1)
        self.assertIn('description', result)

    def test_i220_rate_of_descent_quiet(self):
        """Test I220: Rate of Descent (quiet mode)."""
        # Test: -1500 ft/min (descending)
        rocd = -1500.0
        rocd_raw = int(rocd / 6.25)

        data = struct.pack('>h', rocd_raw)
        result, consumed = self.decoder_quiet._decode_i220(data, 0)

        self.assertEqual(consumed, 2)
        self.assertIsInstance(result, float)
        self.assertAlmostEqual(result, rocd, places=1)

    def test_i220_truncated(self):
        """Test I220 with truncated data."""
        data = b'\x01'  # Only 1 byte

        with self.assertRaises(DecoderError) as cm:
            self.decoder_verbose._decode_i220(data, 0)

        self.assertIn("I220 truncated", str(cm.exception))

    def test_i390_not_implemented(self):
        """Test I390: Flight Plan Related Data (not fully implemented)."""
        # This should raise an error indicating incomplete implementation
        data = bytes([0x80])  # Primary subfield byte

        with self.assertRaises(DecoderError) as cm:
            self.decoder_verbose._decode_i390(data, 0)

        self.assertIn("I390", str(cm.exception))
        self.assertIn("not fully implemented", str(cm.exception))

    def test_i390_truncated(self):
        """Test I390 with truncated data."""
        data = b''

        with self.assertRaises(DecoderError) as cm:
            self.decoder_verbose._decode_i390(data, 0)

        self.assertIn("I390 truncated", str(cm.exception))


class TestCAT062UnsupportedFRN(unittest.TestCase):
    """Test handling of unsupported FRNs."""

    def setUp(self):
        """Set up test fixtures."""
        self.decoder = CAT062Decoder(verbose=True)

    def test_unsupported_frn(self):
        """Test that unsupported FRN raises appropriate error."""
        data = b'\x00\x00\x00\x00'

        with self.assertRaises(DecoderError) as cm:
            self.decoder._decode_data_item(99, data, 0)

        self.assertIn("Unsupported CAT062 FRN 99", str(cm.exception))


class TestCAT062RecordDecoding(unittest.TestCase):
    """Test complete CAT062 record decoding."""

    def setUp(self):
        """Set up test fixtures."""
        self.decoder = CAT062Decoder(verbose=False)

    def _encode_cat062_record(self, frn_list, item_data_list):
        """Helper to encode a CAT062 record for testing."""
        # Encode FSPEC
        fspec = bytearray()
        max_frn = max(frn_list) if frn_list else 0
        num_octets = (max_frn + 6) // 7

        for octet_idx in range(num_octets):
            octet = 0
            for bit_idx in range(7):
                frn = octet_idx * 7 + bit_idx + 1
                if frn in frn_list:
                    octet |= (1 << (7 - bit_idx))

            # Set FX bit if not last octet
            if octet_idx < num_octets - 1:
                octet |= 0x01

            fspec.append(octet)

        # Combine FSPEC + data items
        record = bytes(fspec) + b''.join(item_data_list)
        return record

    def test_decode_simple_record(self):
        """Test decoding a simple record with I010 and I040."""
        # FRN 1 (I010) + FRN 12 (I040)
        i010_data = struct.pack('BB', 5, 10)  # SAC=5, SIC=10
        i040_data = struct.pack('>H', 1234)   # Track number = 1234

        record = self._encode_cat062_record([1, 12], [i010_data, i040_data])

        result, consumed = self.decoder.decode_record(record, 0)

        self.assertEqual(result['category'], 62)
        self.assertEqual(result['I010']['SAC'], 5)
        self.assertEqual(result['I010']['SIC'], 10)
        self.assertEqual(result['I040'], 1234)
        self.assertGreater(consumed, 0)

    def test_decode_record_with_position_and_velocity(self):
        """Test decoding record with position (I105) and velocity (I185)."""
        # FRN 5 (I105) + FRN 7 (I185)
        # Position: 48.8°N, 2.3°E
        lat, lon = 48.8, 2.3
        lsb = 180.0 / (2 ** 25)
        lat_raw = int(lat / lsb)
        lon_raw = int(lon / lsb)
        i105_data = struct.pack('>ii', lat_raw, lon_raw)

        # Velocity: Vx=100, Vy=50
        vx_raw = int(100.0 / 0.25)
        vy_raw = int(50.0 / 0.25)
        i185_data = struct.pack('>hh', vx_raw, vy_raw)

        record = self._encode_cat062_record([5, 7], [i105_data, i185_data])

        result, consumed = self.decoder.decode_record(record, 0)

        self.assertEqual(result['category'], 62)
        self.assertAlmostEqual(result['I105']['lat'], lat, places=5)
        self.assertAlmostEqual(result['I105']['lon'], lon, places=5)
        self.assertAlmostEqual(result['I185']['Vx'], 100.0, places=1)
        self.assertAlmostEqual(result['I185']['Vy'], 50.0, places=1)

    def test_decode_record_multiple_items(self):
        """Test decoding record with many data items."""
        # FRN 1, 4, 5, 9, 12, 15, 20
        items = []
        frns = []

        # I010 (FRN 1)
        frns.append(1)
        items.append(struct.pack('BB', 7, 8))

        # I070 (FRN 4)
        frns.append(4)
        items.append(struct.pack('>I', 3600 * 128)[1:])  # 1 hour

        # I105 (FRN 5)
        frns.append(5)
        lsb = 180.0 / (2 ** 25)
        items.append(struct.pack('>ii', int(45.0 / lsb), int(10.0 / lsb)))

        # I060 (FRN 9)
        frns.append(9)
        items.append(struct.pack('>H', 0o1234))

        # I040 (FRN 12)
        frns.append(12)
        items.append(struct.pack('>H', 5678))

        # I200 (FRN 15)
        frns.append(15)
        items.append(bytes([0x80]))

        # I220 (FRN 20)
        frns.append(20)
        items.append(struct.pack('>h', int(1000 / 6.25)))

        record = self._encode_cat062_record(frns, items)
        result, consumed = self.decoder.decode_record(record, 0)

        self.assertEqual(result['category'], 62)
        self.assertEqual(len(result) - 2, len(frns))  # -2 for 'category' and 'len'


class TestCAT062DatablockDecoding(unittest.TestCase):
    """Test CAT062 data block decoding."""

    def _encode_datablock(self, records):
        """Helper to encode a CAT062 data block."""
        # Header: CAT (1 byte) + LEN (2 bytes)
        records_data = b''.join(records)
        total_length = 3 + len(records_data)

        header = struct.pack('!BH', 62, total_length)
        return header + records_data

    def _encode_simple_record(self, sac, sic, track_num):
        """Helper to encode a simple record."""
        # FSPEC: FRN 1 (I010) + FRN 12 (I040)
        # 0b10000001, 0b00001000 = 0x81, 0x08
        fspec = bytes([0x81, 0x08])
        i010 = struct.pack('BB', sac, sic)
        i040 = struct.pack('>H', track_num)
        return fspec + i010 + i040

    def test_decode_single_record_datablock(self):
        """Test decoding data block with single record."""
        record = self._encode_simple_record(1, 2, 100)
        datablock = self._encode_datablock([record])

        decoder = CAT062Decoder(verbose=False)
        results = decoder.decode_datablock(datablock)

        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]['category'], 62)
        self.assertEqual(results[0]['I010']['SAC'], 1)
        self.assertEqual(results[0]['I010']['SIC'], 2)
        self.assertEqual(results[0]['I040'], 100)

    def test_decode_multiple_records_datablock(self):
        """Test decoding data block with multiple records."""
        records = []
        for i in range(5):
            records.append(self._encode_simple_record(0, 1, 1000 + i))

        datablock = self._encode_datablock(records)

        decoder = CAT062Decoder(verbose=False)
        results = decoder.decode_datablock(datablock)

        self.assertEqual(len(results), 5)
        for i, result in enumerate(results):
            self.assertEqual(result['category'], 62)
            self.assertEqual(result['I040'], 1000 + i)

    def test_decode_datablock_truncated(self):
        """Test decoding truncated data block."""
        data = b'\x3e\x00'  # Only category and partial length

        decoder = CAT062Decoder(verbose=False)
        with self.assertRaises(DecoderError):
            decoder.decode_datablock(data)

    def test_decode_datablock_wrong_category(self):
        """Test decoding data block with wrong category."""
        record = self._encode_simple_record(1, 2, 100)
        datablock = self._encode_datablock([record])

        # Change category to 48
        datablock = b'\x30' + datablock[1:]

        decoder = CAT062Decoder(verbose=False)
        with self.assertRaises(DecoderError) as cm:
            decoder.decode_datablock(datablock)

        self.assertIn("Category mismatch", str(cm.exception))

    def test_decode_datablock_length_mismatch(self):
        """Test decoding data block with incorrect length."""
        record = self._encode_simple_record(1, 2, 100)
        # Create header with wrong length
        header = struct.pack('!BH', 62, 1000)  # Claim 1000 bytes
        datablock = header + record

        decoder = CAT062Decoder(verbose=False)
        with self.assertRaises(DecoderError) as cm:
            decoder.decode_datablock(datablock)

        self.assertIn("exceeds actual data", str(cm.exception))


class TestCAT062ConvenienceFunction(unittest.TestCase):
    """Test the decode_cat062 convenience function."""

    def _encode_datablock(self, records):
        """Helper to encode a CAT062 data block."""
        records_data = b''.join(records)
        total_length = 3 + len(records_data)
        header = struct.pack('!BH', 62, total_length)
        return header + records_data

    def _encode_simple_record(self, sac, sic):
        """Helper to encode a simple record."""
        fspec = bytes([0x80])  # Only FRN 1 (I010)
        i010 = struct.pack('BB', sac, sic)
        return fspec + i010

    def test_decode_cat062_verbose(self):
        """Test decode_cat062 function in verbose mode."""
        record = self._encode_simple_record(10, 20)
        datablock = self._encode_datablock([record])

        results = decode_cat062(datablock, verbose=True)

        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]['category'], 62)

    def test_decode_cat062_quiet(self):
        """Test decode_cat062 function in quiet mode."""
        record = self._encode_simple_record(30, 40)
        datablock = self._encode_datablock([record])

        results = decode_cat062(datablock, verbose=False)

        self.assertEqual(len(results), 1)
        self.assertEqual(results[0]['category'], 62)


class TestCAT062EdgeCases(unittest.TestCase):
    """Test edge cases and boundary conditions."""

    def setUp(self):
        """Set up test fixtures."""
        self.decoder = CAT062Decoder(verbose=False)

    def test_max_coordinate_values(self):
        """Test I105 with maximum coordinate values."""
        # Maximum values: ±180 degrees
        lat = 90.0
        lon = 180.0

        lsb = 180.0 / (2 ** 25)
        lat_raw = int(lat / lsb)
        lon_raw = int(lon / lsb)

        data = struct.pack('>ii', lat_raw, lon_raw)
        result, consumed = self.decoder._decode_i105(data, 0)

        self.assertAlmostEqual(result['lat'], lat, places=4)
        self.assertAlmostEqual(result['lon'], lon, places=4)

    def test_min_coordinate_values(self):
        """Test I105 with minimum coordinate values."""
        lat = -90.0
        lon = -180.0

        lsb = 180.0 / (2 ** 25)
        lat_raw = int(lat / lsb)
        lon_raw = int(lon / lsb)

        data = struct.pack('>ii', lat_raw, lon_raw)
        result, consumed = self.decoder._decode_i105(data, 0)

        self.assertAlmostEqual(result['lat'], lat, places=4)
        self.assertAlmostEqual(result['lon'], lon, places=4)

    def test_max_velocity_values(self):
        """Test I185 with maximum velocity values."""
        # Max signed 16-bit value
        vx = 32767 * 0.25
        vy = 32767 * 0.25

        data = struct.pack('>hh', 32767, 32767)
        result, consumed = self.decoder._decode_i185(data, 0)

        self.assertAlmostEqual(result['Vx'], vx, places=1)
        self.assertAlmostEqual(result['Vy'], vy, places=1)

    def test_min_velocity_values(self):
        """Test I185 with minimum velocity values."""
        # Min signed 16-bit value
        vx = -32768 * 0.25
        vy = -32768 * 0.25

        data = struct.pack('>hh', -32768, -32768)
        result, consumed = self.decoder._decode_i185(data, 0)

        self.assertAlmostEqual(result['Vx'], vx, places=1)
        self.assertAlmostEqual(result['Vy'], vy, places=1)

    def test_midnight_time(self):
        """Test I070 with midnight time (0 seconds)."""
        data = struct.pack('>I', 0)[1:]  # 3 bytes
        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertEqual(result, 0.0)

    def test_end_of_day_time(self):
        """Test I070 with end of day time (23:59:59)."""
        time_seconds = 23 * 3600 + 59 * 60 + 59
        time_128 = int(time_seconds * 128)

        data = struct.pack('>I', time_128)[1:]  # 3 bytes
        result, consumed = self.decoder._decode_i070(data, 0)

        self.assertAlmostEqual(result, time_seconds, places=1)

    def test_zero_track_number(self):
        """Test I040 with track number 0."""
        data = struct.pack('>H', 0)
        result, consumed = self.decoder._decode_i040(data, 0)

        self.assertEqual(result, 0)

    def test_max_track_number(self):
        """Test I040 with maximum track number (65535)."""
        data = struct.pack('>H', 65535)
        result, consumed = self.decoder._decode_i040(data, 0)

        self.assertEqual(result, 65535)

    def test_max_mode3a_code(self):
        """Test I060 with maximum Mode 3/A code (0o7777)."""
        code = 0o7777
        data = struct.pack('>H', code & 0x0FFF)
        result, consumed = self.decoder._decode_i060(data, 0)

        self.assertEqual(result['code'], code)

    def test_zero_rocd(self):
        """Test I220 with zero rate of climb/descent."""
        data = struct.pack('>h', 0)
        result, consumed = self.decoder._decode_i220(data, 0)

        self.assertEqual(result, 0.0)

    def test_max_rocd(self):
        """Test I220 with maximum ROCD value."""
        rocd_max = 32767 * 6.25
        data = struct.pack('>h', 32767)
        result, consumed = self.decoder._decode_i220(data, 0)

        self.assertAlmostEqual(result, rocd_max, places=1)


class TestCAT062FRNMapping(unittest.TestCase):
    """Test FRN to data item mapping."""

    def test_frn_map_initialization(self):
        """Test that FRN map is correctly initialized."""
        decoder = CAT062Decoder(verbose=True)

        # Check key mappings
        self.assertEqual(decoder.frn_map[1], 'I010')
        self.assertEqual(decoder.frn_map[4], 'I070')
        self.assertEqual(decoder.frn_map[5], 'I105')
        self.assertEqual(decoder.frn_map[7], 'I185')
        self.assertEqual(decoder.frn_map[9], 'I060')
        self.assertEqual(decoder.frn_map[12], 'I040')
        self.assertEqual(decoder.frn_map[15], 'I200')
        self.assertEqual(decoder.frn_map[20], 'I220')
        self.assertEqual(decoder.frn_map[21], 'I390')

    def test_category_attribute(self):
        """Test that decoder has correct category."""
        decoder = CAT062Decoder()
        self.assertEqual(decoder.category, 62)

    def test_verbose_attribute(self):
        """Test that verbose attribute is correctly set."""
        decoder_v = CAT062Decoder(verbose=True)
        decoder_q = CAT062Decoder(verbose=False)

        self.assertTrue(decoder_v.verbose)
        self.assertFalse(decoder_q.verbose)


class TestCAT062Integration(unittest.TestCase):
    """Integration tests for complete decoding workflows."""

    def test_full_track_record(self):
        """Test decoding a complete track record with all essential items."""
        # Build a comprehensive record
        decoder = CAT062Decoder(verbose=True)

        # Prepare data items
        items = []
        frns = []

        # I010: SAC/SIC
        frns.append(1)
        items.append(struct.pack('BB', 15, 20))

        # I070: Time
        frns.append(4)
        items.append(struct.pack('>I', int(45000 * 128))[1:])

        # I105: Position
        frns.append(5)
        lsb = 180.0 / (2 ** 25)
        items.append(struct.pack('>ii', int(50.0 / lsb), int(8.0 / lsb)))

        # I185: Velocity
        frns.append(7)
        items.append(struct.pack('>hh', int(250 / 0.25), int(150 / 0.25)))

        # I060: Mode 3/A
        frns.append(9)
        items.append(struct.pack('>H', 0o3456))

        # I040: Track number
        frns.append(12)
        items.append(struct.pack('>H', 7890))

        # I200: Mode of movement
        frns.append(15)
        items.append(bytes([0x50]))

        # I220: ROCD
        frns.append(20)
        items.append(struct.pack('>h', int(500 / 6.25)))

        # Encode record
        fspec = bytearray()
        num_octets = (max(frns) + 6) // 7
        for octet_idx in range(num_octets):
            octet = 0
            for bit_idx in range(7):
                frn = octet_idx * 7 + bit_idx + 1
                if frn in frns:
                    octet |= (1 << (7 - bit_idx))
            if octet_idx < num_octets - 1:
                octet |= 0x01
            fspec.append(octet)

        record_data = bytes(fspec) + b''.join(items)

        # Decode
        result, consumed = decoder.decode_record(record_data, 0)

        # Verify all items present
        self.assertEqual(result['category'], 62)
        self.assertIn('I010', result)
        self.assertIn('I070', result)
        self.assertIn('I105', result)
        self.assertIn('I185', result)
        self.assertIn('I060', result)
        self.assertIn('I040', result)
        self.assertIn('I200', result)
        self.assertIn('I220', result)


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
