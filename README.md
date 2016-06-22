# Open Test Platform for LTE/LTE-U

This is a GitHub repository for the Open Test Platform for LTE/LTE-U (short OTP4LTE-U) published by Microsoft. OTP4LTE-U is released as open source, under [MIT license](https://github.com/Microsoft/OTP4LTE-U/blob/master/LICENSE.txt). 

OTP4LTE-U implements a basic subset of LTE PHY features, as well as the rudimentary MAC, RLC, PDCP and RRC, sufficient to connect a standard LTE dongle. OTP4LTE-U also implements the LTE-U specification and can be used to test LTE-U behaviour and impact on other technologies in 5 GHz spectrum.  


OTP4LTE-U is built in [Ziria](http://research.microsoft.com/en-us/projects/ziria/), a domain-specific programming language (DSL) built for programming software-defined radios (SDR). Ziria is relased as open source, and is available on [GitHub](https://github.com/dimitriv/Ziria).

OTP4LTE-U is built for [Sora SDR platform](https://github.com/Microsoft/Sora). It can also run on [BladeRF SDR](https://www.nuand.com/), although BladeRF currently doesn't support 5 GHz spectrum access. 




--------
## Demo

Check the [video demo](http://research.microsoft.com/apps/video/?id=264975).




--------
## Build instructions

For build instructions please read [BUILD](BUILD.md)



--------
## Directory structure

Current directory structure is as follows:

- DnlinkTX: Ziria files implementing all parts of LTE Downlink transmitter
  - DnlinkTx/tests: unit tests for DnlinkTX

- UplinkRX: Ziria files implementing all parts of LTE Uplink transmitter
  - UplinkRx/PRACH: PRACH-related code
  - UplinkRx/tests: unit tests for DnlinkTX

- MAC: C files that wrap Ziria files to compile into a full eNodeB

- eNodeB: Top-level Ziria files implementing eNodeB and scripts to make it. 
  - Run make here to start eNodeB

- MatlabTests: Various matlab tests that verify Ziria code
  - MatlabTests/DnlinkTx: Matlab tests for DnlinkTX
    - MatlabTests/DnlinkTx/tests: unit tests for DnlinkTX

- Perf: Collection of documented performance evaluation results

- LTE-U: LTE-U compliant MAC implementation. See [LTE-U README](https://github.com/Microsoft/OTP4LTE-U/blob/master/LTE-U/README.md) for more information.



--------
## Current limitations:

### Uplink:
  - Uplink packet size is currently limited to 8 RBs. 
  - The receiver decodes uplink packets in every sub-frame, 
    regardless of whether something can be send 
    (depending on whether DCI0 grant was sent)
  - DCI0 is sent in subframe 0 of each frame. 

### Dnlink:
  - Currently, Dnlink packets are only sent in subframe 4 of each frame 
    (hardcoded in mac_tx_callback() in mac_tx.c)
  - Since we don't have a real-time feedback, we sent RAR in every frame
    It is hardcoded in subframe RAR_SUBFRAME (==6)



--------
## Code of conduct:

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.