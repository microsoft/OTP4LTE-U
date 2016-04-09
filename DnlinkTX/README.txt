*******************************
Quick overview of DnlinkTX PHY:
*******************************

- dlSCH : struct MACDLPacket is the packet arriving from MAC layer that has to be transmitted

- configureDCI sets RB schedule (start, length), HARQNo, NDI, RV and block length
  + Currently block length is fully implemented for SIBs and RAR (RA/P/SI-RNTI) 
    and only several block lengths are supported for QPSK for the rest

- PDCCH position is currently hard-coded in addDLSCHToSubframe to CCE4 for downlink

- struct PDSCHParams (sched = configurePDSCH(...) ) contains RNTI, RB schedule, modulation
  + Modulation is currently hard-coded to QPSK in configurePDSCH

- countPDSCHToSubframe counts the number of available REs for a given sched

- dlsch calculates CRC and performs turbo coding and rate matching

- pdsch is scrambling with gold sequence and modulating

- addPDSCHToSubframe is copying modulated dsch data to cellGrid
