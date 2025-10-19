#!/usr/bin/python

__author__ = 'dsalanti'

import socket
import struct
import asterix

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Security Note: Binding to all interfaces ('') is required for multicast reception
# to work correctly. In production deployments, consider:
# 1. Using firewall rules to restrict access to port 21111
# 2. Running in isolated network segments for ASTERIX traffic
# 3. Implementing application-level authentication if needed
# 4. Binding to specific interface IP if multicast is on a known interface
# lgtm [py/bind-socket-all-network-interfaces]
sock.bind(('', 21111))  # nosec B104 - Required for multicast reception
mreq = struct.pack("=4sl", socket.inet_aton("232.1.1.11"), socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

counter = 1

while True:
    asterix_packet = sock.recv(10240)
    parsed = asterix.parse(asterix_packet)
    print('%d. Receiver received = %s' % (counter, parsed))
    counter += 1
