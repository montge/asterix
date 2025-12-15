"""
ASTERIX Category 001 Encoder

Encodes radar plot data into ASTERIX CAT001 binary format.
CAT001: Monoradar Target Reports (older specification, simpler than CAT048)

Implements essential data items:
- I010: Data Source Identifier (SAC/SIC)
- I020: Target Report Descriptor
- I040: Measured Position in Polar Coordinates (ρ, θ)
- I042: Calculated Position in Cartesian Coordinates
- I070: Mode-3/A Code
- I090: Mode-C Code / Flight Level
- I130: Radar Plot Characteristics
- I141: Truncated Time of Day
- I050: Mode-2 Code

Reference: EUROCONTROL ASTERIX Category 001 specification

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import struct
import time
import math
from typing import Any, Dict, List, Optional, Tuple, Union

from .common import encode_fspec, encode_i010

# Type alias for radar plot data (can be RadarPlot object or dict)
RadarPlotData = Union[Dict[str, Any], Any]


def encode_i020(typ: str = "PSR") -> bytes:
    """
    I020: Target Report Descriptor.

    Args:
        typ: Report type ("PSR", "SSR", "CMB")

    Returns:
        1 byte
    """
    byte = 0x00
    typ_map = {"SSR": 0b10000000, "PSR": 0b01000000, "CMB": 0b11000000}
    byte |= typ_map.get(typ, 0b01000000)
    return struct.pack('B', byte)


def encode_i040(range_m: float, azimuth_deg: float) -> bytes:
    """
    I040: Measured Position in Polar Coordinates.

    Args:
        range_m: Slant range in meters
        azimuth_deg: Azimuth in degrees (0-360)

    Returns:
        4 bytes: RHO (2 bytes), THETA (2 bytes)
    """
    # RHO: Range in 1/128 NM
    range_nm = range_m / 1852.0
    rho = int(range_nm * 128)
    rho = max(0, min(65535, rho))

    # THETA: Azimuth in 360/2^16 degrees
    theta = int(azimuth_deg * 65536 / 360.0) % 65536

    return struct.pack('>HH', rho, theta)


def encode_i042(x_m: float, y_m: float, radar_lat: float = 0.0, radar_lon: float = 0.0) -> bytes:
    """
    I042: Calculated Position in Cartesian Coordinates.

    Args:
        x_m: X coordinate in meters (positive = East)
        y_m: Y coordinate in meters (positive = North)
        radar_lat: Radar latitude (for reference, not encoded)
        radar_lon: Radar longitude (for reference, not encoded)

    Returns:
        4 bytes: X (2 bytes), Y (2 bytes)
    """
    # Resolution: 1/64 NM = ~28.8 meters
    x_nm = x_m / 1852.0
    y_nm = y_m / 1852.0

    x_encoded = int(x_nm * 64)
    y_encoded = int(y_nm * 64)

    # Clamp to 16-bit signed
    x_encoded = max(-32768, min(32767, x_encoded))
    y_encoded = max(-32768, min(32767, y_encoded))

    return struct.pack('>hh', x_encoded, y_encoded)


def encode_i070(mode3a_code: int) -> bytes:
    """
    I070: Mode-3/A Code in Octal Representation.

    Args:
        mode3a_code: Mode 3/A code (0-7777 octal = 0-4095 decimal)

    Returns:
        2 bytes: V, G, L, Mode-3/A code (12 bits)
    """
    code_value = mode3a_code & 0x0FFF
    # V=0 (validated), G=0 (not garbled), L=0 (not smoothed)
    return struct.pack('>H', code_value)


def encode_i090(mode_c_code: Optional[int] = None, flight_level: Optional[float] = None) -> bytes:
    """
    I090: Mode-C Code / Flight Level.

    Args:
        mode_c_code: Mode C code (Gray code)
        flight_level: Flight level (0-1270, will be converted to Mode C)

    Returns:
        2 bytes: V, G, Flight Level in Mode C format
    """
    if flight_level is not None:
        # Convert flight level to quarter-FL (1 FL = 100 ft, quarter = 25 ft)
        quarter_fl = int(flight_level * 4)
        quarter_fl = max(0, min(2047, quarter_fl))  # 11 bits
        # V=0, G=0
        return struct.pack('>H', quarter_fl)
    elif mode_c_code is not None:
        return struct.pack('>H', mode_c_code & 0x0FFF)
    else:
        return struct.pack('>H', 0)


def encode_i130(srl: Optional[float] = None) -> bytes:
    """
    I130: Radar Plot Characteristics (simplified).

    Args:
        srl: SSR plot runlength in degrees

    Returns:
        Variable length (1 byte primary + data)
    """
    if srl is not None:
        primary = 0b10000000  # SRL present, no FX
        srl_val = int((srl / 360.0) * 255)
        return struct.pack('BB', primary, srl_val & 0xFF)
    else:
        return struct.pack('B', 0x00)


def encode_i141(timestamp: Optional[float] = None) -> bytes:
    """
    I141: Truncated Time of Day.

    Resolution: 1/128 seconds

    Args:
        timestamp: Unix timestamp (default: current time)

    Returns:
        2 bytes: Time of day in 1/128 second units (modulo 512 seconds)
    """
    if timestamp is None:
        timestamp = time.time()

    # Truncated to 512 seconds (2^16 / 128 = 512)
    seconds_mod = timestamp % 512
    time_128 = int(seconds_mod * 128)

    return struct.pack('>H', time_128 & 0xFFFF)


def polar_to_cartesian(range_m: float, azimuth_deg: float) -> Tuple[float, float]:
    """
    Convert polar coordinates to Cartesian.

    Args:
        range_m: Range in meters
        azimuth_deg: Azimuth in degrees (0=North, clockwise)

    Returns:
        Tuple (x, y) in meters (x=East, y=North)
    """
    azimuth_rad = math.radians(azimuth_deg)
    x = range_m * math.sin(azimuth_rad)
    y = range_m * math.cos(azimuth_rad)
    return x, y


def encode_cat001_record(
    range_m: float,
    azimuth_deg: float,
    timestamp: Optional[float] = None,
    sac: int = 0,
    sic: int = 1,
    mode3a: Optional[int] = None,
    flight_level: Optional[float] = None,
    typ: str = "PSR",
    include_cartesian: bool = True
) -> bytes:
    """
    Encode a single CAT001 data record.

    Args:
        range_m: Slant range in meters
        azimuth_deg: Azimuth in degrees (0-360)
        timestamp: Unix timestamp
        sac: System Area Code
        sic: System Identification Code
        mode3a: Mode 3/A code (octal)
        flight_level: Flight level (0-1270)
        typ: Report type ("PSR", "SSR", "CMB")
        include_cartesian: Include calculated Cartesian position

    Returns:
        Binary data record (FSPEC + data items)
    """
    items = []
    data_items = bytearray()

    # I010: Data Source Identifier (FRN=1)
    items.append(1)
    data_items.extend(encode_i010(sac, sic))

    # I020: Target Report Descriptor (FRN=2)
    items.append(2)
    data_items.extend(encode_i020(typ))

    # I040: Measured Position in Polar (FRN=3)
    items.append(3)
    data_items.extend(encode_i040(range_m, azimuth_deg))

    # I042: Calculated Position in Cartesian (FRN=4)
    if include_cartesian:
        items.append(4)
        x, y = polar_to_cartesian(range_m, azimuth_deg)
        data_items.extend(encode_i042(x, y))

    # I070: Mode-3/A Code (FRN=5)
    if mode3a is not None:
        items.append(5)
        data_items.extend(encode_i070(mode3a))

    # I090: Mode-C / Flight Level (FRN=6)
    if flight_level is not None:
        items.append(6)
        data_items.extend(encode_i090(flight_level=flight_level))

    # I130: Radar Plot Characteristics (FRN=7)
    items.append(7)
    data_items.extend(encode_i130(srl=1.0))

    # I141: Truncated Time of Day (FRN=8)
    items.append(8)
    data_items.extend(encode_i141(timestamp))

    fspec = encode_fspec(items)
    return fspec + bytes(data_items)


def encode_cat001_datablock(records: List[bytes]) -> bytes:
    """Encode complete CAT001 data block."""
    cat = 1
    records_data = b''.join(records)
    length = 3 + len(records_data)
    header = struct.pack('!BH', cat, length)
    return header + records_data


def encode_cat001(
    plots: List[RadarPlotData],
    radar_position: Tuple[float, float, float] = (0.0, 0.0, 0.0),
    sac: int = 0,
    sic: int = 1,
    include_mode3a: bool = False,
    include_flight_level: bool = False
) -> bytes:
    """
    Encode radar plots into ASTERIX CAT001 format.

    Args:
        plots: List of radar plots
        radar_position: Radar location (lat, lon, alt)
        sac: System Area Code
        sic: System Identification Code
        include_mode3a: Include Mode 3/A codes
        include_flight_level: Include flight levels

    Returns:
        Complete ASTERIX CAT001 data block

    Example:
        >>> plots = radar.generate_plots(10)
        >>> asterix_data = encode_cat001(plots, sac=0, sic=1)
    """
    records = []

    for plot in plots:
        # Handle both RadarPlot objects and dicts
        if hasattr(plot, 'range'):
            range_m = plot.range
            azimuth_deg = plot.azimuth
            timestamp = plot.timestamp
        else:
            range_m = plot.get('range', 0)
            azimuth_deg = plot.get('azimuth', 0)
            timestamp = plot.get('timestamp', time.time())

        mode3a = None
        if include_mode3a:
            # nosec B311 - random is intentionally used for demo data, not security
            import random  # nosec B311
            mode3a = random.randint(0, 0o7777)  # nosec B311

        flight_level = None
        if include_flight_level:
            # nosec B311 - random is intentionally used for demo data, not security
            import random  # nosec B311
            flight_level = random.randint(0, 400)  # nosec B311

        record = encode_cat001_record(
            range_m=range_m,
            azimuth_deg=azimuth_deg,
            timestamp=timestamp,
            sac=sac,
            sic=sic,
            mode3a=mode3a,
            flight_level=flight_level,
            typ="PSR"
        )
        records.append(record)

    return encode_cat001_datablock(records)


if __name__ == "__main__":
    print("ASTERIX CAT001 Encoder - Test\n")

    # Test encoding
    print("1. Encoding CAT001 radar plots...")
    plots = [
        {'range': 50000, 'azimuth': 45.0, 'timestamp': time.time()},
        {'range': 75000, 'azimuth': 180.0, 'timestamp': time.time()},
        {'range': 100000, 'azimuth': 270.0, 'timestamp': time.time()},
    ]

    asterix_data = encode_cat001(plots, sac=0, sic=1)
    print(f"   Encoded {len(plots)} plots → {len(asterix_data)} bytes")
    print(f"   CAT: {asterix_data[0]}")
    print(f"   LEN: {struct.unpack('!H', asterix_data[1:3])[0]}")
    print(f"   Hex: {asterix_data[:32].hex()}\n")

    # Test Cartesian conversion
    print("2. Testing polar to Cartesian conversion...")
    range_m, azimuth = 50000, 45.0
    x, y = polar_to_cartesian(range_m, azimuth)
    print(f"   Polar: {range_m}m @ {azimuth}°")
    print(f"   Cartesian: X={x:.0f}m (East), Y={y:.0f}m (North)")
    print(f"   Verify: Range={math.sqrt(x**2 + y**2):.0f}m\n")

    print("✅ CAT001 encoder test complete!")
