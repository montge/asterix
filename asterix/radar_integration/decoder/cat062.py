"""
ASTERIX Category 062 Decoder

Decodes ASTERIX CAT062 binary format (System Track Data).

Supported data items (subset):
- I010: Data Source Identifier (SAC/SIC)
- I040: Track Number
- I060: Track Mode 3/A Code
- I070: Time of Track Information
- I105: Calculated Position in WGS-84 Coordinates
- I185: Calculated Track Velocity
- I200: Mode of Movement
- I210: Calculated Acceleration
- I220: Calculated Rate of Climb/Descent
- I390: Flight Plan Related Data (Compound)

Reference: EUROCONTROL ASTERIX Category 062 specification

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import struct
import math
from typing import Dict, Any, Tuple

from .base import (
    AsterixDecoder,
    DecoderError,
    decode_uint,
    decode_int,
)


class CAT062Decoder(AsterixDecoder):
    """
    ASTERIX CAT062 Decoder (System Track Data).

    Decodes system track information including WGS-84 position, velocity,
    flight plan data, and track characteristics.

    Example:
        >>> decoder = CAT062Decoder()
        >>> records = decoder.decode_datablock(asterix_data)
        >>> print(records[0]['I105'])  # WGS-84 position
        {'lat': 52.5, 'lon': 13.4}
    """

    def __init__(self, verbose: bool = True):
        """Initialize CAT062 decoder."""
        super().__init__(category=62, verbose=verbose)

    def _init_frn_map(self) -> None:
        """Initialize FRN to data item mapping for CAT062."""
        self.frn_map = {
            1: 'I010',   # Data Source Identifier
            2: 'Reserved',
            3: 'I015',   # Service Identification
            4: 'I070',   # Time of Track Information
            5: 'I105',   # Calculated Position in WGS-84
            6: 'I100',   # Calculated Position in Cartesian
            7: 'I185',   # Calculated Track Velocity
            # FRN 8-14 (second FSPEC octet)
            8: 'I210',   # Calculated Acceleration
            9: 'I060',   # Track Mode 3/A Code
            10: 'I245',  # Target Identification
            11: 'I380',  # Aircraft Derived Data
            12: 'I040',  # Track Number
            13: 'I080',  # Track Status
            14: 'I290',  # System Track Update Ages
            # FRN 15-21 (third FSPEC octet)
            15: 'I200',  # Mode of Movement
            16: 'I295',  # Track Data Ages
            17: 'I136',  # Measured Flight Level
            18: 'I130',  # Calculated Track Geometric Altitude
            19: 'I135',  # Calculated Track Barometric Altitude
            20: 'I220',  # Calculated Rate of Climb/Descent
            21: 'I390',  # Flight Plan Related Data (Compound)
        }

    def _decode_data_item(
        self, frn: int, data: bytes, offset: int
    ) -> Tuple[Any, int]:
        """Decode a CAT062 data item."""
        if frn == 1:  # I010
            return self._decode_i010(data, offset)
        elif frn == 4:  # I070
            return self._decode_i070(data, offset)
        elif frn == 5:  # I105
            return self._decode_i105(data, offset)
        elif frn == 7:  # I185
            return self._decode_i185(data, offset)
        elif frn == 9:  # I060
            return self._decode_i060(data, offset)
        elif frn == 12:  # I040
            return self._decode_i040(data, offset)
        elif frn == 15:  # I200
            return self._decode_i200(data, offset)
        elif frn == 20:  # I220
            return self._decode_i220(data, offset)
        elif frn == 21:  # I390
            return self._decode_i390(data, offset)
        else:
            raise DecoderError(f"Unsupported CAT062 FRN {frn}")

    def _decode_i010(self, data: bytes, offset: int) -> Tuple[Dict[str, int], int]:
        """Decode I010: Data Source Identifier."""
        if offset + 2 > len(data):
            raise DecoderError("I010 truncated")

        sac = data[offset]
        sic = data[offset + 1]

        return {'SAC': sac, 'SIC': sic}, 2

    def _decode_i070(self, data: bytes, offset: int) -> Tuple[float, int]:
        """
        Decode I070: Time of Track Information.

        Format: 3 bytes (1/128 seconds since midnight)
        """
        if offset + 3 > len(data):
            raise DecoderError("I070 truncated")

        time_128 = decode_uint(data, offset, 3)
        time_seconds = time_128 / 128.0

        if self.verbose:
            return {
                'value': time_seconds,
                'time_128': time_128,
            }, 3
        else:
            return time_seconds, 3

    def _decode_i105(self, data: bytes, offset: int) -> Tuple[Dict[str, float], int]:
        """
        Decode I105: Calculated Position in WGS-84 Coordinates.

        Format: 8 bytes
        - Bytes 1-4: Latitude (two's complement, 180/2^25 degrees LSB)
        - Bytes 5-8: Longitude (two's complement, 180/2^25 degrees LSB)
        """
        if offset + 8 > len(data):
            raise DecoderError("I105 truncated")

        lat_raw = decode_int(data, offset, 4)
        lon_raw = decode_int(data, offset + 4, 4)

        # Convert to degrees
        lat = lat_raw * 180.0 / (2 ** 25)
        lon = lon_raw * 180.0 / (2 ** 25)

        result = {
            'lat': lat,
            'lon': lon,
        }

        if self.verbose:
            result['description'] = f"WGS-84: {lat:.6f}°, {lon:.6f}°"

        return result, 8

    def _decode_i185(self, data: bytes, offset: int) -> Tuple[Dict[str, float], int]:
        """
        Decode I185: Calculated Track Velocity.

        Format: 4 bytes
        - Bytes 1-2: Vx (two's complement, 0.25 m/s LSB)
        - Bytes 3-4: Vy (two's complement, 0.25 m/s LSB)
        """
        if offset + 4 > len(data):
            raise DecoderError("I185 truncated")

        vx_raw = decode_int(data, offset, 2)
        vy_raw = decode_int(data, offset + 2, 2)

        vx = vx_raw * 0.25  # m/s
        vy = vy_raw * 0.25  # m/s

        result = {
            'Vx': vx,
            'Vy': vy,
        }

        if self.verbose:
            # Calculate speed and heading
            speed = math.sqrt(vx**2 + vy**2)
            heading = (math.degrees(math.atan2(vx, vy)) + 360) % 360
            result['speed'] = speed
            result['heading'] = heading
            result['description'] = f"Speed: {speed:.1f} m/s, Heading: {heading:.1f}°"

        return result, 4

    def _decode_i060(self, data: bytes, offset: int) -> Tuple[Dict[str, Any], int]:
        """Decode I060: Track Mode 3/A Code."""
        if offset + 2 > len(data):
            raise DecoderError("I060 truncated")

        value = decode_uint(data, offset, 2)

        # Similar to CAT048 I070
        code = value & 0x0FFF

        result = {'code': code}

        if self.verbose:
            result['octal'] = f"{code:04o}"

        return result, 2

    def _decode_i040(self, data: bytes, offset: int) -> Tuple[int, int]:
        """
        Decode I040: Track Number.

        Format: 2 bytes (track number)
        """
        if offset + 2 > len(data):
            raise DecoderError("I040 truncated")

        track_num = decode_uint(data, offset, 2)

        if self.verbose:
            return {'track_number': track_num}, 2
        else:
            return track_num, 2

    def _decode_i200(self, data: bytes, offset: int) -> Tuple[Dict[str, Any], int]:
        """
        Decode I200: Mode of Movement.

        Format: 1 byte
        - Bits 8-7: TRANS (Transversal acceleration)
        - Bits 6-5: LONG (Longitudinal acceleration)
        - Bits 4-3: VERT (Vertical rate)
        - Bit 2: ADF (Altitude discrepancy flag)
        """
        if offset >= len(data):
            raise DecoderError("I200 truncated")

        byte = data[offset]

        trans = (byte >> 6) & 0x03
        long_ = (byte >> 4) & 0x03
        vert = (byte >> 2) & 0x03
        adf = bool(byte & 0x02)

        return {
            'TRANS': trans,
            'LONG': long_,
            'VERT': vert,
            'ADF': adf,
        }, 1

    def _decode_i220(self, data: bytes, offset: int) -> Tuple[float, int]:
        """
        Decode I220: Calculated Rate of Climb/Descent.

        Format: 2 bytes (two's complement, 6.25 ft/min LSB)
        """
        if offset + 2 > len(data):
            raise DecoderError("I220 truncated")

        rocd_raw = decode_int(data, offset, 2)
        rocd = rocd_raw * 6.25  # ft/min

        if self.verbose:
            return {
                'ROCD': rocd,
                'description': f"Rate of Climb/Descent: {rocd:.1f} ft/min",
            }, 2
        else:
            return rocd, 2

    def _decode_i390(self, data: bytes, offset: int) -> Tuple[Dict[str, Any], int]:
        """
        Decode I390: Flight Plan Related Data (Compound Item).

        Format: Variable length compound
        - Byte 1: Primary subfield (indicates which subfields present)
        - Subsequent bytes: Subfield data

        This is a simplified implementation (full spec has many subfields).
        """
        if offset >= len(data):
            raise DecoderError("I390 truncated")

        primary = data[offset]
        bytes_consumed = 1
        result = {}

        # Simplified: just track which subfields are present
        # Full implementation would decode each subfield
        result['subfields_present'] = bin(primary)

        # For now, we'll skip the actual subfield decoding
        # (would need to parse each subfield based on primary byte)

        # This is a placeholder - full implementation needed
        # For safety, we'll raise an error for now
        raise DecoderError("I390 (Flight Plan) decoding not fully implemented yet")

        # return result, bytes_consumed


def decode_cat062(data: bytes, verbose: bool = True) -> list:
    """
    Decode ASTERIX CAT062 data (convenience function).

    Args:
        data: Raw ASTERIX CAT062 binary data
        verbose: Include descriptions and metadata

    Returns:
        List of decoded records
    """
    decoder = CAT062Decoder(verbose=verbose)
    return decoder.decode_datablock(data)
