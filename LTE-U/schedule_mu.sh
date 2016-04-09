#!/bin/bash


TYPE=(0 1)
ONTIME=(0 5 10 15)
interval=20



PERIOD=$((15*60))

START=$(date +%s)
NOW=$(date +%s)
DIFF=$((NOW-START))

cc=0


for tcp in "${TYPE[@]}"
do
  for ontime in "${ONTIME[@]}"
  do
    BEGIN=$(date +%s)
    offtime=$(($interval-$ontime))
    echo "Start experiment at ontime: $ontime , offtime:, $offtime , at time:"$BEGIN
      
    ./run_eNodeB.sh $ontime $offtime 25 0 &	      
      
    END=$(date +%s)
    DIFF=$((END-BEGIN))
    sleep $((PERIOD-DIFF))
    killall mac.out &
    killall mac.out &
    (( cc++ ))
  done
done

echo "there are totally $cc rounds"

