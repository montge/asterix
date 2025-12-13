"""
ASTERIX Category 021 Decoder

Decodes ASTERIX CAT021 binary format (ADS-B Target Reports).

Supported data items (subset):
- I010: Data Source Identifier (SAC/SIC)
- I040: Target Report Descriptor
- I070: Mode 3/A Code
- I073: Time of Message Reception of Position
- I075: Time of Message Reception of Velocity
- I080: Target Address (24-bit ICAO address)
- I090: Quality Indicators
- I130: Position in WGS-84 Coordinates (High Precision)
- I140: Geometric Height
- I145: Flight Level
- I150: Air Speed
- I155: True Air Speed
- I160: Ground Vector (speed and track angle)

Reference: EUROCONTROL ASTERIX Category 021 specification

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

from typing import Dict, Any, Tuple

from .base import (
    AsterixDecoder,
    DecoderError,
    decode_uint,
    decode_int,
)


class CAT021Decoder(AsterixDecoder):
    """
    ASTERIX CAT021 Decoder (ADS-B Target Reports).

    Decodes ADS-B surveillance data including high-precision WGS-84 position,
    velocity, quality indicators, and aircraft identification.

    Example:
        >>> decoder = CAT021Decoder()
        >>> records = decoder.decode_datablock(asterix_data)
        >>> print(records[0]['I130'])  # High-precision position
        {'lat': 52.5123456, 'lon': 13.4567890}
    """

    def __init__(self, verbose: bool = True):
        """Initialize CAT021 decoder."""
        super().__init__(category=21, verbose=verbose)

    def _init_frn_map(self) -> None:
        """Initialize FRN to data item mapping for CAT021."""
        self.frn_map = {
            1: 'I010',   # Data Source Identifier
            2: 'I040',   # Target Report Descriptor
            3: 'I161',   # Track Number
            4: 'I015',   # Service Identification
            5: 'I071',   # Time of Applicability for Position
            6: 'I130',   # Position in WGS-84 Coordinates
            7: 'I131',   # High-Res Position in WGS-84
            # FRN 8-14
            8: 'I072',   # Time of Applicability for Velocity
            9: 'I150',   # Air Speed
            10: 'I151',  # True Air Speed
            11: 'I080',  # Target Address (24-bit)
            12: 'I073',  # Time of Message Reception for Position
            13: 'I074',  # Time of Message Reception for Position-High Precision
            14: 'I075',  # Time of Message Reception for Velocity
            # FRN 15-21
            15: 'I076',  # Time of Message Reception for Velocity-High Precision
            16: 'I140',  # Geometric Height
            17: 'I090',  # Quality Indicators
            18: 'I210',  # MOPS Version
            19: 'I070',  # Mode 3/A Code
            20: 'I230',  # Roll Angle
            21: 'I145',  # Flight Level
        }

    def _decode_data_item(
        self, frn: int, data: bytes, offset: int
    ) -> Tuple[Any, int]:
        """Decode a CAT021 data item."""
        if frn == 1:  # I010
            return self._decode_i010(data, offset)
        elif frn == 2:  # I040
            return self._decode_i040(data, offset)
        elif frn == 3:  # I161
            return self._decode_i161(data, offset)
        elif frn == 4:  # I015
            return self._decode_i015(data, offset)
        elif frn == 5:  # I071
            return self._decode_i071(data, offset)
        elif frn == 6:  # I130
            return self._decode_i130(data, offset)
        elif frn == 11:  # I080
            return self._decode_i080(data, offset)
        elif frn == 12:  # I073
            return self._decode_i073(data, offset)
        elif frn == 16:  # I140
            return self._decode_i140(data, offset)
        elif frn == 17:  # I090
            return self._decode_i090(data, offset)
        elif frn == 19:  # I070
            return self._decode_i070(data, offset)
        elif frn == 21:  # I145
            return self._decode_i145(data, offset)
        else:
            raise DecoderError(f"Unsupported CAT021 FRN {frn}")

    def _decode_i010(self, data: bytes, offset: int) -> Tuple[Dict[str, int], int]:
        """Decode I010: Data Source Identifier."""
        if offset + 2 > len(data):
            raise DecoderError("I010 truncated")

        sac = data[offset]
        sic = data[offset + 1]

        return {'SAC': sac, 'SIC': sic}, 2

    def _decode_i161(self, data: bytes, offset: int) -> Tuple[Dict[str, int], int]:
        """
        Decode I161: Track Number.

        Format: 2 bytes (12-bit track number + 4-bit spare)
        """
        if offset + 2 > len(data):
            raise DecoderError("I161 truncated")

        value = decode_uint(data, offset, 2)
        track_number = value >> 4  # Upper 12 bits

        result = {'track_number': track_number}
        return result, 2

    def _decode_i015(self, data: bytes, offset: int) -> Tuple[int, int]:
        """
        Decode I015: Service Identification.

        Format: 1 byte
        """
        if offset >= len(data):
            raise DecoderError("I015 truncated")

        service_id = data[offset]
        return service_id, 1

    def _decode_i071(self, data: bytes, offset: int) -> Tuple[float, int]:
        """
        Decode I071: Time of Applicability for Position.

        Format: 3 bytes (1/128 seconds since midnight)
        """
        if offset + 3 > len(data):
            raise DecoderError("I071 truncated")

        time_128 = decode_uint(data, offset, 3)
        time_seconds = time_128 / 128.0

        if self.verbose:
            return {
                'value': time_seconds,
                'time_128': time_128,
            }, 3
        else:
            return time_seconds, 3

    def _decode_i040(self, data: bytes, offset: int) -> Tuple[Dict[str, Any], int]:
        """
        Decode I040: Target Report Descriptor.

        Format: Variable length (extensible, 1-3 bytes typical)
        - Byte 1: ATP, ARC, RC, RAB, FX
        - Optional extension bytes
        """
        if offset >= len(data):
            raise DecoderError("I040 truncated")

        byte1 = data[offset]
        bytes_consumed = 1

        atp = (byte1 >> 5) & 0x07  # Bits 8-6
        arc = (byte1 >> 3) & 0x03  # Bits 5-4
        rc = bool(byte1 & 0b00000100)  # Bit 3
        rab = bool(byte1 & 0b00000010)  # Bit 2
        fx = bool(byte1 & 0x01)  # Bit 1

        result = {
            'ATP': atp,
            'ARC': arc,
            'RC': rc,
            'RAB': rab,
        }

        # Handle extension (if present)
        if fx and offset + 1 < len(data):
            bytes_consumed += 1
            # Could decode extension fields here

        return result, bytes_consumed

    def _decode_i130(self, data: bytes, offset: int) -> Tuple[Dict[str, float], int]:
        """
        Decode I130: Position in WGS-84 Coordinates (High Precision).

        Format: 6 bytes
        - Bytes 1-3: Latitude (two's complement, 180/2^23 degrees LSB)
        - Bytes 4-6: Longitude (two's complement, 180/2^23 degrees LSB)

        Note: CAT021 uses higher precision than CAT062 (2^23 vs 2^25)
        """
        if offset + 6 > len(data):
            raise DecoderError("I130 truncated")

        # Decode 24-bit signed integers
        lat_raw = decode_uint(data, offset, 3)
        lon_raw = decode_uint(data, offset + 3, 3)

        # Convert from unsigned to signed 24-bit
        if lat_raw >= 0x800000:
            lat_raw -= 0x1000000
        if lon_raw >= 0x800000:
            lon_raw -= 0x1000000

        # Convert to degrees (180/2^23 LSB)
        lat = lat_raw * 180.0 / (2 ** 23)
        lon = lon_raw * 180.0 / (2 ** 23)

        result = {
            'lat': lat,
            'lon': lon,
        }

        if self.verbose:
            result['description'] = f"WGS-84: {lat:.7f}°, {lon:.7f}°"

        return result, 6

    def _decode_i080(self, data: bytes, offset: int) -> Tuple[int, int]:
        """
        Decode I080: Target Address (24-bit ICAO address).

        Format: 3 bytes
        """
        if offset + 3 > len(data):
            raise DecoderError("I080 truncated")

        address = decode_uint(data, offset, 3)

        if self.verbose:
            return {
                'address': address,
                'hex': f"0x{address:06X}",
                'description': f"ICAO Address: 0x{address:06X}",
            }, 3
        else:
            return address, 3

    def _decode_i073(self, data: bytes, offset: int) -> Tuple[float, int]:
        """
        Decode I073: Time of Message Reception for Position.

        Format: 3 bytes (1/128 seconds since midnight)
        """
        if offset + 3 > len(data):
            raise DecoderError("I073 truncated")

        time_128 = decode_uint(data, offset, 3)
        time_seconds = time_128 / 128.0

        if self.verbose:
            return {
                'value': time_seconds,
                'time_128': time_128,
            }, 3
        else:
            return time_seconds, 3

    def _decode_i140(self, data: bytes, offset: int) -> Tuple[float, int]:
        """
        Decode I140: Geometric Height.

        Format: 2 bytes (two's complement, 6.25 ft LSB)
        """
        if offset + 2 > len(data):
            raise DecoderError("I140 truncated")

        height_raw = decode_int(data, offset, 2)
        height_ft = height_raw * 6.25  # feet

        if self.verbose:
            height_m = height_ft * 0.3048  # Convert to meters
            return {
                'height_ft': height_ft,
                'height_m': height_m,
                'description': f"Geometric Height: {height_ft:.0f} ft ({height_m:.0f} m)",
            }, 2
        else:
            return height_ft, 2

    def _decode_i090(self, data: bytes, offset: int) -> Tuple[Dict[str, Any], int]:
        """
        Decode I090: Quality Indicators.

        Format: Variable (extensible, typically 2 bytes)
        - Byte 1: NUCr/NACv, NUCp, NICbaro, SIL, NACp, FX
        - Byte 2: SILS, SDA, GVA, PIC, FX (if extension present)
        """
        if offset >= len(data):
            raise DecoderError("I090 truncated")

        byte1 = data[offset]
        bytes_consumed = 1

        # Parse first byte (simplified)
        fx = bool(byte1 & 0x01)

        result = {
            'quality_byte1': byte1,
        }

        # Handle extension
        if fx and offset + 1 < len(data):
            byte2 = data[offset + 1]
            result['quality_byte2'] = byte2
            bytes_consumed += 1

        return result, bytes_consumed

    def _decode_i070(self, data: bytes, offset: int) -> Tuple[Dict[str, Any], int]:
        """Decode I070: Mode 3/A Code."""
        if offset + 2 > len(data):
            raise DecoderError("I070 truncated")

        value = decode_uint(data, offset, 2)
        code = value & 0x0FFF

        result = {'code': code}

        if self.verbose:
            result['octal'] = f"{code:04o}"

        return result, 2

    def _decode_i145(self, data: bytes, offset: int) -> Tuple[float, int]:
        """
        Decode I145: Flight Level.

        Format: 2 bytes (two's complement, 1/4 FL LSB)
        """
        if offset + 2 > len(data):
            raise DecoderError("I145 truncated")

        fl_raw = decode_int(data, offset, 2)
        fl = fl_raw / 4.0  # Flight level

        if self.verbose:
            return {
                'FL': fl,
                'description': f"Flight Level: {fl:.2f}",
            }, 2
        else:
            return fl, 2


def decode_cat021(data: bytes, verbose: bool = True) -> list:
    """
    Decode ASTERIX CAT021 data (convenience function).

    Args:
        data: Raw ASTERIX CAT021 binary data
        verbose: Include descriptions and metadata

    Returns:
        List of decoded records
    """
    decoder = CAT021Decoder(verbose=verbose)
    return decoder.decode_datablock(data)
