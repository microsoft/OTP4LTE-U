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

#ifdef SORA_RF
#include <soratime.h>
#include <thread_func.h>
#include <stdlib.h>
#include <time.h>
#include <rxstream.h>
#include "params.h"
#include "numerics.h"
#include "RegisterRW.h"
#include "mac.h"
#include "sora_ip.h"



// Read samples are queues at the driver level, but there seem to be no back-pressure towards the firmware
// So if there is a large latency between two reads, the samples get lost on read, but they are fine all the way thourgh firmware
// In order to allow for more queuing at read, we actually read more often and queue locally 
// We can schedule the read anywhere in the code, explicitly, using function readSoraAndQueue
// The overflow seems to happen when latency is above 600us between consecutive reads
// Uncomment below to allow this behaviour
// NOTE: This does not seem to solve the problem at the moment
// #define READ_IN_WORKER_THREAD


// If enabled it first allows TX and RX to sync and then sends repeated 0...999 sequence through from TX to RX directly
// overwriting user TX inputs, but running the real TX/RX code with the real timing
// If there are no errors, output cnt_Sora_FRL_2nd_RX_data should be 0
// #define DEBUG_LOSSES

// If defined disables sync and forwards the TX straight to the RX (i.e. SetTXRXSync(2))
// #define DEBUG_TXRX	

// If defined uses sync but after that forwards the TX straight to the RX (i.e. SetTXRXSync(2))
// #define DEBUG_TXRX_THROUGH

#ifdef DEBUG_LOSSES
  #define DEBUG_TXRX_THROUGH
#endif


// Statistics about number of TX/RX syncs missed
ULONG outOfSyncs = 0;


// Samples of queue sizes (for debugging)
// Samples of queue sizes (for debugging)
ULONGLONG samp_PC_queue[MAX_CMD_FIFO_QUEUE_SIZE+1];
ULONGLONG samp_FPGA_queue[MAX_NO_TX_BUFS+1];
ULONGLONG samp_RX_queue_full, samp_RX_queue_free;

// Rounds in which underflow occurs
#define DEB_MAX_UNDERFLOW_QUEUE	20
int cnt_samp_FPGA_underflow = 0;
ULONGLONG samp_FPGA_underflow[DEB_MAX_UNDERFLOW_QUEUE];


// Global init done variable
// This is set to true once all inits are done
// After that RX and TX start syncing
// Otherwise, if sync happens first, a long init can break it.
volatile int rx_init_done = false;
volatile int tx_init_done = false;


// Setting test_IP
extern bool test_IP;



void init_sora_hw()
{
	// Start Sora HW

	// Initialize Sora user mode extension
	BOOLEAN succ = SoraUInitUserExtension("\\\\.\\HWTest");
	if (!succ)
	{
		printf("Error: fail to find a Sora UMX capable device!\n");
		exit(1);
	}

	// Override radio ids (as they are hardcoded in firmware)
	params_tx->radioParams.radioId = TARGET_RADIO_TX;
	params_rx->radioParams.radioId = TARGET_RADIO_RX;

	// IMPORTANT!!!!
	// NOTE: Make sure you start TX before RX
	// Otherwise, the current firmware will cause BSOD!
	// (this is a known bug that should be fixed one day...)
	if (params_tx->outType == TY_SDR)
	{
		RadioStart(params_tx);
		InitSoraTx(params_tx);
	}

	if (params_rx->inType == TY_SDR)
	{
		RadioStart(params_rx);
		InitSoraRx(params_rx);
	}

	// Firmware is one for all radios
	RadioSetFirmwareParameters();

	// Start NDIS if not in testIP mode
	if (params_tx->inType == TY_IP && !test_IP)
	{
		HRESULT hResult = SoraUEnableGetTxPacket();
		if (hResult != S_OK)
		{
			printf("*** NDIS binding failed!\n");
		}

		// TODO: Hardcoded Sora MAC address - load from OS/command line
		memset(ethFrame, 0, 14 * sizeof(char));
		ethFrame[0] = 0x02;
		ethFrame[1] = 0x50;
		ethFrame[2] = 0xF2;
		ethFrame[3] = 0x78;
		ethFrame[4] = 0xFD;
		ethFrame[5] = 0x6C;

		ethFrame[12] = 0x08;

		//assert(hResult == S_OK);
		//Ndis_init(NULL);
	}

	/* No init to be done here, just use SoraUIndicateRxPacket
	if (params_rx->outType == TY_IP)
	{
	// To be implemented
	HRESULT hResult = SoraUEnableGetRxPacket();
	assert(hResult == S_OK);
	}
	*/


	// Init radio context blocks
	readSoraCtx *rctx = (readSoraCtx *)params_rx->TXBuffer;
	writeSoraCtx *ctx = (writeSoraCtx *)params_tx->TXBuffer;


	// Start Sora TX worker that is doing PCI transfer to Sora and the actual TX
	initTXCtx(ctx, rctx);

	// Start RX thread
	initRXCtx(rctx, ctx->TXBufferSize);

}



void stop_sora_hw()
{
	// Start Sora HW
	if (params_rx->inType == TY_SDR || params_tx->outType == TY_SDR)
	{
		RadioStop(params_tx);
	}
	// Start NDIS
	if (params_tx->inType == TY_IP)
	{
		if (hUplinkThread != NULL)
		{
			// Sora cleanup.
			SoraUThreadStop(hUplinkThread);
			SoraUThreadFree(hUplinkThread);
		}
		SoraUDisableGetTxPacket();
		// Winsock cleanup.
		closesocket(ConnectSocket);
		WSACleanup();

	}

	if (params_rx->outType == TY_IP)
	{
		// To be implemented
		/*
		if (hUplinkThread != NULL)
		{
		// Sora cleanup.
		SoraUThreadStop(hUplinkThread);
		SoraUThreadFree(hUplinkThread);
		}
		SoraUDisableGetTxPacket();
		// Winsock cleanup.
		closesocket(ConnectSocket);
		WSACleanup();
		*/
	}

	SoraUCleanUserExtension();
}





#ifdef READ_IN_WORKER_THREAD
FINL
void readSoraAndQueue()
{
	HRESULT hr = S_OK;
	readSoraCtx *rctx = (readSoraCtx *)params_rx->TXBuffer;
	FLAG fReachEnd;
	// Signal block contains 7 vcs = 28 complex16
	static SignalBlock block;
	complex16* pSamples = (complex16*)(&block[0]);

	if (rctx->RXSynced)
	{
		// Stats
		if (s_ts_isFull(rctx->readQueue, 0))
		{
			samp_RX_queue_full++;
		}
		else
		{
			// Read from the radio and store in worker/RX queue
			// We read 28 samples @30.72 MHz, so each read takes at most ~1us (not sure if they are all blocking).
			// So this read should not overall block the performance of the TX part
			while (!s_ts_isFull(rctx->readQueue, 0))
			{
				// This queue should not be full. For debugging, if queue full just skip the read, data will be also queued at the driver 
				hr = FastSoraRadioReadRxStream(&(params_rx->radioParams.dev->RxStream), &fReachEnd, block);
				if (!FAILED(hr)) {
					// This can fail initially because read is in sync with the write
					// so some initial reads my timeout. But this will later get in sync
					// So instead of quitting here we monitor that the number of missed reads goes to 0.
					s_ts_put(rctx->readQueue, 0, (char *)pSamples);
					samp_RX_queue_free++;
				}
				else
				{
					break;
				}
			}
		}
	}
}
#endif



void initTXCtx(writeSoraCtx *ctx, readSoraCtx *rctx)
{
	ctx->prepareBlocked = 0; 
	ctx->transferBlocked = 0;
	ctx->TXRunning = true;
	ctx->rctx = rctx;
}


void initRXCtx(readSoraCtx *rctx, ULONG buf_size)
{
	rctx->rxBlocked = 0;
	rctx->rxSucc = 0;
	rctx->RXSynced = false;
	rctx->TXBufferSize = buf_size;
	rctx->syncStateCnt = buf_size - 28 - 1;
	rctx->RXSymbol = 0;
	rctx->RXFrame = 0;
}



// Uncomment to sample TX/RX samples to 30.72 MHz LTE frequency
#ifndef DEBUG_LOSSES
  #define USE_SAMPLING
#endif 

// Setting experimental parameters for the modified firmware
// Does not affect the original firmware
void RadioSetFirmwareParameters()
{
	// *** RX
#ifdef USE_SAMPLING
	SetRXDownsampleParameters(1);
#else
	SetRXDownsampleParameters(0);
#endif

	// *** TX
	SetContinuousTXParameters(cmd_fifo_queue_size);

#ifdef USE_SAMPLING
	SetTXUpsampleParameters(1);
#else
	SetTXUpsampleParameters(0);
#endif

	// TXRXSync
	// Note: Use of TXRXSync will prevent TX unless RX is working at the same time!
	// 0 - no transfer from TX to RX
	// 1 - 1bit transfer from TX to RX
	// 2 - all bits transfer from TX to RX
	//SetTXRXSync(2);

	// We set to 2 first to send a special sync sequence
	SetTXRXSync(2);
}



void RadioStart(BlinkParams *params) {
	// always start radio first, it will reset radio to the default setting
    SoraURadioStart(params->radioParams.radioId);

	if (params->radioParams.radioId == TARGET_RADIO_RX)
	{
		SoraURadioSetRxPA(params->radioParams.radioId, params->radioParams.RXpa);
		SoraURadioSetRxGain(params->radioParams.radioId, params->radioParams.RXgain);
	}

	if (params->radioParams.radioId == TARGET_RADIO_TX)
	{
		printf("RADIO=%lu, GAIN=%lu\n", params->radioParams.radioId, params->radioParams.TXgain);
		SoraURadioSetTxGain(params->radioParams.radioId, params->radioParams.TXgain);
	}

	// DEBUG
	SoraURadioSetCentralFreq(params->radioParams.radioId, params->radioParams.CentralFrequency);
	//SoraUWriteRadioRegister(TARGET_RADIO_TX, 0x07, params->radioParams.CentralFrequency);

	// This is not supported with TVWS MIMO:
	//SoraURadioSetFreqOffset(params->radioParams.radioId, params->radioParams.FreqencyOffset);				
	// Instead we write this (where FreqencyOffset is in 1/4096 MHz):
	SoraUWriteRadioRegister(params->radioParams.radioId, 0x09, params->radioParams.FreqencyOffset);

	/*
	if (params->radioParams.radioId == TARGET_RADIO_TX)
	{
		//SoraUWriteRadioRegister(TARGET_RADIO_TX, 0x07, 500);
		//SoraUWriteRadioRegister(TARGET_RADIO_TX, 0x09, params->radioParams.FreqencyOffset);
		//SoraUWriteRadioRegister(TARGET_RADIO_TX, 0x09, 600);
		SoraUWriteRadioRegister(TARGET_RADIO_TX, 0x09, params->radioParams.FreqencyOffset);
	}
	*/


    SoraURadioSetSampleRate(params->radioParams.radioId, params->radioParams.SampleRate);
	params->TXBuffer = NULL;
	params->pRxBuf = NULL;

	// DEBUG
	printf("RadioID: %ld, RXpa: %ld, RXgain: %ld, TXgain: %ld, CentralFrequency: %ld, FreqencyOffset: %ld, SampleRate: %ld\n", 
		params->radioParams.radioId, params->radioParams.RXpa, params->radioParams.RXgain,
		params->radioParams.TXgain, params->radioParams.CentralFrequency, params->radioParams.FreqencyOffset,
		params->radioParams.SampleRate);

	hDeviceHandle = GetDeviceHandle(DEVNAME);
	if (hDeviceHandle == INVALID_HANDLE_VALUE)
	{
		printf("Can't find device %s\n", DEVNAME);
		exit(1);
	}
}



void RadioStop(BlinkParams *params) {
	writeSoraCtx *ctx = (writeSoraCtx *)params->TXBuffer;

	for (int i = 0; i < no_tx_bufs; i++)
	{
		if (ctx->TXBuffers[i] != NULL)
		{
			SoraUReleaseBuffer((PVOID)params->TXBuffer);
		}
	}
	free(ctx);

	if (params->pRxBuf != NULL)
	{
		HRESULT hr;
		SoraURadioReleaseRxStream(&params->radioParams.dev->RxStream, params->radioParams.radioId);
        hr = SoraURadioUnmapRxSampleBuf(params->radioParams.radioId, params->pRxBuf);
	}
}



void InitSoraRx(BlinkParams *params)
{
    HRESULT hr;
    ULONG nRxBufSize = 0;

	readSoraCtx *rctx = (readSoraCtx *)inmem_malloc(sizeof(readSoraCtx));
	params->TXBuffer = (PVOID)rctx;

	// Create queue between worker and RX
	// (only used if READ_IN_WORKER_THREAD defined)
	size_t size = 28*sizeof(complex16);
	rctx->readQueue = s_ts_init(1, &size);

	// Map Rx Buffer 
    hr = SoraURadioMapRxSampleBuf( params->radioParams.radioId, &params->pRxBuf, & nRxBufSize);
    if ( FAILED (hr) ) {
        fprintf (stderr, "Error: Fail to map Sora Rx buffer!\n" );
        exit(1);
    }
    
    // Generate a sample stream from mapped Rx buffer
	params->radioParams.dev = (SoraRadioParam *)inmem_malloc(sizeof(SoraRadioParam));
	SoraURadioAllocRxStream(&(params->radioParams.dev->RxStream), params->radioParams.radioId, (PUCHAR)params->pRxBuf, nRxBufSize);
}


void InitSoraTx(BlinkParams *params)
{
	writeSoraCtx *ctx = (writeSoraCtx *)inmem_malloc(sizeof(writeSoraCtx));
	params->TXBuffer = (PVOID)ctx;

	ctx->prepareBuf = 0;
	ctx->transferBuf = 0;

	ctx->firstTx = 0;
	ctx->lastTx = 0;

	ctx->idleTXDetected = 0;
	ctx->TXBufferSize = params->radioParams.TXBufferSize;

	printf("TX-sora-tx-buffer-size=%ld, cmd-fifo-queue-size=%d, no-tx-bufs=%d\n", 
		ctx->TXBufferSize, cmd_fifo_queue_size, no_tx_bufs);
	for (int i = 0; i < no_tx_bufs; i++)
	{
		ctx->TXBuffers[i] = SoraUAllocBuffer(ctx->TXBufferSize * sizeof(complex16));		// alloc tx sample buffers
		if (ctx->TXBuffers[i] == NULL)
		{
			fprintf(stderr, "Error: Fail to allocate Sora Tx buffer memory!\n");
			exit(1);
		}

		// Set to 0 as it is not expensive in init and can be useful for gaps between packets
		memset(ctx->TXBuffers[i], 0, params->radioParams.TXBufferSize * sizeof(complex16));
	}

}



// readSora reads <size> of complex16 inputs from Sora radio
// It is a blocking function and returns only once everything is read
void readSora(BlinkParams *params, complex16 *ptr, int size)
{
    HRESULT hr;
    FLAG fReachEnd;
	// Signal block contains 7 vcs = 28 complex16
    static SignalBlock block;
	complex16* pSamples = (complex16*)(&block[0]);
    static int indexSrc = 28;
	int oldIndexSrc;
	complex16* ptrDst = ptr;
	int remaining = size;
	readSoraCtx *rctx = (readSoraCtx *)params->TXBuffer;

	// DEBUG
	int32 deb_cnt = 0;
	int16 deb_max = 0;
	int16 deb_old = 0;

	// Wait until both TX and RX are done with init before starting sync
	if (!rx_init_done)
	{
		printf("RX C Init done...\n");
		fflush(stdout);
	}
	rx_init_done = true;
	while (!tx_init_done);


#ifndef DEBUG_TXRX	
	// Wait for the special sync sequence from the TX 
	// before starting to receive data. See TX for description.
	while (!rctx->RXSynced)
	{
		// Read from the radio
		hr = SoraRadioReadRxStream(&(params->radioParams.dev->RxStream), &fReachEnd, block);
		if (FAILED(hr)) {
			// This can happen initially because read is in sync with the write
			// so some initial reads my timeout. But this will later get in sync
			// So instead of quitting here we monitor that the number of missed reads goes to 0.
			//fprintf (stderr, "Error: Fail to read Sora Rx buffer!\n" );
			//exit(1);
			rctx->rxBlocked++;
			continue;
		}

		int i = 0;
		while (i < 28 && rctx->syncStateCnt < rctx->TXBufferSize - 1)
		{
			// DEBUG
			/*
			deb_max = max(deb_max, pSamples[i].im);
			deb_cnt++;
			if (deb_cnt == 100000000) {
				printf("DEB: %d\n", deb_max);
			}
			if (pSamples[i].im > 0 && 
				(pSamples[i].im - deb_old == 1 || pSamples[i].im > 10000)) printf("%d ", pSamples[i].im);
			deb_old = pSamples[i].im;
			*/



			if (rctx->syncStateCnt + 1 == pSamples[i].im)
			{
				rctx->syncStateCnt = pSamples[i].im;
			}
			else
			{
				static bool printErr = true;
				if (rctx->syncStateCnt != rctx->TXBufferSize - 28 - 1 && printErr)
				{
					printf("rctx->syncStateCnt=%ld, pSamples[%d].im=%d\n", rctx->syncStateCnt, i, pSamples[i].im);
					printErr = false;
				}
			}
			i++;
		}

		if (rctx->syncStateCnt >= rctx->TXBufferSize - 1)
		{
			rctx->RXSynced = true;
			rctx->RXSymbol = 28 - i;
			rctx->RXFrame = 0;
			printf("\n**************************************************************************\n");
			printf("RX synchronized (syncStateCnt=%ld, i=%d, pSamples[i].im=%d, rxBlocked=%llu, rctx->rxSucc=%llu).\n", rctx->syncStateCnt, i, pSamples[i].im, rctx->rxBlocked, rctx->rxSucc);
			printf("**************************************************************************\n\n");

#ifndef DEBUG_TXRX_THROUGH
			// Switch to sending 1-bit sync info
			SetTXRXSync(1);
#endif

			indexSrc = i;
		}
	}
#endif


	if (indexSrc < 28) {
		oldIndexSrc = indexSrc;
		// Something has already been read previously, so copy that first
		memcpy((void *)ptrDst, (void *)(pSamples + indexSrc), min(remaining, 28 - oldIndexSrc)*sizeof(complex16));
		indexSrc += min(remaining, 28 - oldIndexSrc);
		ptrDst += min(remaining, 28 - oldIndexSrc);
		remaining -= min(remaining, 28 - oldIndexSrc);
	}

	while (remaining > 0)
	{
#ifndef READ_IN_WORKER_THREAD
		// Read from the radio
		hr = SoraRadioReadRxStream(&(params->radioParams.dev->RxStream), &fReachEnd, block);
		if (FAILED(hr)) {
			// This can happen initially because read is in sync with the write
			// so some initial reads my timeout. But this will later get in sync
			// So instead of quitting here we monitor that the number of missed reads goes to 0.
			//fprintf (stderr, "Error: Fail to read Sora Rx buffer!\n" );
			//exit(1);
			rctx->rxBlocked++;
			continue;
		}
#else
		readSoraAndQueue();

		if (!s_ts_get(rctx->readQueue, 0, (char *)pSamples))
		{
			rctx->rxBlocked++;
			continue;
		}
#endif
		rctx->rxSucc++;
		indexSrc = 0;

		// Check sync
		if (rctx->RXSymbol + 28 > RXTX_SYNC_PERIOD)
		{
			// DEBUG
			/*
			if (rctx->RXFrame % 50 == 0)
			{
				printf("%d\n", pSamples[RXTX_SYNC_PERIOD - rctx->RXSymbol].im);
			}
			*/

			if ((pSamples[RXTX_SYNC_PERIOD - rctx->RXSymbol].re & 1) != 1 ||
				(pSamples[RXTX_SYNC_PERIOD - rctx->RXSymbol].im & 1) != 1)
			{
				if (outOfSyncs == 0)
				{
					printf("First outOfSyncs occured after %llu (%llu, %llu) reads.\n", 
						rctx->rxBlocked + rctx->rxSucc, rctx->rxBlocked, rctx->rxSucc);
					/*
					printf("**** Dumping extra stats:\n");
					writeSoraCtx *_ctx = (writeSoraCtx *)params_tx->TXBuffer;
					readSoraCtx *_rctx = (readSoraCtx *)params_rx->TXBuffer;
					bool outOfSync = printRadioStats(_ctx, _rctx);
					*/
					fflush(stdout);
				}
				outOfSyncs++;
			}

			rctx->RXSymbol = rctx->RXSymbol + 28 - RXTX_SYNC_PERIOD;
			rctx->RXFrame = (rctx->RXFrame + 1) % 1024;

		}
		else
		{
			rctx->RXSymbol += 28;
		}


		// Copy
		memcpy((void *)ptrDst, (void *)(pSamples + indexSrc), min(remaining, 28)*sizeof(complex16));
		indexSrc += min(remaining, 28);
		ptrDst += min(remaining, 28);
		remaining -= min(remaining, 28);

	}

#ifdef DEBUG_LOSSES
	// In DEBUG_LOSSES mode ignore input since it is copied from TX, can cause false RX, and mess up timing
	memset(ptr, 0, size*sizeof(complex16));
#endif

}



// Transit a buffer of <size> complex16 numbers
void writeSora(BlinkParams *params, complex16 *ptr, ULONG size)
{
    HRESULT hr;
	static int indexSrc = 0;
	int indexPtr = 0;
	writeSoraCtx *ctx = (writeSoraCtx *)params->TXBuffer;
	complex16 *remainBuf = ptr;
	ULONG remainCnt = size;
	int nextPrepareBuf;
	static complex16 *syncBuf1 = NULL, *syncBuf2 = NULL;
	static bool sendSendSyncPreamb = true;

	static LONG TXCnt = 0;


#ifdef DEBUG_LOSSES
	static complex16 incArr[2000];
	static int incAddInd = 0;
#endif


#ifndef DEBUG_TXRX	
	// Init sync buffer
	if (syncBuf1 == NULL)
	{
		syncBuf1 = (complex16*)inmem_malloc(ctx->TXBufferSize * sizeof(complex16));
		syncBuf2 = (complex16*)inmem_malloc(ctx->TXBufferSize * sizeof(complex16));
		if (syncBuf1 == NULL || syncBuf2 == NULL)
		{
			printf("Cannot malloc syncBufs!\n");
			exit(1);
		}
		for (int i = 0; i < ctx->TXBufferSize; i++)
		{
			syncBuf1[i].re = i;
			syncBuf1[i].im = i;
			syncBuf2[i].re = 0;
			syncBuf2[i].im = 0;
		}
	}
#endif

	// Wait until both TX and RX are done with init before starting sync
	if (!tx_init_done)
	{
		printf("TX C Init done...\n");
		fflush(stdout);

#ifdef DEBUG_LOSSES
		// Create test sequence
		for (int i = 0; i < 2000; i++)
		{
			incArr[i].re = (i % 1000);
			incArr[i].im = (i % 1000);
		}
#endif

	}
	tx_init_done = true;
	while (!rx_init_done);


#ifdef DEBUG_LOSSES
	// Send test sequence instead of the real one
	ptr = incArr + incAddInd;
	incAddInd = (incAddInd + size) % 1000;
#endif

#ifndef DEBUG_TXRX	
	// 2s delay before syncing
	const int syncPreambLen = ((2*30720000) / (int) ctx->TXBufferSize);

	// Send special sequence
	// Sequence starts with bunch of 0s to wait until the firmware's queues stabilize
	// and then sends a ramp that is detected by the RX. Data transmission continues 
	// immediately after the ramp so after it we are in sync with RX.
	if (sendSendSyncPreamb)
	{
		nextPrepareBuf = (ctx->prepareBuf + 1) % no_tx_bufs;

		for (int syncBufCnt = 0; syncBufCnt <= syncPreambLen; syncBufCnt++)
		{
			complex16 *currentBuf = (complex16*)ctx->TXBuffers[ctx->prepareBuf];

			if (syncBufCnt == syncPreambLen)
			{
				memcpy((void *)currentBuf, (void *)syncBuf1, ctx->TXBufferSize * sizeof(complex16));
			}
			else
			{
				memcpy((void *)currentBuf, (void *)syncBuf2, ctx->TXBufferSize * sizeof(complex16));
			}

			// Spin wait until there is a space in the buffer queue
			while (nextPrepareBuf == ctx->transferBuf)
			{
				ctx->prepareBlocked++;
			}

			// Full buffer ready to be sent
			// We don't really send here. 
			// We just advance the pointer and release the record
			// to be transferred and sent by other threads
			ctx->prepareBuf = nextPrepareBuf;
			nextPrepareBuf = (ctx->prepareBuf + 1) % no_tx_bufs;
			indexSrc = 0;
		}
		sendSendSyncPreamb = false;
	}


  #ifndef DEBUG_LOSSES
	// Set the bit on LSB RXTX Sync channel
	// For speed we set the bit only once every RXTX_SYNC_PERIOD (10ms - 1 frame)
	// and we don't reset the bit otherwise. We count on this bit being random so
	// we'll detect out of sync in on average 20ms
	if (TXCnt == 0)
	{
		ptr[0].re |= 1;
		ptr[0].im |= 1;
	}
	else if (TXCnt + size > RXTX_SYNC_PERIOD)
	{
		ptr[RXTX_SYNC_PERIOD - TXCnt - 1].re |= 1;
		ptr[RXTX_SYNC_PERIOD - TXCnt - 1].im |= 1;
	}
	TXCnt = (TXCnt + size) % RXTX_SYNC_PERIOD;
  #endif
#endif




	nextPrepareBuf = (ctx->prepareBuf + 1) % no_tx_bufs;

	while (remainCnt > 0)
	{
		ULONG inc = min(ctx->TXBufferSize - indexSrc, remainCnt);
		complex16 *currentBuf = (complex16*)ctx->TXBuffers[ctx->prepareBuf];
		memcpy((void *)(currentBuf + indexSrc), (void *)(ptr + indexPtr), inc * sizeof(complex16));
		indexPtr += inc;
		indexSrc += inc;
		remainCnt -= inc;

		if (indexSrc >= ctx->TXBufferSize)
		{
			bool block = false;
			// Spin wait until there is a space in the buffer queue
			while (nextPrepareBuf == ctx->transferBuf)
			{
				block = true;
			}
			if (block) ctx->prepareBlocked++;

			// Full buffer ready to be sent
			// We don't really send here. 
			// We just advance the pointer and release the record
			// to be transferred and sent by other threads
			ctx->prepareBuf = nextPrepareBuf;
			nextPrepareBuf = (ctx->prepareBuf + 1) % no_tx_bufs;
			indexSrc = 0;
		}
	}
}


// Returns a - b, assuming that a > b and ignoring wrapping
ULONG diff_wrap(ULONG a, ULONG b)
{
	if (b > a)
	{
		return (ULONG_MAX - b) + a + 1;

	}
	else
	{
		return (a - b);
	}
}



// Returns a - b, assuming that a > b and ignoring wrapping
ULONG diff_wrap_max(ULONG a, ULONG b, ULONG max)
{
	if (b > a)
	{
		return (max - b) + a;

	}
	else
	{
		return (a - b);
	}
}


// Buffers are ready, transfer them
DWORD WINAPI SoraTXWorker(void * pParam)
{
	HRESULT hr = S_OK;
	bool transferred = false;
	bool idle_detected = FALSE;
	writeSoraCtx *ctx = (writeSoraCtx *)pParam;

	ULONG firstTx32, lastTx32;
	ULONG cnt_tx = 0, cnt_rx = 0;

	ULONG samp_PC_queue_ptr = 0;
	ULONG samp_FPGA_queue_ptr = 0;
	ULONG samp_queue_subsample = 0;

	memset(samp_PC_queue, 0, sizeof(ULONGLONG)*MAX_NO_TX_BUFS);
	memset(samp_FPGA_queue, 0, sizeof(ULONGLONG)*MAX_CMD_FIFO_QUEUE_SIZE);
	samp_RX_queue_full = 0;
	samp_RX_queue_free = 0;

	bool blockPC = false;
	bool blockFPGA = false;

	ctx->idleTXDetected = 0;

	ULONGLONG cnt_rnd = 0;
	cnt_samp_FPGA_underflow = 0;
	memset(samp_FPGA_underflow, 0, sizeof(ULONGLONG)* DEB_MAX_UNDERFLOW_QUEUE);


	while (!rx_init_done || !tx_init_done);

	// Synchronize initial queue positions
	hr = ReadRegister(0x0550, &lastTx32);
	hr = ReadRegister(0x0554, &firstTx32);
	ctx->lastTx = lastTx32;
	ctx->firstTx = firstTx32;

	// TODO: here we might want to cache-align all the data to avoid cache misses
	while (true)
	{
		bool bPC = ctx->transferBuf != ctx->prepareBuf;
		bool bFPGA = diff_wrap(ctx->lastTx, ctx->firstTx) < cmd_fifo_queue_size;
		blockPC = blockPC || (!bPC);
		blockFPGA = blockFPGA || (!bFPGA);
		if (bPC && bFPGA)
		{
			int queuePos = ctx->lastTx % cmd_fifo_queue_size;
			hr = SoraURadioTransferEx(TARGET_RADIO_TX, ctx->TXBuffers[ctx->transferBuf],
				ctx->TXBufferSize * sizeof(complex16), &(ctx->BufID[queuePos]));
			hr = SoraURadioTx(TARGET_RADIO_TX, ctx->BufID[queuePos]);
			ctx->lastTx++;

			if (!SUCCEEDED(hr))
			{
				printf("SoraURadioTransferEx %d failed!\n", ctx->transferBuf);
				return FALSE;
			}
			ctx->transferBuf = (ctx->transferBuf + 1) % no_tx_bufs;

			// Store queue size samples for debugging
			ULONG d1 = diff_wrap_max((ULONG)ctx->prepareBuf, (ULONG)ctx->transferBuf, no_tx_bufs);
			d1 = min(d1, MAX_NO_TX_BUFS);
			ULONG d2 = diff_wrap_max(ctx->lastTx, ctx->firstTx, cmd_fifo_queue_size);
			d2 = min(d2, MAX_CMD_FIFO_QUEUE_SIZE);
			samp_PC_queue[d1] ++;
			samp_FPGA_queue[d2] ++;

			if (d2 < 6) {
				samp_FPGA_underflow[cnt_samp_FPGA_underflow] = cnt_rnd + 1;
				cnt_samp_FPGA_underflow = min(cnt_samp_FPGA_underflow + 1, DEB_MAX_UNDERFLOW_QUEUE);
			}

			if (blockPC) ctx->transferBlocked++;
			if (blockFPGA) ctx->transferBlockedFPGA++;
			blockPC = false;
			blockFPGA = false;
		}


		hr = ReadRegister(0x0550, &lastTx32);
		hr = ReadRegister(0x0554, &firstTx32);

		// Don't free the one just dequeued as the transmission could still be ongoing
		// Only free once the subsequent has been dequeued
		// Althoug 1 here makes sense, empirically seems that 2 is the minimum
		while (diff_wrap(firstTx32, ctx->firstTx) > 2)
		{
			hr = SoraURadioTxFree(TARGET_RADIO_TX, ctx->BufID[ctx->firstTx % cmd_fifo_queue_size]);
			ctx->firstTx++;
		}

		/*
		// Indeed, it never happened so removed to speed up the code
		if (lastTx32 != ctx->lastTx)
		{
			printf("This should not happen!\n");
		}
		*/

// For some reason we cannot read Sora from this thread, as timing collapses. Didn't spend much time investigating this issue.
/*
#ifdef READ_IN_WORKER_THREAD
		readSoraAndQueue();
#endif
*/

		cnt_rnd++;
	}

	ctx->TXRunning = false;

	return 0;
}




void prtStat(int32 num)
{
	if (num > 0)
	{
		printf("  %3ld   ", num);
	}
	else
	{
		printf("        ");
	}
}

// Print debug info
// Returns true if out of sync
bool printRadioStats(writeSoraCtx *ctx, readSoraCtx *rctx)
{
	HRESULT hres;

	static ULONG debug_ups_count=0, last_debug_ups_count=0;
	static ULONG d_cnt_Radio_TX_FIFO_rden=0, last_d_cnt_Radio_TX_FIFO_rden = 0;
	static ULONG d_cnt_Radio_TX_FIFO_pempty=0, last_d_cnt_Radio_TX_FIFO_pempty = 0;
	static ULONG d_cnt_Sora_FRL_2nd_RX_data=0, last_d_cnt_Sora_FRL_2nd_RX_data = 0;
	static ULONG d_cnt_RX_FIFO_2nd_data_out=0, last_d_cnt_RX_FIFO_2nd_data_out=0;
	static ULONG cnt_upsample_out=0, cnt_ups_ddr2_rden=0, cnt_Radio_TX_FIFO_full=0, cnt_TX_DDR_fifo_full=0;
	static ULONG last_cnt_upsample_out = 0, last_cnt_ups_ddr2_rden = 0, 
		last_cnt_Radio_TX_FIFO_full = 0, last_cnt_TX_DDR_fifo_full = 0;
	static ULONG Total_idle_cnt = 0, last_Total_idle_cnt = 0;
	static ULONG last_idle_duration=0, last_pc_idle_duration=0, last_pc_idle_w_inactive=0;
	static ULONG cmd_fifo_cnt=0, dbg_sync_cnt=0, deb_FRL_data_count=0, deb_FRL_TS_count=0;
	static ULONG cnt_cmd_fifo_empty=0, last_cnt_cmd_fifo_empty=0;
	static ULONG dbg_sync_underflow = 0, last_dbg_sync_underflow = 0;
	static ULONG dbg_sync_overflow = 0, last_dbg_sync_overflow = 0;
	ULONG TXOutAddr, TXOutSize, deb_reg;
	ULONG RXOverflow1, RXOverflow2;
	bool isOutOfSync;
	bool warning = false;
	static int toggle = 0;

	hres = ReadRegister(0x0510, &Total_idle_cnt);
	hres = ReadRegister(0x0514, &last_idle_duration);
	hres = ReadRegister(0x0518, &last_pc_idle_duration);
	hres = ReadRegister(0x051c, &last_pc_idle_w_inactive);

	// *** ControlQueue
	hres = ReadRegister(0x0520, &cmd_fifo_cnt);

	// *** RX_data_fifo_SORA_FRL_2nd_inst/RX_TS_fifo_2nd_inst
	// Data and TS count in RX_data_fifo_SORA_FRL_2nd_inst/RX_TS_fifo_2nd_inst
	hres = ReadRegister(0x0530, &deb_FRL_data_count);
	hres = ReadRegister(0x0534, &deb_FRL_TS_count);
	hres = ReadRegister(0x0528, &d_cnt_Sora_FRL_2nd_RX_data);	// Test difference in input sequence (special debugging)
	hres = ReadRegister(0x052c, &d_cnt_RX_FIFO_2nd_data_out);	// Test difference in input sequence (special debugging)
	hres = ReadRegister(0x0080, &RXOverflow1);					// Counts [RX_FIFO_full, RX_TS_FIFO_full]
	hres = ReadRegister(0x0084, &RXOverflow2);					// Counts [RX_FIFO_2nd_full, RX_TS_FIFO_2nd_full]

	// *** rxtx_queue_inst
	// Counts overflows/underflows for rxtx_queue_inst
	hres = ReadRegister(0x0538, &dbg_sync_underflow);
	hres = ReadRegister(0x053c, &dbg_sync_overflow);
	hres = ReadRegister(0x0524, &dbg_sync_cnt);					// Data count

	// *** Upsampler
	hres = ReadRegister(0x0540, &cnt_upsample_out);
	hres = ReadRegister(0x0544, &cnt_ups_ddr2_rden);
	hres = ReadRegister(0x0558, &debug_ups_count);

	// *** dma_ddr2
	// ToFRL_data_fifo_inst full counter
	hres = ReadRegister(0x0548, &cnt_Radio_TX_FIFO_full);
	hres = ReadRegister(0x054c, &cnt_TX_DDR_fifo_full);

	// *** Sora_FRL_RCB_inst
	hres = ReadRegister(0x055c, &d_cnt_Radio_TX_FIFO_rden);		// counts Radio_TX_FIFO_rden (from Sora_FRL_RCB_inst) 
	hres = ReadRegister(0x0560, &d_cnt_Radio_TX_FIFO_pempty);	// counts Radio_TX_FIFO_rden (from dma_ddr2_if)

	// *** register_table
	hres = ReadRegister(0x0564, &cnt_cmd_fifo_empty);			// Queue sync counter
	hres = ReadRegister(0x0568, &TXOutAddr);
	hres = ReadRegister(0x056c, &TXOutSize);
	hres = ReadRegister(0x0570, &deb_reg);




	// These should all be zeros in normal conditions
	if (Total_idle_cnt - last_Total_idle_cnt > 0 || rctx->rxBlocked)
	{
		warning = true;
	}
	if ((LONG)dbg_sync_underflow - (LONG)last_dbg_sync_underflow > 0)
	{
		warning = true;
	}
	if ((LONG)cnt_ups_ddr2_rden - (LONG)last_cnt_ups_ddr2_rden == 0)
	{
		warning = true;
	}
	if (outOfSyncs != 0)
	{
		isOutOfSync = true;
		warning = true;
	}
	else
	{
		isOutOfSync = false;
	}



	printf("\n");


	if (debugPrint(DEBUG_PRINT_RADIO))
	{

		/*
		printf("Total_idle_cnt: %u, last_idle_duration: %u, last_pc_idle_duration: %u, last_pc_idle_w_inactive: %u\n",
		Total_idle_cnt - last_Total_idle_cnt, last_idle_duration, last_pc_idle_duration, last_pc_idle_w_inactive);
		printf("                        queues: %u, %u, %u, %u\n",
		cmd_fifo_cnt, dbg_sync_cnt, deb_FRL_data_count, deb_FRL_TS_count);
		printf("                        prepareBlocked: %llu, transferBlocked: %llu, txBlocked: %llu, rxBlocked: %llu\n",
		ctx->prepareBlocked, ctx->transferBlocked, ctx->txBlocked, rctx->rxBlocked);
		printf("debugGlobalCnt = %ld, debugGlobalCnt2=%ld\n",
		debugGlobalCnt, debugGlobalCnt2);
		printf("cnt_upsample_out = %ld, cnt_ups_ddr2_rden = %ld, cnt_Radio_TX_FIFO_full = %ld, cnt_TX_DDR_fifo_full = %ld\n",
		cnt_upsample_out - last_cnt_upsample_out, cnt_ups_ddr2_rden - last_cnt_ups_ddr2_rden,
		cnt_Radio_TX_FIFO_full - last_cnt_Radio_TX_FIFO_full, cnt_TX_DDR_fifo_full - last_cnt_TX_DDR_fifo_full);
		printf("%ld %ld\n",
		(LONG)d_cnt_Sora_FRL_2nd_RX_data - (LONG)last_d_cnt_Sora_FRL_2nd_RX_data,
		(LONG)d_cnt_RX_FIFO_2nd_data_out - (LONG)last_d_cnt_RX_FIFO_2nd_data_out);
		*/


		// These should all be zeros in normal conditions
		if (Total_idle_cnt - last_Total_idle_cnt > 0 || rctx->rxBlocked)
		{
			printf(" ******** ");
		}

		printf("Total_idle_cnt: %u, prepareBlockedPC: %llu, transferBlockedPC: %llu, transferBlockedFPGA: %llu, rxBlocked: %llu/%llu\n",
			Total_idle_cnt - last_Total_idle_cnt, ctx->prepareBlocked, ctx->transferBlocked, ctx->transferBlockedFPGA,
			rctx->rxBlocked, rctx->rxBlocked + rctx->rxSucc);
		printf("     prepareBuf: %d, transferBuf: %d\n", ctx->prepareBuf, ctx->transferBuf);

		printf("     queues: %u, %u, %u, %u, %u\n",
			cmd_fifo_cnt, dbg_sync_cnt, deb_FRL_data_count, deb_FRL_TS_count, debug_ups_count);

		// These (debugGlobalCnt and debugGlobalCnt2) should be zeros when sending a sawf unction (0-999, 0-999, ...)
		printf("     cnt_Sora_FRL_2nd_RX_data=%ld, cnt_RX_FIFO_2nd_data_out=%ld\n",
			(LONG)d_cnt_Sora_FRL_2nd_RX_data - (LONG)last_d_cnt_Sora_FRL_2nd_RX_data,
			(LONG)d_cnt_RX_FIFO_2nd_data_out - (LONG)last_d_cnt_RX_FIFO_2nd_data_out);

		// These should all be zeros in normal conditions
		if ((LONG)dbg_sync_underflow - (LONG)last_dbg_sync_underflow > 0)
		{
			printf(" ******** ");
			warning = true;
		}
		printf("     cnt_cmd_fifo_empty = %ld, dbg_sync_underflow = %ld, dbg_sync_overflow = %ld\n",
			(LONG)cnt_cmd_fifo_empty - (LONG)last_cnt_cmd_fifo_empty,
			(LONG)dbg_sync_underflow - (LONG)last_dbg_sync_underflow,
			(LONG)dbg_sync_overflow - (LONG)last_dbg_sync_overflow);

		printf("     deb_reg = %lu\n", deb_reg);


		/*
		// DEBUG
		printf("     cnt_cmd_fifo_empty = %ld, TXOutAddr = %lu, TXOutSize = %lu\n", (LONG)cnt_cmd_fifo_empty - (LONG)last_cnt_cmd_fifo_empty, TXOutAddr, TXOutSize);
		printf("allocBuf(%d): ", naB);
		for (int ii = 0; ii < naB; ii++) printf("%lu ", allocBuf[ii]);
		printf("\nusedBuf(%d): ", nuB);
		for (int ii = 0; ii < nuB; ii++) printf("%lu (%lu) ", usedBuf[ii], usedBufSiz[ii]);
		printf("\n");

		printf("\nDallocBuf(%d): ", da);
		for (int ii = 0; ii < da; ii++) printf("%lu ", DallocBuf[ii]);
		printf("\nDusedBuf(%d): ", du);
		for (int ii = 0; ii < du; ii++) printf("%lu ", DusedBuf[ii]);
		printf("\nDusedInd(%d): ", du);
		for (int ii = 0; ii < du; ii++) printf("%lu ", DusedInd[ii]);
		*/





		// These should all be zeros in normal conditions
		if ((LONG)cnt_ups_ddr2_rden - (LONG)last_cnt_ups_ddr2_rden == 0)
		{
			printf(" ******** ");
			warning = true;
		}
		printf("     cnt_ups_ddr2_rden = %ld, cnt_upsample_out = %ld, cnt_Radio_TX_FIFO_full = %ld, cnt_TX_DDR_fifo_full = %ld\n",
			(LONG)cnt_ups_ddr2_rden - (LONG)last_cnt_ups_ddr2_rden,
			(LONG)cnt_upsample_out - (LONG)last_cnt_upsample_out,
			(LONG)cnt_Radio_TX_FIFO_full - (LONG)last_cnt_Radio_TX_FIFO_full,
			(LONG)cnt_TX_DDR_fifo_full - (LONG)last_cnt_TX_DDR_fifo_full);
		printf("     cnt_Radio_TX_FIFO_rden = %ld, cnt_Radio_TX_FIFO_pempty = %ld\n",
			(LONG)d_cnt_Radio_TX_FIFO_rden - (LONG)last_d_cnt_Radio_TX_FIFO_rden,
			(LONG)d_cnt_Radio_TX_FIFO_pempty - (LONG)last_d_cnt_Radio_TX_FIFO_pempty);

		// These should all be zeros in normal conditions
		if (outOfSyncs != 0)
		{
			isOutOfSync = true;
			warning = true;
			printf(" ******** ");
		}
		else
		{
			isOutOfSync = false;
		}
		printf("     outOfSyncs = %lu\n", outOfSyncs);


		// DEBUG
		printf("RX DEBUG: %u %u\n", RXOverflow1, RXOverflow2);

		// Print queue size samples for debugging
		printf("samp_PC_queue: ");
		for (int i = 0; i < no_tx_bufs + 1; i++)
		{
			printf("%d ", samp_PC_queue[i]);
			samp_PC_queue[i] = 0;
		}

		printf("\nsamp_FPGA_queue: ");
		for (int i = 0; i < cmd_fifo_queue_size + 1; i++)
		{
			printf("%d ", samp_FPGA_queue[i]);
			samp_FPGA_queue[i] = 0;
		}

		printf("\nsamp_FPGA_queue_underflow: ");
		for (int i = 0; i < cnt_samp_FPGA_underflow; i++)
		{
			printf("%llu ", samp_FPGA_underflow[i]);
		}
		cnt_samp_FPGA_underflow = 0;

#ifdef READ_IN_WORKER_THREAD 
		printf("\nsamp_RX_queue: free=%llu, full=%llu\n", samp_RX_queue_free, samp_RX_queue_full);
		samp_RX_queue_full = 0;
		samp_RX_queue_free = 0;
#endif
		printf("\n");
	}



	/*
	if (isOutOfSync) printf("!!! OUT OF SYNC !!!\n");
	if (warning) printf("*** ");
	printf("%c rxSCH=%ld, rxIP=%ld, rxCCH=%ld, rxRACH=%ld, txSCH=%ld, txIP=%ld, txCCH=%ld, errIP=%ld, errFrag=%ld, errRLC=%ld, errRRC=%ld\n", 
		(toggle == 0) ? '.' : '*', rxSCH, rxIP, rxCCH, rxRACH, txSCH, txIP, txCCH, errIP, errFrag, errRLC, errRRC);
	*/

	if (isOutOfSync) printf("!!! OUT OF SYNC !!!\n");
	if (warning) printf("*** ");
	printf("%c rxSCH,   rxIP,  rxCCH, rxRACH,  txSCH,   txIP,  txCCH,  errIP, errFrg, errRLC, errRRC, errQue, rrcRCN, rrcATT\n", (toggle == 0) ? '.' : '*');
	prtStat(rxSCH);
	prtStat(rxIP);
	prtStat(rxCCH);
	prtStat(rxRACH);
	prtStat(txSCH);
	prtStat(txIP);
	prtStat(txCCH);
	prtStat(errIP);
	prtStat(errFrag);
	prtStat(errRLC);
	prtStat(errRRC);
	prtStat(errQue);
	prtStat(RRCRecComp);
	prtStat(RRCAttComp);
	printf("\n");


	printf("MCS stats: ");
	for (int i = 0; i < MAC_NO_MCS; i++)
	{
		if (txMCS[i] > 0) printf("%2d(%3ld) ", i, txMCS[i]);
	}
	printf("\n");
	printf("NBR stats: ");
	for (int i = 0; i < MAC_NO_NBR; i++)
	{
		if (txNBR[i] > 0) printf("%2d(%3ld) ", i, txNBR[i]);
	}
	printf("\n");


	last_Total_idle_cnt = Total_idle_cnt;
	last_cnt_upsample_out = cnt_upsample_out;
	last_cnt_ups_ddr2_rden = cnt_ups_ddr2_rden;
	last_cnt_Radio_TX_FIFO_full = cnt_Radio_TX_FIFO_full;
	last_cnt_TX_DDR_fifo_full = cnt_TX_DDR_fifo_full;
	last_d_cnt_Sora_FRL_2nd_RX_data = d_cnt_Sora_FRL_2nd_RX_data;
	last_d_cnt_RX_FIFO_2nd_data_out = d_cnt_RX_FIFO_2nd_data_out;
	last_debug_ups_count = debug_ups_count;
	last_d_cnt_Radio_TX_FIFO_rden = d_cnt_Radio_TX_FIFO_rden;
	last_d_cnt_Radio_TX_FIFO_pempty = d_cnt_Radio_TX_FIFO_pempty;
	last_cnt_cmd_fifo_empty = cnt_cmd_fifo_empty;
	last_dbg_sync_underflow = dbg_sync_underflow;
	last_dbg_sync_overflow = dbg_sync_overflow;
	outOfSyncs = 0;


	ctx->prepareBlocked = 0; ctx->transferBlocked = 0; ctx->transferBlockedFPGA = 0; rctx->rxBlocked = 0;

	rxSCH = 0; 
	rxCCH = 0;
	rxRACH = 0;
	rxIP = 0;
	txSCH = 0;
	txIP = 0;
	txCCH = 0;
	errIP = 0, errFrag = 0, errRLC = 0, errRRC = 0, errQue = 0;
	RRCRecComp = 0, RRCAttComp = 0;
	memset(txMCS, 0, sizeof(int32)*MAC_NO_MCS);
	memset(txNBR, 0, sizeof(int32)*MAC_NO_NBR);
	toggle = toggle ^ 1;

	fflush(stdout);

	return (isOutOfSync);
}
#endif


