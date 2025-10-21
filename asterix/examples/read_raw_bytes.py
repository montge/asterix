#!/usr/bin/env python3
"""
Parse raw ASTERIX binary data from a byte array.

This example demonstrates the simplest form of ASTERIX parsing - taking a raw
byte array representing a single ASTERIX packet and parsing it into a structured
format. This is useful when you have ASTERIX data already loaded into memory or
received from a network socket.

The example uses a CAT048 (Monoradar Target Reports) packet, which contains
surveillance data from a single radar sensor including position, velocity, and
track information.

Usage:
    python read_raw_bytes.py

Requirements:
    - asterix_decoder package installed (pip install asterix_decoder)

Example Output:
    The script will output a human-readable description of the ASTERIX packet,
    including all data items and their decoded values. For CAT048, this typically
    includes:
    - Data Source Identifier (I010)
    - Time of Day (I140)
    - Target Report Descriptor (I020)
    - Measured Position in Polar Coordinates (I040)
    - Mode-3/A Code (I070)
    - Flight Level (I090)
    - Radar Plot Characteristics (I130)
    - Track Number (I161)
    - Track Status (I170)
    - Calculated Track Velocity (I200)

See Also:
    - read_raw_file.py: Parse ASTERIX data from a file
    - read_pcap_file.py: Parse ASTERIX data from PCAP network captures
"""

__author__ = 'dsalanti'

import asterix


def main():
    """
    Parse and display a hardcoded CAT048 ASTERIX packet.

    This function demonstrates the basic ASTERIX parsing workflow:
    1. Create a byte array with raw ASTERIX data
    2. Parse it using asterix.parse()
    3. Format and display the results using asterix.describe()

    The example packet structure:
    - Bytes 0-2: ASTERIX header (0x30 0x00 0x30 = CAT048, 48 bytes)
    - Bytes 3+: FSPEC (Field Specification) and data items

    Returns:
        None

    Example:
        >>> main()
        Data Block 1/1
          Category: 048 (Monoradar Target Reports)
          Length: 48 bytes
          Records: 1
            Record 1:
              I010 (Data Source Identifier)
                SAC: 2
                SIC: 25
              I140 (Time of Day)
                ToD: 12345.67 seconds
              ...

    Note:
        The example uses a predefined CAT048 packet. For parsing your own
        ASTERIX data, replace the asterix_packet bytearray with your data.
    """
    # This is binary presentation of asterix packet of CAT048
    #                  len  cat 048
    #                 ---- ---------
    asterix_packet = bytearray(
        [0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9, 0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
         0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10, 0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
         0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb, 0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5])

    # Parse the ASTERIX packet - returns a list of dictionaries
    # Each dictionary represents one data block with all its data items
    parsed = asterix.parse(asterix_packet)

    # Convert parsed data to human-readable text format
    # This includes field names, descriptions, and decoded values
    formatted = asterix.describe(parsed)
    print(formatted)


if __name__ == '__main__':
    main()
