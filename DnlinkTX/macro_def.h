/*********************************************************************************

MIT License

Copyright (c) 2016 Microsoft

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*********************************************************************************/

#pragma once 

/*
-- To be verified: I assume that a single packet cannot be larger than 
-- 110 * 12 * 14 REs, = 18480 bits with QPSK
-- #define MAX_MSG_SIZE       18480

-- TODO DEBUG: Various tests crash with stack overflow/mem access errors with 18480 buffer. 
-- Also, we target 10 MHz == 50 RBs. 
*/

#define MAX_MSG_SIZE            (50*12*14)

/*
-- Highest modulation 64QAM
-- WARNING!!! If you change this you have to change the size of data in struct MACDLPacket in MAC/mac_struct.h
-- TODO: Put back to *6 once gold sequence generation is run at init time
*/
#define MAX_BIT_MSG_SIZE        (MAX_MSG_SIZE*2)                
/*
-- Highest output (decoded) pkt size in bits supported by our turbo decoder (LTE_lib)
*/
#define MAX_UL_BIT_MSG_SIZE        6144                

/*
-- TODO: Replace this with (MAX_MSG_SIZE/3) throughout
-- #define MAX_MSG_SIZE_DIV_3  6160
*/
#define MAX_GOLD_SEQUENCE_LEN           MAX_BIT_MSG_SIZE
#define MAX_PDCCH_GOLD_SEQUENCE_LEN     MAX_BIT_MSG_SIZE

/*
-- Number of frames in LTE Dnlink
*/
#define LTE_NO_FRAMES	1024



#define MAX_NDLRB           110
/*
-- We hardcode width for vectorizers efficency
*/
#ifdef BW_20
  #define NDLRB               100
  #define NULRB               100
#else
  #define NDLRB               50
  #define NULRB               50
#endif




#define PHICH_NORMAL      0


/*
-- NG: [1/6 1/2 1 2] = [0, 1, 2, 3];
*/
#define NG_1_6   0
#define NG_1_2   1
#define NG_1     2
#define NG_2     3




/*
-- TODO: Change to enumerables
*/
#define DCI_FORMAT_0    0
#define DCI_FORMAT_1    1
#define DCI_FORMAT_1A   2





#define PDSCH_MODULATION_BPSK   0
#define PDSCH_MODULATION_QPSK   1
#define PDSCH_MODULATION_16QAM  2
#define PDSCH_MODULATION_64QAM  3

#define PDSCH_TX_SCHEME_PORT_0  0
#define PDSCH_TX_SCHEME_DIVERS  1




/*
-- TODO: Change to enumerables
-- TODO: Think of how to use the same constants in MAC in C
-- (e.g. through shared include file)
*/
#define MAC_NO_PKT     -1

#define MAC_DL_SCH      0
#define MAC_UL_SCH      1
#define MAC_BCH         2
#define MAC_PCH         3
#define MAC_RACH        4
#define MAC_DL_CCH	5
#define MAC_UL_CCH      6
#define MAC_DL_EMPTY    7        
#define MAC_DL_CTS      8

#define MAC_SIB1        0
#define MAC_SIB2        1
#define MAC_RAR         2
#define MAC_RRC_Setup   3
#define MAC_RRC_Reconf  4
#define MAC_DCI0		5
#define MAC_IP			6
#define MAC_EMPTY		7        

#define MAC_PUCCH_ACK   0
#define MAC_PUCCH_NACK  1
#define MAC_PUCCH_SR    2
#define MAC_PUCCH_NSR   3
