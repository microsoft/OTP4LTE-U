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

`cd <git_root>/LTE-U`

`make -B mac.out`

This will take some time as the compiler is producing a highly optimized code. 



***************************************************************************************
## LTE-U with static parameters


To run LTE-U, with predefined parameters (no CSAT) type:

`cd <git_root>/LTE-U`

`./run_eNodeB.sh <ONCYCLE> <OFFCYCLE> <TXGAIN> <ctsframes>`

where:
- ONCYCLE is duration of ON cycle in ms
- OFFCYCLE is duration of OFF cycle in ms
- TXGAIN is radio gain at TX (See BLADE RF section below)
- ctsframes denotes how many 1ms subframes to send filled with CTSes at the beginning of ON cycle 
  (e.g. 1 will send ~17 CTSes during 1ms before the ON period)

Currently, on BladeRF, the WiFi channel is set to 2462MHz and has to be changed by
modifying run_eNodeB accordingly.

For a successful demo one needs to make sure that the incumbent WiFi
run in the same WiFi channel as LTE-U.






***************************************************************************************
## LTE-U with CSAT


To run CSAT, go to LTE/LTE-U/controller/. 

Controller currently works only on Linux. To build CSAT, one needs:
- libpcap-dev, version 1.5.3
- libjsoncpp-dev, version 0.6.0

(If you use newer version of libpcap you may need to rename the include file; see http://www.linuxquestions.org/questions/fedora-35/can%27t-compile-hping3-net-bpf-h-no-such-file-or-directory-315366/)

To get the WiFi sniffer working, one needs to install Ubuntu 14.04 or
15.10.  It may work with other versions of Linux that support the
appropriate Intel driver, but it hasn't been tested.

To make the code, type

  `cd LTE-U/controller`

  `make`


To set up monitoring interface (required to run CSAT) run
setup_monitor.sh in scripts subfolder:

  `cd scripts`

  `./setup_monitor.sh`


The output will look like this:

```
sudo service network-manager stop
sudo rfkill unblock wifi
sudo rfkill unblock all
sudo ifconfig wlp3s0 down
sudo ifconfig mon0 down
mon0: ERROR while getting interface flags: No such device
sudo iw dev mon0 del
command failed: No such device (-19)
sudo iw phy phy0 interface add mon0 type monitor
sudo iwconfig mon0 mode monitor
sudo ifconfig mon0 up
sudo iw dev mon0 set freq 2462
sudo service bluetooth stop
```

The error should be ignored if ran for the first time, since the mon0 hasn't been created yet. 




To connect WiFi monitor one needs to connect the monitor (Linux) with
the LTE-U transmitter (Windows) over a UDP socket. The IP parameters
for this connection are currently hardcoded in 
- LTE/LTE-U/controller/main.cc (on Linux) and
- LTE\MAC\mac_udpsocket.c (on Windows)

The parameters are:
- `const string kLocalIP = "192.168.1.10";`
- `const int kLocalPort = 55555;`
- `const string kRemoteIP = "192.168.1.11";`
- `const int kRemotePort = 4444;`

So the IP addresses of nodes should be configured accordingly. 

CSAT parameters are currently hardcoded in code/WiFiLTEU/main.cc (on
Linux) in class CSAT::CSAT(). In future this will be rewritten to be
passed from the command line.

To run CSAT+LTE-U:
- On Windows, run 

  `cd <git_root>/LTE-U`

  `./run_eNodeB.sh` 

  with any parameters (currently the parameters cannot be omitted).

- On Linux, run 

  `cd <git_root>/LTE-U/controller`

  `sudo ./wifiteu <CSAT_threshold_1> <CSAT_threshold_2> <start_ON_time> <unused>`
  



***************************************************************************************
## LTE-U with SORA and BladeRF SDRs

LTE-U currently supports both Sora and BladeRF radio platforms. However, BladeRF 
currently doesn't work at 5 GHz so it cannot be used for standard-compliant testing. 

The default Makefile will build the environment for Sora. To change to BladeRF, please copy 
file Makefile.BLADERF to Makefile and build it. 




***************************************************************************************
## Getting help

For further questions and help, please email [Bozidar](mailto:bozidar@microsoft.com) or [Lei](mailto:kanglei1130@gmail.com).
