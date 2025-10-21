#!/usr/bin/env python3
"""
Parse ASTERIX data from PCAP network capture files.

This example demonstrates how to extract and parse ASTERIX data from PCAP
(Packet Capture) files. PCAP is a common format for recording network traffic,
often used when ASTERIX data is transmitted over UDP/IP networks. This is
particularly useful for:
- Analyzing recorded network traffic from radar systems
- Debugging ASTERIX data transmission issues
- Replaying historical surveillance data

The example extracts UDP payloads from Ethernet/IP/UDP packets and parses
each payload as ASTERIX data.

Usage:
    python read_pcap_file.py

Requirements:
    - asterix_decoder package installed (pip install asterix_decoder)
    - dpkt package for PCAP parsing (pip install dpkt)

Example Output:
    For each UDP packet in the PCAP file:
    Parsing packet 1 : 30:00:1e:fd:f7:02:19:c7:13:84:...
    [{'cat': 48, 'len': 30, 'records': [...]}]

    Parsing packet 2 : 22:00:1a:fd:e0:02:19:c7:15:8a:...
    [{'cat': 34, 'len': 26, 'records': [...]}]

Network Stack:
    Ethernet Frame -> IP Packet -> UDP Datagram -> ASTERIX Data

    This example unpacks each layer to extract the ASTERIX payload.

See Also:
    - read_raw_file.py: Parse raw ASTERIX files without network encapsulation
    - read_final_file.py: Parse FINAL format files (alternative encapsulation)
    - multicast_receive.py: Receive ASTERIX data from live network multicast
"""

__author__ = 'dsalanti'

import asterix
import dpkt


def main():
    """
    Read and parse ASTERIX data from a PCAP file.

    This function demonstrates the complete workflow for PCAP processing:
    1. Load a sample PCAP file from the asterix package resources
    2. Iterate through each packet in the PCAP
    3. Extract the UDP payload (ASTERIX data)
    4. Parse and display the ASTERIX content

    The sample file contains CAT034 (Service Messages) and CAT048
    (Monoradar Target Reports) packets.

    Returns:
        None

    Raises:
        FileNotFoundError: If the sample PCAP file cannot be found
        dpkt.dpkt.NeedData: If PCAP file is corrupted or incomplete
        AttributeError: If packet structure doesn't match expected Ethernet/IP/UDP

    Example:
        >>> main()
        Parsing packet 1 : 30:00:1e:fd:f7:02:19:c7:13:84:...
        [{'cat': 48, 'len': 30, 'records': [{'010': {'SAC': 2, 'SIC': 25}, ...}]}]

        Parsing packet 2 : 22:00:1a:fd:e0:02:19:c7:15:8a:...
        [{'cat': 34, 'len': 26, 'records': [...]}]

    Note:
        To parse your own PCAP files, replace the get_sample_file() call
        with your file path:

        >>> with open('/path/to/your/file.pcap', 'rb') as f:
        >>>     pcap = dpkt.pcap.Reader(f)
        >>>     # ... rest of processing
    """
    # Get the path to the bundled sample PCAP file
    # This file contains mixed CAT034 and CAT048 packets
    sample_filename = asterix.get_sample_file('cat_034_048.pcap')

    with open(sample_filename, 'rb') as f:
        # Create PCAP reader - this handles the PCAP file format
        pcap = dpkt.pcap.Reader(f)

        cntr = 1
        # Iterate through packets: ts = timestamp, buf = packet bytes
        for ts, buf in pcap:
            # Unpack network layers:
            # Ethernet frame contains the IP packet
            eth = dpkt.ethernet.Ethernet(buf)

            # IP packet contains the UDP datagram
            # UDP datagram contains the ASTERIX data as payload
            data = eth.ip.udp.data

            # Format packet data as hex string for display (colon-separated)
            hexdata = ":".join("{:02x}".format(ord(c)) for c in str(data))
            print('Parsing packet %d : %s' % (cntr, hexdata))
            cntr += 1

            # Parse the ASTERIX data from UDP payload
            # Returns list of data blocks, each with category, length, and records
            parsed = asterix.parse(data)
            print(parsed)


if __name__ == '__main__':
    main()
