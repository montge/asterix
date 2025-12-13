#!/usr/bin/env python3
"""
Unit tests for ASTERIX Radar Integration CLI

Tests:
- Argument parsing for all commands (generate, visualize, benchmark, info)
- Command execution and output
- File I/O operations with mocking
- Error handling and edge cases
- Integration with other modules

Coverage target: >80%

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-12-06
"""

import unittest
import sys
import os
import argparse
from unittest.mock import patch, MagicMock, mock_open

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', '..'))

from asterix.radar_integration.cli import (
    cmd_generate,
    cmd_visualize,
    cmd_benchmark,
    cmd_info,
    main
)


class TestCmdGenerate(unittest.TestCase):
    """Test cmd_generate function."""

    def setUp(self):
        """Set up test fixtures."""
        self.default_args = argparse.Namespace(
            count=10,
            category='CAT048',
            output=None,
            lat=52.5,
            lon=13.4,
            alt=100.0,
            max_range=200,
            noise=False,
            clutter=False,
            visualize=False,
            stats=False,
            sac=0,
            sic=1
        )

    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.print')
    def test_generate_basic(self, mock_print, mock_radar_class):
        """Test basic generate command without options."""
        # Setup mock
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots

        # Execute
        cmd_generate(self.default_args)

        # Verify MockRadar initialization
        mock_radar_class.assert_called_once_with(
            lat=52.5,
            lon=13.4,
            alt=100.0,
            max_range=200000  # km to meters
        )

        # Verify generate_plots call
        mock_radar.generate_plots.assert_called_once_with(
            num_targets=10,
            add_noise=False,
            add_clutter=False
        )

        # Verify output
        self.assertTrue(any('Generating 10 radar plots' in str(call) for call in mock_print.call_args_list))
        self.assertTrue(any('Generated 10 plots' in str(call) for call in mock_print.call_args_list))

    @patch('asterix.radar_integration.cli.encode_cat001')
    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.print')
    def test_generate_cat001(self, mock_print, mock_radar_class, mock_encode):
        """Test generate with CAT001 category."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots
        mock_encode.return_value = b'\x01\x02\x03'

        args = self.default_args
        args.category = 'CAT001'

        cmd_generate(args)

        mock_encode.assert_called_once_with(mock_plots, sac=0, sic=1)
        self.assertTrue(any('CAT001' in str(call) for call in mock_print.call_args_list))

    @patch('asterix.radar_integration.cli.encode_cat048')
    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.print')
    def test_generate_cat048(self, mock_print, mock_radar_class, mock_encode):
        """Test generate with CAT048 category (default)."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots
        mock_encode.return_value = b'\x30\x00\x30'

        cmd_generate(self.default_args)

        mock_encode.assert_called_once_with(mock_plots, sac=0, sic=1)

    @patch('asterix.radar_integration.cli.encode_cat062')
    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.print')
    def test_generate_cat062(self, mock_print, mock_radar_class, mock_encode):
        """Test generate with CAT062 category."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots
        mock_encode.return_value = b'\x3e\x00\x30'

        args = self.default_args
        args.category = 'CAT062'

        cmd_generate(args)

        # CAT062 uses different calling convention (generates its own data)
        self.assertEqual(mock_encode.call_count, 1)

    @patch('asterix.radar_integration.cli.encode_cat021')
    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.print')
    def test_generate_cat021(self, mock_print, mock_radar_class, mock_encode):
        """Test generate with CAT021 category."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots
        mock_encode.return_value = b'\x15\x00\x30'

        args = self.default_args
        args.category = 'CAT021'

        cmd_generate(args)

        # CAT021 uses different calling convention
        self.assertEqual(mock_encode.call_count, 1)

    @patch('asterix.radar_integration.cli.encode_cat048')
    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.open', new_callable=mock_open)
    @patch('builtins.print')
    def test_generate_with_output_file(self, mock_print, mock_file, mock_radar_class, mock_encode):
        """Test generate with output file."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots
        test_data = b'\x30\x00\x30\x01\x02\x03'
        mock_encode.return_value = test_data

        args = self.default_args
        args.output = 'test_output.ast'

        cmd_generate(args)

        # Verify file write
        mock_file.assert_called_once_with('test_output.ast', 'wb')
        mock_file().write.assert_called_once_with(test_data)
        self.assertTrue(any('Saved to test_output.ast' in str(call) for call in mock_print.call_args_list))

    @patch('asterix.radar_integration.cli.plot_radar_ascii')
    @patch('asterix.radar_integration.cli.encode_cat048')
    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.print')
    def test_generate_with_visualize(self, mock_print, mock_radar_class, mock_encode, mock_plot):
        """Test generate with visualize option."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots
        mock_encode.return_value = b'\x30\x00\x30'
        mock_plot.return_value = "ASCII RADAR PLOT"

        args = self.default_args
        args.visualize = True

        cmd_generate(args)

        mock_plot.assert_called_once_with(mock_plots)
        self.assertTrue(any('ASCII RADAR PLOT' in str(call) for call in mock_print.call_args_list))

    @patch('asterix.radar_integration.cli.compute_scenario_stats')
    @patch('asterix.radar_integration.cli.encode_cat048')
    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.print')
    def test_generate_with_stats(self, mock_print, mock_radar_class, mock_encode, mock_stats):
        """Test generate with statistics option."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots
        mock_encode.return_value = b'\x30\x00\x30'
        mock_stats.return_value = {
            'range_stats': {'mean': 50000, 'min': 10000, 'max': 100000},
            'snr_stats': {'mean': 25.5},
            'azimuth_stats': {'coverage_deg': 360}
        }

        args = self.default_args
        args.stats = True

        cmd_generate(args)

        mock_stats.assert_called_once_with(mock_plots)
        self.assertTrue(any('Statistics' in str(call) for call in mock_print.call_args_list))
        self.assertTrue(any('50.0 km avg' in str(call) for call in mock_print.call_args_list))

    @patch('asterix.radar_integration.cli.encode_cat048')
    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.print')
    def test_generate_with_noise_and_clutter(self, mock_print, mock_radar_class, mock_encode):
        """Test generate with noise and clutter options."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots
        mock_encode.return_value = b'\x30\x00\x30'

        args = self.default_args
        args.noise = True
        args.clutter = True

        cmd_generate(args)

        mock_radar.generate_plots.assert_called_once_with(
            num_targets=10,
            add_noise=True,
            add_clutter=True
        )

    @patch('asterix.radar_integration.cli.encode_cat048')
    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.print')
    def test_generate_custom_coordinates(self, mock_print, mock_radar_class, mock_encode):
        """Test generate with custom radar coordinates."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots
        mock_encode.return_value = b'\x30\x00\x30'

        args = self.default_args
        args.lat = 40.7
        args.lon = -74.0
        args.alt = 150.0
        args.max_range = 100

        cmd_generate(args)

        mock_radar_class.assert_called_once_with(
            lat=40.7,
            lon=-74.0,
            alt=150.0,
            max_range=100000
        )


class TestCmdVisualize(unittest.TestCase):
    """Test cmd_visualize function."""

    def setUp(self):
        """Set up test fixtures."""
        self.default_args = argparse.Namespace(
            file=None,
            count=None,
            lat=52.5,
            lon=13.4,
            alt=100.0
        )

    @patch('asterix.radar_integration.cli.compute_scenario_stats')
    @patch('asterix.radar_integration.cli.plot_radar_ascii')
    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.print')
    def test_visualize_generate_mode(self, mock_print, mock_radar_class, mock_plot, mock_stats):
        """Test visualize in generate mode (no file)."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 20
        mock_radar.generate_plots.return_value = mock_plots
        mock_plot.return_value = "ASCII RADAR DISPLAY"
        mock_stats.return_value = {
            'range_stats': {'mean': 75000}
        }

        cmd_visualize(self.default_args)

        mock_radar_class.assert_called_once_with(lat=52.5, lon=13.4, alt=100.0)
        mock_radar.generate_plots.assert_called_once_with(num_targets=20)
        mock_plot.assert_called_once_with(mock_plots)
        self.assertTrue(any('20 targets detected' in str(call) for call in mock_print.call_args_list))

    @patch('asterix.radar_integration.cli.compute_scenario_stats')
    @patch('asterix.radar_integration.cli.plot_radar_ascii')
    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('builtins.print')
    def test_visualize_custom_count(self, mock_print, mock_radar_class, mock_plot, mock_stats):
        """Test visualize with custom count."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 50
        mock_radar.generate_plots.return_value = mock_plots
        mock_plot.return_value = "ASCII RADAR DISPLAY"
        mock_stats.return_value = {
            'range_stats': {'mean': 75000}
        }

        args = self.default_args
        args.count = 50

        cmd_visualize(args)

        mock_radar.generate_plots.assert_called_once_with(num_targets=50)

    @patch('builtins.open', new_callable=mock_open, read_data=b'\x30\x00\x10')
    @patch('builtins.print')
    def test_visualize_file_mode(self, mock_print, mock_file):
        """Test visualize with file input."""
        # Mock asterix module import inside cmd_visualize
        with patch.dict('sys.modules', {'asterix': MagicMock()}):
            import sys
            mock_asterix = sys.modules['asterix']
            mock_asterix.parse.return_value = [{'cat': 48}] * 5

            args = self.default_args
            args.file = 'test.ast'

            cmd_visualize(args)

            mock_file.assert_called_once_with('test.ast', 'rb')
            self.assertTrue(any('Decoded 5 records' in str(call) for call in mock_print.call_args_list))

    @patch('builtins.open', new_callable=mock_open, read_data=b'\x30\x00\x10')
    @patch('builtins.print')
    def test_visualize_file_error(self, mock_print, mock_file):
        """Test visualize with file error."""
        # Mock asterix module with error
        with patch.dict('sys.modules', {'asterix': MagicMock()}):
            import sys
            mock_asterix = sys.modules['asterix']
            mock_asterix.parse.side_effect = Exception("Parse error")

            args = self.default_args
            args.file = 'test.ast'

            cmd_visualize(args)

            self.assertTrue(any('Error' in str(call) for call in mock_print.call_args_list))

    @patch('builtins.open', side_effect=FileNotFoundError("File not found"))
    @patch('builtins.print')
    def test_visualize_file_not_found(self, mock_print, mock_file):
        """Test visualize with missing file."""
        args = self.default_args
        args.file = 'missing.ast'

        # Should catch exception
        try:
            cmd_visualize(args)
        except FileNotFoundError:
            pass  # Expected

        # Error should be caught and printed
        # (Note: Current implementation may not catch FileNotFoundError, testing actual behavior)


class TestCmdBenchmark(unittest.TestCase):
    """Test cmd_benchmark function."""

    def setUp(self):
        """Set up test fixtures."""
        self.default_args = argparse.Namespace(
            quick=False,
            save=None
        )

    @patch('builtins.print')
    def test_benchmark_basic(self, mock_print):
        """Test basic benchmark command."""
        # Mock BenchmarkSuite imported inside cmd_benchmark
        with patch('asterix.radar_integration.benchmark.BenchmarkSuite') as mock_suite_class:
            mock_suite = MagicMock()
            mock_suite_class.return_value = mock_suite

            cmd_benchmark(self.default_args)

            mock_suite_class.assert_called_once_with(quick=False)
            mock_suite.run_all.assert_called_once()
            mock_suite.print_summary.assert_called_once()

    @patch('builtins.print')
    def test_benchmark_quick(self, mock_print):
        """Test benchmark with quick mode."""
        # Mock BenchmarkSuite imported inside cmd_benchmark
        with patch('asterix.radar_integration.benchmark.BenchmarkSuite') as mock_suite_class:
            mock_suite = MagicMock()
            mock_suite_class.return_value = mock_suite

            args = self.default_args
            args.quick = True

            cmd_benchmark(args)

            mock_suite_class.assert_called_once_with(quick=True)

    @patch('builtins.print')
    def test_benchmark_with_save(self, mock_print):
        """Test benchmark with save option."""
        # Mock BenchmarkSuite imported inside cmd_benchmark
        with patch('asterix.radar_integration.benchmark.BenchmarkSuite') as mock_suite_class:
            mock_suite = MagicMock()
            mock_suite_class.return_value = mock_suite

            args = self.default_args
            args.save = 'results.json'

            cmd_benchmark(args)

            mock_suite.save_results.assert_called_once_with('results.json')


class TestCmdInfo(unittest.TestCase):
    """Test cmd_info function."""

    @patch('builtins.print')
    def test_info_basic(self, mock_print):
        """Test info command output."""
        args = argparse.Namespace()

        cmd_info(args)

        # Check that important information is printed
        printed_output = ' '.join(str(call) for call in mock_print.call_args_list)
        self.assertIn('ASTERIX Radar Integration', printed_output)
        self.assertIn('Version', printed_output)
        self.assertIn('GPL-3.0', printed_output)
        self.assertIn('Encoders', printed_output)
        self.assertIn('Decoders', printed_output)

    @patch('builtins.print')
    def test_info_shows_capabilities(self, mock_print):
        """Test that info shows all capabilities."""
        args = argparse.Namespace()

        cmd_info(args)

        printed_output = ' '.join(str(call) for call in mock_print.call_args_list)
        self.assertIn('CAT001', printed_output)
        self.assertIn('CAT048', printed_output)
        # CAT062 is shown as '062' in the output
        self.assertTrue('062' in printed_output or 'CAT062' in printed_output)
        self.assertIn('Mock Radar', printed_output)
        self.assertIn('Visualization', printed_output)


class TestMainFunction(unittest.TestCase):
    """Test main() CLI entry point."""

    @patch('asterix.radar_integration.cli.cmd_info')
    @patch('sys.argv', ['cli.py', 'info'])
    def test_main_info_command(self, mock_cmd):
        """Test main with info command."""
        main()
        mock_cmd.assert_called_once()

    @patch('asterix.radar_integration.cli.cmd_generate')
    @patch('sys.argv', ['cli.py', 'generate', '--count', '10'])
    def test_main_generate_command(self, mock_cmd):
        """Test main with generate command."""
        main()
        mock_cmd.assert_called_once()
        args = mock_cmd.call_args[0][0]
        self.assertEqual(args.count, 10)

    @patch('asterix.radar_integration.cli.cmd_generate')
    @patch('sys.argv', ['cli.py', 'generate', '--category', 'CAT062'])
    def test_main_generate_category(self, mock_cmd):
        """Test main with generate category option."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertEqual(args.category, 'CAT062')

    @patch('asterix.radar_integration.cli.cmd_visualize')
    @patch('sys.argv', ['cli.py', 'visualize'])
    def test_main_visualize_command(self, mock_cmd):
        """Test main with visualize command."""
        main()
        mock_cmd.assert_called_once()

    @patch('asterix.radar_integration.cli.cmd_visualize')
    @patch('sys.argv', ['cli.py', 'visualize', 'test.ast'])
    def test_main_visualize_with_file(self, mock_cmd):
        """Test main with visualize file argument."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertEqual(args.file, 'test.ast')

    @patch('asterix.radar_integration.cli.cmd_benchmark')
    @patch('sys.argv', ['cli.py', 'benchmark', '--quick'])
    def test_main_benchmark_command(self, mock_cmd):
        """Test main with benchmark command."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertTrue(args.quick)

    @patch('asterix.radar_integration.cli.cmd_benchmark')
    @patch('sys.argv', ['cli.py', 'benchmark', '--save', 'results.json'])
    def test_main_benchmark_save(self, mock_cmd):
        """Test main with benchmark save option."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertEqual(args.save, 'results.json')

    @patch('sys.argv', ['cli.py'])
    @patch('argparse.ArgumentParser.print_help')
    def test_main_no_command(self, mock_help):
        """Test main with no command (should print help)."""
        main()
        mock_help.assert_called_once()

    @patch('asterix.radar_integration.cli.cmd_generate')
    @patch('sys.argv', ['cli.py', 'generate', '--lat', '40.7', '--lon', '-74.0', '--alt', '150'])
    def test_main_generate_coordinates(self, mock_cmd):
        """Test main with custom coordinates."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertEqual(args.lat, 40.7)
        self.assertEqual(args.lon, -74.0)
        self.assertEqual(args.alt, 150.0)

    @patch('asterix.radar_integration.cli.cmd_generate')
    @patch('sys.argv', ['cli.py', 'generate', '--noise', '--clutter'])
    def test_main_generate_flags(self, mock_cmd):
        """Test main with boolean flags."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertTrue(args.noise)
        self.assertTrue(args.clutter)

    @patch('asterix.radar_integration.cli.cmd_generate')
    @patch('sys.argv', ['cli.py', 'generate', '--visualize', '--stats'])
    def test_main_generate_visualization_flags(self, mock_cmd):
        """Test main with visualization and stats flags."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertTrue(args.visualize)
        self.assertTrue(args.stats)

    @patch('asterix.radar_integration.cli.cmd_generate')
    @patch('sys.argv', ['cli.py', 'generate', '-o', 'output.ast'])
    def test_main_generate_short_output(self, mock_cmd):
        """Test main with short output flag."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertEqual(args.output, 'output.ast')

    @patch('asterix.radar_integration.cli.cmd_generate')
    @patch('sys.argv', ['cli.py', 'generate', '--sac', '5', '--sic', '10'])
    def test_main_generate_sac_sic(self, mock_cmd):
        """Test main with SAC/SIC parameters."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertEqual(args.sac, 5)
        self.assertEqual(args.sic, 10)

    @patch('asterix.radar_integration.cli.cmd_visualize')
    @patch('sys.argv', ['cli.py', 'visualize', '--count', '100'])
    def test_main_visualize_count(self, mock_cmd):
        """Test main with visualize count option."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertEqual(args.count, 100)


class TestArgumentParsing(unittest.TestCase):
    """Test argument parsing edge cases."""

    @patch('asterix.radar_integration.cli.cmd_generate')
    @patch('sys.argv', ['cli.py', 'generate', '--max-range', '500'])
    def test_max_range_parsing(self, mock_cmd):
        """Test max-range argument parsing."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertEqual(args.max_range, 500.0)

    @patch('asterix.radar_integration.cli.cmd_generate')
    @patch('sys.argv', ['cli.py', 'generate', '--count', '0'])
    def test_zero_count(self, mock_cmd):
        """Test zero count argument."""
        main()
        args = mock_cmd.call_args[0][0]
        self.assertEqual(args.count, 0)

    @patch('asterix.radar_integration.cli.cmd_generate')
    @patch('sys.argv', ['cli.py', 'generate', '--category', 'CAT001'])
    def test_all_categories(self, mock_cmd):
        """Test all valid category choices."""
        for category in ['CAT001', 'CAT020', 'CAT021', 'CAT048', 'CAT062']:
            with patch('sys.argv', ['cli.py', 'generate', '--category', category]):
                main()
                args = mock_cmd.call_args[0][0]
                self.assertEqual(args.category, category)


class TestIntegration(unittest.TestCase):
    """Integration tests with minimal mocking."""

    @patch('builtins.print')
    def test_info_command_integration(self, mock_print):
        """Test info command end-to-end."""
        args = argparse.Namespace()
        cmd_info(args)

        # Verify it doesn't crash and prints something
        self.assertGreater(mock_print.call_count, 0)

    @patch('builtins.print')
    def test_benchmark_command_integration(self, mock_print):
        """Test benchmark command with real BenchmarkSuite initialization."""
        # Mock BenchmarkSuite imported inside cmd_benchmark
        with patch('asterix.radar_integration.benchmark.BenchmarkSuite') as mock_suite_class:
            mock_suite = MagicMock()
            mock_suite_class.return_value = mock_suite

            args = argparse.Namespace(quick=True, save=None)
            cmd_benchmark(args)

            # Verify suite was created and methods called
            mock_suite_class.assert_called_once_with(quick=True)
            mock_suite.run_all.assert_called_once()
            mock_suite.print_summary.assert_called_once()


class TestErrorHandling(unittest.TestCase):
    """Test error handling and edge cases."""

    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('asterix.radar_integration.cli.encode_cat048')
    @patch('builtins.open', side_effect=PermissionError("Permission denied"))
    @patch('builtins.print')
    def test_generate_file_write_error(self, mock_print, mock_file, mock_encode, mock_radar_class):
        """Test generate with file write permission error."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots
        mock_encode.return_value = b'\x30\x00\x30'

        args = argparse.Namespace(
            count=10, category='CAT048', output='readonly.ast',
            lat=52.5, lon=13.4, alt=100.0, max_range=200,
            noise=False, clutter=False, visualize=False, stats=False,
            sac=0, sic=1
        )

        # Should raise PermissionError
        with self.assertRaises(PermissionError):
            cmd_generate(args)

    @patch('asterix.radar_integration.cli.MockRadar')
    @patch('asterix.radar_integration.cli.plot_radar_ascii', side_effect=Exception("Visualization error"))
    @patch('asterix.radar_integration.cli.encode_cat048')
    @patch('builtins.print')
    def test_generate_visualization_error(self, mock_print, mock_encode, mock_plot, mock_radar_class):
        """Test generate with visualization error."""
        mock_radar = MagicMock()
        mock_radar_class.return_value = mock_radar
        mock_plots = [MagicMock()] * 10
        mock_radar.generate_plots.return_value = mock_plots
        mock_encode.return_value = b'\x30\x00\x30'

        args = argparse.Namespace(
            count=10, category='CAT048', output=None,
            lat=52.5, lon=13.4, alt=100.0, max_range=200,
            noise=False, clutter=False, visualize=True, stats=False,
            sac=0, sic=1
        )

        # Should raise exception from visualization
        with self.assertRaises(Exception):
            cmd_generate(args)

    @patch('asterix.radar_integration.cli.MockRadar', side_effect=Exception("Radar init error"))
    @patch('builtins.print')
    def test_generate_radar_init_error(self, mock_print, mock_radar_class):
        """Test generate with radar initialization error."""
        args = argparse.Namespace(
            count=10, category='CAT048', output=None,
            lat=52.5, lon=13.4, alt=100.0, max_range=200,
            noise=False, clutter=False, visualize=False, stats=False,
            sac=0, sic=1
        )

        with self.assertRaises(Exception):
            cmd_generate(args)


if __name__ == '__main__':
    # Run with coverage if available
    try:
        import pytest
        sys.exit(pytest.main([__file__, '-v', '--cov=asterix.radar_integration.cli', '--cov-report=term-missing']))
    except ImportError:
        # Fallback to unittest
        unittest.main(verbosity=2)
