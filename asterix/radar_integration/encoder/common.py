"""
ASTERIX Encoder Common Utilities

Shared encoding functions used across multiple ASTERIX categories.
Consolidates common data item encoders to reduce code duplication.

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-12-14
"""

import struct
import time
from typing import Any, Dict, List, Optional, Union
from dataclasses import dataclass

# Type aliases for common ASTERIX data structures
RadarPlotData = Union[Dict[str, Any], Any]  # Radar plot dict or object
TrackData = Dict[str, Any]  # System track dictionary
TargetData = Dict[str, Any]  # Generic target data dictionary


@dataclass
class ASTERIXDataItem:
    """Represents a single ASTERIX data item."""
    frn: int  # Field Reference Number
    data: bytes  # Binary data


def encode_fspec(items: List[int]) -> bytes:
    """
    Encode Field Specification (FSPEC) for present data items.

    The FSPEC indicates which data items are present in the record using
    a variable-length bit field structure.

    Args:
        items: List of FRN (Field Reference Numbers) present in the record

    Returns:
        FSPEC bytes (variable length, 1-N octets)

    Note:
        FSPEC uses 7 bits per octet for data item presence indication.
        Bit 1 is the extension indicator (1=more octets follow, 0=last octet).
        Bits 8-2 indicate presence of FRNs 1-7, 8-14, 15-21, etc.

    Example:
        >>> encode_fspec([1, 2, 3])  # First 3 data items present
        b'\\xe0'  # Binary: 11100000 - FRN 1,2,3 set, no extension
    """
    max_frn = max(items) if items else 0
    num_octets = (max_frn + 6) // 7  # Calculate required FSPEC octets

    fspec = bytearray(num_octets)

    for frn in items:
        octet_idx = (frn - 1) // 7
        bit_idx = 7 - ((frn - 1) % 7)  # Bit 8-2 (counting from MSB)

        if octet_idx < num_octets:
            fspec[octet_idx] |= (1 << bit_idx)

    # Set extension bits (bit 1) for all but last octet
    for i in range(num_octets - 1):
        fspec[i] |= 0x01  # Set bit 1

    return bytes(fspec)


def encode_i010(sac: int, sic: int) -> bytes:
    """
    I010: Data Source Identifier (SAC/SIC)

    Common to most ASTERIX categories. Identifies the radar/sensor
    system that generated the data.

    Args:
        sac: System Area Code (0-255)
        sic: System Identification Code (0-255)

    Returns:
        2 bytes: SAC, SIC
    """
    return struct.pack('BB', sac, sic)


def encode_time_of_day(timestamp: Optional[float] = None) -> bytes:
    """
    Encode Time of Day (3 bytes, 1/128 second resolution).

    Used by multiple categories:
    - CAT048 I140: Time of Day
    - CAT062 I070: Time of Track Information
    - CAT021 I073: Time of Message Reception - Position

    Args:
        timestamp: Unix timestamp (default: current time)

    Returns:
        3 bytes: Time of day in 1/128 second units since midnight UTC
    """
    if timestamp is None:
        timestamp = time.time()

    # Get seconds since midnight (UTC)
    seconds_since_midnight = timestamp % 86400

    # Convert to 1/128 second units
    time_units = int(seconds_since_midnight * 128)

    # Ensure within valid range (24-bit)
    time_units = time_units & 0xFFFFFF

    return struct.pack('>I', time_units)[1:]  # Take last 3 bytes


def encode_wgs84_position(lat: float, lon: float, high_precision: bool = False) -> bytes:
    """
    Encode WGS-84 latitude/longitude position.

    Used by multiple categories with different precisions:
    - CAT062 I105: 180/2^25 degrees (standard precision, 6 bytes)
    - CAT021 I130: 180/2^23 degrees (high precision, 6 bytes)
    - CAT020 I041: 180/2^25 degrees (standard precision, 8 bytes total)

    Args:
        lat: Latitude in degrees (-90 to +90)
        lon: Longitude in degrees (-180 to +180)
        high_precision: Use 180/2^23 (True) or 180/2^25 (False) resolution

    Returns:
        6 bytes: Latitude (3 bytes), Longitude (3 bytes)
    """
    if high_precision:
        # 180/2^23 degrees resolution (~0.021 meters)
        scale = (2**23) / 180.0
    else:
        # 180/2^25 degrees resolution (~0.67 meters)
        scale = (2**25) / 180.0

    lat_units = int(lat * scale)
    lon_units = int(lon * scale)

    # Convert to 3-byte signed integers (24-bit two's complement)
    lat_bytes = struct.pack('>i', lat_units)[1:]  # Take last 3 bytes
    lon_bytes = struct.pack('>i', lon_units)[1:]

    return lat_bytes + lon_bytes


def encode_aircraft_address(icao_address: int) -> bytes:
    """
    Encode ICAO 24-bit aircraft address.

    Used by:
    - CAT048 I220: Aircraft Address
    - CAT021 I080: Target Address
    - CAT062 I380: Aircraft Derived Data (subfield)

    Args:
        icao_address: 24-bit ICAO Mode-S address (0 to 0xFFFFFF)

    Returns:
        3 bytes: Aircraft address
    """
    return struct.pack('>I', icao_address & 0xFFFFFF)[1:]  # Take last 3 bytes


def encode_callsign(callsign: str) -> bytes:
    """
    Encode aircraft callsign using 6-bit IA5 encoding.

    Used by:
    - CAT048 I240: Aircraft Identification
    - CAT021 I170: Target Identification
    - CAT062 I390/TAG: Callsign

    Args:
        callsign: Aircraft callsign (up to 8 characters)

    Returns:
        6 bytes: Callsign in 6-bit IA5 encoding
    """
    # Pad or truncate to exactly 8 characters
    callsign = callsign.upper().ljust(8)[:8]

    # Encode using 6-bit IA5 (A=1, B=2, ..., Z=26, space=32, 0-9=48-57)
    result = 0
    for char in callsign:
        if 'A' <= char <= 'Z':
            code = ord(char) - ord('A') + 1
        elif '0' <= char <= '9':
            code = ord(char) - ord('0') + 48
        else:
            code = 32  # Space

        result = (result << 6) | (code & 0x3F)

    return struct.pack('>Q', result)[2:]  # Take last 6 bytes


def encode_track_number(track_num: int) -> bytes:
    """
    Encode track number (2 bytes).

    Used by:
    - CAT048 I161: Track Number
    - CAT021 I161: Track Number
    - CAT062 I015: Track Number

    Args:
        track_num: Track number (0-65535)

    Returns:
        2 bytes: Track number
    """
    return struct.pack('>H', track_num & 0xFFFF)


def encode_flight_level(fl: int) -> bytes:
    """
    Encode flight level (2 bytes, 1/4 FL resolution).

    Used by:
    - CAT048 I090: Flight Level
    - CAT021 I145: Flight Level
    - CAT062 I136: Measured Flight Level

    Args:
        fl: Flight level (e.g., 350 for FL350)

    Returns:
        2 bytes: Flight level in 1/4 FL units
    """
    # Convert to 1/4 FL units
    fl_units = int(fl * 4)

    # Handle negative flight levels (two's complement)
    if fl_units < 0:
        fl_units = fl_units & 0xFFFF

    return struct.pack('>h', fl_units)


def encode_cartesian_velocity(vx: float, vy: float) -> bytes:
    """
    Encode Cartesian velocity (4 bytes, 0.25 m/s resolution).

    Used by:
    - CAT062 I185: Calculated Track Velocity
    - CAT020 I500/VX/VY: Track Velocity

    Args:
        vx: X velocity in m/s (positive = East)
        vy: Y velocity in m/s (positive = North)

    Returns:
        4 bytes: Vx (2 bytes), Vy (2 bytes) in 0.25 m/s units
    """
    vx_units = int(vx * 4)  # 0.25 m/s resolution
    vy_units = int(vy * 4)

    return struct.pack('>hh', vx_units, vy_units)
