#!/usr/bin/env python3
"""
Parse raw ASTERIX data files and demonstrate parsing options.

This example shows how to parse raw ASTERIX binary files (files containing
pure ASTERIX data without additional encapsulation). It demonstrates three
different parsing modes:

1. Verbose mode (default): Includes field descriptions and meanings
2. Compact mode: Returns only field names and values
3. Textual description: Human-readable formatted output

This is the most common use case for parsing ASTERIX data from files that
contain concatenated ASTERIX data blocks, such as recordings from radar
systems or data loggers.

Usage:
    python read_raw_file.py

Requirements:
    - asterix_decoder package installed (pip install asterix_decoder)

Example Output:
    asterix_decoder version: X.Y.Z

    Items with description
    ----------------------
    ('I010', {'SAC': (1, 'System Area Code'), 'SIC': (2, 'System Identification Code')})
    ('I015', {'SI': (3, 'Service Identification')})
    ...

    Items without description
    ----------------------
    ('I010', {'SAC': 1, 'SIC': 2})
    ('I015', {'SI': 3})
    ...

    Textual description of data
    ----------------------
    Data Block 1/2
      Category: 062
      Length: 156 bytes
      Records: 1
        Record 1:
          I010 (Data Source Identifier)
            SAC: 1
            SIC: 2
          ...

ASTERIX Categories in Sample:
    - CAT062: System Track Data (processed multi-sensor data)
    - CAT065: SDPS (Surveillance Data Processing System) Status Messages

See Also:
    - read_raw_bytes.py: Parse ASTERIX data from a byte array
    - read_pcap_file.py: Parse ASTERIX from PCAP network captures
    - read_final_file.py: Parse FINAL format encapsulated files
"""

__author__ = 'dsalanti'

import asterix


def main():
    """
    Parse a raw ASTERIX file and demonstrate different output modes.

    This function demonstrates:
    1. Loading a raw ASTERIX file from package resources
    2. Parsing with verbose mode (includes descriptions)
    3. Parsing with compact mode (values only)
    4. Generating human-readable text descriptions

    The sample file contains:
    - Multiple ASTERIX data blocks
    - CAT062 (System Track Data) records
    - CAT065 (SDPS Status Messages) records

    Each data block may contain multiple records, and each record
    contains multiple data items (fields).
    """
    # Display library version for troubleshooting
    print(asterix.__version__)

    # Get sample file from package resources
    # This file contains raw ASTERIX binary data (CAT062 and CAT065)
    sample_filename = asterix.get_sample_file('cat062cat065.raw')

    with open(sample_filename, "rb") as f:
        # Read entire file into memory
        data = f.read()

        # ===== Parsing Mode 1: Verbose (with descriptions) =====
        # Each field value is returned as a tuple: (value, description)
        # This provides context about what each field means
        print('Items with description')
        print('----------------------')
        parsed = asterix.parse(data)
        for packet in parsed:
            # Each packet is a data block containing multiple records
            for item in packet.items():
                # item is a tuple: (field_name, field_data)
                print(item)

        # ===== Parsing Mode 2: Compact (without descriptions) =====
        # Each field value is returned as just the value
        # This is more efficient and cleaner for programmatic use
        print('Items without description')
        print('----------------------')
        parsed = asterix.parse(data, verbose=False)
        for packet in parsed:
            for item in packet.items():
                print(item)

        # ===== Parsing Mode 3: Textual description =====
        # Convert parsed data to human-readable formatted text
        # This is ideal for logging, debugging, or displaying to users
        print('Textual description of data')
        print('----------------------')
        formatted = asterix.describe(parsed)
        print(formatted)


if __name__ == '__main__':
    main()
