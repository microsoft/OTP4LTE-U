import fileinput
import string
import sys
import argparse
import re
import operator


# Converts eNodeB's output to Wireshark readable hexdump of IP packets with fake Ethernet headers

parser = argparse.ArgumentParser()
parser.add_argument('--file', help='Input file', required=True)
args = parser.parse_args()
file_name = string.lower(args.file);


lc = 0;
for line in fileinput.input(file_name):
  if len(line) > 16:
    print "0000    00 00 00 00 00 00 00 00 00 00 00 00 08 00",
    lc = lc + 1
    for i in range(0, (len(line)-16)/2):
      print line[16+2*i:16+2*i+2], 
    print ""



