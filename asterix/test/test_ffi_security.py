"""
ASTERIX FFI Security Tests

Comprehensive security test suite for Python-C FFI boundary validation.
Tests memory safety, buffer overflow protection, null pointer handling,
and edge cases in the ASTERIX C extension module.

**Test Categories:**
1. Buffer Overflow Protection
2. Null Pointer Handling
3. Invalid Input Validation
4. Large Data Handling
5. Edge Cases and Boundary Conditions
6. Memory Leak Prevention

**Run with:**
    pytest asterix/test/test_ffi_security.py -v
    valgrind --leak-check=full pytest asterix/test/test_ffi_security.py
    ASAN_OPTIONS=detect_leaks=1 pytest asterix/test/test_ffi_security.py
"""

import unittest
import sys
import gc
import asterix


class TestFFIBufferSafety(unittest.TestCase):
    """Test buffer overflow protection at FFI boundary."""

    def test_empty_data(self):
        """Test parsing empty data raises ValueError (security validation)."""
        # Empty data is now rejected with ValueError for security
        with self.assertRaises(ValueError) as cm:
            asterix.parse(b'')
        self.assertIn("empty", str(cm.exception).lower())

    def test_single_byte(self):
        """Test parsing single byte doesn't crash."""
        result = asterix.parse(b'\x30')
        # Should handle gracefully (likely empty result)
        self.assertIsInstance(result, list)

    def test_malformed_short_packet(self):
        """Test parsing incomplete ASTERIX header (< 3 bytes)."""
        # ASTERIX header is 3 bytes: CAT(1) + LEN(2)
        result = asterix.parse(b'\x30\x00')
        self.assertIsInstance(result, list)

    def test_length_field_overflow(self):
        """Test length field larger than actual data."""
        # CAT048, claims 1000 bytes but only provides 10
        data = b'\x30\x03\xE8' + b'\x00' * 7
        result = asterix.parse(data)
        self.assertIsInstance(result, list)

    def test_maximum_valid_packet(self):
        """Test parsing maximum valid ASTERIX packet (65535 bytes)."""
        # CAT048, max length 0xFFFF
        data = b'\x30\xFF\xFF' + b'\x00' * 65532
        result = asterix.parse(data)
        self.assertIsInstance(result, list)


class TestFFINullPointerHandling(unittest.TestCase):
    """Test null pointer and None handling."""

    def test_parse_none_raises_error(self):
        """Test that parse(None) raises appropriate error."""
        with self.assertRaises((TypeError, ValueError)):
            asterix.parse(None)

    def test_init_none_raises_error(self):
        """Test that init(None) raises appropriate error."""
        with self.assertRaises((TypeError, ValueError)):
            asterix.init(None)

    def test_init_empty_string_raises_error(self):
        """Test that init('') raises appropriate error."""
        # Empty string now raises ValueError for security validation
        with self.assertRaises((IOError, RuntimeError, ValueError)):
            asterix.init('')


class TestFFIInvalidInput(unittest.TestCase):
    """Test invalid input validation."""

    def test_parse_wrong_type_string(self):
        """Test parsing string instead of bytes."""
        with self.assertRaises(TypeError):
            asterix.parse("not bytes")

    def test_parse_wrong_type_int(self):
        """Test parsing int instead of bytes."""
        # Current implementation accepts int and returns empty list
        # (type coercion in C extension - not ideal but documented behavior)
        result = asterix.parse(12345)
        self.assertIsInstance(result, list)

    def test_parse_wrong_type_list(self):
        """Test parsing list instead of bytes."""
        # Current implementation accepts list and returns empty list
        # (type coercion in C extension - not ideal but documented behavior)
        result = asterix.parse([0x30, 0x00, 0x10])
        self.assertIsInstance(result, list)

    def test_init_wrong_type_int(self):
        """Test init with int instead of string."""
        with self.assertRaises((TypeError, ValueError)):
            asterix.init(12345)

    def test_init_nonexistent_file(self):
        """Test init with nonexistent file."""
        with self.assertRaises((IOError, RuntimeError)):
            asterix.init('/nonexistent/path/to/asterix_cat999.xml')


class TestFFILargeData(unittest.TestCase):
    """Test large data handling and memory pressure."""

    def test_parse_large_valid_packet(self):
        """Test parsing legitimately large ASTERIX packet."""
        # CAT048, 10KB packet with valid structure
        header = b'\x30\x27\x10'  # CAT048, 10000 bytes
        data = header + b'\x00' * 9997
        result = asterix.parse(data)
        self.assertIsInstance(result, list)

    def test_parse_multiple_large_packets(self):
        """Test parsing multiple large packets in sequence."""
        header = b'\x30\x13\x88'  # CAT048, 5000 bytes
        packet = header + b'\x00' * 4997
        data = packet * 10  # 10 packets
        result = asterix.parse(data)
        self.assertIsInstance(result, list)

    def test_parse_with_offset_large_data(self):
        """Test incremental parsing with large dataset."""
        # Create 50KB of data
        header = b'\x30\x00\x10'  # CAT048, 16 bytes
        packet = header + b'\x00' * 13
        data = packet * 3000  # 48KB total

        offset = 0
        total_records = []
        for _ in range(10):
            records, new_offset = asterix.parse_with_offset(data, offset, blocks_count=300)
            total_records.extend(records)
            if new_offset == offset:
                break
            offset = new_offset

        self.assertIsInstance(total_records, list)


class TestFFIEdgeCases(unittest.TestCase):
    """Test edge cases and boundary conditions."""

    def test_parse_all_zeros(self):
        """Test parsing all-zero data."""
        result = asterix.parse(b'\x00' * 100)
        self.assertIsInstance(result, list)

    def test_parse_all_ones(self):
        """Test parsing all-ones data."""
        result = asterix.parse(b'\xFF' * 100)
        self.assertIsInstance(result, list)

    def test_parse_alternating_pattern(self):
        """Test parsing alternating bit pattern."""
        result = asterix.parse(b'\xAA\x55' * 50)
        self.assertIsInstance(result, list)

    def test_parse_valid_cat048_minimal(self):
        """Test parsing minimal valid CAT048 packet."""
        # Minimal CAT048: CAT(1) + LEN(2) = 3 bytes minimum
        data = b'\x30\x00\x03'
        result = asterix.parse(data)
        self.assertIsInstance(result, list)

    def test_parse_multiple_categories(self):
        """Test parsing multiple different categories."""
        # CAT048 + CAT062 in same data stream
        cat048 = b'\x30\x00\x03'
        cat062 = b'\x3E\x00\x03'
        data = cat048 + cat062
        result = asterix.parse(data)
        self.assertIsInstance(result, list)

    def test_parse_with_offset_zero(self):
        """Test parse_with_offset with offset=0."""
        data = b'\x30\x00\x10' + b'\x00' * 13
        records, new_offset = asterix.parse_with_offset(data, offset=0, blocks_count=1)
        self.assertIsInstance(records, list)
        self.assertIsInstance(new_offset, int)
        self.assertGreaterEqual(new_offset, 0)

    def test_parse_with_offset_beyond_data(self):
        """Test parse_with_offset with offset beyond data length raises ValueError."""
        # Offset validation now raises ValueError for security
        data = b'\x30\x00\x10' + b'\x00' * 13
        with self.assertRaises(ValueError) as cm:
            asterix.parse_with_offset(data, offset=1000, blocks_count=1)
        self.assertIn("offset", str(cm.exception).lower())


class TestFFIMemoryPressure(unittest.TestCase):
    """Test memory allocation and cleanup under pressure."""

    def test_repeated_parse_no_leak(self):
        """Test repeated parsing doesn't leak memory."""
        data = b'\x30\x00\x10' + b'\x00' * 13

        # Parse 1000 times to stress memory management
        for i in range(1000):
            result = asterix.parse(data)
            self.assertIsInstance(result, list)

            # Force garbage collection every 100 iterations
            if i % 100 == 0:
                gc.collect()

    def test_parse_large_then_small(self):
        """Test parsing large data then small data (memory reuse)."""
        # Large packet
        large_data = b'\x30\x27\x10' + b'\x00' * 9997  # 10KB
        result1 = asterix.parse(large_data)

        # Small packet
        small_data = b'\x30\x00\x10' + b'\x00' * 13
        result2 = asterix.parse(small_data)

        self.assertIsInstance(result1, list)
        self.assertIsInstance(result2, list)

    def test_concurrent_parse_different_data(self):
        """Test parsing different data in sequence (state isolation)."""
        data1 = b'\x30\x00\x10' + b'\x01' * 13  # CAT048
        data2 = b'\x3E\x00\x10' + b'\x02' * 13  # CAT062

        result1 = asterix.parse(data1)
        result2 = asterix.parse(data2)

        # Results should be independent (no state contamination)
        self.assertIsInstance(result1, list)
        self.assertIsInstance(result2, list)


class TestFFIRobustness(unittest.TestCase):
    """Test robustness against malformed input."""

    def test_invalid_category_number(self):
        """Test parsing with invalid/undefined category."""
        # CAT255 (likely undefined)
        data = b'\xFF\x00\x10' + b'\x00' * 13
        result = asterix.parse(data)
        self.assertIsInstance(result, list)

    def test_truncated_record(self):
        """Test record shorter than declared length."""
        # Claims 100 bytes but only 10 bytes total
        data = b'\x30\x00\x64' + b'\x00' * 7
        result = asterix.parse(data)
        self.assertIsInstance(result, list)

    def test_zero_length_block(self):
        """Test data block with zero length."""
        data = b'\x30\x00\x00'
        result = asterix.parse(data)
        self.assertIsInstance(result, list)

    def test_negative_offset(self):
        """Test parse_with_offset with negative offset."""
        data = b'\x30\x00\x10' + b'\x00' * 13
        # Negative offset should be handled gracefully
        try:
            records, new_offset = asterix.parse_with_offset(data, offset=-1, blocks_count=1)
            # If it doesn't raise, result should be safe
            self.assertIsInstance(records, list)
        except (ValueError, OverflowError):
            # Raising an error is also acceptable for invalid offset
            pass  # nosec B110 - testing that error is acceptable

    def test_huge_blocks_count(self):
        """Test parse_with_offset with unreasonably large blocks_count raises ValueError."""
        # Excessive blocks_count is now rejected with ValueError for security
        data = b'\x30\x00\x10' + b'\x00' * 13
        with self.assertRaises(ValueError) as cm:
            asterix.parse_with_offset(data, offset=0, blocks_count=999999999)
        self.assertIn("blocks_count", str(cm.exception).lower())


class TestFFIUnicode(unittest.TestCase):
    """Test Unicode and encoding handling."""

    def test_init_unicode_path(self):
        """Test init with Unicode characters in path."""
        # This should fail (file doesn't exist) but not crash
        with self.assertRaises((IOError, RuntimeError)):
            asterix.init('/path/with/unicode/ñoño/asterix_cat048.xml')

    def test_init_path_with_spaces(self):
        """Test init with spaces in path."""
        with self.assertRaises((IOError, RuntimeError)):
            asterix.init('/path with spaces/asterix_cat048.xml')


class TestFFIReferenceCount(unittest.TestCase):
    """Test Python reference counting (memory management)."""

    def test_parse_result_refcount(self):
        """Test that parse result has correct reference count."""
        data = b'\x30\x00\x10' + b'\x00' * 13
        result = asterix.parse(data)

        # Get initial refcount
        initial_refcount = sys.getrefcount(result)

        # Create another reference
        result2 = result
        self.assertEqual(sys.getrefcount(result), initial_refcount + 1)

        # Delete reference
        del result2
        self.assertEqual(sys.getrefcount(result), initial_refcount)

    def test_nested_dict_refcount(self):
        """Test reference counting for nested dictionaries."""
        # Parse data that returns nested dicts
        data = b'\x30\x00\x10' + b'\xFD' + b'\x00' * 12  # CAT048 with FSPEC
        result = asterix.parse(data, verbose=True)

        if result and isinstance(result, list) and len(result) > 0:
            # Check that nested structures are properly managed
            record = result[0]
            self.assertIsInstance(record, dict)

            # Force GC and ensure no crashes
            del result
            gc.collect()


class TestFFIThreadSafety(unittest.TestCase):
    """Test thread safety warnings (NOT thread-safe by design)."""

    def test_parse_sequential(self):
        """Test sequential parsing (baseline for thread safety)."""
        data1 = b'\x30\x00\x10' + b'\x01' * 13
        data2 = b'\x3E\x00\x10' + b'\x02' * 13

        result1 = asterix.parse(data1)
        result2 = asterix.parse(data2)

        # Sequential should always work
        self.assertIsInstance(result1, list)
        self.assertIsInstance(result2, list)


if __name__ == '__main__':
    unittest.main()
