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

/* 
   Copyright (c) Microsoft Corporation
   All rights reserved. 

   Licensed under the Apache License, Version 2.0 (the ""License""); you
   may not use this file except in compliance with the License. You may
   obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
   LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR
   A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.

   See the Apache Version 2.0 License for specific language governing
   permissions and limitations under the License.
*/
// Put to 0 to run tests on a PC without Sora board
#pragma once
#include <soratime.h>
#include <thread_func.h>
#include <stdlib.h>
#include <time.h>
#include <rxstream.h>
#include "params.h"
#include "numerics.h"


// Note on depay between TX and RX
// There are two queues on the TX side. The first one is in software, 
// which is a number of buffers (no_tx_bufs) queued at MAC level for transmission.
// The second one is hardware, queues at Sora's firmware. 
// This one stores cmd_fifo_queue_size buffers
// Each of these buffers has a size of TX-sora-tx-buffer-size bytes. 
// 
// Currently, we manage to successfully work with
// no_tx_bufs == 2, cmd_fifo_queue_size == 2 and TX-sora-tx-buffer-size == 16384
// This means that TX queues in total 
// (no_tx_bufs + cmd_fifo_queue_size) * TX-sora-tx-buffer-size = 65536 samples
// Since each subframe is 30720 samples, this is 2.133 subframes. 
// So the minimum delay between TX and RX is 3 subframes. 


// Sora's firmware has a queue for SoraURadioTx commands
// NOTE: Has to be power of 2!
#define MAX_CMD_FIFO_QUEUE_SIZE		16
extern int cmd_fifo_queue_size;

// In this implementation of sora_radio we treat params->TXBuffer
// as an array of buffers that allow for continuous transmission
#define MAX_NO_TX_BUFS				16
extern int no_tx_bufs;

// Max size of buffer between worker and RX threads
#define MAX_WORK_RX_BUF				1024



// Define the macro below (preferably through Makefile) to run tests without Sora
// #define SW_TEST

// TX and RX radios are currently hard-coded by firmware design
#define TARGET_RADIO_TX 0
#define TARGET_RADIO_RX 1


// We use LSB of TX and RX sample as a low-rate synchronization channel
// We will set LSB to 1 every SYNC_PERIOD, and to 0 otherwise
// SYNC_PERIOD is at a start of each frame, once every 10ms
#define RXTX_SYNC_PERIOD (((2048 + 160) + 6 * (2048 + 144)) * 20)



typedef struct {
	// Blocking statistics
	ULONGLONG rxBlocked;
	ULONGLONG rxSucc;

	// RXTX Sync variables
	volatile bool RXSynced;
	long syncStateCnt;
	long RXSymbol;
	long RXFrame;
	ULONG TXBufferSize;

	// read queue between worker and RX threads
	// see description of READ_IN_WORKER_THREAD for more details
	ts_context * readQueue;

} readSoraCtx;


// TODO: Make variables here cache aligned to avoid cache misses
typedef struct {
	PVOID TXBuffers[MAX_NO_TX_BUFS];
	ULONG BufID[MAX_CMD_FIFO_QUEUE_SIZE];
	ULONG TXBufferSize;

	// Pointers into the racing TXBuffers queue:
	// prepareBuf cannot go beyond transferBuf, 
	// transferBuf cannot go beyond prepareBuf
	// Note: this queue is shared between threads
	volatile int prepareBuf;
	volatile int transferBuf;

	// Pointers in a circular BufID queue, which corresponds to
	// Sora cmd_fifo_queue, denoting the first and the last
	// queues command
	ULONG firstTx;
	ULONG lastTx;


	// Blocking statistics
	ULONGLONG prepareBlocked;
	ULONGLONG transferBlocked;
	ULONGLONG transferBlockedFPGA;

	// RXTX Sync variables
	readSoraCtx *rctx;

	ulong idleTXDetected;
	bool TXTransferRunning;
	bool TXRunning;
} writeSoraCtx;



void init_sora_hw();
void stop_sora_hw();


void initTXCtx(writeSoraCtx *ctx, readSoraCtx *rctx);
void initRXCtx(readSoraCtx *rctx, ULONG buf_size);


void RadioSetFirmwareParameters();

void RadioStart(BlinkParams *params);
void RadioStop(BlinkParams *params);
void InitSoraRx(BlinkParams *params);
void InitSoraTx(BlinkParams *params);


// BOZIDAR TODO: Once debugging is finished, put these as inline and move here!
void readSora(BlinkParams *params, complex16 *ptr, int size);
void writeSora(BlinkParams *params, complex16 *ptr, ULONG size);


// Sora TX worker thread
DWORD WINAPI SoraTXWorker(void * pParam);



bool printRadioStats(writeSoraCtx *ctx, readSoraCtx *rctx);

int Sora_SwitchTXFrequency(BlinkParams *params, bool center);



// Modification of SoraRadioReadRxStream that promptly returns on an empty buffer (no spin wait)
__forceinline
HRESULT
SORAAPI
FastSoraRadioReadRxStream(
	PSORA_RADIO_RX_STREAM pRxStream,
	FLAG * pbTouched,
	SignalBlock& block)
{
	PRX_BLOCK pbScanPoint = SoraRadioGetRxStreamPos(pRxStream);
	HRESULT hr =
		SoraCheckSignalBlock(pbScanPoint, SoraGetStreamVStreamMask(pRxStream), 1, pbTouched);
	if (FAILED(hr))
		return hr; // hardware error

	block = (SignalBlock&)(pbScanPoint->u.SampleBlock);

	// Advance scan ponter
	__SoraRadioAdvanceRxStreamPos(pRxStream);
	return hr;
}

