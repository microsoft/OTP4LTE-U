#!/bin/bash


ONTIME=(20 10 5)


GAIN=(10 25)

LOOPS=10
PERIOD=$((LOOPS*60))

START=$(date +%s)
NOW=$(date +%s)
DIFF=$((NOW-START))


for ontime in "${ONTIME[@]}"
do
  for gain in "${GAIN[@]}"
  do
    for (( dutycycle= 10; dutycycle< 100; dutycycle+= 10))
    do
      BEGIN=$(date +%s)
      offtime=$(((ontime*(100-dutycycle) + dutycyle/2)/dutycycle))
      echo "Start experiment at ontime: $ontime , offtime:, $offtime , dutycycle: $dutycyle% gain: $gain,  at time:"$BEGIN
      
      if [ $offtime -ne 0 ]; then
        ./run_eNodeB.sh $ontime $offtime $gain 1 &	      
      fi
      
      END=$(date +%s)
      DIFF=$((END-BEGIN))
      sleep $((PERIOD-DIFF))
      killall mac.out &
      killall mac.out &

    done  
  done
done

