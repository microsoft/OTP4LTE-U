#!/bin/bash


echo "sudo rfkill unblock wifi"
sudo rfkill unblock wifi 

echo "sudo rfkill unblock all"
sudo rfkill unblock all
#sudo rfkill list


#delete the monitor interface
echo "sudo ifconfig mon0 down"
sudo ifconfig mon0 down
echo "sudo iw dev mon0 del"
sudo iw dev mon0 del


#echo "sudo iw phy phy0 interface add wlp3s0 type managed"
#sudo iw phy phy0 interface add wlp3s0 type managed
#sleep 3

echo "ifconfig wlp3s0 up"
sudo ifconfig wlp3s0 up

sleep 1
echo "sudo service network-manager start"
sudo service network-manager start

sleep 1
echo "sudo service bluetooth stop"
sudo service bluetooth stop

