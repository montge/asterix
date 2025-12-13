"""
Base ASTERIX Decoder Classes

Provides abstract base class and common utilities for ASTERIX decoding.
All category-specific decoders inherit from AsterixDecoder.

Key components:
- AsterixDecoder: Abstract base class for category decoders
- decode_fspec: Parse Field Specification (FSPEC) bytes
- DecoderError: Custom exception for decoding errors

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

import struct
from abc import ABC, abstractmethod
from typing import List, Tuple, Dict, Any


class DecoderError(Exception):
    """Custom exception for ASTERIX decoding errors."""
    pass


def decode_fspec(data: bytes, offset: int = 0) -> Tuple[List[int], int]:
    """
    Decode Field Specification (FSPEC) from ASTERIX data.

    FSPEC uses variable-length encoding:
    - Each octet has 7 data bits (bits 8-2) indicating presence of FRNs
    - Bit 1 is the FX (extension) bit: 1=more octets, 0=last octet
    - FRN numbering: octet 1 = FRN 1-7, octet 2 = FRN 8-14, etc.

    Args:
        data: Binary data containing FSPEC
        offset: Starting offset in data (default: 0)

    Returns:
        Tuple of (frn_list, bytes_consumed)
        - frn_list: List of present FRN numbers (e.g., [1, 2, 4, 6])
        - bytes_consumed: Number of bytes read from data

    Raises:
        DecoderError: If data is too short or malformed

    Example:
        >>> # FSPEC: 0xE0 = 0b11100000 (FRNs 1, 2, 3 present, no extension)
        >>> frns, consumed = decode_fspec(b'\\xe0\\x12\\x34')
        >>> print(frns)
        [1, 2, 3]
        >>> print(consumed)
        1
    """
    if offset >= len(data):
        raise DecoderError(f"FSPEC offset {offset} >= data length {len(data)}")

    frn_list = []
    octet_index = 0
    bytes_consumed = 0

    while True:
        if offset + bytes_consumed >= len(data):
            raise DecoderError(
                f"FSPEC truncated at offset {offset + bytes_consumed}"
            )

        octet = data[offset + bytes_consumed]
        bytes_consumed += 1

        # Parse bits 8-2 (7 bits) for FRN presence
        for bit_index in range(7):
            bit_mask = 1 << (7 - bit_index)  # Bit 8, 7, 6, ..., 2
            if octet & bit_mask:
                frn = octet_index * 7 + bit_index + 1
                frn_list.append(frn)

        # Check FX bit (bit 1)
        fx_bit = octet & 0x01
        if not fx_bit:
            # No more FSPEC octets
            break

        octet_index += 1

        # Safety: prevent infinite loops
        if octet_index > 100:
            raise DecoderError("FSPEC too long (>100 octets)")

    return frn_list, bytes_consumed


class AsterixDecoder(ABC):
    """
    Abstract base class for ASTERIX category decoders.

    Each ASTERIX category (CAT048, CAT062, etc.) implements a subclass that
    provides category-specific decoding logic. This base class handles common
    operations like FSPEC parsing and data block structure.

    Attributes:
        category: ASTERIX category number (e.g., 48, 62, 21)
        verbose: If True, include descriptions and metadata in output
        frn_map: Mapping from FRN number to data item name (e.g., {1: 'I010', 4: 'I040'})

    Subclasses must implement:
        - _decode_data_item(frn, data, offset): Decode a single data item
        - _init_frn_map(): Initialize the FRN to item name mapping
    """

    def __init__(self, category: int, verbose: bool = True):
        """
        Initialize decoder.

        Args:
            category: ASTERIX category number
            verbose: Include descriptions and metadata in output
        """
        self.category = category
        self.verbose = verbose
        self.frn_map: Dict[int, str] = {}
        self._init_frn_map()

    @abstractmethod
    def _init_frn_map(self) -> None:
        """
        Initialize FRN to data item name mapping.

        Must set self.frn_map dictionary mapping FRN numbers to item names.
        Example: {1: 'I010', 2: 'I140', 4: 'I040', ...}
        """
        pass

    @abstractmethod
    def _decode_data_item(
        self, frn: int, data: bytes, offset: int
    ) -> Tuple[Any, int]:
        """
        Decode a single data item.

        Args:
            frn: Field Reference Number
            data: Binary data buffer
            offset: Current offset in data

        Returns:
            Tuple of (decoded_value, bytes_consumed)
            - decoded_value: Decoded data (dict, int, float, etc.)
            - bytes_consumed: Number of bytes read

        Raises:
            DecoderError: If decoding fails
        """
        pass

    def decode_record(
        self, data: bytes, offset: int = 0
    ) -> Tuple[Dict[str, Any], int]:
        """
        Decode a single ASTERIX data record.

        A record consists of:
        1. FSPEC (variable length)
        2. Data items (in order of FRN appearance)

        Args:
            data: Binary data buffer
            offset: Starting offset in data

        Returns:
            Tuple of (record_dict, bytes_consumed)
            - record_dict: Decoded record with 'category' and data items
            - bytes_consumed: Total bytes read for this record

        Raises:
            DecoderError: If record is malformed
        """
        if offset >= len(data):
            raise DecoderError(f"Record offset {offset} >= data length {len(data)}")

        record_start = offset

        # Parse FSPEC
        frn_list, fspec_len = decode_fspec(data, offset)
        offset += fspec_len

        # Initialize record
        record = {
            'category': self.category,
            'len': 0,  # Will be updated at end
        }

        # Decode each data item in FRN order
        for frn in frn_list:
            item_name = self.frn_map.get(frn, f'I{frn:03d}')

            try:
                decoded_value, item_len = self._decode_data_item(frn, data, offset)
                record[item_name] = decoded_value
                offset += item_len

            except Exception as e:
                # Try to recover gracefully
                if self.verbose:
                    record[item_name] = {'error': str(e)}
                raise DecoderError(
                    f"Failed to decode {item_name} (FRN {frn}) at offset {offset}: {e}"
                )

        # Update record length
        bytes_consumed = offset - record_start
        record['len'] = bytes_consumed

        return record, bytes_consumed

    def decode_datablock(self, data: bytes) -> List[Dict[str, Any]]:
        """
        Decode a complete ASTERIX data block.

        Data block structure:
        - Byte 0: CAT (category number)
        - Bytes 1-2: LEN (total length including header)
        - Bytes 3-N: Data records

        Args:
            data: Complete ASTERIX data block

        Returns:
            List of decoded records

        Raises:
            DecoderError: If data block is invalid
        """
        if len(data) < 3:
            raise DecoderError(f"Data block too short: {len(data)} bytes")

        # Parse header
        category = data[0]
        length = struct.unpack('!H', data[1:3])[0]

        if category != self.category:
            raise DecoderError(
                f"Category mismatch: expected {self.category}, got {category}"
            )

        if length > len(data):
            raise DecoderError(
                f"Data block length {length} exceeds actual data {len(data)}"
            )

        # Decode records
        records = []
        offset = 3  # Start after header

        while offset < length:
            try:
                record, record_len = self.decode_record(data, offset)
                records.append(record)
                offset += record_len

            except DecoderError as e:
                # Stop on error (don't try to continue with corrupted data)
                raise DecoderError(
                    f"Failed to decode record at offset {offset}: {e}"
                )

        return records

    def decode_multiple_blocks(self, data: bytes) -> List[Dict[str, Any]]:
        """
        Decode multiple consecutive ASTERIX data blocks.

        Args:
            data: Binary data containing one or more data blocks

        Returns:
            Combined list of all decoded records from all blocks

        Raises:
            DecoderError: If any block is invalid
        """
        all_records = []
        offset = 0

        while offset < len(data):
            if len(data) - offset < 3:
                # Not enough data for another block
                break

            # Get block length
            length = struct.unpack('!H', data[offset + 1:offset + 3])[0]

            if offset + length > len(data):
                raise DecoderError(
                    f"Block at offset {offset} extends beyond data (length {length})"
                )

            # Decode this block
            block_data = data[offset:offset + length]
            records = self.decode_datablock(block_data)
            all_records.extend(records)

            offset += length

        return all_records


# Utility functions for common decoding patterns

def decode_uint(data: bytes, offset: int, length: int) -> int:
    """Decode unsigned integer (big-endian)."""
    if offset + length > len(data):
        raise DecoderError(f"Cannot read {length} bytes at offset {offset}")
    return int.from_bytes(data[offset:offset + length], byteorder='big', signed=False)


def decode_int(data: bytes, offset: int, length: int) -> int:
    """Decode signed integer (big-endian, two's complement)."""
    if offset + length > len(data):
        raise DecoderError(f"Cannot read {length} bytes at offset {offset}")
    return int.from_bytes(data[offset:offset + length], byteorder='big', signed=True)


def decode_octal(data: bytes, offset: int, length: int, mask: int) -> int:
    """Decode octal value (common in ASTERIX for Mode 3/A codes)."""
    value = decode_uint(data, offset, length) & mask
    return value


def decode_6bit_ascii(data: bytes, offset: int, num_chars: int) -> str:
    """
    Decode 6-bit ASCII characters (common in ASTERIX).

    6-bit ASCII: A-Z = 1-26, space = 32, 0-9 = 48-57
    """
    if offset + ((num_chars * 6 + 7) // 8) > len(data):
        raise DecoderError(f"Cannot read {num_chars} 6-bit chars at offset {offset}")

    chars = []
    bit_offset = offset * 8

    for i in range(num_chars):
        # Extract 6 bits
        byte_idx = bit_offset // 8
        bit_idx = bit_offset % 8

        if bit_idx <= 2:
            # 6 bits fit in current byte
            value = (data[byte_idx] >> (2 - bit_idx)) & 0x3F
        else:
            # 6 bits span two bytes
            high_bits = (data[byte_idx] << (bit_idx - 2)) & 0x3F
            low_bits = (data[byte_idx + 1] >> (10 - bit_idx)) & 0x3F
            value = high_bits | low_bits

        # Convert to ASCII
        if value == 32:
            chars.append(' ')
        elif 1 <= value <= 26:
            chars.append(chr(ord('A') + value - 1))
        elif 48 <= value <= 57:
            chars.append(chr(ord('0') + value - 48))
        else:
            chars.append('?')

        bit_offset += 6

    return ''.join(chars)


def bytes_consumed_for_fspec(frn_list: List[int]) -> int:
    """Calculate how many bytes a given FRN list would consume in FSPEC."""
    if not frn_list:
        return 0
    max_frn = max(frn_list)
    return (max_frn + 6) // 7
