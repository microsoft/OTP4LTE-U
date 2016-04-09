#!/bin/bash


OFFTIME=(90 20 10 1)
GAIN=(-35 -33 -30 -27 -24 -21 -18 -15 -10 -5 0 1 5 10 15 19 20 22 23 25)
#POWRE=(-109 -107 -104 -101 -98 -95 -92 -89 -86 -83 -82 -81 -78 -73 -68 -64 -63 -61 -60 -58)

LOOPS=5
PERIOD=$((LOOPS*60))

START=$(date +%s)
NOW=$(date +%s)
DIFF=$((NOW-START))

ontime=10
for offtime in "${OFFTIME[@]}"
do
  for gain in "${GAIN[@]}"
  do
    BEGIN=$(date +%s)
    echo "Start experiment at ontime 10ms offtime:"$offtime", at time:"$BEGIN
    echo "Start measurement at LTE power:"$gain

    ./run_eNodeB.sh $ontime $offtime $gain &
	
    END=$(date +%s)
    DIFF=$((END-BEGIN))
    sleep $((PERIOD-DIFF))
    killall mac.out &
    killall mac.out &
    killall mac.out &
  done
done

