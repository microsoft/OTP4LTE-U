# OTP4LTE-U: LTE-U implementation

***************************************************************************************
## OVERVIEW


The system consists of one PC that runs Windows and has an SDR
connected to it. It is currently tested with Windows and BladeRF and
Sora boards, although it can probably easily be configured to work
with Linux + BladeRF. 

In order to run CSAT, one also requires a second PC/laptop with Ubuntu
to act as CSAT controller. Linux is required as Linux WiFi drivers
offer better support for WiFi sniffing in promiscuous mode. Controller
code is in subdirectory controller.




***************************************************************************************
## BUILD


To make PHY/MAC, run:
make -B mac.out



***************************************************************************************
## LTE-U with static parameters


To run LTE-U, with predefined parameters (no CSAT) type:
./run_eNodeB.sh <ONCYCLE> <OFFCYCLE> <TXGAIN> <ctsframes>
where:
- ONCYCLE is duration of ON cycle in ms
- OFFCYCLE is duration of OFF cycle in ms
- TXGAIN is radio gain at TX (See BLADE RF section below)
- ctsframes denotes how many 1ms subframes to send filled with CTSes at the beginning of ON cycle 
  (e.g. 1 will send ~17 CTSes during 1ms before the ON period)

Currently the WiFi channel is set to 2462MHz and has to be changed by
modifying run_eNodeB accordingly.

For a successful demo one needs to make sure that the incumbent WiFi
run in the same WiFi channel as LTE-U.






***************************************************************************************
## LTE-U with CSAT


To run CSAT, go to LTE/LTE-U/controller/. 

Controller currently works only on Linux. To build CSAT, one needs:
- libpcap, version ...
- libjsoncpp, version ...

To get the WiFi sniffer working, one needs to install Ubuntu 14.04 or
15.10.  It may work with other versions of Linux that support the
appropriate Intel driver, but it hasn't been tested.

To make the code, type
  make

To set up monitoring interface (required to run CSAT) run
setup_monitor.sh in scripts subfolder. 

To connect WiFi monitor one needs to connect the monitor (Linux) with
the LTE-U transmitter (Windows) over a UDP socket. The IP parameters
for this connection are currently hardcoded in 
- LTE/LTE-U/controller/main.cc (on Linux) and
- LTE\MAC\mac_udpsocket.c (on Windows)
The parameters are:
  const string kLocalIP = "192.168.1.10";
  const int kLocalPort = 55555;
  const string kRemoteIP = "192.168.1.11";
  const int kRemotePort = 4444;
So the IP addresses of nodes should be configured accordingly. 

CSAT parameters are currently hardcoded in code/WiFiLTEU/main.cc (on
Linux) in class CSAT::CSAT(). In future this will be rewritten to be
passed from the command line.

To run CSAT+LTE-U:
- Run ./run_eNodeB.sh on Windows with any parameters (currently the
  parameters cannot be omitted).
- Run sudo ./wifiteu <p1> <p2> <p3> <p4>
  






***************************************************************************************
## BLADE RF


For bladeRF: 
TXGAIN should be from -35 to 25 for bladeRF, but it is not linear. 
It needs callibration: measure TXGAIN and compare with spectrum analyzer output.
For example, for Lei's experimental setup with two Minicircuits ERA5 amplifier, 
these are the observed relations:

TXGAIN, output power [dBm]
-35,-34
-30,-33
-27,-31.3
-25,-30.5
-23,-29
-20,-26.6
-17,-23.8
-13,-20
-10,-17.1
-7,-14.2
0,-11.1
2,-9.2
4,-7.2
7,-4.1
10,-1.2
13,1.8
16,4.7
19,7.6
22,10.2
25,12.4
