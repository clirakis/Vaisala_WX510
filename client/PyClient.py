#!/usr/bin/python2.7
import socket
import sys
"""
04-Jul-13
example copied from http://docs.python.org/2/library/socketserver.html
"""

HOST, PORT = "192.168.1.8", 9999
data = " ".join(sys.argv[1:])
print "Host: ", HOST
# Create a socket (SOCK_STREAM means a TCP socket)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Connect to server and send data
    sock.connect((HOST, PORT))
    sock.sendall(data + "\n")

    # Receive data from the server and shut down
    #received = sock.recv(1024)
finally:
    sock.close()

print "Sent:     {}".format(data)
#print "Received: {}".format(received)

