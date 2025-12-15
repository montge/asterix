"""
Unit tests for ASTERIX CAT048 (Monoradar Target Reports) encoder.

Tests individual data item encoding, complete record encoding,
and round-trip validation.

License: GPL-3.0
"""

import unittest
import struct
import time
from asterix.radar_integration.encoder.cat048 import (
    encode_i140,
    encode_i020,
    encode_i040,
    encode_i070,
    encode_i130,
    encode_i220,
    encode_cat048_record,
    encode_cat048_datablock,
    encode_cat048,
)


class TestI140TimeOfDay(unittest.TestCase):
    """Tests for I140: Time of Day encoding."""

    def test_length(self):
        """Test output is always 3 bytes."""
        data = encode_i140()
        self.assertEqual(len(data), 3)

    def test_midnight(self):
        """Test encoding at midnight."""
        midnight = 1700000000 - (1700000000 % 86400)
        data = encode_i140(midnight)
        value = struct.unpack('>I', b'\x00' + data)[0]
        self.assertEqual(value, 0)

    def test_noon(self):
        """Test encoding at noon (12:00:00)."""
        noon = 1700000000 - (1700000000 % 86400) + 43200
        data = encode_i140(noon)
        value = struct.unpack('>I', b'\x00' + data)[0]
        # 12 hours = 43200 seconds * 128 = 5,529,600 time units
        expected = 43200 * 128
        self.assertEqual(value, expected)

    def test_one_second_resolution(self):
        """Test 1/128 second resolution."""
        t1 = 1000.0
        t2 = 1001.0
        data1 = encode_i140(t1)
        data2 = encode_i140(t2)
        val1 = struct.unpack('>I', b'\x00' + data1)[0]
        val2 = struct.unpack('>I', b'\x00' + data2)[0]
        self.assertEqual(val2 - val1, 128)

    def test_default_current_time(self):
        """Test using current time as default."""
        data = encode_i140()
        self.assertEqual(len(data), 3)


class TestI020TargetReportDescriptor(unittest.TestCase):
    """Tests for I020: Target Report Descriptor."""

    def test_length(self):
        """Test output is 1 byte."""
        data = encode_i020()
        self.assertEqual(len(data), 1)

    def test_psr_type(self):
        """Test PSR (Primary Surveillance Radar) type."""
        data = encode_i020(typ="PSR")
        # PSR = 0b010 in bits 8-6
        self.assertEqual(data[0] & 0b11100000, 0b01000000)

    def test_ssr_type(self):
        """Test SSR (Secondary Surveillance Radar) type."""
        data = encode_i020(typ="SSR")
        # SSR = 0b001 in bits 8-6
        self.assertEqual(data[0] & 0b11100000, 0b00100000)

    def test_cmb_type(self):
        """Test CMB (Combined PSR + SSR) type."""
        data = encode_i020(typ="CMB")
        # CMB = 0b011 in bits 8-6
        self.assertEqual(data[0] & 0b11100000, 0b01100000)

    def test_mon_type(self):
        """Test MON (Monostatic radar) type."""
        data = encode_i020(typ="MON")
        # MON = 0b100 in bits 8-6
        self.assertEqual(data[0] & 0b11100000, 0b10000000)

    def test_simulated_flag(self):
        """Test SIM (simulated target) flag."""
        data = encode_i020(sim=True)
        self.assertEqual(data[0] & 0b00010000, 0b00010000)

    def test_not_simulated(self):
        """Test SIM flag not set."""
        data = encode_i020(sim=False)
        self.assertEqual(data[0] & 0b00010000, 0)

    def test_rad_flag(self):
        """Test RAD (report from aircraft beacon) flag."""
        data = encode_i020(rad=1)
        self.assertEqual(data[0] & 0b00001000, 0b00001000)

    def test_combined_flags(self):
        """Test multiple flags combined."""
        data = encode_i020(typ="CMB", sim=True, rad=1)
        self.assertEqual(data[0] & 0b11100000, 0b01100000)  # CMB
        self.assertEqual(data[0] & 0b00010000, 0b00010000)  # SIM
        self.assertEqual(data[0] & 0b00001000, 0b00001000)  # RAD


class TestI040MeasuredPosition(unittest.TestCase):
    """Tests for I040: Measured Position in Polar Coordinates."""

    def test_length(self):
        """Test output is 4 bytes."""
        data = encode_i040(50000, 90)
        self.assertEqual(len(data), 4)

    def test_zero_position(self):
        """Test zero range and azimuth."""
        data = encode_i040(0, 0)
        rho, theta = struct.unpack('>HH', data)
        self.assertEqual(rho, 0)
        self.assertEqual(theta, 0)

    def test_range_conversion(self):
        """Test range conversion (meters to 1/256 NM)."""
        # 1 NM = 1852 meters
        # 50 NM * 256 = 12800
        range_m = 50 * 1852
        data = encode_i040(range_m, 0)
        rho, _ = struct.unpack('>HH', data)
        self.assertEqual(rho, 12800)

    def test_azimuth_conversion(self):
        """Test azimuth conversion (degrees to 360/2^16)."""
        # 90 degrees = 90 * 65536 / 360 = 16384
        data = encode_i040(0, 90)
        _, theta = struct.unpack('>HH', data)
        self.assertEqual(theta, 16384)

    def test_azimuth_180(self):
        """Test azimuth at 180 degrees (South)."""
        data = encode_i040(0, 180)
        _, theta = struct.unpack('>HH', data)
        self.assertEqual(theta, 32768)

    def test_azimuth_270(self):
        """Test azimuth at 270 degrees (West)."""
        data = encode_i040(0, 270)
        _, theta = struct.unpack('>HH', data)
        self.assertEqual(theta, 49152)

    def test_azimuth_wraps(self):
        """Test azimuth 360 wraps to 0."""
        data = encode_i040(0, 360)
        _, theta = struct.unpack('>HH', data)
        self.assertEqual(theta, 0)

    def test_max_range(self):
        """Test maximum range (clamped to 16-bit)."""
        # Very large range should be clamped
        data = encode_i040(500000000, 0)  # 500,000 km
        rho, _ = struct.unpack('>HH', data)
        self.assertEqual(rho, 65535)  # Max 16-bit value


class TestI070Mode3A(unittest.TestCase):
    """Tests for I070: Mode-3/A Code."""

    def test_length(self):
        """Test output is 2 bytes."""
        data = encode_i070(0o1234)
        self.assertEqual(len(data), 2)

    def test_zero_code(self):
        """Test Mode 3/A code 0."""
        data = encode_i070(0)
        value = struct.unpack('>H', data)[0]
        self.assertEqual(value & 0x0FFF, 0)

    def test_typical_code(self):
        """Test typical Mode 3/A code."""
        # 1234 octal = 668 decimal
        data = encode_i070(0o1234)
        value = struct.unpack('>H', data)[0]
        self.assertEqual(value & 0x0FFF, 0o1234)

    def test_max_code_7777(self):
        """Test maximum Mode 3/A code (7777 octal)."""
        # 7777 octal = 4095 decimal
        data = encode_i070(0o7777)
        value = struct.unpack('>H', data)[0]
        self.assertEqual(value & 0x0FFF, 4095)

    def test_overflow_masked(self):
        """Test codes > 4095 are masked."""
        data = encode_i070(0x1FFF)  # > 12 bits
        value = struct.unpack('>H', data)[0]
        self.assertEqual(value & 0x0FFF, 0x0FFF)


class TestI130RadarPlotCharacteristics(unittest.TestCase):
    """Tests for I130: Radar Plot Characteristics."""

    def test_empty(self):
        """Test with no subfields."""
        data = encode_i130()
        self.assertEqual(len(data), 1)
        self.assertEqual(data[0], 0)

    def test_srl_only(self):
        """Test SRL (SSR plot runlength) subfield."""
        data = encode_i130(srl=90.0)  # 90 degrees
        self.assertGreater(len(data), 1)
        # Primary should have SRL bit set
        self.assertEqual(data[0] & 0b01000000, 0b01000000)

    def test_srr_only(self):
        """Test SRR (number of replies) subfield."""
        data = encode_i130(srr=10)
        self.assertGreater(len(data), 1)
        # Primary should have SRR bit set
        self.assertEqual(data[0] & 0b00100000, 0b00100000)

    def test_srl_and_srr(self):
        """Test both SRL and SRR subfields."""
        data = encode_i130(srl=45.0, srr=5)
        self.assertGreater(len(data), 2)
        # Primary should have both bits set
        self.assertEqual(data[0] & 0b01100000, 0b01100000)

    def test_srl_conversion(self):
        """Test SRL conversion (degrees to 0-255)."""
        # 180 degrees = 127.5 (rounded to 127)
        data = encode_i130(srl=180.0)
        srl_val = data[1]
        expected = int((180.0 / 360.0) * 255)
        self.assertEqual(srl_val, expected)


class TestI220AircraftAddress(unittest.TestCase):
    """Tests for I220: Aircraft Address (Mode S)."""

    def test_length(self):
        """Test output is 3 bytes."""
        data = encode_i220(0x3950A1)
        self.assertEqual(len(data), 3)

    def test_zero_address(self):
        """Test address 0."""
        data = encode_i220(0)
        self.assertEqual(data, b'\x00\x00\x00')

    def test_typical_address(self):
        """Test typical Mode S address."""
        data = encode_i220(0x3950A1)
        # Decode
        value = struct.unpack('>I', b'\x00' + data)[0]
        self.assertEqual(value, 0x3950A1)

    def test_max_address(self):
        """Test maximum 24-bit address."""
        data = encode_i220(0xFFFFFF)
        value = struct.unpack('>I', b'\x00' + data)[0]
        self.assertEqual(value, 0xFFFFFF)


class TestCAT048RecordEncoding(unittest.TestCase):
    """Test complete CAT048 record encoding."""

    def test_minimal_record(self):
        """Test minimal record (mandatory items only)."""
        record = encode_cat048_record(
            range_m=50000,
            azimuth_deg=90
        )
        # FSPEC + I010 + I140 + I020 + I040 + I130
        self.assertGreater(len(record), 10)

    def test_with_mode3a(self):
        """Test record with Mode 3/A code."""
        record = encode_cat048_record(
            range_m=50000,
            azimuth_deg=90,
            mode3a=0o1234
        )
        self.assertGreater(len(record), 10)

    def test_with_aircraft_address(self):
        """Test record with aircraft address."""
        record = encode_cat048_record(
            range_m=50000,
            azimuth_deg=90,
            aircraft_address=0x3950A1
        )
        self.assertGreater(len(record), 10)

    def test_complete_record(self):
        """Test complete record with all items."""
        record = encode_cat048_record(
            range_m=50000,
            azimuth_deg=90,
            timestamp=time.time(),
            sac=5,
            sic=10,
            mode3a=0o7654,
            aircraft_address=0xABCDEF,
            typ="CMB"
        )
        self.assertGreater(len(record), 15)


class TestCAT048DatablockEncoding(unittest.TestCase):
    """Test CAT048 data block encoding."""

    def test_single_record(self):
        """Test data block with single record."""
        record = encode_cat048_record(range_m=50000, azimuth_deg=90)
        block = encode_cat048_datablock([record])

        # Check category
        self.assertEqual(block[0], 48)

        # Check length
        length = struct.unpack('!H', block[1:3])[0]
        self.assertEqual(length, len(block))

    def test_multiple_records(self):
        """Test data block with multiple records."""
        records = [
            encode_cat048_record(range_m=30000, azimuth_deg=45),
            encode_cat048_record(range_m=60000, azimuth_deg=180),
            encode_cat048_record(range_m=90000, azimuth_deg=270),
        ]
        block = encode_cat048_datablock(records)

        # Check category
        self.assertEqual(block[0], 48)

        # Check length
        length = struct.unpack('!H', block[1:3])[0]
        self.assertEqual(length, len(block))


class TestCAT048HighLevelAPI(unittest.TestCase):
    """Test high-level CAT048 encoding API."""

    def test_encode_with_dicts(self):
        """Test encoding with dictionary input."""
        plots = [
            {'range': 50000, 'azimuth': 90, 'timestamp': time.time()},
            {'range': 75000, 'azimuth': 180, 'timestamp': time.time()},
        ]
        data = encode_cat048(plots, sac=0, sic=1)

        self.assertEqual(data[0], 48)
        length = struct.unpack('!H', data[1:3])[0]
        self.assertEqual(length, len(data))

    def test_encode_with_radar_plot_objects(self):
        """Test encoding with RadarPlot-like objects."""
        class MockPlot:
            def __init__(self, range_m, azimuth, timestamp):
                self.range = range_m
                self.azimuth = azimuth
                self.timestamp = timestamp

        plots = [
            MockPlot(50000, 90, time.time()),
            MockPlot(75000, 180, time.time()),
        ]
        data = encode_cat048(plots, sac=0, sic=1)

        self.assertEqual(data[0], 48)

    def test_encode_empty_plots(self):
        """Test encoding with empty plot list."""
        data = encode_cat048([], sac=0, sic=1)

        # Should still have valid header
        self.assertEqual(data[0], 48)
        length = struct.unpack('!H', data[1:3])[0]
        self.assertEqual(length, 3)  # Header only


class TestCAT048EdgeCases(unittest.TestCase):
    """Edge case tests for CAT048 encoding."""

    def test_zero_range(self):
        """Test target at zero range."""
        record = encode_cat048_record(range_m=0, azimuth_deg=0)
        self.assertGreater(len(record), 0)

    def test_max_range(self):
        """Test target at maximum range."""
        record = encode_cat048_record(range_m=500 * 1852, azimuth_deg=0)  # 500 NM
        self.assertGreater(len(record), 0)

    def test_fractional_azimuth(self):
        """Test fractional azimuth."""
        record = encode_cat048_record(range_m=50000, azimuth_deg=135.75)
        self.assertGreater(len(record), 0)

    def test_different_sac_sic(self):
        """Test with different SAC/SIC values."""
        record = encode_cat048_record(
            range_m=50000,
            azimuth_deg=90,
            sac=255,
            sic=255
        )
        self.assertGreater(len(record), 0)


if __name__ == '__main__':
    unittest.main()
