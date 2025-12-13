"""
ASTERIX Category 021 Encoder

Encodes ADS-B target report data into ASTERIX CAT021 binary format.
CAT021: ADS-B Target Reports (Automatic Dependent Surveillance-Broadcast)

Implements essential data items:
- I010: Data Source Identifier (SAC/SIC)
- I040: Target Report Descriptor
- I161: Track Number
- I015: Service Identification
- I071: Time of Applicability for Position
- I130: Position in WGS-84 Coordinates (high precision)
- I080: Target Address (ICAO 24-bit address)
- I170: Target Identification (callsign)
- I145: Flight Level
- I140: Geometric Height (GNSS altitude)
- I200: Target Status
- I155: Barometric Vertical Rate
- I150: Air Speed
- I151: True Airspeed
- I152: Magnetic Heading

Reference: EUROCONTROL ASTERIX Category 021 specification v2.6

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import struct
import time
from typing import List, Optional, Dict


def encode_fspec(items: List[int]) -> bytes:
    """
    Encode Field Specification (FSPEC) for present data items.

    Args:
        items: List of FRN (Field Reference Numbers) present

    Returns:
        FSPEC bytes (variable length, 1-N octets)
    """
    max_frn = max(items) if items else 0
    num_octets = (max_frn + 6) // 7

    fspec = bytearray(num_octets)

    for frn in items:
        octet_idx = (frn - 1) // 7
        bit_idx = 7 - ((frn - 1) % 7)

        if octet_idx < num_octets:
            fspec[octet_idx] |= (1 << bit_idx)

    # Set extension bits for all but last octet
    for i in range(num_octets - 1):
        fspec[i] |= 0x01

    return bytes(fspec)


def encode_i010(sac: int, sic: int) -> bytes:
    """
    I010: Data Source Identifier

    Args:
        sac: System Area Code (0-255)
        sic: System Identification Code (0-255)

    Returns:
        2 bytes: SAC, SIC
    """
    return struct.pack('BB', sac, sic)


def encode_i040(atp: int = 0, arc: int = 0, rc: int = 0) -> bytes:
    """
    I040: Target Report Descriptor

    Args:
        atp: Address Type (0=24-bit, 1=duplicate, 2=surface, 3=anonymous)
        arc: Altitude Reporting Capability (0=100ft, 1=25ft)
        rc: Range Check (0=default, 1=GNSS)

    Returns:
        1 byte (extensible, using 1 byte for simplicity)
    """
    byte = 0x00

    # Bits 6-5: ATP
    byte |= ((atp & 0x03) << 5)

    # Bit 4: ARC
    if arc:
        byte |= 0b00010000

    # Bit 3: RC
    if rc:
        byte |= 0b00001000

    # Bit 1: FX (0=no extension)

    return struct.pack('B', byte)


def encode_i161(track_number: int) -> bytes:
    """
    I161: Track Number (unique identifier for this track)

    Args:
        track_number: Track number (0-4095, 12 bits)

    Returns:
        2 bytes: Track number (12 bits) + spare (4 bits)
    """
    track = track_number & 0x0FFF
    return struct.pack('>H', track << 4)  # Left-align in 16 bits


def encode_i015(service_id: int = 0) -> bytes:
    """
    I015: Service Identification

    Args:
        service_id: Service identification (0-255)

    Returns:
        1 byte: Service ID
    """
    return struct.pack('B', service_id)


def encode_i071(timestamp: Optional[float] = None) -> bytes:
    """
    I071: Time of Applicability for Position

    Args:
        timestamp: Unix timestamp (default: current time)

    Returns:
        3 bytes: Time in 1/128 second units since midnight
    """
    if timestamp is None:
        timestamp = time.time()

    seconds_since_midnight = timestamp % 86400
    time_128 = int(seconds_since_midnight * 128)

    return struct.pack('>I', time_128)[1:]


def encode_i130(lat: float, lon: float) -> bytes:
    """
    I130: Position in WGS-84 Coordinates (High Resolution)

    Resolution: 180/2^23 degrees = ~0.021 meters at equator

    Args:
        lat: Latitude in degrees (-90 to +90)
        lon: Longitude in degrees (-180 to +180)

    Returns:
        6 bytes: LAT (3 bytes), LON (3 bytes)
    """
    # Resolution: 180 / 2^23 degrees
    scale = 2**23 / 180.0

    # Convert to encoded values (signed 24-bit)
    lat_encoded = int(lat * scale)
    lon_encoded = int(lon * scale)

    # Clamp to 24-bit signed range
    lat_encoded = max(-2**23, min(2**23 - 1, lat_encoded))
    lon_encoded = max(-2**23, min(2**23 - 1, lon_encoded))

    # Handle two's complement for negative values
    if lat_encoded < 0:
        lat_encoded = (1 << 24) + lat_encoded
    if lon_encoded < 0:
        lon_encoded = (1 << 24) + lon_encoded

    # Pack as 3 bytes each
    lat_bytes = struct.pack('>I', lat_encoded)[1:]
    lon_bytes = struct.pack('>I', lon_encoded)[1:]

    return lat_bytes + lon_bytes


def encode_i080(aircraft_address: int) -> bytes:
    """
    I080: Target Address (ICAO 24-bit address)

    Args:
        aircraft_address: 24-bit Mode S address (0x000000-0xFFFFFF)

    Returns:
        3 bytes: Aircraft address
    """
    return struct.pack('>I', aircraft_address & 0xFFFFFF)[1:]


def encode_i170(callsign: str) -> bytes:
    """
    I170: Target Identification (Aircraft Callsign)

    Args:
        callsign: Aircraft callsign (up to 8 characters, IA5 alphabet)

    Returns:
        6 bytes: Callsign in IA5 encoding (left-adjusted, space-padded)
    """
    # Ensure uppercase and pad to 8 characters
    callsign = callsign.upper()[:8].ljust(8)

    # Encode as IA5 (6-bit characters packed into bytes)
    # For simplicity, use ASCII encoding (IA5 subset)
    # Real IA5: maps A-Z to 0x01-0x1A, space to 0x20
    encoded = bytearray(6)

    for i, char in enumerate(callsign):
        if i >= 8:
            break
        # Simple IA5 encoding
        if char == ' ':
            val = 0x20
        elif 'A' <= char <= 'Z':
            val = ord(char) - ord('A') + 1
        elif '0' <= char <= '9':
            val = ord(char) - ord('0') + 48
        else:
            val = 0x20

        # Pack 8 characters into 6 bytes (6-bit encoding)
        # Character positions: 0-7, packed as 8*6=48 bits = 6 bytes
        byte_idx = (i * 6) // 8
        bit_offset = (i * 6) % 8

        if byte_idx < 6:
            encoded[byte_idx] |= (val >> bit_offset) & 0xFF
            if byte_idx + 1 < 6 and bit_offset > 2:
                encoded[byte_idx + 1] |= (val << (8 - bit_offset)) & 0xFF

    return bytes(encoded)


def encode_i145(flight_level: float) -> bytes:
    """
    I145: Flight Level (barometric altitude)

    Resolution: 0.25 FL (25 feet)

    Args:
        flight_level: Flight level (0-1500, i.e., FL000-FL1500)

    Returns:
        2 bytes: Flight level in 0.25 FL units
    """
    fl_encoded = int(flight_level * 4)  # 0.25 FL resolution
    fl_encoded = max(0, min(65535, fl_encoded))
    return struct.pack('>H', fl_encoded)


def encode_i140(height_ft: float) -> bytes:
    """
    I140: Geometric Height (GNSS altitude)

    Resolution: 6.25 feet

    Args:
        height_ft: Geometric height in feet (-1500 to +150000 ft)

    Returns:
        2 bytes: Height in 6.25 ft units (signed)
    """
    height_encoded = int(height_ft / 6.25)

    # Clamp to 16-bit signed range
    height_encoded = max(-32768, min(32767, height_encoded))

    return struct.pack('>h', height_encoded)


def encode_i200(icf: int = 0, lnav: int = 0, ps: int = 0, ss: int = 0) -> bytes:
    """
    I200: Target Status

    Args:
        icf: Intent Change Flag (0=no change, 1=change)
        lnav: LNAV mode (0=not active, 1=active)
        ps: Priority Status (0-7)
        ss: Surveillance Status (0-15)

    Returns:
        1 byte: Target status flags
    """
    byte = 0x00

    # Bit 8: ICF
    if icf:
        byte |= 0b10000000

    # Bit 7: LNAV
    if lnav:
        byte |= 0b01000000

    # Bits 6-4: PS (3 bits)
    byte |= ((ps & 0x07) << 3)

    # Bits 3-0: SS (4 bits) - but bit 1 is FX, so only 3 bits
    # For simplicity, omit extension

    return struct.pack('B', byte)


def encode_i155(vrate_fpm: float) -> bytes:
    """
    I155: Barometric Vertical Rate

    Resolution: 6.25 ft/min

    Args:
        vrate_fpm: Vertical rate in feet/minute (-32768 to +32767 fpm)

    Returns:
        2 bytes: Vertical rate in 6.25 ft/min units (signed)
    """
    vrate_encoded = int(vrate_fpm / 6.25)
    vrate_encoded = max(-32768, min(32767, vrate_encoded))

    return struct.pack('>h', vrate_encoded)


def encode_i150(airspeed_kt: float) -> bytes:
    """
    I150: Air Speed (IAS or Mach)

    Resolution: 2^-14 NM/s = ~0.001 kt

    Args:
        airspeed_kt: Air speed in knots (0-1500 kt typical)

    Returns:
        2 bytes: Air speed in 2^-14 NM/s units
    """
    # Convert knots to NM/s: 1 kt = 1/3600 NM/s
    speed_nms = airspeed_kt / 3600.0

    # Scale: 2^-14 NM/s units
    speed_encoded = int(speed_nms * (2**14))
    speed_encoded = max(0, min(65535, speed_encoded))

    return struct.pack('>H', speed_encoded)


def encode_i151(true_airspeed_kt: float) -> bytes:
    """
    I151: True Airspeed

    Resolution: 1 knot

    Args:
        true_airspeed_kt: True airspeed in knots

    Returns:
        2 bytes: True airspeed (bit 16: RE, bits 15-1: TAS)
    """
    tas = int(true_airspeed_kt)
    tas = max(0, min(32767, tas))  # 15 bits

    # Bit 16: RE (Range Exceeded), bits 15-1: TAS
    return struct.pack('>H', tas << 1)


def encode_i152(magnetic_heading_deg: float) -> bytes:
    """
    I152: Magnetic Heading

    Resolution: 360/2^16 degrees = ~0.0055°

    Args:
        magnetic_heading_deg: Magnetic heading in degrees (0-360)

    Returns:
        2 bytes: Magnetic heading
    """
    heading = magnetic_heading_deg % 360.0
    heading_encoded = int(heading * 65536 / 360.0)
    heading_encoded = heading_encoded % 65536

    return struct.pack('>H', heading_encoded)


def encode_cat021_record(
    lat: float,
    lon: float,
    aircraft_address: int,
    timestamp: Optional[float] = None,
    sac: int = 0,
    sic: int = 1,
    track_number: Optional[int] = None,
    callsign: Optional[str] = None,
    flight_level: Optional[float] = None,
    gnss_height_ft: Optional[float] = None,
    airspeed_kt: Optional[float] = None,
    true_airspeed_kt: Optional[float] = None,
    magnetic_heading_deg: Optional[float] = None,
    vertical_rate_fpm: Optional[float] = None,
    service_id: int = 0
) -> bytes:
    """
    Encode a single CAT021 ADS-B data record.

    Args:
        lat: Latitude in degrees (WGS-84)
        lon: Longitude in degrees (WGS-84)
        aircraft_address: ICAO 24-bit Mode S address
        timestamp: Unix timestamp (default: current time)
        sac: System Area Code
        sic: System Identification Code
        track_number: Track number (0-4095)
        callsign: Aircraft callsign (up to 8 chars)
        flight_level: Flight level (barometric altitude)
        gnss_height_ft: GNSS geometric height in feet
        airspeed_kt: Indicated air speed in knots
        true_airspeed_kt: True airspeed in knots
        magnetic_heading_deg: Magnetic heading in degrees
        vertical_rate_fpm: Barometric vertical rate in ft/min
        service_id: Service identification

    Returns:
        Binary data record (FSPEC + data items)
    """
    items = []
    data_items = bytearray()

    # I010: Data Source Identifier (FRN=1)
    items.append(1)
    data_items.extend(encode_i010(sac, sic))

    # I040: Target Report Descriptor (FRN=2)
    items.append(2)
    data_items.extend(encode_i040())

    # I161: Track Number (FRN=3)
    if track_number is not None:
        items.append(3)
        data_items.extend(encode_i161(track_number))

    # I015: Service Identification (FRN=4)
    items.append(4)
    data_items.extend(encode_i015(service_id))

    # I071: Time of Applicability (FRN=5)
    items.append(5)
    data_items.extend(encode_i071(timestamp))

    # I130: Position in WGS-84 (FRN=6)
    items.append(6)
    data_items.extend(encode_i130(lat, lon))

    # I080: Target Address (FRN=7)
    items.append(7)
    data_items.extend(encode_i080(aircraft_address))

    # I170: Target Identification (FRN=11)
    if callsign is not None:
        items.append(11)
        data_items.extend(encode_i170(callsign))

    # I145: Flight Level (FRN=13)
    if flight_level is not None:
        items.append(13)
        data_items.extend(encode_i145(flight_level))

    # I140: Geometric Height (FRN=14)
    if gnss_height_ft is not None:
        items.append(14)
        data_items.extend(encode_i140(gnss_height_ft))

    # I155: Barometric Vertical Rate (FRN=18)
    if vertical_rate_fpm is not None:
        items.append(18)
        data_items.extend(encode_i155(vertical_rate_fpm))

    # I150: Air Speed (FRN=19)
    if airspeed_kt is not None:
        items.append(19)
        data_items.extend(encode_i150(airspeed_kt))

    # I151: True Airspeed (FRN=20)
    if true_airspeed_kt is not None:
        items.append(20)
        data_items.extend(encode_i151(true_airspeed_kt))

    # I152: Magnetic Heading (FRN=21)
    if magnetic_heading_deg is not None:
        items.append(21)
        data_items.extend(encode_i152(magnetic_heading_deg))

    # I200: Target Status (FRN=22)
    items.append(22)
    data_items.extend(encode_i200())

    # Encode FSPEC
    fspec = encode_fspec(items)

    return fspec + bytes(data_items)


def encode_cat021_datablock(records: List[bytes]) -> bytes:
    """
    Encode a complete CAT021 data block with header.

    Args:
        records: List of encoded data records

    Returns:
        Complete data block (CAT, LEN, records)
    """
    cat = 21
    records_data = b''.join(records)
    length = 3 + len(records_data)

    header = struct.pack('!BH', cat, length)
    return header + records_data


def encode_cat021(
    adsb_reports: List[Dict],
    sac: int = 0,
    sic: int = 1,
    service_id: int = 0
) -> bytes:
    """
    Encode a list of ADS-B reports into ASTERIX CAT021 format.

    Args:
        adsb_reports: List of ADS-B reports (dicts with lat, lon, aircraft_address, etc.)
        sac: System Area Code
        sic: System Identification Code
        service_id: Service identification

    Returns:
        Complete ASTERIX CAT021 data block as bytes

    Example:
        >>> reports = [
        ...     {
        ...         'lat': 48.8584, 'lon': 2.2945,
        ...         'aircraft_address': 0x3950A1,
        ...         'callsign': 'AFR123',
        ...         'flight_level': 350,
        ...         'gnss_height_ft': 35000,
        ...         'airspeed_kt': 450,
        ...         'track_number': 1234,
        ...         'timestamp': time.time()
        ...     }
        ... ]
        >>> asterix_data = encode_cat021(reports, sac=0, sic=1)
    """
    records = []

    for i, report in enumerate(adsb_reports):
        record = encode_cat021_record(
            lat=report.get('lat', 0.0),
            lon=report.get('lon', 0.0),
            aircraft_address=report.get('aircraft_address', 0),
            timestamp=report.get('timestamp'),
            sac=sac,
            sic=sic,
            track_number=report.get('track_number'),
            callsign=report.get('callsign'),
            flight_level=report.get('flight_level'),
            gnss_height_ft=report.get('gnss_height_ft'),
            airspeed_kt=report.get('airspeed_kt'),
            true_airspeed_kt=report.get('true_airspeed_kt'),
            magnetic_heading_deg=report.get('magnetic_heading_deg'),
            vertical_rate_fpm=report.get('vertical_rate_fpm'),
            service_id=service_id
        )
        records.append(record)

    return encode_cat021_datablock(records)


if __name__ == "__main__":
    # Test the encoder
    print("ASTERIX CAT021 Encoder - Test\n")

    # Test 1: Single ADS-B report
    print("1. Encoding single ADS-B report...")
    record = encode_cat021_record(
        lat=48.8584,
        lon=2.2945,
        aircraft_address=0x3950A1,
        callsign="AFR123",
        flight_level=350,
        gnss_height_ft=35000,
        airspeed_kt=450,
        track_number=1234,
        sac=0,
        sic=1
    )
    print(f"   Record length: {len(record)} bytes")
    print(f"   Hex: {record[:32].hex()}...\n")

    # Test 2: Multiple ADS-B reports
    print("2. Encoding data block with 3 ADS-B reports...")
    reports = [
        {
            'lat': 48.8584, 'lon': 2.2945,
            'aircraft_address': 0x3950A1,
            'callsign': 'AFR123',
            'flight_level': 350,
            'track_number': 1001
        },
        {
            'lat': 51.5074, 'lon': -0.1278,
            'aircraft_address': 0x4009A8,
            'callsign': 'BAW456',
            'flight_level': 370,
            'track_number': 1002
        },
        {
            'lat': 40.7128, 'lon': -74.0060,
            'aircraft_address': 0xA12345,
            'callsign': 'UAL789',
            'flight_level': 390,
            'track_number': 1003
        }
    ]

    asterix_data = encode_cat021(reports, sac=0, sic=1)
    print(f"   Data block length: {len(asterix_data)} bytes")
    print(f"   CAT: {asterix_data[0]}")
    length = struct.unpack('!H', asterix_data[1:3])[0]
    print(f"   LEN: {length} bytes")
    print(f"   Hex (first 48 bytes): {asterix_data[:48].hex()}\n")

    # Test 3: High-precision position encoding
    print("3. Testing high-precision WGS-84 encoding...")
    lat_test = 48.858844  # Eiffel Tower
    lon_test = 2.294351
    pos_data = encode_i130(lat_test, lon_test)
    print(f"   Input: {lat_test:.6f}°N, {lon_test:.6f}°E")
    print(f"   Encoded: {pos_data.hex()}")

    # Decode to verify
    lat_decoded = struct.unpack('>I', b'\x00' + pos_data[:3])[0]
    lon_decoded = struct.unpack('>I', b'\x00' + pos_data[3:])[0]

    # Handle two's complement
    if lat_decoded & 0x800000:
        lat_decoded = lat_decoded - (1 << 24)
    if lon_decoded & 0x800000:
        lon_decoded = lon_decoded - (1 << 24)

    scale = 2**23 / 180.0
    lat_recovered = lat_decoded / scale
    lon_recovered = lon_decoded / scale

    print(f"   Decoded: {lat_recovered:.6f}°N, {lon_recovered:.6f}°E")
    print(f"   Error: {abs(lat_test - lat_recovered) * 111000:.2f}m, "
          f"{abs(lon_test - lon_recovered) * 111000:.2f}m\n")

    print("✅ CAT021 encoder test complete!")
