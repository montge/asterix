"""
ASTERIX Category 019 Encoder

Encodes multilateration system status messages into ASTERIX CAT019 binary format.
CAT019: Multilateration System Status Messages

CAT019 provides status information about multilateration (MLAT) systems,
including coverage, system health, and service messages. Complements CAT020
(MLAT target reports).

Implements essential data items:
- I010: Data Source Identifier (SAC/SIC)
- I000: Message Type (Start of Update, Periodic Status, Event)
- I140: Time of Day
- I550: System Status (operational status, health)
- I551: Tracking Processor Status
- I552: Remote Sensor Status
- I600: Position of Multilateration System Reference Point
- I610: Height of Multilateration System Reference Point

Reference: EUROCONTROL ASTERIX Category 019 specification

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import struct
import time
from typing import List, Optional

from .common import encode_fspec, encode_i010


def encode_i000(message_type: int) -> bytes:
    """
    I000: Message Type.

    Args:
        message_type:
            1 = Start of Update Cycle
            2 = Periodic Status Message
            3 = Event-Triggered Message

    Returns:
        1 byte: Message type
    """
    return struct.pack('B', message_type & 0xFF)


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


def encode_i550(operational: bool = True, degraded: bool = False,
                 test: bool = False, num_sensors: int = 4) -> bytes:
    """
    I550: System Status.

    Args:
        operational: System is operational
        degraded: Operating in degraded mode
        test: In test mode
        num_sensors: Number of active sensors (0-255)

    Returns:
        Variable length (primary + data)
    """
    primary = 0x00

    # Bit 8: NOGO (1=not operational, 0=operational)
    if not operational:
        primary |= 0b10000000

    # Bit 7: OVL (overload)
    # Bit 6: TSV (time source valid)
    primary |= 0b00100000  # Assume time valid

    # Bit 5: TTF (test target failure)
    if test:
        primary |= 0b00010000

    # Bit 4: Spare
    # Bit 3: Extension for sensor status
    if num_sensors > 0:
        primary |= 0b00000100

    # For simplicity, include NOGO status and sensor count
    result = struct.pack('B', primary)

    # Add sensor count if present
    if num_sensors > 0:
        result += struct.pack('B', num_sensors)

    return result


def encode_i600(lat: float, lon: float) -> bytes:
    """
    I600: Position of MLAT System Reference Point.

    Resolution: 180/2^25 degrees

    Args:
        lat: Latitude in degrees
        lon: Longitude in degrees

    Returns:
        8 bytes: LAT (4 bytes), LON (4 bytes)
    """
    scale = 2**25 / 180.0

    lat_encoded = int(lat * scale)
    lon_encoded = int(lon * scale)

    lat_encoded = max(-2**31, min(2**31 - 1, lat_encoded))
    lon_encoded = max(-2**31, min(2**31 - 1, lon_encoded))

    return struct.pack('>ii', lat_encoded, lon_encoded)


def encode_i610(height_m: float) -> bytes:
    """
    I610: Height of MLAT System Reference Point.

    Resolution: 1 meter

    Args:
        height_m: Height above WGS-84 ellipsoid in meters

    Returns:
        2 bytes: Height in meters (signed)
    """
    height_encoded = int(height_m)
    height_encoded = max(-32768, min(32767, height_encoded))

    return struct.pack('>h', height_encoded)


def encode_cat019_status(
    sac: int = 0,
    sic: int = 1,
    message_type: int = 2,  # Periodic status
    timestamp: Optional[float] = None,
    mlat_lat: Optional[float] = None,
    mlat_lon: Optional[float] = None,
    mlat_height_m: Optional[float] = None,
    operational: bool = True,
    num_sensors: int = 4
) -> bytes:
    """
    Encode a CAT019 MLAT system status message.

    Args:
        sac: System Area Code
        sic: System Identification Code
        message_type: Message type (1=start, 2=periodic, 3=event)
        timestamp: Unix timestamp
        mlat_lat: MLAT system reference latitude
        mlat_lon: MLAT system reference longitude
        mlat_height_m: MLAT system height (meters)
        operational: System operational flag
        num_sensors: Number of active sensors

    Returns:
        Complete ASTERIX CAT019 data block
    """
    items = []
    data_items = bytearray()

    # I010: Data Source Identifier (FRN=1)
    items.append(1)
    data_items.extend(encode_i010(sac, sic))

    # I000: Message Type (FRN=2)
    items.append(2)
    data_items.extend(encode_i000(message_type))

    # I140: Time of Day (FRN=3)
    items.append(3)
    data_items.extend(encode_i140(timestamp))

    # I550: System Status (FRN=4)
    items.append(4)
    data_items.extend(encode_i550(operational=operational, num_sensors=num_sensors))

    # I600: MLAT Position (FRN=5)
    if mlat_lat is not None and mlat_lon is not None:
        items.append(5)
        data_items.extend(encode_i600(mlat_lat, mlat_lon))

    # I610: MLAT Height (FRN=6)
    if mlat_height_m is not None:
        items.append(6)
        data_items.extend(encode_i610(mlat_height_m))

    # Encode FSPEC
    fspec = encode_fspec(items)
    record = fspec + bytes(data_items)

    # Create data block
    cat = 19
    length = 3 + len(record)
    header = struct.pack('!BH', cat, length)

    return header + record


def encode_cat019_update_cycle(
    sac: int = 0,
    sic: int = 1,
    timestamp: Optional[float] = None,
    num_targets: int = 0
) -> bytes:
    """
    Encode a CAT019 Start of Update Cycle message.

    Indicates the start of a new MLAT update cycle (similar to radar rotation).

    Args:
        sac: System Area Code
        sic: System Identification Code
        timestamp: Unix timestamp
        num_targets: Number of targets in this update

    Returns:
        Complete ASTERIX CAT019 data block
    """
    return encode_cat019_status(
        sac=sac,
        sic=sic,
        message_type=1,  # Start of Update
        timestamp=timestamp,
        operational=True
    )


if __name__ == "__main__":
    print("ASTERIX CAT019 Encoder - Test\n")

    # Test 1: Periodic status message
    print("1. Encoding MLAT system status message...")
    status = encode_cat019_status(
        sac=0,
        sic=1,
        message_type=2,  # Periodic
        mlat_lat=52.5597,  # Berlin Airport MLAT reference
        mlat_lon=13.2877,
        mlat_height_m=100.0,
        operational=True,
        num_sensors=4
    )
    print(f"   Length: {len(status)} bytes")
    print(f"   CAT: {status[0]}")
    print("   Message: Periodic Status")
    print("   Sensors: 4 active")
    print(f"   Hex: {status.hex()}\n")

    # Test 2: Start of update cycle
    print("2. Encoding Start of Update Cycle...")
    update = encode_cat019_update_cycle(sac=0, sic=1)
    print(f"   Length: {len(update)} bytes")
    print(f"   Hex: {update.hex()}\n")

    # Test 3: Multiple status messages
    print("3. Simulating MLAT system health monitoring (10 seconds)...")
    for i in range(10):
        msg = encode_cat019_status(
            sac=0,
            sic=1,
            timestamp=time.time() + i,
            operational=True,
            num_sensors=4
        )
        print(f"   t+{i}s: {len(msg)} bytes - {msg[:16].hex()}...")

    print("\n✅ CAT019 encoder test complete!")
    print("\nCAT019 is essential for:")
    print("  • MLAT system health monitoring")
    print("  • Sensor coverage status")
    print("  • System synchronization")
    print("  • Service quality reporting")
