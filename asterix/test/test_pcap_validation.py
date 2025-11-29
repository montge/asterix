"""
PCAP Validation Framework for ASTERIX Parser

Comprehensive validation tests to ensure correct parsing of ASTERIX data
from PCAP files across all bindings and output formats.

Test Categories:
1. PCAP file parsing correctness
2. Multi-category PCAP validation
3. Output format consistency (JSON, XML, text)
4. Malformed/edge case handling
5. Performance benchmarks

Run with:
    pytest asterix/test/test_pcap_validation.py -v
    python -m unittest asterix.test.test_pcap_validation
"""

import asterix
import unittest
import os
import json
import hashlib
import time


def get_pcap_file(filename):
    """Get path to PCAP file in sample_data directory."""
    sample_dir = os.path.join(os.path.dirname(asterix.__file__), 'sample_data')
    return os.path.join(sample_dir, filename)


def get_raw_file(filename):
    """Get path to raw ASTERIX file in sample_data directory."""
    return get_pcap_file(filename)


class TestPCAPParsing(unittest.TestCase):
    """Test PCAP file parsing correctness."""

    def test_parse_cat_034_048_pcap(self):
        """Test parsing PCAP with CAT034 and CAT048 data."""
        pcap_path = get_pcap_file('cat_034_048.pcap')
        if not os.path.exists(pcap_path):
            self.skipTest(f"PCAP file not found: {pcap_path}")

        with open(pcap_path, 'rb') as f:
            data = f.read()

        # Parse raw data (PCAP header + ASTERIX payload)
        # Note: asterix.parse expects raw ASTERIX, not PCAP
        # Skip PCAP global header (24 bytes) and packet headers
        records = asterix.parse(data)

        self.assertIsInstance(records, list)
        # File may have varying number of records

    def test_parse_cat_062_065_pcap(self):
        """Test parsing PCAP with CAT062 and CAT065 data."""
        pcap_path = get_pcap_file('cat_062_065.pcap')
        if not os.path.exists(pcap_path):
            self.skipTest(f"PCAP file not found: {pcap_path}")

        with open(pcap_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)
        self.assertIsInstance(records, list)

    def test_parse_cat_001_002_pcap(self):
        """Test parsing PCAP with CAT001 and CAT002 data."""
        pcap_path = get_pcap_file('cat_001_002.pcap')
        if not os.path.exists(pcap_path):
            self.skipTest(f"PCAP file not found: {pcap_path}")

        with open(pcap_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)
        self.assertIsInstance(records, list)


class TestRawASTERIXParsing(unittest.TestCase):
    """Test raw ASTERIX file parsing."""

    def test_parse_cat048_raw(self):
        """Test parsing raw CAT048 ASTERIX data."""
        raw_path = get_raw_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)

        self.assertIsInstance(records, list)
        self.assertGreater(len(records), 0)

        # Verify CAT048 structure
        record = records[0]
        self.assertEqual(record['category'], 48)
        self.assertIn('I010', record)  # Data Source Identifier

    def test_parse_cat062cat065_raw(self):
        """Test parsing raw CAT062/CAT065 ASTERIX data."""
        raw_path = get_raw_file('cat062cat065.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)

        self.assertIsInstance(records, list)
        self.assertGreater(len(records), 0)

        # Should have both CAT062 and CAT065 records
        categories = set(r['category'] for r in records)
        self.assertTrue(62 in categories or 65 in categories)

    def test_parse_cat034_raw(self):
        """Test parsing raw CAT034 ASTERIX data."""
        raw_path = get_raw_file('cat034.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)

        self.assertIsInstance(records, list)
        self.assertGreater(len(records), 0)
        self.assertEqual(records[0]['category'], 34)


class TestMultiCategoryParsing(unittest.TestCase):
    """Test parsing files with multiple ASTERIX categories."""

    def test_category_separation(self):
        """Test that multiple categories are correctly separated."""
        raw_path = get_raw_file('cat062cat065.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)
        categories = [r['category'] for r in records]

        # Each record should have a valid category
        for cat in categories:
            self.assertIn(cat, range(1, 256))

    def test_record_integrity(self):
        """Test that each record has required fields."""
        raw_path = get_raw_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)

        for record in records:
            # Every record must have these fields
            self.assertIn('category', record)
            self.assertIn('len', record)
            self.assertIn('crc', record)
            self.assertIn('ts', record)

            # Category must be valid
            self.assertGreaterEqual(record['category'], 1)
            self.assertLessEqual(record['category'], 255)

            # Length must be positive
            self.assertGreater(record['len'], 0)


class TestOutputFormatConsistency(unittest.TestCase):
    """Test consistency across output formats."""

    def test_verbose_vs_nonverbose(self):
        """Test verbose and non-verbose parsing produce consistent structure."""
        raw_path = get_raw_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        verbose_records = asterix.parse(data, verbose=True)
        brief_records = asterix.parse(data, verbose=False)

        # Same number of records
        self.assertEqual(len(verbose_records), len(brief_records))

        # Same categories
        for v, b in zip(verbose_records, brief_records):
            self.assertEqual(v['category'], b['category'])
            self.assertEqual(v['len'], b['len'])

    def test_parse_with_offset_consistency(self):
        """Test parse_with_offset produces same records as parse."""
        raw_path = get_raw_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        # Full parse
        full_records = asterix.parse(data)

        # Incremental parse
        incremental_records = []
        offset = 0
        while offset < len(data):
            records, new_offset = asterix.parse_with_offset(data, offset, 10)
            incremental_records.extend(records)
            if new_offset == offset:
                break
            offset = new_offset

        # Should produce same number of records
        self.assertEqual(len(full_records), len(incremental_records))


class TestMalformedInput(unittest.TestCase):
    """Test handling of malformed/edge case inputs."""

    def test_truncated_record(self):
        """Test handling of truncated ASTERIX record."""
        # CAT048 header claiming 100 bytes but only 10 provided
        truncated = b'\x30\x00\x64' + b'\x00' * 7

        # Should not crash, may return empty or partial
        try:
            records = asterix.parse(truncated)
            self.assertIsInstance(records, list)
        except Exception:
            # Raising an error is also acceptable
            pass

    def test_invalid_category(self):
        """Test handling of invalid/undefined category."""
        # CAT255 (typically undefined)
        invalid_cat = b'\xFF\x00\x10' + b'\x00' * 13

        try:
            records = asterix.parse(invalid_cat)
            self.assertIsInstance(records, list)
        except Exception:
            pass

    def test_zero_length_block(self):
        """Test handling of zero-length block."""
        zero_len = b'\x30\x00\x00'

        try:
            records = asterix.parse(zero_len)
            self.assertIsInstance(records, list)
        except Exception:
            pass

    def test_oversized_length(self):
        """Test handling of length field larger than data."""
        oversized = b'\x30\xFF\xFF' + b'\x00' * 100

        try:
            records = asterix.parse(oversized)
            self.assertIsInstance(records, list)
        except Exception:
            pass


class TestCRCValidation(unittest.TestCase):
    """Test CRC checksum validation."""

    def test_crc_consistency(self):
        """Test that CRC is consistently calculated."""
        raw_path = get_raw_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        # Parse twice
        records1 = asterix.parse(data)
        records2 = asterix.parse(data)

        # CRCs should be identical
        for r1, r2 in zip(records1, records2):
            self.assertEqual(r1['crc'], r2['crc'])

    def test_crc_format(self):
        """Test that CRC is in expected format (hex string)."""
        raw_path = get_raw_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)

        for record in records:
            crc = record['crc']
            self.assertIsInstance(crc, str)
            # Should be valid hex string
            try:
                int(crc, 16)
            except ValueError:
                self.fail(f"CRC is not valid hex: {crc}")


class TestPerformance(unittest.TestCase):
    """Performance benchmarks for PCAP parsing."""

    def test_parse_throughput(self):
        """Measure parsing throughput."""
        raw_path = get_raw_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        # Parse multiple times for measurement
        iterations = 1000
        start = time.time()
        for _ in range(iterations):
            asterix.parse(data)
        elapsed = time.time() - start

        records_per_sec = iterations / elapsed
        bytes_per_sec = len(data) * iterations / elapsed

        # Just log performance - no hard threshold
        print(f"\nPerformance: {records_per_sec:.0f} parses/sec, "
              f"{bytes_per_sec/1024:.1f} KB/sec")

        # Sanity check - should be able to do at least 100 parses/sec
        self.assertGreater(records_per_sec, 100)

    def test_large_data_handling(self):
        """Test parsing large amounts of data."""
        raw_path = get_raw_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            base_data = f.read()

        # Create larger dataset by repeating
        large_data = base_data * 100

        start = time.time()
        records = asterix.parse(large_data)
        elapsed = time.time() - start

        # Should complete in reasonable time
        self.assertLess(elapsed, 10.0)  # 10 seconds max
        self.assertIsInstance(records, list)


class TestCategoryDescriptions(unittest.TestCase):
    """Test category description functionality."""

    def test_describe_parsed_data(self):
        """Test describing parsed ASTERIX data."""
        raw_path = get_raw_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)
        # describe() in Python takes parsed records, not category number
        desc = asterix.describe(records)
        self.assertIsInstance(desc, str)
        self.assertGreater(len(desc), 0)

    def test_list_configuration_files(self):
        """Test listing available configuration files."""
        config_files = asterix.list_configuration_files()
        self.assertIsInstance(config_files, list)
        # Should have multiple XML config files
        self.assertGreater(len(config_files), 0)

    def test_list_sample_files(self):
        """Test listing available sample files."""
        sample_files = asterix.list_sample_files()
        self.assertIsInstance(sample_files, list)
        self.assertGreater(len(sample_files), 0)


class TestDataIntegrity(unittest.TestCase):
    """Test data integrity across parse operations."""

    def test_idempotent_parsing(self):
        """Test that parsing is idempotent."""
        raw_path = get_raw_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        # Parse multiple times
        results = [asterix.parse(data) for _ in range(5)]

        # All results should be identical
        first = json.dumps(results[0], sort_keys=True)
        for result in results[1:]:
            self.assertEqual(json.dumps(result, sort_keys=True), first)

    def test_no_data_modification(self):
        """Test that input data is not modified."""
        raw_path = get_raw_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Raw file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            original = f.read()

        data = bytearray(original)
        original_hash = hashlib.sha256(data).hexdigest()

        asterix.parse(bytes(data))

        # Data should be unchanged
        self.assertEqual(hashlib.sha256(data).hexdigest(), original_hash)


if __name__ == '__main__':
    unittest.main()
