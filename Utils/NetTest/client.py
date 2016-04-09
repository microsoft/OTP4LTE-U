import socket
import sys
import argparse
import string
import time

# Send a selected number of UDP packet of selected size
# Each payload consists of the same byte repeated msgSize times
# and the byte increases from 0-128 throughout packets
# which can be used to detect missed packets

# --msgSize==200 produces TBS==1872
# IP_header == 20B, UDP_header == 8B, payload == 200B -> total 1824 bits (+ MAC header)

parser = argparse.ArgumentParser()
parser.add_argument('--dstIP', help='Destination IP address', required=True)
parser.add_argument('--sleep', help='Sleep time in ms (min Windows sleep time is ~10ms)', default="20")
parser.add_argument('--batch', help='Number of packets to send before sleeping', default="1")
parser.add_argument('--port', help='Port number', default="8888")
parser.add_argument('--msgSize', help='UDP payload size (in bytes)', default="120")
args = parser.parse_args()
dstIP = string.lower(args.dstIP);
sleepMS = float(args.sleep)/1000.0;
batch = int(args.batch);
port = int(args.port);
msgSize = int(args.msgSize);

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

msgPool = ""

for i in range(0, 128):
    for j in range(0, msgSize):
        msgPool += str(unichr(i))

cnt = 0
while(1):
    for i in range(0, batch):
        s.sendto(msgPool[cnt*msgSize:(cnt+1)*msgSize], (dstIP, port))
        #print cnt*msgSize, (cnt+1)*msgSize, msgPool[cnt*msgSize:(cnt+1)*msgSize]
        cnt = (cnt+1)%128
    time.sleep(sleepMS)

