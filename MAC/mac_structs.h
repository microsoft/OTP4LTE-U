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
#include <types.h>

// Macro DEFINEs shared between Ziria and C
#include "../DnlinkTX/macro_def.h"


// Make sure this matches Ziria definition to the last bit!!!
#ifndef STRUCT_MACDLPACKET
#define STRUCT_MACDLPACKET
typedef struct {
	int16 frame;								// Where to schedule the packet
	int16 subframe;
	int8 packetType;							// MAC_DL_SCH, etc
	int8 packetSubtype;							// SIB1, SIB2, RAR, etc
	unsigned char data[MAX_BIT_MSG_SIZE/8];		// message data
	int16 len;									// data length in bits
	int16 RV;									// redundancy version(36.321. 5.3.1, last paragraph)
	int16 HARQNo;								// HARQ channel number
	int16 NDI;									// New data indicator
	int32 RNTI;									// RNTI to be used(this has to be int32 since we don't have unsigned int16)
	int16 lenRB;								// Number of RBs to be used to send the packet
	int16 startRB;								// Index of start RB
} MACDLPacket;
#endif

// No packet - packet to sent do Ziria when nothing else is available
const MACDLPacket noPkt = { 0, 0, MAC_NO_PKT, MAC_NO_PKT, "", 0, 0, 0, 0, 0, 0, 0 };



// Make sure this matches Ziria definition to the last bit!!!
#define ULMSG_DATA_LEN		(MAX_UL_BIT_MSG_SIZE/8)
#ifndef STRUCT_MACULPACKET
#define STRUCT_MACULPACKET
typedef struct {
	int16 frame;								// Where to schedule the packet
	int16 subframe;
	int8 packetType;							// MAC_DL_SCH, etc
	unsigned char data[ULMSG_DATA_LEN];			// message data
	int16 len;									// data length in bits
	unsigned char crcOK;						// CRC OK
} MACULPacket;
#endif


