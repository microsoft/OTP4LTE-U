#!/bin/bash

FREQ=2462

echo "sudo service network-manager stop" 
sudo service network-manager stop

echo "sudo rfkill unblock wifi"
sudo rfkill unblock wifi 

echo "sudo rfkill unblock all"
sudo rfkill unblock all
#sudo rfkill list

echo "sudo ifconfig wlp3s0 down"
sudo ifconfig wlp3s0 down

#echo "sudo iw dev wlp3s0 del (not really necessary)"
#sudo iw dev wlp3s0 del

echo "sudo ifconfig mon0 down"
sudo ifconfig mon0 down

echo "sudo iw dev mon0 del"
sudo iw dev mon0 del

echo "sudo iw phy phy0 interface add mon0 type monitor"
sudo iw phy phy0 interface add mon0 type monitor

sleep 1
echo "sudo iwconfig mon0 mode monitor"
sudo iwconfig mon0 mode monitor

sleep 1
echo "sudo ifconfig mon0 up"
sudo ifconfig mon0 up
 
sleep 1
echo "sudo iw dev mon0 set freq $FREQ"
sudo iw dev mon0 set freq $FREQ 

sleep 1
echo "sudo service bluetooth stop"
sudo service bluetooth stop




