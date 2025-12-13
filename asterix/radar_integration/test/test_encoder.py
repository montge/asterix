#!/usr/bin/env python3
"""
Unit tests for ASTERIX CAT048 encoder

Tests:
- FSPEC encoding
- Individual data item encoding (I010, I040, I140, etc.)
- Data record encoding
- Data block encoding
- Complete encode_cat048() function
- Round-trip encode/decode validation

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

from asterix.radar_integration.encoder.cat048 import (
    encode_fspec,
    encode_i010,
    encode_i040,
    encode_i070,
    encode_i130,
    encode_i140,
    encode_i220,
    encode_i020,
    encode_cat048_record,
    encode_cat048_datablock,
    encode_cat048
)
from asterix.radar_integration.mock_radar import RadarPlot, MockRadar


class TestFSPECEncoding(unittest.TestCase):
    """Test Field Specification (FSPEC) encoding."""

    def test_fspec_single_octet(self):
        """Test FSPEC with items in first octet (FRN 1-7)."""
        # Items 1, 2, 3 present
        fspec = encode_fspec([1, 2, 3])
        self.assertEqual(len(fspec), 1)
        # Bit 8=1 (FRN 1), Bit 7=1 (FRN 2), Bit 6=1 (FRN 3)
        # Expected: 0b11100000 = 0xE0 (no FX bit since single octet)
        self.assertEqual(fspec[0], 0xE0)

    def test_fspec_extension(self):
        """Test FSPEC with extension to second octet."""
        # Items 1, 8 (forces two octets)
        fspec = encode_fspec([1, 8])
        self.assertEqual(len(fspec), 2)
        # First octet: bit 8=1 (FRN 1), bit 1=1 (FX)
        self.assertEqual(fspec[0] & 0x01, 0x01)  # FX bit set
        # Second octet: bit 8=1 (FRN 8), bit 1=0 (no more FX)
        self.assertEqual(fspec[1] & 0xFE, 0x80)  # FRN 8

    def test_fspec_empty(self):
        """Test FSPEC with no items."""
        fspec = encode_fspec([])
        self.assertEqual(len(fspec), 0)


class TestDataItemEncoding(unittest.TestCase):
    """Test individual data item encoding functions."""

    def test_i010_encoding(self):
        """Test I010 (SAC/SIC) encoding."""
        data = encode_i010(sac=123, sic=45)
        self.assertEqual(len(data), 2)
        self.assertEqual(data[0], 123)
        self.assertEqual(data[1], 45)

    def test_i040_encoding(self):
        """Test I040 (polar coordinates) encoding."""
        # 50 km at 45 degrees
        data = encode_i040(range_m=50000.0, azimuth_deg=45.0)
        self.assertEqual(len(data), 4)

        # Decode to verify
        rho, theta = struct.unpack('>HH', data)
        # RHO: 50000m / 1852 = 26.998 NM, * 256 = 6911.6
        self.assertAlmostEqual(rho, 6912, delta=2)
        # THETA: 45 / 360 * 65536 = 8192
        self.assertEqual(theta, 8192)

    def test_i140_encoding(self):
        """Test I140 (time of day) encoding."""
        # Use known timestamp
        timestamp = 1234567890.0  # Some arbitrary timestamp
        data = encode_i140(timestamp)
        self.assertEqual(len(data), 3)

        # Decode to verify format
        time_value = struct.unpack('>I', b'\x00' + data)[0]
        self.assertGreater(time_value, 0)

    def test_i070_encoding(self):
        """Test I070 (Mode 3/A) encoding."""
        # Mode 3/A code 0o1234 (octal)
        data = encode_i070(0o1234)
        self.assertEqual(len(data), 2)
        # Verify the code is preserved in binary
        code_value = struct.unpack('>H', data)[0]
        self.assertEqual(code_value & 0x0FFF, 0o1234)

    def test_i130_encoding(self):
        """Test I130 (radar plot characteristics) encoding."""
        data = encode_i130(srl=1.0, srr=5)
        self.assertGreaterEqual(len(data), 1)  # At least primary byte

    def test_i220_encoding(self):
        """Test I220 (aircraft address) encoding."""
        address = 0xABCDEF
        data = encode_i220(address)
        self.assertEqual(len(data), 3)
        decoded = struct.unpack('>I', b'\x00' + data)[0]
        self.assertEqual(decoded, address)

    def test_i020_encoding(self):
        """Test I020 (target report descriptor) encoding."""
        data = encode_i020(typ="PSR")
        self.assertEqual(len(data), 1)
        # PSR type should set appropriate bits
        self.assertGreater(data[0], 0)


class TestRecordEncoding(unittest.TestCase):
    """Test complete record encoding."""

    def test_encode_cat048_record(self):
        """Test encoding a complete CAT048 record."""
        record = encode_cat048_record(
            range_m=50000.0,
            azimuth_deg=135.0,
            timestamp=time.time(),
            sac=0,
            sic=1,
            mode3a=None,
            typ="PSR"
        )
        # Record should have FSPEC + data items
        self.assertGreater(len(record), 10)

    def test_encode_cat048_record_with_mode3a(self):
        """Test record encoding with Mode 3/A."""
        record = encode_cat048_record(
            range_m=50000.0,
            azimuth_deg=135.0,
            sac=0,
            sic=1,
            mode3a=0o1234
        )
        # Should be longer with Mode 3/A
        self.assertGreater(len(record), 12)

    def test_encode_cat048_record_with_address(self):
        """Test record encoding with aircraft address."""
        record = encode_cat048_record(
            range_m=50000.0,
            azimuth_deg=135.0,
            sac=0,
            sic=1,
            aircraft_address=0x123456
        )
        # Should be longer with address
        self.assertGreater(len(record), 14)


class TestDataBlockEncoding(unittest.TestCase):
    """Test data block encoding."""

    def test_encode_datablock_single_record(self):
        """Test encoding a data block with single record."""
        record = encode_cat048_record(
            range_m=50000.0,
            azimuth_deg=45.0,
            sac=0,
            sic=1
        )
        datablock = encode_cat048_datablock([record])

        # Check header
        self.assertGreaterEqual(len(datablock), 3)
        self.assertEqual(datablock[0], 48)  # CAT
        length = struct.unpack('!H', datablock[1:3])[0]
        self.assertEqual(length, len(datablock))

    def test_encode_datablock_multiple_records(self):
        """Test encoding a data block with multiple records."""
        records = [
            encode_cat048_record(range_m=30000.0, azimuth_deg=45.0, sac=0, sic=1),
            encode_cat048_record(range_m=60000.0, azimuth_deg=180.0, sac=0, sic=1),
            encode_cat048_record(range_m=90000.0, azimuth_deg=270.0, sac=0, sic=1),
        ]
        datablock = encode_cat048_datablock(records)

        # Check header
        self.assertEqual(datablock[0], 48)
        length = struct.unpack('!H', datablock[1:3])[0]
        self.assertEqual(length, len(datablock))


class TestFullEncoding(unittest.TestCase):
    """Test complete encode_cat048() function."""

    def test_encode_cat048_with_radar_plots(self):
        """Test encoding with RadarPlot objects."""
        plots = [
            RadarPlot(range=50000.0, azimuth=45.0, timestamp=time.time()),
            RadarPlot(range=75000.0, azimuth=90.0, timestamp=time.time()),
        ]
        data = encode_cat048(plots, sac=0, sic=1)

        # Check header
        self.assertGreaterEqual(len(data), 3)
        self.assertEqual(data[0], 48)

    def test_encode_cat048_with_dicts(self):
        """Test encoding with dictionary inputs."""
        plots = [
            {'range': 50000.0, 'azimuth': 45.0, 'timestamp': time.time()},
            {'range': 75000.0, 'azimuth': 90.0, 'timestamp': time.time()},
        ]
        data = encode_cat048(plots, sac=0, sic=1)
        self.assertEqual(data[0], 48)

    def test_encode_cat048_empty(self):
        """Test encoding with no plots."""
        data = encode_cat048([], sac=0, sic=1)
        # Should still have header
        self.assertEqual(len(data), 3)
        self.assertEqual(data[0], 48)

    def test_encode_cat048_with_mock_radar(self):
        """Test encoding with mock radar generated data."""
        radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        plots = radar.generate_plots(num_targets=5)
        data = encode_cat048(plots, sac=0, sic=1)

        # Verify structure
        self.assertEqual(data[0], 48)
        length = struct.unpack('!H', data[1:3])[0]
        self.assertEqual(length, len(data))
        self.assertGreater(len(data), 50)  # Should have substantial data


class TestRoundTrip(unittest.TestCase):
    """Test round-trip encode/decode if ASTERIX parser available."""

    def setUp(self):
        """Check if ASTERIX parser is available."""
        try:
            import asterix  # noqa: F401 - imported for availability check
            self.asterix_available = True
        except ImportError:
            self.asterix_available = False

    def test_round_trip_single_plot(self):
        """Test encoding and decoding a single plot."""
        if not self.asterix_available:
            self.skipTest("ASTERIX decoder not available")

        import asterix

        # Create a plot with known values
        plots = [RadarPlot(range=50000.0, azimuth=45.0, timestamp=time.time())]
        data = encode_cat048(plots, sac=0, sic=1)

        # Decode
        decoded = asterix.parse(data)
        self.assertEqual(len(decoded), 1)

        # Check values
        rec = decoded[0]
        self.assertEqual(rec.get('category'), 48)

        # Handle both dict formats (with/without 'val' key)
        i010 = rec.get('I010', {})
        sac = i010.get('SAC', {}).get('val', i010.get('SAC')) if isinstance(i010.get('SAC'), dict) else i010.get('SAC')
        sic = i010.get('SIC', {}).get('val', i010.get('SIC')) if isinstance(i010.get('SIC'), dict) else i010.get('SIC')
        self.assertEqual(sac, 0)
        self.assertEqual(sic, 1)

        # Check position (with tolerance for encoding precision)
        if 'I040' in rec:
            i040 = rec['I040']
            # Handle both dict formats
            rho = i040.get('RHO', {}).get('val', i040.get('RHO')) if isinstance(i040.get('RHO'), dict) else i040.get('RHO')
            theta = i040.get('THETA', {}).get('val', i040.get('THETA')) if isinstance(i040.get('THETA'), dict) else i040.get('THETA')

            decoded_range = rho * 1852.0  # NM to meters
            decoded_azimuth = theta
            self.assertAlmostEqual(decoded_range, 50000.0, delta=500)
            self.assertAlmostEqual(decoded_azimuth, 45.0, delta=0.5)

    def test_round_trip_multiple_plots(self):
        """Test encoding and decoding multiple plots."""
        if not self.asterix_available:
            self.skipTest("ASTERIX decoder not available")

        import asterix

        radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
        plots = radar.generate_plots(num_targets=10)
        data = encode_cat048(plots, sac=0, sic=1)

        decoded = asterix.parse(data)
        self.assertEqual(len(decoded), len(plots))

        # Verify each record
        for plot, record in zip(plots, decoded):
            self.assertEqual(record.get('category'), 48)
            self.assertIn('I010', record)
            self.assertIn('I040', record)


if __name__ == '__main__':
    unittest.main()
