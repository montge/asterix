"""
ASTERIX Decoder - Pure Python ASTERIX Binary Decoder

This package provides pure Python decoding functionality to convert ASTERIX binary
format back into structured data. Complements the encoder to make the suite bidirectional.

Supported categories:
- CAT048: Transmission of Monoradar Target Reports
- CAT062: System Track Data
- CAT021: ADS-B Target Reports

Key features:
- Pure Python implementation (no C dependencies)
- Compatible with C++ decoder output format
- Round-trip validation support (encode → decode → compare)
- Graceful error handling
- Educational value for learning ASTERIX protocol

Usage:
    from asterix.radar_integration.decoder import decode_asterix, CAT048Decoder

    # Decode any ASTERIX data (auto-detects category)
    records = decode_asterix(asterix_binary_data)

    # Use category-specific decoder
    decoder = CAT048Decoder()
    records = decoder.decode_datablock(asterix_binary_data)

    # Validate round-trip encoding
    from asterix.radar_integration.decoder.validator import validate_round_trip
    success, stats = validate_round_trip(original_plots, decoded_records)

Architecture:
- base.py: Base decoder classes and FSPEC parsing
- cat048.py: CAT048 decoder (monoradar target reports)
- cat062.py: CAT062 decoder (system track data)
- cat021.py: CAT021 decoder (ADS-B reports)
- validator.py: Round-trip validation utilities

---

License: GPL-3.0
Author: ASTERIX Integration Team
Date: 2025-11-23
"""

from .base import AsterixDecoder, decode_fspec, DecoderError
from .cat048 import CAT048Decoder, decode_cat048
from .cat062 import CAT062Decoder, decode_cat062
from .cat021 import CAT021Decoder, decode_cat021

# Auto-detect category decoder mapping
_CATEGORY_DECODERS = {
    48: CAT048Decoder,
    62: CAT062Decoder,
    21: CAT021Decoder,
}


def decode_asterix(data: bytes, verbose: bool = True) -> list:
    """
    Decode ASTERIX binary data (auto-detects category).

    This is the primary decoding function that automatically detects the ASTERIX
    category and uses the appropriate decoder. Compatible with asterix.parse() API.

    Args:
        data: Raw ASTERIX binary data (one or more data blocks)
        verbose: If True, include descriptions and metadata (default: True)

    Returns:
        List of decoded records (dicts matching C++ decoder format)

    Raises:
        DecoderError: If data is invalid or category unsupported

    Example:
        >>> from asterix.radar_integration.decoder import decode_asterix
        >>> records = decode_asterix(asterix_data)
        >>> print(records[0]['category'])
        48
        >>> print(records[0]['I040'])  # Position data
        {'RHO': 27.0, 'THETA': 135.5}
    """
    if not data or len(data) < 3:
        raise DecoderError(f"Data too short: {len(data)} bytes (minimum 3)")

    # Parse data block header
    category = data[0]

    # Select appropriate decoder
    decoder_class = _CATEGORY_DECODERS.get(category)
    if not decoder_class:
        raise DecoderError(f"Unsupported ASTERIX category: {category}")

    decoder = decoder_class(verbose=verbose)
    return decoder.decode_datablock(data)


__all__ = [
    'AsterixDecoder',
    'decode_fspec',
    'DecoderError',
    'CAT048Decoder',
    'decode_cat048',
    'CAT062Decoder',
    'decode_cat062',
    'CAT021Decoder',
    'decode_cat021',
    'decode_asterix',
]

__version__ = '0.1.0'
