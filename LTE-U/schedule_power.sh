#!/bin/bash


ONTIME=(20 10 5 0)

GAIN=(-35 -30 -27 -25 -23 -20 -17 -13 -10 -7 0 2 4 7 10 13 16 19 22 25)



LOOPS=10
PERIOD=$((LOOPS*60))

START=$(date +%s)
NOW=$(date +%s)
DIFF=$((NOW-START))

cc=0

for ontime in "${ONTIME[@]}"
do
  if [ $ontime -eq 0 ]; then
    ontime=100
    offtime=0
  else

    offtime=$((ontime/4))
  fi
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
    (( ++cc  ))
  done
  
done

echo "there are totally $cc rounds"


