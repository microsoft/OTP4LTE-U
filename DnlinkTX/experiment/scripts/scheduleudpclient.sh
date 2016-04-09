#!/bin/sh


LAPTOPIP=10.85.254.59

FNAME=$1
DURATION=50

TIMES=30

START=$(date +%s)
echo "UDP client start at:"$START

iperf -u -c $LAPTOPIP -i 1 -t $TIMES -b 100M -p 5001 -y C >> $FNAME & 


NOW=$(date +%s)
DIFF=$((NOW-START))

sleep $((DURATION-DIFF))
NOW=$(date +%s)
echo "UDP client end at:"$NOW


