#!/bin/env python3

import socket
import time

DEST_IP = "239.255.6.6"
DEST_PORT = 9666

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

try:
    print(f"Sending sequence to {DEST_IP}:{DEST_PORT}...")

    for i in range(1, 99999):
        message = str(i).encode('utf-8')

        sock.sendto(message, (DEST_IP, DEST_PORT))

        if i % 100 == 0:
            print(f"Sent: {i}")

        time.sleep(0.001)

    print("Sequence complete.")

finally:
    sock.close()
