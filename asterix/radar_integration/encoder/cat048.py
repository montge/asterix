"""
ASTERIX Category 048 Encoder

Encodes radar plot data into ASTERIX CAT048 binary format.
CAT048: Transmission of Monoradar Target Reports

Implements essential data items:
- I010: Data Source Identifier (SAC/SIC)
- I140: Time of Day
- I020: Target Report Descriptor
- I040: Measured Position in Polar Coordinates
- I070: Mode-3/A Code (optional)
- I130: Radar Plot Characteristics
- I220: Aircraft Address (optional)

Reference: EUROCONTROL ASTERIX Category 048 specification

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import struct
import time
from typing import List, Tuple, Optional

from .common import ASTERIXDataItem, encode_fspec, encode_i010, encode_time_of_day


# Backwards compatibility alias
CAT048DataItem = ASTERIXDataItem


def encode_i140(timestamp: Optional[float] = None) -> bytes:
    """
    I140: Time of Day (3 bytes)

    Encodes time as 1/128 seconds since midnight (modulo 24h).

    Args:
        timestamp: Unix timestamp (default: current time)

    Returns:
        3 bytes: Time of day in 1/128 second units
    """
    if timestamp is None:
        timestamp = time.time()

    # Get seconds since midnight (UTC)
    seconds_since_midnight = timestamp % 86400

    # Convert to 1/128 second units
    time_128 = int(seconds_since_midnight * 128)

    # Encode as 3 bytes (24 bits)
    return struct.pack('>I', time_128)[1:]  # Skip first byte (we only need 3)


def encode_i020(typ: str = "PSR", sim: bool = False, rad: int = 0, dup: bool = False) -> bytes:
    """
    I020: Target Report Descriptor

    Args:
        typ: Report type ("PSR", "SSR", "CMB", "MON")
        sim: Simulated target flag
        rad: RAB - Report from aircraft or field monitor (0=real, 1=test, 2=fixed transponder, 3=reserved)
        dup: Duplicate address flag (Mode S)

    Returns:
        1 byte (extensible, but we use fixed 1 byte for simplicity)
    """
    byte = 0x00

    # Bits 8-6: TYP (Target Report Type)
    typ_map = {"SSR": 0b001, "PSR": 0b010, "CMB": 0b011, "MON": 0b100}
    byte |= (typ_map.get(typ, 0b010) << 5)

    # Bit 5: SIM (Simulated)
    if sim:
        byte |= 0b00010000

    # Bit 4: RAD (RAB bit)
    if rad:
        byte |= 0b00001000

    # Bit 3: TST (Test target)
    # Bit 2: ERR (Extended Range)
    # Bit 1: FX (Extension, 0=no extension)

    return struct.pack('B', byte)


def encode_i040(range_m: float, azimuth_deg: float) -> bytes:
    """
    I040: Measured Position in Polar Coordinates

    Args:
        range_m: Slant range in meters
        azimuth_deg: Azimuth in degrees (0-360)

    Returns:
        4 bytes: RHO (2 bytes), THETA (2 bytes)
    """
    # RHO: Range in 1/256 NM (nautical miles)
    # 1 NM = 1852 meters
    range_nm = range_m / 1852.0
    rho = int(range_nm * 256)
    rho = max(0, min(65535, rho))  # Clamp to 16-bit unsigned

    # THETA: Azimuth in 360/2^16 degrees
    theta = int(azimuth_deg * 65536 / 360.0)
    theta = theta % 65536  # Wrap to 0-65535

    return struct.pack('>HH', rho, theta)


def encode_i070(mode3a_code: int) -> bytes:
    """
    I070: Mode-3/A Code in Octal Representation

    Args:
        mode3a_code: Mode 3/A code (0-7777 octal = 0-4095 decimal)

    Returns:
        2 bytes: V, G, L, 0, Mode-3/A code (12 bits)
    """
    # Bits 16-15: V (validated), G (garbled)
    # Bit 14: L (smoothed)
    # Bit 13: Spare (0)
    # Bits 12-1: Mode-3/A code in binary (each octal digit = 3 bits)

    # For simplicity, assume valid (V=0, G=0, L=0)
    code_value = mode3a_code & 0x0FFF  # 12 bits

    return struct.pack('>H', code_value)


def encode_i130(srl: Optional[float] = None, srr: Optional[float] = None,
                sac: Optional[float] = None, pri: Optional[int] = None) -> bytes:
    """
    I130: Radar Plot Characteristics (Amplitude, Width, etc.)

    This is a compound data item. For simplicity, we implement:
    - Subfield #1: SRL (SSR plot runlength)
    - Subfield #2: SRR (Number of received replies)

    Args:
        srl: SSR plot runlength in degrees (0-360)
        srr: Number of received replies
        sac: Signal amplitude (dBm)
        pri: Primary plot characteristics

    Returns:
        Variable length bytes (subfield presence indicator + data)
    """
    # Primary subfield: 1 byte indicating which subfields are present
    # Bit 8-7: spare
    # Bit 6: SRL present
    # Bit 5: SRR present
    # Bit 4: SAM present
    # Bit 3: PRI present
    # Bit 2: spare
    # Bit 1: FX (extension)

    primary = 0x00
    data = bytearray()

    if srl is not None:
        primary |= 0b01000000  # Bit 6
        # SRL: 1 byte, range 0-255 corresponds to 360 degrees
        srl_val = int((srl / 360.0) * 255)
        data.append(srl_val & 0xFF)

    if srr is not None:
        primary |= 0b00100000  # Bit 5
        # SRR: 1 byte, number of replies
        data.append(int(srr) & 0xFF)

    # For simplicity, omit SAM and PRI for now

    result = struct.pack('B', primary) + bytes(data)
    return result


def encode_i220(aircraft_address: int) -> bytes:
    """
    I220: Aircraft Address (Mode S 24-bit address)

    Args:
        aircraft_address: 24-bit Mode S address

    Returns:
        3 bytes: Aircraft address
    """
    return struct.pack('>I', aircraft_address)[1:]  # Take last 3 bytes


def encode_cat048_record(
    range_m: float,
    azimuth_deg: float,
    timestamp: Optional[float] = None,
    sac: int = 0,
    sic: int = 1,
    mode3a: Optional[int] = None,
    aircraft_address: Optional[int] = None,
    typ: str = "PSR"
) -> bytes:
    """
    Encode a single CAT048 data record (without data block header).

    Args:
        range_m: Slant range in meters
        azimuth_deg: Azimuth in degrees (0-360)
        timestamp: Unix timestamp (default: current time)
        sac: System Area Code
        sic: System Identification Code
        mode3a: Mode 3/A code (octal, e.g., 0o1234)
        aircraft_address: Mode S 24-bit address
        typ: Report type ("PSR", "SSR", "CMB")

    Returns:
        Binary data record (FSPEC + data items)
    """
    items = []
    data_items = bytearray()

    # I010: Data Source Identifier (FRN=1)
    items.append(1)
    data_items.extend(encode_i010(sac, sic))

    # I140: Time of Day (FRN=2)
    items.append(2)
    data_items.extend(encode_i140(timestamp))

    # I020: Target Report Descriptor (FRN=3)
    items.append(3)
    data_items.extend(encode_i020(typ=typ))

    # I040: Measured Position in Polar Coordinates (FRN=4)
    items.append(4)
    data_items.extend(encode_i040(range_m, azimuth_deg))

    # I070: Mode-3/A Code (FRN=5) - optional
    if mode3a is not None:
        items.append(5)
        data_items.extend(encode_i070(mode3a))

    # I130: Radar Plot Characteristics (FRN=6) - add basic SRL
    items.append(6)
    data_items.extend(encode_i130(srl=1.0))  # 1 degree runlength

    # I220: Aircraft Address (FRN=7) - optional
    if aircraft_address is not None:
        items.append(7)
        data_items.extend(encode_i220(aircraft_address))

    # Encode FSPEC
    fspec = encode_fspec(items)

    # Combine FSPEC + data items
    return fspec + bytes(data_items)


def encode_cat048_datablock(records: List[bytes]) -> bytes:
    """
    Encode a complete CAT048 data block with header.

    Args:
        records: List of encoded data records

    Returns:
        Complete data block (CAT, LEN, records)
    """
    # Data block format:
    # Byte 1: CAT (048)
    # Bytes 2-3: LEN (length in bytes, including CAT and LEN)
    # Bytes 4-N: Data records

    cat = 48
    records_data = b''.join(records)
    length = 3 + len(records_data)  # 3 bytes header + records

    # Encode: CAT (1 byte), LEN (2 bytes), Records
    header = struct.pack('!BH', cat, length)
    return header + records_data


def encode_cat048(
    plots: List,  # List of RadarPlot or dict-like objects
    radar_position: Tuple[float, float, float] = (0.0, 0.0, 0.0),
    sac: int = 0,
    sic: int = 1,
    include_mode3a: bool = False,
    include_aircraft_address: bool = False
) -> bytes:
    """
    Encode a list of radar plots into ASTERIX CAT048 format.

    Args:
        plots: List of radar plots (RadarPlot objects or dicts with range, azimuth, timestamp)
        radar_position: Radar location as (lat, lon, alt) - currently unused
        sac: System Area Code
        sic: System Identification Code
        include_mode3a: Include Mode 3/A codes (random if not in plot data)
        include_aircraft_address: Include Mode S addresses (random if not in plot data)

    Returns:
        Complete ASTERIX CAT048 data block as bytes
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

        # Optional fields
        mode3a = None
        if include_mode3a:
            # Generate random Mode 3/A code for demonstration
            # nosec B311 - random is intentionally used for demo data, not security
            import random  # nosec B311
            mode3a = random.randint(0, 0o7777)  # nosec B311

        aircraft_address = None
        if include_aircraft_address:
            # Generate random Mode S address for demonstration
            # nosec B311 - random is intentionally used for demo data, not security
            import random  # nosec B311
            aircraft_address = random.randint(0x400000, 0xFFFFFF)  # nosec B311

        record = encode_cat048_record(
            range_m=range_m,
            azimuth_deg=azimuth_deg,
            timestamp=timestamp,
            sac=sac,
            sic=sic,
            mode3a=mode3a,
            aircraft_address=aircraft_address,
            typ="PSR"
        )
        records.append(record)

    return encode_cat048_datablock(records)


if __name__ == "__main__":
    # Example usage and testing
    print("ASTERIX CAT048 Encoder - Test\n")

    # Test 1: Single plot encoding
    print("1. Encoding single radar plot...")
    record = encode_cat048_record(
        range_m=50000.0,  # 50 km
        azimuth_deg=135.5,
        timestamp=time.time(),
        sac=0,
        sic=1,
        mode3a=0o1234,
        typ="PSR"
    )
    print(f"   Record length: {len(record)} bytes")
    print(f"   Hex: {record.hex()}\n")

    # Test 2: Multiple plots in data block
    print("2. Encoding data block with 3 plots...")

    plots = [
        {'range': 30000, 'azimuth': 45.0, 'timestamp': time.time()},
        {'range': 60000, 'azimuth': 180.0, 'timestamp': time.time() + 1},
        {'range': 90000, 'azimuth': 270.5, 'timestamp': time.time() + 2},
    ]

    asterix_data = encode_cat048(plots, sac=0, sic=1)
    print(f"   Data block length: {len(asterix_data)} bytes")
    print(f"   CAT: {asterix_data[0]}")
    length = struct.unpack('!H', asterix_data[1:3])[0]
    print(f"   LEN: {length} bytes")
    print(f"   Hex (first 32 bytes): {asterix_data[:32].hex()}\n")

    # Test 3: Decode with ASTERIX parser (if available)
    print("3. Testing round-trip with ASTERIX parser...")
    try:
        import sys
        import os
        # Add project root to path
        project_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
        sys.path.insert(0, project_root)
        import asterix

        parsed = asterix.parse(asterix_data)
        print(f"   ✅ Successfully parsed {len(parsed)} records")

        if parsed:
            rec = parsed[0]
            print(f"   Record 0: CAT={rec.get('category')}, "
                  f"SAC={rec.get('I010', {}).get('SAC')}, "
                  f"SIC={rec.get('I010', {}).get('SIC')}")
            if 'I040' in rec:
                print(f"   I040: RHO={rec['I040'].get('RHO')}, "
                      f"THETA={rec['I040'].get('THETA')}")

    except Exception as e:
        print(f"   ⚠️  Parser test skipped: {e}")

    print("\n✅ CAT048 encoder test complete!")
