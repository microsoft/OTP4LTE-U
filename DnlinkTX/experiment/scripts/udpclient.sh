#!/bin/bash

DATAFOLDER=./dataudp/
PERIOD=60
#in second
TXGAINS=(5 25)
MEASURE="./scripts/scheduleudpclient.sh"
SUMMARY="./summary/"

mkdir -p $DATAFOLDER


#for i in "${TXGAINS[@]}" 
for i in {-20..25}
do
	BEGIN=$(date +%s)
	echo "LTE begins at:"$BEGIN
	
	make dnlink_empty.run TXGAIN=$i &
    
	echo "Start measurement at TXGAIN:"$i
    	START=$(date +%s)    
    	#bring up one measurement, which kills itself
	OUTFILE=$DATAFOLDER"udpclient_"$i  

	echo $OUTFILE

	exec $MEASURE $OUTFILE & 
    
	END=$(date +%s)
	DIFF=$((END-BEGIN))
	sleep $((PERIOD-DIFF))

	killall dnlink_empty.out &


	rate=$(tail -1 $OUTFILE | awk -F, '{print $9}')
	echo $rate
	echo $i,$rate >> $SUMMARY"result_40_80"
	NOW=$(date +%s)
	echo "LTE ends at:"$NOW
done


