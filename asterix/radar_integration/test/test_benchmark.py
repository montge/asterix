#!/usr/bin/env python3
"""
Unit tests for ASTERIX Radar Integration Benchmark Suite

Tests cover:
- BenchmarkResult dataclass creation and serialization
- BenchmarkSuite initialization and configuration
- Benchmark execution (with mocked functions)
- Result collection and reporting
- JSON export functionality
- Command-line argument parsing

License: GPL-3.0
"""

import unittest
import json
import tempfile
import os
from unittest.mock import patch, MagicMock

from asterix.radar_integration.benchmark import (
    BenchmarkResult,
    BenchmarkSuite,
    main
)


class TestBenchmarkResult(unittest.TestCase):
    """Test BenchmarkResult dataclass."""

    def test_benchmark_result_creation(self):
        """Test creating a BenchmarkResult."""
        result = BenchmarkResult(
            name="Test Benchmark",
            category="Test",
            iterations=100,
            total_time=1.5,
            avg_time=0.015,
            throughput=66.67,
            unit="ops/s"
        )

        self.assertEqual(result.name, "Test Benchmark")
        self.assertEqual(result.category, "Test")
        self.assertEqual(result.iterations, 100)
        self.assertEqual(result.total_time, 1.5)
        self.assertEqual(result.avg_time, 0.015)
        self.assertAlmostEqual(result.throughput, 66.67, places=2)
        self.assertEqual(result.unit, "ops/s")
        self.assertIsNone(result.details)

    def test_benchmark_result_with_details(self):
        """Test creating a BenchmarkResult with details."""
        details = {"platform": "linux", "python_version": "3.10"}
        result = BenchmarkResult(
            name="Test",
            category="Test",
            iterations=100,
            total_time=1.0,
            avg_time=0.01,
            throughput=100.0,
            unit="ops",
            details=details
        )

        self.assertEqual(result.details, details)

    def test_to_dict_without_details(self):
        """Test to_dict() method without details."""
        result = BenchmarkResult(
            name="Test",
            category="Test",
            iterations=100,
            total_time=1.0,
            avg_time=0.01,
            throughput=100.0,
            unit="ops"
        )

        result_dict = result.to_dict()

        self.assertEqual(result_dict['name'], "Test")
        self.assertEqual(result_dict['category'], "Test")
        self.assertEqual(result_dict['iterations'], 100)
        self.assertEqual(result_dict['total_time'], 1.0)
        self.assertEqual(result_dict['avg_time'], 0.01)
        self.assertEqual(result_dict['throughput'], 100.0)
        self.assertEqual(result_dict['unit'], "ops")
        self.assertEqual(result_dict['details'], {})  # None converted to {}

    def test_to_dict_with_details(self):
        """Test to_dict() method with details."""
        details = {"key": "value"}
        result = BenchmarkResult(
            name="Test",
            category="Test",
            iterations=100,
            total_time=1.0,
            avg_time=0.01,
            throughput=100.0,
            unit="ops",
            details=details
        )

        result_dict = result.to_dict()
        self.assertEqual(result_dict['details'], details)


class TestBenchmarkSuite(unittest.TestCase):
    """Test BenchmarkSuite class."""

    def setUp(self):
        """Set up test fixtures."""
        # Use quick mode to speed up tests
        self.suite = BenchmarkSuite(quick=True)

    def test_initialization_quick_mode(self):
        """Test BenchmarkSuite initialization in quick mode."""
        suite = BenchmarkSuite(quick=True)
        self.assertTrue(suite.quick)
        self.assertEqual(len(suite.results), 0)
        self.assertIsNotNone(suite.radar)

    def test_initialization_normal_mode(self):
        """Test BenchmarkSuite initialization in normal mode."""
        suite = BenchmarkSuite(quick=False)
        self.assertFalse(suite.quick)
        self.assertEqual(len(suite.results), 0)
        self.assertIsNotNone(suite.radar)

    def test_run_benchmark_basic(self):
        """Test running a basic benchmark."""
        call_count = 0

        def test_func():
            nonlocal call_count
            call_count += 1

        # Use very small iterations to speed up test
        result = self.suite.run_benchmark(
            name="Test Function",
            category="Test",
            func=test_func,
            iterations=10,
            unit="calls/s"
        )

        # Verify warmup (1 iteration = 10 // 10) + benchmark (10 iterations) = 11
        self.assertEqual(call_count, 11)
        self.assertEqual(result.name, "Test Function")
        self.assertEqual(result.category, "Test")
        self.assertEqual(result.iterations, 10)
        self.assertGreater(result.total_time, 0)
        self.assertGreater(result.avg_time, 0)
        self.assertGreater(result.throughput, 0)
        self.assertEqual(result.unit, "calls/s")

        # Result should be stored
        self.assertEqual(len(self.suite.results), 1)
        self.assertEqual(self.suite.results[0], result)

    def test_run_benchmark_auto_iterations_quick(self):
        """Test run_benchmark with auto iterations in quick mode."""
        suite = BenchmarkSuite(quick=True)

        def fast_func():
            pass

        result = suite.run_benchmark(
            name="Fast Test",
            category="Test",
            func=fast_func,
            iterations=None  # Auto
        )

        # Quick mode should use 100 iterations
        self.assertEqual(result.iterations, 100)

    def test_run_benchmark_auto_iterations_normal(self):
        """Test run_benchmark with auto iterations in normal mode."""
        suite = BenchmarkSuite(quick=False)

        def fast_func():
            pass

        result = suite.run_benchmark(
            name="Normal Test",
            category="Test",
            func=fast_func,
            iterations=None  # Auto
        )

        # Normal mode should use 1000 iterations
        self.assertEqual(result.iterations, 1000)

    def test_run_benchmark_zero_time_edge_case(self):
        """Test benchmark with near-zero elapsed time."""
        # Mock time.perf_counter to return same value (zero elapsed time)
        with patch('asterix.radar_integration.benchmark.time.perf_counter') as mock_time:
            mock_time.side_effect = [0.0, 0.0, 0.0, 0.0]  # start, end (warmup and benchmark)

            def instant_func():
                pass

            result = self.suite.run_benchmark(
                name="Instant",
                category="Test",
                func=instant_func,
                iterations=10
            )

            # Throughput should be 0 when elapsed time is 0
            self.assertEqual(result.throughput, 0)

    def test_run_benchmark_stores_multiple_results(self):
        """Test that multiple benchmarks accumulate results."""
        def func1():
            pass

        def func2():
            pass

        self.suite.run_benchmark("Test 1", "Cat1", func1, iterations=5)
        self.suite.run_benchmark("Test 2", "Cat2", func2, iterations=5)

        self.assertEqual(len(self.suite.results), 2)
        self.assertEqual(self.suite.results[0].name, "Test 1")
        self.assertEqual(self.suite.results[1].name, "Test 2")

    @patch('builtins.print')
    def test_benchmark_mock_radar_generation(self, mock_print):
        """Test benchmark_mock_radar_generation method."""
        # This will actually run but in quick mode with very fast execution
        suite = BenchmarkSuite(quick=True)
        suite.benchmark_mock_radar_generation()

        # Should have 4 results: 10 plots, 100 plots, 1000 plots, track
        self.assertEqual(len(suite.results), 4)
        self.assertEqual(suite.results[0].name, "Generate 10 plots")
        self.assertEqual(suite.results[1].name, "Generate 100 plots")
        self.assertEqual(suite.results[2].name, "Generate 1000 plots")
        self.assertEqual(suite.results[3].name, "Generate track (60s)")

        # Verify all are in Mock Radar category
        for result in suite.results:
            self.assertEqual(result.category, "Mock Radar")

        # Verify print was called
        self.assertTrue(mock_print.called)

    @patch('builtins.print')
    def test_benchmark_encoders(self, mock_print):
        """Test benchmark_encoders method."""
        suite = BenchmarkSuite(quick=True)
        suite.benchmark_encoders()

        # Should have 7 encoder benchmarks
        self.assertEqual(len(suite.results), 7)

        # Verify encoder names
        encoder_names = [r.name for r in suite.results]
        self.assertIn("CAT001 (10 plots)", encoder_names)
        self.assertIn("CAT020 (10 targets)", encoder_names)
        self.assertIn("CAT021 (10 reports)", encoder_names)
        self.assertIn("CAT034 North Marker", encoder_names)
        self.assertIn("CAT048 (10 plots)", encoder_names)
        self.assertIn("CAT062 (10 tracks)", encoder_names)
        self.assertIn("CAT048 (100 plots)", encoder_names)

        # Verify all are in Encoder category
        for result in suite.results:
            self.assertEqual(result.category, "Encoder")

    @patch('builtins.print')
    def test_benchmark_statistics(self, mock_print):
        """Test benchmark_statistics method."""
        suite = BenchmarkSuite(quick=True)
        suite.benchmark_statistics()

        # Should have 1 result
        self.assertEqual(len(suite.results), 1)
        self.assertEqual(suite.results[0].name, "Scenario stats (100 plots)")
        self.assertEqual(suite.results[0].category, "Statistics")

    @patch('builtins.print')
    def test_benchmark_visualization(self, mock_print):
        """Test benchmark_visualization method."""
        suite = BenchmarkSuite(quick=True)
        suite.benchmark_visualization()

        # Should have 2 results: ASCII (10) and ASCII (100)
        self.assertEqual(len(suite.results), 2)
        self.assertEqual(suite.results[0].name, "ASCII radar (10 plots)")
        self.assertEqual(suite.results[1].name, "ASCII radar (100 plots)")

        # Verify all are in Visualization category
        for result in suite.results:
            self.assertEqual(result.category, "Visualization")

    @patch('builtins.print')
    def test_run_all(self, mock_print):
        """Test run_all method."""
        suite = BenchmarkSuite(quick=True)
        suite.run_all()

        # Should have results from all benchmark categories
        # 4 (mock radar) + 7 (encoders) + 1 (stats) + 2 (viz) = 14 total
        self.assertEqual(len(suite.results), 14)

        # Verify categories are present
        categories = {r.category for r in suite.results}
        self.assertIn("Mock Radar", categories)
        self.assertIn("Encoder", categories)
        self.assertIn("Statistics", categories)
        self.assertIn("Visualization", categories)

    @patch('builtins.print')
    def test_print_summary(self, mock_print):
        """Test print_summary method."""
        # Add some mock results
        self.suite.results = [
            BenchmarkResult("Test 1", "Cat A", 100, 1.0, 0.01, 100.0, "ops"),
            BenchmarkResult("Test 2", "Cat A", 100, 1.0, 0.01, 100.0, "ops"),
            BenchmarkResult("Test 3", "Cat B", 100, 1.0, 0.01, 100.0, "ops"),
        ]

        self.suite.print_summary()

        # Verify print was called with summary information
        self.assertTrue(mock_print.called)
        print_calls = [str(call) for call in mock_print.call_args_list]
        output = ' '.join(print_calls)

        # Check for category headers
        self.assertIn("Cat A", output)
        self.assertIn("Cat B", output)

    def test_save_results(self):
        """Test save_results method."""
        # Add some mock results
        self.suite.results = [
            BenchmarkResult("Test 1", "Cat A", 100, 1.0, 0.01, 100.0, "ops"),
            BenchmarkResult("Test 2", "Cat B", 200, 2.0, 0.01, 100.0, "ops/s"),
        ]

        # Save to temporary file
        with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as f:
            temp_file = f.name

        try:
            self.suite.save_results(temp_file)

            # Verify file was created
            self.assertTrue(os.path.exists(temp_file))

            # Load and verify JSON content
            with open(temp_file, 'r') as f:
                data = json.load(f)

            self.assertIn('timestamp', data)
            self.assertIn('quick_mode', data)
            self.assertIn('total_tests', data)
            self.assertIn('results', data)

            self.assertEqual(data['quick_mode'], True)
            self.assertEqual(data['total_tests'], 2)
            self.assertEqual(len(data['results']), 2)

            # Verify first result
            result1 = data['results'][0]
            self.assertEqual(result1['name'], "Test 1")
            self.assertEqual(result1['category'], "Cat A")
            self.assertEqual(result1['iterations'], 100)
            self.assertEqual(result1['total_time'], 1.0)
            self.assertEqual(result1['avg_time'], 0.01)
            self.assertEqual(result1['throughput'], 100.0)
            self.assertEqual(result1['unit'], "ops")

            # Verify second result
            result2 = data['results'][1]
            self.assertEqual(result2['name'], "Test 2")
            self.assertEqual(result2['category'], "Cat B")

        finally:
            # Cleanup
            if os.path.exists(temp_file):
                os.remove(temp_file)


class TestMain(unittest.TestCase):
    """Test main() function and CLI argument parsing."""

    @patch('asterix.radar_integration.benchmark.BenchmarkSuite')
    @patch('sys.argv', ['benchmark.py', '--quick'])
    def test_main_quick_mode(self, mock_suite_class):
        """Test main() with --quick flag."""
        mock_suite = MagicMock()
        mock_suite_class.return_value = mock_suite

        main()

        # Verify suite was created with quick=True
        mock_suite_class.assert_called_once_with(quick=True)

        # Verify run_all was called
        mock_suite.run_all.assert_called_once()

        # Verify print_summary was called
        mock_suite.print_summary.assert_called_once()

        # Verify save_results was NOT called
        mock_suite.save_results.assert_not_called()

    @patch('asterix.radar_integration.benchmark.BenchmarkSuite')
    @patch('sys.argv', ['benchmark.py'])
    def test_main_normal_mode(self, mock_suite_class):
        """Test main() without --quick flag."""
        mock_suite = MagicMock()
        mock_suite_class.return_value = mock_suite

        main()

        # Verify suite was created with quick=False
        mock_suite_class.assert_called_once_with(quick=False)

        # Verify run_all was called
        mock_suite.run_all.assert_called_once()

    @patch('asterix.radar_integration.benchmark.BenchmarkSuite')
    @patch('sys.argv', ['benchmark.py', '--category', 'mock'])
    def test_main_category_mock(self, mock_suite_class):
        """Test main() with --category mock."""
        mock_suite = MagicMock()
        mock_suite_class.return_value = mock_suite

        main()

        # Verify only benchmark_mock_radar_generation was called
        mock_suite.benchmark_mock_radar_generation.assert_called_once()
        mock_suite.run_all.assert_not_called()

    @patch('asterix.radar_integration.benchmark.BenchmarkSuite')
    @patch('sys.argv', ['benchmark.py', '--category', 'encoder'])
    def test_main_category_encoder(self, mock_suite_class):
        """Test main() with --category encoder."""
        mock_suite = MagicMock()
        mock_suite_class.return_value = mock_suite

        main()

        # Verify only benchmark_encoders was called
        mock_suite.benchmark_encoders.assert_called_once()
        mock_suite.run_all.assert_not_called()

    @patch('asterix.radar_integration.benchmark.BenchmarkSuite')
    @patch('sys.argv', ['benchmark.py', '--category', 'stats'])
    def test_main_category_stats(self, mock_suite_class):
        """Test main() with --category stats."""
        mock_suite = MagicMock()
        mock_suite_class.return_value = mock_suite

        main()

        # Verify only benchmark_statistics was called
        mock_suite.benchmark_statistics.assert_called_once()
        mock_suite.run_all.assert_not_called()

    @patch('asterix.radar_integration.benchmark.BenchmarkSuite')
    @patch('sys.argv', ['benchmark.py', '--category', 'viz'])
    def test_main_category_viz(self, mock_suite_class):
        """Test main() with --category viz."""
        mock_suite = MagicMock()
        mock_suite_class.return_value = mock_suite

        main()

        # Verify only benchmark_visualization was called
        mock_suite.benchmark_visualization.assert_called_once()
        mock_suite.run_all.assert_not_called()

    @patch('asterix.radar_integration.benchmark.BenchmarkSuite')
    @patch('sys.argv', ['benchmark.py', '--category', 'all'])
    def test_main_category_all(self, mock_suite_class):
        """Test main() with --category all."""
        mock_suite = MagicMock()
        mock_suite_class.return_value = mock_suite

        main()

        # Verify run_all was called
        mock_suite.run_all.assert_called_once()

    @patch('asterix.radar_integration.benchmark.BenchmarkSuite')
    @patch('sys.argv', ['benchmark.py', '--save', 'results.json'])
    def test_main_save_results(self, mock_suite_class):
        """Test main() with --save flag."""
        mock_suite = MagicMock()
        mock_suite_class.return_value = mock_suite

        main()

        # Verify save_results was called with correct filename
        mock_suite.save_results.assert_called_once_with('results.json')

    @patch('asterix.radar_integration.benchmark.BenchmarkSuite')
    @patch('sys.argv', ['benchmark.py', '--quick', '--category', 'encoder', '--save', 'output.json'])
    def test_main_combined_flags(self, mock_suite_class):
        """Test main() with multiple flags combined."""
        mock_suite = MagicMock()
        mock_suite_class.return_value = mock_suite

        main()

        # Verify suite was created with quick=True
        mock_suite_class.assert_called_once_with(quick=True)

        # Verify only encoder benchmark was run
        mock_suite.benchmark_encoders.assert_called_once()

        # Verify results were saved
        mock_suite.save_results.assert_called_once_with('output.json')


class TestBenchmarkIntegration(unittest.TestCase):
    """Integration tests for full benchmark workflows."""

    def test_full_benchmark_workflow_quick(self):
        """Test complete benchmark workflow in quick mode."""
        suite = BenchmarkSuite(quick=True)

        # Run a single benchmark
        def simple_func():
            return sum(range(100))

        result = suite.run_benchmark(
            name="Simple Sum",
            category="Math",
            func=simple_func,
            iterations=10
        )

        # Verify result
        self.assertIsInstance(result, BenchmarkResult)
        self.assertEqual(result.name, "Simple Sum")
        self.assertGreater(result.throughput, 0)

        # Save results to temp file
        with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as f:
            temp_file = f.name

        try:
            suite.save_results(temp_file)

            # Verify file exists and is valid JSON
            with open(temp_file, 'r') as f:
                data = json.load(f)

            self.assertEqual(data['total_tests'], 1)
            self.assertEqual(data['results'][0]['name'], "Simple Sum")

        finally:
            if os.path.exists(temp_file):
                os.remove(temp_file)

    def test_benchmark_result_serialization_roundtrip(self):
        """Test BenchmarkResult serialization and deserialization."""
        original = BenchmarkResult(
            name="Roundtrip Test",
            category="Testing",
            iterations=500,
            total_time=2.5,
            avg_time=0.005,
            throughput=200.0,
            unit="ops/s",
            details={"key": "value"}
        )

        # Convert to dict
        result_dict = original.to_dict()

        # Serialize to JSON
        json_str = json.dumps(result_dict)

        # Deserialize from JSON
        loaded_dict = json.loads(json_str)

        # Verify all fields match
        self.assertEqual(loaded_dict['name'], original.name)
        self.assertEqual(loaded_dict['category'], original.category)
        self.assertEqual(loaded_dict['iterations'], original.iterations)
        self.assertEqual(loaded_dict['total_time'], original.total_time)
        self.assertEqual(loaded_dict['avg_time'], original.avg_time)
        self.assertEqual(loaded_dict['throughput'], original.throughput)
        self.assertEqual(loaded_dict['unit'], original.unit)
        self.assertEqual(loaded_dict['details'], original.details)


if __name__ == '__main__':
    unittest.main()
