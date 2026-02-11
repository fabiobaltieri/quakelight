#!/bin/env python3

import socket
import struct

def start_ipv4_multicast_receiver():
    multicast_group = '239.255.6.6'
    server_address = ('', 9667)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(server_address)
    group = socket.inet_aton(multicast_group)
    mreq = struct.pack('4sL', group, socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    print(f"Listening on {multicast_group}:{server_address[1]}")

    try:
        while True:
            data, address = sock.recvfrom(1024)
            print(f"\nReceived {len(data)} bytes from {address}")
            print(f"Data: {data.decode('utf-8', errors='ignore')}")
    except KeyboardInterrupt:
        print("Closing");
    finally:
        sock.close()

def start_ipv6_multicast_receiver():
    multicast_group = 'ff12::fab:666'
    port = 9667

    sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('::', port))
    group_bin = socket.inet_pton(socket.AF_INET6, multicast_group)
    mreq = group_bin + struct.pack('@I', 0)
    sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_JOIN_GROUP, mreq)

    print(f"Listening on [{multicast_group}]:{port}")

    try:
        while True:
            data, address = sock.recvfrom(1024)
            print(f"\nReceived {len(data)} bytes from {address}")
            print(f"Data: {data.decode('utf-8', errors='ignore')}")
    except KeyboardInterrupt:
        print("Closing");
    finally:
        sock.close()

if __name__ == "__main__":
    start_ipv4_multicast_receiver()
    start_ipv6_multicast_receiver()
