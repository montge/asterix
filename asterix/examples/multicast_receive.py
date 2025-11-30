#!/usr/bin/env python3
"""
Receive and parse ASTERIX data from UDP multicast streams.

This example demonstrates how to receive ASTERIX data transmitted over UDP
multicast networks. Multicast is commonly used in Air Traffic Management (ATM)
systems to distribute surveillance data from radar sensors to multiple
processing systems simultaneously.

Network Multicast Basics:
    - Multicast uses special IP addresses (224.0.0.0 to 239.255.255.255)
    - Multiple receivers can join the same multicast group
    - Data sent to the group is received by all members
    - More efficient than sending individual copies to each receiver

This example:
    - Joins multicast group 232.1.1.11 on port 21111
    - Receives UDP packets containing ASTERIX data
    - Parses each packet and displays the results
    - Runs indefinitely (press Ctrl+C to stop)

Usage:
    python multicast_receive.py

    The script will run continuously, printing each received packet.
    Press Ctrl+C to stop.

Requirements:
    - asterix_decoder package installed (pip install asterix_decoder)
    - Network access to multicast group 232.1.1.11
    - Firewall rules allowing UDP port 21111
    - A multicast sender transmitting ASTERIX data

Example Output:
    1. Receiver received = [{'cat': 48, 'len': 48, 'records': [...]}]
    2. Receiver received = [{'cat': 48, 'len': 48, 'records': [...]}]
    3. Receiver received = [{'cat': 34, 'len': 26, 'records': [...]}]
    ...

Network Configuration:
    Multicast Group: 232.1.1.11
    Port: 21111
    Protocol: UDP
    Interface: All interfaces (INADDR_ANY)

Security Notes:
    - This example binds to all network interfaces ('') as required for multicast
    - In production, consider:
      * Firewall rules to restrict access to port 21111
      * Running in isolated network segments for ATM traffic
      * Binding to specific interface IP if multicast is on known interface
      * Implementing application-level authentication

See Also:
    - multicast_send_receive.py: Combined sender and receiver example
    - read_pcap_file.py: Parse recorded multicast traffic from PCAP files
"""

__author__ = 'dsalanti'

import socket
import struct
import asterix


def setup_multicast_receiver(multicast_group, port):
    """
    Configure and bind a socket for receiving multicast UDP packets.

    This function sets up a UDP socket with the necessary options for
    multicast reception:
    1. Creates a UDP socket
    2. Enables address reuse (allows multiple processes to bind same port)
    3. Binds to all interfaces on the specified port
    4. Joins the multicast group

    Args:
        multicast_group (str): Multicast IP address (e.g., "232.1.1.11")
            Must be in the range 224.0.0.0 to 239.255.255.255
        port (int): UDP port number (e.g., 21111)

    Returns:
        socket.socket: Configured socket ready to receive multicast packets

    Raises:
        socket.error: If socket setup fails or multicast join fails
        OSError: If port is already in use without SO_REUSEADDR

    Example:
        >>> sock = setup_multicast_receiver("232.1.1.11", 21111)
        >>> data = sock.recv(10240)  # Receive up to 10KB
        >>> print(f"Received {len(data)} bytes")
        Received 48 bytes

    Note:
        The socket binds to '' (all interfaces) which is required for multicast
        to work correctly. This is a security consideration in production.

        Multicast IP Address Ranges:
        - 224.0.0.0 - 224.0.0.255: Reserved for local network control
        - 224.0.1.0 - 238.255.255.255: Globally scoped multicast
        - 239.0.0.0 - 239.255.255.255: Administratively scoped (organization-local)
    """
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

    # Allow multiple processes to bind to the same port
    # This is useful when running multiple receivers
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Bind to all interfaces on the specified port
    # Security Note: Binding to all interfaces ('') is required for multicast
    # reception to work correctly. In production deployments, consider:
    # 1. Using firewall rules to restrict access to the port
    # 2. Running in isolated network segments for ASTERIX traffic
    # 3. Implementing application-level authentication if needed
    # 4. Binding to specific interface IP if multicast is on a known interface
    # Binding to all interfaces is required for multicast reception
    # Security: Documented above, use firewall rules in production
    # CodeQL suppression: Required for multicast - use network isolation in production
    sock.bind(('', port))  # codeql[py/bind-socket-all-network-interfaces] lgtm nosec B104

    # Join the multicast group
    # mreq = 4 bytes (multicast address) + 4 bytes (interface address)
    # INADDR_ANY means join on all available interfaces
    mreq = struct.pack("=4sl", socket.inet_aton(multicast_group), socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    return sock


def main():
    """
    Receive and parse ASTERIX packets from multicast network.

    This function:
    1. Sets up a multicast receiver socket
    2. Enters an infinite loop to receive packets
    3. Parses each packet as ASTERIX data
    4. Displays the parsed results with a counter

    The loop runs indefinitely until interrupted (Ctrl+C).

    Returns:
        None

    Raises:
        KeyboardInterrupt: When user presses Ctrl+C
        socket.error: On network errors
        asterix.ParseError: If ASTERIX data is malformed

    Example:
        >>> main()
        Listening for ASTERIX data on multicast 232.1.1.11:21111
        Press Ctrl+C to stop...
        1. Receiver received = [{'cat': 48, 'len': 48, 'records': [...]}]
        2. Receiver received = [{'cat': 48, 'len': 48, 'records': [...]}]
        3. Receiver received = [{'cat': 34, 'len': 26, 'records': [...]}]
        ^C
        Receiver stopped by user

    Note:
        To test this script without a real ASTERIX multicast source, you can:
        1. Run multicast_send_receive.py in another terminal
        2. Use a network traffic generator
        3. Configure the multicast group to match your network's ASTERIX source

        To change the multicast configuration, modify MULTICAST_GROUP and PORT
        constants in the main() function.
    """
    # Multicast configuration
    MULTICAST_GROUP = "232.1.1.11"
    PORT = 21111
    BUFFER_SIZE = 10240  # 10KB buffer for receiving packets

    # Set up multicast receiver
    sock = setup_multicast_receiver(MULTICAST_GROUP, PORT)

    print(f"Listening for ASTERIX data on multicast {MULTICAST_GROUP}:{PORT}")
    print("Press Ctrl+C to stop...")

    counter = 1

    try:
        while True:
            # Receive UDP packet (blocks until data arrives)
            # Returns up to BUFFER_SIZE bytes
            asterix_packet = sock.recv(BUFFER_SIZE)

            # Parse ASTERIX data from UDP payload
            # Returns list of data blocks
            parsed = asterix.parse(asterix_packet)

            # Display packet counter and parsed data
            print('%d. Receiver received = %s' % (counter, parsed))
            counter += 1

    except KeyboardInterrupt:
        print("\nReceiver stopped by user")
    finally:
        sock.close()


if __name__ == '__main__':
    main()
