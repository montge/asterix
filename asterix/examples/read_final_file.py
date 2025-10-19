#!/usr/bin/env python3
"""
Parse ASTERIX data from FINAL format files.

This example demonstrates how to parse ASTERIX data encapsulated in the FINAL
(Croatia Control proprietary) packet format. FINAL is a custom framing protocol
that adds metadata to ASTERIX packets, including:
- Timestamp information (time of day in 10ms resolution)
- Board and line identifiers (hardware source information)
- Recording day number
- Packet size information

The FINAL format is structured as:
    [Header: 7 bytes] [ASTERIX Data: N bytes] [Padding: 4 bytes (0xA5A5A5A5)]

Header Structure:
    - Bytes 0-1: Data size (big-endian 16-bit)
    - Byte 2: Board number
    - Byte 3: Line number
    - Byte 4: Recording day
    - Bytes 5-7: Timestamp (3 bytes, 10ms resolution)

Usage:
    python read_final_file.py

    IMPORTANT: Before running, update the sample_filename path to point
    to your actual FINAL format file.

Requirements:
    - asterix_decoder package installed (pip install asterix_decoder)
    - A FINAL format file (.ff extension)

Example Output:
    1.	time=0:12:34.560	Size=48	board=1	line=2	day=123)
    [{'cat': 48, 'len': 48, 'records': [...]}]

    2.	time=0:12:34.570	Size=52	board=1	line=2	day=123)
    [{'cat': 48, 'len': 52, 'records': [...]}]
    ...
    Finished

Error Handling:
    The script will exit with an error if:
    - The file path is not updated (still points to placeholder)
    - Padding validation fails (wrong padding bytes)
    - File format is incorrect

See Also:
    - read_raw_file.py: Parse raw ASTERIX without encapsulation
    - read_pcap_file.py: Parse PCAP encapsulated ASTERIX
"""

__author__ = 'dsalanti'

import asterix
import struct
import datetime
import sys
import os


def parse_final_header(header_bytes):
    """
    Parse FINAL format packet header.

    Args:
        header_bytes: 7 bytes representing the FINAL header

    Returns:
        tuple: (data_size, board_nr, line_nr, rec_day, timestamp_ms)
            - data_size (int): Total packet size including header and padding
            - board_nr (int): Board number (hardware identifier)
            - line_nr (int): Line number (communication line identifier)
            - rec_day (int): Recording day number
            - timestamp_ms (int): Time of day in milliseconds

    The timestamp is encoded as 3 bytes representing 10ms units,
    converted to milliseconds for easier handling.
    """
    final_header_format = ">hBBBBBB"
    (data_size, board_nr, line_nr, rec_day, time1, time2, time3) = struct.unpack(
        final_header_format, header_bytes
    )

    # Convert 3-byte timestamp (10ms units) to milliseconds
    # Formula: (time1 * 65536 + time2 * 256 + time3) * 10
    timems = (time1 * 65536 + time2 * 256 + time3) * 10

    return data_size, board_nr, line_nr, rec_day, timems


def validate_padding(padding_bytes):
    """
    Validate FINAL format padding bytes.

    FINAL packets end with 4 padding bytes, all set to 0xA5.
    This serves as a packet integrity check.

    Args:
        padding_bytes: 4 bytes that should contain the padding pattern

    Returns:
        bool: True if padding is valid, False otherwise
    """
    final_padding_value = (0xA5, 0xA5, 0xA5, 0xA5)
    padding = struct.unpack("BBBB", padding_bytes)
    return padding == final_padding_value


def main():
    """
    Parse a FINAL format file and extract ASTERIX data.

    This function:
    1. Opens a FINAL format file
    2. Reads packets in a loop:
       - Parses the 7-byte header
       - Extracts timestamp and metadata
       - Reads the ASTERIX data payload
       - Validates the 4-byte padding
    3. Parses each ASTERIX payload
    4. Displays results with metadata

    Note:
        You MUST update the sample_filename path before running this script.
        The placeholder path will cause a FileNotFoundError.

    Raises:
        FileNotFoundError: If the sample file doesn't exist
        struct.error: If file format is incorrect
        SystemExit: If padding validation fails
    """
    # NOTE: Update this path to point to your actual FINAL format file
    sample_filename = 'path/to/your/sample_file.ff'  # Example path - update before use

    # Validate that the user has updated the path
    if sample_filename == 'path/to/your/sample_file.ff' or not os.path.exists(sample_filename):
        print("ERROR: Please update the sample_filename path to point to your FINAL format file")
        print("Example: sample_filename = '/home/user/data/asterix_recording.ff'")
        sys.exit(1)

    # FINAL format constants
    final_header_size = 7  # bytes
    final_padding_size = 4  # bytes

    with open(sample_filename, "rb") as f:
        packetnr = 1

        # Read packets until end of file
        while True:
            # Read header
            header_bytes = f.read(final_header_size)
            if len(header_bytes) != final_header_size:
                # End of file or incomplete packet
                break

            # Parse header to extract metadata
            data_size, board_nr, line_nr, rec_day, timems = parse_final_header(header_bytes)

            # Calculate ASTERIX payload size
            # Total size minus header and padding
            packet_size = data_size - final_header_size - final_padding_size

            # Convert milliseconds to timedelta for readable display
            timestamp = datetime.timedelta(milliseconds=timems)

            # Display packet metadata
            print("%d.\ttime=%s\tSize=%d\tboard=%d\tline=%d\tday=%d)" % (
                packetnr, timestamp, packet_size, board_nr, line_nr, rec_day
            ))
            packetnr += 1

            # Read ASTERIX payload
            packet_bytes = f.read(packet_size)

            # Parse ASTERIX data
            # Returns list of data blocks
            packets = asterix.parse(packet_bytes)

            # Display each data block
            for packet in packets:
                print(packet)

            # Read and validate padding
            padding_bytes = f.read(4)
            if not validate_padding(padding_bytes):
                print("ERROR: Wrong padding! Expected 0xA5A5A5A5")
                print("This indicates file corruption or incorrect file format")
                sys.exit(1)

    print("Finished")


if __name__ == '__main__':
    main()
