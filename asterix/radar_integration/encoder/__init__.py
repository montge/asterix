"""
ASTERIX Encoder - Convert radar data to ASTERIX binary format

This package provides encoding functionality to convert radar detection data
(plots, tracks, etc.) into ASTERIX binary format for testing and validation.

Supported categories:
- CAT048: Transmission of Monoradar Target Reports
- CAT062: Transmission of System Track Data (fused radar tracks)

Usage:
    from asterix.radar_integration.encoder import encode_cat048, encode_cat062
    from asterix.radar_integration import MockRadar

    # CAT048: Monoradar target reports
    radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
    plots = radar.generate_plots(num_targets=10)
    asterix_data = encode_cat048(plots, radar_position=(52.5, 13.4, 100.0))

    # CAT062: System tracks
    tracks = [
        {'lat': 48.8584, 'lon': 2.2945, 'altitude_ft': 35000,
         'vx': 100.0, 'vy': 200.0, 'callsign': 'AFR123'}
    ]
    asterix_data = encode_cat062(tracks, sac=0, sic=1)

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

from .cat048 import encode_cat048, encode_cat048_datablock
from .cat062 import (
    encode_cat062,
    encode_cat062_datablock,
    encode_cat062_record,
    encode_i010,
    encode_i070,
    encode_i105,
    encode_i135,
    encode_i185,
    encode_i390,
)

__all__ = [
    # CAT048
    'encode_cat048',
    'encode_cat048_datablock',
    # CAT062
    'encode_cat062',
    'encode_cat062_datablock',
    'encode_cat062_record',
    'encode_i010',
    'encode_i070',
    'encode_i105',
    'encode_i135',
    'encode_i185',
    'encode_i390',
]
__version__ = '0.2.0'
