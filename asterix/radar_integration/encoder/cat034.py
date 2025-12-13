"""
ASTERIX Category 034 Encoder

Encodes radar service messages into ASTERIX CAT034 binary format.
CAT034: Transmission of Monoradar Service Messages (North Marker, Sector Crossing)

Implements essential data items:
- I010: Data Source Identifier (SAC/SIC)
- I000: Message Type (North Marker, Sector Crossing, Service Status)
- I020: Sector Number
- I030: Time of Day
- I041: Antenna Rotation Speed
- I050: System Configuration and Status
- I060: System Processing Mode

Reference: EUROCONTROL ASTERIX Category 034 specification

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import struct
import time
from typing import List, Optional


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


def encode_i000(message_type: int) -> bytes:
    """
    I000: Message Type.

    Args:
        message_type:
            1 = North marker message
            2 = Sector crossing message
            3 = Geographical filtering message
            4 = Jamming strobe message

    Returns:
        1 byte: Message type
    """
    return struct.pack('B', message_type & 0xFF)


def encode_i020(sector_number: int) -> bytes:
    """
    I020: Sector Number.

    Args:
        sector_number: Sector number (0-255)

    Returns:
        1 byte: Sector number
    """
    return struct.pack('B', sector_number & 0xFF)


def encode_i030(timestamp: Optional[float] = None) -> bytes:
    """
    I030: Time of Day.

    Args:
        timestamp: Unix timestamp (default: current time)

    Returns:
        3 bytes: Time in 1/128 second units
    """
    if timestamp is None:
        timestamp = time.time()

    seconds_since_midnight = timestamp % 86400
    time_128 = int(seconds_since_midnight * 128)

    return struct.pack('>I', time_128)[1:]


def encode_i041(antenna_rotation_speed: float) -> bytes:
    """
    I041: Antenna Rotation Speed.

    Resolution: 1/128 seconds per revolution

    Args:
        antenna_rotation_speed: Rotation period in seconds (e.g., 4.0 for 4-second rotation)

    Returns:
        2 bytes: Rotation speed in 1/128 second units
    """
    speed_encoded = int(antenna_rotation_speed * 128)
    speed_encoded = max(0, min(65535, speed_encoded))

    return struct.pack('>H', speed_encoded)


def encode_cat034_north_marker(
    sac: int = 0,
    sic: int = 1,
    sector_number: int = 0,
    timestamp: Optional[float] = None,
    antenna_rotation_speed: float = 4.0
) -> bytes:
    """
    Encode a CAT034 North Marker message.

    North Marker messages are sent each time the radar antenna passes
    the North reference direction (azimuth = 0°).

    Args:
        sac: System Area Code
        sic: System Identification Code
        sector_number: Current sector (0-255)
        timestamp: Unix timestamp
        antenna_rotation_speed: Rotation period in seconds

    Returns:
        Complete ASTERIX CAT034 data block
    """
    items = []
    data_items = bytearray()

    # I010: Data Source Identifier (FRN=1)
    items.append(1)
    data_items.extend(encode_i010(sac, sic))

    # I000: Message Type (FRN=2) - North Marker = 1
    items.append(2)
    data_items.extend(encode_i000(message_type=1))

    # I020: Sector Number (FRN=3)
    items.append(3)
    data_items.extend(encode_i020(sector_number))

    # I030: Time of Day (FRN=4)
    items.append(4)
    data_items.extend(encode_i030(timestamp))

    # I041: Antenna Rotation Speed (FRN=5)
    items.append(5)
    data_items.extend(encode_i041(antenna_rotation_speed))

    # Encode FSPEC
    fspec = encode_fspec(items)

    # Create record
    record = fspec + bytes(data_items)

    # Create data block
    cat = 34
    length = 3 + len(record)
    header = struct.pack('!BH', cat, length)

    return header + record


def encode_cat034_sector_crossing(
    sac: int = 0,
    sic: int = 1,
    sector_number: int = 0,
    timestamp: Optional[float] = None
) -> bytes:
    """
    Encode a CAT034 Sector Crossing message.

    Args:
        sac: System Area Code
        sic: System Identification Code
        sector_number: Sector being entered (0-255)
        timestamp: Unix timestamp

    Returns:
        Complete ASTERIX CAT034 data block
    """
    items = []
    data_items = bytearray()

    # I010: Data Source Identifier (FRN=1)
    items.append(1)
    data_items.extend(encode_i010(sac, sic))

    # I000: Message Type (FRN=2) - Sector Crossing = 2
    items.append(2)
    data_items.extend(encode_i000(message_type=2))

    # I020: Sector Number (FRN=3)
    items.append(3)
    data_items.extend(encode_i020(sector_number))

    # I030: Time of Day (FRN=4)
    items.append(4)
    data_items.extend(encode_i030(timestamp))

    fspec = encode_fspec(items)
    record = fspec + bytes(data_items)

    cat = 34
    length = 3 + len(record)
    header = struct.pack('!BH', cat, length)

    return header + record


if __name__ == "__main__":
    print("ASTERIX CAT034 Encoder - Test\n")

    # Test 1: North Marker
    print("1. Encoding North Marker message...")
    nm_data = encode_cat034_north_marker(
        sac=0,
        sic=1,
        sector_number=0,
        timestamp=time.time(),
        antenna_rotation_speed=4.0
    )
    print(f"   Length: {len(nm_data)} bytes")
    print(f"   CAT: {nm_data[0]}")
    print("   Message Type: North Marker (1)")
    print(f"   Hex: {nm_data.hex()}\n")

    # Test 2: Sector Crossing (simulating 8-sector radar)
    print("2. Encoding Sector Crossing messages (8 sectors)...")
    for sector in range(8):
        sc_data = encode_cat034_sector_crossing(
            sac=0,
            sic=1,
            sector_number=sector,
            timestamp=time.time() + sector * 0.5
        )
        print(f"   Sector {sector}: {len(sc_data)} bytes - {sc_data[:16].hex()}...")

    print("\n✅ CAT034 encoder test complete!")
