#!/usr/bin/env python3
"""
Demonstrate multicast ASTERIX transmission and reception using threads.

This example shows a complete multicast communication system with both sender
and receiver components running concurrently. This is useful for:
- Testing multicast network configurations
- Simulating ASTERIX data streams for development
- Understanding the full multicast communication cycle
- Demonstrating thread-based network programming patterns

The example uses Python threading to run sender and receiver simultaneously:
- Sender thread: Transmits a CAT048 ASTERIX packet every 100ms
- Receiver thread: Receives and parses packets, displays formatted output

Threading Pattern:
    Main Thread
    ├── SenderThread: Periodically sends ASTERIX packets via multicast
    └── ReceiverThread: Listens for and processes incoming packets

Usage:
    python multicast_send_receive.py

    The script will start both sender and receiver threads.
    Both threads will run for 100,000 iterations (configurable).
    Press Ctrl+C to stop early.

Requirements:
    - asterix_decoder package installed (pip install asterix_decoder)
    - Loopback multicast support (usually enabled by default)
    - Network interface that supports multicast

Example Output:
    Starting Sender
    Starting Receiver
    1. Sender sending : b'0\x00...'
    Data Block 1/1
      Category: 048 (Monoradar Target Reports)
      Length: 48 bytes
      Records: 1
        Record 1:
          I010 (Data Source Identifier)
            SAC: 2
            SIC: 25
          ...
    2. Sender sending : b'0\x00...'
    ...
    Exiting Sender
    Exiting Receiver

Network Configuration:
    Multicast Group: 224.51.105.104
    Port: 5000
    Protocol: UDP
    TTL: 2 (limited to local network)

Key Concepts:
    - Multicast TTL (Time To Live): Limits packet propagation
    - Thread synchronization with exitFlag
    - Socket reuse for multiple receivers
    - Multicast group membership management

See Also:
    - multicast_receive.py: Standalone receiver example
    - read_raw_bytes.py: Understanding ASTERIX packet structure
"""

__author__ = 'dsalanti'

import threading
import time
import socket
import struct
import asterix

# Global flag for coordinating thread shutdown
exitFlag = False

# Sample CAT048 ASTERIX packet (Monoradar Target Reports)
# This packet contains radar surveillance data
asterix_packet = bytes([
    0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9, 0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
    0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10, 0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
    0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb, 0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5
])


class SenderThread(threading.Thread):
    """
    Thread that sends ASTERIX packets via UDP multicast.

    This thread repeatedly sends a predefined ASTERIX packet to a multicast
    group at regular intervals. It demonstrates:
    - Multicast packet transmission
    - UDP socket configuration for sending
    - Thread-based periodic transmission
    - TTL (Time To Live) configuration for multicast

    Attributes:
        threadID (int): Unique identifier for the thread
        name (str): Human-readable thread name
        counter (int): Number of packets remaining to send
        sock (socket.socket): UDP socket configured for multicast transmission
    """

    def __init__(self, threadID, name, counter):
        """
        Initialize the sender thread.

        Args:
            threadID (int): Unique thread identifier
            name (str): Thread name for logging
            counter (int): Number of packets to send before stopping

        Example:
            >>> sender = SenderThread(1, "ASTERIX Sender", 1000)
            >>> sender.start()  # Begin sending packets
        """
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.counter = counter

        # Create UDP socket for sending
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

        # Set multicast TTL (Time To Live)
        # TTL=2 means packets can traverse up to 2 routers
        # This limits multicast propagation to local network
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)

    def run(self):
        """
        Send ASTERIX packets periodically until counter reaches zero.

        This method runs in a separate thread and:
        1. Sends the ASTERIX packet to the multicast group
        2. Waits 100ms between transmissions
        3. Decrements the counter
        4. Stops when counter reaches zero or exitFlag is set

        The method handles the exitFlag for graceful shutdown.

        Returns:
            None

        Example:
            After starting the thread:
            >>> sender = SenderThread(1, "Sender", 5)
            >>> sender.start()
            Starting Sender
            5. Sender sending : b'0\x00...'
            4. Sender sending : b'0\x00...'
            3. Sender sending : b'0\x00...'
            2. Sender sending : b'0\x00...'
            1. Sender sending : b'0\x00...'
            Exiting Sender
        """
        print("Starting " + self.name)

        while self.counter:
            if exitFlag:
                # Exit requested by main thread or another component
                break

            # Wait between transmissions (100ms interval)
            time.sleep(0.1)

            # Send ASTERIX packet to multicast group
            print("%d. Sender sending : %s" % (self.counter, str(asterix_packet)))
            self.sock.sendto(asterix_packet, ("224.51.105.104", 5000))

            self.counter -= 1

        print("Exiting " + self.name)


class ReceiverThread(threading.Thread):
    """
    Thread that receives and parses ASTERIX packets from UDP multicast.

    This thread listens on a multicast group, receives ASTERIX packets,
    parses them, and displays human-readable output. It demonstrates:
    - Multicast group membership
    - UDP socket configuration for receiving
    - ASTERIX parsing and formatting
    - Thread-based continuous reception

    Attributes:
        threadID (int): Unique identifier for the thread
        name (str): Human-readable thread name
        counter (int): Number of packets remaining to receive
        sock (socket.socket): UDP socket configured for multicast reception
    """

    def __init__(self, threadID, name, counter):
        """
        Initialize the receiver thread.

        Args:
            threadID (int): Unique thread identifier
            name (str): Thread name for logging
            counter (int): Number of packets to receive before stopping

        Example:
            >>> receiver = ReceiverThread(1, "ASTERIX Receiver", 1000)
            >>> receiver.start()  # Begin receiving packets
        """
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.counter = counter

        # Create UDP socket for receiving
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

        # Enable address reuse to allow multiple receivers on same port
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    def run(self):
        """
        Receive and parse ASTERIX packets until counter reaches zero.

        This method runs in a separate thread and:
        1. Binds to the multicast port on all interfaces
        2. Joins the multicast group
        3. Receives packets in a loop
        4. Parses each packet as ASTERIX data
        5. Displays formatted output
        6. Stops when counter reaches zero or exitFlag is set

        The method handles the exitFlag for graceful shutdown and formats
        output using asterix.describe() for human readability.

        Returns:
            None

        Example:
            After starting the thread:
            >>> receiver = ReceiverThread(1, "Receiver", 2)
            >>> receiver.start()
            Starting Receiver
            Data Block 1/1
              Category: 048 (Monoradar Target Reports)
              Length: 48 bytes
              ...
            Data Block 1/1
              Category: 048 (Monoradar Target Reports)
              Length: 48 bytes
              ...
            Exiting Receiver
        """
        print("Starting " + self.name)

        # Bind to all interfaces on port 5000
        # Required for multicast reception
        self.sock.bind(('', 5000))

        # Join the multicast group 224.51.105.104
        # mreq = 4 bytes (multicast address) + 4 bytes (interface)
        # INADDR_ANY means join on all available interfaces
        mreq = struct.pack("=4sl", socket.inet_aton("224.51.105.104"), socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

        while self.counter:
            if exitFlag:
                # Exit requested by main thread or another component
                break

            # Receive packet (blocks until data arrives)
            # Buffer size: 10240 bytes (10KB)
            asterix_packet = self.sock.recv(10240)

            # Parse ASTERIX data from UDP payload
            parsed = asterix.parse(asterix_packet)

            # Display formatted output (human-readable)
            # describe() converts parsed data to readable text
            print(asterix.describe(parsed))

            self.counter -= 1

        print("Exiting " + self.name)


def main():
    """
    Create and start sender and receiver threads for multicast demo.

    This function demonstrates a complete multicast communication system:
    1. Creates sender thread to transmit ASTERIX packets
    2. Creates receiver thread to receive and parse packets
    3. Starts both threads concurrently
    4. Main thread exits after starting worker threads

    The threads will run until their counters reach zero or the process
    is interrupted.

    Returns:
        None

    Thread Configuration:
        - Both threads configured for 100,000 iterations
        - Sender transmits every 100ms
        - Receiver processes packets as they arrive
        - Total runtime: ~2.7 hours if not interrupted

    Example:
        >>> main()
        Starting Sender
        Starting Receiver
        Exiting Main Thread
        1. Sender sending : b'0\x00...'
        Data Block 1/1
          Category: 048 (Monoradar Target Reports)
          ...
        2. Sender sending : b'0\x00...'
        ...

    Note:
        The sender and receiver use the same multicast group (224.51.105.104:5000)
        so packets sent by the sender are received by the receiver. This creates
        a complete loopback test environment.

        To customize the behavior:
        - Modify 'repeat' variable to change number of packets
        - Adjust sleep duration in SenderThread.run() to change transmission rate
        - Change multicast group/port in thread classes for different configurations
    """
    # Number of packets to send/receive
    # Set to large number for long-running demo
    repeat = 100000

    # Create sender and receiver threads
    # ThreadID 1 for both (not used in this example)
    sender_thread = SenderThread(1, "Sender", repeat)
    receiver_thread = ReceiverThread(1, "Receiver", repeat)

    # Start both threads concurrently
    sender_thread.start()
    receiver_thread.start()

    print("Exiting Main Thread")
    # Main thread exits, but worker threads continue running


if __name__ == '__main__':
    main()
