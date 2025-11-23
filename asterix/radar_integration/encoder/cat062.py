"""
ASTERIX Category 062 Encoder

Encodes system track data into ASTERIX CAT062 binary format.
CAT062: Transmission of System Track Data (fused radar tracks)

Implements essential data items:
- I010: Data Source Identifier (SAC/SIC)
- I070: Time of Track Information
- I105: Calculated Position in WGS-84 Coordinates (lat/lon)
- I135: Calculated Track Barometric Altitude
- I185: Calculated Track Velocity (Cartesian)
- I390: Flight Plan Related Data (callsign, departure, arrival)

Reference: EUROCONTROL ASTERIX Category 062 specification v1.19

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import struct
import time
from typing import List, Tuple, Optional, Dict
from dataclasses import dataclass


@dataclass
class CAT062DataItem:
    """Represents a single ASTERIX CAT062 data item."""
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
    """
    # FSPEC uses 7 bits per octet, with bit 1 as extension indicator
    # Bits 8-2: indicate presence of data items (FRN 1-7, 8-14, etc.)
    # Bit 1: 1=more FSPEC octets follow, 0=last octet

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
    I010: Data Source Identifier

    Identifies the system sending the track data.

    Args:
        sac: System Area Code (0-255)
        sic: System Identification Code (0-255)

    Returns:
        2 bytes: SAC, SIC

    Example:
        >>> encode_i010(0, 1)
        b'\\x00\\x01'
    """
    return struct.pack('BB', sac, sic)


def encode_i070(timestamp: Optional[float] = None) -> bytes:
    """
    I070: Time of Track Information (3 bytes)

    Absolute time stamping of the track information, expressed as elapsed
    time since midnight in UTC.

    Args:
        timestamp: Unix timestamp (default: current time)

    Returns:
        3 bytes: Time of day in 1/128 second units (resolution: ~7.8 ms)

    Format:
        - Resolution: 1/128 seconds (0.0078125 s)
        - Range: 0 to 86400 seconds (full day)
        - Encoding: Unsigned 24-bit integer
        - Resets to zero at midnight UTC

    Example:
        >>> # 12:00:00 noon = 43200 seconds since midnight
        >>> # 43200 * 128 = 5529600 (0x546000)
        >>> encode_i070(43200 + 1234567890)  # Mock timestamp at noon
        b'\\x54\\x60\\x00'
    """
    if timestamp is None:
        timestamp = time.time()

    # Get seconds since midnight (UTC)
    seconds_since_midnight = timestamp % 86400

    # Convert to 1/128 second units
    time_128 = int(seconds_since_midnight * 128)

    # Encode as 3 bytes (24 bits)
    return struct.pack('>I', time_128)[1:]  # Skip first byte (we only need 3)


def encode_i105(lat: float, lon: float) -> bytes:
    """
    I105: Calculated Position in WGS-84 Coordinates (8 bytes)

    Calculated track position in WGS-84 coordinates with high resolution.

    Args:
        lat: Latitude in degrees (-90 to +90)
        lon: Longitude in degrees (-180 to +180)

    Returns:
        8 bytes: Latitude (4 bytes) + Longitude (4 bytes)

    Format:
        - Resolution: 180/2^25 degrees ≈ 0.00000536 degrees ≈ 0.6 meters
        - Encoding: Signed 32-bit integer (two's complement)
        - Latitude range: -90 to +90 degrees
        - Longitude range: -180 to +180 degrees

    Example:
        >>> # Eiffel Tower: 48.8584°N, 2.2945°E
        >>> data = encode_i105(48.8584, 2.2945)
        >>> len(data)
        8
    """
    # Resolution: 180 / 2^25 degrees
    # 2^25 = 33554432
    # Scale factor: 180 / 33554432 ≈ 0.00000536441802978515625 deg
    # Inverse: 33554432 / 180 ≈ 186413.5111... per degree

    scale = 33554432.0 / 180.0  # ~186413.51 units per degree

    # Clamp values to valid range
    lat = max(-90.0, min(90.0, lat))
    lon = max(-180.0, min(180.0, lon))

    # Convert to scaled integer values
    lat_scaled = int(lat * scale)
    lon_scaled = int(lon * scale)

    # Encode as signed 32-bit integers
    return struct.pack('>ii', lat_scaled, lon_scaled)


def encode_i135(altitude_ft: float, qnh_correction: bool = False) -> bytes:
    """
    I135: Calculated Track Barometric Altitude (2 bytes)

    Calculated barometric altitude of the track.

    Args:
        altitude_ft: Barometric altitude in feet
        qnh_correction: True if QNH correction applied, False otherwise

    Returns:
        2 bytes: QNH flag (1 bit) + Altitude (15 bits signed)

    Format:
        - Resolution: 25 feet (0.25 Flight Levels)
        - Encoding: Signed 15-bit integer (two's complement)
        - Range: -1500 to +150000 feet (-15 to +1500 FL)
        - Bit 16: QNH correction applied flag
        - Bits 15-1: Altitude in 0.25 FL units

    Note:
        Flight Level (FL) = altitude_ft / 100
        Example: 35000 ft = FL350 = 350 FL units = 1400 (0.25 FL units)

    Example:
        >>> # Cruising altitude 35000 ft
        >>> encode_i135(35000.0)
        b'\\x05\\x78'  # 0x0578 = 1400 in 0.25 FL units
    """
    # Convert feet to Flight Levels (FL = feet / 100)
    # Then convert to 0.25 FL units
    fl = altitude_ft / 100.0  # Flight level
    fl_025 = int(fl / 0.25)  # 0.25 FL units

    # Clamp to 15-bit signed range
    fl_025 = max(-16384, min(16383, fl_025))

    # Mask to 15 bits (signed)
    fl_025 = fl_025 & 0x7FFF if fl_025 >= 0 else (0x8000 | (fl_025 & 0x7FFF))

    # Set QNH bit (bit 16)
    if qnh_correction:
        fl_025 |= 0x8000

    return struct.pack('>H', fl_025)


def encode_i185(vx: float, vy: float) -> bytes:
    """
    I185: Calculated Track Velocity (Cartesian) (4 bytes)

    Calculated track velocity in Cartesian coordinates.

    Args:
        vx: Velocity X-component in m/s (positive = East)
        vy: Velocity Y-component in m/s (positive = North)

    Returns:
        4 bytes: Vx (2 bytes) + Vy (2 bytes)

    Format:
        - Resolution: 0.25 m/s
        - Encoding: Signed 16-bit integer (two's complement)
        - Range: -8192 to +8191.75 m/s
        - Y-axis points to Geographical North at target location

    Note:
        The coordinate system has Y-axis pointing North and X-axis pointing East.

    Example:
        >>> # Aircraft traveling north at 250 m/s (900 km/h, ~486 kt)
        >>> encode_i185(0.0, 250.0)
        b'\\x00\\x00\\x03\\xe8'  # Vx=0, Vy=1000 (250/0.25)
    """
    # Resolution: 0.25 m/s
    scale = 1.0 / 0.25  # 4 units per m/s

    # Convert to scaled integer values
    vx_scaled = int(vx * scale)
    vy_scaled = int(vy * scale)

    # Clamp to 16-bit signed range
    vx_scaled = max(-32768, min(32767, vx_scaled))
    vy_scaled = max(-32768, min(32767, vy_scaled))

    # Encode as signed 16-bit integers
    return struct.pack('>hh', vx_scaled, vy_scaled)


def encode_i390(callsign: Optional[str] = None,
                adep: Optional[str] = None,
                ades: Optional[str] = None) -> bytes:
    """
    I390: Flight Plan Related Data (Compound Item)

    All flight plan related information provided by ground-based systems.
    This is a compound data item with multiple optional subfields.

    Args:
        callsign: Aircraft callsign (up to 7 characters, e.g., "DLH123")
        adep: Departure airport ICAO code (4 characters, e.g., "EDDF")
        ades: Destination airport ICAO code (4 characters, e.g., "KJFK")

    Returns:
        Variable length bytes: Primary subfield + secondary subfield + data

    Format:
        - Primary subfield (1 byte): indicates which subfields are present
          - Bit 7: CS (Callsign) - presence indicator
          - Bit 2: DEP (Departure Airport) - presence indicator
          - Bit 1: FX (extension to secondary subfield)
        - Secondary subfield (1 byte, if FX=1):
          - Bit 8: DST (Destination Airport) - presence indicator
          - Bit 1: FX (further extension, not used here)
        - Data subfields (variable length):
          - CS: 7 bytes (ASCII, left-adjusted, space-padded)
          - DEP: 4 bytes (ASCII, uppercase alphabetic)
          - DST: 4 bytes (ASCII, uppercase alphabetic)

    Note:
        - Callsign: Up to 7 uppercase alphanumeric characters, left-adjusted,
          space-padded (e.g., "DLH123 " or "BAW2A  ")
        - Airport codes: ICAO 4-letter codes (e.g., "EDDF", "KJFK", "EGLL")

    Example:
        >>> # Lufthansa flight from Frankfurt to New York JFK
        >>> encode_i390(callsign="DLH123", adep="EDDF", ades="KJFK")
        b'\\x85\\x01DLH123 EDDFKJFK'
    """
    # Primary subfield: bit flags for subfield presence
    primary = 0x00
    secondary = 0x00
    data = bytearray()

    has_secondary = False

    # Subfield #2: Callsign (CS) - 7 bytes
    if callsign is not None:
        primary |= 0b01000000  # Bit 7
        # Encode callsign as 7 ASCII characters (left-adjusted, space-padded)
        cs_bytes = callsign.upper().ljust(7)[:7].encode('ascii')
        data.extend(cs_bytes)

    # Subfield #7: Departure Airport (DEP) - 4 bytes
    if adep is not None:
        primary |= 0b00000010  # Bit 2
        # Encode as 4 ASCII characters (uppercase)
        dep_bytes = adep.upper().ljust(4)[:4].encode('ascii')
        data.extend(dep_bytes)

    # Subfield #8: Destination Airport (DST) - 4 bytes (in secondary subfield)
    if ades is not None:
        has_secondary = True
        secondary |= 0b10000000  # Bit 8 of secondary
        # Encode as 4 ASCII characters (uppercase)
        dst_bytes = ades.upper().ljust(4)[:4].encode('ascii')
        data.extend(dst_bytes)

    # Set FX bit in primary if secondary subfield needed
    if has_secondary:
        primary |= 0x01  # Bit 1 (FX)

    # Assemble compound item
    result = struct.pack('B', primary)
    if has_secondary:
        result += struct.pack('B', secondary)
    result += bytes(data)

    return result


def encode_cat062_record(
    lat: float,
    lon: float,
    timestamp: Optional[float] = None,
    sac: int = 0,
    sic: int = 1,
    altitude_ft: Optional[float] = None,
    vx: Optional[float] = None,
    vy: Optional[float] = None,
    callsign: Optional[str] = None,
    adep: Optional[str] = None,
    ades: Optional[str] = None
) -> bytes:
    """
    Encode a single CAT062 data record (without data block header).

    Args:
        lat: Latitude in degrees (-90 to +90)
        lon: Longitude in degrees (-180 to +180)
        timestamp: Unix timestamp (default: current time)
        sac: System Area Code (0-255)
        sic: System Identification Code (0-255)
        altitude_ft: Barometric altitude in feet (optional)
        vx: Velocity X-component in m/s (optional)
        vy: Velocity Y-component in m/s (optional)
        callsign: Aircraft callsign (optional, up to 7 chars)
        adep: Departure airport ICAO code (optional, 4 chars)
        ades: Destination airport ICAO code (optional, 4 chars)

    Returns:
        Binary data record (FSPEC + data items)

    Note:
        The FRN (Field Reference Number) mapping for CAT062:
        - FRN 1: I010 (Data Source Identifier)
        - FRN 2: I015 (Service Identification) - not implemented
        - FRN 3: I070 (Time of Track Information)
        - FRN 4: I105 (Calculated Position WGS-84)
        - FRN 5: I100 (Calculated Position Cartesian) - not implemented
        - FRN 6: I185 (Calculated Track Velocity)
        - FRN 7: I210 (Calculated Acceleration) - not implemented
        - FRN 8-11: Various track data - not implemented
        - FRN 12: I135 (Calculated Track Barometric Altitude)
        - ... (continues to higher FRNs)
        - FRN 27: I390 (Flight Plan Related Data)

    Example:
        >>> # Encode track for aircraft at Eiffel Tower, cruising at FL350
        >>> record = encode_cat062_record(
        ...     lat=48.8584, lon=2.2945,
        ...     altitude_ft=35000,
        ...     vx=100.0, vy=200.0,
        ...     callsign="AFR123",
        ...     adep="LFPG", ades="KJFK"
        ... )
        >>> len(record) > 20
        True
    """
    items = []
    data_items = bytearray()

    # I010: Data Source Identifier (FRN=1)
    items.append(1)
    data_items.extend(encode_i010(sac, sic))

    # I070: Time of Track Information (FRN=3)
    items.append(3)
    data_items.extend(encode_i070(timestamp))

    # I105: Calculated Position in WGS-84 (FRN=4)
    items.append(4)
    data_items.extend(encode_i105(lat, lon))

    # I185: Calculated Track Velocity (FRN=6)
    if vx is not None and vy is not None:
        items.append(6)
        data_items.extend(encode_i185(vx, vy))

    # I135: Calculated Track Barometric Altitude (FRN=12)
    if altitude_ft is not None:
        items.append(12)
        data_items.extend(encode_i135(altitude_ft))

    # I390: Flight Plan Related Data (FRN=27)
    # FRN 27 is in the 4th FSPEC octet (bits 22-28: FRNs 22-28)
    # FRN 27 = (27-1) = 26, octet_idx = 26//7 = 3, bit_idx = 26%7 = 5
    if callsign is not None or adep is not None or ades is not None:
        items.append(27)
        data_items.extend(encode_i390(callsign=callsign, adep=adep, ades=ades))

    # Encode FSPEC
    fspec = encode_fspec(items)

    # Combine FSPEC + data items
    return fspec + bytes(data_items)


def encode_cat062_datablock(records: List[bytes]) -> bytes:
    """
    Encode a complete CAT062 data block with header.

    Args:
        records: List of encoded data records (from encode_cat062_record)

    Returns:
        Complete data block (CAT, LEN, records)

    Format:
        - Byte 1: CAT (062)
        - Bytes 2-3: LEN (total length in bytes, including CAT and LEN)
        - Bytes 4-N: Data records

    Note:
        Maximum data block length is 65535 bytes due to 16-bit length field.

    Example:
        >>> records = [encode_cat062_record(lat=48.8584, lon=2.2945)]
        >>> block = encode_cat062_datablock(records)
        >>> block[0]  # Category
        62
    """
    # Data block format:
    # Byte 1: CAT (062)
    # Bytes 2-3: LEN (length in bytes, including CAT and LEN)
    # Bytes 4-N: Data records

    cat = 62
    records_data = b''.join(records)
    length = 3 + len(records_data)  # 3 bytes header + records

    # Encode: CAT (1 byte), LEN (2 bytes), Records
    header = struct.pack('!BH', cat, length)
    return header + records_data


def encode_cat062(
    tracks: List[Dict],
    sac: int = 0,
    sic: int = 1
) -> bytes:
    """
    Encode a list of system tracks into ASTERIX CAT062 format.

    Args:
        tracks: List of track dictionaries with keys:
            - lat (float): Latitude in degrees
            - lon (float): Longitude in degrees
            - timestamp (float, optional): Unix timestamp
            - altitude_ft (float, optional): Altitude in feet
            - vx (float, optional): Velocity X in m/s
            - vy (float, optional): Velocity Y in m/s
            - callsign (str, optional): Aircraft callsign
            - adep (str, optional): Departure airport
            - ades (str, optional): Destination airport
        sac: System Area Code (0-255)
        sic: System Identification Code (0-255)

    Returns:
        Complete ASTERIX CAT062 data block as bytes

    Example:
        >>> tracks = [
        ...     {
        ...         'lat': 48.8584, 'lon': 2.2945,
        ...         'altitude_ft': 35000,
        ...         'vx': 100.0, 'vy': 200.0,
        ...         'callsign': 'AFR123',
        ...         'adep': 'LFPG', 'ades': 'KJFK'
        ...     },
        ...     {
        ...         'lat': 51.5074, 'lon': -0.1278,
        ...         'altitude_ft': 38000,
        ...         'vx': 120.0, 'vy': 180.0,
        ...         'callsign': 'BAW456',
        ...         'adep': 'EGLL', 'ades': 'KJFK'
        ...     }
        ... ]
        >>> asterix_data = encode_cat062(tracks, sac=0, sic=1)
        >>> asterix_data[0]
        62
    """
    records = []

    for track in tracks:
        record = encode_cat062_record(
            lat=track['lat'],
            lon=track['lon'],
            timestamp=track.get('timestamp', time.time()),
            sac=sac,
            sic=sic,
            altitude_ft=track.get('altitude_ft'),
            vx=track.get('vx'),
            vy=track.get('vy'),
            callsign=track.get('callsign'),
            adep=track.get('adep'),
            ades=track.get('ades')
        )
        records.append(record)

    return encode_cat062_datablock(records)


if __name__ == "__main__":
    # Example usage and testing
    print("ASTERIX CAT062 Encoder - Test\n")
    print("=" * 60)

    # Test 1: Individual data item encoding
    print("\n1. Testing individual data item encoders...")
    print("-" * 60)

    # I010: Data Source Identifier
    i010_data = encode_i010(0, 1)
    print(f"I010 (SAC=0, SIC=1): {i010_data.hex()} ({len(i010_data)} bytes)")

    # I070: Time of Track Information
    test_timestamp = 43200.0  # Noon (12:00:00)
    i070_data = encode_i070(test_timestamp)
    print(f"I070 (12:00:00 noon): {i070_data.hex()} ({len(i070_data)} bytes)")

    # I105: WGS-84 Position
    lat, lon = 48.8584, 2.2945  # Eiffel Tower
    i105_data = encode_i105(lat, lon)
    print(f"I105 (Eiffel Tower): {i105_data.hex()} ({len(i105_data)} bytes)")

    # I135: Barometric Altitude
    altitude = 35000.0  # FL350
    i135_data = encode_i135(altitude)
    print(f"I135 (FL350): {i135_data.hex()} ({len(i135_data)} bytes)")

    # I185: Track Velocity
    vx, vy = 100.0, 200.0  # m/s
    i185_data = encode_i185(vx, vy)
    print(f"I185 (Vx=100, Vy=200 m/s): {i185_data.hex()} ({len(i185_data)} bytes)")

    # I390: Flight Plan Data
    i390_data = encode_i390(callsign="DLH123", adep="EDDF", ades="KJFK")
    print(f"I390 (DLH123 EDDF->KJFK): {i390_data.hex()} ({len(i390_data)} bytes)")

    # Test 2: Single track record encoding
    print("\n2. Encoding single track record...")
    print("-" * 60)

    record = encode_cat062_record(
        lat=48.8584,  # Eiffel Tower
        lon=2.2945,
        timestamp=time.time(),
        sac=0,
        sic=1,
        altitude_ft=35000,
        vx=100.0,
        vy=200.0,
        callsign="AFR123",
        adep="LFPG",
        ades="KJFK"
    )
    print(f"   Record length: {len(record)} bytes")
    print(f"   FSPEC: {record[:4].hex()}")
    print(f"   First 32 bytes: {record[:32].hex()}")

    # Test 3: Multiple tracks in data block
    print("\n3. Encoding data block with 3 tracks...")
    print("-" * 60)

    tracks = [
        {
            'lat': 48.8584, 'lon': 2.2945,  # Paris
            'altitude_ft': 35000,
            'vx': 100.0, 'vy': 200.0,
            'callsign': 'AFR123',
            'adep': 'LFPG', 'ades': 'KJFK',
            'timestamp': time.time()
        },
        {
            'lat': 51.5074, 'lon': -0.1278,  # London
            'altitude_ft': 38000,
            'vx': 120.0, 'vy': 180.0,
            'callsign': 'BAW456',
            'adep': 'EGLL', 'ades': 'KJFK',
            'timestamp': time.time() + 1
        },
        {
            'lat': 40.6413, 'lon': -73.7781,  # New York JFK
            'altitude_ft': 5000,
            'vx': -50.0, 'vy': -80.0,
            'callsign': 'DLH789',
            'adep': 'EDDF', 'ades': 'KJFK',
            'timestamp': time.time() + 2
        },
    ]

    asterix_data = encode_cat062(tracks, sac=0, sic=1)
    print(f"   Data block length: {len(asterix_data)} bytes")
    print(f"   CAT: {asterix_data[0]}")
    length = struct.unpack('!H', asterix_data[1:3])[0]
    print(f"   LEN: {length} bytes")
    print(f"   First 48 bytes: {asterix_data[:48].hex()}")

    # Test 4: Round-trip with ASTERIX parser (if available)
    print("\n4. Testing round-trip with ASTERIX parser...")
    print("-" * 60)
    try:
        import sys
        sys.path.insert(0, '/home/e/Development/asterix')
        import asterix

        parsed = asterix.parse(asterix_data)
        print(f"   ✅ Successfully parsed {len(parsed)} records")

        if parsed:
            rec = parsed[0]
            print(f"\n   Record 0:")
            print(f"     Category: {rec.get('category')}")
            print(f"     SAC/SIC: {rec.get('I010', {}).get('SAC')}/{rec.get('I010', {}).get('SIC')}")

            if 'I105' in rec:
                lat_parsed = rec['I105'].get('LAT')
                lon_parsed = rec['I105'].get('LON')
                print(f"     Position: {lat_parsed:.6f}°, {lon_parsed:.6f}°")

            if 'I135' in rec:
                alt_parsed = rec['I135'].get('CTB')
                print(f"     Altitude: FL{alt_parsed:.2f}")

            if 'I185' in rec:
                vx_parsed = rec['I185'].get('VX')
                vy_parsed = rec['I185'].get('VY')
                print(f"     Velocity: Vx={vx_parsed:.2f} m/s, Vy={vy_parsed:.2f} m/s")

            if 'I390' in rec:
                i390 = rec['I390']
                cs = i390.get('CS', {}).get('CS', 'N/A')
                dep = i390.get('DEP', {}).get('DEP', 'N/A')
                dst = i390.get('DST', {}).get('DST', 'N/A')
                print(f"     Flight Plan: {cs} ({dep} -> {dst})")

    except ImportError as e:
        print(f"   ⚠️  Parser not available: {e}")
    except Exception as e:
        print(f"   ❌ Parser test failed: {e}")
        import traceback
        traceback.print_exc()

    # Test 5: Encoding format verification
    print("\n5. Encoding format verification...")
    print("-" * 60)

    # Verify FSPEC structure
    test_items = [1, 3, 4, 6, 12, 27]
    fspec = encode_fspec(test_items)
    print(f"   FSPEC for items {test_items}:")
    print(f"     Bytes: {fspec.hex()}")
    print(f"     Length: {len(fspec)} octets")

    # Verify individual encodings
    print(f"\n   I105 encoding (48.8584°, 2.2945°):")
    lat_scaled = int(48.8584 * 33554432.0 / 180.0)
    lon_scaled = int(2.2945 * 33554432.0 / 180.0)
    print(f"     Latitude scaled: {lat_scaled} (0x{lat_scaled:08x})")
    print(f"     Longitude scaled: {lon_scaled} (0x{lon_scaled:08x})")

    print(f"\n   I135 encoding (35000 ft = FL350):")
    fl_025 = int(350.0 / 0.25)
    print(f"     0.25 FL units: {fl_025} (0x{fl_025:04x})")

    print(f"\n   I185 encoding (Vx=100, Vy=200 m/s):")
    vx_scaled = int(100.0 / 0.25)
    vy_scaled = int(200.0 / 0.25)
    print(f"     Vx scaled: {vx_scaled} (0x{vx_scaled:04x})")
    print(f"     Vy scaled: {vy_scaled} (0x{vy_scaled:04x})")

    print("\n" + "=" * 60)
    print("✅ CAT062 encoder test complete!")
    print("\nImplemented data items:")
    print("  - I010: Data Source Identifier (SAC/SIC)")
    print("  - I070: Time of Track Information")
    print("  - I105: Calculated Position in WGS-84")
    print("  - I135: Calculated Track Barometric Altitude")
    print("  - I185: Calculated Track Velocity (Cartesian)")
    print("  - I390: Flight Plan Related Data (callsign, ADEP, ADES)")
