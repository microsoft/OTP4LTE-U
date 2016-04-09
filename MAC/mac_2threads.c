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

#ifdef BLADE_RF
#include "bladerf_radio.h"
#endif


#include "utils.h"
#include "mac.h"


// Setting test_IP
extern bool test_IP;





void init_mac(BlinkParams *params_tx, BlinkParams *params_rx)
{
#ifndef SW_TEST
#ifdef SORA_RF
	init_sora_hw();
#endif
#ifdef BLADE_RF
	if (BladeRF_RadioStart(params_tx, params_rx) < 0)
	{
		exit(1);
	}
#endif
#endif // SW_TEST

	// Start measuring time
	if (params_tx != NULL) {
		initMeasurementInfo(&(params_tx->measurementInfo), params_tx->latencyCDFSize);
		initBufCtxBlock(&buf_ctx_tx);
		initHeapCtxBlock(&heap_ctx_tx, params_tx->heapSize);
		wpl_global_init_tx(params_tx->heapSize);
	}
	if (params_rx != NULL) {
		initMeasurementInfo(&(params_rx->measurementInfo), params_rx->latencyCDFSize);
		initBufCtxBlock(&buf_ctx_rx);
		initHeapCtxBlock(&heap_ctx_rx, params_rx->heapSize);
		wpl_global_init_rx(params_rx->heapSize);
	}
}



void stop_mac()
{
	// Free thread separators
	// NB: these are typically allocated in blink_set_up_threads
	ts_free();

#ifndef SW_TEST
#ifdef SORA_RF
	stop_sora_hw();
#endif
#ifdef BLADE_RF
	BladeRF_RadioStop(params_tx, params_rx);
#endif
#endif // SW_TEST
}




// TX Thread
// Assumption: 
// - Input can be taken from anywhere. If the input is from IP, then we run our own MAC.
// - Output can be sent on the radio or in a file (for tests) or in dummy (for perf).
DWORD WINAPI go_thread_tx(void * pParam)
{
	thread_info *ti = (thread_info *)pParam;
	
	mac_tx(pParam);

	wpl_output_finalize_tx();

	fflush(stdout);
	ti->fRunning = false;

	return 0;
}




//BOOLEAN __stdcall go_thread_rx(void * pParam)
DWORD WINAPI go_thread_rx(void * pParam)
{
	ULONGLONG ttstart, ttend;
	thread_info *ti = (thread_info *)pParam;
	
	mac_rx(pParam);

	wpl_output_finalize_rx();

	fflush(stdout);
	ti->fRunning = false;

	return 0;
}

