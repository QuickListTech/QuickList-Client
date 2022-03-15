#!/usr/bin/python

import socket
import sys
import json

from threading import Thread

# Create a UDS socket
sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = '/tmp/quicklist.sock'
print('connecting to {}'.format(server_address))

def readSocket():
    while 1:
        data = sock.recv(1024)
        print('received {!r}'.format(data))

def readInput():
    print("Type message:")

    for line in sys.stdin:
        out = line.strip().encode()
        print('sending {!r}'.format(out))
        sock.sendall(out)
try:
    sock.connect(server_address)
except socket.error as msg:
    print(msg)
    sys.exit(1)

try:

    t1 = Thread(target=readSocket)
    t2 = Thread(target=readInput)

    t1.start()
    t2.start();

    t1.join();
    t2.join();

finally:
    print('closing socket')
    sock.close()
