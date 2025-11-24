"""
ASTERIX Encoder - Convert radar data to ASTERIX binary format

This package provides encoding functionality to convert radar detection data
(plots, tracks, etc.) into ASTERIX binary format for testing and validation.

Supported categories:
- CAT001: Monoradar Target Reports (older specification)
- CAT019: Multilateration System Status Messages
- CAT020: Multilateration Target Reports (MLAT)
- CAT021: ADS-B Target Reports (high precision)
- CAT034: Monoradar Service Messages (North Marker, Sector Crossing)
- CAT048: Monoradar Target Reports (modern specification)
- CAT062: System Track Data (fused multi-sensor tracks)

Usage:
    from asterix.radar_integration.encoder import (
        encode_cat001, encode_cat020, encode_cat021,
        encode_cat034_north_marker, encode_cat048, encode_cat062
    )
    from asterix.radar_integration import MockRadar

    # Generate radar data
    radar = MockRadar(lat=52.5, lon=13.4, alt=100.0)
    plots = radar.generate_plots(num_targets=10)

    # Encode to various categories
    cat001_data = encode_cat001(plots)  # Older radar reports
    cat048_data = encode_cat048(plots)  # Modern radar reports

    # MLAT surface surveillance
    mlat_targets = [{'lat': 52.5, 'lon': 13.4, 'aircraft_address': 0x3C6544, ...}]
    cat020_data = encode_cat020(mlat_targets)

    # ADS-B
    adsb_reports = [{'lat': 48.8, 'lon': 2.3, 'aircraft_address': 0x3950A1, ...}]
    cat021_data = encode_cat021(adsb_reports)

    # System tracks
    tracks = [{'lat': 48.8, 'lon': 2.3, 'altitude_ft': 35000, 'vx': 100, 'vy': 200, ...}]
    cat062_data = encode_cat062(tracks)

    # Radar service messages
    north_marker = encode_cat034_north_marker(sac=0, sic=1, antenna_rotation_speed=4.0)

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

from .cat001 import encode_cat001, encode_cat001_datablock, encode_cat001_record
from .cat019 import encode_cat019_status, encode_cat019_update_cycle
from .cat020 import encode_cat020, encode_cat020_datablock, encode_cat020_record
from .cat021 import encode_cat021, encode_cat021_datablock, encode_cat021_record
from .cat034 import encode_cat034_north_marker, encode_cat034_sector_crossing
from .cat048 import encode_cat048, encode_cat048_datablock
from .cat062 import (
    encode_cat062,
    encode_cat062_datablock,
    encode_cat062_record,
)

__all__ = [
    # CAT001
    'encode_cat001',
    'encode_cat001_datablock',
    'encode_cat001_record',
    # CAT019
    'encode_cat019_status',
    'encode_cat019_update_cycle',
    # CAT020
    'encode_cat020',
    'encode_cat020_datablock',
    'encode_cat020_record',
    # CAT021
    'encode_cat021',
    'encode_cat021_datablock',
    'encode_cat021_record',
    # CAT034
    'encode_cat034_north_marker',
    'encode_cat034_sector_crossing',
    # CAT048
    'encode_cat048',
    'encode_cat048_datablock',
    # CAT062
    'encode_cat062',
    'encode_cat062_datablock',
    'encode_cat062_record',
]
__version__ = '0.6.0'

