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
#include "sora_threads.h"
#include "sora_thread_queues.h"
#include "sora_ip.h"
#endif

#include "mac.h"
#include "mac_msg_parsing.h"



// File used in test mode to write out MAC output, for testing
FILE *fout = NULL;


ts_context *mac_rx_out_queue = NULL;


// Hacked way to send RCC Connection Setup
extern unsigned char RRC_ID[6];
extern bool RRC_ID_set;
extern bool RRC_ConnectionReconfiguration;


void __stdcall mac_rx_callback()
{
	MACULPacket *pkt = (MACULPacket *)buf_ctx_rx.chunk_output_buffer;
	unsigned char tx_bytes[2048];
	int tx_len;

	// Sanity check - packet cannot be longer than pkt->data length, which is MSG_DATA_LEN
	if (pkt->len > (ULMSG_DATA_LEN * 8))
	{
		pkt->len = (ULMSG_DATA_LEN * 8);
	}

	if (pkt->packetType == MAC_UL_CCH)
	{
		// Pass PUCCHs directly to mac_tx
		if (s_ts_isFull(mac_rx_out_queue, 0))
		{
			printf("mac_rx_out_queue full, dropping packet!\n");
		}
		else
		{
			s_ts_put(mac_rx_out_queue, 0, (char *)buf_ctx_rx.chunk_output_buffer);
		}
	}
	else
	{
		// Print MAC payload
		sdu out[64];
		int len_out = mac_decode(out, 64, pkt->data, pkt->len >> 3);

		// Ignore empty MAC packets (e.g. 3E1F00000000000000...)
		if (len_out > 0)
		{

			if (debugPrint(DEBUG_PRINT_ULSCH))
			{
				printf("RX Pkt at (%d, %d), %dB, CRC=%d: ", pkt->frame, pkt->subframe, pkt->len >> 3, pkt->crcOK);
				for (int i = 0; i < pkt->len >> 3; i++) printf("%02X", pkt->data[i]);
				printf("\n");
				print_sdus(out, len_out);
			}

			if (pkt->crcOK)
			{
				if (s_ts_isFull(mac_rx_out_queue, 0))
				{
					printf("mac_rx_out_queue full, dropping packet!\n");
				}
				else
				{
					s_ts_put(mac_rx_out_queue, 0, (char *)buf_ctx_rx.chunk_output_buffer);
				}
			}
		}
	}


	// Clear the output
	// IMPORTANT: This has to be done at the end of every mac_rx_callback() call!!!
	buf_ctx_rx.chunk_output_idx = 0;
}




//BOOLEAN __stdcall go_thread_rx(void * pParam)
void mac_rx(void * pParam)
{
	ULONGLONG ttstart, ttend;

#ifdef TEST
	fout = fopen(params_rx->outFileName, "w");
	if (fout == NULL)
	{
		printf("Cannot open output file %s. Exiting...\n", params_rx->outFileName);
	}
#endif

	printf("Starting RX ...\n");


	if (params_rx->outType == TY_FILE || params_rx->outType == TY_DUMMY)
	{
		// Used for debugging output waveforms
	}
	else
	{
		// Init RX out queue
		size_t size = sizeof(MACULPacket);
		mac_rx_out_queue = s_ts_init(1, &size);


		// In this MAC RX always first prepares the buffers in memory
		params_rx->outType = TY_MEM;

		buf_ctx_rx.mem_output_buf_size = MEM_BUF_RX_MAX_CNT * sizeof(MACULPacket);
		
		buf_ctx_rx.mem_output_buf = (void *)inmem_malloc(buf_ctx_rx.mem_output_buf_size);
		buf_ctx_rx.buf_output_callback = mac_rx_callback;
	}

	wpl_input_initialize_rx();

	// Start measuring time
	ttstart = SoraGetCPUTimestamp(&(params_tx->measurementInfo.tsinfo));

	// Run Ziria TX code
	wpl_go_rx();

	// Stop measuring time
	ttend = SoraGetCPUTimestamp(&(params_tx->measurementInfo.tsinfo));
	printf("RX Time Elapsed: %ld us \n",
		SoraTimeElapsed((ttend / 1000 - ttstart / 1000), &(params_tx->measurementInfo.tsinfo)));

	printf("RX Total input items (including EOF): %d (%d B), output items: %d (%d B)\n",
		buf_ctx_rx.total_in, buf_ctx_rx.total_in*buf_ctx_rx.size_in,
		buf_ctx_rx.total_out, buf_ctx_rx.total_out*buf_ctx_rx.size_out);

	if (params_rx->latencySampling > 0)
	{
		printf("RX Min write latency: %ld, max write latency: %ld\n", (ulong)params_rx->measurementInfo.minDiff, (ulong)params_rx->measurementInfo.maxDiff);
		printf("RX CDF: \n   ");
		unsigned int i = 0;
		while (i < params_rx->measurementInfo.aDiffPtr)
		{
			printf("%ld ", params_rx->measurementInfo.aDiff[i]);
			if (i % 10 == 9)
			{
				printf("\n   ");
			}
			i++;
		}
		printf("\n");
	}

#ifdef TEST
	fclose(fout);
#endif

}

