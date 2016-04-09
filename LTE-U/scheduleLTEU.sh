#!/bin/bash


PERIOD=60 
CYCLE=100

for i in {8..9}
do
	BEGIN=$(date +%s)
	echo "round: "$i" LTEU begins at:"$BEGIN
	
	on=$((i*10*CYCLE/100))
	off=$((CYCLE-on))
	echo $on" "$off
	./run_eNodeB.sh $on $off &
	
	END=$(date +%s)
	DIFF=$((END-BEGIN))
	sleep $((PERIOD-DIFF))

	killall mac.out &

	NOW=$(date +%s)
	echo "LTEU ends at:"$NOW
done



