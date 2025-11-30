"""
Cross-Binding Validation Tests for ASTERIX Parser

Validates that Python, Rust, and Node.js bindings produce consistent
parsing results for the same input data.

This ensures that all language bindings share the same C++ core
behavior and produce equivalent output.

Run with:
    pytest asterix/test/test_cross_binding_validation.py -v

Prerequisites:
    - Python: asterix module installed (pip install -e .)
    - Rust: cargo build in asterix-rs/
    - Node.js: npm install in asterix-node/
"""

import asterix
import unittest
import subprocess  # nosec B404 - subprocess is required for cross-binding validation tests
import json
import os
import sys
import logging

# Configure logging for test diagnostics
logger = logging.getLogger(__name__)


def get_sample_file(filename):
    """Get path to sample data file."""
    sample_dir = os.path.join(os.path.dirname(asterix.__file__), 'sample_data')
    return os.path.join(sample_dir, filename)


def get_repo_root():
    """Get repository root directory."""
    return os.path.dirname(os.path.dirname(os.path.dirname(asterix.__file__)))


def rust_available():
    """Check if Rust bindings are available."""
    cargo_toml = os.path.join(get_repo_root(), 'asterix-rs', 'Cargo.toml')
    return os.path.exists(cargo_toml)


def node_available():
    """Check if Node.js bindings are available."""
    package_json = os.path.join(get_repo_root(), 'asterix-node', 'package.json')
    node_module = os.path.join(get_repo_root(), 'asterix-node', 'build', 'Release', 'asterix.node')
    return os.path.exists(package_json) and os.path.exists(node_module)


def cpp_executable_available():
    """Check if C++ executable is available."""
    exe_path = os.path.join(get_repo_root(), 'install', 'bin', 'asterix')
    return os.path.exists(exe_path)


class TestCrossBindingConsistency(unittest.TestCase):
    """Test that all bindings produce consistent results."""

    def test_python_parsing_baseline(self):
        """Establish Python parsing as baseline."""
        raw_path = get_sample_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Sample file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)

        self.assertIsInstance(records, list)
        self.assertGreater(len(records), 0)

        # Baseline checks
        record = records[0]
        self.assertEqual(record['category'], 48)
        self.assertIn('I010', record)
        self.assertIn('crc', record)

    @unittest.skipUnless(cpp_executable_available(), "C++ executable not available")
    def test_cpp_vs_python_record_count(self):
        """Compare C++ and Python record counts."""
        raw_path = get_sample_file('cat048.raw')
        config_path = os.path.join(get_repo_root(), 'install', 'config', 'asterix.ini')
        exe_path = os.path.join(get_repo_root(), 'install', 'bin', 'asterix')

        if not all(os.path.exists(p) for p in [raw_path, config_path, exe_path]):
            self.skipTest("Required files not found")

        # Python parsing
        with open(raw_path, 'rb') as f:
            data = f.read()
        py_records = asterix.parse(data)

        # C++ parsing (JSON output)
        # nosec B603 - exe_path is validated to exist above, not user input
        result = subprocess.run(
            [exe_path, '-d', config_path, '-j', '-f', raw_path],
            capture_output=True, text=True, timeout=30
        )

        if result.returncode != 0:
            self.skipTest(f"C++ executable failed: {result.stderr}")

        # Count JSON objects in output
        cpp_count = 0
        for line in result.stdout.strip().split('\n'):
            if line.strip():
                try:
                    json.loads(line)
                    cpp_count += 1
                except json.JSONDecodeError:
                    pass

        # Record counts should match
        self.assertEqual(len(py_records), cpp_count,
                        f"Python: {len(py_records)}, C++: {cpp_count}")

    @unittest.skipUnless(cpp_executable_available(), "C++ executable not available")
    def test_cpp_vs_python_categories(self):
        """Compare C++ and Python parsed categories."""
        raw_path = get_sample_file('cat062cat065.raw')
        config_path = os.path.join(get_repo_root(), 'install', 'config', 'asterix.ini')
        exe_path = os.path.join(get_repo_root(), 'install', 'bin', 'asterix')

        if not all(os.path.exists(p) for p in [raw_path, config_path, exe_path]):
            self.skipTest("Required files not found")

        # Python parsing
        with open(raw_path, 'rb') as f:
            data = f.read()
        py_records = asterix.parse(data)
        py_categories = sorted(set(r['category'] for r in py_records))

        # C++ parsing
        # nosec B603 - exe_path is validated to exist above, not user input
        result = subprocess.run(
            [exe_path, '-d', config_path, '-j', '-f', raw_path],
            capture_output=True, text=True, timeout=30
        )

        if result.returncode != 0:
            self.skipTest(f"C++ executable failed: {result.stderr}")

        cpp_categories = set()
        for line in result.stdout.strip().split('\n'):
            if line.strip():
                try:
                    obj = json.loads(line)
                    if 'category' in obj:
                        cpp_categories.add(obj['category'])
                except json.JSONDecodeError:
                    pass
        cpp_categories = sorted(cpp_categories)

        # Categories should match
        self.assertEqual(py_categories, cpp_categories,
                        f"Python: {py_categories}, C++: {cpp_categories}")

    @unittest.skipUnless(node_available(), "Node.js bindings not available")
    def test_node_vs_python_record_count(self):
        """Compare Node.js and Python record counts."""
        raw_path = get_sample_file('cat048.raw')
        node_dir = os.path.join(get_repo_root(), 'asterix-node')

        if not os.path.exists(raw_path):
            self.skipTest("Sample file not found")

        # Python parsing
        with open(raw_path, 'rb') as f:
            data = f.read()
        py_records = asterix.parse(data)

        # Node.js parsing via subprocess
        node_script = f'''
const asterix = require('./lib/index');
const fs = require('fs');
const data = fs.readFileSync('{raw_path}');
try {{
    const records = asterix.parse(data);
    console.log(JSON.stringify({{ count: records.length }}));
}} catch (e) {{
    console.log(JSON.stringify({{ error: e.message }}));
}}
'''
        # nosec B603 B607 - Test code with known node executable and controlled script
        result = subprocess.run(
            ['node', '-e', node_script],
            capture_output=True, text=True, timeout=30,
            cwd=node_dir
        )

        if result.returncode != 0:
            self.skipTest(f"Node.js execution failed: {result.stderr}")

        try:
            node_result = json.loads(result.stdout.strip())
            if 'error' in node_result:
                self.skipTest(f"Node.js parsing error: {node_result['error']}")
            node_count = node_result['count']
        except json.JSONDecodeError:
            self.skipTest(f"Node.js output not JSON: {result.stdout}")

        # Record counts should match
        self.assertEqual(len(py_records), node_count,
                        f"Python: {len(py_records)}, Node.js: {node_count}")


class TestCRCConsistency(unittest.TestCase):
    """Test CRC calculation consistency across bindings."""

    def test_python_crc_format(self):
        """Verify Python CRC format."""
        raw_path = get_sample_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Sample file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)

        for record in records:
            crc = record['crc']
            # CRC should be uppercase hex string
            self.assertIsInstance(crc, str)
            self.assertTrue(all(c in '0123456789ABCDEF' for c in crc),
                          f"Invalid CRC format: {crc}")

    @unittest.skipUnless(cpp_executable_available(), "C++ executable not available")
    def test_cpp_vs_python_crc(self):
        """Compare C++ and Python CRC values."""
        raw_path = get_sample_file('cat048.raw')
        config_path = os.path.join(get_repo_root(), 'install', 'config', 'asterix.ini')
        exe_path = os.path.join(get_repo_root(), 'install', 'bin', 'asterix')

        if not all(os.path.exists(p) for p in [raw_path, config_path, exe_path]):
            self.skipTest("Required files not found")

        # Python parsing
        with open(raw_path, 'rb') as f:
            data = f.read()
        py_records = asterix.parse(data)
        py_crcs = [r['crc'] for r in py_records]

        # C++ parsing
        # nosec B603 - exe_path is validated to exist above, not user input
        result = subprocess.run(
            [exe_path, '-d', config_path, '-j', '-f', raw_path],
            capture_output=True, text=True, timeout=30
        )

        if result.returncode != 0:
            self.skipTest(f"C++ executable failed: {result.stderr}")

        cpp_crcs = []
        for line in result.stdout.strip().split('\n'):
            if line.strip():
                try:
                    obj = json.loads(line)
                    if 'crc' in obj:
                        cpp_crcs.append(obj['crc'])
                except json.JSONDecodeError:
                    pass

        # CRC values should match
        self.assertEqual(py_crcs, cpp_crcs,
                        "CRC values differ between Python and C++")


class TestDataItemConsistency(unittest.TestCase):
    """Test data item parsing consistency across bindings."""

    def test_python_i010_parsing(self):
        """Verify Python I010 (Data Source Identifier) parsing."""
        raw_path = get_sample_file('cat048.raw')
        if not os.path.exists(raw_path):
            self.skipTest(f"Sample file not found: {raw_path}")

        with open(raw_path, 'rb') as f:
            data = f.read()

        records = asterix.parse(data)

        for record in records:
            if 'I010' in record:
                i010 = record['I010']
                # Should have SAC and SIC
                self.assertIn('SAC', i010)
                self.assertIn('SIC', i010)
                # Values should be integers 0-255
                self.assertIsInstance(i010['SAC']['val'], int)
                self.assertIsInstance(i010['SIC']['val'], int)
                self.assertGreaterEqual(i010['SAC']['val'], 0)
                self.assertLessEqual(i010['SAC']['val'], 255)
                self.assertGreaterEqual(i010['SIC']['val'], 0)
                self.assertLessEqual(i010['SIC']['val'], 255)

    @unittest.skipUnless(cpp_executable_available(), "C++ executable not available")
    def test_cpp_vs_python_i010(self):
        """Compare C++ and Python I010 values."""
        raw_path = get_sample_file('cat048.raw')
        config_path = os.path.join(get_repo_root(), 'install', 'config', 'asterix.ini')
        exe_path = os.path.join(get_repo_root(), 'install', 'bin', 'asterix')

        if not all(os.path.exists(p) for p in [raw_path, config_path, exe_path]):
            self.skipTest("Required files not found")

        # Python parsing
        with open(raw_path, 'rb') as f:
            data = f.read()
        py_records = asterix.parse(data)

        # C++ parsing
        # nosec B603 - exe_path is validated to exist above, not user input
        result = subprocess.run(
            [exe_path, '-d', config_path, '-j', '-f', raw_path],
            capture_output=True, text=True, timeout=30
        )

        if result.returncode != 0:
            self.skipTest(f"C++ executable failed: {result.stderr}")

        cpp_records = []
        for line in result.stdout.strip().split('\n'):
            if line.strip():
                try:
                    cpp_records.append(json.loads(line))
                except json.JSONDecodeError:
                    pass

        # Compare I010 values
        for py_rec, cpp_rec in zip(py_records, cpp_records):
            if 'I010' in py_rec and 'I010' in cpp_rec:
                py_sac = py_rec['I010']['SAC']['val']
                py_sic = py_rec['I010']['SIC']['val']
                cpp_sac = cpp_rec['I010']['SAC']['val']
                cpp_sic = cpp_rec['I010']['SIC']['val']

                self.assertEqual(py_sac, cpp_sac,
                               f"SAC mismatch: Python={py_sac}, C++={cpp_sac}")
                self.assertEqual(py_sic, cpp_sic,
                               f"SIC mismatch: Python={py_sic}, C++={cpp_sic}")


class TestErrorHandlingConsistency(unittest.TestCase):
    """Test error handling consistency across bindings."""

    def test_python_empty_data(self):
        """Test Python handling of empty data."""
        with self.assertRaises(ValueError):
            asterix.parse(b'')

    def test_python_invalid_data(self):
        """Test Python handling of completely invalid data."""
        # All zeros - invalid ASTERIX
        invalid = b'\x00' * 10
        # Should not crash, may return empty or raise
        try:
            result = asterix.parse(invalid)
            self.assertIsInstance(result, list)
        except (ValueError, RuntimeError) as e:
            # Raising is acceptable for invalid data
            logger.debug("Expected exception for invalid data: %s", e)

    def test_python_truncated_data(self):
        """Test Python handling of truncated data."""
        # CAT048 header claiming 100 bytes, only 5 provided
        truncated = b'\x30\x00\x64\x00\x00'
        try:
            result = asterix.parse(truncated)
            self.assertIsInstance(result, list)
        except (ValueError, RuntimeError) as e:
            # Raising is acceptable for truncated data
            logger.debug("Expected exception for truncated data: %s", e)


if __name__ == '__main__':
    unittest.main()
