"""
ASTERIX Category 048 Decoder

Decodes ASTERIX CAT048 binary format (Transmission of Monoradar Target Reports).

Supported data items:
- I010: Data Source Identifier (SAC/SIC)
- I020: Target Report Descriptor
- I040: Measured Position in Polar Coordinates
- I070: Mode-3/A Code in Octal Representation
- I090: Flight Level in Binary Representation
- I130: Radar Plot Characteristics (Compound item)
- I140: Time of Day
- I220: Aircraft Address
- I240: Aircraft Identification
- I250: Mode S MB Data

Reference: EUROCONTROL ASTERIX Category 048 specification

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
    decode_6bit_ascii,
)


class CAT048Decoder(AsterixDecoder):
    """
    ASTERIX CAT048 Decoder (Monoradar Target Reports).

    Decodes radar plot data including position, Mode 3/A codes, Mode S addresses,
    and radar plot characteristics. Output format matches C++ decoder for compatibility.

    Example:
        >>> decoder = CAT048Decoder()
        >>> records = decoder.decode_datablock(asterix_data)
        >>> print(records[0]['I040'])  # Position
        {'RHO': 27.0, 'THETA': 135.5}
    """

    def __init__(self, verbose: bool = True):
        """Initialize CAT048 decoder."""
        super().__init__(category=48, verbose=verbose)

    def _init_frn_map(self) -> None:
        """Initialize FRN to data item mapping for CAT048."""
        # Based on CAT048 User Application Profile (UAP)
        self.frn_map = {
            1: 'I010',   # Data Source Identifier
            2: 'I140',   # Time of Day
            3: 'I020',   # Target Report Descriptor
            4: 'I040',   # Measured Position in Polar Coordinates
            5: 'I070',   # Mode-3/A Code
            6: 'I090',   # Flight Level
            7: 'I130',   # Radar Plot Characteristics
            # FRN 8-14 (second FSPEC octet)
            8: 'I220',   # Aircraft Address
            9: 'I240',   # Aircraft Identification
            10: 'I250',  # Mode S MB Data
            11: 'I161',  # Track Number
            12: 'I042',  # Calculated Position in Cartesian Coordinates
            13: 'I200',  # Calculated Track Velocity in Polar Coordinates
            14: 'I170',  # Track Status
            # Additional items in later octets can be added as needed
        }

    def _decode_data_item(
        self, frn: int, data: bytes, offset: int
    ) -> Tuple[Any, int]:
        """
        Decode a CAT048 data item.

        Args:
            frn: Field Reference Number
            data: Binary data buffer
            offset: Current offset in data

        Returns:
            Tuple of (decoded_value, bytes_consumed)
        """
        # Dispatch to item-specific decoder
        item_name = self.frn_map.get(frn, f'I{frn:03d}')

        if frn == 1:  # I010
            return self._decode_i010(data, offset)
        elif frn == 2:  # I140
            return self._decode_i140(data, offset)
        elif frn == 3:  # I020
            return self._decode_i020(data, offset)
        elif frn == 4:  # I040
            return self._decode_i040(data, offset)
        elif frn == 5:  # I070
            return self._decode_i070(data, offset)
        elif frn == 6:  # I090
            return self._decode_i090(data, offset)
        elif frn == 7:  # I130
            return self._decode_i130(data, offset)
        elif frn == 8:  # I220
            return self._decode_i220(data, offset)
        elif frn == 9:  # I240
            return self._decode_i240(data, offset)
        elif frn == 10:  # I250
            return self._decode_i250(data, offset)
        else:
            # Unknown item - try to skip gracefully
            raise DecoderError(f"Unsupported FRN {frn} ({item_name})")

    def _decode_i010(self, data: bytes, offset: int) -> Tuple[Dict[str, int], int]:
        """
        Decode I010: Data Source Identifier (SAC/SIC).

        Format: 2 bytes
        - Byte 1: SAC (System Area Code)
        - Byte 2: SIC (System Identification Code)

        Returns:
            ({'SAC': int, 'SIC': int}, 2)
        """
        if offset + 2 > len(data):
            raise DecoderError("I010 truncated")

        sac = data[offset]
        sic = data[offset + 1]

        result = {'SAC': sac, 'SIC': sic}

        if self.verbose:
            result['description'] = f"SAC={sac} SIC={sic}"

        return result, 2

    def _decode_i140(self, data: bytes, offset: int) -> Tuple[float, int]:
        """
        Decode I140: Time of Day.

        Format: 3 bytes (24 bits)
        - Value in 1/128 seconds since midnight (modulo 24h)

        Returns:
            (time_in_seconds, 3)
        """
        if offset + 3 > len(data):
            raise DecoderError("I140 truncated")

        # Read 3 bytes as 24-bit unsigned integer
        time_128 = decode_uint(data, offset, 3)

        # Convert to seconds
        time_seconds = time_128 / 128.0

        if self.verbose:
            hours = int(time_seconds // 3600)
            minutes = int((time_seconds % 3600) // 60)
            seconds = time_seconds % 60
            # Return dict with metadata
            return {
                'value': time_seconds,
                'time_128': time_128,
                'description': f"{hours:02d}:{minutes:02d}:{seconds:06.3f}",
            }, 3
        else:
            return time_seconds, 3

    def _decode_i020(self, data: bytes, offset: int) -> Tuple[Dict[str, Any], int]:
        """
        Decode I020: Target Report Descriptor.

        Format: 1 byte (extensible, but usually 1 byte)
        - Bits 8-6: TYP (Target Report Type)
        - Bit 5: SIM (Simulated target)
        - Bit 4: RDP (Report from RDP chain)
        - Bit 3: SPI (Special Position Identification)
        - Bit 2: RAB (Report from aircraft or field monitor)
        - Bit 1: FX (Extension indicator)

        Returns:
            (descriptor_dict, bytes_consumed)
        """
        if offset >= len(data):
            raise DecoderError("I020 truncated")

        byte1 = data[offset]
        bytes_consumed = 1

        # Parse bits
        typ = (byte1 >> 5) & 0x07  # Bits 8-6
        sim = bool(byte1 & 0b00010000)  # Bit 5
        rdp = bool(byte1 & 0b00001000)  # Bit 4
        spi = bool(byte1 & 0b00000100)  # Bit 3
        rab = bool(byte1 & 0b00000010)  # Bit 2
        fx = bool(byte1 & 0x01)  # Bit 1

        # Decode TYP field
        typ_map = {
            0: "No detection",
            1: "Single SSR detection",
            2: "Single PSR detection",
            3: "SSR + PSR detection",
            4: "Single ModeS All-Call",
            5: "Single ModeS Roll-Call",
            6: "ModeS All-Call + PSR",
            7: "ModeS Roll-Call + PSR",
        }

        result = {
            'TYP': typ,
            'SIM': sim,
            'RDP': rdp,
            'SPI': spi,
            'RAB': rab,
        }

        if self.verbose:
            result['TYP_description'] = typ_map.get(typ, "Unknown")

        # Handle extension (rare, but possible)
        if fx and offset + 1 < len(data):
            # Extension byte exists (not commonly used)
            bytes_consumed += 1

        return result, bytes_consumed

    def _decode_i040(self, data: bytes, offset: int) -> Tuple[Dict[str, float], int]:
        """
        Decode I040: Measured Position in Polar Coordinates.

        Format: 4 bytes
        - Bytes 1-2: RHO (slant range in 1/256 NM)
        - Bytes 3-4: THETA (azimuth in 360/2^16 degrees)

        Returns:
            ({'RHO': float, 'THETA': float}, 4)
        """
        if offset + 4 > len(data):
            raise DecoderError("I040 truncated")

        rho_raw = decode_uint(data, offset, 2)
        theta_raw = decode_uint(data, offset + 2, 2)

        # Convert to physical units
        rho_nm = rho_raw / 256.0  # Nautical miles
        theta_deg = theta_raw * 360.0 / 65536.0  # Degrees

        result = {
            'RHO': rho_nm,
            'THETA': theta_deg,
        }

        if self.verbose:
            rho_m = rho_nm * 1852.0  # Convert to meters
            result['RHO_m'] = rho_m
            result['description'] = f"Range: {rho_nm:.2f} NM ({rho_m:.0f} m), Azimuth: {theta_deg:.2f}°"

        return result, 4

    def _decode_i070(self, data: bytes, offset: int) -> Tuple[Dict[str, Any], int]:
        """
        Decode I070: Mode-3/A Code in Octal Representation.

        Format: 2 bytes
        - Bit 16: V (validated)
        - Bit 15: G (garbled)
        - Bit 14: L (smoothed)
        - Bit 13: Spare (0)
        - Bits 12-1: Mode-3/A code (12 bits in octal)

        Returns:
            ({'code': int, 'V': bool, 'G': bool, 'L': bool}, 2)
        """
        if offset + 2 > len(data):
            raise DecoderError("I070 truncated")

        value = decode_uint(data, offset, 2)

        v = bool(value & 0x8000)  # Bit 16
        g = bool(value & 0x4000)  # Bit 15
        l = bool(value & 0x2000)  # Bit 14
        code = value & 0x0FFF  # Bits 12-1

        result = {
            'V': v,
            'G': g,
            'L': l,
            'code': code,
        }

        if self.verbose:
            # Convert to octal string for display
            octal_str = f"{code:04o}"
            result['octal'] = octal_str
            result['description'] = f"Mode 3/A: {octal_str}"
            if not v:
                result['description'] += " (not validated)"
            if g:
                result['description'] += " (garbled)"

        return result, 2

    def _decode_i090(self, data: bytes, offset: int) -> Tuple[Dict[str, Any], int]:
        """
        Decode I090: Flight Level in Binary Representation.

        Format: 2 bytes
        - Bit 16: V (validated)
        - Bit 15: G (garbled)
        - Bits 14-1: Flight level (1/4 FL, two's complement)

        Returns:
            ({'FL': float, 'V': bool, 'G': bool}, 2)
        """
        if offset + 2 > len(data):
            raise DecoderError("I090 truncated")

        value = decode_uint(data, offset, 2)

        v = bool(value & 0x8000)  # Bit 16
        g = bool(value & 0x4000)  # Bit 15

        # Bits 14-1: Flight level (two's complement)
        fl_raw = value & 0x3FFF
        if fl_raw & 0x2000:  # Sign bit
            fl_raw = fl_raw - 0x4000

        fl = fl_raw / 4.0  # 1/4 FL resolution

        result = {
            'V': v,
            'G': g,
            'FL': fl,
        }

        if self.verbose:
            result['description'] = f"Flight Level: {fl:.2f}"
            if not v:
                result['description'] += " (not validated)"
            if g:
                result['description'] += " (garbled)"

        return result, 2

    def _decode_i130(self, data: bytes, offset: int) -> Tuple[Dict[str, Any], int]:
        """
        Decode I130: Radar Plot Characteristics (Compound Item).

        Format: Variable length compound item
        - Byte 1: Primary subfield (indicates which subfields present)
        - Subsequent bytes: Subfield data

        Subfields:
        - Bit 7 (spare)
        - Bit 6: SRL (SSR plot runlength)
        - Bit 5: SRR (Number of received replies)
        - Bit 4: SAM (Amplitude of received replies)
        - Bit 3: PRL (Primary plot runlength)
        - Bit 2: PAM (Primary plot amplitude)
        - Bit 1: RPD (Difference in range)
        - Bit 0: FX (extension)

        Returns:
            (i130_dict, bytes_consumed)
        """
        if offset >= len(data):
            raise DecoderError("I130 truncated")

        primary = data[offset]
        bytes_consumed = 1
        result = {}

        # Check which subfields are present
        if primary & 0b01000000:  # SRL
            if offset + bytes_consumed >= len(data):
                raise DecoderError("I130 SRL truncated")
            srl_raw = data[offset + bytes_consumed]
            srl = (srl_raw / 255.0) * 360.0  # Convert to degrees
            result['SRL'] = srl
            bytes_consumed += 1

        if primary & 0b00100000:  # SRR
            if offset + bytes_consumed >= len(data):
                raise DecoderError("I130 SRR truncated")
            result['SRR'] = data[offset + bytes_consumed]
            bytes_consumed += 1

        if primary & 0b00010000:  # SAM
            if offset + bytes_consumed >= len(data):
                raise DecoderError("I130 SAM truncated")
            sam_raw = decode_int(data, offset + bytes_consumed, 1)
            result['SAM'] = sam_raw  # dBm
            bytes_consumed += 1

        if primary & 0b00001000:  # PRL
            if offset + bytes_consumed >= len(data):
                raise DecoderError("I130 PRL truncated")
            prl_raw = data[offset + bytes_consumed]
            prl = (prl_raw / 255.0) * 360.0  # Convert to degrees
            result['PRL'] = prl
            bytes_consumed += 1

        if primary & 0b00000100:  # PAM
            if offset + bytes_consumed >= len(data):
                raise DecoderError("I130 PAM truncated")
            pam_raw = decode_int(data, offset + bytes_consumed, 1)
            result['PAM'] = pam_raw  # dBm
            bytes_consumed += 1

        if primary & 0b00000010:  # RPD
            if offset + bytes_consumed >= len(data):
                raise DecoderError("I130 RPD truncated")
            rpd_raw = decode_int(data, offset + bytes_consumed, 1)
            result['RPD'] = rpd_raw * 1.0 / 256.0  # NM
            bytes_consumed += 1

        # Handle extension (FX bit)
        if primary & 0x01:
            # Extension exists (not commonly used)
            if offset + bytes_consumed >= len(data):
                raise DecoderError("I130 extension truncated")
            # Skip extension for now
            bytes_consumed += 1

        return result, bytes_consumed

    def _decode_i220(self, data: bytes, offset: int) -> Tuple[int, int]:
        """
        Decode I220: Aircraft Address (Mode S 24-bit address).

        Format: 3 bytes
        - 24-bit Mode S address

        Returns:
            (address, 3)
        """
        if offset + 3 > len(data):
            raise DecoderError("I220 truncated")

        address = decode_uint(data, offset, 3)

        if self.verbose:
            return {
                'address': address,
                'hex': f"0x{address:06X}",
                'description': f"Mode S Address: 0x{address:06X}",
            }, 3
        else:
            return address, 3

    def _decode_i240(self, data: bytes, offset: int) -> Tuple[str, int]:
        """
        Decode I240: Aircraft Identification.

        Format: 6 bytes (48 bits)
        - 8 characters, 6 bits each (6-bit ASCII)

        Returns:
            (callsign_string, 6)
        """
        if offset + 6 > len(data):
            raise DecoderError("I240 truncated")

        # Decode 8 characters of 6-bit ASCII
        callsign = decode_6bit_ascii(data, offset, 8)

        if self.verbose:
            return {
                'callsign': callsign.strip(),
                'description': f"Callsign: {callsign.strip()}",
            }, 6
        else:
            return callsign.strip(), 6

    def _decode_i250(self, data: bytes, offset: int) -> Tuple[Dict[str, Any], int]:
        """
        Decode I250: Mode S MB Data.

        Format: Variable length (repetitive item)
        - Byte 1: REP (repetition factor, number of MB data blocks)
        - Following bytes: MB data (8 bytes per block)

        Returns:
            (mb_data_dict, bytes_consumed)
        """
        if offset >= len(data):
            raise DecoderError("I250 truncated")

        rep = data[offset]
        bytes_consumed = 1

        mb_blocks = []

        for i in range(rep):
            if offset + bytes_consumed + 8 > len(data):
                raise DecoderError(f"I250 MB block {i} truncated")

            mb_data = data[offset + bytes_consumed:offset + bytes_consumed + 8]
            mb_blocks.append(mb_data.hex())
            bytes_consumed += 8

        result = {
            'REP': rep,
            'MB_DATA': mb_blocks,
        }

        return result, bytes_consumed


def decode_cat048(data: bytes, verbose: bool = True) -> list:
    """
    Decode ASTERIX CAT048 data (convenience function).

    Args:
        data: Raw ASTERIX CAT048 binary data
        verbose: Include descriptions and metadata

    Returns:
        List of decoded records

    Example:
        >>> records = decode_cat048(asterix_data)
        >>> print(records[0]['I040'])
        {'RHO': 27.0, 'THETA': 135.5}
    """
    decoder = CAT048Decoder(verbose=verbose)
    return decoder.decode_datablock(data)
