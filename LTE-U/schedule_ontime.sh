#!/bin/bash

GAIN=(25)

LOOPS=10
PERIOD=$((LOOPS*60))

START=$(date +%s)
NOW=$(date +%s)
DIFF=$((NOW-START))

cc=0
for ontime in {20..4} 
do
  for (( dutycycle= 20; dutycycle< 100; dutycycle+= 30))
  do
    offtime=$(((ontime*(100-dutycycle) + dutycyle/2)/dutycycle))
    for gain in "${GAIN[@]}"
    do
      BEGIN=$(date +%s)
      echo "Start experiment at ontime $ontime offtime:$offtime gain: $gain at time:$BEGIN"
      
      ./run_eNodeB.sh $ontime $offtime $gain 0 &
	
      END=$(date +%s)
      DIFF=$((END-BEGIN))
      sleep $((PERIOD-DIFF))
      killall mac.out &
      killall mac.out &
      killall mac.out &
      (( cc++ ))
    done
  done
done

echo "there are totally $cc rounds"

