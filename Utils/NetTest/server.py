import socket
import sys
import argparse
import string
import time

# Send a selected number of UDP packet of selected size
# Each payload consists of the same byte repeated msgSize times
# and the byte increases from 0-128 throughout packets
# which can be used to detect missed packets

parser = argparse.ArgumentParser()
parser.add_argument('--localIP', help='Local IP address of interface to bind to', required=True)
parser.add_argument('--port', help='Port number', default="8888")
parser.add_argument('--verbose', help='Verbose output printing each received packet', action='store_true')
args = parser.parse_args()
localIP = string.lower(args.localIP);
port = int(args.port);
verbose = args.verbose



s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((localIP, port))

startTime = time.time()
pl = -1
no_total_pkts = 0;
no_rcvd_pkts = 0;
no_error_pkts = 0;

while (1):
    #data, addr = s.recvfrom(1024)
    rcv = s.recvfrom(1024)
    data = rcv[0]
    addr = rcv[1][0]
    srcport = rcv[1][1]

    # Check received packet
    error = 0
    d = data[0]
    for c in data:
      if (c != d):
          error = 1
    if error:
        no_error_pkts +=1
    else:
        no_rcvd_pkts +=1 

    # Check missing packets
    d = ord(data[0])
    if pl >= 0:
        no_total_pkts += (d - pl) % 128
        pl = d
    pl = d

    if verbose:
        print "Pkt from (", addr, ",", srcport, "), len:", len(data), ", value:", ord(data[0])

    # Print stats
    t = time.time()
    if t - startTime > 1:
        print "Stats:", no_rcvd_pkts, no_error_pkts, no_total_pkts - no_rcvd_pkts - no_error_pkts, "packets in", (t - startTime), "s"
        startTime = t
        no_total_pkts = 0
        no_rcvd_pkts = 0
        no_error_pkts = 0
