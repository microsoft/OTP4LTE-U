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

#include "threads.h"
#include "sora_radio.h"
#include "sora_threads.h"
#include "sora_thread_queues.h"
#include "sora_ip.h"
#include "RegisterRW.h"
#endif

#include "mac.h"
#include "wpl_alloc.h"
#include "buf.h"
#include "utils.h"
#include "debug.h"

#include "Tests\c\unit_tests.h"


// Contex blocks
BufContextBlock buf_ctx_tx, buf_ctx_rx;
HeapContextBlock heap_ctx_tx, heap_ctx_rx;
BufContextBlock *pbuf_ctx_tx = &buf_ctx_tx;
BufContextBlock *pbuf_ctx_rx = &buf_ctx_rx;
HeapContextBlock *pheap_ctx_tx = &heap_ctx_tx;
HeapContextBlock *pheap_ctx_rx = &heap_ctx_rx;

// Global stop switch
// Once this is set to true, all threads exit
int stop_program = false;

// UDP debugging
extern unsigned int udp_pkt_rcvd;
extern unsigned int udp_pkt_err;
extern unsigned int udp_pkt_miss;



// Parameters
BlinkParams paramsArr[2];
BlinkParams *params_tx, *params_rx;

// Used to assemble ethernet frames to be delivered to the OS/NDIS
char ethFrame[2048];



// TODO: This timer should be per code/thread, rather than shared
// Global, used by __ext_print_time from sora_ext_lib.c
// could be set to any valid TimeMeasurements struct. 
TimeMeasurements measurementInfo;


// Global parameters
bool runMACTests = false;
int32 energyDetectionThreshold = 2000000000;		// Triggers energy detection debug procedure
int LTEBand = 0;
int cmd_fifo_queue_size = 2;
int no_tx_bufs = 2;
int dnlinkNRB = -1;									// Number of RBs in dnlink to be used by for TX
int dnlinkMCS = -1;									// MCS to be used for dnlink TX
int noEmptySubF = 0;								// Subframes in LTE-U off cycle (no data, RS, anything)
int emptySubF[10];
int noDataSubF = 0;									// Subframes to be used for IP/data transmissions
int dataSubF[10];
int SIB1SF = 5;
int SIB2SF = 0;

// LTE-U related global parameters
LTEU_context lteu_context;


// Global logs
int32 rxSCH = 0, rxIP = 0, rxCCH = 0, rxRACH = 0, txSCH = 0, txIP = 0, txCCH = 0;
int32 errIP = 0, errFrag = 0, errRLC = 0, errRRC = 0, errQue = 0;
int32 RRCRecComp = 0, RRCAttComp = 0;
int32 txMCS[MAC_NO_MCS];
int32 txNBR[MAC_NO_NBR];


// Setting debug level
int32 debug_level = DEBUG_PRINT_RRC;				// Default - only RRC messages
unsigned char __ext_debugPrint(int32 level) { return (unsigned char) (debug_level & level); }

// Setting test_IP
bool test_IP = false;

// Only needed to support the call from Ziria code
// We currently don't support multiple threads in MAC
int SetUpThreads(PSORA_UTHREAD_PROC* User_Routines)
{
	return 0;
}


//DEBUG
extern volatile LONG debugGlobalCnt;
extern volatile LONG debugGlobalCnt2;


int __cdecl main(int argc, char **argv) 
{
	ULONGLONG ttstart, ttend;
	HANDLE  hThreadTX, hThreadTXW, hThreadRX, hThreadSocket;
	thread_info t_info_rx, t_info_tx, t_info_transf, t_info_transm;
	thread_info shared;


	params_tx = &(paramsArr[0]);
	params_rx = &(paramsArr[1]);

	for (int i = 0; i < argc; ++i) {
		fprintf(stdout, "%d th parameter %s\n", i, argv[i]);
	}
	
	// Initialize the global parameters
	try_parse_args(paramsArr, argc, argv);


	// Run MAC unit tests and exit
	/*
	//if (runMACTests)
	{
		all_unit_tests();
		return 0;
	}
	*/


	printf("Setting up threads...\n");

	// **** TX/RX(2)-threaded MAC

	// Initialize various parameters
	init_mac(params_tx, params_rx);


	// Print various parameters
	printf("energyDetectionThreshold=%ld, debug_level=%ld, test_IP=%ld, LTEBand=%ld, ", energyDetectionThreshold, debug_level, test_IP, LTEBand);
	if (dnlinkMCS >= 0)
	{
		printf("MCS=%d\n", dnlinkMCS);
	}
	else if (dnlinkNRB)
	{
		printf("NRB=%d\n", dnlinkNRB);
	}
	else
	{
		printf("Using hardcoded rate\n");
	}
	memset(txMCS, 0, sizeof(int32)*MAC_NO_MCS);
	memset(txNBR, 0, sizeof(int32)*MAC_NO_NBR);
	printf("Empty subframes: ");
	for (int i = 0; i < noEmptySubF; i++) printf("%d ", emptySubF[i]);
	printf("\n");
	printf("Data subframes: ");
	for (int i = 0; i < noDataSubF; i++) printf("%d ", dataSubF[i]);
	printf("\n");
	printf("SIB1 subframe : %d, SIB2 subframe : %d\n", SIB1SF, SIB2SF);
	


	// Init timer to be used
	measurementInfo = params_tx->measurementInfo;


#if defined SORA_RF && !defined SW_TEST
	writeSoraCtx *ctx = (writeSoraCtx *)params_tx->TXBuffer;
	readSoraCtx *rctx = (readSoraCtx *)params_rx->TXBuffer;
	hThreadTXW = StartWinUThread(SoraTXWorker, (PVOID)(ctx), 3, 1);
	if (!hThreadTXW)
	{
		printf("Worker TX threads allocation failed!\n");
		exit(1);
	} else {
		printf("Started worker TX thread.\n");
	}
#endif
	
	// Start TX threads
	t_info_tx.threadID = 0;
	t_info_tx.fRunning = true;
	
#ifdef LTE_U
	InitializeCriticalSection(&lteu_context.critSec);
#endif

	hThreadTX = StartWinUThread(go_thread_tx, (PVOID)(&t_info_tx), 2, 1);
	if (!hThreadTX)
	{
		printf("TX threads allocation failed!\n");
		exit(1);
	} else {
		printf("Started TX thread.\n");
	}
	
	
	t_info_rx.threadID = 3;
	t_info_rx.fRunning = true;
	hThreadRX = StartWinUThread(go_thread_rx, (void *)(&t_info_rx), 1, 1);
	if (!hThreadRX)
	{
		printf("RX thread allocation failed!\n");
		exit(1);
	} else {
		printf("Started RX thread.\n");
	}
	
#ifdef LTE_U
	shared.threadID = 0;
	shared.fRunning = true;
	
	#ifdef SORA_RF
		// In case of Sora we have no free core, so we reuse core 0 with the rest of OS (not time critical)
		hThreadSocket = StartWinUThread(go_thread_udpsocket, (PVOID)(&shared), 0, 0);
    #else
		hThreadSocket = StartWinUThread(go_thread_udpsocket, (PVOID)(&shared), 3, 1);
	#endif

	if (!hThreadSocket)
	{
		printf("Socket threads allocation failed!\n");
		exit(1);
	}
	else {
		printf("Started UDP thread.\n");
	}
#endif

	fflush(stdout);

	// Start measuring time
	ttstart = SoraGetCPUTimestamp(&(params_tx->measurementInfo.tsinfo));

	ULONGLONG intCnt = 0, firstInterrupt = 0;


#ifdef SW_TEST
	while (t_info_rx.fRunning || t_info_tx.fRunning)
	{
		// Wait for the threads
		Sleep(100);
	}
#endif

//#ifdef BLADE_RF
#if defined BLADE_RF && !defined SW_TEST
	while (t_info_rx.fRunning || t_info_tx.fRunning)
	{
		if (test_IP)
		{
			printf("UDP Stats: Rcvd=%ld, Err=%ld, Miss=%ld\n", udp_pkt_rcvd, udp_pkt_err, udp_pkt_miss);
			udp_pkt_rcvd = 0;
			udp_pkt_err = 0;
			udp_pkt_miss = 0;
		}

		// Wait for the threads
		Sleep(5000);
	}
#endif

//#ifdef SORA_RF
#if defined SORA_RF && !defined SW_TEST
	printf("Waiting for sync...\n");
	fflush(stdout);
	while (t_info_rx.fRunning && t_info_tx.fRunning && ctx->TXRunning)
	{
		// Don't wait for Sync for too long as there might be a problem with the 
		// firmware that we can only see with printRadioStats coming up later
		for (int tt = 0; tt < 5 && !rctx->RXSynced; tt++)
		{
			Sleep(1000);
		}

#ifndef LTE_U

		if (firstInterrupt > 0)
		{
			printf(" ******** %llu iterations without interruption\n", firstInterrupt);
		}

		if (!rctx->RXSynced)
		{
			printf("\n\n ******** TX and RX not in sync (syncStateCnt=%lu, rxBlocked=%llu)!\n\n", rctx->syncStateCnt, rctx->rxBlocked);
			break;
		}
#endif

		bool outOfSync = printRadioStats(ctx, rctx);

		if (test_IP)
		{
			printf("UDP Stats: Rcvd=%ld, Err=%ld, Miss=%ld\n", udp_pkt_rcvd, udp_pkt_err, udp_pkt_miss);
			udp_pkt_rcvd = 0;
			udp_pkt_err = 0;
			udp_pkt_miss = 0;
		}

		if (firstInterrupt == 0 && outOfSync)
		{
			firstInterrupt = intCnt;
		}

		intCnt++;

		// Wait for the threads
		Sleep(5000);
	}
#endif

	stop_program = true;


	// Stop measuring time
	ttend = SoraGetCPUTimestamp(&(params_tx->measurementInfo.tsinfo));
	printf("Time Elapsed: %ld us \n",
		SoraTimeElapsed((ttend / 1000 - ttstart / 1000), &(params_tx->measurementInfo.tsinfo)));


	// Stop any remaining thread
	CloseHandle(hThreadTX);
	CloseHandle(hThreadRX);

#ifdef LTE_U
	CloseHandle(hThreadSocket);
	DeleteCriticalSection(&lteu_context.critSec);
#endif

//#ifdef SORA_RF
#if defined SORA_RF && !defined SW_TEST
	CloseHandle(hThreadTXW);
#endif

	stop_mac();


	return 0;
}




