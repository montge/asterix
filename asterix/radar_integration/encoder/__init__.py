"""
ASTERIX Encoder - Convert radar data to ASTERIX binary format

This package provides encoding functionality to convert radar detection data
(plots, tracks, etc.) into ASTERIX binary format for testing and validation.

Supported categories:
- CAT048: Transmission of Monoradar Target Reports

Usage:
    from asterix.radar_integration.encoder import encode_cat048
    from asterix.radar_integration import MockRadar

    radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
    plots = radar.generate_plots(num_targets=10)

    asterix_data = encode_cat048(plots, radar_position=(52.5, 13.4, 100.0))

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

from .cat048 import encode_cat048, encode_cat048_datablock

__all__ = ['encode_cat048', 'encode_cat048_datablock']
__version__ = '0.1.0'
