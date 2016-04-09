#!/bin/sh

LAPTOPIP=10.85.254.59
LOCALIP=10.190.103.164

PERIOD=500 
#in second
TXGAINS=(18 19 20 21 22 23 24 25)


for i in "${TXGAINS[@]}" 
do
	BEGIN=$(date +%s)
	echo "LTE begins at:"$BEGIN
	
	make dnlink_empty.run TXGAIN=$i &

	END=$(date +%s)
	DIFF=$((END-BEGIN))
	sleep $((PERIOD-DIFF))

	killall dnlink_empty.out &

	NOW=$(date +%s)
	echo "LTE ends at:"$NOW
done



