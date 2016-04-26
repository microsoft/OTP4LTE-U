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

#ifdef SORA_PLATFORM
#include <winsock2.h> // ws2_32.lib required
#include <ws2tcpip.h>

#include <sora.h>
#include <brick.h>
#include <dspcomm.h>
#include <soratime.h>
#include <windows.h>

#include "sora_radio.h"
#include "sora_ip.h"
#endif
#include "sora_threads.h"
#include "sora_thread_queues.h"

#include "mac.h"
#include "mac_tx_out_queue.h"
#include "txrx_sync.h"
#include "debug.h"
#include "mac_msg_parsing.h"

#ifdef BLADE_RF
  #include "bladerf_radio.h"
#endif

extern int LTEBand;





// TODO: calculate PRACH subframe and RA_RNTI based on this one
const int16 RAR_SUBFRAME = 6;




MACContext mac_context;


// Here we schedule packets that should be sent regularly
// (e.g. MIB, SIB1, SIB2, LTE-U off cycle, etc)
// The first of each of the packets is schedule here
// The subsequent is recurrently scheduled from mac_tx_callback()
void init_mac_tx_callback()
{
	MACDLPacket newPkt;
	// SIB2
	fill_SIB2(&newPkt, 0);
	add_mac_tx_out_queue(newPkt);
	// TODO: RAR should not be hard-coded
	// RAR
	fill_RAR(&newPkt, 0, RAR_SUBFRAME);
	add_mac_tx_out_queue(newPkt);
	// SIB1
	fill_SIB1(&newPkt, 0);
	add_mac_tx_out_queue(newPkt);
	// DCI0
	fill_DCI0(&newPkt, 0, 0);
	add_mac_tx_out_queue(newPkt);
	// EmptySF
	if (noEmptySubF > 0)
	{
		fill_EMPTY(&newPkt, 0, emptySubF[0]);
		mac_context.nextESF = 1;
	}
	add_mac_tx_out_queue(newPkt);

#ifdef TEST
	// Generate some packets for test
	for (int16 frame = 0; frame < 8; frame++)
	{
		const int pkt_len = 65;
		const unsigned char pkt_template[pkt_len] = { 0x21, 0x0D, 0x1F, 0xA0, 0x00, 0x00, 0x08, 0x00, 0x18, 0x3a, 0xa8, 0x08, 0x00, 0x00, 0x00, 0x00 };
			//0x3C, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			//0x60, 0x10, 0x9B, 0x00, 0x30, 0x00, 0x0B, 0x00, 0x00, 0x00,
			//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			//0x00, 0x00, 0x00 };
		MACDLPacket *pkt = &newPkt;

		pkt->frame = frame;
		pkt->subframe = 4;
		pkt->packetType = MAC_DL_SCH;
		pkt->packetSubtype = MAC_RRC_Setup;
		memset((void *)pkt->data, 0, pkt_len * sizeof(unsigned char));
		//memcpy(pkt->data, pkt_template, sizeof(pkt_template));
		pkt->len = pkt_len * 8;
		pkt->RV = 0;
		pkt->HARQNo = 0;
		pkt->NDI = frame % 2;
		pkt->RNTI = 64;
		add_mac_tx_out_queue(newPkt);
	}
#endif

}




void init_mac_context()
{
	// Last NDI and HARQ used
	mac_context.lastNDI = 0;
	mac_context.lastHARQ = 0;

	// One before the first available subframe to schedule the next RRC packet
	// NOTE: At the moment we still cannot send on subframes 1,3,7 or 9 due to the weird UE bug!
	mac_context.lastRRCframe = 0;
	mac_context.lastRRCsubFrame = 0;

	// This is a position of packet inside subframe
	// We cannot use position as long as we have hardcoded DCI0s
	//static int16 lastRRCPosition = 0;

	// One before the first available subframe to schedule the next IP packet
	// NOTE: At the moment we still cannot send on subframes 1,3,7 or 9 due to the weird UE bug!
	mac_context.lastIPframe = 0;
	mac_context.lastIPsubFrame = 0;

	// Index of next empty subframe in <emptySubF>, for LTE-U off cycle
	mac_context.nextESF = 0;
}




void init_mac_tx()
{
	init_mac_context();
	init_mac_tx_out_queue();
	init_mac_tx_callback();
}






// *********************************************************************************************
// Schedule signalling packets (RRC)
// Check if there is a new message from RX in mac_rx_out_queue
// *********************************************************************************************
void mac_tx_schedule_RRC()
{
	TimingInfo time = __ext_getTXTime();
	TimingInfo rxTime = __ext_getRXTime();
	MACULPacket ULSCHPkt;
	MACDLPacket newPkt;

	while (mac_rx_out_queue != NULL && !s_ts_isEmpty(mac_rx_out_queue, 0))
	{
		s_ts_get(mac_rx_out_queue, 0, (char *)&ULSCHPkt);

		if (ULSCHPkt.packetType == MAC_UL_CCH)
		{
			if (debugPrint(DEBUG_PRINT_ULCCH))
			{
				printf("MAC PUCCH at (%d, %d) - ", ULSCHPkt.frame, ULSCHPkt.subframe);
				switch (ULSCHPkt.len)
				{
				case MAC_PUCCH_ACK:
					printf("ACK\n");
					break;
				case MAC_PUCCH_NACK:
					printf("NACK\n");
					break;
				case MAC_PUCCH_SR:
					printf("SR\n");
					break;
				case MAC_PUCCH_NSR:
					printf("NSR\n");
					break;
				}
			}

			// Now we just ignore PUCCH and send DCI0 regularly
			// TODO: Add DCI0 here...
			//fill_DCI0(&newPkt, (time.frame + 1) % LTE_NO_FRAMES, 0);
			//add_mac_tx_out_queue(newPkt);
			rxCCH++;
		}
		else
		{
			// Process received packet including creating RRC response
			rxSCH++;
			memset(newPkt.data, 0, 768);
			if (debugPrint(DEBUG_PRINT_ULSCH) && !(ULSCHPkt.data[0] == 0x3e && ULSCHPkt.data[1] == 0x1f))
			{
				char str[1000];
				bytes_to_hex(str, 1000, ULSCHPkt.data, ULSCHPkt.len >> 3);
				printf("^ %4d.%d %s \n", rxTime.frame, rxTime.subFrame, str);
			}
			newPkt.len = end_to_end(ULSCHPkt.data, (ULSCHPkt.len >> 3), newPkt.data, 1050) * 8;
			//if (newPkt.len > 0) { char str[1000]; bytes_to_hex(str, 1000, newPkt.data, newPkt.len >> 3); printf("v %s\n", str); }

			// DEBUG
			//char str[1000];
			//bytes_to_hex(str, 1000, ULSCHPkt.data, newPkt.len/8);



			if (newPkt.len > 0)
			{
				// Schedule
				if (checkTimeHorizon(time.frame, time.subFrame, mac_context.lastRRCframe, mac_context.lastRRCsubFrame) == 1)
				{
					nextRRCSubframe(&mac_context.lastRRCframe, &mac_context.lastRRCsubFrame);
				}
				else
				{
					printf("RRC queue full (time.frame=%d, time.subframe=%d, lastRRCframe=%d, lastRRCsubFrame=%d)!\n",
						time.frame, time.subFrame, mac_context.lastRRCframe, mac_context.lastRRCsubFrame);
					errQue++;
					break;
				}


				int orig_len = newPkt.len;
				newPkt.len = 153 * 8;
				// The longest example
				// newPkt.len = 233 * 8;

				//newPkt.frame = (time.frame + 1) % LTE_NO_FRAMES;
				//newPkt.subframe = 2;
				newPkt.frame = mac_context.lastRRCframe;
				newPkt.subframe = mac_context.lastRRCsubFrame;

				newPkt.packetType = MAC_DL_SCH;
				newPkt.packetSubtype = MAC_RRC_Setup;		// TODO: Not really used, remove
				newPkt.RV = 0;
				newPkt.RNTI = 64;
				newPkt.lenRB = 12;
				//newPkt.lenRB = 23;   // BR: I believe this is the largest possible with 50 NRB
				newPkt.startRB = 0;

				newPkt.NDI = mac_context.lastNDI;
				newPkt.HARQNo = mac_context.lastHARQ;

				updateHARQ(&mac_context.lastHARQ, &mac_context.lastNDI, maxHARQBuf);

				// DEBUG
				//fill_RRC_Conn_Setup(&newPkt, (time.frame + 1) % LTE_NO_FRAMES, 4, RRC_ID);

				if (add_mac_tx_out_queue(newPkt) == 0)
				{
					logError("Error: mac_tx_out_queue full, dropping packet!\n");
					errQue++;
				}

				logDLSCH(newPkt, orig_len, "RRC");
				txSCH++;
				updateMCSStats(-1, newPkt.lenRB);
			}
		}
	}


	// Make sure these are not lagging behind
	if (compareTime(time.frame, time.subFrame, mac_context.lastRRCframe, mac_context.lastRRCsubFrame) == 1)
	{
		mac_context.lastRRCframe = time.frame;
		mac_context.lastRRCsubFrame = time.subFrame;
	}
}



void mac_tx_schedule_IP()
{
	TimingInfo time = __ext_getTXTime();
	TimingInfo rxTime = __ext_getRXTime();
	MACDLPacket newPkt;

	// *********************************************************************************************
	// Schedule IP packets
	// *********************************************************************************************
	do
	{
		int orig_len = -1;
		int mcs;
		int modulation = PDSCH_MODULATION_QPSK;					// Currently hardcoded to QPSK and unused

		if (checkTimeHorizon(time.frame, time.subFrame, mac_context.lastIPframe, mac_context.lastIPsubFrame) == 1)
		{
			newPkt.len = mac_tx_get_IP_packets(newPkt.data, 1050) * 8;
			if (newPkt.len <= 0)
			{
				// No packet
				break;
			}
		}
		else
		{
			logError("IP queue full!\n");
			errQue++;
			break;
		}

		orig_len = newPkt.len;

		// Rate selection:
		if (dnlinkMCS >= 0)
		{
			newPkt.len = getTBSFromMCS(dnlinkMCS, newPkt.len, &modulation, &newPkt.lenRB);
			mcs = dnlinkMCS;
		}
		else if (dnlinkNRB >= 0)
		{
			// Get the smallest possible transport block size (TBS) that can fit the packet
			newPkt.len = getTBSFromNRB(PDSCH_MODULATION_QPSK, dnlinkNRB, newPkt.len);
			newPkt.lenRB = dnlinkNRB;
			mcs = -1;
		}
		else
		{
			newPkt.len = 153 * 8;
			newPkt.lenRB = 12;
			mcs = -1;
		}


		if (newPkt.len > 0)
		{
			// Schedule the IP packet
			nextIPSubframe(&mac_context.lastIPframe, &mac_context.lastIPsubFrame);

			//newPkt.frame = (time.frame + 1) % LTE_NO_FRAMES;
			//newPkt.subframe = 4;
			newPkt.frame = mac_context.lastIPframe;
			newPkt.subframe = mac_context.lastIPsubFrame;

			newPkt.packetType = MAC_DL_SCH;
			newPkt.packetSubtype = MAC_IP;		// TODO: Not really used, remove
			newPkt.RV = 0;
			newPkt.RNTI = 64;

			newPkt.startRB = 0;
			newPkt.NDI = mac_context.lastNDI;
			newPkt.HARQNo = mac_context.lastHARQ;

			updateHARQ(&mac_context.lastHARQ, &mac_context.lastNDI, maxHARQBuf);

			if (add_mac_tx_out_queue(newPkt) == 0)
			{
				logError("Error: mac_tx_out_queue full, dropping packet!\n");
				errQue++;
			}

			logDLSCH(newPkt, orig_len, "IP");
			txSCH++; txIP++;
			updateMCSStats(mcs, newPkt.lenRB);
		}
		else
		{
			printf("Cannot calculate TBS for packet size %d with NRB=%d\n", orig_len, newPkt.lenRB);
		}
	} while (newPkt.len > 0);


	// Make sure these are not lagging behind
	if (compareTime(time.frame, time.subFrame, mac_context.lastIPframe, mac_context.lastIPsubFrame) == 1)
	{
		mac_context.lastIPframe = time.frame;
		mac_context.lastIPsubFrame = time.subFrame;
	}
}




// *********************************************************************************************
// Count PRACHs and printout if needed
// *********************************************************************************************
void mac_tx_PRACH_stat()
{
	TimingInfo time = __ext_getTXTime();
	TimingInfo rxTime = __ext_getRXTime();
	//printf("MAC TX time: %d, %d\n", time.frame, time.subFrame);

	// This uses a quick 
	int8 rapid = __ext_getRAPID();
	if (rapid != -1)
	{
		if (debugPrint(DEBUG_PRINT_PRACH))
		{
			printf("TX MAC time: (%d, %d), PRACH received, id %d\n", time.frame, time.subFrame, rapid);
		}
		rxRACH++;
		__ext_setRAPID(-1);
	}
}



// *********************************************************************************************
// Schedule all the packets from the queue that are marked for this frame/subframe
// *********************************************************************************************
void mac_tx_schedule_next_SF()
{
	TimingInfo time = __ext_getTXTime();
	TimingInfo rxTime = __ext_getRXTime();
	MACDLPacket *DLSCHPkt = ((MACDLPacket *)buf_ctx_tx.mem_input_buf);
	MACDLPacket newPkt;
	int qptr = find_mac_tx_out_queue(time.frame, time.subFrame);
	int qorig = qptr;

	while (qptr != -1)
	{
		*DLSCHPkt = outQueue[qptr].pkt;
		DLSCHPkt++;
		buf_ctx_tx.chunk_input_entries++;

		if (debugPrint(DEBUG_PRINT_DLSCH))
		{
			if (outQueue[qptr].pkt.packetSubtype == MAC_RRC_Setup)
			{
				printf("->-> (%d,%d): MAC_RRC_Setup\n", time.frame, time.subFrame);
			}

			if (outQueue[qptr].pkt.packetSubtype == MAC_RRC_Reconf)
			{
				printf("->-> (%d,%d): MAC_RRC_Reconf\n", time.frame, time.subFrame);
			}

			if (outQueue[qptr].pkt.packetSubtype == MAC_IP)
			{
				printf("->-> (%d,%d): MAC_IP len=%d\n", time.frame, time.subFrame, outQueue[qptr].pkt.len);
			}
		}

		// For recurrent (control) packet append the follow up to the list
		switch (outQueue[qptr].pkt.packetSubtype)
		{
		case MAC_SIB2:
			// SIB2
			fill_SIB2(&newPkt, (outQueue[qptr].pkt.frame + 8) % LTE_NO_FRAMES);
			if (add_mac_tx_out_queue(newPkt) == 0)
			{
				printf("Error: dropped SIB2, catastrophic!\n");
				errQue++;
			}
			break;

		case MAC_RAR:
			// RAR
			fill_RAR(&newPkt, (outQueue[qptr].pkt.frame + 1) % LTE_NO_FRAMES, RAR_SUBFRAME);
			if (add_mac_tx_out_queue(newPkt) == 0)
			{
				printf("Error: dropped RAR, catastrophic!\n");
				errQue++;
			}
			break;

		case MAC_SIB1:
			// SIB1
			fill_SIB1(&newPkt, (outQueue[qptr].pkt.frame + 2) % LTE_NO_FRAMES);
			if (add_mac_tx_out_queue(newPkt) == 0)
			{
				printf("Error: dropped SIB1, catastrophic!\n");
				errQue++;
			}
			break;

		case MAC_DCI0:
			// DLCCH, DCI0
			//int16 frame = outQueue[qptr].pkt.frame;
			//int16 subframe = outQueue[qptr].pkt.subframe;
			//int16 nextFrame, nextSubFrame;

			//// We schedule in all subframes but 7, since we reserve 7 for ACKs
			//if (subframe == 9) {
			//	nextFrame = (frame + 1) % LTE_NO_FRAMES;
			//	nextSubFrame = 0;
			//} else {
			//	nextFrame = frame;
			//	nextSubFrame = (subframe != 6) ? (subframe + 1) : (subframe + 2);
			//}
			//fill_DCI0(&newPkt, nextFrame, nextSubFrame);
			fill_DCI0(&newPkt, (outQueue[qptr].pkt.frame + 1) % LTE_NO_FRAMES, 0);
			if (add_mac_tx_out_queue(newPkt) == 0)
			{
				printf("Error: dropped DCI0, catastrophic!\n");
				errQue++;
			}
			break;

		case MAC_EMPTY:
			// LTE-U off cycle
			schedule_EMPTY(&newPkt, outQueue[qptr].pkt.frame, outQueue[qptr].pkt.subframe);

			if (add_mac_tx_out_queue(newPkt) == 0)
			{
				printf("Error: dropped Emtpy-SubF, catastrophic!\n");
				errQue++;
			}
			break;

		}

		// Find the next packet from the same subframe, if any
		qptr = get_next_mac_tx_out_queue(qptr);
	}
	if (qorig != -1)
	{
		delete_mac_tx_out_queue(time.frame, time.subFrame);
	}

	// Send no packet meaning the end for this frame
	*DLSCHPkt = noPkt;
	DLSCHPkt++;
	buf_ctx_tx.chunk_input_entries++;
}


//by Lei
//for duty cycle
static int counter = 0;
static int outoncycle = 0;
static int outoffcyle = 0;
static int outinterval = 0;

static int outncts = 19;
static int outctsgap = 20;
static int ctsframes = 0;

static int outctsduration = 10000; //us cts duration

static int sync = 0;

static int oncc = 0;
static int offcc = 0;


static int total_subframes = 0;
static int total_on = 0;
static int total_off = 0;


const int measure_cycle = 1000 * 2; //2s
static long last_time = 0;
struct timeval now;


#ifdef LTE_U
void mac_tx_schedule_next_SF_LTEU()
{
	TimingInfo time = __ext_getTXTime();
	TimingInfo rxTime = __ext_getRXTime();
	MACDLPacket *DLSCHPkt = ((MACDLPacket *)buf_ctx_tx.mem_input_buf);
	MACDLPacket newPkt;

	if (last_time == 0) {
		gettimeofday(&now);
		last_time = now.tv_sec * (int)1e6 + now.tv_usec;
	}

	total_subframes++;
	if (total_subframes > measure_cycle) {
		gettimeofday(&now);
		long cur_time = now.tv_sec * (int)1e6 + now.tv_usec;		
		fprintf(stdout, "subframes sent: %d, real time elapsed: %ld us, total on : %d, total off: %d, dnNRB: %d\n", total_subframes - 1, cur_time - last_time, total_on, total_off, dnlinkNRB);
		total_subframes = 1;
		total_on = 0;
		total_off = 0;
		last_time = cur_time;
	}

	//fprintf(stdout, "on : %d, off: %d\n", lteu_context.onCycle, lteu_context.offCycle);

	EnterCriticalSection(&lteu_context.critSec);
	if (lteu_context.updated) {
		LONGLONG now = SoraGetTimeofDay(&(params_tx->measurementInfo.tsinfo));
		outoncycle = lteu_context.onCycle;
		outoffcyle = lteu_context.offCycle;
		outinterval = lteu_context.interval;
		ctsframes = lteu_context.ctssubframes;
		lteu_context.updated = false;
		if (0==sync) {
			sync = 1;
			counter = 0;
		}
	}
	LeaveCriticalSection(&lteu_context.critSec);

	if (counter == 0 && outoncycle > 0) {
#ifdef BLADE_RF
		int status = BladeRF_SwitchTXFrequency(params_tx->radioParams.dev, true);
		if (status < 0) {
			printf("Error when swiching to center frequency\n");
		}
#else
		int status = Sora_SwitchTXFrequency(params_tx, true);
		if (status < 0) {
			printf("Error when swiching to center frequency\n");
		}
#endif
	}
	if (counter == outoncycle && outoffcyle > 0) {
#ifdef BLADE_RF
		int status = BladeRF_SwitchTXFrequency(params_tx->radioParams.dev, false);
		if (status < 0) {
			printf("Error when swiching to side frequency\n");
		}
#else
		int status = Sora_SwitchTXFrequency(params_tx, false);
		if (status < 0) {
			printf("Error when swiching to side frequency\n");
		}
#endif
	}
	
	if (counter < ctsframes) {
		oncc++;
		total_on++;

		//fprintf(stdout, "sending out CTS from MAC\n");
		outctsduration = (outoncycle - (counter + 1)) * 1000; //the CTS requested duration is less than the outoncycle
		newPkt = { time.frame, time.subFrame, MAC_DL_CTS, MAC_IP, "CTS", outncts, outctsgap, 0, 0, 64, outctsduration, 0};
		*DLSCHPkt = newPkt;
		DLSCHPkt++;
		buf_ctx_tx.chunk_input_entries++;
	} else if (counter < outoncycle) {

		oncc++;
		total_on++;
		if (dnlinkNRB > 0) {
			int maxnrb = 49;
			int len = getTBSFromNRB_special(PDSCH_MODULATION_QPSK, min(dnlinkNRB, maxnrb));
			newPkt = { time.frame, time.subFrame, MAC_DL_SCH, MAC_IP, "filled packet", len, 0, 0, 0, 64, dnlinkNRB, maxnrb - min(dnlinkNRB, maxnrb) };
			memset(newPkt.data, 0xff, 2048);
			*DLSCHPkt = newPkt;
			DLSCHPkt++;
			buf_ctx_tx.chunk_input_entries++;
			
		} else {
			//sending pilots only
			int len = 0;
			newPkt = { time.frame, time.subFrame, MAC_DL_SCH, MAC_IP, "filled packet", len, 0, 0, 0, 64, dnlinkNRB,0};
			memset(newPkt.data, 0xff, 2048);
			*DLSCHPkt = newPkt;
			DLSCHPkt++;
			buf_ctx_tx.chunk_input_entries++;
		}
		
	} else {

		//bladerf_calibrate_dc(params_tx->radioParams.dev, BLADERF_DC_CAL_TX_LPF);
		
		
		offcc++;
		total_off++;

		/*
		newPkt = { time.frame, time.subFrame, MAC_DL_EMPTY, MAC_EMPTY, "empty packet", 0, 0, 0, 0, 65535, 12, 0 };
		*DLSCHPkt = newPkt;
		DLSCHPkt++;
		buf_ctx_tx.chunk_input_entries++;
		*/

		//turn off by switching channel
		if (dnlinkNRB > 0) {
			int maxnrb = 49;
			int len = getTBSFromNRB_special(PDSCH_MODULATION_QPSK, min(dnlinkNRB, maxnrb));
			newPkt = { time.frame, time.subFrame, MAC_DL_SCH, MAC_IP, "filled packet", len, 0, 0, 0, 64, dnlinkNRB, maxnrb - min(dnlinkNRB, maxnrb) };
			memset(newPkt.data, 0xff, 2048);
			*DLSCHPkt = newPkt;
			DLSCHPkt++;
			buf_ctx_tx.chunk_input_entries++;

		}
		else {
			//sending pilots only
			int len = 0;
			newPkt = { time.frame, time.subFrame, MAC_DL_SCH, MAC_IP, "filled packet", len, 0, 0, 0, 64, dnlinkNRB, 0 };
			memset(newPkt.data, 0xff, 2048);
			*DLSCHPkt = newPkt;
			DLSCHPkt++;
			buf_ctx_tx.chunk_input_entries++;
		}
	}
	
	counter++;
	if (counter >= outinterval) {
		counter = 0;
	}

	// Send no packet meaning the end for this frame
	*DLSCHPkt = noPkt;
	DLSCHPkt++;
	buf_ctx_tx.chunk_input_entries++;



}
#endif // LTE_U




// Buffer is empty, send more packets
// Gets called by L1 in every subframe
void __stdcall mac_tx_callback()
{
	// Reset the buffer before adding new packets
	buf_ctx_tx.chunk_input_entries = 0;
	buf_ctx_tx.chunk_input_idx = 0;



	// Overview:
	//
	// mac_tx_schedule_RRC() and mac_tx_schedule_IP() process any incoming packets 
	// from uplink or IP and schedule any required response by assigning 
	// (frame,subframe) to it and storing it in <outQueue>
	//
	// mac_tx_schedule_next_SF() then checks <outQueue> for any packet that should
	// be sent in the next subframe, and passes it to PHY
	// mac_tx_schedule_next_SF() also schedules SIBs and other packets that occur regularly


	// Schedule RRC based on received uplink packets, if any
	mac_tx_schedule_RRC();


	// Schedule IP packets, if any
	mac_tx_schedule_IP();
	
	
	// Update PRACH stats
	mac_tx_PRACH_stat();

	
#ifdef LTE_U
	mac_tx_schedule_next_SF_LTEU();
#else
	// Regular LTE
	// Schedule packets for the 
	mac_tx_schedule_next_SF();
#endif
}







// TX Thread
// Assumption: 
// - Input is generated internally by the MAC code
// - Output can be sent on the radio or in a file (for tests) or in dummy (for perf).
void mac_tx(void * pParam)
{
	ULONGLONG ttstart, ttend;
	bool MACfinished = false;

	// In this MAC TX always first prepares the buffers in memory
	params_tx->inType = TY_MEM;
	buf_ctx_tx.mem_input_buf_size = MEM_BUF_TX_MAX_CNT * sizeof(MACDLPacket);
	buf_ctx_tx.mem_input_buf = (void *)inmem_malloc(buf_ctx_tx.mem_input_buf_size);



#ifdef TEST
	// We use the old AT&T SIB1&SIB2 for tests
	//unsigned char sib1[18] = { 0x68, 0x4c, 0x42, 0x82, 0x19, 0x10, 0xf6, 0x6e, 0x82, 0x11, 0x92, 0x46, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00 };
	//const int sib2_len = 28;
	//unsigned char sib2[sib2_len] = { 0x00, 0x00, 0x00, 0xfe, 0x9f, 0x84, 0x80, 0x00, 0x18, 0x5a, 0x7a, 0x02, 0x00, 0x04, 0x08, 0x00, 0xc2, 0xca, 0xd5, 0x56, 0x0f, 0x6e, 0x86, 0xb9, 0xc8, 0xc1, 0x00, 0x00 };
	initSIBs(17, 739.0, 709.0, 1);
#else
#ifdef US
	// US
	// LIME (Barbados):
	unsigned char sib1_old[18] = { 0x68, 0x4d, 0x0a, 0xc0, 0x19, 0x10, 0xf6, 0x6e, 0x82, 0x11, 0x92, 0x46, 0xa9, 0x80, 0x00, 0x00, 0x00, 0x00 };
	unsigned char sib2_old[27] = { 0x00, 0x00, 0x00, 0xfe, 0xbf, 0x04, 0x80, 0x00, 0x18, 0x5a, 0x7a, 0x02, 0x00, 0x08, 0x00, 0x00, 0x02, 0xcb, 0xf9, 0x56, 0x0f, 0x6e, 0x86, 0xbd, 0xd8, 0xc1, 0x00 };

	// BR: Why is this SIB very different then the ones I am using? We should be using the same parameters,
	// and expose those that need to be changed in initSIB function.
	memcpy(sib1, sib1_old, sib1_len);
	memcpy(sib2, sib2_old, sib2_len);
	//initSIBs(17, 739.0, 709.0, 0);
#else

	// Band 17, TEST
	// unsigned char sib1[sib1_len] = { 0x68, 0x40, 0x04, 0x03, 0x00, 0x01, 0x00, 0x00, 0x10, 0x09, 0x24, 0x6A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 };
	// unsigned char sib1_4[sib1_len] = { 0x68, 0x40, 0x04, 0x03, 0x00, 0x01, 0x00, 0x00, 0x10, 0x09, 0x24, 0x6A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 };
	// Qualcomm Huawei dongle seems to ignore EARFCNUL and only work at 709 MHz
	// 709 MHz
	//unsigned char sib2[sib2_len] = { 0x00, 0x00, 0x00, 0xfe, 0x9f, 0x84, 0x80, 0x00, 0x18, 0x5a, 0x7a, 0x02, 0x00, 0x04, 0x08, 0x00, 0xc2, 0xca, 0xd5, 0x56, 0x0f, 0x6e, 0x86, 0xb9, 0xc8, 0xc1, 0x00 };
	//unsigned char sib2_4[sib2_len] = { 0x00, 0x00, 0x00, 0xfe, 0x9f, 0x84, 0x80, 0x00, 0x18, 0x5a, 0x7a, 0x02, 0x00, 0x04, 0x08, 0x00, 0xc2, 0xca, 0xd5, 0x56, 0x0f, 0x6e, 0x86, 0xb9, 0xc8, 0xc1, 0x00 };
	
	float txFreq = (float)params_tx->radioParams.CentralFrequency;
	float rxFreq = (float)params_rx->radioParams.CentralFrequency;
	
	initSIBs(LTEBand, txFreq, rxFreq, 0);
#endif
#endif



	printf("Starting TX in mac_tx.c ...\n");
	if (params_tx->outType == TY_SDR)
	{
		buf_ctx_tx.mem_output_buf_size = params_tx->outMemorySize;
		buf_ctx_tx.mem_output_buf = inmem_malloc(buf_ctx_tx.mem_output_buf_size);


#ifdef LTE_U
		buf_ctx_tx.ti = (thread_info*)pParam;

		// Init the rest of the structure
		EnterCriticalSection(&lteu_context.critSec);
		lteu_context.ttLastUpdate = 0;
		lteu_context.interval = lteu_context.onCycle + lteu_context.offCycle;
		lteu_context.updated = true;
		LeaveCriticalSection(&lteu_context.critSec);
#endif

	}

	wpl_input_initialize_tx();
	resetBufCtxBlock(&buf_ctx_tx);							// reset context block (all counters)

	// Start with an empty input, and we will use the callback
	buf_ctx_tx.buf_input_callback = mac_tx_callback;
	buf_ctx_tx.chunk_input_entries = 0;


	// MAC-level init
	init_mac_tx();


	// Start measuring time
	ttstart = SoraGetCPUTimestamp(&(params_tx->measurementInfo.tsinfo));

	// TODO: Restructure - while is probably not needed now
	while (!MACfinished)
	{	
		// Run Ziria TX code to preapre the buffer
		wpl_init_heap(pheap_ctx_tx, params_tx->heapSize);	// reset memory management

		// Run Ziria TX code
		wpl_go_tx();

		// Reset output counters (important for file output, when testing)
		wpl_output_reset_tx();

		MACfinished = true;
	}

	wpl_output_finalize_tx();


	// Stop measuring time
	ttend = SoraGetCPUTimestamp(&(params_tx->measurementInfo.tsinfo));
	printf("TX Time Elapsed: %ld us \n",
		SoraTimeElapsed((ttend / 1000 - ttstart / 1000), &(params_tx->measurementInfo.tsinfo)));

	printf("TX Total input items (including EOF): %d (%d B), output items: %d (%d B)\n",
		buf_ctx_tx.total_in, buf_ctx_tx.total_in*buf_ctx_tx.size_in,
		buf_ctx_tx.total_out, buf_ctx_tx.total_out*buf_ctx_tx.size_out);

	if (params_tx->latencySampling > 0)
	{
		printf("TX Min write latency: %ld, max write latency: %ld\n", (ulong)params_tx->measurementInfo.minDiff, (ulong)params_tx->measurementInfo.maxDiff);
		printf("TX CDF: \n   ");
		unsigned int i = 0;
		while (i < params_tx->measurementInfo.aDiffPtr)
		{
			printf("%ld ", params_tx->measurementInfo.aDiff[i]);
			if (i % 10 == 9)
			{
				printf("\n   ");
			}
			i++;
		}
		printf("\n");
	}
}
