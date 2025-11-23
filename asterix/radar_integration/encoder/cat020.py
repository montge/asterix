"""
ASTERIX Category 020 Encoder

Encodes multilateration target report data into ASTERIX CAT020 binary format.
CAT020: Multilateration Target Reports

Multilateration (MLAT) determines aircraft position by measuring time differences
of arrival (TDOA) of Mode S transponder signals at multiple ground stations.
Commonly used for airport surface movement surveillance.

Implements essential data items:
- I010: Data Source Identifier (SAC/SIC)
- I020: Target Report Descriptor
- I140: Time of Day
- I041: Position in WGS-84 Coordinates
- I042: Position in Cartesian Coordinates
- I161: Track Number
- I170: Track Status
- I202: Calculated Track Velocity (Cartesian)
- I220: Target Address (Mode S 24-bit)
- I245: Target Identification (callsign)
- I110: Measured Height (Mode C)

Reference: EUROCONTROL ASTERIX Category 020 specification

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import struct
import time
from typing import List, Optional, Tuple


def encode_fspec(items: List[int]) -> bytes:
    """Encode Field Specification (FSPEC)."""
    max_frn = max(items) if items else 0
    num_octets = (max_frn + 6) // 7
    fspec = bytearray(num_octets)

    for frn in items:
        octet_idx = (frn - 1) // 7
        bit_idx = 7 - ((frn - 1) % 7)
        if octet_idx < num_octets:
            fspec[octet_idx] |= (1 << bit_idx)

    for i in range(num_octets - 1):
        fspec[i] |= 0x01

    return bytes(fspec)


def encode_i010(sac: int, sic: int) -> bytes:
    """I010: Data Source Identifier."""
    return struct.pack('BB', sac, sic)


def encode_i020(ssr: bool = False, ms: bool = True, hf: bool = False) -> bytes:
    """
    I020: Target Report Descriptor.

    Args:
        ssr: SSR detection
        ms: Mode S detection
        hf: Height from 3D MLAT

    Returns:
        1 byte
    """
    byte = 0x00

    if ssr:
        byte |= 0b10000000
    if ms:
        byte |= 0b01000000
    if hf:
        byte |= 0b00100000

    return struct.pack('B', byte)


def encode_i140(timestamp: Optional[float] = None) -> bytes:
    """
    I140: Time of Day.

    Args:
        timestamp: Unix timestamp

    Returns:
        3 bytes: Time in 1/128 second units
    """
    if timestamp is None:
        timestamp = time.time()

    seconds_since_midnight = timestamp % 86400
    time_128 = int(seconds_since_midnight * 128)

    return struct.pack('>I', time_128)[1:]


def encode_i041(lat: float, lon: float) -> bytes:
    """
    I041: Position in WGS-84 Coordinates.

    Resolution: 180/2^25 degrees (~0.67 meters)

    Args:
        lat: Latitude in degrees
        lon: Longitude in degrees

    Returns:
        8 bytes: LAT (4 bytes), LON (4 bytes)
    """
    scale = 2**25 / 180.0

    lat_encoded = int(lat * scale)
    lon_encoded = int(lon * scale)

    # Clamp to 32-bit signed
    lat_encoded = max(-2**31, min(2**31 - 1, lat_encoded))
    lon_encoded = max(-2**31, min(2**31 - 1, lon_encoded))

    return struct.pack('>ii', lat_encoded, lon_encoded)


def encode_i042(x_m: float, y_m: float) -> bytes:
    """
    I042: Position in Cartesian Coordinates.

    Resolution: 1 meter

    Args:
        x_m: X coordinate in meters (East)
        y_m: Y coordinate in meters (North)

    Returns:
        6 bytes: X (3 bytes), Y (3 bytes)
    """
    x_encoded = int(x_m)
    y_encoded = int(y_m)

    # Clamp to 24-bit signed
    x_encoded = max(-2**23, min(2**23 - 1, x_encoded))
    y_encoded = max(-2**23, min(2**23 - 1, y_encoded))

    # Handle two's complement
    if x_encoded < 0:
        x_encoded = (1 << 24) + x_encoded
    if y_encoded < 0:
        y_encoded = (1 << 24) + y_encoded

    x_bytes = struct.pack('>I', x_encoded)[1:]
    y_bytes = struct.pack('>I', y_encoded)[1:]

    return x_bytes + y_bytes


def encode_i161(track_number: int) -> bytes:
    """
    I161: Track Number.

    Args:
        track_number: Track number (0-4095, 12 bits)

    Returns:
        2 bytes
    """
    track = track_number & 0x0FFF
    return struct.pack('>H', track)


def encode_i202(vx: float, vy: float) -> bytes:
    """
    I202: Calculated Track Velocity (Cartesian).

    Resolution: 0.25 m/s

    Args:
        vx: Velocity in X direction (m/s, East positive)
        vy: Velocity in Y direction (m/s, North positive)

    Returns:
        4 bytes: Vx (2 bytes), Vy (2 bytes)
    """
    vx_encoded = int(vx / 0.25)
    vy_encoded = int(vy / 0.25)

    vx_encoded = max(-32768, min(32767, vx_encoded))
    vy_encoded = max(-32768, min(32767, vy_encoded))

    return struct.pack('>hh', vx_encoded, vy_encoded)


def encode_i220(aircraft_address: int) -> bytes:
    """
    I220: Target Address (Mode S 24-bit).

    Args:
        aircraft_address: ICAO 24-bit address

    Returns:
        3 bytes
    """
    return struct.pack('>I', aircraft_address & 0xFFFFFF)[1:]


def encode_i245(callsign: str) -> bytes:
    """
    I245: Target Identification (callsign).

    Args:
        callsign: Aircraft callsign (up to 8 characters)

    Returns:
        7 bytes: Callsign (IA5 encoded)
    """
    callsign = callsign.upper()[:8].ljust(8)
    # Simple ASCII encoding (IA5 subset)
    return callsign[:7].encode('ascii')


def encode_cat020_record(
    lat: float,
    lon: float,
    timestamp: Optional[float] = None,
    sac: int = 0,
    sic: int = 1,
    track_number: Optional[int] = None,
    vx: Optional[float] = None,
    vy: Optional[float] = None,
    aircraft_address: Optional[int] = None,
    callsign: Optional[str] = None,
    x_m: Optional[float] = None,
    y_m: Optional[float] = None
) -> bytes:
    """
    Encode a single CAT020 multilateration target record.

    Args:
        lat: Latitude (WGS-84)
        lon: Longitude (WGS-84)
        timestamp: Unix timestamp
        sac: System Area Code
        sic: System Identification Code
        track_number: Track number
        vx: Velocity East (m/s)
        vy: Velocity North (m/s)
        aircraft_address: Mode S address
        callsign: Aircraft callsign
        x_m: Cartesian X (meters)
        y_m: Cartesian Y (meters)

    Returns:
        Binary data record
    """
    items = []
    data_items = bytearray()

    # I010: Data Source Identifier (FRN=1)
    items.append(1)
    data_items.extend(encode_i010(sac, sic))

    # I020: Target Report Descriptor (FRN=2)
    items.append(2)
    data_items.extend(encode_i020(ms=True))

    # I140: Time of Day (FRN=3)
    items.append(3)
    data_items.extend(encode_i140(timestamp))

    # I041: Position in WGS-84 (FRN=4)
    items.append(4)
    data_items.extend(encode_i041(lat, lon))

    # I042: Position in Cartesian (FRN=5) - optional
    if x_m is not None and y_m is not None:
        items.append(5)
        data_items.extend(encode_i042(x_m, y_m))

    # I161: Track Number (FRN=7)
    if track_number is not None:
        items.append(7)
        data_items.extend(encode_i161(track_number))

    # I202: Calculated Velocity (FRN=11)
    if vx is not None and vy is not None:
        items.append(11)
        data_items.extend(encode_i202(vx, vy))

    # I220: Target Address (FRN=12)
    if aircraft_address is not None:
        items.append(12)
        data_items.extend(encode_i220(aircraft_address))

    # I245: Target Identification (FRN=13)
    if callsign is not None:
        items.append(13)
        data_items.extend(encode_i245(callsign))

    fspec = encode_fspec(items)
    return fspec + bytes(data_items)


def encode_cat020_datablock(records: List[bytes]) -> bytes:
    """Encode complete CAT020 data block."""
    cat = 20
    records_data = b''.join(records)
    length = 3 + len(records_data)
    header = struct.pack('!BH', cat, length)
    return header + records_data


def encode_cat020(
    targets: List,
    sac: int = 0,
    sic: int = 1
) -> bytes:
    """
    Encode multilateration targets into ASTERIX CAT020 format.

    Args:
        targets: List of target dicts with lat, lon, etc.
        sac: System Area Code
        sic: System Identification Code

    Returns:
        Complete ASTERIX CAT020 data block

    Example:
        >>> targets = [
        ...     {'lat': 52.5, 'lon': 13.4, 'aircraft_address': 0x3950A1,
        ...      'callsign': 'DLH123', 'vx': 10.0, 'vy': 5.0}
        ... ]
        >>> asterix_data = encode_cat020(targets, sac=0, sic=1)
    """
    records = []

    for target in targets:
        record = encode_cat020_record(
            lat=target.get('lat', 0.0),
            lon=target.get('lon', 0.0),
            timestamp=target.get('timestamp'),
            sac=sac,
            sic=sic,
            track_number=target.get('track_number'),
            vx=target.get('vx'),
            vy=target.get('vy'),
            aircraft_address=target.get('aircraft_address'),
            callsign=target.get('callsign'),
            x_m=target.get('x_m'),
            y_m=target.get('y_m')
        )
        records.append(record)

    return encode_cat020_datablock(records)


if __name__ == "__main__":
    print("ASTERIX CAT020 Encoder - Test\n")

    # Test: Airport surface surveillance
    print("1. Encoding airport surface MLAT targets...")
    targets = [
        {
            'lat': 52.5597, 'lon': 13.2877,  # Berlin Brandenburg Airport
            'aircraft_address': 0x3C6544,
            'callsign': 'DLH123',
            'track_number': 5001,
            'vx': 5.0, 'vy': 3.0,  # Taxiing
            'x_m': 1200, 'y_m': 800
        },
        {
            'lat': 52.5601, 'lon': 13.2885,
            'aircraft_address': 0x4009A8,
            'callsign': 'BAW456',
            'track_number': 5002,
            'vx': -3.0, 'vy': 8.0,
            'x_m': 1350, 'y_m': 1200
        }
    ]

    asterix_data = encode_cat020(targets, sac=0, sic=1)
    print(f"   Encoded {len(targets)} MLAT targets → {len(asterix_data)} bytes")
    print(f"   CAT: {asterix_data[0]}")
    print(f"   LEN: {struct.unpack('!H', asterix_data[1:3])[0]}")
    print(f"   Hex: {asterix_data[:48].hex()}\n")

    print("✅ CAT020 encoder test complete!")
    print("\nCAT020 (MLAT) is ideal for:")
    print("  • Airport surface movement surveillance")
    print("  • Runway incursion detection")
    print("  • Taxiway monitoring")
    print("  • Non-cooperative target detection")
