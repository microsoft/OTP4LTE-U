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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>



#include "mac.h"
#include "mac_tx_out_queue.h"
#include "txrx_sync.h"
#include "debug.h"
#include "mac_msg_parsing.h"
#include "tbs_table.h"
#include "mcs_table.h"



extern int LTEBand;

//const int sib1_len = 18;
unsigned char sib1[sib1_len];
//const int sib2_len = 28;
unsigned char sib2[sib2_len];



inline void writeIntAtBitLoc(unsigned char * buf, unsigned char num, int bitPos, int bitWidth)
{
	int bytePos = bitPos / 8;
	int relBitPos = bitPos % 8;
	int BW1 = (8 - relBitPos);
	int BW2 = bitWidth - BW1;
	assert(bitWidth <= 8);

	char mask = ((1 << relBitPos) - 1) << (8 - relBitPos);
	if (BW2 < 0)
	{
		mask |= (1 << (-BW2)) - 1;
	}
	buf[bytePos] &= mask;
	if (BW2 < 0)
	{
		buf[bytePos] |= (num << (-BW2));
	}
	else {
		buf[bytePos] |= (num >> BW2);
	}
	buf[bytePos + 1] &= (1 << (8 - BW2)) - 1;
	buf[bytePos + 1] |= num << (8 - BW2);
}


// Changing band:
// - Change FreqBandIndicator value in SIB1
// - Change ul-CarrierFreq(ARFCN-ValueEUTRA) value for uplink in SIB2
// - Change TX/RX frequencies in run_eNodeB.sh
// NB: Dnlink frequency is auo-detected by UE and UL is specified in SIB2
// NB: It seems that DL and UL have to be moved together, keeping distance constant, to make it work! (learned from IP Access)
//     For example, Qualcomm Huawei dongle seems to ignore EARFCNUL and only work at 709 MHz because DL doesn't change
void initSIBs(int band, float DLfreqIn, float ULfreqIn, bool isUS)
{
	unsigned char sib1_template1[sib1_len] = { 0x68, 0x4c, 0x42, 0x82, 0x19, 0x10, 0xf6, 0x6e, 0x82, 0x11, 0x92, 0x46, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char sib2_template1[sib2_len] = { 0x00, 0x00, 0x00, 0xfe, 0x9f, 0x84, 0x80, 0x00, 0x18, 0x5a, 0x7a, 0x02, 0x00, 0x04, 0x08, 0x00, 0xc2, 0xca, 0xd5, 0x56, 0x0f, 0x6e, 0x86, 0xb9, 0xc8, 0xc1, 0x00, 0x00 };
	const int freqBandPos1 = (12 * 8) + 3;

	unsigned char sib1_template2[sib1_len] = { 0x68, 0x40, 0x04, 0x03, 0x00, 0x01, 0x00, 0x00, 0x10, 0x09, 0x24, 0x6A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char sib2_template2[sib2_len] = { 0x00, 0x00, 0x00, 0xfe, 0x9f, 0x84, 0x80, 0x00, 0x18, 0x5a, 0x7a, 0x02, 0x00, 0x04, 0x08, 0x00, 0xc2, 0xca, 0xd5, 0x56, 0x0f, 0x6e, 0x86, 0xb9, 0xc8, 0xc1, 0x00, 0x00 };
	const int freqBandPos2 = (11 * 8) + 7;


	// From http://niviuk.free.fr/lte_band.php, ulFreqEARFCN[2*(i-1)] - freq, ulFreqEARFCN[2*i-1] = EARFCN
	float ulFreqEARFCN_low[26 * 2] = { 1920, 18000, 1850, 18600, 1710, 19200, 1710, 19950, 824, 20400,
		830, 20650, 2500, 20750, 880, 21450, 1749.9, 21800, 1710, 22150,
		1427.9, 22750, 699, 23010, 777, 23180, 788, 23280, 704, 23730,
		815, 23850, 830, 24000, 832, 24150, 1447.9, 24450, 3410, 24600,
		2000, 25500, 1626.5, 25700, 1850, 26040, 814, 26690, 807, 27040,
		703, 27210 };

	// Duplex distance - this has to be constant, so we check
	float ulFreqEARFCN_space[26 * 2] = { 190, 80, 95, 400, 45, 45, 120, 45, 95, 400, 48, 30, -31, -30, 30, 45, 45, -41, 48, 100, 180, -101.5, 80, 45, 45, 55 };


	int freqBandPos;
	float DLfreq;
	float ULfreq;
	if (DLfreqIn > 1000000.0) {
		DLfreq = DLfreqIn / 1000000.0;
	}
	else {
		DLfreq = DLfreqIn;
	}

	if (ULfreqIn > 1000000.0) {
		ULfreq = ULfreqIn / 1000000.0;
	}
	else {
		ULfreq = ULfreqIn;
	}

	printf("Creating SIBs for band %d, DLFreq=%.1f MHz, DLFreq=%.1f MHz\n", band, DLfreq, ULfreq);

	if (isUS) {
		memcpy(sib1, sib1_template1, sib1_len);
		memcpy(sib2, sib2_template1, sib2_len);
		freqBandPos = freqBandPos1;
	}
	else
	{
		memcpy(sib1, sib1_template2, sib1_len);
		memcpy(sib2, sib2_template2, sib2_len);
		freqBandPos = freqBandPos2;
	}

	// SIB1
	writeIntAtBitLoc(sib1, band - 1, freqBandPos, 6);

	// SIB2
	const int ULEUTRA_pos = (22 * 8) + 7;
	int bandind = (band <= 14) ? (band) : (band - 2);		// No bands 15 and 16
	unsigned int ARFCN_ValueEUTRA = (unsigned int)(10.0 * (ULfreq - ulFreqEARFCN_low[2 * (bandind - 1)]))
		+ (unsigned int)ulFreqEARFCN_low[2 * bandind - 1];
	writeIntAtBitLoc(sib2, (ARFCN_ValueEUTRA >> 8), ULEUTRA_pos, 8);
	writeIntAtBitLoc(sib2, (ARFCN_ValueEUTRA & 0xff), ULEUTRA_pos + 8, 8);

	//writeIntAtBitLoc(sib2, (unsigned char) ra_ResponseWindowSize, (4 * 8) + 2, 3);
	//writeIntAtBitLoc(sib2, (unsigned char) maxHARQ_Msg3Tx, (5 * 8), 3);

	/*
	if (ulFreqEARFCN_space[bandind - 1] != DLfreq - ULfreq)
	{
		printf("Error: DLfreq=%.1f MHz, ULfreq=%.1f MHz and distance has to be %.1f MHz!", DLfreq, ULfreq, ulFreqEARFCN_space[bandind - 1]);
		exit(1);
	}
	*/
}



void fill_SIB1(MACDLPacket *pkt, int16 frame)
{
	pkt->frame = frame;
	pkt->subframe = SIB1SF;
	pkt->packetType = MAC_DL_SCH;
	pkt->packetSubtype = MAC_SIB1;
	memcpy((void *)pkt->data, sib1, 18 * sizeof(unsigned char));
	pkt->len = 144;
	pkt->RV = (int16)fmod(ceil(3.0 * fmod(((double)pkt->frame / 2.0), 4.0) / 2.0), 4);
	pkt->NDI = 0;
	pkt->HARQNo = 0;
	pkt->RNTI = SI_RNTI;
	pkt->lenRB = 12;
	pkt->startRB = 0;
}

void fill_SIB2(MACDLPacket *pkt, int16 frame)
{
	pkt->frame = frame;
	pkt->subframe = SIB2SF;
	pkt->packetType = MAC_DL_SCH;
	pkt->packetSubtype = MAC_SIB2;
	memcpy((void *)pkt->data, sib2, sib2_len * sizeof(unsigned char));
	pkt->len = 224;
	// TODO: 36.321 5.3.1 last para; safe to hard-code 0 for our scheduler
	pkt->RV = 0;
	pkt->NDI = 0;
	pkt->HARQNo = 0;
	pkt->RNTI = SI_RNTI;
	pkt->lenRB = 12;
	pkt->startRB = 0;
}





/*
let rar =
{ '0, -- Extension
'1, -- Type
'0,'0,'0,'0,'0,'0, -- RAPID; overwrite this
'0, -- Reserved
'0,'0,'0,'0,'0,'0,'0,'0,'0,'0,'0, -- Timing Advance
'0, -- Hopping
'0,'0,'1,'1,'0,'0,'1,'0,'0,'0, -- RB assignment; 200 = 5 PRBs starting at 0; 120 bits for our MCS
'0,'0,'0,'0, -- MCS; 0 means QPSK, I_TBS = 0, rv_idx = 0
'0,'0,'0, -- TPC; 0DB PUSCH power
'0, -- UL delay
'0, -- CQI request
'0,'0,'0,'0,'0,'0,'0,'0,'0,'1,'0,'0,'0,'0,'0,'0 -- Temp C-RNTI = 64
};
*/
//unsigned char rar[7] = { 0x43, 0x00, 0x01, 0x90, 0x00, 0x00, 0x40 };

/*
let rar =
{ '0, -- Extension
'1, -- Type
'0,'0,'0,'0,'0,'0, -- RAPID; overwrite this
'0, -- Reserved
'0,'0,'0,'0,'0,'0,'0,'0,'0,'0,'0, -- Timing Advance
'0, -- Hopping
-- '0,'1,'0,'1,'0,'1,'1,'1,'1,'0, -- RB assignment; 350 = 8 PRBs starting at 0; 208 bits for our MCS
'0,'1,'0,'1,'0,'1,'1,'1,'1,'1, -- RB assignment; 351 = 8 PRBs starting at 1; 208 bits for our MCS
'0,'0,'0,'0, -- MCS; 0 means QPSK, I_TBS = 0, rv_idx = 0
'0,'0,'0, -- TPC; 0DB PUSCH power
'0, -- UL delay
'0, -- CQI request
'0,'0,'0,'0,'0,'0,'0,'0,'0,'1,'0,'0,'0,'0,'0,'0 -- Temp C-RNTI = 64
};
*/
// The old RAR, with PRB=8, starting at 0 we can decode with our ifft
//unsigned char rar[7] = { 0x40, 0x00, 0x02, 0xBC, 0x00, 0x00, 0x40 };
// The new RAR, with PRB=8, starting at 1 we can decode with our ifft
unsigned char rar[7] = { 0x40, 0x00, 0x02, 0xBE, 0x00, 0x00, 0x40 };


// RAR
void fill_RAR(MACDLPacket *pkt, int16 frame, int16 subframe)
{
	// RAR
	pkt->frame = frame;
	pkt->subframe = subframe;
	pkt->packetType = MAC_DL_SCH;
	pkt->packetSubtype = MAC_RAR;
	memcpy((void *)pkt->data, rar, 7 * sizeof(unsigned char));
	pkt->len = 56;
	// TODO: should we be using DCI format 1C for RAR ?
	pkt->RV = 0;
	pkt->NDI = 0;
	pkt->HARQNo = 0;
	pkt->RNTI = 2;			// RA-RNTI
	pkt->lenRB = 12;
	pkt->startRB = 0;
}



void fill_DCI0(MACDLPacket *pkt, int16 frame, int16 subframe)
{
	static unsigned char DCI0NDI0[4] = { 0x50, 0x1F, 0x10, 0x00 };
	static unsigned char DCI0NDI1[4] = { 0x50, 0x1F, 0x14, 0x00 };

	pkt->frame = frame;
	pkt->subframe = subframe;
	pkt->packetType = MAC_DL_CCH;
	pkt->packetSubtype = MAC_DCI0;
	// Set NDI according to the formula devised by Doug on 13/6/2015
	if ((((frame * 10) + subframe) >> 3) % 2)
	{
		memcpy((void *)pkt->data, DCI0NDI1, 4 * sizeof(unsigned char));
	}
	else
	{
		memcpy((void *)pkt->data, DCI0NDI0, 4 * sizeof(unsigned char));
	}
	pkt->len = 27;
	pkt->RV = 0;			// N/A
	pkt->NDI = 0;			// N/A
	pkt->HARQNo = 0;		// N/A
	pkt->RNTI = SI_RNTI;	// N/A
	pkt->lenRB = 12;		// N/A
	pkt->startRB = 0;		// N/A
}


// Empty packet used to generate an empty subframe (no RS)
// simulating LTE-U's off cycle
inline void fill_EMPTY(MACDLPacket *pkt, int16 frame, int16 subframe)
{
	pkt->frame = frame;
	pkt->subframe = subframe;
	pkt->packetType = MAC_DL_EMPTY;
	pkt->packetSubtype = MAC_EMPTY;
	pkt->len = 0;			// N/A
	pkt->RV = 0;			// N/A
	pkt->NDI = 0;			// N/A
	pkt->HARQNo = 0;		// N/A
	pkt->RNTI = SI_RNTI;	// N/A
	pkt->lenRB = 12;		// N/A
	pkt->startRB = 0;		// N/A
}


// LTE-U off cycle
void schedule_EMPTY(MACDLPacket *pkt, int16 frame, int16 subframe)
{
	int nextFrame = frame;

	if (mac_context.nextESF == noEmptySubF)
	{
		nextFrame = (frame + 1) % LTE_NO_FRAMES;
		mac_context.nextESF = 0;
	}

	fill_EMPTY(pkt, nextFrame, emptySubF[mac_context.nextESF]);

	mac_context.nextESF++;
}




extern unsigned int rlc_tx_sn[4];
int mac_tx_get_IP_packets(unsigned char * tx_bytes, int outlen)
{
	sdu ip;
	PACKET_HANDLE ReadEthernetFramePacketHandle;
	PUCHAR ReadEthernetFramePointer = NULL;
	UINT ReadEthernetFrameBytesSize = 0;
	DWORD timeout = 0; // milliseconds
	HRESULT hResult = SoraUGetTxPacket(&ReadEthernetFramePacketHandle, (void**)&ReadEthernetFramePointer,
		&ReadEthernetFrameBytesSize, timeout);

#ifdef MAC_TEST
	if (genIP)
	{
		// Some arbitrary payload
		genIP = false;

		// TODO: Seems that lcid of IP is 3. Is that always right?
		ip.lcid = 3;

		ip.len = 20;
		// Remove ethernet header, send IP only

		TimingInfo txTime = __ext_getTXTime();
		logIP(DEBUG_LOG_DN, txTime, 3, "IP", ip.bytes, ip.len);


		int pdcp_len, rrc_len, rlc_len;
		unsigned char pdcp_bytes[MAX_MAC_MSG_SIZE], rrc_bytes[MAX_MAC_MSG_SIZE], rlc_bytes[MAX_MAC_MSG_SIZE];
		unsigned int pdcp_sn = 0; // TODO: what to do here?
		pdcp_len = pdcp_encode(pdcp_bytes, 2048, ip.lcid, &pdcp_sn, ip.bytes, ip.len);
		rlc_len = rlc_encode(rlc_bytes, 2048, ip.lcid, &rlc_tx_sn[ip.lcid], pdcp_bytes, pdcp_len);
		memcpy(ip.bytes, rlc_bytes, rlc_len);
		ip.len = rlc_len;

		return mac_encode(tx_bytes, outlen, &ip, 1);
	}
#endif

	if (hResult == S_OK && ReadEthernetFrameBytesSize > 14 + 4)
	{
		// TODO: Seems that lcid of IP is 3. Is that always right?
		ip.lcid = 3;

		ip.len = min(MAX_MAC_MSG_SIZE, ReadEthernetFrameBytesSize - 14);
		// Remove ethernet header, send IP only
		memcpy(ip.bytes, (void *)(ReadEthernetFramePointer + 14), ip.len * sizeof(char));

		TimingInfo txTime = __ext_getTXTime();
		logIP(DEBUG_LOG_DN, txTime, 3, "IP", ip.bytes, ip.len);


		// DEBUG
		//char str[4096];
		//bytes_to_hex(str, 4096, ReadEthernetFramePointer, ReadEthernetFrameBytesSize);
		//printf("IP taken from NDIS: size: %d, msg:%s\n", ReadEthernetFrameBytesSize, str);
		//bytes_to_hex(str, 4096, ip.bytes, ip.len);
		//printf("IP: size: %d, msg:%s\n", ip.len, str);


		HRESULT hResult = SoraUCompleteTxPacket(ReadEthernetFramePacketHandle, STATUS_SUCCESS);

		int pdcp_len, rrc_len, rlc_len;
		unsigned char pdcp_bytes[MAX_MAC_MSG_SIZE], rrc_bytes[MAX_MAC_MSG_SIZE], rlc_bytes[MAX_MAC_MSG_SIZE];
		unsigned int pdcp_sn = 0; // TODO: what to do here?
		pdcp_len = pdcp_encode(pdcp_bytes, 2048, ip.lcid, &pdcp_sn, ip.bytes, ip.len);
		rlc_len = rlc_encode(rlc_bytes, 2048, ip.lcid, &rlc_tx_sn[ip.lcid], pdcp_bytes, pdcp_len);
		memcpy(ip.bytes, rlc_bytes, rlc_len);
		ip.len = rlc_len;

		return mac_encode(tx_bytes, outlen, &ip, 1);
	}
	else
	{
		return -1;
	}
}





// Look up the smalles transport block size larger than message <len>
// that can be used to encode the packet on <NRB> resource blocks with <modulation>
// NB: The largest possible NRB==23 for 5MHz channel with 50 RB
int getTBSFromNRB(int modulation, int NRB, int len)
{
	int minInd=0, maxInd=-1;
	switch (modulation)
	{
	case PDSCH_MODULATION_QPSK:
		minInd = mcs_interval_table[0][0];
		maxInd = mcs_interval_table[0][1];
		break;
	case PDSCH_MODULATION_16QAM:
		minInd = mcs_interval_table[1][0];
		maxInd = mcs_interval_table[1][1];
		break;
	case PDSCH_MODULATION_64QAM:
		minInd = mcs_interval_table[2][0];
		maxInd = mcs_interval_table[2][1];
		break;
	default:
		break;
	}

	int ind = minInd;
	while (tbs_table[ind * 23 + (NRB - 1)] < len && ind <= maxInd)
	{
		ind++;
	}
	if (ind <= maxInd && tbs_table[ind * 23 + (NRB - 1)] >= len)
	{
		return tbs_table[ind * 23 + (NRB - 1)];
	}

	return -1;
}




// Look up the smalles transport block size larger than message <len>
// that can be used to encode the packet on <NRB> resource blocks with <modulation>
// NB: The largest possible NRB==23 for 5MHz channel with 50 RB
int getTBSFromNRB_special(int modulation, int NRB)
{
	int minInd = 0, maxInd = -1;
	switch (modulation)
	{
	case PDSCH_MODULATION_QPSK:
		minInd = mcs_interval_table[0][0];
		maxInd = mcs_interval_table[0][1];
		break;
	case PDSCH_MODULATION_16QAM:
		minInd = mcs_interval_table[1][0];
		maxInd = mcs_interval_table[1][1];
		break;
	case PDSCH_MODULATION_64QAM:
		minInd = mcs_interval_table[2][0];
		maxInd = mcs_interval_table[2][1];
		break;
	default:
		break;
	}


	return tbs_table[3 * 23 + (NRB - 1)];
}



// Look up the smalles transport block size (TBBS) larger than message <len>
// that can be used to encode the packet with <mcs>, and also 
// returns the corresponding <NRB> and <modulation>
// NB: The largest possible NRB==23 for 5MHz channel with 50 RB
int getTBSFromMCS(int mcs, int len, int *modulation, int16 *NRB)
{
	if (mcs < 0 || mcs > 28)
	{
		printf("ERROR: MCS has to be between 0 and 28!\n");
		return -1;
	}
	*modulation = mcs_table[mcs][1];
	int iTBS = mcs_table[mcs][2];

	int ind = 0;
	while (tbs_table[iTBS * 23 + ind] < len && ind < 23)
	{
		ind++;
	}
	if (ind < 23 && tbs_table[iTBS * 23 + ind] >= len)
	{
		*NRB = ind+1;
		return tbs_table[iTBS * 23 + ind];
	}

	*NRB = -1;
	return -1;
}


void updateMCSStats(int16 mcs, int16 lenRB)
{
	if (mcs >= 0 && mcs < MAC_NO_MCS)
	{
		txMCS[mcs]++;
	}
	if (mcs >= 1 && mcs <= MAC_NO_NBR)
	{
		txNBR[lenRB - 1]++;
	}

}



// Compare (frame1, subframe1) with (frame2, subframe2) and returns:
// 0 - if equal
// 1 - if (frame2, subframe2) < (frame1, subframe1)
// 2 - if (frame2, subframe2) > (frame1, subframe1)
int compareTime(int16 frame1, int16 subframe1, int16 frame2, int16 subframe2)
{
	if (frame1 == frame2 && subframe1 == subframe2)
	{
		return 0;
	}
	if ((frame1 != frame2 && (LTE_NO_FRAMES + frame2 - frame1) % LTE_NO_FRAMES < 512) || (frame1 == frame2 && subframe2 > subframe1))
	{
		return 2;
	}
	return 1;
}


// Make sure you don't schedule more than MAX_TIME_HORIZON frames ahead of time
// (frame1, subframe1) - current time
// (frame2, subframe2) - next schedule
int checkTimeHorizon(int16 frame1, int16 subframe1, int16 frame2, int16 subframe2)
{
	if (compareTime(frame1, subframe1, frame2, subframe2) == 2 &&
		(LTE_NO_FRAMES + frame2 - frame1) % LTE_NO_FRAMES > MAX_TIME_HORIZON)
	{
		return 0;
	}
	return 1;
}



// Scheduling overview:
// Dnlink:
// - Subframe 0: SIB2 (every 8 frames) using CCE0, DCI0 (every frame) using CCE4
// - Subframe 1: EMPTY!
// - Subframe 2: RRC
// - Subframe 3: EMPTY!
// - Subframe 4: IP
// - Subframe 5: SIB1 (every 2 frames) using CCE0
// - Subframe 6: RAR (every frame) using CCE0
// - Subframe 7: EMPTY!
// - Subframe 8: IP
// - Subframe 9: EMPTY!

// Schedule next RRC packet
// We currently schedule RRC packets in subframes 2
void nextRRCSubframe(int16 *frame, int16 *subframe)
{
	if (*subframe+1 > 2)
	{
		*frame = (*frame + 1) % LTE_NO_FRAMES;
	}
	*subframe = 2;
}


// Schedule next IP packet
// List of subframes available for data scheduling is given in dataSubF
void nextIPSubframe(int16 *frame, int16 *subframe)
{
	if (*subframe + 1 > dataSubF[noDataSubF-1])
	{
		*frame = (*frame + 1) % LTE_NO_FRAMES;
		*subframe = dataSubF[0];
	} else {
		int i = 0;
		while (i < noDataSubF && *subframe + 1 > dataSubF[i])
		{
			i++;
		}
		if (i >= noDataSubF) i = noDataSubF - 1;
		*subframe = dataSubF[i];
	}
}


// Toggle NDI for every consecutive DLSCH packet in the same HARQ, as instructed by Doug on 13/6/15
// Increase HARQ buffer after every transmission, as instructed by Doug on 5/10/15
void updateHARQ(int16 *lastHARQ, int16 *lastNDI, int16 maxHARQ)
{
	if (*lastHARQ == maxHARQ - 1)
	{
		*lastHARQ = 0;
		// Toggle NDI for every consecutive DLSCH packet, as instructed by Doug on 13/6/15
		*lastNDI = (*lastNDI + 1) % 2;
	}
	else
	{
		(*lastHARQ)++;
	}
}