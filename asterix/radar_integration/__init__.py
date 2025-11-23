"""
ASTERIX Radar Integration Package

This package provides tools for integrating radar simulators with ASTERIX encoding/decoding:
- Mock radar data generation for testing
- ASTERIX CAT048 encoding (radar target reports)
- Example scenarios and validation utilities

Supported ASTERIX categories:
- CAT048: Transmission of Monoradar Target Reports

Usage:
    from asterix.radar_integration import MockRadar, generate_aircraft_scenario
    from asterix.radar_integration.encoder import encode_cat048

    # Generate synthetic radar data
    radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
    plots = radar.generate_plots(num_targets=10)

    # Encode to ASTERIX CAT048
    asterix_data = encode_cat048(plots, sac=0, sic=1)

    # Decode with main ASTERIX parser
    import asterix
    decoded = asterix.parse(asterix_data)

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

from .mock_radar import (
    MockRadar,
    RadarPlot,
    RadarPosition,
    generate_aircraft_scenario,
    generate_approach_scenario
)

__all__ = [
    'MockRadar',
    'RadarPlot',
    'RadarPosition',
    'generate_aircraft_scenario',
    'generate_approach_scenario',
]

__version__ = '0.1.0'
