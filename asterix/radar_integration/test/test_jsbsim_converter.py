#!/usr/bin/env python3
"""
Unit tests for JSBSim to ASTERIX converter

Tests:
- FlightState dataclass
- JSBSimToAsterixConverter initialization
- Aircraft loading (with mocking)
- Initial condition setting
- Scenario running
- Flight state capture
- ADS-B report conversion (CAT021)
- System track conversion (CAT062)
- run_scenario_to_asterix function
- Error handling and edge cases

Mock Strategy:
- Mock jsbsim.FGFDMExec to avoid requiring JSBSim installation
- Mock file I/O for run_scenario_to_asterix
- Use realistic flight data for validation

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-12-06
"""

import unittest
import sys
import os
from unittest.mock import MagicMock, patch, mock_open

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', '..'))

from asterix.radar_integration.jsbsim_converter import (
    FlightState,
    JSBSimToAsterixConverter,
    run_scenario_to_asterix
)


class TestFlightState(unittest.TestCase):
    """Test FlightState dataclass."""

    def test_default_initialization(self):
        """Test FlightState with default values."""
        state = FlightState()
        self.assertEqual(state.lat, 0.0)
        self.assertEqual(state.lon, 0.0)
        self.assertEqual(state.alt_msl_ft, 0.0)
        self.assertEqual(state.alt_agl_ft, 0.0)
        self.assertEqual(state.heading_deg, 0.0)
        self.assertEqual(state.pitch_deg, 0.0)
        self.assertEqual(state.roll_deg, 0.0)
        self.assertEqual(state.airspeed_kt, 0.0)
        self.assertEqual(state.groundspeed_kt, 0.0)
        self.assertEqual(state.vertical_speed_fps, 0.0)
        self.assertEqual(state.velocity_north_fps, 0.0)
        self.assertEqual(state.velocity_east_fps, 0.0)
        self.assertEqual(state.velocity_down_fps, 0.0)
        self.assertEqual(state.timestamp, 0.0)

    def test_custom_initialization(self):
        """Test FlightState with custom values."""
        state = FlightState(
            lat=52.5,
            lon=13.4,
            alt_msl_ft=5000.0,
            alt_agl_ft=4800.0,
            heading_deg=90.0,
            pitch_deg=5.0,
            roll_deg=2.0,
            airspeed_kt=120.0,
            groundspeed_kt=115.0,
            vertical_speed_fps=10.0,
            velocity_north_fps=50.0,
            velocity_east_fps=100.0,
            velocity_down_fps=-10.0,
            timestamp=1234567890.0
        )
        self.assertEqual(state.lat, 52.5)
        self.assertEqual(state.lon, 13.4)
        self.assertEqual(state.alt_msl_ft, 5000.0)
        self.assertEqual(state.heading_deg, 90.0)
        self.assertEqual(state.timestamp, 1234567890.0)


class TestJSBSimToAsterixConverter(unittest.TestCase):
    """Test JSBSimToAsterixConverter class."""

    def setUp(self):
        """Set up test fixtures."""
        self.converter = JSBSimToAsterixConverter(sac=10, sic=20)

    def test_initialization(self):
        """Test converter initialization."""
        self.assertEqual(self.converter.sac, 10)
        self.assertEqual(self.converter.sic, 20)
        self.assertIsNone(self.converter.fdm)
        self.assertEqual(self.converter.flight_states, [])
        self.assertEqual(self.converter.track_number_base, 3000)

    def test_initialization_defaults(self):
        """Test converter initialization with defaults."""
        converter = JSBSimToAsterixConverter()
        self.assertEqual(converter.sac, 0)
        self.assertEqual(converter.sic, 1)

    @patch('builtins.__import__')
    def test_load_aircraft_success(self, mock_import):
        """Test successful aircraft loading."""
        mock_jsbsim = MagicMock()
        mock_fdm = MagicMock()
        mock_jsbsim.FGFDMExec.return_value = mock_fdm

        def import_side_effect(name, *args, **kwargs):
            if name == 'jsbsim':
                return mock_jsbsim
            return __import__(name, *args, **kwargs)

        mock_import.side_effect = import_side_effect

        self.converter.load_aircraft('c172p')

        mock_jsbsim.FGFDMExec.assert_called_once_with(None)
        mock_fdm.load_model.assert_called_once_with('c172p')
        self.assertIsNotNone(self.converter.fdm)

    @patch('builtins.__import__')
    def test_load_aircraft_different_models(self, mock_import):
        """Test loading different aircraft models."""
        mock_jsbsim = MagicMock()
        mock_fdm = MagicMock()
        mock_jsbsim.FGFDMExec.return_value = mock_fdm

        def import_side_effect(name, *args, **kwargs):
            if name == 'jsbsim':
                return mock_jsbsim
            return __import__(name, *args, **kwargs)

        mock_import.side_effect = import_side_effect

        for aircraft in ['c172p', 'f16', '747', 'A320']:
            converter = JSBSimToAsterixConverter()
            converter.load_aircraft(aircraft)
            mock_fdm.load_model.assert_called_with(aircraft)

    def test_load_aircraft_no_jsbsim(self):
        """Test aircraft loading when jsbsim not installed."""
        with patch.dict('sys.modules', {'jsbsim': None}):
            with self.assertRaises(ImportError) as context:
                self.converter.load_aircraft('c172p')
            self.assertIn('jsbsim not installed', str(context.exception))

    @patch('builtins.__import__')
    def test_load_aircraft_failure(self, mock_import):
        """Test aircraft loading failure."""
        mock_jsbsim = MagicMock()
        mock_fdm = MagicMock()
        mock_jsbsim.FGFDMExec.return_value = mock_fdm
        mock_fdm.load_model.side_effect = Exception("Model not found")

        def import_side_effect(name, *args, **kwargs):
            if name == 'jsbsim':
                return mock_jsbsim
            return __import__(name, *args, **kwargs)

        mock_import.side_effect = import_side_effect

        with self.assertRaises(RuntimeError) as context:
            self.converter.load_aircraft('invalid_model')
        self.assertIn('Failed to load aircraft', str(context.exception))

    @patch('builtins.__import__')
    def test_set_initial_condition(self, mock_import):
        """Test setting initial conditions."""
        mock_jsbsim = MagicMock()
        mock_fdm = MagicMock()
        mock_jsbsim.FGFDMExec.return_value = mock_fdm

        def import_side_effect(name, *args, **kwargs):
            if name == 'jsbsim':
                return mock_jsbsim
            return __import__(name, *args, **kwargs)

        mock_import.side_effect = import_side_effect

        self.converter.load_aircraft('c172p')

        self.converter.set_initial_condition(
            lat=52.5,
            lon=13.4,
            alt_ft=5000,
            heading_deg=90,
            airspeed_kt=120
        )

        # Verify all properties were set
        self.assertEqual(mock_fdm.__setitem__.call_count, 5)
        mock_fdm.run_ic.assert_called_once()

    @patch('builtins.__import__')
    def test_set_initial_condition_defaults(self, mock_import):
        """Test setting initial conditions with defaults."""
        mock_jsbsim = MagicMock()
        mock_fdm = MagicMock()
        mock_jsbsim.FGFDMExec.return_value = mock_fdm

        def import_side_effect(name, *args, **kwargs):
            if name == 'jsbsim':
                return mock_jsbsim
            return __import__(name, *args, **kwargs)

        mock_import.side_effect = import_side_effect

        self.converter.load_aircraft('c172p')

        self.converter.set_initial_condition()

        mock_fdm.run_ic.assert_called_once()

    def test_set_initial_condition_no_aircraft(self):
        """Test setting initial conditions without loading aircraft."""
        with self.assertRaises(RuntimeError) as context:
            self.converter.set_initial_condition()
        self.assertIn('Aircraft not loaded', str(context.exception))

    @patch('builtins.__import__')
    @patch('time.time')
    def test_capture_state(self, mock_time, mock_import):
        """Test capturing flight state."""
        mock_time.return_value = 1234567890.0
        mock_jsbsim = MagicMock()
        mock_fdm = MagicMock()
        mock_jsbsim.FGFDMExec.return_value = mock_fdm

        def import_side_effect(name, *args, **kwargs):
            if name == 'jsbsim':
                return mock_jsbsim
            return __import__(name, *args, **kwargs)

        mock_import.side_effect = import_side_effect

        # Configure mock to return specific values
        mock_fdm.__getitem__.side_effect = lambda key: {
            'position/lat-geod-deg': 52.5,
            'position/long-gc-deg': 13.4,
            'position/h-sl-ft': 5000.0,
            'position/h-agl-ft': 4800.0,
            'attitude/psi-deg': 90.0,
            'attitude/theta-deg': 5.0,
            'attitude/phi-deg': 2.0,
            'velocities/vc-kts': 120.0,
            'velocities/vg-kts': 115.0,
            'velocities/h-dot-fps': 10.0,
            'velocities/v-north-fps': 50.0,
            'velocities/v-east-fps': 100.0,
            'velocities/v-down-fps': -10.0
        }.get(key, 0.0)

        self.converter.load_aircraft('c172p')
        state = self.converter._capture_state()

        self.assertEqual(state.lat, 52.5)
        self.assertEqual(state.lon, 13.4)
        self.assertEqual(state.alt_msl_ft, 5000.0)
        self.assertEqual(state.heading_deg, 90.0)
        self.assertEqual(state.timestamp, 1234567890.0)

    @patch('builtins.__import__')
    def test_run_scenario(self, mock_import):
        """Test running simulation scenario."""
        mock_jsbsim = MagicMock()
        mock_fdm = MagicMock()
        mock_jsbsim.FGFDMExec.return_value = mock_fdm

        def import_side_effect(name, *args, **kwargs):
            if name == 'jsbsim':
                return mock_jsbsim
            return __import__(name, *args, **kwargs)

        mock_import.side_effect = import_side_effect

        # Simulate 3 seconds with 1 second intervals
        sim_times = [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5]
        time_iter = iter(sim_times)

        def get_sim_time(key):
            if key == 'simulation/sim-time-sec':
                try:
                    return next(time_iter)
                except StopIteration:
                    return 10.0  # Force loop exit
            return 0.0

        mock_fdm.__getitem__.side_effect = get_sim_time

        self.converter.load_aircraft('c172p')
        self.converter.run_scenario(duration=3.0, dt=1.0, throttle=0.7)

        # Should have recorded 4 states (0, 1, 2, 3 seconds)
        self.assertGreaterEqual(len(self.converter.flight_states), 3)

    def test_run_scenario_no_aircraft(self):
        """Test running scenario without loading aircraft."""
        with self.assertRaises(RuntimeError) as context:
            self.converter.run_scenario()
        self.assertIn('Aircraft not loaded', str(context.exception))

    def test_get_adsb_reports_empty(self):
        """Test getting ADS-B reports with no flight states."""
        reports = self.converter.get_adsb_reports()
        self.assertEqual(reports, [])

    def test_get_adsb_reports_single_state(self):
        """Test getting ADS-B reports with single flight state."""
        state = FlightState(
            lat=52.5,
            lon=13.4,
            alt_msl_ft=5000.0,
            alt_agl_ft=4800.0,
            heading_deg=90.0,
            airspeed_kt=120.0,
            groundspeed_kt=115.0,
            vertical_speed_fps=10.0,
            velocity_north_fps=50.0,
            velocity_east_fps=100.0,
            timestamp=1234567890.0
        )
        self.converter.flight_states.append(state)

        reports = self.converter.get_adsb_reports()

        self.assertEqual(len(reports), 1)
        report = reports[0]

        # Verify basic fields
        self.assertEqual(report['lat'], 52.5)
        self.assertEqual(report['lon'], 13.4)
        self.assertEqual(report['aircraft_address'], 0x500000 + 3000)
        self.assertEqual(report['callsign'], 'JSB3000')

        # Verify calculations
        self.assertEqual(report['flight_level'], 50.0)  # 5000 ft / 100
        self.assertEqual(report['gnss_height_ft'], 5000.0)
        self.assertEqual(report['vertical_rate_fpm'], 600.0)  # 10 fps * 60
        self.assertEqual(report['track_number'], 3000)

    def test_get_adsb_reports_custom_track_number(self):
        """Test ADS-B reports with custom track number."""
        state = FlightState(lat=52.5, lon=13.4, timestamp=1234567890.0)
        self.converter.flight_states.append(state)

        reports = self.converter.get_adsb_reports(track_number=5000)

        self.assertEqual(reports[0]['track_number'], 5000)
        self.assertEqual(reports[0]['aircraft_address'], 0x500000 + 5000)
        self.assertEqual(reports[0]['callsign'], 'JSB5000')

    def test_get_adsb_reports_multiple_states(self):
        """Test ADS-B reports with multiple flight states."""
        for i in range(5):
            state = FlightState(
                lat=52.5 + i * 0.01,
                lon=13.4 + i * 0.01,
                alt_msl_ft=5000.0 + i * 100,
                vertical_speed_fps=10.0,
                timestamp=1234567890.0
            )
            self.converter.flight_states.append(state)

        reports = self.converter.get_adsb_reports()

        self.assertEqual(len(reports), 5)

        # Verify progression
        for i, report in enumerate(reports):
            self.assertEqual(report['lat'], 52.5 + i * 0.01)
            self.assertEqual(report['lon'], 13.4 + i * 0.01)
            self.assertEqual(report['flight_level'], (5000.0 + i * 100) / 100.0)
            # Timestamp should increment
            self.assertEqual(report['timestamp'], 1234567890.0 + i)

    def test_get_adsb_reports_velocity_conversion(self):
        """Test velocity conversion in ADS-B reports."""
        state = FlightState(
            velocity_north_fps=100.0,  # feet/second
            velocity_east_fps=200.0,
            timestamp=1234567890.0
        )
        self.converter.flight_states.append(state)

        reports = self.converter.get_adsb_reports()
        report = reports[0]

        # Velocities converted but not directly in report (used for calculations)
        # Verify report contains all expected keys
        self.assertIn('lat', report)
        self.assertIn('lon', report)
        self.assertIn('aircraft_address', report)

    def test_get_system_tracks_empty(self):
        """Test getting system tracks with no flight states."""
        tracks = self.converter.get_system_tracks()
        self.assertEqual(tracks, [])

    def test_get_system_tracks_single_state(self):
        """Test getting system tracks with single flight state."""
        state = FlightState(
            lat=52.5,
            lon=13.4,
            alt_msl_ft=5000.0,
            velocity_north_fps=50.0,  # North
            velocity_east_fps=100.0,  # East
            timestamp=1234567890.0
        )
        self.converter.flight_states.append(state)

        tracks = self.converter.get_system_tracks()

        self.assertEqual(len(tracks), 1)
        track = tracks[0]

        self.assertEqual(track['lat'], 52.5)
        self.assertEqual(track['lon'], 13.4)
        self.assertEqual(track['altitude_ft'], 5000.0)

        # Verify velocity conversion (fps to m/s) and swapping
        # vx should be East (velocity_east_fps * 0.3048)
        # vy should be North (velocity_north_fps * 0.3048)
        self.assertAlmostEqual(track['vx'], 100.0 * 0.3048, places=4)
        self.assertAlmostEqual(track['vy'], 50.0 * 0.3048, places=4)

        self.assertEqual(track['callsign'], 'JSB3000')
        self.assertEqual(track['adep'], 'JSIM')
        self.assertEqual(track['ades'], 'JSIM')

    def test_get_system_tracks_multiple_states(self):
        """Test system tracks with multiple flight states."""
        for i in range(3):
            state = FlightState(
                lat=52.5 + i * 0.01,
                lon=13.4 + i * 0.01,
                alt_msl_ft=5000.0 + i * 100,
                velocity_north_fps=50.0 + i * 10,
                velocity_east_fps=100.0 + i * 20,
                timestamp=1234567890.0
            )
            self.converter.flight_states.append(state)

        tracks = self.converter.get_system_tracks()

        self.assertEqual(len(tracks), 3)

        # Verify progression
        for i, track in enumerate(tracks):
            self.assertEqual(track['lat'], 52.5 + i * 0.01)
            self.assertEqual(track['altitude_ft'], 5000.0 + i * 100)
            self.assertEqual(track['timestamp'], 1234567890.0 + i)


class TestRunScenarioToAsterix(unittest.TestCase):
    """Test run_scenario_to_asterix function."""

    @patch('builtins.__import__')
    @patch('builtins.open', new_callable=mock_open)
    def test_run_scenario_cat021(self, mock_file, mock_import):
        """Test run_scenario_to_asterix with CAT021."""
        # Create mock encoder module
        mock_encoder = MagicMock()
        mock_encode_cat021 = MagicMock(return_value=b'\x15\x00\x10' * 10)
        mock_encoder.encode_cat021 = mock_encode_cat021
        mock_encoder.encode_cat062 = MagicMock()

        mock_jsbsim = MagicMock()
        mock_fdm = MagicMock()
        mock_jsbsim.FGFDMExec.return_value = mock_fdm

        original_import = __import__

        def import_side_effect(name, *args, **kwargs):
            if name == 'jsbsim':
                return mock_jsbsim
            elif name == 'asterix.radar_integration.encoder':
                return mock_encoder
            return original_import(name, *args, **kwargs)

        mock_import.side_effect = import_side_effect

        # Mock simulation time progression
        sim_times = [0.0, 1.0, 2.0, 3.0]
        time_iter = iter(sim_times)

        def get_value(key):
            if key == 'simulation/sim-time-sec':
                try:
                    return next(time_iter)
                except StopIteration:
                    return 10.0
            return 0.0

        mock_fdm.__getitem__.side_effect = get_value

        run_scenario_to_asterix(
            aircraft='c172p',
            initial_lat=52.5,
            initial_lon=13.4,
            initial_alt_ft=5000,
            duration=3.0,
            output_file='test_output.ast',
            category='CAT021',
            sac=10,
            sic=20
        )

        # Verify encode_cat021 was called
        mock_encode_cat021.assert_called_once()
        call_args = mock_encode_cat021.call_args
        self.assertEqual(call_args[1]['sac'], 10)
        self.assertEqual(call_args[1]['sic'], 20)

        # Verify file was written
        mock_file.assert_called_once_with('test_output.ast', 'wb')
        mock_file().write.assert_called_once()

    @patch('builtins.__import__')
    @patch('builtins.open', new_callable=mock_open)
    def test_run_scenario_cat062(self, mock_file, mock_import):
        """Test run_scenario_to_asterix with CAT062."""
        # Create mock encoder module
        mock_encoder = MagicMock()
        mock_encode_cat062 = MagicMock(return_value=b'\x3e\x00\x10' * 10)
        mock_encoder.encode_cat021 = MagicMock()
        mock_encoder.encode_cat062 = mock_encode_cat062

        mock_jsbsim = MagicMock()
        mock_fdm = MagicMock()
        mock_jsbsim.FGFDMExec.return_value = mock_fdm

        original_import = __import__

        def import_side_effect(name, *args, **kwargs):
            if name == 'jsbsim':
                return mock_jsbsim
            elif name == 'asterix.radar_integration.encoder':
                return mock_encoder
            return original_import(name, *args, **kwargs)

        mock_import.side_effect = import_side_effect

        # Mock simulation time progression
        sim_times = [0.0, 1.0, 2.0, 3.0]
        time_iter = iter(sim_times)

        def get_value(key):
            if key == 'simulation/sim-time-sec':
                try:
                    return next(time_iter)
                except StopIteration:
                    return 10.0
            return 0.0

        mock_fdm.__getitem__.side_effect = get_value

        run_scenario_to_asterix(
            aircraft='f16',
            initial_lat=37.62,
            initial_lon=-122.38,
            initial_alt_ft=10000,
            duration=3.0,
            output_file='test_output.ast',
            category='CAT062',
            sac=5,
            sic=15
        )

        # Verify encode_cat062 was called
        mock_encode_cat062.assert_called_once()
        call_args = mock_encode_cat062.call_args
        self.assertEqual(call_args[1]['sac'], 5)
        self.assertEqual(call_args[1]['sic'], 15)

        # Verify file was written
        mock_file.assert_called_once_with('test_output.ast', 'wb')

    @patch('builtins.__import__')
    def test_run_scenario_invalid_category(self, mock_import):
        """Test run_scenario_to_asterix with invalid category."""
        # Create mock encoder module
        mock_encoder = MagicMock()
        mock_encoder.encode_cat021 = MagicMock()
        mock_encoder.encode_cat062 = MagicMock()

        mock_jsbsim = MagicMock()
        mock_fdm = MagicMock()
        mock_jsbsim.FGFDMExec.return_value = mock_fdm

        original_import = __import__

        def import_side_effect(name, *args, **kwargs):
            if name == 'jsbsim':
                return mock_jsbsim
            elif name == 'asterix.radar_integration.encoder':
                return mock_encoder
            return original_import(name, *args, **kwargs)

        mock_import.side_effect = import_side_effect

        # Mock simulation time
        mock_fdm.__getitem__.side_effect = lambda key: 10.0 if key == 'simulation/sim-time-sec' else 0.0

        with self.assertRaises(ValueError) as context:
            run_scenario_to_asterix(
                aircraft='c172p',
                duration=1.0,
                category='CAT999',
                output_file='test.ast'
            )
        self.assertIn('Unsupported category', str(context.exception))

    @patch('builtins.__import__')
    @patch('builtins.open', new_callable=mock_open)
    def test_run_scenario_default_parameters(self, mock_file, mock_import):
        """Test run_scenario_to_asterix with default parameters."""
        # Create mock encoder module
        mock_encoder = MagicMock()
        mock_encode_cat021 = MagicMock(return_value=b'\x15\x00\x10')
        mock_encoder.encode_cat021 = mock_encode_cat021
        mock_encoder.encode_cat062 = MagicMock()

        mock_jsbsim = MagicMock()
        mock_fdm = MagicMock()
        mock_jsbsim.FGFDMExec.return_value = mock_fdm

        original_import = __import__

        def import_side_effect(name, *args, **kwargs):
            if name == 'jsbsim':
                return mock_jsbsim
            elif name == 'asterix.radar_integration.encoder':
                return mock_encoder
            return original_import(name, *args, **kwargs)

        mock_import.side_effect = import_side_effect

        # Mock simulation time - must exceed default duration (300s)
        # to allow loop to exit
        mock_fdm.__getitem__.side_effect = lambda key: 500.0 if key == 'simulation/sim-time-sec' else 0.0

        # Call with minimal parameters (using defaults)
        run_scenario_to_asterix()

        # Verify default aircraft was loaded
        mock_fdm.load_model.assert_called_once_with('c172p')

        # Verify encode_cat021 was called with defaults
        call_args = mock_encode_cat021.call_args
        self.assertEqual(call_args[1]['sac'], 0)
        self.assertEqual(call_args[1]['sic'], 1)


class TestEdgeCasesAndValidation(unittest.TestCase):
    """Test edge cases and validation logic."""

    def test_flight_state_negative_values(self):
        """Test FlightState with negative values (valid for some fields)."""
        state = FlightState(
            lat=-45.0,
            lon=-90.0,
            alt_msl_ft=-100.0,  # Below sea level
            pitch_deg=-10.0,  # Nose down
            roll_deg=-20.0,  # Bank left
            vertical_speed_fps=-20.0  # Descending
        )
        self.assertEqual(state.lat, -45.0)
        self.assertEqual(state.vertical_speed_fps, -20.0)

    def test_adsb_reports_zero_altitude(self):
        """Test ADS-B reports at zero altitude."""
        converter = JSBSimToAsterixConverter()
        state = FlightState(alt_msl_ft=0.0, timestamp=1234567890.0)
        converter.flight_states.append(state)

        reports = converter.get_adsb_reports()
        self.assertEqual(reports[0]['flight_level'], 0.0)

    def test_adsb_reports_high_altitude(self):
        """Test ADS-B reports at high altitude."""
        converter = JSBSimToAsterixConverter()
        state = FlightState(alt_msl_ft=45000.0, timestamp=1234567890.0)
        converter.flight_states.append(state)

        reports = converter.get_adsb_reports()
        self.assertEqual(reports[0]['flight_level'], 450.0)

    def test_system_tracks_zero_velocity(self):
        """Test system tracks with zero velocity (stationary)."""
        converter = JSBSimToAsterixConverter()
        state = FlightState(
            velocity_north_fps=0.0,
            velocity_east_fps=0.0,
            timestamp=1234567890.0
        )
        converter.flight_states.append(state)

        tracks = converter.get_system_tracks()
        self.assertEqual(tracks[0]['vx'], 0.0)
        self.assertEqual(tracks[0]['vy'], 0.0)

    def test_converter_large_track_number(self):
        """Test converter with large track number."""
        converter = JSBSimToAsterixConverter()
        converter.track_number_base = 99999

        state = FlightState(timestamp=1234567890.0)
        converter.flight_states.append(state)

        reports = converter.get_adsb_reports()
        self.assertEqual(reports[0]['track_number'], 99999)
        self.assertEqual(reports[0]['aircraft_address'], 0x500000 + 99999)

    def test_vertical_rate_conversion_precision(self):
        """Test vertical rate conversion maintains precision."""
        converter = JSBSimToAsterixConverter()
        state = FlightState(
            vertical_speed_fps=12.5,  # fps
            timestamp=1234567890.0
        )
        converter.flight_states.append(state)

        reports = converter.get_adsb_reports()
        # 12.5 fps * 60 = 750 fpm
        self.assertEqual(reports[0]['vertical_rate_fpm'], 750.0)

    def test_velocity_conversion_precision(self):
        """Test velocity conversion maintains precision."""
        converter = JSBSimToAsterixConverter()
        state = FlightState(
            velocity_north_fps=100.0,
            velocity_east_fps=150.0,
            timestamp=1234567890.0
        )
        converter.flight_states.append(state)

        tracks = converter.get_system_tracks()
        # fps to m/s: multiply by 0.3048
        self.assertAlmostEqual(tracks[0]['vx'], 150.0 * 0.3048, places=6)
        self.assertAlmostEqual(tracks[0]['vy'], 100.0 * 0.3048, places=6)


if __name__ == '__main__':
    unittest.main()
